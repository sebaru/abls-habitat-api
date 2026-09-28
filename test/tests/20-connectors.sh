#!/usr/bin/env bash
# =============================================================================
# 19-connectors.sh - Tests des endpoints Connecteurs
# =============================================================================
# Endpoints testés: POST /imsg/set, POST /sms/set, POST /shelly/set,
#                   POST /meteo/set, POST /ups/set, POST /teleinfoedf/set
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 19 - Connecteurs"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: POST /imsg/set - Configurer le connecteur XMPP
# =============================================================================
log_info "Test: POST /imsg/set - mise à jour connecteur XMPP TEST_IMSG"
RESPONSE=$(api_call POST /imsg/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_IMSG\",\"description\":\"XMPP modifié\",\"jabberid\":\"test@xmpp.test\",\"password\":\"testpass\"}")

assert_http_status 200 "POST /imsg/set → HTTP 200"

IMSG_DESC=$(db_domain_query \
    "SELECT description FROM imsg WHERE agent_tech_id='TEST_IMSG' LIMIT 1;")
_test_start
if [[ "${IMSG_DESC}" == "XMPP modifié" ]]; then
    _test_pass "POST /imsg/set: description mise à jour en BD"
else
    _test_fail "POST /imsg/set: description non mise à jour en BD" "BD='${IMSG_DESC}'"
fi

api_call POST /imsg/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_IMSG\",\"description\":\"XMPP de test\",\"jabberid\":\"test@xmpp.test\",\"password\":\"testpass\"}" >/dev/null

log_info "Test: POST /imsg/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /imsg/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_IMSG\",\"description\":\"Tentative\",\"jabberid\":\"test@xmpp.test\",\"password\":\"testpass\"}")
assert_http_status 403 "POST /imsg/set readonly → HTTP 403"

