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

PHIDGET_LIST_DB=$(db_domain_query "SELECT COUNT(*) FROM agent_phidget;")
PHIDGET_LIST_API=$(echo "${RESPONSE}" | jq '.phidgets | length' 2>/dev/null)
_test_start
if [[ "${PHIDGET_LIST_API}" == "${PHIDGET_LIST_DB}" ]]; then
    _test_pass "GET /phidget/list nombre cohérent avec BD (${PHIDGET_LIST_DB})"
else
    _test_fail "GET /phidget/list nombre incohérent" "API=${PHIDGET_LIST_API}, BD=${PHIDGET_LIST_DB}"
fi

_test_start
if echo "${RESPONSE}" | jq -e '.phidgets[] | select(.agent_tech_id == "TEST_PHIDGET") |
    .hostname == "192.168.1.201" and .serial == 12345 and has("server_hostname") and has("heartbeat_time") and has("agent_alive") and (has("is_alive") | not)' >/dev/null 2>&1; then
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

_test_start
if echo "${RESPONSE}" | jq -e 'any(.IO[]; .agent_tech_id == "TEST_PHIDGET")' >/dev/null 2>&1; then
    _test_pass "GET /phidget/get?agent_tech_id=TEST_PHIDGET contient les I/O de TEST_PHIDGET"
else
    _test_fail "GET /phidget/get?agent_tech_id=TEST_PHIDGET ne contient pas les I/O de TEST_PHIDGET" "${RESPONSE}"
fi

PHIDGET_DB=$(db_domain_query "SELECT COUNT(*) FROM agent_phidget_IO WHERE agent_tech_id='TEST_PHIDGET';")
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

