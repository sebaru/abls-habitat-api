#!/usr/bin/env bash
# =============================================================================
# 01-unauthenticated.sh - Tests des endpoints sans authentification
# =============================================================================
# Endpoints testés: GET /ping, GET /status
# Aucun JWT ni X-ABLS-DOMAIN requis pour ces endpoints.
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 01 - Endpoints sans authentification"

# Snapshot de l'état initial de la BD (pour vérifier qu'elle n'est pas modifiée)
INITIAL_USERS=$(db_query "SELECT COUNT(*) FROM users;" abls_master)
INITIAL_DOMAINS=$(db_query "SELECT COUNT(*) FROM domains;" abls_master)

# =============================================================================
# TEST: GET /ping sans token
# =============================================================================
log_info "Test: GET /ping sans token"
RESPONSE=$(api_call_no_auth GET /ping)

assert_http_status 401 "GET /ping sans token retourne HTTP 401"
assert_json_field "${RESPONSE}" "api_error" "not_empty" "GET /ping sans token retourne un message d'erreur"

# =============================================================================
# TEST: GET /status
# =============================================================================
log_info "Test: GET /status"
RESPONSE=$(api_call_no_auth GET /status)

assert_http_status 200 "GET /status retourne HTTP 200"
assert_json_field "${RESPONSE}" "product" "ABLS-HABITAT-API" "GET /status product correct"
assert_json_field "${RESPONSE}" "version" "not_empty" "GET /status version présente"
assert_json_field "${RESPONSE}" "vendor" "ABLS-HABITAT" "GET /status vendor correct"
assert_json_field "${RESPONSE}" "idp_url" "not_empty" "GET /status idp_url présent"
assert_json_field "${RESPONSE}" "idp_realm" "not_empty" "GET /status idp_realm présent"

# Le nombre de domaines doit être cohérent avec la BD
NBR_DOMAINS=$(echo "${RESPONSE}" | jq -r '.nbr_domains // 0' 2>/dev/null)
EXPECTED_DOMAINS=$(db_query "SELECT COUNT(*) FROM domains;" abls_master)
_test_start
if [[ "${NBR_DOMAINS}" == "${EXPECTED_DOMAINS}" ]]; then
    _test_pass "GET /status nbr_domains cohérent avec BD (${NBR_DOMAINS})"
else
    _test_fail "GET /status nbr_domains incohérent" "réponse=${NBR_DOMAINS}, BD=${EXPECTED_DOMAINS}"
fi

# =============================================================================
# TEST: Méthodes non autorisées
# =============================================================================
log_info "Test: POST /ping (méthode non autorisée)"
api_call_no_auth POST /ping >/dev/null
assert_http_status 405 "POST /ping retourne HTTP 405 (Method Not Allowed)"

# =============================================================================
# TEST: Endpoint inexistant
# =============================================================================
log_info "Test: GET /endpoint-qui-nexiste-pas"
api_call_no_auth GET /endpoint-qui-nexiste-pas >/dev/null
# L'API peut retourner 404 ou 400 selon l'implémentation
_test_start
if [[ "${LAST_HTTP_CODE}" == "404" || "${LAST_HTTP_CODE}" == "400" || "${LAST_HTTP_CODE}" == "401" ]]; then
    _test_pass "GET /endpoint-inexistant retourne une erreur (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "GET /endpoint-inexistant" "Code HTTP inattendu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# Vérification: la BD n'a pas été modifiée par ces tests
# =============================================================================
FINAL_USERS=$(db_query "SELECT COUNT(*) FROM users;" abls_master)
FINAL_DOMAINS=$(db_query "SELECT COUNT(*) FROM domains;" abls_master)

_test_start
if [[ "${INITIAL_USERS}" == "${FINAL_USERS}" && "${INITIAL_DOMAINS}" == "${FINAL_DOMAINS}" ]]; then
    _test_pass "BD non modifiée par les tests sans auth (users=${FINAL_USERS}, domains=${FINAL_DOMAINS})"
else
    _test_fail "BD modifiée de façon inattendue" \
        "users: ${INITIAL_USERS}→${FINAL_USERS}, domains: ${INITIAL_DOMAINS}→${FINAL_DOMAINS}"
fi

print_suite_summary "Suite 01 - Unauthenticated"
[[ ${TESTS_FAILED} -eq 0 ]]
