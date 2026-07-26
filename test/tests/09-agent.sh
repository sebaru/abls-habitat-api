#!/usr/bin/env bash
# =============================================================================
# 09-agent.sh - Tests des agents
# =============================================================================
# Endpoints testés: GET /agent/list, GET /agent/get,
#                   POST /server/set/headless, POST /server/set/master,
#                   POST /agent/reset, POST /agent/upgrade, POST /agent/send,
#                   DELETE /agent/delete
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

# L'agent de test doit être présent
AGENT_FOUND=$(echo "${RESPONSE}" | jq -r --arg u "${TEST_AGENT_UUID}" \
    '.agents[] | select(.agent_uuid == $u) | .agent_uuid' 2>/dev/null)
_test_start
if [[ "${AGENT_FOUND}" == "${TEST_AGENT_UUID}" ]]; then
    _test_pass "GET /agent/list contient l'agent de test"
else
    _test_fail "GET /agent/list ne contient pas l'agent de test (${TEST_AGENT_UUID})"
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
assert_json_array_not_empty "${RESPONSE}" "server" "GET /servers/list retourne des serveurs"

SERVERS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM servers;")
SERVERS_IN_API=$(echo "${RESPONSE}" | jq '.server | length' 2>/dev/null)
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
# TEST: POST /server/set/headless - Modification du mode headless
# =============================================================================
HEADLESS_BEFORE=$(db_domain_query "SELECT headless FROM servers WHERE server_uuid='${TEST_AGENT_UUID}' LIMIT 1;")
if [[ "${HEADLESS_BEFORE}" == "1" ]]; then
    TARGET_HEADLESS=false
    TARGET_HEADLESS_DB=0
    RESTORE_HEADLESS=true
else
    TARGET_HEADLESS=true
    TARGET_HEADLESS_DB=1
    RESTORE_HEADLESS=false
fi

log_info "Test: POST /server/set/headless - modification headless"
RESPONSE=$(api_call POST /server/set/headless "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"headless\":${TARGET_HEADLESS}}")

assert_http_status 200 "POST /server/set/headless → HTTP 200"

HEADLESS_DB=$(db_domain_query "SELECT headless FROM servers WHERE server_uuid='${TEST_AGENT_UUID}' LIMIT 1;")
_test_start
if [[ "${HEADLESS_DB}" == "${TARGET_HEADLESS_DB}" ]]; then
    _test_pass "POST /server/set/headless headless mis a jour en BD"
else
    _test_fail "POST /server/set/headless headless non mis a jour en BD" "BD='${HEADLESS_DB}'"
fi

# Restaurer
api_call POST /server/set/headless "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"headless\":${RESTORE_HEADLESS}}" >/dev/null

log_info "Test: POST /server/set/headless - readonly (acces insuffisant)"
RESPONSE=$(api_call POST /server/set/headless "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"headless\":true}")
assert_http_status 403 "POST /server/set/headless readonly -> HTTP 403"

log_info "Test: POST /server/set/headless - tentative de modification description refusee"
RESPONSE=$(api_call POST /server/set/headless "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"headless\":true,\"description\":\"Tentative\"}")
assert_http_status 400 "POST /server/set/headless avec description -> HTTP 400"

# =============================================================================
# TEST: POST /server/set/master - Modification du serveur master
# =============================================================================
MASTER_BEFORE=$(db_domain_query "SELECT is_master FROM servers WHERE server_uuid='${TEST_AGENT_UUID}' LIMIT 1;")
if [[ "${MASTER_BEFORE}" == "1" ]]; then
    TARGET_MASTER=false
    TARGET_MASTER_DB=0
    RESTORE_MASTER=true
else
    TARGET_MASTER=true
    TARGET_MASTER_DB=1
    RESTORE_MASTER=false
fi

log_info "Test: POST /server/set/master - modification master"
RESPONSE=$(api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"master\":${TARGET_MASTER}}")

assert_http_status 200 "POST /server/set/master -> HTTP 200"

MASTER_DB=$(db_domain_query "SELECT is_master FROM servers WHERE server_uuid='${TEST_AGENT_UUID}' LIMIT 1;")
_test_start
if [[ "${MASTER_DB}" == "${TARGET_MASTER_DB}" ]]; then
    _test_pass "POST /server/set/master master mis a jour en BD"
else
    _test_fail "POST /server/set/master master non mis a jour en BD" "BD='${MASTER_DB}'"
fi

# Restaurer
api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"master\":${RESTORE_MASTER}}" >/dev/null

log_info "Test: POST /server/set/master - readonly (acces insuffisant)"
RESPONSE=$(api_call POST /server/set/master "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"master\":true}")
assert_http_status 403 "POST /server/set/master readonly -> HTTP 403"

log_info "Test: POST /server/set/master - tentative de modification description refusee"
RESPONSE=$(api_call POST /server/set/master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"server_uuid\":\"${TEST_AGENT_UUID}\",\"master\":true,\"description\":\"Tentative\"}")
assert_http_status 400 "POST /server/set/master avec description -> HTTP 400"

# =============================================================================
# TEST: POST /agent/reset
# =============================================================================
log_info "Test: POST /agent/reset - envoi commande reset (MQTT)"
RESPONSE=$(api_call POST /agent/reset "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /agent/reset → HTTP 200"
else
    _test_fail "POST /agent/reset" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /agent/upgrade
# =============================================================================
log_info "Test: POST /agent/upgrade - envoi commande upgrade"
RESPONSE=$(api_call POST /agent/upgrade "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /agent/upgrade → HTTP 200"
else
    _test_fail "POST /agent/upgrade" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /agent/send - Envoi d'un message à l'agent
# =============================================================================
log_info "Test: POST /agent/send - envoi message"
RESPONSE=$(api_call POST /agent/send "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"message\":\"TEST_PING\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" || "${LAST_HTTP_CODE}" == "400" ]]; then
    _test_pass "POST /agent/send → HTTP ${LAST_HTTP_CODE} (pas d'erreur 500)"
else
    _test_fail "POST /agent/send" "code HTTP inattendu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: DELETE /agent/delete - Créer puis supprimer un agent temporaire
# =============================================================================
log_info "Test: DELETE /agent/delete - création puis suppression"
TEMP_AGENT_UUID="aaaaaaaa-1111-0000-0000-000000000099"
db_domain_query "INSERT IGNORE INTO agents (agent_uuid, agent_hostname, description) VALUES ('${TEMP_AGENT_UUID}', 'temp-host', 'Agent temporaire');" >/dev/null 2>&1 || true

AGENT_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM agents;")
RESPONSE=$(api_call DELETE /agent/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEMP_AGENT_UUID}\"}")

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
