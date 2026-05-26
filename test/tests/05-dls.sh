#!/usr/bin/env bash
# =============================================================================
# 05-dls.sh - Tests des endpoints DLS (programmes logiques)
# =============================================================================
# Endpoints testés: GET /dls/list, GET /dls/source, POST /dls/set,
#                   POST /dls/enable, POST /dls/delete, POST /dls/params
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
assert_json_array_not_empty "${RESPONSE}" "dls" "GET /dls/list retourne des programmes"

# Cohérence avec la BD
DLS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM dls;")
DLS_IN_API=$(echo "${RESPONSE}" | jq '.dls | length' 2>/dev/null)
_test_start
if [[ "${DLS_IN_API}" == "${DLS_IN_DB}" ]]; then
    _test_pass "GET /dls/list nombre cohérent avec BD (${DLS_IN_DB})"
else
    _test_fail "GET /dls/list nombre incohérent" "API=${DLS_IN_API}, BD=${DLS_IN_DB}"
fi

# Le DLS de test doit être présent
TEST_DLS_FOUND=$(echo "${RESPONSE}" | jq -r '.dls[] | select(.tech_id == "TEST_DLS") | .tech_id' 2>/dev/null)
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
# TEST: POST /dls/set - Modification meta avec paramètres obligatoires
# =============================================================================
log_info "Test: POST /dls/set - modification metadata avec paramètres obligatoires"

CURRENT_DLS_ID=$(db_domain_query "SELECT dls_id FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
CURRENT_SYN_ID=$(db_domain_query "SELECT syn_id FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
CURRENT_NAME=$(db_domain_query "SELECT name FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
CURRENT_SHORTNAME=$(db_domain_query "SELECT shortname FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
CURRENT_PACKAGE=$(db_domain_query "SELECT package FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")

# Evite une erreur jq --argjson si la base ne renvoie pas de valeur
[[ -z "${CURRENT_DLS_ID}" ]] && CURRENT_DLS_ID=0
[[ -z "${CURRENT_SYN_ID}" ]] && CURRENT_SYN_ID=0

UPDATED_NAME="${CURRENT_NAME} (test set)"
UPDATED_SHORTNAME="${CURRENT_SHORTNAME}_set"

SET_PAYLOAD=$(jq -cn \
    --argjson dls_id "${CURRENT_DLS_ID}" \
    --arg tech_id "TEST_DLS" \
    --argjson syn_id "${CURRENT_SYN_ID}" \
    --arg name "${UPDATED_NAME}" \
    --arg shortname "${UPDATED_SHORTNAME}" \
    --arg package "${CURRENT_PACKAGE}" \
    '{"dls_id":$dls_id,"tech_id":$tech_id,"syn_id":$syn_id,"name":$name,"shortname":$shortname,"package":$package}')

RESPONSE=$(api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${SET_PAYLOAD}")

assert_http_status 200 "POST /dls/set → HTTP 200"

# Vérifier la mise à jour des champs meta en BD
NAME_UPDATED=$(db_domain_query "SELECT name FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
SHORTNAME_UPDATED=$(db_domain_query "SELECT shortname FROM dls WHERE tech_id='TEST_DLS' LIMIT 1;")
_test_start
if [[ "${NAME_UPDATED}" == "${UPDATED_NAME}" && "${SHORTNAME_UPDATED}" == "${UPDATED_SHORTNAME}" ]]; then
    _test_pass "POST /dls/set metadonnées mises à jour en BD"
else
    _test_fail "POST /dls/set metadonnées non mises à jour en BD" \
        "name attendu='${UPDATED_NAME}', actual='${NAME_UPDATED}' ; shortname attendu='${UPDATED_SHORTNAME}', actual='${SHORTNAME_UPDATED}'"
fi

# Restaurer les champs initiaux
RESTORE_PAYLOAD=$(jq -cn \
    --argjson dls_id "${CURRENT_DLS_ID}" \
    --arg tech_id "TEST_DLS" \
    --argjson syn_id "${CURRENT_SYN_ID}" \
    --arg name "${CURRENT_NAME}" \
    --arg shortname "${CURRENT_SHORTNAME}" \
    --arg package "${CURRENT_PACKAGE}" \
    '{"dls_id":$dls_id,"tech_id":$tech_id,"syn_id":$syn_id,"name":$name,"shortname":$shortname,"package":$package}')

RESPONSE=$(api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${RESTORE_PAYLOAD}")
assert_http_status 200 "POST /dls/set restore → HTTP 200"

# Vérifier qu'un payload incomplet est refusé
log_info "Test: POST /dls/set - paramètres obligatoires manquants"
RESPONSE=$(api_call POST /dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS"}')

assert_http_status 400 "POST /dls/set incomplet → HTTP 400"

# =============================================================================
# TEST: POST /dls/enable - Modification enable/disable
# =============================================================================
log_info "Test: POST /dls/enable - activation du DLS"
RESPONSE=$(api_call POST /dls/enable "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","enable":true}')

assert_http_status 200 "POST /dls/enable enable → HTTP 200"
assert_db_field "dls" "enable" "1" \
    "POST /dls/enable enable=1 en BD" "tech_id='TEST_DLS'"

# Désactiver
RESPONSE=$(api_call POST /dls/enable "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tech_id":"TEST_DLS","enable":false}')

assert_http_status 200 "POST /dls/enable disable → HTTP 200"
assert_db_field "dls" "enable" "0" \
    "POST /dls/enable enable=0 en BD" "tech_id='TEST_DLS'"

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
