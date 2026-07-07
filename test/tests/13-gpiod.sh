#!/usr/bin/env bash
# =============================================================================
# 13-gpiod.sh - Tests des endpoints GPIOd
# =============================================================================
# Endpoints testés: GET /gpiod/list, POST /gpiod/set, POST /gpiod/set/io
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 13 - GPIOd"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# GPIOd
# =============================================================================

log_info "Test: GET /gpiod/list"
RESPONSE=$(api_call GET /gpiod/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /gpiod/list → HTTP 200"

GPIO_FOUND=$(echo "${RESPONSE}" | jq -r '.gpiod[] | select(.thread_tech_id == "TEST_GPIOD") | .thread_tech_id' 2>/dev/null)
_test_start
if [[ "${GPIO_FOUND}" == "TEST_GPIOD" ]]; then
    _test_pass "GET /gpiod/list contient TEST_GPIOD"
else
    _test_fail "GET /gpiod/list ne contient pas TEST_GPIOD" "${RESPONSE}"
fi

GPIOD_DB=$(db_domain_query "SELECT COUNT(*) FROM gpiod;")
GPIOD_API=$(echo "${RESPONSE}" | jq '.gpiod | length' 2>/dev/null)
_test_start
if [[ "${GPIOD_API}" == "${GPIOD_DB}" ]]; then
    _test_pass "GET /gpiod/list nombre cohérent avec BD (${GPIOD_DB})"
else
    _test_fail "GET /gpiod/list nombre incohérent" "API=${GPIOD_API}, BD=${GPIOD_DB}"
fi

log_info "Test: GET /gpiod/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /gpiod/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /gpiod/list readonly → HTTP 403"

# POST /gpiod/set
log_info "Test: POST /gpiod/set - modification description"
RESPONSE=$(api_call POST /gpiod/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_GPIOD","description":"GPIO modifié","enable":true,"debug":false}')

assert_http_status 200 "POST /gpiod/set → HTTP 200"

GPIO_DESC=$(db_domain_query "SELECT description FROM gpiod WHERE thread_tech_id='TEST_GPIOD' LIMIT 1;")
_test_start
if [[ "${GPIO_DESC}" == "GPIO modifié" ]]; then
    _test_pass "POST /gpiod/set description mise à jour en BD"
else
    _test_fail "POST /gpiod/set description non mise à jour en BD" "BD='${GPIO_DESC}'"
fi

api_call POST /gpiod/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_GPIOD","description":"GPIO de test","enable":true,"debug":false}' >/dev/null

log_info "Test: POST /gpiod/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /gpiod/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_GPIOD","description":"Tentative","enable":true,"debug":false}')
assert_http_status 403 "POST /gpiod/set readonly → HTTP 403"

# POST /gpiod/set/io
log_info "Test: POST /gpiod/set/io - modification libellé GPIO_01"
RESPONSE=$(api_call POST /gpiod/set/io "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"thread_tech_id":"TEST_GPIOD","thread_acronyme":"GPIO_01","num":0,"mode_inout":0,"mode_activelow":false,"libelle":"GPIO IO modifié"}')

assert_http_status 200 "POST /gpiod/set/io → HTTP 200"

GPIO_IO_LIB=$(db_domain_query "SELECT libelle FROM gpiod_IO WHERE thread_tech_id='TEST_GPIOD' AND thread_acronyme='GPIO_01' LIMIT 1;")
_test_start
if [[ "${GPIO_IO_LIB}" == "GPIO IO modifié" ]]; then
    _test_pass "POST /gpiod/set/io libellé mis à jour en BD"
else
    _test_fail "POST /gpiod/set/io libellé non mis à jour en BD" "BD='${GPIO_IO_LIB}'"
fi
db_domain_query "UPDATE gpiod_IO SET libelle='GPIO test 01' WHERE thread_tech_id='TEST_GPIOD' AND thread_acronyme='GPIO_01';" >/dev/null 2>&1 || true

print_suite_summary "Suite 13 - GPIOd"
[[ ${TESTS_FAILED} -eq 0 ]]
