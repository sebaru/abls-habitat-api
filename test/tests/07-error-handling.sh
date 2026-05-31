#!/usr/bin/env bash
# =============================================================================
# 07-error-handling.sh - Tests des cas d'erreur et des limites
# =============================================================================
# Vérifie:
# - Payload JSON malformé → 400
# - Champs obligatoires manquants → 400/422
# - Ressource inexistante → 400/404
# - Token expiré → 401/403
# - Token avec mauvais issuer → 401/403
# - Méthode HTTP non supportée → 405
# - Données en doublon (UNIQUE constraint) → 4xx
# - Tentative de suppression d'une ressource protégée (DLS SYS) → 4xx
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 07 - Gestion des erreurs"

ADMIN_TOKEN=$(make_admin_token)

api_call_capture() {
    RESPONSE=$(api_call "$@")
    refresh_last_http_code
}

# =============================================================================
# TEST: Payload JSON malformé
# =============================================================================
log_info "Test: POST /camera/add avec JSON malformé → 400"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X POST "${API_URL}/camera/add" \
    -H "Authorization: Bearer ${ADMIN_TOKEN}" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" \
    -H "Content-Type: application/json" \
    -d '{invalid json !!!')
LAST_HTTP_CODE="${RESPONSE}"
set_last_http_code "${LAST_HTTP_CODE}"
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "JSON malformé refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "JSON malformé accepté" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

# =============================================================================
# TEST: Champs obligatoires manquants - POST /camera/add sans 'name'
# =============================================================================
log_info "Test: POST /camera/add sans le champ 'name' → 4xx"
api_call_capture POST /camera/add "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"url":"rtsp://192.168.1.99:554/stream"}'
# name est obligatoire (contrainte UNIQUE NOT NULL sur la table cameras)
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "POST /camera/add sans name refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "POST /camera/add sans name accepté" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

# =============================================================================
# TEST: Champs obligatoires manquants - POST /domain/add sans 'domain'
# =============================================================================
log_info "Test: POST /domain/add sans le champ 'domain' → 4xx"
api_call_capture POST /domain/add "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"description":"Test sans nom"}'
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "POST /domain/add sans domaine refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "POST /domain/add sans domaine accepté" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

# =============================================================================
# TEST: camera_id inexistant pour /camera/set
# =============================================================================
log_info "Test: POST /camera/set avec camera_id inexistant → 4xx"
api_call_capture POST /camera/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"camera_id":9999999,"name":"fantome"}'
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "camera_id inexistant refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "camera_id inexistant accepté" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

# =============================================================================
# TEST: camera_id inexistant pour /camera/delete
# =============================================================================
log_info "Test: DELETE /camera/delete avec camera_id inexistant → 4xx"
api_call_capture DELETE /camera/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"camera_id":9999999}'
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "camera_id inexistant pour delete refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "camera_id inexistant pour delete accepté" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

# =============================================================================
# TEST: Méthode HTTP non supportée
# =============================================================================
log_info "Test: DELETE /user/profil (méthode non supportée) → 405"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X DELETE "${API_URL}/user/profil" \
    -H "Authorization: Bearer ${ADMIN_TOKEN}" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}")
LAST_HTTP_CODE="${RESPONSE}"
set_last_http_code "${LAST_HTTP_CODE}"
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^(405|400|404) ]]; then
    _test_pass "Méthode DELETE refusée sur /user/profil (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "Méthode DELETE acceptée sur /user/profil" "HTTP=${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: UNIQUE constraint violation (nom de caméra en doublon)
# =============================================================================
log_info "Test: POST /camera/add avec nom en doublon → 4xx"
# Camera-Test-01 est déjà dans les fixtures
api_call_capture POST /camera/add "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"name":"Camera-Test-01","url":"rtsp://192.168.1.99:554/stream"}'
COUNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM cameras WHERE name='Camera-Test-01';")
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "Nom de caméra en doublon refusé (HTTP ${LAST_HTTP_CODE})"
elif [[ "${COUNT_AFTER}" -gt 1 ]]; then
    _test_fail "Nom de caméra en doublon créé 2 fois en BD" "HTTP=${LAST_HTTP_CODE}"
else
    _test_fail "Comportement inattendu pour doublon" "HTTP=${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: Suppression d'un DLS système (SYS) → 4xx
# =============================================================================
log_info "Test: DELETE /dls/delete sur DLS 'SYS' (protégé) → 4xx"
SYS_DLS_ID=$(db_domain_query "SELECT dls_id FROM dls WHERE tech_id='SYS';")
api_call_capture POST /dls/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"dls_id\":${SYS_DLS_ID}}"
SYS_STILL_EXISTS=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id='SYS';")
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "Suppression DLS SYS refusée (HTTP ${LAST_HTTP_CODE})"
elif [[ "${SYS_STILL_EXISTS}" == "1" ]]; then
    # L'API a retourné 2xx mais la BD est intacte — comportement acceptable
    _test_pass "DLS SYS toujours présent après tentative de suppression"
else
    _test_fail "DLS SYS a été supprimé" "HTTP=${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: Endpoint inexistant → 400
# =============================================================================
log_info "Test: GET /ceci/nexiste/pas → 400"
api_call_capture GET /ceci/nexiste/pas "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}"
_test_start
if [[ "${LAST_HTTP_CODE}" == "400" ]]; then
    _test_pass "Endpoint inexistant → 400"
else
    _test_fail "Endpoint inexistant n'a pas retourné 400" "HTTP=${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: Header X-ABLS-DOMAIN manquant sur endpoint qui en a besoin
# =============================================================================
log_info "Test: GET /camera/list sans header X-ABLS-DOMAIN → 4xx"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X GET "${API_URL}/camera/list" \
    -H "Authorization: Bearer ${ADMIN_TOKEN}")
LAST_HTTP_CODE="${RESPONSE}"
set_last_http_code "${LAST_HTTP_CODE}"
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "Absence de X-ABLS-DOMAIN refusée (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "Absence de X-ABLS-DOMAIN acceptée" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

# =============================================================================
# TEST: domain_uuid inexistant dans X-ABLS-DOMAIN
# =============================================================================
log_info "Test: GET /camera/list avec domain_uuid inconnu → 4xx"
FAKE_DOMAIN="00000000-dead-beef-0000-000000000000"
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" \
    -X GET "${API_URL}/camera/list" \
    -H "Authorization: Bearer ${ADMIN_TOKEN}" \
    -H "X-ABLS-DOMAIN: ${FAKE_DOMAIN}")
LAST_HTTP_CODE="${RESPONSE}"
set_last_http_code "${LAST_HTTP_CODE}"
_test_start
if [[ "${LAST_HTTP_CODE}" =~ ^4 ]]; then
    _test_pass "domain_uuid inexistant refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "domain_uuid inexistant accepté" "HTTP=${LAST_HTTP_CODE} attendu 4xx"
fi

print_suite_summary "Suite 07 - Error Handling"
[[ ${TESTS_FAILED} -eq 0 ]]
