#!/usr/bin/env bash
# =============================================================================
# 18-messages.sh - Tests des endpoints Messages
# =============================================================================
# Endpoints testés: GET /message/list, POST /message/set
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 18 - Messages"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /message/list
# =============================================================================
log_info "Test: GET /message/list"
RESPONSE=$(api_call GET /message/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /message/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "msgs" "GET /message/list retourne des messages"

TEST_MSG_FOUND=$(echo "${RESPONSE}" | jq -r '.msgs[] | select(.tech_id == "TEST_DLS" and .acronyme == "TEST_MSG") | .acronyme' 2>/dev/null | head -1)
_test_start
if [[ "${TEST_MSG_FOUND}" == "TEST_MSG" ]]; then
    _test_pass "GET /message/list contient TEST_DLS/TEST_MSG"
else
    _test_fail "GET /message/list ne contient pas TEST_DLS/TEST_MSG" "${RESPONSE}"
fi

log_info "Test: GET /message/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /message/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /message/list readonly → HTTP 403"

# =============================================================================
# TEST: POST /message/set - Modifier le libellé d'un message
# =============================================================================
log_info "Test: POST /message/set - mise à jour libellé TEST_DLS/TEST_MSG"
RESPONSE=$(api_call POST /message/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG","libelle":"Message de test modifié","niveau":1}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /message/set → HTTP 200"
else
    _test_fail "POST /message/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

MSG_LIBELLE=$(db_domain_query \
    "SELECT libelle FROM msgs WHERE tech_id='TEST_DLS' AND acronyme='TEST_MSG' LIMIT 1;")
_test_start
if [[ "${MSG_LIBELLE}" == "Message de test modifié" ]]; then
    _test_pass "POST /message/set: libellé mis à jour en BD"
else
    _test_fail "POST /message/set: libellé non mis à jour en BD" "BD='${MSG_LIBELLE}'"
fi

api_call POST /message/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG","libelle":"Message de test","niveau":1}' >/dev/null

log_info "Test: POST /message/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /message/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG","libelle":"Tentative","niveau":1}')
assert_http_status 403 "POST /message/set readonly → HTTP 403"

print_suite_summary "Suite 18 - Messages"
[[ ${TESTS_FAILED} -eq 0 ]]
