#!/usr/bin/env bash
# =============================================================================
# 14-archive.sh - Tests des endpoints Archive
# =============================================================================
# Endpoints testés: GET /archive/status/hot, GET /archive/status/cold,
#                   POST /archive/set, POST /archive/rebuild,
#                   POST /archive/move_hot_to_cold, POST /archive/get,
#                   DELETE /archive/delete, DELETE /archive/delete_old_cold
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 14 - Archive"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /archive/status/hot
# =============================================================================
log_info "Test: GET /archive/status/hot"
RESPONSE=$(api_call GET "/archive/status/hot?tech_id=TEST_DLS&acronyme=TEST_AI" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /archive/status/hot → HTTP 200"

log_info "Test: GET /archive/status/hot - readonly (accès insuffisant)"
RESPONSE=$(api_call GET "/archive/status/hot?tech_id=TEST_DLS&acronyme=TEST_AI" \
    "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /archive/status/hot readonly → HTTP 403"

# =============================================================================
# TEST: GET /archive/status/cold
# =============================================================================
log_info "Test: GET /archive/status/cold"
RESPONSE=$(api_call GET "/archive/status/cold?tech_id=TEST_DLS&acronyme=TEST_AI" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /archive/status/cold → HTTP 200"

# =============================================================================
# TEST: POST /archive/set - Configurer l'archivage d'un mnémo
# =============================================================================
log_info "Test: POST /archive/set - activation de l'archivage"
RESPONSE=$(api_call POST /archive/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI","archive_enable_hot":true,"duree_hot":7,"archive_enable_cold":false,"duree_cold":0}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /archive/set → HTTP 200"
else
    _test_fail "POST /archive/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

ARCH_HOT=$(db_domain_query "SELECT archive_enable_hot FROM mnemos_AI WHERE tech_id='TEST_DLS' AND acronyme='TEST_AI' LIMIT 1;")
_test_start
if [[ "${ARCH_HOT}" == "1" ]]; then
    _test_pass "POST /archive/set: archive_enable_hot=1 en BD"
else
    _test_fail "POST /archive/set: archive_enable_hot non mis à jour en BD" "BD='${ARCH_HOT}'"
fi

log_info "Test: POST /archive/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /archive/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI","archive_enable_hot":false,"duree_hot":0,"archive_enable_cold":false,"duree_cold":0}')
assert_http_status 403 "POST /archive/set readonly → HTTP 403"

# Restaurer
api_call POST /archive/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI","archive_enable_hot":false,"duree_hot":0,"archive_enable_cold":false,"duree_cold":0}' >/dev/null

# =============================================================================
# TEST: POST /archive/rebuild - Reconstruire l'archive d'un mnémo
# =============================================================================
log_info "Test: POST /archive/rebuild"
RESPONSE=$(api_call POST /archive/rebuild "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /archive/rebuild → HTTP 200"
else
    _test_fail "POST /archive/rebuild" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /archive/move_hot_to_cold
# =============================================================================
log_info "Test: POST /archive/move_hot_to_cold"
RESPONSE=$(api_call POST /archive/move_hot_to_cold "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /archive/move_hot_to_cold → HTTP 200"
else
    _test_fail "POST /archive/move_hot_to_cold" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /archive/get - Récupérer des données archivées
# =============================================================================
log_info "Test: POST /archive/get"
NOW=$(date +%s)
YESTERDAY=$((NOW - 86400))
RESPONSE=$(api_call POST /archive/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"tech_id\":\"TEST_DLS\",\"acronyme\":\"TEST_AI\",\"date_start\":${YESTERDAY},\"date_end\":${NOW}}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /archive/get → HTTP 200"
else
    _test_fail "POST /archive/get" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: DELETE /archive/delete - Supprimer les archives d'un mnémo
# =============================================================================
log_info "Test: DELETE /archive/delete"
RESPONSE=$(api_call DELETE /archive/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "DELETE /archive/delete → HTTP 200"
else
    _test_fail "DELETE /archive/delete" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: DELETE /archive/delete_old_cold - Purger les vieilles archives froides
# =============================================================================
log_info "Test: DELETE /archive/delete_old_cold"
RESPONSE=$(api_call DELETE /archive/delete_old_cold "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"TEST_AI"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "DELETE /archive/delete_old_cold → HTTP 200"
else
    _test_fail "DELETE /archive/delete_old_cold" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

print_suite_summary "Suite 14 - Archive"
[[ ${TESTS_FAILED} -eq 0 ]]