log_info "Test: GET /phidget/get - agent existant sans I/O"
if db_domain_query "INSERT INTO agent_phidget (server_uuid, agent_tech_id, hostname, serial) VALUES ('${TEST_AGENT_UUID}', 'TEST_PHIDGET_EMPTY', '192.0.2.254', -1);" >/dev/null; then
    RESPONSE=$(api_call GET "/phidget/get?agent_tech_id=TEST_PHIDGET_EMPTY" "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
    assert_http_status 200 "GET /phidget/get agent sans I/O → HTTP 200"
    assert_json_field "${RESPONSE}" "agent_tech_id" "TEST_PHIDGET_EMPTY" "GET /phidget/get agent sans I/O identifié"
    _test_start
    if echo "${RESPONSE}" | jq -e '.IO == [] and .nbr_IO == 0' >/dev/null 2>&1; then
        _test_pass "GET /phidget/get agent sans I/O retourne un tableau vide"
    else
        _test_fail "GET /phidget/get agent sans I/O" "${RESPONSE}"
    fi
    if ! db_domain_query "DELETE FROM agent_phidget WHERE agent_tech_id='TEST_PHIDGET_EMPTY';" >/dev/null; then
        _test_start
        _test_fail "Nettoyage de TEST_PHIDGET_EMPTY" "échec SQL"
    fi
else
    _test_start
    _test_fail "Préparation de l'agent sans I/O" "échec SQL"
fi

log_info "Test: GET /phidget/get?agent_tech_id=TEST_PHIDGET - readonly (accès insuffisant)"
RESPONSE=$(api_call GET "/phidget/get?agent_tech_id=TEST_PHIDGET" "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /phidget/get?agent_tech_id=TEST_PHIDGET readonly → HTTP 403"

# POST /phidget/set
log_info "Test: POST /phidget/set - modification description"
RESPONSE=$(api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget modifié","hostname":"192.168.1.201","password":"","serial":12345}')

assert_http_status 200 "POST /phidget/set → HTTP 200"

PHI_DESC=$(db_domain_query "SELECT description FROM agent_phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;")
_test_start
if [[ "${PHI_DESC}" == "Phidget modifié" ]]; then
    _test_pass "POST /phidget/set description mise à jour en BD"
else
    _test_fail "POST /phidget/set description non mise à jour en BD" "BD='${PHI_DESC}'"
fi

api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget de test","hostname":"192.168.1.201","password":"","serial":12345}' >/dev/null

# POST /phidget/set crée (ou met à jour) le plugin DLS de l'agent
log_info "Test: POST /phidget/set - création du plugin DLS"
db_domain_query "DELETE FROM dls WHERE tech_id='TEST_PHIDGET';" >/dev/null 2>&1 || true
PHIDGET_SET_PAYLOAD='{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Phidget de test","hostname":"192.168.1.201","password":"","serial":12345}'
RESPONSE=$(api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${PHIDGET_SET_PAYLOAD}")
assert_http_status 200 "POST /phidget/set (plugin DLS) → HTTP 200"
assert_db_row_exists "dls" "POST /phidget/set: plugin DLS créé" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "name" "Phidget de test" "POST /phidget/set: DLS name correct" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "shortname" "Phidget de test" "POST /phidget/set: DLS shortname correct" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "package" "Agent_phidget" "POST /phidget/set: DLS package correct" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "enable" "1" "POST /phidget/set: DLS enable=1 à la création" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "syn_id" "2" "POST /phidget/set: DLS syn_id=2 à la création" "tech_id='TEST_PHIDGET'"

log_info "Test: POST /phidget/set - idempotence du plugin DLS, enable préservé"
db_domain_query "UPDATE dls SET name='Ancien nom', shortname='Ancien shortname', package='Old_package', enable=0 WHERE tech_id='TEST_PHIDGET';" >/dev/null 2>&1
RESPONSE=$(api_call POST /phidget/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${PHIDGET_SET_PAYLOAD}")
assert_http_status 200 "POST /phidget/set (idempotence) → HTTP 200"
DLS_COUNT=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id='TEST_PHIDGET';")
_test_start
if [[ "${DLS_COUNT}" == "1" ]]; then
    _test_pass "POST /phidget/set (idempotence) pas de doublon DLS"
else
    _test_fail "POST /phidget/set (idempotence) doublon DLS détecté" "count=${DLS_COUNT}"
fi
assert_db_field "dls" "name" "Phidget de test" "POST /phidget/set (idempotence) DLS name remis à jour" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "package" "Agent_phidget" "POST /phidget/set (idempotence) DLS package remis à jour" "tech_id='TEST_PHIDGET'"
assert_db_field "dls" "enable" "0" "POST /phidget/set (idempotence) DLS enable préservé" "tech_id='TEST_PHIDGET'"
db_domain_query "DELETE FROM dls WHERE tech_id='TEST_PHIDGET';" >/dev/null 2>&1 || true

log_info "Test: POST /phidget/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /phidget/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"server_uuid":"ffffffff-0000-0000-0000-000000000001","agent_tech_id":"TEST_PHIDGET","description":"Tentative","hostname":"x","password":"","serial":12345}')
assert_http_status 403 "POST /phidget/set readonly → HTTP 403"

# POST /phidget/set/io
PHI_IO_ID=$(db_domain_query "SELECT phidget_io_id FROM agent_phidget_IO WHERE agent_tech_id='TEST_PHIDGET' AND agent_acronyme='PHI_IO_01' LIMIT 1;")
if [[ ! "${PHI_IO_ID}" =~ ^[0-9]+$ ]]; then
    _test_start
    _test_fail "Préparation de PHI_IO_01" "phidget_io_id invalide: '${PHI_IO_ID}'"
    print_suite_summary "Suite 12 - Endpoints Phidget"
    exit 1
fi
PHI_IO_RESTORE=$(db_domain_query "SELECT CONCAT('UPDATE agent_phidget_IO SET agent_acronyme=', QUOTE(agent_acronyme), ', classe=', QUOTE(classe), ', port=', port, ', capteur=', QUOTE(capteur), ', description=', QUOTE(description), ', unite=', QUOTE(unite), ', intervalle=', intervalle, ', archivage=', archivage, ' WHERE phidget_io_id=', phidget_io_id, ';') FROM agent_phidget_IO WHERE phidget_io_id=${PHI_IO_ID};")
if [[ -z "${PHI_IO_RESTORE}" ]]; then
    _test_start
    _test_fail "Sauvegarde de PHI_IO_01" "échec SQL"
    print_suite_summary "Suite 12 - Endpoints Phidget"
    exit 1
fi
log_info "Test: POST /phidget/set/io - modification libellé PHI_IO_01"
RESPONSE=$(api_call POST /phidget/set/io "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"phidget_io_id\":${PHI_IO_ID},\"capteur\":\"DIGITAL-INPUT\",\"description\":\"IO Phidget modifié\",\"unite\":\"bool\",\"intervalle\":5000,\"archivage\":36000}")

assert_http_status 200 "POST /phidget/set/io → HTTP 200"

PHI_IO_LIB=$(db_domain_query "SELECT description FROM agent_phidget_IO WHERE phidget_io_id=${PHI_IO_ID};")
PHI_IO_UNITE=$(db_domain_query "SELECT unite FROM agent_phidget_IO WHERE phidget_io_id=${PHI_IO_ID};")
_test_start
if [[ "${PHI_IO_LIB}" == "IO Phidget modifié" && "${PHI_IO_UNITE}" == "bool" ]]; then
    _test_pass "POST /phidget/set/io libellé et unité mis à jour en BD"
else
    _test_fail "POST /phidget/set/io libellé/unité non mis à jour en BD" "BD libelle='${PHI_IO_LIB}', unite='${PHI_IO_UNITE}'"
fi
assert_db_field "agent_phidget_IO" "agent_acronyme" "DI00" "POST /phidget/set/io acronyme recalculé" "phidget_io_id=${PHI_IO_ID}"
if ! db_domain_query "${PHI_IO_RESTORE}" >/dev/null; then
    _test_start
    _test_fail "Restauration de PHI_IO_01" "échec SQL"
fi

# Publie un message agent sur AGENT/TEST_PHIDGET/<topic> et capture le relais browsers/AGENT/STATUS (vide si aucun)
mqtt_agent_exchange() {
    local topic="$1" payload="$2" timeout="$3" relay_file sub_pid
    MQTT_PUB_OK=true
    relay_file=$(mktemp)
    mosquitto_sub -V mqttv311 -h 127.0.0.1 -p 11883 \
        -u "${TEST_DOMAIN_UUID}-browser" -P browser_test_001 \
        -t "${TEST_DOMAIN_UUID}/browsers/AGENT/STATUS" -C 1 -W "${timeout}" \
        >"${relay_file}" 2>/dev/null &
    sub_pid=$!
    sleep 0.2
    mosquitto_pub -V mqttv311 -h 127.0.0.1 -p 11883 \
        -u "${TEST_DOMAIN_UUID}-agent" -P test_mqtt_pass \
        -t "${TEST_DOMAIN_UUID}/AGENT/TEST_PHIDGET/${topic}" \
        -m "${payload}" >/dev/null 2>&1 || MQTT_PUB_OK=false
    wait "${sub_pid}" 2>/dev/null
    MQTT_RELAY=$(cat "${relay_file}")
    rm -f "${relay_file}"
}

phi_db() {
    db_domain_query "SELECT $1 FROM agent_phidget WHERE agent_tech_id='TEST_PHIDGET' LIMIT 1;"
}

PHI_HEARTBEAT_SQL="DATE_FORMAT(heartbeat_time, '%Y-%m-%d %H:%i:%s')"
PHI_OLD_HEARTBEAT="2000-01-01 00:00:00"
PHI_STATUS_RESTORE=$(phi_db "HEX(agent_status)")
PHI_ALIVE_RESTORE=$(phi_db "agent_alive")
PHI_HEARTBEAT_RESTORE=$(phi_db "${PHI_HEARTBEAT_SQL}")
PHI_MQTT_LOCAL_RESTORE=$(phi_db "mqtt_local_connected")
if ! db_domain_query "UPDATE agent_phidget SET agent_status='waiting for status test', agent_alive=0, heartbeat_time='${PHI_OLD_HEARTBEAT}', mqtt_local_connected=1 WHERE agent_tech_id='TEST_PHIDGET';" >/dev/null; then
    _test_start
    _test_fail "Préparation du statut MQTT" "échec SQL"
fi

if ! command -v mosquitto_pub >/dev/null 2>&1 || ! command -v mosquitto_sub >/dev/null 2>&1; then
    _test_start
    _test_fail "Tests MQTT AGENT/STATUS" "mosquitto_pub ou mosquitto_sub introuvable"
else
    log_info "Test: MQTT AGENT/TEST_PHIDGET/STATUS avec status et alive=true - persiste et relaie"
    _test_start
    mqtt_agent_exchange STATUS '{"status":"running from test","alive":true,"mqtt_local_connected":false}' 5
    DB_HEARTBEAT=$(phi_db "${PHI_HEARTBEAT_SQL}")
    DB_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db mqtt_local_connected)"
    if [[ "${MQTT_PUB_OK}" == "true" && "${DB_STATE}" == "running from test|1|0" ]] && \
       echo "${MQTT_RELAY}" | jq -e --arg db_heartbeat "${DB_HEARTBEAT}" --arg old "${PHI_OLD_HEARTBEAT}" \
        '.agent_tech_id == "TEST_PHIDGET" and .agent_classe == "phidget" and
         .status == "running from test" and .alive == true and .mqtt_local_connected == false and
         .heartbeat_time == $db_heartbeat and .heartbeat_time != $old and
         (.heartbeat_time | test("^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}$"))' >/dev/null 2>&1; then
        _test_pass "MQTT STATUS alive=true met à jour agent_status, agent_alive, heartbeat_time et relaie aux browsers"
    else
        _test_fail "MQTT STATUS alive=true" "payload='${MQTT_RELAY}', base='${DB_STATE}|${DB_HEARTBEAT}'"
    fi

    log_info "Test: MQTT AGENT/TEST_PHIDGET/STATUS signe de vie sans status - conserve agent_status"
    _test_start
    db_domain_query "UPDATE agent_phidget SET heartbeat_time='${PHI_OLD_HEARTBEAT}' WHERE agent_tech_id='TEST_PHIDGET';" >/dev/null
    mqtt_agent_exchange STATUS '{"alive":true,"mqtt_local_connected":true}' 5
    DB_HEARTBEAT=$(phi_db "${PHI_HEARTBEAT_SQL}")
    DB_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db mqtt_local_connected)"
    if [[ "${MQTT_PUB_OK}" == "true" && "${DB_STATE}" == "running from test|1|1" ]] && \
       echo "${MQTT_RELAY}" | jq -e --arg db_heartbeat "${DB_HEARTBEAT}" --arg old "${PHI_OLD_HEARTBEAT}" \
        '.agent_tech_id == "TEST_PHIDGET" and (has("status") | not) and .alive == true and
         .mqtt_local_connected == true and .heartbeat_time == $db_heartbeat and .heartbeat_time != $old' >/dev/null 2>&1; then
        _test_pass "MQTT signe de vie met à jour heartbeat_time sans toucher agent_status"
    else
        _test_fail "MQTT signe de vie" "payload='${MQTT_RELAY}', base='${DB_STATE}|${DB_HEARTBEAT}'"
    fi

    log_info "Test: MQTT AGENT/TEST_PHIDGET/STATUS alive=false - DOWN sans heartbeat"
    _test_start
    BEFORE_HEARTBEAT=$(phi_db "${PHI_HEARTBEAT_SQL}")
    mqtt_agent_exchange STATUS '{"status":"Agent is stopped (0)","alive":false,"mqtt_local_connected":false}' 5
    DB_HEARTBEAT=$(phi_db "${PHI_HEARTBEAT_SQL}")
    DB_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db mqtt_local_connected)"
    if [[ "${MQTT_PUB_OK}" == "true" && "${DB_STATE}" == "Agent is stopped (0)|0|1" && "${DB_HEARTBEAT}" == "${BEFORE_HEARTBEAT}" ]] && \
       echo "${MQTT_RELAY}" | jq -e \
        '.agent_tech_id == "TEST_PHIDGET" and .status == "Agent is stopped (0)" and .alive == false and
         (has("heartbeat_time") | not) and (has("mqtt_local_connected") | not)' >/dev/null 2>&1; then
        _test_pass "MQTT STATUS alive=false passe agent_alive à 0 sans modifier heartbeat_time"
    else
        _test_fail "MQTT STATUS alive=false" "payload='${MQTT_RELAY}', base='${DB_STATE}|${DB_HEARTBEAT}', avant='${BEFORE_HEARTBEAT}'"
    fi

    log_info "Test: MQTT AGENT/TEST_PHIDGET/STATUS sans champ alive - rejeté"
    _test_start
    BEFORE_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db "${PHI_HEARTBEAT_SQL}")"
    mqtt_agent_exchange STATUS '{"status":"ignored from test"}' 2
    AFTER_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db "${PHI_HEARTBEAT_SQL}")"
    if [[ "${MQTT_PUB_OK}" == "true" && -z "${MQTT_RELAY}" && "${AFTER_STATE}" == "${BEFORE_STATE}" ]]; then
        _test_pass "MQTT STATUS sans alive n'est ni persisté ni relayé"
    else
        _test_fail "MQTT STATUS sans alive" "payload='${MQTT_RELAY}', avant='${BEFORE_STATE}', après='${AFTER_STATE}'"
    fi

    log_info "Test: MQTT AGENT/TEST_PHIDGET/HEARTBEAT - topic abandonné, ignoré"
    _test_start
    BEFORE_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db "${PHI_HEARTBEAT_SQL}")"
    mqtt_agent_exchange HEARTBEAT '{"alive":true,"mqtt_local_connected":true}' 2
    AFTER_STATE="$(phi_db agent_status)|$(phi_db agent_alive)|$(phi_db "${PHI_HEARTBEAT_SQL}")"
    if [[ "${MQTT_PUB_OK}" == "true" && -z "${MQTT_RELAY}" && "${AFTER_STATE}" == "${BEFORE_STATE}" ]]; then
        _test_pass "MQTT HEARTBEAT n'est plus traité par l'API"
    else
        _test_fail "MQTT HEARTBEAT ignoré" "payload='${MQTT_RELAY}', avant='${BEFORE_STATE}', après='${AFTER_STATE}'"
    fi
fi

if ! db_domain_query "UPDATE agent_phidget SET agent_status=UNHEX('${PHI_STATUS_RESTORE}'), agent_alive=${PHI_ALIVE_RESTORE:-0}, heartbeat_time='${PHI_HEARTBEAT_RESTORE}', mqtt_local_connected=${PHI_MQTT_LOCAL_RESTORE:-0} WHERE agent_tech_id='TEST_PHIDGET';" >/dev/null; then
    _test_start
    _test_fail "Restauration du statut MQTT" "échec SQL"
fi

print_suite_summary "Suite 12 - Endpoints Phidget"
[[ ${TESTS_FAILED} -eq 0 ]]
