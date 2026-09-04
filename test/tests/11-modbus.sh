#!/usr/bin/env bash
# =============================================================================
# 11-modbus.sh - Tests des endpoints Modbus
# =============================================================================
# Endpoints testés: GET /modbus/list,
#                   POST /modbus/set, POST /modbus/set/di, POST /modbus/set/do,
#                   POST /modbus/set/ai, POST /modbus/set/ao
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 11 - Modbus"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /modbus/list
# =============================================================================
log_info "Test: GET /modbus/list"
RESPONSE=$(api_call GET /modbus/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /modbus/list → HTTP 200"

MOD_FOUND=$(echo "${RESPONSE}" | jq -r '.modbus[] | select(.agent_tech_id == "TEST_MODBUS") | .agent_tech_id' 2>/dev/null)
_test_start
if [[ "${MOD_FOUND}" == "TEST_MODBUS" ]]; then
    _test_pass "GET /modbus/list contient TEST_MODBUS"
else
    _test_fail "GET /modbus/list ne contient pas TEST_MODBUS" "${RESPONSE}"
fi

# Cohérence BD
MODBUS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM modbus;")
MODBUS_IN_API=$(echo "${RESPONSE}" | jq '.modbus | length' 2>/dev/null)
_test_start
if [[ "${MODBUS_IN_API}" == "${MODBUS_IN_DB}" ]]; then
    _test_pass "GET /modbus/list nombre cohérent avec BD (${MODBUS_IN_DB})"
else
    _test_fail "GET /modbus/list nombre incohérent" "API=${MODBUS_IN_API}, BD=${MODBUS_IN_DB}"
fi

log_info "Test: GET /modbus/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /modbus/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /modbus/list readonly → HTTP 403"

# =============================================================================
# TEST: POST /modbus/set - Modifier la description du thread
# =============================================================================
log_info "Test: POST /modbus/set - modification description"
RESPONSE=$(api_call POST /modbus/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_MODBUS","description":"Automate de test modifié","hostname":"192.168.1.200","watchdog":50,"max_request_par_sec":50}')

assert_http_status 200 "POST /modbus/set → HTTP 200"

DESC_DB=$(db_domain_query "SELECT description FROM modbus WHERE agent_tech_id='TEST_MODBUS' LIMIT 1;")
_test_start
if [[ "${DESC_DB}" == "Automate de test modifié" ]]; then
    _test_pass "POST /modbus/set description mise à jour en BD"
else
    _test_fail "POST /modbus/set description non mise à jour en BD" "BD='${DESC_DB}'"
fi

# Restaurer
api_call POST /modbus/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_MODBUS","description":"Automate de test","hostname":"192.168.1.200","watchdog":50,"max_request_par_sec":50}' >/dev/null

log_info "Test: POST /modbus/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /modbus/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_MODBUS","description":"Tentative","hostname":"x","watchdog":50,"max_request_par_sec":50}')
assert_http_status 403 "POST /modbus/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /modbus/set/di - Modifier une entrée digitale
# =============================================================================
log_info "Test: POST /modbus/set/di - modification libellé MOD_DI_01"
MODBUS_DI_ID=$(db_domain_query "SELECT modbus_di_id FROM modbus_DI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DI_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/di "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_di_id\":${MODBUS_DI_ID},\"libelle\":\"DI Test modifiée\",\"borne\":\"I1\",\"ed\":\"DI\",\"flip\":false,\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/di → HTTP 200"

DI_LIBELLE=$(db_domain_query "SELECT libelle FROM modbus_DI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DI_01' LIMIT 1;")
_test_start
if [[ "${DI_LIBELLE}" == "DI Test modifiée" ]]; then
    _test_pass "POST /modbus/set/di libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/di libellé non mis à jour en BD" "BD='${DI_LIBELLE}'"
fi
# Restaurer
db_domain_query "UPDATE modbus_DI SET libelle='Entrée digitale test 01' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DI_01';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /modbus/set/do - Modifier une sortie digitale
# =============================================================================
log_info "Test: POST /modbus/set/do - modification libellé MOD_DO_01"
MODBUS_DO_ID=$(db_domain_query "SELECT modbus_do_id FROM modbus_DO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DO_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/do "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_do_id\":${MODBUS_DO_ID},\"libelle\":\"DO Test modifiée\",\"borne\":\"Q1\",\"ed\":\"DO\",\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/do → HTTP 200"

DO_LIBELLE=$(db_domain_query "SELECT libelle FROM modbus_DO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DO_01' LIMIT 1;")
_test_start
if [[ "${DO_LIBELLE}" == "DO Test modifiée" ]]; then
    _test_pass "POST /modbus/set/do libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/do libellé non mis à jour en BD" "BD='${DO_LIBELLE}'"
fi
db_domain_query "UPDATE modbus_DO SET libelle='Sortie digitale test 01' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DO_01';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /modbus/set/ai - Modifier une entrée analogique
# =============================================================================
log_info "Test: POST /modbus/set/ai - modification libellé MOD_AI_01"
MODBUS_AI_ID=$(db_domain_query "SELECT modbus_ai_id FROM modbus_AI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/ai "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_ai_id\":${MODBUS_AI_ID},\"libelle\":\"AI Test modifiée\",\"type_borne\":0,\"min\":0,\"max\":100,\"borne\":\"IW1\",\"ed\":\"AI\",\"unite\":\"%\",\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/ai → HTTP 200"

AI_LIBELLE=$(db_domain_query "SELECT libelle FROM modbus_AI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' LIMIT 1;")
_test_start
if [[ "${AI_LIBELLE}" == "AI Test modifiée" ]]; then
    _test_pass "POST /modbus/set/ai libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/ai libellé non mis à jour en BD" "BD='${AI_LIBELLE}'"
fi
db_domain_query "UPDATE modbus_AI SET libelle='Entrée analogique test 01' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /modbus/set/ao - Modifier une sortie analogique
# =============================================================================
log_info "Test: POST /modbus/set/ao - modification libellé MOD_AO_01"
MODBUS_AO_ID=$(db_domain_query "SELECT modbus_ao_id FROM modbus_AO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/ao "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_ao_id\":${MODBUS_AO_ID},\"libelle\":\"AO Test modifiée\",\"type_borne\":0,\"min\":0,\"max\":100,\"borne\":\"QW1\",\"ed\":\"AO\",\"unite\":\"%\",\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/ao → HTTP 200"

AO_LIBELLE=$(db_domain_query "SELECT libelle FROM modbus_AO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01' LIMIT 1;")
_test_start
if [[ "${AO_LIBELLE}" == "AO Test modifiée" ]]; then
    _test_pass "POST /modbus/set/ao libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/ao libellé non mis à jour en BD" "BD='${AO_LIBELLE}'"
fi
db_domain_query "UPDATE modbus_AO SET libelle='Sortie analogique test 01' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01';" >/dev/null 2>&1 || true

print_suite_summary "Suite 11 - Modbus"
[[ ${TESTS_FAILED} -eq 0 ]]
