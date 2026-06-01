#!/usr/bin/env bash
# =============================================================================
# 08-syn.sh - Tests des synoptiques
# =============================================================================
# Endpoints testés: GET /syn/list, GET /syn/child, GET /syn/show,
#                   POST /syn/set, POST /syn/save, POST /syn/clic,
#                   POST /syn/ack, POST /syn/move,
#                   DELETE /syn/delete,
#                   GET /syn/camera/list, POST /syn/camera/add,
#                   DELETE /syn/camera/delete
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 08 - Synoptiques"

ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /syn/list
# =============================================================================
log_info "Test: GET /syn/list"
RESPONSE=$(api_call GET /syn/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /syn/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "syns" "GET /syn/list retourne des synoptiques"

# HOME doit être dans la liste
HOME_FOUND=$(echo "${RESPONSE}" | jq -r '.syns[] | select(.page == "HOME") | .page' 2>/dev/null)
_test_start
if [[ "${HOME_FOUND}" == "HOME" ]]; then
    _test_pass "GET /syn/list contient le synoptique HOME"
else
    _test_fail "GET /syn/list ne contient pas le synoptique HOME" "${RESPONSE}"
fi

# Cohérence avec la BD
SYNS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM syns;")
SYNS_IN_API=$(echo "${RESPONSE}" | jq '.syns | length' 2>/dev/null)
_test_start
if [[ "${SYNS_IN_API}" == "${SYNS_IN_DB}" ]]; then
    _test_pass "GET /syn/list nombre cohérent avec BD (${SYNS_IN_DB})"
else
    _test_fail "GET /syn/list nombre incohérent" "API=${SYNS_IN_API}, BD=${SYNS_IN_DB}"
fi

# =============================================================================
# TEST: GET /syn/child
# =============================================================================
log_info "Test: GET /syn/child?syn_id=1"
RESPONSE=$(api_call GET "/syn/child?syn_id=1" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /syn/child → HTTP 200"
_test_start
if echo "${RESPONSE}" | jq -e '.' >/dev/null 2>&1; then
    _test_pass "GET /syn/child retourne du JSON valide"
else
    _test_fail "GET /syn/child ne retourne pas du JSON valide" "${RESPONSE}"
fi

# =============================================================================
# TEST: GET /syn/show
# =============================================================================
log_info "Test: GET /syn/show?page=HOME"
RESPONSE=$(api_call GET "/syn/show?page=HOME" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /syn/show → HTTP 200"
_test_start
if echo "${RESPONSE}" | jq -e '.' >/dev/null 2>&1; then
    _test_pass "GET /syn/show retourne du JSON valide"
else
    _test_fail "GET /syn/show ne retourne pas du JSON valide" "${RESPONSE}"
fi

# =============================================================================
# TEST: POST /syn/set - Modifier le libellé du synoptique TEST_SYN
# =============================================================================
log_info "Test: POST /syn/set - modification libellé"
TEST_SYN_ID=$(db_domain_query "SELECT syn_id FROM syns WHERE page='TEST_SYN' LIMIT 1;")
[[ -z "${TEST_SYN_ID}" ]] && TEST_SYN_ID=2

RESPONSE=$(api_call POST /syn/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"syn_id\":${TEST_SYN_ID},\"libelle\":\"Synoptique Test Modifié\",\"page\":\"TEST_SYN\",\"access_level\":6,\"place\":0}")

assert_http_status 200 "POST /syn/set → HTTP 200"

LIB_DB=$(db_domain_query "SELECT libelle FROM syns WHERE syn_id=${TEST_SYN_ID} LIMIT 1;")
_test_start
if [[ "${LIB_DB}" == "Synoptique Test Modifié" ]]; then
    _test_pass "POST /syn/set libellé mis à jour en BD"
else
    _test_fail "POST /syn/set libellé non mis à jour en BD" "BD='${LIB_DB}'"
fi

# Restaurer
api_call POST /syn/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"syn_id\":${TEST_SYN_ID},\"libelle\":\"Synoptique Test\",\"page\":\"TEST_SYN\",\"access_level\":6,\"place\":0}" >/dev/null

log_info "Test: POST /syn/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /syn/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"syn_id\":${TEST_SYN_ID},\"libelle\":\"Tentative\",\"page\":\"TEST_SYN\",\"access_level\":0,\"place\":0}")
assert_http_status 403 "POST /syn/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /syn/save - Sauvegarder l'image d'un synoptique
# =============================================================================
log_info "Test: POST /syn/save - sauvegarde image synoptique"
TINY_PNG="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg=="
RESPONSE=$(api_call POST /syn/save "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"syn_id\":${TEST_SYN_ID},\"image\":\"${TINY_PNG}\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /syn/save → HTTP 200"
else
    _test_fail "POST /syn/save" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /syn/clic - Clic sur un élément du synoptique
# =============================================================================
log_info "Test: POST /syn/clic - action clic"
RESPONSE=$(api_call POST /syn/clic "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"syn_id\":${TEST_SYN_ID},\"tech_id\":\"TEST_DLS\",\"acronyme\":\"TEST_DI\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" || "${LAST_HTTP_CODE}" == "400" ]]; then
    _test_pass "POST /syn/clic → HTTP ${LAST_HTTP_CODE} (pas d'erreur 500)"
else
    _test_fail "POST /syn/clic" "code HTTP inattendu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /syn/ack - Acquittement alarme depuis synoptique
# =============================================================================
log_info "Test: POST /syn/ack - acquittement"
RESPONSE=$(api_call POST /syn/ack "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" || "${LAST_HTTP_CODE}" == "400" ]]; then
    _test_pass "POST /syn/ack → HTTP ${LAST_HTTP_CODE} (pas d'erreur 500)"
else
    _test_fail "POST /syn/ack" "code HTTP inattendu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /syn/move - Déplacer un élément du synoptique
# =============================================================================
log_info "Test: POST /syn/move - déplacement élément"
RESPONSE=$(api_call POST /syn/move "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"syn_id\":${TEST_SYN_ID},\"tech_id\":\"TEST_DLS\",\"acronyme\":\"TEST_DI\",\"posx\":10,\"posy\":20}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" || "${LAST_HTTP_CODE}" == "400" ]]; then
    _test_pass "POST /syn/move → HTTP ${LAST_HTTP_CODE} (pas d'erreur 500)"