log_info "Test: GET /imsg/list"
RESPONSE=$(api_call GET /imsg/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /imsg/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "imsg" "GET /imsg/list retourne des agents XMPP"

_test_start
if echo "${RESPONSE}" | jq -e '.imsg[] | select(.agent_tech_id == "TEST_IMSG") | .jabberid == "test@xmpp.test" and has("server_hostname") and has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /imsg/list retourne la configuration de TEST_IMSG"
else
    _test_fail "GET /imsg/list ne retourne pas la configuration attendue" "${RESPONSE}"
fi

log_info "Test: GET /imsg/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /imsg/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /imsg/list readonly → HTTP 403"

log_info "Test: GET /imsg/get?agent_tech_id=TEST_IMSG"
RESPONSE=$(api_call GET "/imsg/get?agent_tech_id=TEST_IMSG" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /imsg/get → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_IMSG" "GET /imsg/get agent_tech_id correct"

log_info "Test: GET /imsg/get - paramètre manquant"
RESPONSE=$(api_call GET /imsg/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /imsg/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /imsg/get - agent_tech_id inconnu"
RESPONSE=$(api_call GET "/imsg/get?agent_tech_id=UNKNOWN_IMSG" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /imsg/get avec agent_tech_id inconnu → HTTP 404"

# =============================================================================
# TEST: POST /sms/set - Configurer le connecteur SMS (OVH)
# =============================================================================
log_info "Test: POST /sms/set - mise à jour connecteur SMS TEST_SMS"
RESPONSE=$(api_call POST /sms/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SMS\",\"description\":\"SMS modifié\",\"ovh_service_name\":\"svc-test\",\"ovh_application_key\":\"appkey\",\"ovh_application_secret\":\"appsecret\",\"ovh_consumer_key\":\"conskey\"}")

assert_http_status 200 "POST /sms/set → HTTP 200"

SMS_DESC=$(db_domain_query \
    "SELECT description FROM sms WHERE agent_tech_id='TEST_SMS' LIMIT 1;")
_test_start
if [[ "${SMS_DESC}" == "SMS modifié" ]]; then
    _test_pass "POST /sms/set: description mise à jour en BD"
else
    _test_fail "POST /sms/set: description non mise à jour en BD" "BD='${SMS_DESC}'"
fi

api_call POST /sms/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SMS\",\"description\":\"SMS de test\",\"ovh_service_name\":\"svc-test\",\"ovh_application_key\":\"appkey\",\"ovh_application_secret\":\"appsecret\",\"ovh_consumer_key\":\"conskey\"}" >/dev/null

log_info "Test: POST /sms/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /sms/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SMS\",\"description\":\"Tentative\",\"ovh_service_name\":\"svc-test\",\"ovh_application_key\":\"appkey\",\"ovh_application_secret\":\"appsecret\",\"ovh_consumer_key\":\"conskey\"}")
assert_http_status 403 "POST /sms/set readonly → HTTP 403"

log_info "Test: GET /sms/list"
RESPONSE=$(api_call GET /sms/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /sms/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "sms" "GET /sms/list retourne des agents SMS"

_test_start
if echo "${RESPONSE}" | jq -e '.sms[] | select(.agent_tech_id == "TEST_SMS") | .ovh_service_name == "svc-test" and has("server_hostname") and has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /sms/list retourne la configuration de TEST_SMS"
else
    _test_fail "GET /sms/list ne retourne pas la configuration attendue" "${RESPONSE}"
fi

log_info "Test: GET /sms/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /sms/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /sms/list readonly → HTTP 403"

log_info "Test: GET /sms/get?agent_tech_id=TEST_SMS"
RESPONSE=$(api_call GET "/sms/get?agent_tech_id=TEST_SMS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /sms/get → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_SMS" "GET /sms/get agent_tech_id correct"

log_info "Test: GET /sms/get - paramètre manquant"
RESPONSE=$(api_call GET /sms/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /sms/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /sms/get - agent_tech_id inconnu"
RESPONSE=$(api_call GET "/sms/get?agent_tech_id=UNKNOWN_SMS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /sms/get avec agent_tech_id inconnu → HTTP 404"

# =============================================================================
# TEST: POST /shelly/set - Configurer le connecteur Shelly
# =============================================================================
log_info "Test: POST /shelly/set - mise à jour connecteur Shelly TEST_SHELLY"
RESPONSE=$(api_call POST /shelly/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SHELLY\",\"description\":\"Shelly modifié\",\"hostname\":\"192.168.1.202\",\"string_id\":\"shellypro2-aabbccddeeff\"}")

assert_http_status 200 "POST /shelly/set → HTTP 200"

SHELLY_DESC=$(db_domain_query \
    "SELECT description FROM shelly WHERE agent_tech_id='TEST_SHELLY' LIMIT 1;")
_test_start
if [[ "${SHELLY_DESC}" == "Shelly modifié" ]]; then
    _test_pass "POST /shelly/set: description mise à jour en BD"
else
    _test_fail "POST /shelly/set: description non mise à jour en BD" "BD='${SHELLY_DESC}'"
fi

api_call POST /shelly/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SHELLY\",\"description\":\"Shelly de test\",\"hostname\":\"192.168.1.202\",\"string_id\":\"shellypro2-aabbccddeeff\"}" >/dev/null

log_info "Test: POST /shelly/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /shelly/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SHELLY\",\"description\":\"Tentative\",\"hostname\":\"192.168.1.202\",\"string_id\":\"shellypro2-aabbccddeeff\"}")
assert_http_status 403 "POST /shelly/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /meteo/set - Configurer le connecteur Météo
# =============================================================================
log_info "Test: POST /meteo/set - mise à jour connecteur Météo TEST_METEO"
RESPONSE=$(api_call POST /meteo/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_METEO\",\"description\":\"Météo modifiée\",\"code_insee\":\"75056\",\"token\":\"fake_meteo_token_001\"}")

assert_http_status 200 "POST /meteo/set → HTTP 200"

METEO_DESC=$(db_domain_query \
    "SELECT description FROM meteo WHERE agent_tech_id='TEST_METEO' LIMIT 1;")
_test_start
if [[ "${METEO_DESC}" == "Météo modifiée" ]]; then
    _test_pass "POST /meteo/set: description mise à jour en BD"
else
    _test_fail "POST /meteo/set: description non mise à jour en BD" "BD='${METEO_DESC}'"
fi

api_call POST /meteo/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_METEO\",\"description\":\"Météo de test\",\"code_insee\":\"75056\",\"token\":\"fake_meteo_token_001\"}" >/dev/null

log_info "Test: POST /meteo/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /meteo/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_METEO\",\"description\":\"Tentative\",\"code_insee\":\"75056\",\"token\":\"fake_meteo_token_001\"}")
assert_http_status 403 "POST /meteo/set readonly → HTTP 403"

log_info "Test: GET /meteo/list"
RESPONSE=$(api_call GET /meteo/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /meteo/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "meteo" "GET /meteo/list retourne des agents météo"

_test_start
if echo "${RESPONSE}" | jq -e '.meteo[] | select(.agent_tech_id == "TEST_METEO") |
    .code_insee == "75056" and .description == "Météo de test" and has("server_hostname") and has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /meteo/list retourne la configuration complète de TEST_METEO"
else
    _test_fail "GET /meteo/list ne retourne pas la configuration attendue pour TEST_METEO" "${RESPONSE}"
fi

log_info "Test: GET /meteo/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /meteo/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /meteo/list readonly → HTTP 403"

log_info "Test: GET /meteo/get?agent_tech_id=TEST_METEO"
RESPONSE=$(api_call GET "/meteo/get?agent_tech_id=TEST_METEO" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /meteo/get → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_METEO" "GET /meteo/get agent_tech_id correct"

_test_start
if echo "${RESPONSE}" | jq -e 'has("IO")' >/dev/null 2>&1; then
    _test_pass "GET /meteo/get retourne les mnémoniques"
else
    _test_fail "GET /meteo/get ne retourne pas les mnémoniques" "${RESPONSE}"
fi

log_info "Test: GET /meteo/get - paramètre manquant"
RESPONSE=$(api_call GET "/meteo/get" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /meteo/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /meteo/get - agent_tech_id inconnu"
RESPONSE=$(api_call GET "/meteo/get?agent_tech_id=UNKNOWN_METEO" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /meteo/get avec agent_tech_id inconnu → HTTP 404"

# =============================================================================
# TEST: POST /ups/set - Configurer le connecteur UPS
# =============================================================================
log_info "Test: POST /ups/set - mise à jour connecteur UPS TEST_UPS"
RESPONSE=$(api_call POST /ups/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_UPS\",\"description\":\"UPS de test\",\"host\":\"192.168.1.203\",\"name\":\"UPS-TEST\",\"admin_username\":\"admin\",\"admin_password\":\"upspass\"}")

assert_http_status 200 "POST /ups/set → HTTP 200"

UPS_HOST=$(db_domain_query \
    "SELECT host FROM ups WHERE agent_tech_id='TEST_UPS' LIMIT 1;")
_test_start
if [[ "${UPS_HOST}" == "192.168.1.203" ]]; then
    _test_pass "POST /ups/set: host correct en BD"
else
    _test_fail "POST /ups/set: host incorrect en BD" "BD='${UPS_HOST}'"
fi

log_info "Test: POST /ups/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /ups/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_UPS\",\"description\":\"Tentative\",\"host\":\"192.168.1.203\",\"name\":\"UPS-TEST\",\"admin_username\":\"admin\",\"admin_password\":\"upspass\"}")
assert_http_status 403 "POST /ups/set readonly → HTTP 403"

log_info "Test: GET /ups/list"
RESPONSE=$(api_call GET /ups/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /ups/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "ups" "GET /ups/list retourne des agents onduleur"

_test_start
if echo "${RESPONSE}" | jq -e '.ups[] | select(.agent_tech_id == "TEST_UPS") |
    .host == "192.168.1.203" and .name == "UPS-TEST" and has("server_hostname") and has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /ups/list retourne la configuration complète de TEST_UPS"
else
    _test_fail "GET /ups/list ne retourne pas la configuration attendue pour TEST_UPS" "${RESPONSE}"
fi

log_info "Test: GET /ups/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /ups/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /ups/list readonly → HTTP 403"

log_info "Test: GET /ups/get?agent_tech_id=TEST_UPS"
RESPONSE=$(api_call GET "/ups/get?agent_tech_id=TEST_UPS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /ups/get → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_UPS" "GET /ups/get agent_tech_id correct"

_test_start
if echo "${RESPONSE}" | jq -e '(.DI | type == "array") and (.DO | type == "array") and (.AI | type == "array") and (.AO | type == "array")' >/dev/null 2>&1; then
    _test_pass "GET /ups/get retourne les mnémoniques DI/DO/AI/AO"
else
    _test_fail "GET /ups/get ne retourne pas les mnémoniques DI/DO/AI/AO" "${RESPONSE}"
fi

log_info "Test: GET /ups/get - paramètre manquant"
RESPONSE=$(api_call GET "/ups/get" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /ups/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /ups/get - agent_tech_id inconnu"
RESPONSE=$(api_call GET "/ups/get?agent_tech_id=UNKNOWN_UPS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /ups/get avec agent_tech_id inconnu → HTTP 404"

# =============================================================================
# TEST: POST /teleinfoedf/set - Configurer le connecteur Téléinfo EDF
# =============================================================================
log_info "Test: POST /teleinfoedf/set - mise à jour connecteur TEST_TELEINFO"
RESPONSE=$(api_call POST /teleinfoedf/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_TELEINFO\",\"description\":\"Téléinfo modifiée\",\"port\":\"/dev/ttyUSB0\",\"standard\":1}")

assert_http_status 200 "POST /teleinfoedf/set → HTTP 200"

TELEINFO_DESC=$(db_domain_query \
    "SELECT description FROM teleinfoedf WHERE agent_tech_id='TEST_TELEINFO' LIMIT 1;")
_test_start
if [[ "${TELEINFO_DESC}" == "Téléinfo modifiée" ]]; then
    _test_pass "POST /teleinfoedf/set: description mise à jour en BD"
else
    _test_fail "POST /teleinfoedf/set: description non mise à jour en BD" "BD='${TELEINFO_DESC}'"
fi

api_call POST /teleinfoedf/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_TELEINFO\",\"description\":\"Téléinfo EDF de test\",\"port\":\"/dev/ttyUSB0\",\"standard\":0}" >/dev/null

log_info "Test: POST /teleinfoedf/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /teleinfoedf/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_TELEINFO\",\"description\":\"Tentative\",\"port\":\"/dev/ttyUSB0\",\"standard\":0}")
assert_http_status 403 "POST /teleinfoedf/set readonly → HTTP 403"

print_suite_summary "Suite 19 - Connecteurs"
[[ ${TESTS_FAILED} -eq 0 ]]
