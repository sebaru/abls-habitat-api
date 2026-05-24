#!/usr/bin/env bash
# =============================================================================
# 06-data-integrity.sh - Vérifications d'intégrité de la base de données
# =============================================================================
# Vérifie:
# - Cohérence des clés étrangères
# - Timestamps valides (created_at <= updated_at)
# - Pas de lignes orphelines
# - Cohérence entre API et BD pour les endpoints de listing
# - Accès aux tables par niveau d'accès correct
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 06 - Intégrité des données"

ADMIN_TOKEN=$(make_admin_token)

# =============================================================================
# TEST: Cohérence users_grants ↔ users et domains (master BD)
# =============================================================================
log_info "Test: FK users_grants → users (pas d'orphelins)"
ORPHAN_GRANTS=$(db_query \
    "SELECT COUNT(*) FROM users_grants ug LEFT JOIN users u USING(user_uuid) WHERE u.user_uuid IS NULL;" \
    abls_master)
_test_start
if [[ "${ORPHAN_GRANTS}" == "0" ]]; then
    _test_pass "users_grants: aucun user_uuid orphelin"
else
    _test_fail "users_grants: ${ORPHAN_GRANTS} lignes avec user_uuid orphelin"
fi

log_info "Test: FK users_grants → domains (pas d'orphelins)"
ORPHAN_DOMAIN_GRANTS=$(db_query \
    "SELECT COUNT(*) FROM users_grants ug LEFT JOIN domains d USING(domain_uuid) WHERE d.domain_uuid IS NULL;" \
    abls_master)
_test_start
if [[ "${ORPHAN_DOMAIN_GRANTS}" == "0" ]]; then
    _test_pass "users_grants: aucun domain_uuid orphelin"
else
    _test_fail "users_grants: ${ORPHAN_DOMAIN_GRANTS} lignes avec domain_uuid orphelin"
fi

# =============================================================================
# TEST: Cohérence users.default_domain_uuid → domains
# =============================================================================
log_info "Test: FK users.default_domain_uuid → domains"
ORPHAN_DEFAULT=$(db_query \
    "SELECT COUNT(*) FROM users u LEFT JOIN domains d ON u.default_domain_uuid=d.domain_uuid WHERE u.default_domain_uuid IS NOT NULL AND d.domain_uuid IS NULL;" \
    abls_master)
_test_start
if [[ "${ORPHAN_DEFAULT}" == "0" ]]; then
    _test_pass "users.default_domain_uuid: aucune FK cassée"
else
    _test_fail "users.default_domain_uuid: ${ORPHAN_DEFAULT} FK cassée(s)"
fi

# =============================================================================
# TEST: Cohérence dls → syns (domain BD)
# =============================================================================
log_info "Test: FK dls → syns (domain)"
ORPHAN_DLS=$(db_domain_query \
    "SELECT COUNT(*) FROM dls d LEFT JOIN syns s ON d.syn_id=s.syn_id WHERE s.syn_id IS NULL;")
_test_start
if [[ "${ORPHAN_DLS}" == "0" ]]; then
    _test_pass "dls: aucun syn_id orphelin"
else
    _test_fail "dls: ${ORPHAN_DLS} enregistrement(s) avec syn_id orphelin"
fi

# =============================================================================
# TEST: DLS système obligatoire présent
# =============================================================================
log_info "Test: DLS SYS (système) existe dans le domaine"
SYS_COUNT=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id='SYS';")
_test_start
if [[ "${SYS_COUNT}" == "1" ]]; then
    _test_pass "DLS SYS présent dans le domaine"
else
    _test_fail "DLS SYS absent ou en doublon dans le domaine" "COUNT=${SYS_COUNT}"
fi

# =============================================================================
# TEST: Synoptique Accueil obligatoire présent
# =============================================================================
log_info "Test: Synoptique HOME (accueil) existe"
HOME_SYN=$(db_domain_query "SELECT COUNT(*) FROM syns WHERE page='HOME';")
_test_start
if [[ "${HOME_SYN}" == "1" ]]; then
    _test_pass "Synoptique HOME présent"
else
    _test_fail "Synoptique HOME absent ou en doublon" "COUNT=${HOME_SYN}"
fi

# =============================================================================
# TEST: Timestamps valides pour les caméras (date_create <= NOW())
# =============================================================================
log_info "Test: Timestamps caméras valides"
FUTURE_DATES=$(db_domain_query \
    "SELECT COUNT(*) FROM cameras WHERE date_create > NOW() + INTERVAL 1 MINUTE;")
_test_start
if [[ "${FUTURE_DATES}" == "0" ]]; then
    _test_pass "cameras: tous les date_create sont dans le passé"
