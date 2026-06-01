#!/usr/bin/env bash
# =============================================================================
# 16-mapping-mnemos.sh - Tests des endpoints Mapping et Mnémos
# =============================================================================
# Endpoints testés:
#   Mapping : GET /mapping/list, POST /mapping/set, DELETE /mapping/delete
#   Mnémos  : GET /mnemos/tech_ids, GET /mnemos/validate,
#             GET /mnemos/list, POST /mnemos/set
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 16 - Mapping et Mnémos"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# MAPPING
# =============================================================================

# TEST: GET /mapping/list
log_info "Test: GET /mapping/list"
RESPONSE=$(api_call GET /mapping/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /mapping/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "mappings" "GET /mapping/list retourne des mappings"

log_info "Test: GET /mapping/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /mapping/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /mapping/list readonly → HTTP 403"

# TEST: POST /mapping/set - Modifier un mapping existant
log_info "Test: POST /mapping/set - mise à jour du mapping TEST_MODBUS/MOD_AI_01"
RESPONSE=$(api_call POST /mapping/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id_src":"TEST_MODBUS","acronyme_src":"MOD_AI_01","tech_id_dst":"TEST_DLS","acronyme_dst":"TEST_AI","enable":true}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /mapping/set → HTTP 200"
else
    _test_fail "POST /mapping/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

MAPPING_OK=$(db_domain_query \
    "SELECT COUNT(*) FROM mappings WHERE tech_id_src='TEST_MODBUS' AND acronyme_src='MOD_AI_01' AND tech_id_dst='TEST_DLS' AND acronyme_dst='TEST_AI';")
_test_start
if [[ "${MAPPING_OK}" -ge 1 ]]; then
    _test_pass "POST /mapping/set: mapping présent en BD"
else
    _test_fail "POST /mapping/set: mapping absent de la BD" "count=${MAPPING_OK}"
fi

log_info "Test: POST /mapping/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /mapping/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id_src":"TEST_MODBUS","acronyme_src":"MOD_AI_01","tech_id_dst":"TEST_DLS","acronyme_dst":"TEST_AI","enable":true}')
assert_http_status 403 "POST /mapping/set readonly → HTTP 403"

# TEST: DELETE /mapping/delete - Supprimer puis recréer le mapping
log_info "Test: DELETE /mapping/delete - suppression du mapping TEST_MODBUS/MOD_AI_01"
MAP_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM mappings;")

RESPONSE=$(api_call DELETE /mapping/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id_src":"TEST_MODBUS","acronyme_src":"MOD_AI_01","tech_id_dst":"TEST_DLS","acronyme_dst":"TEST_AI"}')

assert_http_status 200 "DELETE /mapping/delete → HTTP 200"

MAP_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM mappings;")
_test_start
if [[ "$((MAP_CNT_BEFORE - 1))" == "${MAP_CNT_AFTER}" ]]; then
    _test_pass "DELETE /mapping/delete: mapping supprimé de la BD"
else
    _test_fail "DELETE /mapping/delete: compteur incohérent" \
        "avant=${MAP_CNT_BEFORE}, après=${MAP_CNT_AFTER}"
fi

# Restaurer le mapping supprimé pour ne pas casser d'autres tests
db_domain_query "INSERT IGNORE INTO mappings (tech_id_src, acronyme_src, tech_id_dst, acronyme_dst, enable) VALUES ('TEST_MODBUS','MOD_AI_01','TEST_DLS','TEST_AI',1);" >/dev/null 2>&1 || true

# =============================================================================
# MNÉMOS
# =============================================================================

# TEST: GET /mnemos/tech_ids
log_info "Test: GET /mnemos/tech_ids"
RESPONSE=$(api_call GET /mnemos/tech_ids "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /mnemos/tech_ids → HTTP 200"

TEST_DLS_FOUND=$(echo "${RESPONSE}" | jq -r 'if type=="array" then .[] | select(. == "TEST_DLS") elif .tech_ids then .tech_ids[] | select(. == "TEST_DLS") else empty end' 2>/dev/null)
_test_start
if [[ "${TEST_DLS_FOUND}" == "TEST_DLS" ]]; then
    _test_pass "GET /mnemos/tech_ids contient TEST_DLS"
else
    _test_fail "GET /mnemos/tech_ids ne contient pas TEST_DLS" "${RESPONSE}"
fi

log_info "Test: GET /mnemos/tech_ids - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /mnemos/tech_ids "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /mnemos/tech_ids readonly → HTTP 403"

# TEST: GET /mnemos/validate
log_info "Test: GET /mnemos/validate?tech_id=TEST_DLS"
RESPONSE=$(api_call GET "/mnemos/validate?tech_id=TEST_DLS" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /mnemos/validate → HTTP 200"
else
    _test_fail "GET /mnemos/validate" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# TEST: GET /mnemos/list
log_info "Test: GET /mnemos/list?tech_id=TEST_DLS"
RESPONSE=$(api_call GET "/mnemos/list?tech_id=TEST_DLS" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /mnemos/list → HTTP 200"

TEST_AI_FOUND=$(echo "${RESPONSE}" | jq -r '.. | objects | select(.acronyme? == "TEST_AI") | .acronyme' 2>/dev/null | head -1)
_test_start
if [[ "${TEST_AI_FOUND}" == "TEST_AI" ]]; then
    _test_pass "GET /mnemos/list contient TEST_AI"
else
    _test_fail "GET /mnemos/list ne contient pas TEST_AI" "${RESPONSE}"
fi

# TEST: POST /mnemos/set - Mettre à jour le libellé d'un mnémo
log_info "Test: POST /mnemos/set - mise à jour libellé TEST_DLS/TEST_AI"
RESPONSE=$(api_call POST /mnemos/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI","libelle":"Mnemo AI mis à jour","unite":"°C","type":"AI"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /mnemos/set → HTTP 200"
else
    _test_fail "POST /mnemos/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

LIBELLE=$(db_domain_query \
    "SELECT libelle FROM mnemos_AI WHERE tech_id='TEST_DLS' AND acronyme='TEST_AI' LIMIT 1;")
_test_start
if [[ "${LIBELLE}" == "Mnemo AI mis à jour" ]]; then
    _test_pass "POST /mnemos/set: libellé mis à jour en BD"
else
    _test_fail "POST /mnemos/set: libellé non mis à jour en BD" "BD='${LIBELLE}'"
fi

api_call POST /mnemos/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI","libelle":"Analog Input de test","unite":"°C","type":"AI"}' >/dev/null

log_info "Test: POST /mnemos/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /mnemos/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI","libelle":"Tentative","unite":"°C","type":"AI"}')
assert_http_status 403 "POST /mnemos/set readonly → HTTP 403"

print_suite_summary "Suite 16 - Mapping et Mnémos"
[[ ${TESTS_FAILED} -eq 0 ]]
