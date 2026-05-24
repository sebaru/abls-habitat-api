#!/usr/bin/env bash
# =============================================================================
# 05-dls.sh - Tests des endpoints DLS (programmes logiques)
# =============================================================================
# Endpoints testés: GET /dls/list, GET /dls/source, POST /dls/set,
#                   POST /dls/delete, POST /dls/params
#
# Droits requis: access_level ≥ 6
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 05 - DLS (programmes logiques)"

ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /dls/list
# =============================================================================
log_info "Test: GET /dls/list"
RESPONSE=$(api_call GET /dls/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /dls/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "DLSs" "GET /dls/list retourne des programmes"

# Cohérence avec la BD
DLS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM dls;")
DLS_IN_API=$(echo "${RESPONSE}" | jq '.DLSs | length' 2>/dev/null)
_test_start
if [[ "${DLS_IN_API}" == "${DLS_IN_DB}" ]]; then
    _test_pass "GET /dls/list nombre cohérent avec BD (${DLS_IN_DB})"
else
    _test_fail "GET /dls/list nombre incohérent" "API=${DLS_IN_API}, BD=${DLS_IN_DB}"
fi

# Le DLS de test doit être présent
TEST_DLS_FOUND=$(echo "${RESPONSE}" | jq -r '.DLSs[] | select(.tech_id == "TEST_DLS") | .tech_id' 2>/dev/null)
_test_start
if [[ "${TEST_DLS_FOUND}" == "TEST_DLS" ]]; then
    _test_pass "GET /dls/list contient TEST_DLS"
else
    _test_fail "GET /dls/list ne contient pas TEST_DLS (tech_id)" "${RESPONSE}"
fi

# =============================================================================
# TEST: GET /dls/list - readonly (access insuffisant)
# =============================================================================
log_info "Test: GET /dls/list - readonly"
RESPONSE=$(api_call GET /dls/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /dls/list readonly → HTTP 403"

# =============================================================================
# TEST: GET /dls/source - Récupérer le code source
# =============================================================================
log_info "Test: GET /dls/source?tech_id=TEST_DLS"
RESPONSE=$(api_call GET "/dls/source?tech_id=TEST_DLS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /dls/source → HTTP 200"
assert_json_field "${RESPONSE}" "tech_id" "TEST_DLS" "GET /dls/source tech_id correct"
assert_json_field "${RESPONSE}" "sourcecode" "not_empty" "GET /dls/source contient sourcecode"

# Cohérence avec la BD
SOURCE_DB=$(db_domain_query "SELECT sourcecode FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
SOURCE_API=$(echo "${RESPONSE}" | jq -r '.sourcecode // empty' 2>/dev/null)
_test_start
if [[ "${SOURCE_API}" == "${SOURCE_DB}" ]]; then
    _test_pass "GET /dls/source cohérent avec BD"
else
    _test_fail "GET /dls/source incohérent avec BD" \
        "API='${SOURCE_API}', BD='${SOURCE_DB}'"
fi

# =============================================================================
# TEST: POST /dls/set - Modification du sourcecode
# =============================================================================
log_info "Test: POST /dls/set - modification sourcecode"
NEW_SOURCE="/* Code mis à jour par test fonctionnel */"
RESPONSE=$(api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"tech_id\":\"TEST_DLS\",\"sourcecode\":\"${NEW_SOURCE}\"}")

assert_http_status 200 "POST /dls/set → HTTP 200"

# Vérifier la mise à jour en BD
SOURCE_UPDATED=$(db_domain_query "SELECT sourcecode FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
_test_start
if [[ "${SOURCE_UPDATED}" == "${NEW_SOURCE}" ]]; then
    _test_pass "POST /dls/set sourcecode mis à jour en BD"
else
    _test_fail "POST /dls/set sourcecode non mis à jour en BD" \
        "attendu='${NEW_SOURCE}', actual='${SOURCE_UPDATED}'"
fi

# Restaurer le sourcecode initial
api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","sourcecode":"/* Programme de test fonctionnel */"}' >/dev/null

# =============================================================================
# TEST: POST /dls/set - Modification enable/disable
# =============================================================================
log_info "Test: POST /dls/set - activation du DLS"
RESPONSE=$(api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","enable":1}')

assert_http_status 200 "POST /dls/set enable → HTTP 200"
assert_db_field "dls" "enable" "1" \
    "POST /dls/set enable=1 en BD" "tech_id='TEST_DLS'"

# Désactiver
RESPONSE=$(api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","enable":0}')

assert_http_status 200 "POST /dls/set disable → HTTP 200"
assert_db_field "dls" "enable" "0" \
    "POST /dls/set enable=0 en BD" "tech_id='TEST_DLS'"

# =============================================================================
# TEST: POST /dls/params - Paramètres d'un DLS
# =============================================================================
log_info "Test: POST /dls/params - ajout paramètre"
RESPONSE=$(api_call POST /dls/params "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","acronyme":"PARAM_TEST","libelle":"Paramètre de test","valeur":"42"}')

assert_http_status 200 "POST /dls/params add → HTTP 200"

# Vérifier l'INSERT en BD
assert_db_row_exists "dls_params" \
    "POST /dls/params: paramètre créé en BD" \
    "tech_id='TEST_DLS' AND acronyme='PARAM_TEST'"

PARAM_VALUE=$(db_domain_query "SELECT valeur FROM dls_params WHERE tech_id='TEST_DLS' AND acronyme='PARAM_TEST' LIMIT 1;")
_test_start
if [[ "${PARAM_VALUE}" == "42" ]]; then
    _test_pass "POST /dls/params valeur correcte en BD ('42')"
else
    _test_fail "POST /dls/params valeur incorrecte en BD" "attendu='42', actual='${PARAM_VALUE}'"
fi

# =============================================================================
# TEST: GET /dls/params - Lecture des paramètres
# =============================================================================
log_info "Test: GET /dls/params?tech_id=TEST_DLS"
RESPONSE=$(api_call GET "/dls/params?tech_id=TEST_DLS" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /dls/params → HTTP 200"
PARAM_IN_API=$(echo "${RESPONSE}" | jq -r '.params[] | select(.acronyme == "PARAM_TEST") | .valeur' 2>/dev/null)
_test_start
if [[ "${PARAM_IN_API}" == "42" ]]; then
    _test_pass "GET /dls/params retourne PARAM_TEST avec valeur '42'"
else
    _test_fail "GET /dls/params ne retourne pas PARAM_TEST" "${RESPONSE}"
fi

# =============================================================================
# TEST: POST /dls/delete - Vérification que SYS n'est pas supprimable
# =============================================================================
log_info "Test: POST /dls/delete - système DLS (SYS, ne doit pas être supprimable)"
DLS_COUNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM dls;")
RESPONSE=$(api_call POST /dls/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"SYS"}')

# L'API doit refuser la suppression du DLS système (non deletable)
DLS_COUNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM dls;")
_test_start
if [[ "${DLS_COUNT_BEFORE}" == "${DLS_COUNT_AFTER}" ]]; then
    _test_pass "POST /dls/delete SYS n'a pas supprimé le DLS système (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "POST /dls/delete SYS a supprimé le DLS système!" \
        "avant=${DLS_COUNT_BEFORE}, après=${DLS_COUNT_AFTER}"
fi

print_suite_summary "Suite 05 - DLS"
[[ ${TESTS_FAILED} -eq 0 ]]