else
    _test_fail "cameras: ${FUTURE_DATES} date_create dans le futur (!)"
fi

# =============================================================================
# TEST: Cohérence GET /camera/list ↔ BD (nombres identiques)
# =============================================================================
log_info "Test: Cohérence GET /camera/list ↔ BD"
RESPONSE=$(api_call GET /camera/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
API_COUNT=$(echo "${RESPONSE}" | jq '.cameras | length' 2>/dev/null)
DB_COUNT=$(db_domain_query "SELECT COUNT(*) FROM cameras;")

_test_start
if [[ "${API_COUNT}" == "${DB_COUNT}" ]]; then
    _test_pass "GET /camera/list et BD concordent (${DB_COUNT} caméras)"
else
    _test_fail "GET /camera/list et BD discordent" "API=${API_COUNT}, BD=${DB_COUNT}"
fi

# =============================================================================
# TEST: Tous les camera_id dans l'API correspondent à des lignes en BD
# =============================================================================
log_info "Test: Tous les camera_id de l'API existent en BD"
CAMERA_IDS_API=$(echo "${RESPONSE}" | jq -r '.cameras[].camera_id' 2>/dev/null | sort -n)
_test_start
ALL_VALID=true
for cam_id in ${CAMERA_IDS_API}; do
    EXISTS=$(db_domain_query "SELECT COUNT(*) FROM cameras WHERE camera_id=${cam_id};")
    if [[ "${EXISTS}" != "1" ]]; then
        _test_fail "camera_id=${cam_id} retourné par l'API n'existe pas en BD"
        ALL_VALID=false
    fi
done
if [[ "${ALL_VALID}" == true ]]; then
    _test_pass "Tous les camera_id de l'API existent en BD"
fi

# =============================================================================
# TEST: Cohérence GET /dls/list ↔ BD
# =============================================================================
log_info "Test: Cohérence GET /dls/list ↔ BD"
RESPONSE=$(api_call GET /dls/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
DLS_API_COUNT=$(echo "${RESPONSE}" | jq '.DLSs | length' 2>/dev/null)
DLS_DB_COUNT=$(db_domain_query "SELECT COUNT(*) FROM dls;")

_test_start
if [[ "${DLS_API_COUNT}" == "${DLS_DB_COUNT}" ]]; then
    _test_pass "GET /dls/list et BD concordent (${DLS_DB_COUNT} DLS)"
else
    _test_fail "GET /dls/list et BD discordent" "API=${DLS_API_COUNT}, BD=${DLS_DB_COUNT}"
fi

# =============================================================================
# TEST: access_level des grants dans les plages valides (0-9)
# =============================================================================
log_info "Test: access_level des grants dans la plage valide [0-9]"
INVALID_LEVELS=$(db_query \
    "SELECT COUNT(*) FROM users_grants WHERE access_level < 0 OR access_level > 9;" \
    abls_master)
_test_start
if [[ "${INVALID_LEVELS}" == "0" ]]; then
    _test_pass "Tous les access_level sont dans [0-9]"
else
    _test_fail "users_grants: ${INVALID_LEVELS} access_level hors de [0-9]"
fi

# =============================================================================
# TEST: Unicité des UUIDs de domaines
# =============================================================================
log_info "Test: Unicité des domain_uuid"
DUPLICATE_DOMAINS=$(db_query \
    "SELECT COUNT(*) - COUNT(DISTINCT domain_uuid) FROM domains;" \
    abls_master)
_test_start
if [[ "${DUPLICATE_DOMAINS}" == "0" ]]; then
    _test_pass "domain_uuid: tous uniques"
else
    _test_fail "domains: ${DUPLICATE_DOMAINS} domain_uuid en doublon"
fi

# =============================================================================
# TEST: Cohérence GET /domain/status nbr_cameras ↔ BD
# =============================================================================
log_info "Test: Cohérence GET /domain/status nbr_cameras ↔ BD"
RESPONSE=$(api_call GET /domain/status "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
STATUS_CAM=$(echo "${RESPONSE}" | jq -r '.nbr_cameras // -1' 2>/dev/null)
BD_CAM=$(db_domain_query "SELECT COUNT(*) FROM cameras;")

_test_start
if [[ "${STATUS_CAM}" == "${BD_CAM}" ]]; then
    _test_pass "GET /domain/status nbr_cameras cohérent (${BD_CAM})"
else
    _test_fail "GET /domain/status nbr_cameras incohérent" "API=${STATUS_CAM}, BD=${BD_CAM}"
fi

print_suite_summary "Suite 06 - Data Integrity"
[[ ${TESTS_FAILED} -eq 0 ]]
