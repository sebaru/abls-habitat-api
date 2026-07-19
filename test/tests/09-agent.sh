#!/usr/bin/env bash
# =============================================================================
# 09-agent.sh - Tests des agents
# =============================================================================
# Endpoints testés: GET /agent/list, GET /agent/get,
#                   POST /agent/set, POST /agent/set_master,
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
assert_json_array_not_empty "${RESPONSE}" "servers" "GET /servers/list retourne des serveurs"

SERVERS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM servers;")
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
# TEST: POST /agent/set - Modification de la description
# =============================================================================
log_info "Test: POST /agent/set - modification description"
RESPONSE=$(api_call POST /agent/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"description\":\"Agent de test modifié\"}")

assert_http_status 200 "POST /agent/set → HTTP 200"

DESC_DB=$(db_domain_query "SELECT description FROM agents WHERE agent_uuid='${TEST_AGENT_UUID}' LIMIT 1;")
_test_start
if [[ "${DESC_DB}" == "Agent de test modifié" ]]; then
    _test_pass "POST /agent/set description mise à jour en BD"
else
    _test_fail "POST /agent/set description non mise à jour en BD" "BD='${DESC_DB}'"
fi

# Restaurer
api_call POST /agent/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"description\":\"Agent de test fonctionnel\"}" >/dev/null

log_info "Test: POST /agent/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /agent/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\",\"description\":\"Tentative\"}")
assert_http_status 403 "POST /agent/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /agent/set_master
# =============================================================================
log_info "Test: POST /agent/set_master"
RESPONSE=$(api_call POST /agent/set_master "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"agent_uuid\":\"${TEST_AGENT_UUID}\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /agent/set_master → HTTP 200"
else
    _test_fail "POST /agent/set_master" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

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
