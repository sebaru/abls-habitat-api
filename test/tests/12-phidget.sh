#!/usr/bin/env bash
# =============================================================================
# 12-phidget.sh - Catégorie dédiée aux endpoints Phidget
# =============================================================================
# Endpoints testés: GET /phidget/list?agent_tech_id=..., POST /phidget/set, POST /phidget/set/io
# L'ajout automatique des I/O détectées par l'agent est couvert par la suite 21 (/run/phidget/add/io).
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 12 - Endpoints Phidget"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# Endpoints Phidget
# =============================================================================

log_info "Test: GET /phidget/list?agent_tech_id=TEST_PHIDGET"
RESPONSE=$(api_call GET "/phidget/list?agent_tech_id=TEST_PHIDGET" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /phidget/list?agent_tech_id=TEST_PHIDGET → HTTP 200"

PHI_FOUND=$(echo "${RESPONSE}" | jq -r '.IO[] | select(.agent_tech_id == "TEST_PHIDGET") | .agent_tech_id' 2>/dev/null)
_test_start
if [[ "${PHI_FOUND}" == "TEST_PHIDGET" ]]; then
    _test_pass "GET /phidget/list?agent_tech_id=TEST_PHIDGET contient TEST_PHIDGET"
else
    _test_fail "GET /phidget/list?agent_tech_id=TEST_PHIDGET ne contient pas TEST_PHIDGET" "${RESPONSE}"
fi

PHIDGET_DB=$(db_domain_query "SELECT COUNT(*) FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET';")
PHIDGET_API=$(echo "${RESPONSE}" | jq '.IO | length' 2>/dev/null)
_test_start
if [[ "${PHIDGET_API}" == "${PHIDGET_DB}" ]]; then
    _test_pass "GET /phidget/list?agent_tech_id=TEST_PHIDGET nombre cohérent avec BD (${PHIDGET_DB})"
else
    _test_fail "GET /phidget/list?agent_tech_id=TEST_PHIDGET nombre incohérent" "API=${PHIDGET_API}, BD=${PHIDGET_DB}"
fi

log_info "Test: GET /phidget/list?agent_tech_id=TEST_PHIDGET - readonly (accès insuffisant)"
RESPONSE=$(api_call GET "/phidget/list?agent_tech_id=TEST_PHIDGET" "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /phidget/list?agent_tech_id=TEST_PHIDGET readonly → HTTP 403"

# POST /phidget/set
log_info "Test: POST /phidget/set - modification description"
RESPONSE=$(api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget modifié","hostname":"192.168.1.201","password":"","serial":12345}')

assert_http_status 200 "POST /phidget/set → HTTP 200"

PHI_DESC=$(db_domain_query "SELECT description FROM phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
_test_start
if [[ "${PHI_DESC}" == "Phidget modifié" ]]; then
    _test_pass "POST /phidget/set description mise à jour en BD"
else
    _test_fail "POST /phidget/set description non mise à jour en BD" "BD='${PHI_DESC}'"
fi

api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget de test","hostname":"192.168.1.201","password":"","serial":12345}' >/dev/null

log_info "Test: POST /phidget/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /phidget/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Tentative","hostname":"x","password":"","serial":12345}')
assert_http_status 403 "POST /phidget/set readonly → HTTP 403"

# POST /phidget/set/io
PHI_IO_ID=$(db_domain_query "SELECT phidget_io_id FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
log_info "Test: POST /phidget/set/io - modification libellé PHI_IO_01"
RESPONSE=$(api_call POST /phidget/set/io "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"phidget_io_id\":${PHI_IO_ID},\"capteur\":\"DIGITAL-INPUT\",\"libelle\":\"IO Phidget modifié\",\"unite\":\"bool\",\"intervalle\":5000,\"archivage\":36000}")

assert_http_status 200 "POST /phidget/set/io → HTTP 200"

PHI_IO_LIB=$(db_domain_query "SELECT libelle FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
PHI_IO_UNITE=$(db_domain_query "SELECT unite FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
_test_start
if [[ "${PHI_IO_LIB}" == "IO Phidget modifié" && "${PHI_IO_UNITE}" == "bool" ]]; then
    _test_pass "POST /phidget/set/io libellé et unité mis à jour en BD"
else
    _test_fail "POST /phidget/set/io libellé/unité non mis à jour en BD" "BD libelle='${PHI_IO_LIB}', unite='${PHI_IO_UNITE}'"
fi
db_domain_query "UPDATE phidget_IO SET classe='DI', port=0, capteur='', libelle='Entrée phidget test 01', unite='', intervalle=5000, archivage=36000 WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01';" >/dev/null 2>&1 || true

print_suite_summary "Suite 12 - Endpoints Phidget"
[[ ${TESTS_FAILED} -eq 0 ]]
