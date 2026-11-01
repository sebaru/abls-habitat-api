#!/usr/bin/env bash
# =============================================================================
# 16-histo.sh - Tests des endpoints Historique
# =============================================================================
# Endpoints testés: GET /histo/alive, GET /histo/search,
#                   POST /histo/acquit
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 16 - Historique"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# Les fixtures sont clôturées par le reload DLS des suites précédentes: on recrée des messages actifs.
db_domain_query "INSERT INTO histo_msgs (tech_id, acronyme, syn_page, dls_shortname, typologie, date_create, libelle) VALUES
    ('TEST_DLS', 'TEST_MSG', 'HOME', 'TestDLS', 0, NOW(2) - INTERVAL 2 MINUTE, 'Alarme histo active 1'),
    ('TEST_DLS', 'TEST_MSG', 'HOME', 'TestDLS', 0, NOW(2) - INTERVAL 1 MINUTE, 'Alarme histo active 2');" >/dev/null

# =============================================================================
# TEST: GET /histo/alive - Messages actifs (date_fin IS NULL)
# =============================================================================
log_info "Test: GET /histo/alive"
RESPONSE=$(api_call GET /histo/alive "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /histo/alive → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "histo_msgs" "GET /histo/alive retourne des messages actifs"

log_info "Test: GET /histo/alive - readonly (consultation autorisée)"
RESPONSE=$(api_call GET /histo/alive "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /histo/alive readonly → HTTP 200"

log_info "Test: GET /histo/alive - sans token"
RESPONSE=$(api_call GET /histo/alive "" "${TEST_DOMAIN_UUID}")
_test_start
if [[ "${LAST_HTTP_CODE}" == "401" || "${LAST_HTTP_CODE}" == "403" ]]; then
    _test_pass "GET /histo/alive sans token refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "GET /histo/alive sans token non refusé" "reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: GET /histo/search - Rechercher dans l'historique
# =============================================================================
log_info "Test: GET /histo/search?search=TEST_DLS&period=1d"
RESPONSE=$(api_call GET "/histo/search?search=TEST_DLS&period=1d" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /histo/search → HTTP 200"

log_info "Test: GET /histo/search - champ obligatoire manquant"
RESPONSE=$(api_call GET "/histo/search?search=TEST_DLS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /histo/search sans period → HTTP 400"

log_info "Test: GET /histo/search - readonly (consultation autorisée)"
RESPONSE=$(api_call GET "/histo/search?search=TEST_DLS&period=1d" "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /histo/search readonly → HTTP 200"

# =============================================================================
# TEST: POST /histo/acquit - Acquitter les messages actifs d'un DLS
# =============================================================================
log_info "Test: POST /histo/acquit tech_id=TEST_DLS"
RESPONSE=$(api_call POST /histo/acquit "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{"tech_id":"TEST_DLS"}')

assert_http_status 200 "POST /histo/acquit → HTTP 200"

NOT_ACKED=$(db_domain_query \
    "SELECT COUNT(*) FROM histo_msgs WHERE tech_id='TEST_DLS' AND date_fin IS NULL AND nom_ack IS NULL;")
_test_start
if [[ "${NOT_ACKED}" == "0" ]]; then
    _test_pass "POST /histo/acquit: messages actifs acquittés en BD"
else
    _test_fail "POST /histo/acquit: messages non acquittés en BD" "restants=${NOT_ACKED}"
fi

log_info "Test: POST /histo/acquit - champ obligatoire manquant"
RESPONSE=$(api_call POST /histo/acquit "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')
assert_http_status 400 "POST /histo/acquit sans tech_id → HTTP 400"

print_suite_summary "Suite 16 - Historique"
[[ ${TESTS_FAILED} -eq 0 ]]
