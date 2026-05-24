#!/usr/bin/env bash
# =============================================================================
# 02-user-auth.sh - Tests d'authentification et endpoints utilisateurs
# =============================================================================
# Endpoints testés: GET /user/profil, POST /user/list, POST /user/set
# JWT requis + X-ABLS-DOMAIN pour les endpoints protégés.
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 02 - Authentification et endpoints utilisateurs"

# Préparation des tokens
ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)
DISABLED_TOKEN=$(make_disabled_token)

# =============================================================================
# TEST: Accès sans token
# =============================================================================
log_info "Test: GET /user/profil sans token"
RESPONSE=$(curl -s -w "\n__HTTP_CODE:%{http_code}" \
    -X GET "${API_URL}/user/profil" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" 2>/dev/null)
LAST_HTTP_CODE="${RESPONSE##*__HTTP_CODE:}"
assert_http_status 401 "GET /user/profil sans token → HTTP 401"

# =============================================================================
# TEST: Accès sans header X-ABLS-DOMAIN
# =============================================================================
log_info "Test: GET /user/profil sans X-ABLS-DOMAIN"
RESPONSE=$(curl -s -w "\n__HTTP_CODE:%{http_code}" \
    -X GET "${API_URL}/user/profil" \
    -H "Authorization: Bearer ${ADMIN_TOKEN}" 2>/dev/null)
LAST_HTTP_CODE="${RESPONSE##*__HTTP_CODE:}"
# L'API retourne 400 si le header X-ABLS-DOMAIN est absent
assert_http_status 400 "GET /user/profil sans X-ABLS-DOMAIN → HTTP 400"

# =============================================================================
# TEST: GET /user/profil avec admin valide
# =============================================================================
log_info "Test: GET /user/profil - admin"
RESPONSE=$(api_call GET /user/profil "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /user/profil admin → HTTP 200"
assert_json_field "${RESPONSE}" "user_uuid" "${TEST_ADMIN_UUID}" "GET /user/profil user_uuid correct"
assert_json_field "${RESPONSE}" "email" "admin@test.abls-habitat.fr" "GET /user/profil email correct"
assert_json_field "${RESPONSE}" "enable" "true" "GET /user/profil enable=true"

# Vérifier que l'access_level retourné correspond à la BD
ADMIN_LEVEL_DB=$(db_query "SELECT access_level FROM users_grants WHERE user_uuid='${TEST_ADMIN_UUID}' AND domain_uuid='${TEST_DOMAIN_UUID}';" abls_master)
ADMIN_LEVEL_API=$(echo "${RESPONSE}" | jq -r '.access_level // empty' 2>/dev/null)
_test_start
if [[ "${ADMIN_LEVEL_API}" == "${ADMIN_LEVEL_DB}" ]]; then
    _test_pass "GET /user/profil access_level cohérent avec BD (${ADMIN_LEVEL_DB})"
else
    _test_fail "GET /user/profil access_level incohérent" "API=${ADMIN_LEVEL_API}, BD=${ADMIN_LEVEL_DB}"
fi

# =============================================================================
# TEST: GET /user/profil avec user standard
# =============================================================================
log_info "Test: GET /user/profil - user standard (level 6)"
RESPONSE=$(api_call GET /user/profil "${USER_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /user/profil user standard → HTTP 200"
assert_json_field "${RESPONSE}" "user_uuid" "${TEST_USER_UUID}" "GET /user/profil user UUID correct"
assert_json_field "${RESPONSE}" "access_level" "6" "GET /user/profil access_level=6"

# =============================================================================
# TEST: GET /user/profil avec user désactivé
# =============================================================================
log_info "Test: GET /user/profil - user désactivé"
RESPONSE=$(api_call GET /user/profil "${DISABLED_TOKEN}" "${TEST_DOMAIN_UUID}")

# L'utilisateur désactivé (enable=0) doit être refusé
assert_http_status 401 "GET /user/profil user désactivé → HTTP 401"

# =============================================================================
# TEST: GET /user/list (access_level ≥ 2)
# =============================================================================
log_info "Test: GET /user/list - admin"
RESPONSE=$(api_call GET /user/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /user/list admin → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "users" "GET /user/list contient des utilisateurs"

# L'utilisateur admin doit être dans la liste
USER_IN_LIST=$(echo "${RESPONSE}" | jq -r '.users[] | select(.user_uuid == "'${TEST_ADMIN_UUID}'") | .user_uuid' 2>/dev/null)
_test_start
if [[ "${USER_IN_LIST}" == "${TEST_ADMIN_UUID}" ]]; then
    _test_pass "GET /user/list contient l'admin Test"
else
    _test_fail "GET /user/list ne contient pas l'admin Test" "réponse: ${RESPONSE}"
fi

# =============================================================================
# TEST: GET /user/list avec user readonly (access_level=1, requis ≥ 2)
# =============================================================================
log_info "Test: GET /user/list - readonly (access insuffisant)"
RESPONSE=$(api_call GET /user/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 403 "GET /user/list readonly → HTTP 403 (accès refusé)"

# =============================================================================
# TEST: POST /user/set - Modifier le username de l'utilisateur standard
# =============================================================================
log_info "Test: POST /user/set - modification username"
RESPONSE=$(api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_USER_UUID}\",\"username\":\"User Test Modifié\"}")

assert_http_status 200 "POST /user/set → HTTP 200"

# Vérifier la modification en base
assert_master_db_field "users" "username" "User Test Modifié" \
    "POST /user/set mise à jour en BD" \
    "user_uuid='${TEST_USER_UUID}'"

# Remettre le username initial
api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_USER_UUID}\",\"username\":\"User Test\"}" >/dev/null

# =============================================================================
# TEST: POST /user/set - sans accès suffisant (readonly, level 1)
# =============================================================================
log_info "Test: POST /user/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /user/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_USER_UUID}\",\"username\":\"Tentative non autorisée\"}")

assert_http_status 403 "POST /user/set readonly → HTTP 403"

# Vérification que la BD n'a PAS été modifiée
assert_master_db_field "users" "username" "User Test" \
    "POST /user/set non autorisé n'a pas modifié la BD" \
    "user_uuid='${TEST_USER_UUID}'"

print_suite_summary "Suite 02 - User Auth"
[[ ${TESTS_FAILED} -eq 0 ]]
