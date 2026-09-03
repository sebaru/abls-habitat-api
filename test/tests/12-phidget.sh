#!/usr/bin/env bash
# =============================================================================
# 12-phidget.sh - Catégorie dédiée aux endpoints Phidget
# =============================================================================
# Endpoints testés: GET /phidget/list, GET /phidget/get?agent_tech_id=..., POST /phidget/set, POST /phidget/set/io
# L'ajout automatique des I/O détectées par l'agent est couvert par la suite 21 (/run/phidget/add/io).
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 12 - Endpoints Phidget"

ADMIN_TOKEN=$(make_admin_token)
READONLY_TOKEN=$(make_readonly_token)

# =============================================================================
# Endpoints Phidget
# =============================================================================

log_info "Test: GET /phidget/list"
RESPONSE=$(api_call GET "/phidget/list" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /phidget/list → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "phidgets" "GET /phidget/list retourne des agents phidget"

PHIDGET_LIST_DB=$(db_domain_query "SELECT COUNT(*) FROM phidget;")
PHIDGET_LIST_API=$(echo "${RESPONSE}" | jq '.phidgets | length' 2>/dev/null)
_test_start
if [[ "${PHIDGET_LIST_API}" == "${PHIDGET_LIST_DB}" ]]; then
    _test_pass "GET /phidget/list nombre cohérent avec BD (${PHIDGET_LIST_DB})"
else
    _test_fail "GET /phidget/list nombre incohérent" "API=${PHIDGET_LIST_API}, BD=${PHIDGET_LIST_DB}"
fi

_test_start
if echo "${RESPONSE}" | jq -e '.phidgets[] | select(.agent_tech_id == "TEST_PHIDGET") |
    .hostname == "192.168.1.201" and .serial == 12345 and has("server_hostname") and has("is_alive")' >/dev/null 2>&1; then
    _test_pass "GET /phidget/list retourne la configuration complète de TEST_PHIDGET"
else
    _test_fail "GET /phidget/list ne retourne pas la configuration attendue pour TEST_PHIDGET" "${RESPONSE}"
fi

log_info "Test: GET /phidget/list - readonly (accès insuffisant)"
RESPONSE=$(api_call GET "/phidget/list" "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /phidget/list readonly → HTTP 403"

log_info "Test: GET /phidget/get?agent_tech_id=TEST_PHIDGET"
RESPONSE=$(api_call GET "/phidget/get?agent_tech_id=TEST_PHIDGET" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /phidget/get?agent_tech_id=TEST_PHIDGET → HTTP 200"
assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_PHIDGET" "GET /phidget/get agent_tech_id correct"

PHI_FOUND=$(echo "${RESPONSE}" | jq -r '.IO[] | select(.agent_tech_id == "TEST_PHIDGET") | .agent_tech_id' 2>/dev/null)
_test_start
if [[ "${PHI_FOUND}" == "TEST_PHIDGET" ]]; then
    _test_pass "GET /phidget/get?agent_tech_id=TEST_PHIDGET contient les I/O de TEST_PHIDGET"
else
    _test_fail "GET /phidget/get?agent_tech_id=TEST_PHIDGET ne contient pas les I/O de TEST_PHIDGET" "${RESPONSE}"
fi

PHIDGET_DB=$(db_domain_query "SELECT COUNT(*) FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET';")
PHIDGET_API=$(echo "${RESPONSE}" | jq '.IO | length' 2>/dev/null)
_test_start
if [[ "${PHIDGET_API}" == "${PHIDGET_DB}" ]]; then
    _test_pass "GET /phidget/get?agent_tech_id=TEST_PHIDGET nombre cohérent avec BD (${PHIDGET_DB})"
else
    _test_fail "GET /phidget/get?agent_tech_id=TEST_PHIDGET nombre incohérent" "API=${PHIDGET_API}, BD=${PHIDGET_DB}"
fi

log_info "Test: GET /phidget/get - paramètre manquant"
RESPONSE=$(api_call GET "/phidget/get" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 400 "GET /phidget/get sans agent_tech_id → HTTP 400"

log_info "Test: GET /phidget/get - agent_tech_id inconnu"
RESPONSE=$(api_call GET "/phidget/get?agent_tech_id=UNKNOWN_PHIDGET" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 404 "GET /phidget/get avec agent_tech_id inconnu → HTTP 404"

log_info "Test: GET /phidget/get?agent_tech_id=TEST_PHIDGET - readonly (accès insuffisant)"
RESPONSE=$(api_call GET "/phidget/get?agent_tech_id=TEST_PHIDGET" "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /phidget/get?agent_tech_id=TEST_PHIDGET readonly → HTTP 403"

# POST /phidget/set
log_info "Test: POST /phidget/set - modification description"
RESPONSE=$(api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget modifié","hostname":"192.168.1.201","password":"","serial":12345}')

assert_http_status 200 "POST /phidget/set → HTTP 200"

PHI_DESC=$(db_domain_query "SELECT description FROM phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
_test_start
if [[ "${PHI_DESC}" == "Phidget modifié" ]]; then
    _test_pass "POST /phidget/set description mise à jour en BD"
else
    _test_fail "POST /phidget/set description non mise à jour en BD" "BD='${PHI_DESC}'"
fi

api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget de test","hostname":"192.168.1.201","password":"","serial":12345}' >/dev/null

log_info "Test: POST /phidget/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /phidget/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Tentative","hostname":"x","password":"","serial":12345}')
assert_http_status 403 "POST /phidget/set readonly → HTTP 403"

# POST /phidget/set/io
PHI_IO_ID=$(db_domain_query "SELECT phidget_io_id FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
log_info "Test: POST /phidget/set/io - modification libellé PHI_IO_01"
RESPONSE=$(api_call POST /phidget/set/io "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"phidget_io_id\":${PHI_IO_ID},\"capteur\":\"DIGITAL-INPUT\",\"libelle\":\"IO Phidget modifié\",\"unite\":\"bool\",\"intervalle\":5000,\"archivage\":36000}")

assert_http_status 200 "POST /phidget/set/io → HTTP 200"

PHI_IO_LIB=$(db_domain_query "SELECT libelle FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
PHI_IO_UNITE=$(db_domain_query "SELECT unite FROM phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01' LIMIT 1;")
_test_start
if [[ "${PHI_IO_LIB}" == "IO Phidget modifié" && "${PHI_IO_UNITE}" == "bool" ]]; then
    _test_pass "POST /phidget/set/io libellé et unité mis à jour en BD"
else
    _test_fail "POST /phidget/set/io libellé/unité non mis à jour en BD" "BD libelle='${PHI_IO_LIB}', unite='${PHI_IO_UNITE}'"
fi
db_domain_query "UPDATE phidget_IO SET classe='DI', port=0, capteur='', libelle='Entrée phidget test 01', unite='', intervalle=5000, archivage=36000 WHERE agent_tech_id='TEST_PHIDGET' AND thread_acronyme='PHI_IO_01';" >/dev/null 2>&1 || true

# MQTT STATUS/AGENT -> phidget.agent_status
log_info "Test: MQTT STATUS/AGENT/TEST_PHIDGET - met à jour agent_status"
_test_start
if ! command -v mosquitto_pub >/dev/null 2>&1; then
    _test_fail "MQTT STATUS/AGENT update" "mosquitto_pub introuvable"
else
    mqtt_ok=true
    mosquitto_pub -V mqttv311 -h 127.0.0.1 -p 11883 \
        -u "${TEST_DOMAIN_UUID}-agent" -P test_mqtt_pass \
        -t "${TEST_DOMAIN_UUID}/STATUS/AGENT/TEST_PHIDGET" \
        -m '{"status":"running from test"}' >/dev/null 2>&1 || mqtt_ok=false

    if [[ "${mqtt_ok}" != "true" ]]; then
        _test_fail "MQTT STATUS/AGENT update" "échec publication MQTT"
    else
        PHI_STATUS=""
        for _ in $(seq 1 20); do
            PHI_STATUS=$(db_domain_query "SELECT agent_status FROM phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
            if [[ "${PHI_STATUS}" == "running from test" ]]; then
                break
            fi
            sleep 0.2
        done

        if [[ "${PHI_STATUS}" == "running from test" ]]; then
            _test_pass "MQTT STATUS/AGENT met à jour phidget.agent_status"
        else
            _test_fail "MQTT STATUS/AGENT update" "agent_status='${PHI_STATUS}'"
        fi
    fi
fi

log_info "Test: MQTT STATUS/AGENT/TEST_PHIDGET sans champ status - ignoré"
_test_start
if ! command -v mosquitto_pub >/dev/null 2>&1; then
    _test_fail "MQTT STATUS/AGENT sans status" "mosquitto_pub introuvable"
else
    BEFORE_STATUS=$(db_domain_query "SELECT agent_status FROM phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
    mqtt_ok=true
    mosquitto_pub -V mqttv311 -h 127.0.0.1 -p 11883 \
        -u "${TEST_DOMAIN_UUID}-agent" -P test_mqtt_pass \
        -t "${TEST_DOMAIN_UUID}/STATUS/AGENT/TEST_PHIDGET" \
        -m '{"state":"ignored"}' >/dev/null 2>&1 || mqtt_ok=false

    if [[ "${mqtt_ok}" != "true" ]]; then
        _test_fail "MQTT STATUS/AGENT sans status" "échec publication MQTT"
    else
        sleep 0.5
        AFTER_STATUS=$(db_domain_query "SELECT agent_status FROM phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
        if [[ "${AFTER_STATUS}" == "${BEFORE_STATUS}" ]]; then
            _test_pass "MQTT STATUS/AGENT sans status n'altère pas agent_status"
        else
            _test_fail "MQTT STATUS/AGENT sans status" "avant='${BEFORE_STATUS}', après='${AFTER_STATUS}'"
        fi
    fi
fi

print_suite_summary "Suite 12 - Endpoints Phidget"
[[ ${TESTS_FAILED} -eq 0 ]]
