#!/usr/bin/env bash
# =============================================================================
# 22-run-agent.sh - Tests des endpoints /run/* (authentification agent HMAC)
# =============================================================================
# Endpoints testés:
#   Sécurité : signature invalide → 403, domaine manquant → 400
#   GET  : /run/users/wanna_be_notified, /run/dls/load, /run/dls/plugins,
#          /run/mapping/list, /run/horloges
#   POST : /run/agent/config, /run/mapping/search_txt,
#           /run/user/can_send_txt_cde, /run/modbus/add/io,
#           /run/phidget/add/io, /run/gpiod/add/io,
#           /run/agent/add/{di,ci,do,ai,ao,watchdog,horloge},
#           /run/horloge/add, /run/horloge/add/tick, /run/horloge/del/tick,
#           /run/mnemos/save
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 22 - Endpoints /run/* (Agent HMAC)"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)
DLS_CONFIG='{"agent_classe":"dls","version":"test-dls","start_time":1}'
RESPONSE=$(api_call_agent POST /run/agent/config \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" SYS "${TEST_DOMAIN_SECRET}" "${DLS_CONFIG}")
assert_http_status 200 "DLS agent configuration loaded"
assert_json_field "${RESPONSE}" "audio_tech_id" "MIGRATED_AUDIO" "DLS migrated audio preserved"
assert_json_field "${RESPONSE}" "log_level" "6" "DLS runtime log level independent from compilation debug"
assert_json_field "${RESPONSE}" "agent_tech_id" "SYS" "DLS identity loaded"
assert_db_field "agent_dls" "version" "test-dls" "DLS startup version saved" "agent_tech_id='SYS'"
assert_master_db_field "domains" "debug_compil_dls" "1" "Compilation debug migrated" "domain_uuid='${TEST_DOMAIN_UUID}'"

RESPONSE=$(api_call_agent POST /run/agent/config \
    "${TEST_DOMAIN_UUID}" "00000000-0000-0000-0000-000000000000" SYS "${TEST_DOMAIN_SECRET}" "${DLS_CONFIG}")
assert_http_status 404 "DLS configuration refused on another server"
RESPONSE=$(api_call_agent POST /run/agent/config \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" UNKNOWN_DLS "${TEST_DOMAIN_SECRET}" "${DLS_CONFIG}")
assert_http_status 404 "Unknown DLS identity refused"

RESPONSE=$(api_call GET /agent/dls/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "DLS agent list available"
assert_json_array_not_empty "${RESPONSE}" "agent_dls" "DLS agent list populated"
RESPONSE=$(api_call POST /agent/dls/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"description":"Unauthorized","audio_tech_id":"OTHER_AUDIO"}')
assert_http_status 403 "DLS configuration protected"
RESPONSE=$(api_call POST /agent/dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"description":"Runtime DLS","audio_tech_id":"edited_audio"}')
assert_http_status 200 "DLS audio configuration edited"
assert_db_field "agent_dls" "audio_tech_id" "EDITED_AUDIO" "DLS audio normalized" "agent_tech_id='SYS'"
RESPONSE=$(api_call POST /agent/log_level "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"SYS","log_level":7}')
assert_http_status 200 "DLS generic log level updated"
assert_db_field "agent_dls" "log_level" "7" "DLS runtime log level saved" "agent_tech_id='SYS'"
api_call POST /agent/dls/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"description":"D.L.S","audio_tech_id":"MIGRATED_AUDIO"}' >/dev/null
api_call POST /agent/log_level "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"agent_tech_id":"SYS","log_level":6}' >/dev/null

# =============================================================================
# VALIDATION DE SÉCURITÉ
# =============================================================================

# TEST: Signature incorrecte → 403
log_info "Test: /run/agent/config - signature incorrecte (mauvais secret)"
RESPONSE=$(api_call_agent POST /run/agent/config \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "wrong-secret-xxx" \
    '{"start_time":1,"agent_hostname":"test-host","version":"0.0.0","branche":"test"}')
assert_http_status 403 "/run/agent/config mauvais secret → HTTP 403"

# TEST: X-ABLS-DOMAIN manquant → 400
log_info "Test: /run/agent/config - X-ABLS-DOMAIN manquant"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "${API_URL}/run/agent/config" \
    -H "Content-Type: application/json" \
    -H "Origin: abls-habitat.fr" \
    -H "X-ABLS-SERVER: ${TEST_AGENT_UUID}" \
    -H "X-ABLS-AGENT: ${TEST_AGENT_TECH_ID}" \
    -H "X-ABLS-TIMESTAMP: $(date +%s)" \
    -H "X-ABLS-SIGNATURE: invalide" \
    -d '{"start_time":1}' 2>/dev/null)
_test_start
if [[ "${HTTP_CODE}" == "400" ]]; then
    _test_pass "/run/* sans X-ABLS-DOMAIN → HTTP 400"
else
    _test_fail "/run/* sans X-ABLS-DOMAIN" "attendu: 400, reçu: ${HTTP_CODE}"
fi

# TEST: X-ABLS-SERVER manquant → 400 (le mode legacy agent_uuid seul n'est plus supporté)
log_info "Test: /run/agent/config - X-ABLS-SERVER manquant"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "${API_URL}/run/agent/config" \
    -H "Content-Type: application/json" \
    -H "Origin: abls-habitat.fr" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" \
    -H "X-ABLS-AGENT: ${TEST_AGENT_TECH_ID}" \
    -H "X-ABLS-TIMESTAMP: $(date +%s)" \
    -H "X-ABLS-SIGNATURE: invalide" \
    -d '{"start_time":1}' 2>/dev/null)
_test_start
if [[ "${HTTP_CODE}" == "400" ]]; then
    _test_pass "/run/* sans X-ABLS-SERVER → HTTP 400"
else
    _test_fail "/run/* sans X-ABLS-SERVER" "attendu: 400, reçu: ${HTTP_CODE}"
fi

# =============================================================================
# GET /run/users/wanna_be_notified
# =============================================================================
log_info "Test: GET /run/users/wanna_be_notified"
RESPONSE=$(api_call_agent GET /run/users/wanna_be_notified \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" "")
assert_http_status 200 "GET /run/users/wanna_be_notified → HTTP 200"

# =============================================================================
# GET /run/dls/load
# =============================================================================
log_info "Test: GET /run/dls/load?tech_id=TEST_DLS"
RESPONSE=$(api_call_agent GET "/run/dls/load?tech_id=TEST_DLS" \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" "")
_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /run/dls/load → HTTP 200"
else
    _test_fail "GET /run/dls/load" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# GET /run/horloges
# =============================================================================
log_info "Test: GET /run/horloges"
RESPONSE=$(api_call_agent GET /run/horloges \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" "")
_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /run/horloges → HTTP 200"
else
    _test_fail "GET /run/horloges" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/agent/start is not routed; preserve that contract explicitly
# =============================================================================
log_info "Test: POST /run/agent/start - route absente"
RESPONSE=$(api_call_agent POST /run/agent/start \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" '{}')
assert_http_status 404 "POST /run/agent/start absent du dispatch → HTTP 404"

# =============================================================================
# GET /run/mapping/list
# =============================================================================
log_info "Test: GET /run/mapping/list"
RESPONSE=$(api_call_agent GET /run/mapping/list \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" '')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /run/mapping/list → HTTP 200"
else
    _test_fail "GET /run/mapping/list" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/mapping/search_txt
# =============================================================================
log_info "Test: POST /run/mapping/search_txt"
RESPONSE=$(api_call_agent POST /run/mapping/search_txt \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" \
    '{"agent_acronyme":"MOD_AI_01"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/mapping/search_txt → HTTP 200"
else
    _test_fail "POST /run/mapping/search_txt" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/user/can_send_txt_cde
# =============================================================================
log_info "Test: POST /run/user/can_send_txt_cde"
RESPONSE=$(api_call_agent POST /run/user/can_send_txt_cde \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" \
    '{"xmpp":"test@example.com"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/user/can_send_txt_cde → HTTP 200"
else
    _test_fail "POST /run/user/can_send_txt_cde" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/modbus/add/io - Ajouter des E/S Modbus détectées par l'agent
# =============================================================================
log_info "Test: POST /run/modbus/add/io"
RESPONSE=$(api_call_agent POST /run/modbus/add/io \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "TEST_MODBUS" "${TEST_DOMAIN_SECRET}" \
    '{"nbr_entree_ana":0,"nbr_entree_tor":0,"nbr_sortie_ana":0,"nbr_sortie_tor":0}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/modbus/add/io → HTTP 200"
else
    _test_fail "POST /run/modbus/add/io" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/phidget/add/io - Ajouter des E/S Phidget détectées par l'agent
# =============================================================================
log_info "Test: POST /run/phidget/add/io"
RESPONSE=$(api_call_agent POST /run/phidget/add/io \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "TEST_PHIDGET" "${TEST_DOMAIN_SECRET}" \
    '{"nbr_lignes":0}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/phidget/add/io → HTTP 200"
else
    _test_fail "POST /run/phidget/add/io" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/gpiod/add/io - Ajouter des GPIO détectées par l'agent
# =============================================================================
log_info "Test: POST /run/gpiod/add/io"
RESPONSE=$(api_call_agent POST /run/gpiod/add/io \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "TEST_GPIOD" "${TEST_DOMAIN_SECRET}" \
    '{"nbr_lignes":0}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/gpiod/add/io → HTTP 200"
else
    _test_fail "POST /run/gpiod/add/io" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/horloge/add - Créer une horloge
# =============================================================================
log_info "Test: POST /run/horloge/add"
RESPONSE=$(api_call_agent POST /run/horloge/add \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" \
    '{"tech_id":"TEST_DLS","acronyme":"HORLOGE_RUN_TEST","libelle":"Horloge run test"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/horloge/add → HTTP 200"
else
    _test_fail "POST /run/horloge/add" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/horloge/add/tick - Ajouter une plage horaire
# =============================================================================
log_info "Test: POST /run/horloge/add/tick"
RESPONSE=$(api_call_agent POST /run/horloge/add/tick \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" \
    '{"tech_id":"TEST_DLS","acronyme":"HORLOGE_RUN_TEST","heure":8,"minute":0}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/horloge/add/tick → HTTP 200"
else
    _test_fail "POST /run/horloge/add/tick" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/horloge/del/tick - Supprimer une plage horaire
# =============================================================================
log_info "Test: POST /run/horloge/del/tick"
RESPONSE=$(api_call_agent POST /run/horloge/del/tick \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" \
    '{"tech_id":"TEST_DLS","acronyme":"HORLOGE_RUN_TEST"}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/horloge/del/tick → HTTP 200"
else
    _test_fail "POST /run/horloge/del/tick" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# Nettoyage horloge temporaire
db_domain_query "DELETE FROM mnemos_HORLOGE WHERE tech_id='TEST_DLS' AND acronyme='HORLOGE_RUN_TEST';" >/dev/null 2>&1 || true

# =============================================================================
# POST /run/mnemos/save - Sauvegarder des mnémos depuis un agent
# =============================================================================
log_info "Test: POST /run/mnemos/save - sauvegarde AI"
RESPONSE=$(api_call_agent POST /run/mnemos/save \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" \
    '{"mnemos_AI":[{"classe":"AI","tech_id":"TEST_DLS","acronyme":"TEST_AI","archivage":false}]}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/mnemos/save → HTTP 200"
else
    _test_fail "POST /run/mnemos/save" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# GET /run/dls/plugins
# =============================================================================
log_info "Test: GET /run/dls/plugins"
RESPONSE=$(api_call_agent GET /run/dls/plugins \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" '')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /run/dls/plugins → HTTP 200"
else
    _test_fail "GET /run/dls/plugins" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

for add_route in di ci do ai ao watchdog horloge; do
    RESPONSE=$(api_call_agent POST "/run/agent/add/${add_route}" \
        "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_AGENT_TECH_ID}" "${TEST_DOMAIN_SECRET}" '{}')
    assert_http_status 400 "POST /run/agent/add/${add_route} sans champs requis → HTTP 400"
done

print_suite_summary "Suite 22 - Endpoints /run/* (Agent HMAC)"
[[ ${TESTS_FAILED} -eq 0 ]]
