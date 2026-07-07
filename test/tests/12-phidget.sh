#!/usr/bin/env bash
# =============================================================================
# 12-phidget.sh - Tests des endpoints Phidget
# =============================================================================
# Endpoints testés: GET /phidget/list, POST /phidget/set, POST /phidget/set/io
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 12 - Phidget"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# Phidget
# =============================================================================

log_info "Test: GET /phidget/list"
RESPONSE=$(api_call GET /phidget/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /phidget/list → HTTP 200"

PHI_FOUND=$(echo "${RESPONSE}" | jq -r '.phidget[] | select(.thread_tech_id == "TEST_PHIDGET" or .agent_tech_id == "TEST_PHIDGET") | (.thread_tech_id // .agent_tech_id)' 2>/dev/null)
_test_start
if [[ "${PHI_FOUND}" == "TEST_PHIDGET" ]]; then
    _test_pass "GET /phidget/list contient TEST_PHIDGET"
else
    _test_fail "GET /phidget/list ne contient pas TEST_PHIDGET" "${RESPONSE}"
fi

PHIDGET_DB=$(db_domain_query "SELECT COUNT(*) FROM phidget;")
PHIDGET_API=$(echo "${RESPONSE}" | jq '.phidget | length' 2>/dev/null)
_test_start
if [[ "${PHIDGET_API}" == "${PHIDGET_DB}" ]]; then
    _test_pass "GET /phidget/list nombre cohérent avec BD (${PHIDGET_DB})"
else
    _test_fail "GET /phidget/list nombre incohérent" "API=${PHIDGET_API}, BD=${PHIDGET_DB}"
fi

log_info "Test: GET /phidget/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /phidget/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /phidget/list readonly → HTTP 403"

# POST /phidget/set
log_info "Test: POST /phidget/set - modification description"
RESPONSE=$(api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_PHIDGET","description":"Phidget modifié","hostname":"192.168.1.201","password":"","serial":12345}')

assert_http_status 200 "POST /phidget/set → HTTP 200"

PHI_DESC=$(db_domain_query "SELECT description FROM phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
_test_start
if [[ "${PHI_DESC}" == "Phidget modifié" ]]; then
    _test_pass "POST /phidget/set description mise à jour en BD"
else
    _test_fail "POST /phidget/set description non mise à jour en BD" "BD='${PHI_DESC}'"
fi

api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_PHIDGET","description":"Phidget de test","hostname":"192.168.1.201","password":"","serial":12345}' >/dev/null

log_info "Test: POST /phidget/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /phidget/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_PHIDGET","description":"Tentative","hostname":"x","password":"","serial":12345}')
assert_http_status 403 "POST /phidget/set readonly → HTTP 403"

# POST /phidget/set/io
log_info "Test: POST /phidget/set/io - modification libellé PHI_IO_01"
RESPONSE=$(api_call POST /phidget/set/io "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_PHIDGET","thread_acronyme":"PHI_IO_01","classe":"DI","port":0,"capteur":"","libelle":"IO Phidget modifié","intervalle":5000,"archivage":36000}')

assert_http_status 200 "POST /phidget/set/io → HTTP 200"

PHI_IO_LIB=$(db_domain_query "SELECT libelle FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
_test_start
if [[ "${PHI_IO_LIB}" == "IO Phidget modifié" ]]; then
    _test_pass "POST /phidget/set/io libellé mis à jour en BD"
else
    _test_fail "POST /phidget/set/io libellé non mis à jour en BD" "BD='${PHI_IO_LIB}'"
fi
db_domain_query "UPDATE phidget_IO SET libelle='Entrée phidget test 01' WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01';" >/dev/null 2>&1 || true

print_suite_summary "Suite 12 - Phidget"
[[ ${TESTS_FAILED} -eq 0 ]]
