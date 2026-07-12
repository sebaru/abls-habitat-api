#!/usr/bin/env bash
# =============================================================================
# 10-thread.sh - Tests des threads (connecteurs génériques)
# =============================================================================
# Endpoints testés: GET /thread/list,
#                   POST /thread/enable, POST /agent/log_level, POST /thread/test,
#                   DELETE /thread/delete
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 10 - Threads (connecteurs)"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /thread/list
# =============================================================================
log_info "Test: GET /thread/list"
RESPONSE=$(api_call GET /thread/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /thread/list → HTTP 200"
_test_start
if echo "${RESPONSE}" | jq -e '.' >/dev/null 2>&1; then
    _test_pass "GET /thread/list retourne du JSON valide"
else
    _test_fail "GET /thread/list ne retourne pas du JSON valide" "${RESPONSE}"
fi

# TEST_MODBUS doit être présent dans la liste des threads
MOD_FOUND=$(echo "${RESPONSE}" | jq -r '.. | objects | select(.thread_tech_id == "TEST_MODBUS") | .thread_tech_id' 2>/dev/null | head -1)
_test_start
if [[ "${MOD_FOUND}" == "TEST_MODBUS" ]]; then
    _test_pass "GET /thread/list contient TEST_MODBUS"
else
    _test_fail "GET /thread/list ne contient pas TEST_MODBUS" "${RESPONSE}"
fi

log_info "Test: GET /thread/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /thread/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /thread/list readonly → HTTP 403"

# =============================================================================
# TEST: POST /thread/enable
# =============================================================================
log_info "Test: POST /thread/enable - désactivation TEST_MODBUS"
RESPONSE=$(api_call POST /thread/enable "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_MODBUS","classe":"modbus","enable":false}')

assert_http_status 200 "POST /thread/enable (disable) → HTTP 200"

ENABLE_DB=$(db_domain_query "SELECT enable FROM modbus WHERE thread_tech_id='TEST_MODBUS' LIMIT 1;")
_test_start
if [[ "${ENABLE_DB}" == "0" ]]; then
    _test_pass "POST /thread/enable disable=0 en BD"
else
    _test_fail "POST /thread/enable disable non appliqué en BD" "BD='${ENABLE_DB}'"
fi

log_info "Test: POST /thread/enable - réactivation TEST_MODBUS"
RESPONSE=$(api_call POST /thread/enable "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_MODBUS","classe":"modbus","enable":true}')

assert_http_status 200 "POST /thread/enable (enable) → HTTP 200"

ENABLE_DB=$(db_domain_query "SELECT enable FROM modbus WHERE thread_tech_id='TEST_MODBUS' LIMIT 1;")
_test_start
if [[ "${ENABLE_DB}" == "1" ]]; then
    _test_pass "POST /thread/enable enable=1 restauré en BD"
else
    _test_fail "POST /thread/enable enable non restauré en BD" "BD='${ENABLE_DB}'"
fi

log_info "Test: POST /thread/enable - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /thread/enable "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_MODBUS","classe":"modbus","enable":false}')
assert_http_status 403 "POST /thread/enable readonly → HTTP 403"

# =============================================================================
# TEST: POST /agent/log_level
# =============================================================================
log_info "Test: POST /agent/log_level - LOG_DEBUG sur l'agent de TEST_MODBUS"
RESPONSE=$(api_call POST /agent/log_level "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"TEST_MODBUS","log_level":7}')

assert_http_status 200 "POST /agent/log_level (LOG_DEBUG) → HTTP 200"

LOG_LEVEL_DB=$(db_domain_query "SELECT a.log_level FROM agents a INNER JOIN modbus m USING(agent_uuid) WHERE m.thread_tech_id='TEST_MODBUS' LIMIT 1;")
_test_start
if [[ "${LOG_LEVEL_DB}" == "7" ]]; then
    _test_pass "POST /agent/log_level log_level=7 en BD"
else
    _test_fail "POST /agent/log_level log_level non appliqué en BD" "BD='${LOG_LEVEL_DB}'"
fi

log_info "Test: POST /agent/log_level - LOG_INFO sur l'agent de TEST_MODBUS"
RESPONSE=$(api_call POST /agent/log_level "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"TEST_MODBUS","log_level":6}')

assert_http_status 200 "POST /agent/log_level (LOG_INFO) → HTTP 200"

LOG_LEVEL_DB=$(db_domain_query "SELECT a.log_level FROM agents a INNER JOIN modbus m USING(agent_uuid) WHERE m.thread_tech_id='TEST_MODBUS' LIMIT 1;")
_test_start
if [[ "${LOG_LEVEL_DB}" == "6" ]]; then
    _test_pass "POST /agent/log_level log_level=6 restauré en BD"
else
    _test_fail "POST /agent/log_level log_level non restauré en BD" "BD='${LOG_LEVEL_DB}'"
fi

# =============================================================================
# TEST: POST /thread/test - Envoi commande TEST (MQTT)
# =============================================================================
log_info "Test: POST /thread/test - envoi commande TEST"
RESPONSE=$(api_call POST /thread/test "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_MODBUS","classe":"modbus"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /thread/test → HTTP 200"
else
    _test_fail "POST /thread/test" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: DELETE /thread/delete - Créer puis supprimer un thread temporaire
# =============================================================================
log_info "Test: DELETE /thread/delete - création puis suppression"
db_domain_query "INSERT IGNORE INTO modbus (agent_uuid, thread_tech_id, description, hostname) VALUES ('${TEST_AGENT_UUID}', 'TEST_MOD_TEMP', 'Modbus temp', '192.168.1.250');" >/dev/null 2>&1 || true
# Aussi créer l'entrée dls associée pour satisfaire la suppression complète
db_domain_query "INSERT IGNORE INTO dls (tech_id, syn_id, name, shortname, enable) VALUES ('TEST_MOD_TEMP', 1, 'Modbus temp DLS', 'ModTemp', 0);" >/dev/null 2>&1 || true

THREAD_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM modbus;")
RESPONSE=$(api_call DELETE /thread/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_MOD_TEMP","classe":"modbus"}')

assert_http_status 200 "DELETE /thread/delete → HTTP 200"

THREAD_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM modbus;")
_test_start
if [[ "$((THREAD_CNT_BEFORE - 1))" == "${THREAD_CNT_AFTER}" ]]; then
    _test_pass "DELETE /thread/delete: thread supprimé de la BD"
else
    _test_fail "DELETE /thread/delete: compteur incohérent" \
        "avant=${THREAD_CNT_BEFORE}, après=${THREAD_CNT_AFTER}"
fi
# Nettoyage résiduel
db_domain_query "DELETE FROM modbus WHERE thread_tech_id='TEST_MOD_TEMP';" >/dev/null 2>&1 || true
db_domain_query "DELETE FROM dls WHERE tech_id='TEST_MOD_TEMP';" >/dev/null 2>&1 || true

print_suite_summary "Suite 10 - Threads"
[[ ${TESTS_FAILED} -eq 0 ]]
