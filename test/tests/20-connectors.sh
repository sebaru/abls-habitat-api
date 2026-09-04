#!/usr/bin/env bash
# =============================================================================
# 19-connectors.sh - Tests des endpoints Connecteurs
# =============================================================================
# Endpoints testés: POST /imsgs/set, POST /smsg/set, POST /shelly/set,
#                   POST /meteo/set, POST /ups/set, POST /teleinfoedf/set
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 19 - Connecteurs"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: POST /imsgs/set - Configurer le connecteur XMPP
# =============================================================================
log_info "Test: POST /imsgs/set - mise à jour connecteur XMPP TEST_IMSGS"
RESPONSE=$(api_call POST /imsgs/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"thread_tech_id\":\"TEST_IMSGS\",\"description\":\"XMPP modifié\",\"jabberid\":\"test@xmpp.test\",\"password\":\"testpass\"}")

assert_http_status 200 "POST /imsgs/set → HTTP 200"

IMSGS_DESC=$(db_domain_query \
    "SELECT description FROM imsgs WHERE thread_tech_id='TEST_IMSGS' LIMIT 1;")
_test_start
if [[ "${IMSGS_DESC}" == "XMPP modifié" ]]; then
    _test_pass "POST /imsgs/set: description mise à jour en BD"
else
    _test_fail "POST /imsgs/set: description non mise à jour en BD" "BD='${IMSGS_DESC}'"
fi

api_call POST /imsgs/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"thread_tech_id\":\"TEST_IMSGS\",\"description\":\"XMPP de test\",\"jabberid\":\"test@xmpp.test\",\"password\":\"testpass\"}" >/dev/null

log_info "Test: POST /imsgs/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /imsgs/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"thread_tech_id\":\"TEST_IMSGS\",\"description\":\"Tentative\",\"jabberid\":\"test@xmpp.test\",\"password\":\"testpass\"}")
assert_http_status 403 "POST /imsgs/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /smsg/set - Configurer le connecteur SMS (OVH)
# =============================================================================
log_info "Test: POST /smsg/set - mise à jour connecteur SMS TEST_SMSG"
RESPONSE=$(api_call POST /smsg/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SMSG\",\"description\":\"SMS modifié\",\"ovh_service_name\":\"svc-test\",\"ovh_application_key\":\"appkey\",\"ovh_application_secret\":\"appsecret\",\"ovh_consumer_key\":\"conskey\"}")

assert_http_status 200 "POST /smsg/set → HTTP 200"

SMSG_DESC=$(db_domain_query \
    "SELECT description FROM smsg WHERE agent_tech_id='TEST_SMSG' LIMIT 1;")
_test_start
if [[ "${SMSG_DESC}" == "SMS modifié" ]]; then
    _test_pass "POST /smsg/set: description mise à jour en BD"
else
    _test_fail "POST /smsg/set: description non mise à jour en BD" "BD='${SMSG_DESC}'"
fi

api_call POST /smsg/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SMSG\",\"description\":\"SMS de test\",\"ovh_service_name\":\"svc-test\",\"ovh_application_key\":\"appkey\",\"ovh_application_secret\":\"appsecret\",\"ovh_consumer_key\":\"conskey\"}" >/dev/null

log_info "Test: POST /smsg/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /smsg/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"agent_tech_id\":\"TEST_SMSG\",\"description\":\"Tentative\",\"ovh_service_name\":\"svc-test\",\"ovh_application_key\":\"appkey\",\"ovh_application_secret\":\"appsecret\",\"ovh_consumer_key\":\"conskey\"}")
assert_http_status 403 "POST /smsg/set readonly → HTTP 403"

log_info "Test: GET /smsg/list"
RESPONSE=$(api_call GET /smsg/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /smsg/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "smsg" "GET /smsg/list retourne des agents SMS"

_test_start
if echo "${RESPONSE}" | jq -e '.smsg[] | select(.agent_tech_id == "TEST_SMSG") | .ovh_service_name == "svc-test" and has("server_hostname") and has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /smsg/list retourne la configuration de TEST_SMSG"
else
    _test_fail "GET /smsg/list ne retourne pas la configuration attendue" "${RESPONSE}"
fi

log_info "Test: GET /smsg/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /smsg/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /smsg/list readonly → HTTP 403"

log_info "Test: GET /smsg/get?agent_tech_id=TEST_SMSG"
RESPONSE=$(api_call GET "/smsg/get?agent_tech_id=TEST_SMSG" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /smsg/get → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_SMSG" "GET /smsg/get agent_tech_id correct"

log_info "Test: GET /smsg/get - paramètre manquant"
RESPONSE=$(api_call GET /smsg/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /smsg/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /smsg/get - agent_tech_id inconnu"
RESPONSE=$(api_call GET "/smsg/get?agent_tech_id=UNKNOWN_SMSG" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /smsg/get avec agent_tech_id inconnu → HTTP 404"

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
if echo "${RESPONSE}" | jq -e 'has("IO")' >/dev/null 2>&1; then
    _test_pass "GET /ups/get retourne les mnémoniques"
else
    _test_fail "GET /ups/get ne retourne pas les mnémoniques" "${RESPONSE}"
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
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"thread_tech_id\":\"TEST_TELEINFO\",\"description\":\"Téléinfo modifiée\",\"port\":\"/dev/ttyUSB0\",\"standard\":1}")

assert_http_status 200 "POST /teleinfoedf/set → HTTP 200"

TELEINFO_DESC=$(db_domain_query \
    "SELECT description FROM teleinfoedf WHERE thread_tech_id='TEST_TELEINFO' LIMIT 1;")
_test_start
if [[ "${TELEINFO_DESC}" == "Téléinfo modifiée" ]]; then
    _test_pass "POST /teleinfoedf/set: description mise à jour en BD"
else
    _test_fail "POST /teleinfoedf/set: description non mise à jour en BD" "BD='${TELEINFO_DESC}'"
fi

api_call POST /teleinfoedf/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"thread_tech_id\":\"TEST_TELEINFO\",\"description\":\"Téléinfo EDF de test\",\"port\":\"/dev/ttyUSB0\",\"standard\":0}" >/dev/null

log_info "Test: POST /teleinfoedf/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /teleinfoedf/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"thread_tech_id\":\"TEST_TELEINFO\",\"description\":\"Tentative\",\"port\":\"/dev/ttyUSB0\",\"standard\":0}")
assert_http_status 403 "POST /teleinfoedf/set readonly → HTTP 403"

print_suite_summary "Suite 19 - Connecteurs"
[[ ${TESTS_FAILED} -eq 0 ]]
