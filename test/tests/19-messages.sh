#!/usr/bin/env bash
# =============================================================================
# 19-messages.sh - Tests des endpoints Messages
# =============================================================================
# Endpoints testés: GET /message/list, POST /message/set
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 19 - Messages"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# La compilation DLS (suite 05) purge les messages inutilisés: on recrée TEST_MSG.
db_domain_query "INSERT IGNORE INTO msgs (tech_id, acronyme, libelle) VALUES ('TEST_DLS', 'TEST_MSG', 'Message de test fonctionnel');" >/dev/null

# =============================================================================
# TEST: GET /message/list
# =============================================================================
log_info "Test: GET /message/list"
RESPONSE=$(api_call GET /message/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /message/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "messages" "GET /message/list retourne des messages"

TEST_MSG_FOUND=$(echo "${RESPONSE}" | jq -r '.messages[] | select(.tech_id == "TEST_DLS" and .acronyme == "TEST_MSG") | .acronyme' 2>/dev/null | head -1)
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
# TEST: POST /message/set - Modifier les paramètres de notification d'un message
# =============================================================================
log_info "Test: POST /message/set - mise à jour audio_libelle TEST_DLS/TEST_MSG"
RESPONSE=$(api_call POST /message/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG","audio_libelle":"Message audio modifié","audio_zone_name":"ZD_NONE","notif_sms":0,"notif_chat":1,"rate_limit":5}')

assert_http_status 200 "POST /message/set → HTTP 200"

MSG_LIBELLE=$(db_domain_query \
    "SELECT audio_libelle FROM msgs WHERE tech_id='TEST_DLS' AND acronyme='TEST_MSG' LIMIT 1;")
_test_start
if [[ "${MSG_LIBELLE}" == "Message audio modifié" ]]; then
    _test_pass "POST /message/set: audio_libelle mis à jour en BD"
else
    _test_fail "POST /message/set: audio_libelle non mis à jour en BD" "BD='${MSG_LIBELLE}'"
fi
assert_db_field "msgs" "rate_limit" "5" "POST /message/set: rate_limit mis à jour en BD" "tech_id='TEST_DLS' AND acronyme='TEST_MSG'"

api_call POST /message/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG","audio_libelle":"","audio_zone_name":"ZD_NONE","notif_sms":-1,"notif_chat":-1,"rate_limit":1}' >/dev/null

log_info "Test: POST /message/set - champ obligatoire manquant"
RESPONSE=$(api_call POST /message/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG"}')
assert_http_status 400 "POST /message/set sans rate_limit → HTTP 400"

log_info "Test: POST /message/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /message/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_MSG","audio_libelle":"Tentative","audio_zone_name":"ZD_NONE","notif_sms":0,"notif_chat":0,"rate_limit":1}')
assert_http_status 403 "POST /message/set readonly → HTTP 403"

print_suite_summary "Suite 19 - Messages"
[[ ${TESTS_FAILED} -eq 0 ]]
