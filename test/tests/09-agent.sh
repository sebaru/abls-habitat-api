#!/usr/bin/env bash
# =============================================================================
# 09-agent.sh - Tests des agents
# =============================================================================
# Endpoints testés: GET /agent/list, GET /agent/get,
#                   POST /run/agent/config, POST /server/set/master,
#                   POST /agent/log_level, /agent/enable, /agent/start,
#                   /agent/restart, /agent/stop, /agent/upgrade, POST /agent/send,
#                   DELETE /agent/delete, DELETE /server/delete
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 09 - Agents"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

log_info "Test: GET /log/facility/list"
RESPONSE=$(api_call GET /log/facility/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /log/facility/list returns HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "log_facilities" "Facility catalogue available"
FACILITIES_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM log_facilities;")
assert_json_field "${RESPONSE}" "nbr_log_facilities" "${FACILITIES_IN_DB}" "Facility count matches database"
EXPECTED_FACILITIES='["agent","alexa","api_config","archive","audio","audit","auth","camera","config","database","distrib","dls","dls_monitor","domain","gpio","gpiod","histo","http","icons","imsg","json","local_config","log","logguer","mail","mapping","meteo","mnemo","modbus","monitor","mqtt","mqtt_api","mqtt_local","phidget","plugin","run","server","shelly","signal","sms","synoptique","tableau","teleinfoedf","ups","user","visuel"]'
_test_start
if echo "${RESPONSE}" | jq -e --argjson expected "${EXPECTED_FACILITIES}" '
    (.log_facilities | map(.log_facility)) == $expected and
    (.log_facilities | length) == .nbr_log_facilities and
    (.log_facilities | all((.log_facility_id | type) == "number" and (.log_facility | type) == "string")) and
    (.log_facilities | map(.log_facility_id) | length == (unique | length))
' >/dev/null 2>&1; then
    _test_pass "Complete sorted catalogue outside SRC with unique numeric identifiers"
else
    _test_fail "Incorrect facility catalogue" "${RESPONSE}"
fi
FACILITY_ROWS_DB=$(db_domain_query "SELECT log_facility_id, log_facility FROM log_facilities ORDER BY log_facility;")
FACILITY_ROWS_API=$(echo "${RESPONSE}" | jq -r '.log_facilities[] | [.log_facility_id, .log_facility] | @tsv')
_test_start
if [[ "${FACILITY_ROWS_API}" == "${FACILITY_ROWS_DB}" ]]; then
    _test_pass "Facility catalogue identifiers and names match database"
else
    _test_fail "Facility catalogue differs from database"
fi
RESPONSE=$(api_call GET /log/facility/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "Readonly facility catalogue request refused"
RESPONSE=$(api_call GET /log/facility/list "" "${TEST_DOMAIN_UUID}")
assert_http_status 401 "Unauthenticated facility catalogue request refused"

# =============================================================================
# TEST: GET /agent/list
# =============================================================================
log_info "Test: GET /agent/list"
RESPONSE=$(api_call GET /agent/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /agent/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "agents" "GET /agent/list retourne des agents"

# Cohérence avec la BD
AGENTS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM agents;")
AGENTS_IN_API=$(echo "${RESPONSE}" | jq '.agents | length' 2>/dev/null)
_test_start
if [[ "${AGENTS_IN_API}" == "${AGENTS_IN_DB}" ]]; then
    _test_pass "GET /agent/list nombre cohérent avec BD (${AGENTS_IN_DB})"
else
    _test_fail "GET /agent/list nombre incohérent" "API=${AGENTS_IN_API}, BD=${AGENTS_IN_DB}"
fi

# L'agent phidget de test doit être présent
AGENT_FOUND=$(echo "${RESPONSE}" | jq -r '.agents[] | select(.agent_tech_id == "TEST_PHIDGET") | .agent_tech_id' 2>/dev/null)
_test_start
if [[ "${AGENT_FOUND}" == "TEST_PHIDGET" ]]; then
    _test_pass "GET /agent/list contient l'agent de test"
else
    _test_fail "GET /agent/list ne contient pas l'agent de test (TEST_PHIDGET)"
fi

log_info "Test: GET /agent/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /agent/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /agent/list readonly → HTTP 403"

log_info "Test: GET /agent/list?classe=phidget (filtre classe)"
RESPONSE=$(api_call GET "/agent/list?classe=phidget" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /agent/list?classe=phidget → HTTP 200"

_test_start
if echo "${RESPONSE}" | jq -e '.agents | all(.agent_classe == "phidget")' >/dev/null 2>&1; then
    _test_pass "GET /agent/list?classe=phidget ne retourne que des agents phidget"
else
    _test_fail "GET /agent/list?classe=phidget retourne des classes inattendues"
fi

# =============================================================================
# TEST: GET /servers/list
# =============================================================================
log_info "Test: GET /servers/list"
RESPONSE=$(api_call GET /servers/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /servers/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "servers" "GET /servers/list retourne des serveurs"

SERVERS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM agent_server;")
SERVERS_IN_API=$(echo "${RESPONSE}" | jq '.servers | length' 2>/dev/null)
_test_start
if [[ "${SERVERS_IN_API}" == "${SERVERS_IN_DB}" ]]; then
    _test_pass "GET /servers/list nombre cohérent avec BD (${SERVERS_IN_DB})"
else
    _test_fail "GET /servers/list nombre incohérent" "API=${SERVERS_IN_API}, BD=${SERVERS_IN_DB}"
fi

log_info "Test: GET /servers/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /servers/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /servers/list readonly → HTTP 403"

log_info "Test: GET /server/get?server_uuid=TEST_AGENT_UUID"
RESPONSE=$(api_call GET "/server/get?server_uuid=${TEST_AGENT_UUID}" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /server/get → HTTP 200"
_test_start
if echo "${RESPONSE}" | jq -e --arg uuid "${TEST_AGENT_UUID}" '.server_uuid == $uuid and .agent_tech_id == "test-agent-host" and has("is_alive") and (.local_agents | any(.agent_tech_id == "TEST_PHIDGET"))' >/dev/null 2>&1; then
    _test_pass "GET /server/get retourne serveur et agents locaux"
else
    _test_fail "GET /server/get ne retourne pas les données attendues" "${RESPONSE}"
fi

RESPONSE=$(api_call GET /server/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /server/get sans server_uuid → HTTP 400"
RESPONSE=$(api_call GET "/server/get?server_uuid=00000000-0000-0000-0000-000000000000" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /server/get serveur inconnu → HTTP 404"
RESPONSE=$(api_call GET "/server/get?server_uuid=${TEST_AGENT_UUID}" "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /server/get readonly → HTTP 403"

# =============================================================================
# TEST: GET /agent/get
# =============================================================================
log_info "Test: GET /agent/get?agent_tech_id=TEST_PHIDGET"
RESPONSE=$(api_call GET "/agent/get?agent_tech_id=TEST_PHIDGET" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /agent/get?agent_tech_id=TEST_PHIDGET → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_PHIDGET" "GET /agent/get agent_tech_id correct"

log_info "Test: GET /agent/get - paramètre manquant"
RESPONSE=$(api_call GET "/agent/get" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /agent/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /agent/get?agent_tech_id=UNKNOWN_TECH"
RESPONSE=$(api_call GET "/agent/get?agent_tech_id=UNKNOWN_TECH" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /agent/get avec agent_tech_id inconnu → HTTP 404"

# =============================================================================
# TEST: POST /run/agent/config - ne crée plus le plugin DLS (créé par les endpoints */set)
# =============================================================================
log_info "Test: POST /run/agent/config - pas de création du plugin DLS"
db_domain_query "DELETE FROM dls WHERE tech_id='TEST_PHIDGET';" >/dev/null 2>&1 || true

AGENT_CONFIG_PAYLOAD=$(jq -cn \
    --arg agent_classe "phidget" \
    --arg agent_tech_id "TEST_PHIDGET" \
    --arg version "test" \
    --argjson start_time "$(date +%s)" \
    '{"agent_classe":$agent_classe,"agent_tech_id":$agent_tech_id,"version":$version,"start_time":$start_time}')

RESPONSE=$(api_call_agent POST /run/agent/config \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "TEST_PHIDGET" "${TEST_DOMAIN_SECRET}" \
    "${AGENT_CONFIG_PAYLOAD}")

assert_http_status 200 "POST /run/agent/config → HTTP 200"
assert_db_row_absent "dls" \
    "POST /run/agent/config: plugin DLS non créé" \
    "tech_id='TEST_PHIDGET'"

# =============================================================================
# TEST: POST /server/set/master - Transfert du serveur master
# =============================================================================
NEW_MASTER_UUID="ffffffff-0000-0000-0000-0000000000aa"
db_domain_query "INSERT INTO agent_server (server_uuid, agent_tech_id, description) VALUES ('${NEW_MASTER_UUID}', 'TEST_MASTER_NEW', 'Serveur master temporaire');" >/dev/null

log_info "Test: POST /server/set/master - modification master"
RESPONSE=$(api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${NEW_MASTER_UUID}\"}")

assert_http_status 200 "POST /server/set/master -> HTTP 200"

MASTER_DB=$(db_domain_query "SELECT is_master FROM agent_server WHERE server_uuid='${NEW_MASTER_UUID}' LIMIT 1;")
_test_start
if [[ "${MASTER_DB}" == "1" ]]; then
    _test_pass "POST /server/set/master master mis a jour en BD"
else
    _test_fail "POST /server/set/master master non mis a jour en BD" "BD='${MASTER_DB}'"
fi

# Restaurer
api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\"}" >/dev/null
db_domain_query "DELETE FROM agent_server WHERE server_uuid='${NEW_MASTER_UUID}';" >/dev/null

log_info "Test: POST /server/set/master - readonly (acces insuffisant)"
RESPONSE=$(api_call POST /server/set/master "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"master\":true}")
assert_http_status 403 "POST /server/set/master readonly -> HTTP 403"

log_info "Test: POST /server/set/master - description ignoree"
RESPONSE=$(api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"description\":\"Tentative\"}")
assert_http_status 200 "POST /server/set/master avec description -> HTTP 200"
assert_db_field "agent_server" "description" "Agent de test fonctionnel" \
    "POST /server/set/master ne modifie pas la description" \
    "server_uuid='${TEST_AGENT_UUID}'"

# =============================================================================
# TEST: DELETE /server/delete
# =============================================================================
DEL_SERVER_UUID="ffffffff-0000-0000-0000-0000000000de"
DEL_SERVER_TECH="TEST_SRV_DEL"
DEL_UPS_TECH="TEST_UPS_DEL"

# Fixture: un serveur non-master + un agent UPS dependant + leurs plugins D.L.S
db_domain_query "DELETE FROM agent_server WHERE server_uuid='${DEL_SERVER_UUID}';" >/dev/null 2>&1 || true
db_domain_query "DELETE FROM dls WHERE tech_id IN ('${DEL_SERVER_TECH}','${DEL_UPS_TECH}');" >/dev/null 2>&1 || true
db_domain_query "INSERT INTO agent_server (server_uuid, agent_tech_id, description, is_master) \
    VALUES ('${DEL_SERVER_UUID}', '${DEL_SERVER_TECH}', 'Serveur a supprimer', 0);" >/dev/null
db_domain_query "INSERT INTO agent_ups (server_uuid, agent_tech_id, description, host, name, admin_username, admin_password) \
    VALUES ('${DEL_SERVER_UUID}', '${DEL_UPS_TECH}', 'UPS a supprimer', 'localhost', 'UPS-DEL', 'admin', 'pass');" >/dev/null
db_domain_query "INSERT INTO dls (syn_id, name, shortname, tech_id) \
    VALUES (1, 'Srv del', 'Srv del', '${DEL_SERVER_TECH}'), (1, 'Ups del', 'Ups del', '${DEL_UPS_TECH}');" >/dev/null

log_info "Test: DELETE /server/delete - parametre manquant"
RESPONSE=$(api_call DELETE /server/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "{}")
assert_http_status 400 "DELETE /server/delete sans server_uuid -> HTTP 400"

log_info "Test: DELETE /server/delete - readonly (acces insuffisant)"
RESPONSE=$(api_call DELETE /server/delete "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${DEL_SERVER_UUID}\"}")
assert_http_status 403 "DELETE /server/delete readonly -> HTTP 403"

log_info "Test: DELETE /server/delete - server_uuid inconnu"
RESPONSE=$(api_call DELETE /server/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"ffffffff-0000-0000-0000-0000000000ff\"}")
assert_http_status 404 "DELETE /server/delete avec server_uuid inconnu -> HTTP 404"

log_info "Test: DELETE /server/delete - serveur master refuse"
MASTER_UUID=$(db_domain_query "SELECT server_uuid FROM agent_server WHERE is_master=1 LIMIT 1;")
if [[ -n "${MASTER_UUID}" ]]; then
    RESPONSE=$(api_call DELETE /server/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"server_uuid\":\"${MASTER_UUID}\"}")
    assert_http_status 400 "DELETE /server/delete sur le master -> HTTP 400"
    assert_db_row_exists "agent_server" \
        "DELETE /server/delete: le serveur master est conserve" \
        "server_uuid='${MASTER_UUID}'"
else
    log_info "Aucun serveur master en BD, test master ignore"
fi

log_info "Test: DELETE /server/delete - suppression d'un serveur non-master"
RESPONSE=$(api_call DELETE /server/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${DEL_SERVER_UUID}\"}")
assert_http_status 200 "DELETE /server/delete -> HTTP 200"

assert_db_row_absent "agent_server" \
    "DELETE /server/delete: serveur supprime" \
    "server_uuid='${DEL_SERVER_UUID}'"
assert_db_row_absent "agent_ups" \
    "DELETE /server/delete: agent dependant supprime (cascade)" \
    "agent_tech_id='${DEL_UPS_TECH}'"
assert_db_row_absent "dls" \
    "DELETE /server/delete: plugin D.L.S du serveur supprime" \
    "tech_id='${DEL_SERVER_TECH}'"
assert_db_row_absent "dls" \
    "DELETE /server/delete: plugin D.L.S de l'agent dependant supprime" \
    "tech_id='${DEL_UPS_TECH}'"

# =============================================================================
# TEST: POST /agent/reset - ancienne route non dispatchée
# =============================================================================
log_info "Test: POST /agent/reset - route supprimée"
RESPONSE=$(api_call POST /agent/reset "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\"}")
assert_http_status 404 "POST /agent/reset supprimé → HTTP 404"

for action in log_level enable start restart stop; do
    RESPONSE=$(api_call POST "/agent/${action}" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')
    assert_http_status 400 "POST /agent/${action} sans agent_tech_id → HTTP 400"
done

# =============================================================================
# TEST: POST /agent/upgrade
# =============================================================================
log_info "Test: POST /agent/upgrade - envoi commande upgrade"
RESPONSE=$(api_call POST /agent/upgrade "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"TEST_PHIDGET"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /agent/upgrade → HTTP 200"
else
    _test_fail "POST /agent/upgrade" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /agent/test
# =============================================================================
log_info "Test: POST /agent/test - envoi commande test (MQTT)"
RESPONSE=$(api_call POST /agent/test "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_tech_id\":\"TEST_PHIDGET\"}")
assert_http_status 200 "POST /agent/test → HTTP 200"

log_info "Test: POST /agent/test - paramètre manquant"
RESPONSE=$(api_call POST /agent/test "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "{}")
assert_http_status 400 "POST /agent/test sans agent_tech_id → HTTP 400"

log_info "Test: POST /agent/test - agent_tech_id inconnu"
RESPONSE=$(api_call POST /agent/test "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_tech_id\":\"UNKNOWN_TECH\"}")
assert_http_status 404 "POST /agent/test avec agent_tech_id inconnu → HTTP 404"

# =============================================================================
# TEST: POST /agent/send - Endpoint supprimé
# =============================================================================
log_info "Test: POST /agent/send - endpoint supprimé"
RESPONSE=$(api_call POST /agent/send "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')
assert_http_status 404 "POST /agent/send supprimé → HTTP 404"

# =============================================================================
# TEST: DELETE /agent/delete - Créer puis supprimer un agent temporaire
# =============================================================================
log_info "Test: DELETE /agent/delete - création puis suppression"
TEMP_AGENT_TECH_ID="TEST_PHIDGET_DEL"
db_domain_query "INSERT INTO agent_phidget (server_uuid, agent_tech_id, description, hostname, serial) VALUES ('${TEST_AGENT_UUID}', '${TEMP_AGENT_TECH_ID}', 'Agent temporaire', 'temp-phidget', 99001);" >/dev/null

AGENT_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM agents;")
RESPONSE=$(api_call DELETE /agent/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_tech_id\":\"${TEMP_AGENT_TECH_ID}\"}")

assert_http_status 200 "DELETE /agent/delete → HTTP 200"

AGENT_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM agents;")
_test_start
if [[ "$((AGENT_CNT_BEFORE - 1))" == "${AGENT_CNT_AFTER}" ]]; then
    _test_pass "DELETE /agent/delete: agent supprimé de la BD"
else
    _test_fail "DELETE /agent/delete: compteur incohérent" \
        "avant=${AGENT_CNT_BEFORE}, après=${AGENT_CNT_AFTER}"
fi

print_suite_summary "Suite 09 - Agents"
[[ ${TESTS_FAILED} -eq 0 ]]
