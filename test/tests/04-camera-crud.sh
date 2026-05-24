#!/usr/bin/env bash
# =============================================================================
# 04-camera-crud.sh - Tests CRUD sur les caméras
# =============================================================================
# Endpoints testés: GET /camera/list, POST /camera/add, POST /camera/set,
#                   POST /camera/delete
#
# Droits requis: access_level ≥ 6 (list), ≥ 8 (add/set/delete)
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 04 - Camera CRUD"

ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /camera/list
# =============================================================================
log_info "Test: GET /camera/list - admin"
RESPONSE=$(api_call GET /camera/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /camera/list admin → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "cameras" "GET /camera/list retourne des caméras"

# Vérifier cohérence avec la BD
CAMERAS_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM cameras;")
CAMERAS_IN_API=$(echo "${RESPONSE}" | jq '.cameras | length' 2>/dev/null)
_test_start
if [[ "${CAMERAS_IN_API}" == "${CAMERAS_IN_DB}" ]]; then
    _test_pass "GET /camera/list nombre cohérent avec BD (${CAMERAS_IN_DB})"
else
    _test_fail "GET /camera/list nombre incohérent" "API=${CAMERAS_IN_API}, BD=${CAMERAS_IN_DB}"
fi

# Vérifier que Camera-Test-01 est dans la liste
CAM1_FOUND=$(echo "${RESPONSE}" | jq -r '.cameras[] | select(.name == "Camera-Test-01") | .name' 2>/dev/null)
_test_start
if [[ "${CAM1_FOUND}" == "Camera-Test-01" ]]; then
    _test_pass "GET /camera/list contient Camera-Test-01"
else
    _test_fail "GET /camera/list ne contient pas Camera-Test-01"
fi

# =============================================================================
# TEST: GET /camera/list - user level 6 (autorisé)
# =============================================================================
log_info "Test: GET /camera/list - user level 6"
RESPONSE=$(api_call GET /camera/list "${USER_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /camera/list user level 6 → HTTP 200"

# =============================================================================
# TEST: GET /camera/list - readonly (level 1 < 6 requis)
# =============================================================================
log_info "Test: GET /camera/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /camera/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 403 "GET /camera/list readonly → HTTP 403"

# =============================================================================
# TEST: POST /camera/add - Ajout d'une nouvelle caméra
# =============================================================================
log_info "Test: POST /camera/add - admin"
CAMERAS_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM cameras;")

RESPONSE=$(api_call POST /camera/add "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"name":"Camera-Ajoutee-Test","url":"rtsp://10.0.0.99:554/test","access_level":6,"enable":1}')

assert_http_status 200 "POST /camera/add → HTTP 200"

# Vérifier l'INSERT en base
CAMERAS_AFTER=$(db_domain_query "SELECT COUNT(*) FROM cameras;")
_test_start
if [[ "$((CAMERAS_BEFORE + 1))" == "${CAMERAS_AFTER}" ]]; then
    _test_pass "POST /camera/add a inséré 1 caméra en BD (${CAMERAS_BEFORE} → ${CAMERAS_AFTER})"
else
    _test_fail "POST /camera/add n'a pas incrémenté le nombre de caméras" \
        "avant=${CAMERAS_BEFORE}, après=${CAMERAS_AFTER}"
fi

# Récupérer l'ID de la caméra nouvellement créée
NEW_CAM_ID=$(db_domain_query "SELECT camera_id FROM cameras WHERE name='Camera-Ajoutee-Test' LIMIT 1;")
_test_start
if [[ -n "${NEW_CAM_ID}" ]]; then
    _test_pass "POST /camera/add: caméra trouvée en BD (camera_id=${NEW_CAM_ID})"
else
    _test_fail "POST /camera/add: caméra introuvable en BD" "${RESPONSE}"
fi

# Vérifier les champs en BD
if [[ -n "${NEW_CAM_ID}" ]]; then
    assert_db_field "cameras" "url" "rtsp://10.0.0.99:554/test" \
        "POST /camera/add url correct en BD" \
        "camera_id=${NEW_CAM_ID}"
    assert_db_field "cameras" "access_level" "6" \
        "POST /camera/add access_level correct en BD" \
        "camera_id=${NEW_CAM_ID}"
    assert_db_field "cameras" "enable" "1" \
        "POST /camera/add enable=1 en BD" \
        "camera_id=${NEW_CAM_ID}"
fi

# =============================================================================
# TEST: POST /camera/add - user level 6 (insuffisant, requis ≥ 8)
# =============================================================================
log_info "Test: POST /camera/add - user level 6 (accès insuffisant)"
CAMERAS_BEFORE_UNAUTH=$(db_domain_query "SELECT COUNT(*) FROM cameras;")

RESPONSE=$(api_call POST /camera/add "${USER_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"name":"Camera-Non-Autorisee","url":"rtsp://10.0.0.100/noauth","access_level":0,"enable":1}')

assert_http_status 403 "POST /camera/add level 6 → HTTP 403"

CAMERAS_AFTER_UNAUTH=$(db_domain_query "SELECT COUNT(*) FROM cameras;")
_test_start
if [[ "${CAMERAS_BEFORE_UNAUTH}" == "${CAMERAS_AFTER_UNAUTH}" ]]; then
    _test_pass "POST /camera/add non autorisé n'a pas modifié la BD"
else
    _test_fail "POST /camera/add non autorisé a quand même modifié la BD!" \
        "avant=${CAMERAS_BEFORE_UNAUTH}, après=${CAMERAS_AFTER_UNAUTH}"
fi

# =============================================================================
# TEST: POST /camera/set - Modification du nom de la caméra
# =============================================================================
if [[ -n "${NEW_CAM_ID}" ]]; then
    log_info "Test: POST /camera/set - modification nom"
    RESPONSE=$(api_call POST /camera/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"camera_id\":${NEW_CAM_ID},\"name\":\"Camera-Renommee\"}")

    assert_http_status 200 "POST /camera/set (name) → HTTP 200"

    assert_db_field "cameras" "name" "Camera-Renommee" \
        "POST /camera/set nom mis à jour en BD" \
        "camera_id=${NEW_CAM_ID}"

    # Modification URL
    log_info "Test: POST /camera/set - modification URL"
    RESPONSE=$(api_call POST /camera/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"camera_id\":${NEW_CAM_ID},\"url\":\"rtsp://10.0.0.200:554/newstream\"}")

    assert_http_status 200 "POST /camera/set (url) → HTTP 200"

    assert_db_field "cameras" "url" "rtsp://10.0.0.200:554/newstream" \
        "POST /camera/set url mis à jour en BD" \
        "camera_id=${NEW_CAM_ID}"

    # Désactivation
    log_info "Test: POST /camera/set - désactivation (enable=0)"
    RESPONSE=$(api_call POST /camera/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"camera_id\":${NEW_CAM_ID},\"enable\":0}")

    assert_http_status 200 "POST /camera/set (enable=0) → HTTP 200"

    assert_db_field "cameras" "enable" "0" \
        "POST /camera/set enable=0 mis à jour en BD" \
        "camera_id=${NEW_CAM_ID}"
fi

# =============================================================================
# TEST: POST /camera/delete - Suppression de la caméra créée
# =============================================================================
if [[ -n "${NEW_CAM_ID}" ]]; then
    log_info "Test: POST /camera/delete - suppression de la caméra de test"
    CAMERAS_BEFORE_DEL=$(db_domain_query "SELECT COUNT(*) FROM cameras;")

    RESPONSE=$(api_call POST /camera/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"camera_id\":${NEW_CAM_ID}}")

    assert_http_status 200 "POST /camera/delete → HTTP 200"

    CAMERAS_AFTER_DEL=$(db_domain_query "SELECT COUNT(*) FROM cameras;")
    _test_start
    if [[ "$((CAMERAS_BEFORE_DEL - 1))" == "${CAMERAS_AFTER_DEL}" ]]; then
        _test_pass "POST /camera/delete a supprimé 1 caméra en BD"
    else
        _test_fail "POST /camera/delete n'a pas décrémenté le nombre de caméras" \
            "avant=${CAMERAS_BEFORE_DEL}, après=${CAMERAS_AFTER_DEL}"
    fi

    # Vérifier que la caméra n'existe plus
    assert_db_row_absent "cameras" \
        "POST /camera/delete: caméra absente de la BD" \
        "camera_id=${NEW_CAM_ID}"
fi

# =============================================================================
# TEST: POST /camera/delete - ID inexistant retourne une erreur
# =============================================================================
log_info "Test: POST /camera/delete - caméra inexistante"
RESPONSE=$(api_call POST /camera/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"camera_id":9999999}')

# L'API peut retourner 404 ou 200 selon l'implémentation
_test_start
if [[ "${LAST_HTTP_CODE}" != "500" ]]; then
    _test_pass "POST /camera/delete ID inexistant: pas d'erreur 500 (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "POST /camera/delete ID inexistant retourne HTTP 500 (erreur serveur)"
fi

print_suite_summary "Suite 04 - Camera CRUD"
[[ ${TESTS_FAILED} -eq 0 ]]
