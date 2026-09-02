#!/usr/bin/env bash
# =============================================================================
# 13-audio.sh - Tests des endpoints Audio
# =============================================================================
# Endpoints testés: GET /audio/list, GET /audio/zones/list,
#                   GET /audio/zone/get,
#                   POST /audio/set, POST /audio/zones/set,
#                   POST /audio/zone/map, POST /audio/zone/test,
#                   DELETE /audio/zones/delete, DELETE /audio/zone/unmap
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 13 - Audio"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# TEST: GET /audio/zones/list
# =============================================================================
log_info "Test: GET /audio/zones/list"
RESPONSE=$(api_call GET /audio/zones/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /audio/zones/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "audio_zones" "GET /audio/zones/list retourne des zones"

# ZD_NONE doit être présente
ZD_NONE_FOUND=$(echo "${RESPONSE}" | jq -r '.audio_zones[] | select(.audio_zone_name == "ZD_NONE") | .audio_zone_name' 2>/dev/null)
_test_start
if [[ "${ZD_NONE_FOUND}" == "ZD_NONE" ]]; then
    _test_pass "GET /audio/zones/list contient ZD_NONE"
else
    _test_fail "GET /audio/zones/list ne contient pas ZD_NONE" "${RESPONSE}"
fi

# ZD_TEST doit être présente
ZD_TEST_FOUND=$(echo "${RESPONSE}" | jq -r '.audio_zones[] | select(.audio_zone_name == "ZD_TEST") | .audio_zone_name' 2>/dev/null)
_test_start
if [[ "${ZD_TEST_FOUND}" == "ZD_TEST" ]]; then
    _test_pass "GET /audio/zones/list contient ZD_TEST"
else
    _test_fail "GET /audio/zones/list ne contient pas ZD_TEST" "${RESPONSE}"
fi

log_info "Test: GET /audio/zones/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /audio/zones/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /audio/zones/list readonly → HTTP 403"

# =============================================================================
# TEST: GET /audio/list
# =============================================================================
log_info "Test: GET /audio/list"
RESPONSE=$(api_call GET /audio/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /audio/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "audio" "GET /audio/list retourne des agents audio"

AUDIO_IN_DB=$(db_domain_query "SELECT COUNT(*) FROM audio;")
AUDIO_IN_API=$(echo "${RESPONSE}" | jq '.audio | length' 2>/dev/null)
_test_start
if [[ "${AUDIO_IN_API}" == "${AUDIO_IN_DB}" ]]; then
    _test_pass "GET /audio/list nombre cohérent avec BD (${AUDIO_IN_DB})"
else
    _test_fail "GET /audio/list nombre incohérent" "API=${AUDIO_IN_API}, BD=${AUDIO_IN_DB}"
fi

_test_start
if echo "${RESPONSE}" | jq -e '.audio[] | select(.agent_tech_id == "TEST_AUDIO") |
    .server_uuid == "ffffffff-0000-0000-0000-000000000001" and
    .description == "Audio de test" and .language == "fr" and .device == "default" and .volume == 80 and
    has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /audio/list retourne la configuration complète de TEST_AUDIO"
else
    _test_fail "GET /audio/list ne retourne pas la configuration attendue pour TEST_AUDIO" "${RESPONSE}"
fi

log_info "Test: GET /audio/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET /audio/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /audio/list readonly → HTTP 403"

# =============================================================================
# TEST: GET /audio/zone/get
# =============================================================================
log_info "Test: GET /audio/zone/get?audio_zone_name=ZD_TEST"
RESPONSE=$(api_call GET "/audio/zone/get?audio_zone_name=ZD_TEST" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /audio/zone/get → HTTP 200"
assert_json_field "${RESPONSE}" "audio_zone_name" "ZD_TEST" "GET /audio/zone/get zone_name correct"

# =============================================================================
# TEST: POST /audio/set - Modifier la description du thread audio
# =============================================================================
log_info "Test: POST /audio/set - modification description"
RESPONSE=$(api_call POST /audio/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_AUDIO","description":"Audio modifié","language":"fr","device":"default","volume":80}')

assert_http_status 200 "POST /audio/set → HTTP 200"

AUDIO_DESC=$(db_domain_query "SELECT description FROM audio WHERE agent_tech_id='TEST_AUDIO' LIMIT 1;")
_test_start
if [[ "${AUDIO_DESC}" == "Audio modifié" ]]; then
    _test_pass "POST /audio/set description mise à jour en BD"
else
    _test_fail "POST /audio/set description non mise à jour en BD" "BD='${AUDIO_DESC}'"
fi

api_call POST /audio/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_AUDIO","description":"Audio de test","language":"fr","device":"default","volume":80}' >/dev/null

log_info "Test: POST /audio/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /audio/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_AUDIO","description":"Tentative","language":"fr","device":"default","volume":80}')
assert_http_status 403 "POST /audio/set readonly → HTTP 403"

# =============================================================================
# TEST: POST /audio/zones/set - Modifier la description d'une zone
# =============================================================================
log_info "Test: POST /audio/zones/set - modification description ZD_TEST"
RESPONSE=$(api_call POST /audio/zones/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"audio_zone_name":"ZD_TEST","description":"Zone audio test modifiée"}')

assert_http_status 200 "POST /audio/zones/set → HTTP 200"

ZD_DESC=$(db_domain_query "SELECT description FROM audio_zones WHERE audio_zone_name='ZD_TEST' LIMIT 1;")
_test_start
if [[ "${ZD_DESC}" == "Zone audio test modifiée" ]]; then
    _test_pass "POST /audio/zones/set description mise à jour en BD"
else
    _test_fail "POST /audio/zones/set description non mise à jour en BD" "BD='${ZD_DESC}'"
fi

api_call POST /audio/zones/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"audio_zone_name":"ZD_TEST","description":"Zone audio de test"}' >/dev/null

# =============================================================================
# TEST: POST /audio/zone/map - Associer un thread audio à une zone
# =============================================================================
log_info "Test: POST /audio/zone/map - association TEST_AUDIO → ZD_TEST"
# Nettoyer une éventuelle association existante
db_domain_query "DELETE FROM audio_zone_map WHERE agent_tech_id='TEST_AUDIO' AND audio_zone_id=(SELECT audio_zone_id FROM audio_zones WHERE audio_zone_name='ZD_TEST');" >/dev/null 2>&1 || true

MAP_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM audio_zone_map;")
RESPONSE=$(api_call POST /audio/zone/map "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"audio_zone_name":"ZD_TEST","agent_tech_id":"TEST_AUDIO"}')

assert_http_status 200 "POST /audio/zone/map → HTTP 200"

MAP_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM audio_zone_map;")
_test_start
if [[ "$((MAP_CNT_BEFORE + 1))" == "${MAP_CNT_AFTER}" ]]; then
    _test_pass "POST /audio/zone/map: association créée en BD"
else
    _test_fail "POST /audio/zone/map: association non créée en BD" \
        "avant=${MAP_CNT_BEFORE}, après=${MAP_CNT_AFTER}"
fi

# =============================================================================
# TEST: GET /audio/zone/get - filtrage par agent
# =============================================================================
log_info "Test: GET /audio/zone/get?agent_tech_id=TEST_AUDIO"
RESPONSE=$(api_call GET "/audio/zone/get?agent_tech_id=TEST_AUDIO" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /audio/zone/get par agent → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "audio_zone_map" "GET /audio/zone/get par agent retourne des zones"

log_info "Test: GET /audio/zone/get - aucun filtre"
RESPONSE=$(api_call GET "/audio/zone/get" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /audio/zone/get sans filtre → HTTP 400"

# =============================================================================
# TEST: POST /audio/zone/test - Tester la diffusion audio
# =============================================================================
log_info "Test: POST /audio/zone/test - test diffusion ZD_TEST"
RESPONSE=$(api_call POST /audio/zone/test "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"audio_zone_name":"ZD_TEST","message":"Test de synthèse vocale"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /audio/zone/test → HTTP 200"
else
    _test_fail "POST /audio/zone/test" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: DELETE /audio/zone/unmap - Supprimer l'association
# =============================================================================
log_info "Test: DELETE /audio/zone/unmap - suppression TEST_AUDIO → ZD_TEST"
RESPONSE=$(api_call DELETE /audio/zone/unmap "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"audio_zone_name":"ZD_TEST","agent_tech_id":"TEST_AUDIO"}')

assert_http_status 200 "DELETE /audio/zone/unmap → HTTP 200"

MAP_CNT_DEL=$(db_domain_query "SELECT COUNT(*) FROM audio_zone_map;")
_test_start
if [[ "${MAP_CNT_DEL}" == "${MAP_CNT_BEFORE}" ]]; then
    _test_pass "DELETE /audio/zone/unmap: association supprimée de la BD"
else
    _test_fail "DELETE /audio/zone/unmap: compteur incohérent" \
        "attendu=${MAP_CNT_BEFORE}, actuel=${MAP_CNT_DEL}"
fi

# =============================================================================
# TEST: DELETE /audio/zones/delete - Créer puis supprimer une zone
# =============================================================================
log_info "Test: DELETE /audio/zones/delete - création et suppression"
db_domain_query "INSERT IGNORE INTO audio_zones (audio_zone_name, description) VALUES ('ZD_TEMP_DEL', 'Zone temporaire à supprimer');" >/dev/null 2>&1 || true

ZONES_CNT_BEFORE=$(db_domain_query "SELECT COUNT(*) FROM audio_zones;")
RESPONSE=$(api_call DELETE /audio/zones/delete "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"audio_zone_name":"ZD_TEMP_DEL"}')

assert_http_status 200 "DELETE /audio/zones/delete → HTTP 200"

ZONES_CNT_AFTER=$(db_domain_query "SELECT COUNT(*) FROM audio_zones;")
_test_start
if [[ "$((ZONES_CNT_BEFORE - 1))" == "${ZONES_CNT_AFTER}" ]]; then
    _test_pass "DELETE /audio/zones/delete: zone supprimée de la BD"
else
    _test_fail "DELETE /audio/zones/delete: compteur incohérent" \
        "avant=${ZONES_CNT_BEFORE}, après=${ZONES_CNT_AFTER}"
fi

print_suite_summary "Suite 13 - Audio"
[[ ${TESTS_FAILED} -eq 0 ]]
