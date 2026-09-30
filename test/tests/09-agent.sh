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

SERVERS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM server;")
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
# TEST: POST /run/agent/config - Création/upsert du plugin DLS agent
# =============================================================================
log_info "Test: POST /run/agent/config - création du plugin DLS"
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
assert_db_row_exists "dls" \
    "POST /run/agent/config: plugin DLS créé" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "name" "Phidget de test" \
    "POST /run/agent/config name correct" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "shortname" "Phidget de test" \
    "POST /run/agent/config shortname correct" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "package" "Agent_phidget" \
    "POST /run/agent/config package correct" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "enable" "1" \
    "POST /run/agent/config enable=1 à la création" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "syn_id" "2" \
    "POST /run/agent/config syn_id=2 à la création" \
    "tech_id='TEST_PHIDGET'"

log_info "Test: POST /run/agent/config - idempotence"
db_domain_query "UPDATE dls SET name='Ancien nom', shortname='Ancien shortname', package='Old_package' WHERE tech_id='TEST_PHIDGET';" >/dev/null 2>&1

RESPONSE=$(api_call_agent POST /run/agent/config \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "TEST_PHIDGET" "${TEST_DOMAIN_SECRET}" \
    "${AGENT_CONFIG_PAYLOAD}")

assert_http_status 200 "POST /run/agent/config (idempotence) → HTTP 200"
DLS_COUNT=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id='TEST_PHIDGET';")
_test_start
if [[ "${DLS_COUNT}" == "1" ]]; then
    _test_pass "POST /run/agent/config (idempotence) pas de doublon"
else
    _test_fail "POST /run/agent/config (idempotence) doublon détecté" "count=${DLS_COUNT}"
fi

assert_db_field "dls" "name" "Phidget de test" \
    "POST /run/agent/config (idempotence) name remis à jour" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "shortname" "Phidget de test" \
    "POST /run/agent/config (idempotence) shortname remis à jour" \
    "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "package" "Agent_phidget" \
    "POST /run/agent/config (idempotence) package remis à jour" \
    "tech_id='TEST_PHIDGET'"

db_domain_query "DELETE FROM dls WHERE tech_id='TEST_PHIDGET';" >/dev/null 2>&1 || true

# =============================================================================
# TEST: POST /server/set/master - Transfert du serveur master
# =============================================================================
NEW_MASTER_UUID="ffffffff-0000-0000-0000-0000000000aa"
db_domain_query "INSERT INTO server (server_uuid, agent_tech_id, description) VALUES ('${NEW_MASTER_UUID}', 'TEST_MASTER_NEW', 'Serveur master temporaire');" >/dev/null

log_info "Test: POST /server/set/master - modification master"
RESPONSE=$(api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${NEW_MASTER_UUID}\"}")

assert_http_status 200 "POST /server/set/master -> HTTP 200"

MASTER_DB=$(db_domain_query "SELECT is_master FROM server WHERE server_uuid='${NEW_MASTER_UUID}' LIMIT 1;")
_test_start
if [[ "${MASTER_DB}" == "1" ]]; then
    _test_pass "POST /server/set/master master mis a jour en BD"
else
    _test_fail "POST /server/set/master master non mis a jour en BD" "BD='${MASTER_DB}'"
fi

# Restaurer
api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\"}" >/dev/null
db_domain_query "DELETE FROM server WHERE server_uuid='${NEW_MASTER_UUID}';" >/dev/null

log_info "Test: POST /server/set/master - readonly (acces insuffisant)"
RESPONSE=$(api_call POST /server/set/master "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"master\":true}")
assert_http_status 403 "POST /server/set/master readonly -> HTTP 403"

log_info "Test: POST /server/set/master - description ignoree"
RESPONSE=$(api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"description\":\"Tentative\"}")
assert_http_status 200 "POST /server/set/master avec description -> HTTP 200"
assert_db_field "server" "description" "Agent de test fonctionnel" \
    "POST /server/set/master ne modifie pas la description" \
    "server_uuid='${TEST_AGENT_UUID}'"

# =============================================================================
# TEST: DELETE /server/delete
# =============================================================================
DEL_SERVER_UUID="ffffffff-0000-0000-0000-0000000000de"
DEL_SERVER_TECH="TEST_SRV_DEL"
DEL_UPS_TECH="TEST_UPS_DEL"

# Fixture: un serveur non-master + un agent UPS dependant + leurs plugins D.L.S
db_domain_query "DELETE FROM server WHERE server_uuid='${DEL_SERVER_UUID}';" >/dev/null 2>&1 || true
db_domain_query "DELETE FROM dls WHERE tech_id IN ('${DEL_SERVER_TECH}','${DEL_UPS_TECH}');" >/dev/null 2>&1 || true
db_domain_query "INSERT INTO server (server_uuid, agent_tech_id, description, is_master) \
    VALUES ('${DEL_SERVER_UUID}', '${DEL_SERVER_TECH}', 'Serveur a supprimer', 0);" >/dev/null
db_domain_query "INSERT INTO ups (server_uuid, agent_tech_id, description, host, name, admin_username, admin_password) \
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
MASTER_UUID=$(db_domain_query "SELECT server_uuid FROM server WHERE is_master=1 LIMIT 1;")
if [[ -n "${MASTER_UUID}" ]]; then
    RESPONSE=$(api_call DELETE /server/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"server_uuid\":\"${MASTER_UUID}\"}")
    assert_http_status 400 "DELETE /server/delete sur le master -> HTTP 400"
    assert_db_row_exists "server" \
        "DELETE /server/delete: le serveur master est conserve" \
        "server_uuid='${MASTER_UUID}'"
else
    log_info "Aucun serveur master en BD, test master ignore"
fi

log_info "Test: DELETE /server/delete - suppression d'un serveur non-master"
RESPONSE=$(api_call DELETE /server/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${DEL_SERVER_UUID}\"}")
assert_http_status 200 "DELETE /server/delete -> HTTP 200"

assert_db_row_absent "server" \
    "DELETE /server/delete: serveur supprime" \
    "server_uuid='${DEL_SERVER_UUID}'"
assert_db_row_absent "ups" \
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
db_domain_query "INSERT INTO phidget (server_uuid, agent_tech_id, description, hostname, serial) VALUES ('${TEST_AGENT_UUID}', '${TEMP_AGENT_TECH_ID}', 'Agent temporaire', 'temp-phidget', 99001);" >/dev/null

AGENT_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM agents;")
RESPONSE=$(api_call DELETE /agent/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_tech_id\":\"${TEMP_AGENT_TECH_ID}\"}")

assert_http_status 200 "DELETE /agent/delete → HTTP 200"
assert_json_field "${RESPONSE}" "server_uuid" "${TEST_AGENT_UUID}" \
    "DELETE /agent/delete conserve le serveur cible"

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
