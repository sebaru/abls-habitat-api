#!/usr/bin/env bash
# =============================================================================
# 17-tableau.sh - Tests des endpoints Tableau de bord
# =============================================================================
# Endpoints testés: GET /tableau/list, GET /tableau/map/list,
#                   POST /tableau/set, POST /tableau/map/set,
#                   POST /tableau/map/add,
#                   DELETE /tableau/delete, DELETE /tableau/map/delete
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 17 - Tableau de bord"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /tableau/list
# =============================================================================
log_info "Test: GET /tableau/list"
RESPONSE=$(api_call GET /tableau/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /tableau/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "tableaux" "GET /tableau/list retourne des tableaux"

TAB_FOUND=$(echo "${RESPONSE}" | jq -r '.tableaux[] | select(.tableau_nom == "Test-Tableau-01") | .tableau_nom' 2>/dev/null | head -1)
_test_start
if [[ "${TAB_FOUND}" == "Test-Tableau-01" ]]; then
    _test_pass "GET /tableau/list contient Test-Tableau-01"
else
    _test_fail "GET /tableau/list ne contient pas Test-Tableau-01" "${RESPONSE}"
fi

log_info "Test: GET /tableau/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /tableau/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /tableau/list readonly → HTTP 403"

# Récupérer l'ID du tableau de test
TABLEAU_ID=$(db_domain_query \
    "SELECT tableau_id FROM tableau WHERE tableau_nom='Test-Tableau-01' LIMIT 1;")

# =============================================================================
# TEST: GET /tableau/map/list
# =============================================================================
log_info "Test: GET /tableau/map/list?tableau_id=${TABLEAU_ID}"
RESPONSE=$(api_call GET "/tableau/map/list?tableau_id=${TABLEAU_ID}" \
    "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /tableau/map/list → HTTP 200"

# =============================================================================
# TEST: POST /tableau/set - Modifier le nom du tableau
# =============================================================================
log_info "Test: POST /tableau/set - modification nom tableau"
RESPONSE=$(api_call POST /tableau/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"tableau_id\":${TABLEAU_ID},\"tableau_nom\":\"Test-Tableau-01-Modifié\",\"couleur\":\"#FFFFFF\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /tableau/set → HTTP 200"
else
    _test_fail "POST /tableau/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

TAB_NAME=$(db_domain_query \
    "SELECT tableau_nom FROM tableau WHERE tableau_id=${TABLEAU_ID} LIMIT 1;")
_test_start
if [[ "${TAB_NAME}" == "Test-Tableau-01-Modifié" ]]; then
    _test_pass "POST /tableau/set: nom mis à jour en BD"
else
    _test_fail "POST /tableau/set: nom non mis à jour en BD" "BD='${TAB_NAME}'"
fi

api_call POST /tableau/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"tableau_id\":${TABLEAU_ID},\"tableau_nom\":\"Test-Tableau-01\",\"couleur\":\"#FFFFFF\"}" >/dev/null

log_info "Test: POST /tableau/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /tableau/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"tableau_id\":${TABLEAU_ID},\"tableau_nom\":\"Tentative\",\"couleur\":\"#000000\"}")
assert_http_status 403 "POST /tableau/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /tableau/map/add - Ajouter une entrée dans le tableau
# =============================================================================
log_info "Test: POST /tableau/map/add - ajout TEST_DLS/TEST_DI"
MAP_CNT_BEFORE=$(db_domain_query \
    "SELECT COUNT(*) FROM tableau_map WHERE tableau_id=${TABLEAU_ID};")

RESPONSE=$(api_call POST /tableau/map/add "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"tableau_id\":${TABLEAU_ID},\"tech_id\":\"TEST_DLS\",\"acronyme\":\"TEST_DI\"}")

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /tableau/map/add → HTTP 200"
else
    _test_fail "POST /tableau/map/add" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

MAP_CNT_AFTER=$(db_domain_query \
    "SELECT COUNT(*) FROM tableau_map WHERE tableau_id=${TABLEAU_ID};")
_test_start
if [[ "${MAP_CNT_AFTER}" -gt "${MAP_CNT_BEFORE}" ]]; then
    _test_pass "POST /tableau/map/add: ligne ajoutée en BD"
else
    _test_fail "POST /tableau/map/add: aucune ligne ajoutée en BD" \
        "avant=${MAP_CNT_BEFORE}, après=${MAP_CNT_AFTER}"
fi

# Récupérer l'ID de la nouvelle entrée
MAP_ID=$(db_domain_query \
    "SELECT tableau_map_id FROM tableau_map WHERE tableau_id=${TABLEAU_ID} AND tech_id='TEST_DLS' AND acronyme='TEST_DI' LIMIT 1;")

# =============================================================================
# TEST: POST /tableau/map/set - Modifier l'ordre de l'entrée
# =============================================================================
if [[ -n "${MAP_ID}" && "${MAP_ID}" != "NULL" ]]; then
    log_info "Test: POST /tableau/map/set - modification ordre map_id=${MAP_ID}"
    RESPONSE=$(api_call POST /tableau/map/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"tableau_map_id\":${MAP_ID},\"ordre\":5}")

    _test_start
    if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
        _test_pass "POST /tableau/map/set → HTTP 200"
    else
        _test_fail "POST /tableau/map/set" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
    fi
fi

# =============================================================================
# TEST: DELETE /tableau/map/delete - Supprimer l'entrée ajoutée
# =============================================================================
if [[ -n "${MAP_ID}" && "${MAP_ID}" != "NULL" ]]; then
    log_info "Test: DELETE /tableau/map/delete - suppression map_id=${MAP_ID}"
    RESPONSE=$(api_call DELETE /tableau/map/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"tableau_map_id\":${MAP_ID}}")

    assert_http_status 200 "DELETE /tableau/map/delete → HTTP 200"

    MAP_ABSENT=$(db_domain_query \
        "SELECT COUNT(*) FROM tableau_map WHERE tableau_map_id=${MAP_ID};")
    _test_start
    if [[ "${MAP_ABSENT}" == "0" ]]; then
        _test_pass "DELETE /tableau/map/delete: entrée supprimée de la BD"
    else
        _test_fail "DELETE /tableau/map/delete: entrée encore présente en BD" "count=${MAP_ABSENT}"
    fi
fi

# =============================================================================
# TEST: DELETE /tableau/delete - Créer et supprimer un tableau temporaire
# =============================================================================
log_info "Test: DELETE /tableau/delete - création et suppression tableau temporaire"
RESPONSE=$(api_call POST /tableau/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tableau_nom":"Tableau-Temp-Del","couleur":"#AABBCC"}')

TAB_TEMP_ID=$(db_domain_query \
    "SELECT tableau_id FROM tableau WHERE tableau_nom='Tableau-Temp-Del' LIMIT 1;")

if [[ -n "${TAB_TEMP_ID}" && "${TAB_TEMP_ID}" != "NULL" ]]; then
    TAB_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM tableau;")
    RESPONSE=$(api_call DELETE /tableau/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"tableau_id\":${TAB_TEMP_ID}}")

    assert_http_status 200 "DELETE /tableau/delete → HTTP 200"

    TAB_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM tableau;")
    _test_start
    if [[ "$((TAB_CNT_BEFORE - 1))" == "${TAB_CNT_AFTER}" ]]; then
        _test_pass "DELETE /tableau/delete: tableau supprimé de la BD"
    else
        _test_fail "DELETE /tableau/delete: compteur incohérent" \
            "avant=${TAB_CNT_BEFORE}, après=${TAB_CNT_AFTER}"
    fi
else
    log_info "SKIP: Tableau temporaire non créé, test DELETE/tableau/delete ignoré"
fi

print_suite_summary "Suite 17 - Tableau de bord"
[[ ${TESTS_FAILED} -eq 0 ]]
