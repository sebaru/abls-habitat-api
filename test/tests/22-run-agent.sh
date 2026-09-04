#!/usr/bin/env bash
# =============================================================================
# 21-run-agent.sh - Tests des endpoints /run/* (authentification agent HMAC)
# =============================================================================
# Endpoints testés:
#   Sécurité : signature invalide → 403, domaine manquant → 400
#   GET  : /run/users/wanna_be_notified, /run/dls/load, /run/horloges,
#           /run/agent/config
#   POST : /run/agent/start, /run/mapping/list, /run/mapping/search_txt,
#           /run/user/can_send_txt_cde, /run/modbus/add/io,
#           /run/phidget/add/io, /run/gpiod/add/io,
#           /run/horloge/add, /run/horloge/add/tick, /run/horloge/del/tick,
#           /run/mnemos/save, /run/dls/plugins
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 21 - Endpoints /run/* (Agent HMAC)"

# =============================================================================
# VALIDATION DE SÉCURITÉ
# =============================================================================

# TEST: Signature incorrecte → 403
log_info "Test: /run/agent/start - signature incorrecte (mauvais secret)"
RESPONSE=$(api_call_agent POST /run/agent/start \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "wrong-secret-xxx" \
    '{"start_time":1,"agent_hostname":"test-host","version":"0.0.0","branche":"test"}')
assert_http_status 403 "/run/agent/start mauvais secret → HTTP 403"

# TEST: X-ABLS-DOMAIN manquant → 400
log_info "Test: /run/agent/start - X-ABLS-DOMAIN manquant"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "${API_URL}/run/agent/start" \
    -H "Content-Type: application/json" \
    -H "Origin: abls-habitat.fr" \
    -H "X-ABLS-AGENT: ${TEST_AGENT_UUID}" \
    -H "X-ABLS-TIMESTAMP: $(date +%s)" \
    -H "X-ABLS-SIGNATURE: invalide" \
    -d '{"start_time":1}' 2>/dev/null)
_test_start
if [[ "${HTTP_CODE}" == "400" ]]; then
    _test_pass "/run/* sans X-ABLS-DOMAIN → HTTP 400"
else
    _test_fail "/run/* sans X-ABLS-DOMAIN" "attendu: 400, reçu: ${HTTP_CODE}"
fi

# =============================================================================
# GET /run/users/wanna_be_notified
# =============================================================================
log_info "Test: GET /run/users/wanna_be_notified"
RESPONSE=$(api_call_agent GET /run/users/wanna_be_notified \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" "")
assert_http_status 200 "GET /run/users/wanna_be_notified → HTTP 200"

# =============================================================================
# GET /run/dls/load
# =============================================================================
log_info "Test: GET /run/dls/load?tech_id=TEST_DLS"
RESPONSE=$(api_call_agent GET "/run/dls/load?tech_id=TEST_DLS" \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" "")
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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" "")
_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /run/horloges → HTTP 200"
else
    _test_fail "GET /run/horloges" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/agent/start
# =============================================================================
log_info "Test: POST /run/agent/start"
START_TIME=$(date +%s)
RESPONSE=$(api_call_agent POST /run/agent/start \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
    "{\"start_time\":${START_TIME},\"agent_hostname\":\"test-host-21\",\"version\":\"9.9.9\",\"branche\":\"test\"}")

assert_http_status 200 "POST /run/agent/start → HTTP 200"

# Vérifier que l'agent est bien mis à jour en BD
AGENT_VER=$(db_domain_query \
    "SELECT version FROM agents WHERE agent_uuid='${TEST_AGENT_UUID}' LIMIT 1;")
_test_start
if [[ "${AGENT_VER}" == "9.9.9" ]]; then
    _test_pass "POST /run/agent/start: version mise à jour en BD"
else
    _test_fail "POST /run/agent/start: version non mise à jour en BD" "BD='${AGENT_VER}'"
fi

# =============================================================================
# POST /run/mapping/list
# =============================================================================
log_info "Test: POST /run/mapping/list"
RESPONSE=$(api_call_agent POST /run/mapping/list \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" '{}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/mapping/list → HTTP 200"
else
    _test_fail "POST /run/mapping/list" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/mapping/search_txt
# =============================================================================
log_info "Test: POST /run/mapping/search_txt"
RESPONSE=$(api_call_agent POST /run/mapping/search_txt \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
    '{"thread_acronyme":"MOD_AI_01"}')

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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
    '{"agent_tech_id":"TEST_MODBUS","nbr_entree_ana":0,"nbr_entree_tor":0,"nbr_sortie_ana":0,"nbr_sortie_tor":0}')

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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
    '{"agent_tech_id":"TEST_PHIDGET","nbr_lignes":0}')

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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
    '{"agent_tech_id":"TEST_GPIOD","nbr_lignes":0}')

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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
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
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" \
    '{"mnemos_AI":[{"classe":"AI","tech_id":"TEST_DLS","acronyme":"TEST_AI","archivage":false}]}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/mnemos/save → HTTP 200"
else
    _test_fail "POST /run/mnemos/save" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# POST /run/dls/plugins
# =============================================================================
log_info "Test: POST /run/dls/plugins"
RESPONSE=$(api_call_agent POST /run/dls/plugins \
    "${TEST_DOMAIN_UUID}" "${TEST_AGENT_UUID}" "${TEST_DOMAIN_SECRET}" '{}')

_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "POST /run/dls/plugins → HTTP 200"
else
    _test_fail "POST /run/dls/plugins" "attendu: 200, reçu: ${LAST_HTTP_CODE}"
fi

print_suite_summary "Suite 21 - Endpoints /run/* (Agent HMAC)"
[[ ${TESTS_FAILED} -eq 0 ]]
