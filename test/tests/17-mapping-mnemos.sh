#!/usr/bin/env bash
# =============================================================================
# 17-mapping-mnemos.sh - Tests des endpoints Mapping et Mnémos
# =============================================================================
# Endpoints testés:
#   Mapping : GET /mapping/list, POST /mapping/set, DELETE /mapping/delete
#   Mnémos  : GET /mnemos/tech_ids, GET /mnemos/validate,
#             GET /mnemos/list, POST /mnemos/set
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 17 - Mapping et Mnémos"

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
    '{"agent_tech_id":"TEST_MODBUS","agent_acronyme":"MOD_AI_01","tech_id":"TEST_DLS","acronyme":"TEST_AI"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /mapping/set → HTTP 200"
else
    _test_fail "POST /mapping/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

MAPPING_OK=$(db_domain_query \
    "SELECT COUNT(*) FROM mappings WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' AND tech_id='TEST_DLS' AND acronyme='TEST_AI';")
_test_start
if [[ "${MAPPING_OK}" -ge 1 ]]; then
    _test_pass "POST /mapping/set: mapping présent en BD"
else
    _test_fail "POST /mapping/set: mapping absent de la BD" "count=${MAPPING_OK}"
fi

log_info "Test: POST /mapping/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /mapping/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"TEST_MODBUS","agent_acronyme":"MOD_AI_01","tech_id":"TEST_DLS","acronyme":"TEST_AI"}')
assert_http_status 403 "POST /mapping/set readonly → HTTP 403"

# TEST: DELETE /mapping/delete - Supprimer puis recréer le mapping
log_info "Test: DELETE /mapping/delete - suppression du mapping TEST_MODBUS/MOD_AI_01"
MAPPING_ID=$(db_domain_query \
    "SELECT mapping_id FROM mappings WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' AND tech_id='TEST_DLS' AND acronyme='TEST_AI' LIMIT 1;")
if [[ -n "${MAPPING_ID}" ]]; then
    RESPONSE=$(api_call DELETE /mapping/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        '{"agent_tech_id":"TEST_MODBUS","agent_acronyme":"MOD_AI_01","tech_id":"TEST_DLS","acronyme":"TEST_AI"}')
else
    LAST_HTTP_CODE="000"
    _test_start
    _test_fail "DELETE /mapping/delete: mapping de test absent" "Impossible de récupérer mapping_id"
fi

if [[ -n "${MAPPING_ID}" ]]; then
    assert_http_status 200 "DELETE /mapping/delete → HTTP 200"
fi

MAPPING_OK=$(db_domain_query \
    "SELECT COUNT(*) FROM mappings WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' AND tech_id='TEST_DLS' AND acronyme='TEST_AI';")
_test_start
if [[ "${MAPPING_OK}" == "0" ]]; then
    _test_pass "DELETE /mapping/delete: mapping supprimé de la BD"
else
    _test_fail "DELETE /mapping/delete: mapping encore présent" "count=${MAPPING_OK}"
fi

log_info "Test: DELETE /mapping/delete - champ obligatoire manquant"
RESPONSE=$(api_call DELETE /mapping/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"TEST_MODBUS","agent_acronyme":"MOD_AI_01","tech_id":"TEST_DLS"}')
assert_http_status 400 "DELETE /mapping/delete sans acronyme → HTTP 400"

# Restaurer le mapping supprimé pour ne pas casser d'autres tests
db_domain_query "INSERT IGNORE INTO mappings (agent_tech_id, agent_acronyme, tech_id, acronyme) VALUES ('TEST_MODBUS','MOD_AI_01','TEST_DLS','TEST_AI');" >/dev/null 2>&1 || true

# =============================================================================
# MNÉMOS
# =============================================================================

# TEST: GET /mnemos/tech_ids
log_info "Test: GET /mnemos/tech_ids"
RESPONSE=$(api_call GET /mnemos/tech_ids "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /mnemos/tech_ids → HTTP 200"

TEST_DLS_FOUND=$(echo "${RESPONSE}" | jq -r '.tech_ids[]? | select(.tech_id == "TEST_DLS") | .tech_id' 2>/dev/null)
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
log_info "Test: GET /mnemos/validate?classe=AI&tech_id=TEST_DLS&acronyme=TEST_AI"
RESPONSE=$(api_call GET "/mnemos/validate?classe=AI&tech_id=TEST_DLS&acronyme=TEST_AI" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /mnemos/validate → HTTP 200"
else
    _test_fail "GET /mnemos/validate" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# TEST: GET /mnemos/list
db_domain_query "INSERT IGNORE INTO mnemos_REGISTRE (tech_id, acronyme, libelle) VALUES ('TEST_DLS','TEST_REG_TEST','Registre de test');" >/dev/null
log_info "Test: GET /mnemos/list?classe=R&tech_id=TEST_DLS"
RESPONSE=$(api_call GET "/mnemos/list?classe=R&tech_id=TEST_DLS" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /mnemos/list → HTTP 200"

TEST_REG_FOUND=$(echo "${RESPONSE}" | jq -r '.mnemos_REGISTRE[]? | select(.acronyme == "TEST_REG_TEST") | .acronyme' 2>/dev/null)
_test_start
if [[ "${TEST_REG_FOUND}" == "TEST_REG_TEST" ]]; then
    _test_pass "GET /mnemos/list contient TEST_REG_TEST"
else
    _test_fail "GET /mnemos/list ne contient pas TEST_REG_TEST" "${RESPONSE}"
fi

# TEST: POST /mnemos/set - Mettre à jour l'archivage d'un registre
REG_ARCHIVE_BEFORE=$(db_domain_query "SELECT archivage FROM mnemos_REGISTRE WHERE tech_id='TEST_DLS' AND acronyme='TEST_REG_TEST' LIMIT 1;")
log_info "Test: POST /mnemos/set - mise à jour archivage TEST_DLS/TEST_REG_TEST"
RESPONSE=$(api_call POST /mnemos/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"classe":"R","tech_id":"TEST_DLS","acronyme":"TEST_REG_TEST","archivage":1800}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /mnemos/set → HTTP 200"
else
    _test_fail "POST /mnemos/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

REG_ARCHIVE_AFTER=$(db_domain_query \
    "SELECT archivage FROM mnemos_REGISTRE WHERE tech_id='TEST_DLS' AND acronyme='TEST_REG_TEST' LIMIT 1;")
_test_start
if [[ "${REG_ARCHIVE_AFTER}" == "1800" ]]; then
    _test_pass "POST /mnemos/set: archivage mis à jour en BD"
else
    _test_fail "POST /mnemos/set: archivage non mis à jour en BD" "BD='${REG_ARCHIVE_AFTER}'"
fi

api_call POST /mnemos/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"classe\":\"R\",\"tech_id\":\"TEST_DLS\",\"acronyme\":\"TEST_REG_TEST\",\"archivage\":${REG_ARCHIVE_BEFORE}}" >/dev/null

log_info "Test: POST /mnemos/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /mnemos/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"classe":"R","tech_id":"TEST_DLS","acronyme":"TEST_REG_TEST","archivage":3600}')
assert_http_status 403 "POST /mnemos/set readonly → HTTP 403"

db_domain_query "DELETE FROM mnemos_REGISTRE WHERE tech_id='TEST_DLS' AND acronyme='TEST_REG_TEST';" >/dev/null

print_suite_summary "Suite 17 - Mapping et Mnémos"
[[ ${TESTS_FAILED} -eq 0 ]]
