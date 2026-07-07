#!/usr/bin/env bash
# =============================================================================
# 15-histo.sh - Tests des endpoints Historique
# =============================================================================
# Endpoints testés: GET /histo/alive, GET /histo/search,
#                   POST /histo/acquit
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 15 - Historique"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /histo/alive - Messages actifs (date_fin IS NULL)
# =============================================================================
log_info "Test: GET /histo/alive"
RESPONSE=$(api_call GET /histo/alive "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /histo/alive → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "histo_msgs" "GET /histo/alive retourne des messages actifs"

log_info "Test: GET /histo/alive - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /histo/alive "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /histo/alive readonly → HTTP 403"

# =============================================================================
# TEST: GET /histo/search - Rechercher dans l'historique
# =============================================================================
log_info "Test: GET /histo/search avec filtre tech_id"
NOW=$(date +%s)
TWO_DAYS_AGO=$((NOW - 172800))
RESPONSE=$(api_call GET "/histo/search?tech_id=TEST_DLS&date_start=${TWO_DAYS_AGO}&date_end=${NOW}" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /histo/search → HTTP 200"

log_info "Test: GET /histo/search - readonly (accès insuffisant)"
RESPONSE=$(api_call GET "/histo/search?tech_id=TEST_DLS&date_start=${TWO_DAYS_AGO}&date_end=${NOW}" \
    "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /histo/search readonly → HTTP 403"

# Récupérer un histo_id valide depuis la BD
HISTO_ID=$(db_domain_query \
    "SELECT histo_id FROM histo_msgs WHERE tech_id='TEST_DLS' AND date_fin IS NULL ORDER BY histo_id LIMIT 1;")

# =============================================================================
# TEST: POST /histo/acquit - Acquitter un message
# =============================================================================
if [[ -n "${HISTO_ID}" && "${HISTO_ID}" != "NULL" ]]; then
    log_info "Test: POST /histo/acquit histo_id=${HISTO_ID}"
    RESPONSE=$(api_call POST /histo/acquit "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"histo_id\":${HISTO_ID}}")

    assert_http_status 200 "POST /histo/acquit → HTTP 200"

    DATE_ACQ=$(db_domain_query \
        "SELECT date_acquit FROM histo_msgs WHERE histo_id=${HISTO_ID} LIMIT 1;")
    _test_start
    if [[ -n "${DATE_ACQ}" && "${DATE_ACQ}" != "NULL" ]]; then
        _test_pass "POST /histo/acquit: date_acquit renseignée en BD"
    else
        _test_fail "POST /histo/acquit: date_acquit non renseignée en BD" "histo_id=${HISTO_ID}"
    fi

    log_info "Test: POST /histo/acquit - readonly (accès insuffisant)"
    HISTO_ID2=$(db_domain_query \
        "SELECT histo_id FROM histo_msgs WHERE tech_id='TEST_DLS' AND date_fin IS NULL ORDER BY histo_id DESC LIMIT 1;")
    if [[ -n "${HISTO_ID2}" && "${HISTO_ID2}" != "NULL" ]]; then
        RESPONSE=$(api_call POST /histo/acquit "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
            "{\"histo_id\":${HISTO_ID2}}")
        assert_http_status 403 "POST /histo/acquit readonly → HTTP 403"
    fi
else
    log_info "SKIP: Aucun histo_id actif disponible pour le test /histo/acquit"
fi

print_suite_summary "Suite 15 - Historique"
[[ ${TESTS_FAILED} -eq 0 ]]
