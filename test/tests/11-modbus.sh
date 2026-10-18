#!/usr/bin/env bash
# =============================================================================
# 11-modbus.sh - Tests des endpoints Modbus
# =============================================================================
# Endpoints testés: GET /modbus/list, GET /modbus/get,
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
MODBUS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM agent_modbus;")
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

log_info "Test: GET /modbus/get?agent_tech_id=TEST_MODBUS"
RESPONSE=$(api_call GET "/modbus/get?agent_tech_id=TEST_MODBUS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /modbus/get → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_MODBUS" "GET /modbus/get retourne TEST_MODBUS"

for IO_SPEC in 'DI ed' 'DO sd' 'AI ea' 'AO sa'; do
    read -r IO_TYPE IO_FIELD <<< "${IO_SPEC}"
    assert_db_field "agent_modbus_${IO_TYPE}" "${IO_FIELD}" "${IO_TYPE}" \
        "Migration Modbus ${IO_TYPE} conserve ${IO_FIELD}" \
        "agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_${IO_TYPE}_01'"
    IO_JSON=$(echo "${RESPONSE}" | jq --arg type "${IO_TYPE}" --arg acronyme "MOD_${IO_TYPE}_01" '.[$type][] | select(.agent_acronyme == $acronyme)')
    assert_json_field "${IO_JSON}" "${IO_FIELD}" \
        "${IO_TYPE}" "GET Modbus ${IO_TYPE} expose ${IO_FIELD}"
    IO_COLUMNS=$(db_domain_query "SELECT GROUP_CONCAT(COLUMN_NAME ORDER BY COLUMN_NAME) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='agent_modbus_${IO_TYPE}' AND COLUMN_NAME IN ('ed', 'sd', 'ea', 'sa');")
    _test_start
    if [[ "${IO_COLUMNS}" == "${IO_FIELD}" ]]; then
        _test_pass "Schema Modbus ${IO_TYPE}: ${IO_FIELD} uniquement"
    else
        _test_fail "Schema Modbus ${IO_TYPE} incorrect" "colonnes=${IO_COLUMNS}"
    fi
done

# =============================================================================
# TEST: POST /modbus/set - Modifier la description du thread
# =============================================================================
log_info "Test: POST /modbus/set - modification description"
RESPONSE=$(api_call POST /modbus/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_MODBUS","description":"Automate de test modifié","hostname":"192.168.1.200","watchdog":50}')

assert_http_status 200 "POST /modbus/set → HTTP 200"

DESC_DB=$(db_domain_query "SELECT description FROM agent_modbus WHERE agent_tech_id='TEST_MODBUS' LIMIT 1;")
_test_start
if [[ "${DESC_DB}" == "Automate de test modifié" ]]; then
    _test_pass "POST /modbus/set description mise à jour en BD"
else
    _test_fail "POST /modbus/set description non mise à jour en BD" "BD='${DESC_DB}'"
fi

# Restaurer
api_call POST /modbus/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_MODBUS","description":"Automate de test","hostname":"192.168.1.200","watchdog":50}' >/dev/null

log_info "Test: POST /modbus/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /modbus/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_MODBUS","description":"Tentative","hostname":"x","watchdog":50}')
assert_http_status 403 "POST /modbus/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /modbus/set/di - Modifier une entrée digitale
# =============================================================================
log_info "Test: POST /modbus/set/di - modification libellé MOD_DI_01"
MODBUS_DI_ID=$(db_domain_query "SELECT modbus_di_id FROM agent_modbus_DI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DI_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/di "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_di_id\":${MODBUS_DI_ID},\"description\":\"DI Test modifiée\",\"borne\":\"I1\",\"ed\":\"DI_UPDATED\",\"flip\":false,\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/di → HTTP 200"
assert_db_field "agent_modbus_DI" "ed" "DI_UPDATED" "POST DI persiste ed" "modbus_di_id=${MODBUS_DI_ID}"
RESPONSE=$(api_call GET "/modbus/get?agent_tech_id=TEST_MODBUS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_json_field "$(echo "${RESPONSE}" | jq '.DI[] | select(.agent_acronyme == "MOD_DI_01")')" "ed" "DI_UPDATED" "GET DI relit ed"

DI_LIBELLE=$(db_domain_query "SELECT description FROM agent_modbus_DI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DI_01' LIMIT 1;")
_test_start
if [[ "${DI_LIBELLE}" == "DI Test modifiée" ]]; then
    _test_pass "POST /modbus/set/di libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/di libellé non mis à jour en BD" "BD='${DI_LIBELLE}'"
fi
# Restaurer
db_domain_query "UPDATE agent_modbus_DI SET description='Entrée digitale test 01', ed='DI' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DI_01';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /modbus/set/do - Modifier une sortie digitale
# =============================================================================
log_info "Test: POST /modbus/set/do - modification libellé MOD_DO_01"
MODBUS_DO_ID=$(db_domain_query "SELECT modbus_do_id FROM agent_modbus_DO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DO_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/do "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_do_id\":${MODBUS_DO_ID},\"description\":\"DO Test modifiée\",\"borne\":\"Q1\",\"sd\":\"DO_UPDATED\",\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/do → HTTP 200"
assert_db_field "agent_modbus_DO" "sd" "DO_UPDATED" "POST DO persiste sd" "modbus_do_id=${MODBUS_DO_ID}"
RESPONSE=$(api_call GET "/modbus/get?agent_tech_id=TEST_MODBUS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_json_field "$(echo "${RESPONSE}" | jq '.DO[] | select(.agent_acronyme == "MOD_DO_01")')" "sd" "DO_UPDATED" "GET DO relit sd"

DO_LIBELLE=$(db_domain_query "SELECT description FROM agent_modbus_DO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DO_01' LIMIT 1;")
_test_start
if [[ "${DO_LIBELLE}" == "DO Test modifiée" ]]; then
    _test_pass "POST /modbus/set/do libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/do libellé non mis à jour en BD" "BD='${DO_LIBELLE}'"
fi
db_domain_query "UPDATE agent_modbus_DO SET description='Sortie digitale test 01', sd='DO' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_DO_01';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /modbus/set/ai - Modifier une entrée analogique
# =============================================================================
log_info "Test: POST /modbus/set/ai - modification libellé MOD_AI_01"
MODBUS_AI_ID=$(db_domain_query "SELECT modbus_ai_id FROM agent_modbus_AI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/ai "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_ai_id\":${MODBUS_AI_ID},\"description\":\"AI Test modifiée\",\"type_borne\":0,\"min\":1.25,\"max\":87.5,\"borne\":\"IW1\",\"ea\":\"AI_UPDATED\",\"unite\":\"%\",\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/ai → HTTP 200"
assert_db_field "agent_modbus_AI" "ea" "AI_UPDATED" "POST AI persiste ea" "modbus_ai_id=${MODBUS_AI_ID}"
RESPONSE=$(api_call GET "/modbus/get?agent_tech_id=TEST_MODBUS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_json_field "$(echo "${RESPONSE}" | jq '.AI[] | select(.agent_acronyme == "MOD_AI_01")')" "ea" "AI_UPDATED" "GET AI relit ea"

AI_LIBELLE=$(db_domain_query "SELECT description FROM agent_modbus_AI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' LIMIT 1;")
_test_start
if [[ "${AI_LIBELLE}" == "AI Test modifiée" ]]; then
    _test_pass "POST /modbus/set/ai libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/ai libellé non mis à jour en BD" "BD='${AI_LIBELLE}'"
fi

AI_BOUNDS=$(db_domain_query "SELECT IF(ABS(min - 1.25) < 0.00001 AND ABS(max - 87.5) < 0.00001, 'ok', 'ko') FROM agent_modbus_AI WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01' LIMIT 1;")
_test_start
if [[ "${AI_BOUNDS}" == "ok" ]]; then
    _test_pass "POST /modbus/set/ai conserve les bornes décimales"
else
    _test_fail "POST /modbus/set/ai tronque les bornes décimales" "bornes='${AI_BOUNDS}'"
fi
db_domain_query "UPDATE agent_modbus_AI SET description='Entrée analogique test 01', min=0, max=100, ea='AI' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AI_01';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /modbus/set/ao - Modifier une sortie analogique
# =============================================================================
log_info "Test: POST /modbus/set/ao - modification libellé MOD_AO_01"
MODBUS_AO_ID=$(db_domain_query "SELECT modbus_ao_id FROM agent_modbus_AO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01' LIMIT 1;")
RESPONSE=$(api_call POST /modbus/set/ao "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"modbus_ao_id\":${MODBUS_AO_ID},\"description\":\"AO Test modifiée\",\"type_borne\":0,\"min\":-12.75,\"max\":98.125,\"borne\":\"QW1\",\"sa\":\"AO_UPDATED\",\"unite\":\"%\",\"archivage\":36000}")

assert_http_status 200 "POST /modbus/set/ao → HTTP 200"
assert_db_field "agent_modbus_AO" "sa" "AO_UPDATED" "POST AO persiste sa" "modbus_ao_id=${MODBUS_AO_ID}"
RESPONSE=$(api_call GET "/modbus/get?agent_tech_id=TEST_MODBUS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_json_field "$(echo "${RESPONSE}" | jq '.AO[] | select(.agent_acronyme == "MOD_AO_01")')" "sa" "AO_UPDATED" "GET AO relit sa"

AO_LIBELLE=$(db_domain_query "SELECT description FROM agent_modbus_AO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01' LIMIT 1;")
_test_start
if [[ "${AO_LIBELLE}" == "AO Test modifiée" ]]; then
    _test_pass "POST /modbus/set/ao libellé mis à jour en BD"
else
    _test_fail "POST /modbus/set/ao libellé non mis à jour en BD" "BD='${AO_LIBELLE}'"
fi

AO_BOUNDS=$(db_domain_query "SELECT IF(ABS(min - (-12.75)) < 0.00001 AND ABS(max - 98.125) < 0.00001, 'ok', 'ko') FROM agent_modbus_AO WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01' LIMIT 1;")
_test_start
if [[ "${AO_BOUNDS}" == "ok" ]]; then
    _test_pass "POST /modbus/set/ao conserve les bornes décimales"
else
    _test_fail "POST /modbus/set/ao tronque les bornes décimales" "bornes='${AO_BOUNDS}'"
fi
db_domain_query "UPDATE agent_modbus_AO SET description='Sortie analogique test 01', min=0, max=100, sa='AO' WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_AO_01';" >/dev/null 2>&1 || true

for IO_TYPE in DO AI AO; do
    IO_ID=$(db_domain_query "SELECT modbus_${IO_TYPE,,}_id FROM agent_modbus_${IO_TYPE} WHERE agent_tech_id='TEST_MODBUS' AND agent_acronyme='MOD_${IO_TYPE}_01' LIMIT 1;")
    PAYLOAD=$(jq -nc --arg id_key "modbus_${IO_TYPE,,}_id" --argjson id "${IO_ID}" \
        '{($id_key): $id, description: "Legacy", borne: "X", ed: "OLD", archivage: 36000, min: 0, max: 100, type_borne: 0, unite: "%"}')
    RESPONSE=$(api_call POST "/modbus/set/${IO_TYPE,,}" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${PAYLOAD}")
    assert_http_status 400 "POST ${IO_TYPE} refuse ancien champ ed seul"
done

print_suite_summary "Suite 11 - Modbus"
[[ ${TESTS_FAILED} -eq 0 ]]