else
    _test_fail "POST /syn/move" "code HTTP inattendu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: GET /syn/camera/list
# =============================================================================
log_info "Test: GET /syn/camera/list?syn_id=1"
RESPONSE=$(api_call GET "/syn/camera/list?syn_id=1" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /syn/camera/list → HTTP 200"
_test_start
if echo "${RESPONSE}" | jq -e '.' >/dev/null 2>&1; then
    _test_pass "GET /syn/camera/list retourne du JSON valide"
else
    _test_fail "GET /syn/camera/list ne retourne pas du JSON valide" "${RESPONSE}"
fi

# =============================================================================
# TEST: POST /syn/camera/add + DELETE /syn/camera/delete
# =============================================================================
log_info "Test: POST /syn/camera/add - ajout caméra au synoptique TEST_SYN"
CAM_ID=$(db_domain_query "SELECT camera_id FROM cameras WHERE name='Camera-Test-02' LIMIT 1;")

if [[ -n "${CAM_ID}" ]]; then
    # Supprimer d'abord si existe déjà
    db_domain_query "DELETE FROM syn_cameras WHERE syn_id=${TEST_SYN_ID} AND camera_id=${CAM_ID};" >/dev/null 2>&1 || true

    SYN_CAM_COUNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM syn_cameras WHERE syn_id=${TEST_SYN_ID};")
    RESPONSE=$(api_call POST /syn/camera/add "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"syn_id\":${TEST_SYN_ID},\"camera_id\":${CAM_ID}}")

    assert_http_status 200 "POST /syn/camera/add → HTTP 200"

    SYN_CAM_COUNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM syn_cameras WHERE syn_id=${TEST_SYN_ID};")
    _test_start
    if [[ "$((SYN_CAM_COUNT_BEFORE + 1))" == "${SYN_CAM_COUNT_AFTER}" ]]; then
        _test_pass "POST /syn/camera/add: association créée en BD"
    else
        _test_fail "POST /syn/camera/add: association non créée en BD" \
            "avant=${SYN_CAM_COUNT_BEFORE}, après=${SYN_CAM_COUNT_AFTER}"
    fi

    log_info "Test: DELETE /syn/camera/delete - suppression association"
    RESPONSE=$(api_call DELETE /syn/camera/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"syn_id\":${TEST_SYN_ID},\"camera_id\":${CAM_ID}}")

    assert_http_status 200 "DELETE /syn/camera/delete → HTTP 200"

    SYN_CAM_COUNT_DEL=$(db_domain_query "SELECT COUNT(*) FROM syn_cameras WHERE syn_id=${TEST_SYN_ID} AND camera_id=${CAM_ID};")
    _test_start
    if [[ "${SYN_CAM_COUNT_DEL}" == "0" ]]; then
        _test_pass "DELETE /syn/camera/delete: association supprimée de la BD"
    else
        _test_fail "DELETE /syn/camera/delete: association toujours présente en BD"
    fi
else
    log_warn "Camera-Test-02 introuvable, tests syn/camera/add et delete ignorés"
fi

# =============================================================================
# TEST: DELETE /syn/delete - Suppression du synoptique TEST_SYN
# =============================================================================
log_info "Test: DELETE /syn/delete - synoptique TEST_SYN"
# Créer un synoptique temporaire
db_domain_query "INSERT IGNORE INTO syns (parent_id, libelle, page, access_level) VALUES (1, 'Syn Temp Delete', 'TEST_SYN_DEL', 0);" >/dev/null 2>&1 || true
SYN_DEL_ID=$(db_domain_query "SELECT syn_id FROM syns WHERE page='TEST_SYN_DEL' LIMIT 1;")

if [[ -n "${SYN_DEL_ID}" ]]; then
    SYN_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM syns;")
    RESPONSE=$(api_call DELETE /syn/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"syn_id\":${SYN_DEL_ID}}")

    assert_http_status 200 "DELETE /syn/delete → HTTP 200"

    SYN_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM syns;")
    _test_start
    if [[ "$((SYN_CNT_BEFORE - 1))" == "${SYN_CNT_AFTER}" ]]; then
        _test_pass "DELETE /syn/delete: synoptique supprimé de la BD"
    else
        _test_fail "DELETE /syn/delete: compteur incohérent" \
            "avant=${SYN_CNT_BEFORE}, après=${SYN_CNT_AFTER}"
    fi
else
    log_warn "Création du synoptique temporaire échouée, test delete ignoré"
fi

print_suite_summary "Suite 08 - Synoptiques"
[[ ${TESTS_FAILED} -eq 0 ]]
