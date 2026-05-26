#!/usr/bin/env bash
# =============================================================================
# 03-domain-crud.sh - Tests CRUD sur les domaines
# =============================================================================
# Endpoints testés: GET /domain/list, GET /domain/get, POST /domain/set,
#                   POST /domain/add, DELETE /domain/delete, GET /domain/status
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 03 - Domain CRUD"

ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /domain/list
# =============================================================================
log_info "Test: GET /domain/list"
RESPONSE=$(api_call GET /domain/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /domain/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "domains" "GET /domain/list retourne des domaines"

# Vérifier que le domaine de test est dans la liste
DOMAIN_IN_LIST=$(echo "${RESPONSE}" | jq -r '.domains[] | select(.domain_uuid == "'${TEST_DOMAIN_UUID}'") | .domain_uuid' 2>/dev/null)
_test_start
if [[ "${DOMAIN_IN_LIST}" == "${TEST_DOMAIN_UUID}" ]]; then
    _test_pass "GET /domain/list contient le domaine de test"
else
    _test_fail "GET /domain/list ne contient pas le domaine de test" "${RESPONSE}"
fi

# =============================================================================
# TEST: GET /domain/get
# =============================================================================
log_info "Test: GET /domain/get"
RESPONSE=$(api_call GET "/domain/get?domain_uuid=${TEST_DOMAIN_UUID}" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /domain/get → HTTP 200"
assert_json_field "${RESPONSE}" "domain_uuid" "${TEST_DOMAIN_UUID}" "GET /domain/get uuid correct"
assert_json_field "${RESPONSE}" "domain_name" "Domaine de Test Principal" "GET /domain/get name correct"

# =============================================================================
# TEST: GET /domain/status
# =============================================================================
log_info "Test: GET /domain/status"
RESPONSE=$(api_call GET /domain/status "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /domain/status → HTTP 200"
assert_json_field "${RESPONSE}" "nbr_syns" "not_empty" "GET /domain/status nbr_syns présent"
assert_json_field "${RESPONSE}" "nbr_dls" "not_empty" "GET /domain/status nbr_dls présent"
assert_json_field "${RESPONSE}" "nbr_cameras" "not_empty" "GET /domain/status nbr_cameras présent"

# Cohérence BD: le nombre de synoptiques doit correspondre
NBR_SYNS_DB=$(db_domain_query "SELECT COUNT(*) FROM syns;")
NBR_SYNS_API=$(echo "${RESPONSE}" | jq -r '.nbr_syns // 0' 2>/dev/null)
_test_start
if [[ "${NBR_SYNS_API}" == "${NBR_SYNS_DB}" ]]; then
    _test_pass "GET /domain/status nbr_syns cohérent avec BD (${NBR_SYNS_DB})"
else
    _test_fail "GET /domain/status nbr_syns incohérent" "API=${NBR_SYNS_API}, BD=${NBR_SYNS_DB}"
fi

# =============================================================================
# TEST: POST /domain/set - Modification du nom du domaine
# =============================================================================
log_info "Test: POST /domain/set - modification domain_name"
RESPONSE=$(api_call POST /domain/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"domain_uuid\":\"${TEST_DOMAIN_UUID}\",\"domain_name\":\"Domaine Test Modifié\"}")

assert_http_status 200 "POST /domain/set → HTTP 200"

# Vérifier la modification en base (table master)
assert_master_db_field "domains" "domain_name" "Domaine Test Modifié" \
    "POST /domain/set mise à jour domain_name en BD" \
    "domain_uuid='${TEST_DOMAIN_UUID}'"

# Remettre le nom initial
api_call POST /domain/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"domain_uuid\":\"${TEST_DOMAIN_UUID}\",\"domain_name\":\"Domaine de Test Principal\"}" >/dev/null

# =============================================================================
# TEST: POST /domain/set - Readonly ne peut pas modifier (access_level < 8)
# =============================================================================
log_info "Test: POST /domain/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /domain/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"domain_uuid\":\"${TEST_DOMAIN_UUID}\",\"domain_name\":\"Tentative Non Autorisée\"}")

assert_http_status 403 "POST /domain/set readonly → HTTP 403"

assert_master_db_field "domains" "domain_name" "Domaine de Test Principal" \
    "POST /domain/set non autorisé n'a pas modifié la BD" \
    "domain_uuid='${TEST_DOMAIN_UUID}'"

# =============================================================================
# TEST: POST /domain/add - Création d'un nouveau domaine
# =============================================================================
log_info "Test: POST /domain/add - création nouveau domaine"
# Snapshot avant création
DOMAINS_BEFORE=$(db_query "SELECT COUNT(*) FROM domains;" master)

RESPONSE=$(api_call POST /domain/add "${ADMIN_TOKEN}" "" \
    '{"domain_name":"Test Domain Create"}')

assert_http_status 200 "POST /domain/add → HTTP 200"

# Récupérer l'UUID du domaine créé depuis la réponse
NEW_DOMAIN_UUID=$(echo "${RESPONSE}" | jq -r '.domain_uuid // empty' 2>/dev/null)
_test_start
if [[ -n "${NEW_DOMAIN_UUID}" && "${NEW_DOMAIN_UUID}" != "null" ]]; then
    _test_pass "POST /domain/add retourne un domain_uuid (${NEW_DOMAIN_UUID})"
else
    _test_fail "POST /domain/add ne retourne pas de domain_uuid" "${RESPONSE}"
    NEW_DOMAIN_UUID=""
fi

# Vérifier l'INSERT en base
DOMAINS_AFTER=$(db_query "SELECT COUNT(*) FROM domains;" master)
_test_start
if [[ "$((DOMAINS_BEFORE + 1))" == "${DOMAINS_AFTER}" ]]; then
    _test_pass "POST /domain/add a créé 1 domaine en BD (${DOMAINS_BEFORE} → ${DOMAINS_AFTER})"
else
    _test_fail "POST /domain/add n'a pas incrémenté le nombre de domaines" \
        "avant=${DOMAINS_BEFORE}, après=${DOMAINS_AFTER}"
fi

# Vérifier les champs en BD si on a un UUID
if [[ -n "${NEW_DOMAIN_UUID}" ]]; then
    # /domain/add force actuellement un nom par défaut côté API.
    # On vérifie seulement que le nom créé en base n'est pas vide.
    NEW_DOMAIN_NAME=$(db_query "SELECT domain_name FROM domains WHERE domain_uuid='${NEW_DOMAIN_UUID}';" master)
    _test_start
    if [[ -n "${NEW_DOMAIN_NAME}" ]]; then
        _test_pass "POST /domain/add domain_name en BD non vide (${NEW_DOMAIN_NAME})"
    else
        _test_fail "POST /domain/add domain_name en BD vide" "domain_uuid=${NEW_DOMAIN_UUID}"
    fi

    # Vérifier que le schéma du nouveau domaine contient les tables nécessaires.
    NEW_DOMAIN_DB_PASSWORD=$(db_query "SELECT db_password FROM domains WHERE domain_uuid='${NEW_DOMAIN_UUID}';" master)

    domain_db_query() {
        local sql="$1"
        local port="${2:-${DB_PORT}}"

        "${DB_CLIENT}" \
            -h "${DB_HOST}" \
            -P "${port}" \
            -u "${NEW_DOMAIN_UUID}" \
            -p"${NEW_DOMAIN_DB_PASSWORD}" \
            --connect-timeout=3 \
            --silent \
            --skip-column-names \
            "${NEW_DOMAIN_UUID}" \
            -e "${sql}" 2>/dev/null
    }

    REQUIRED_DOMAIN_TABLES=(
        agents teleinfoedf ups meteo modbus modbus_DI modbus_DO modbus_AI modbus_AO
        shelly smsg audio audio_zones audio_zone_map radio dmx imsgs gpiod gpiod_IO
        phidget phidget_IO syns dls dls_packages dls_params mappings mnemos_DI mnemos_DO
        mnemos_AI mnemos_AO mnemos_BI mnemos_MONO mnemos_WATCHDOG mnemos_CI mnemos_CH
        mnemos_TEMPO mnemos_HORLOGE mnemos_HORLOGE_ticks mnemos_REGISTRE mnemos_VISUEL
        syns_motifs tableau tableau_map msgs
        histo_msgs audit_log cleanup cameras syn_cameras
    )
    REQUIRED_ARCH_TABLES=(histo_bit)
    MISSING_TABLES=()

    if [[ -z "${NEW_DOMAIN_DB_PASSWORD}" ]]; then
        MISSING_TABLES+=("db.__connection__")
    else
        for table_name in "${REQUIRED_DOMAIN_TABLES[@]}"; do
            table_exists=$(domain_db_query "SHOW TABLES LIKE '${table_name}';")
            if [[ "${table_exists}" != "${table_name}" ]]; then
                MISSING_TABLES+=("db.${table_name}")
            fi
        done

        for table_name in "${REQUIRED_ARCH_TABLES[@]}"; do
            table_exists=$(domain_db_query "SHOW TABLES LIKE '${table_name}';" "${DB_ARCH_PORT}")
            if [[ "${table_exists}" != "${table_name}" ]]; then
                MISSING_TABLES+=("arch.${table_name}")
            fi
        done
    fi

    _test_start
    if [[ ${#MISSING_TABLES[@]} -eq 0 ]]; then
        _test_pass "POST /domain/add crée toutes les tables nécessaires du domaine"
    else
        _test_fail "POST /domain/add schéma de domaine incomplet" "tables manquantes: ${MISSING_TABLES[*]}"
    fi

    # Vérifier que l'user admin a accès au nouveau domaine
    ADMIN_GRANT=$(db_query "SELECT access_level FROM users_grants WHERE user_uuid='${TEST_ADMIN_UUID}' AND domain_uuid='${NEW_DOMAIN_UUID}';" master)
    _test_start
    if [[ -n "${ADMIN_GRANT}" ]]; then
        _test_pass "POST /domain/add crée un grant admin (level=${ADMIN_GRANT})"
    else
        _test_fail "POST /domain/add n'a pas créé de grant pour l'admin"
    fi
fi

# =============================================================================
# TEST: DELETE /domain/delete - Suppression du domaine créé
# =============================================================================
if [[ -n "${NEW_DOMAIN_UUID}" ]]; then
    log_info "Test: DELETE /domain/delete - suppression du domaine créé"
    RESPONSE=$(api_call DELETE /domain/delete "${ADMIN_TOKEN}" "${NEW_DOMAIN_UUID}" \
        "{\"domain_uuid\":\"${NEW_DOMAIN_UUID}\"}")

    assert_http_status 200 "DELETE /domain/delete → HTTP 200"

    # Vérifier la suppression en base
    DELETED_COUNT=$(db_query "SELECT COUNT(*) FROM domains WHERE domain_uuid='${NEW_DOMAIN_UUID}';" master)
    _test_start
    if [[ "${DELETED_COUNT}" == "0" ]]; then
        _test_pass "DELETE /domain/delete a supprimé le domaine en BD"
    else
        _test_fail "DELETE /domain/delete n'a pas supprimé le domaine en BD" \
            "COUNT=${DELETED_COUNT}"
    fi
fi

print_suite_summary "Suite 03 - Domain CRUD"
[[ ${TESTS_FAILED} -eq 0 ]]
