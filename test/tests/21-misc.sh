#!/usr/bin/env bash
# =============================================================================
# 20-misc.sh - Tests des endpoints divers
# =============================================================================
# Endpoints testés: GET /search, GET /audit_log/list,
#                   POST /api/reload_icons, DELETE /visuels/delete
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 20 - Divers"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /search - Recherche dans le dictionnaire
# =============================================================================
log_info "Test: GET /search (sans filtre)"
RESPONSE=$(api_call GET /search "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /search → HTTP 200"

_test_start
RESULT_FIELD=$(echo "${RESPONSE}" | jq -r 'has("results")' 2>/dev/null)
if [[ "${RESULT_FIELD}" == "true" ]]; then
    _test_pass "GET /search retourne un champ 'results'"
else
    _test_fail "GET /search ne retourne pas de champ 'results'" "${RESPONSE}"
fi

log_info "Test: GET /search avec filtre 'TEST_DLS'"
RESPONSE=$(api_call GET "/search?search=TEST_DLS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /search?search=TEST_DLS → HTTP 200"

TEST_DLS_FOUND=$(echo "${RESPONSE}" | jq -r '.results[]? | select(.tech_id == "TEST_DLS") | .tech_id' 2>/dev/null | head -1)
_test_start
if [[ "${TEST_DLS_FOUND}" == "TEST_DLS" ]]; then
    _test_pass "GET /search?search=TEST_DLS retourne TEST_DLS"
else
    _test_fail "GET /search?search=TEST_DLS ne retourne pas TEST_DLS" "${RESPONSE}"
fi

log_info "Test: GET /search - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /search "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /search readonly → HTTP 403"

# =============================================================================
# TEST: GET /audit_log/list - Liste des entrées du journal d'audit
# =============================================================================
log_info "Test: GET /audit_log/list"
RESPONSE=$(api_call GET /audit_log/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /audit_log/list → HTTP 200"

_test_start
AUDIT_FIELD=$(echo "${RESPONSE}" | jq -r 'has("audit_logs")' 2>/dev/null)
if [[ "${AUDIT_FIELD}" == "true" ]]; then
    _test_pass "GET /audit_log/list retourne un champ 'audit_logs'"
else
    _test_fail "GET /audit_log/list ne retourne pas de champ 'audit_logs'" "${RESPONSE}"
fi

log_info "Test: GET /audit_log/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /audit_log/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /audit_log/list readonly → HTTP 403"

# =============================================================================
# TEST: POST /api/reload_icons - Recharger les icônes depuis le disque
# =============================================================================
log_info "Test: POST /api/reload_icons"
RESPONSE=$(api_call POST /api/reload_icons "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /api/reload_icons → HTTP 200"
else
    _test_fail "POST /api/reload_icons" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: DELETE /visuels/delete - Supprimer les visuels en mémoire
# =============================================================================
log_info "Test: DELETE /visuels/delete"
RESPONSE=$(api_call DELETE /visuels/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "DELETE /visuels/delete → HTTP 200"
else
    _test_fail "DELETE /visuels/delete" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

print_suite_summary "Suite 20 - Divers"
[[ ${TESTS_FAILED} -eq 0 ]]
