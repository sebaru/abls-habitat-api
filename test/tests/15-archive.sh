#!/usr/bin/env bash
# =============================================================================
# 15-archive.sh - Tests des endpoints Archive
# =============================================================================
# Endpoints testés: GET /archive/status/hot, GET /archive/status/cold,
#                   POST /archive/set, POST /archive/rebuild,
#                   POST /archive/move_hot_to_cold, POST /archive/get,
#                   DELETE /archive/delete, DELETE /archive/delete_old_cold
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 15 - Archive"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# La compilation DLS (suite 05) purge les mnémos inutilisés: on recrée TEST_AI.
db_domain_query "INSERT IGNORE INTO mnemos_AI (tech_id, acronyme, libelle, unite) VALUES ('TEST_DLS', 'TEST_AI', 'Mnémo AI de test', 'kWh');" >/dev/null

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
# TEST: POST /archive/set - Rétention des archives du domaine
# =============================================================================
log_info "Test: POST /archive/set - modification de la rétention"
OLD_HOT=$(db_query "SELECT archive_hot_retention FROM domains WHERE domain_uuid='${TEST_DOMAIN_UUID}';" master)
OLD_COLD=$(db_query "SELECT archive_cold_retention FROM domains WHERE domain_uuid='${TEST_DOMAIN_UUID}';" master)
RESPONSE=$(api_call POST /archive/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"archive_hot_retention":30,"archive_cold_retention":5}')

assert_http_status 200 "POST /archive/set → HTTP 200"
assert_master_db_field "domains" "archive_hot_retention" "30" "POST /archive/set: rétention hot en BD" \
    "domain_uuid='${TEST_DOMAIN_UUID}'"
assert_master_db_field "domains" "archive_cold_retention" "5" "POST /archive/set: rétention cold en BD" \
    "domain_uuid='${TEST_DOMAIN_UUID}'"

log_info "Test: POST /archive/set - champ obligatoire manquant"
RESPONSE=$(api_call POST /archive/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"archive_hot_retention":30}')
assert_http_status 400 "POST /archive/set sans archive_cold_retention → HTTP 400"

log_info "Test: POST /archive/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /archive/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"archive_hot_retention":1,"archive_cold_retention":1}')
assert_http_status 403 "POST /archive/set readonly → HTTP 403"

# Restaurer
api_call POST /archive/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"archive_hot_retention\":${OLD_HOT:-18},\"archive_cold_retention\":${OLD_COLD:-2}}" >/dev/null

# =============================================================================
# TEST: POST /archive/rebuild - Reconstruire une partition hot
# =============================================================================
log_info "Test: POST /archive/rebuild - partname invalide"
RESPONSE=$(api_call POST /archive/rebuild "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"partname":"histo_bit"}')
assert_http_status 400 "POST /archive/rebuild partname invalide → HTTP 400"

log_info "Test: POST /archive/rebuild"
RESPONSE=$(api_call POST /archive/rebuild "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"partname":"p_test"}')
assert_http_status 200 "POST /archive/rebuild → HTTP 200"

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
RESPONSE=$(api_call POST /archive/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"period":"BY_HOUR_ON_2_DAYS","courbes":[{"tech_id":"TEST_DLS","acronyme":"TEST_AI","methode":"AVG"}]}')

assert_http_status 200 "POST /archive/get → HTTP 200"
assert_json_field "${RESPONSE}" "courbe1.acronyme" "TEST_AI" "POST /archive/get décrit la courbe demandée"

log_info "Test: POST /archive/get - période inconnue"
RESPONSE=$(api_call POST /archive/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"period":"BY_CENTURY","courbes":[{"tech_id":"TEST_DLS","acronyme":"TEST_AI"}]}')
assert_http_status 400 "POST /archive/get période inconnue → HTTP 400"

# =============================================================================
# TEST: DELETE /archive/delete - Supprimer une table d'archive froide
# =============================================================================
log_info "Test: DELETE /archive/delete - tablename invalide"
RESPONSE=$(api_call DELETE /archive/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tablename":"histo_msgs"}')
assert_http_status 400 "DELETE /archive/delete tablename invalide → HTTP 400"

log_info "Test: DELETE /archive/delete"
db_arch_query "CREATE TABLE IF NOT EXISTS histo_bit_test_delete (id INT);" >/dev/null
RESPONSE=$(api_call DELETE /archive/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"tablename":"histo_bit_test_delete"}')

assert_http_status 200 "DELETE /archive/delete → HTTP 200"
ARCH_TABLE=$(db_arch_query "SHOW TABLES LIKE 'histo_bit_test_delete';")
_test_start
if [[ -z "${ARCH_TABLE}" ]]; then
    _test_pass "DELETE /archive/delete: table supprimée"
else
    _test_fail "DELETE /archive/delete: table encore présente" "${ARCH_TABLE}"
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

print_suite_summary "Suite 15 - Archive"
[[ ${TESTS_FAILED} -eq 0 ]]
