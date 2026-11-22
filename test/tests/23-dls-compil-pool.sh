#!/usr/bin/env bash
# =============================================================================
# 23-dls-compil-pool.sh - Compilation asynchrone DLS et pool partage
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 23 - Pool de compilation DLS"

ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)
PREFIX="TPOOL_$$"
PUBLIC_IDS=("${PREFIX}_P1" "${PREFIX}_P2" "${PREFIX}_P3" "${PREFIX}_P4")
BAD_ID="${PREFIX}_BAD"
PRIVATE_ID="${PREFIX}_PRIVATE"
SINGLE_ID="${PREFIX}_SINGLE"
SOURCE_ID="${PREFIX}_SOURCES"
PACKAGE_NAME="${PREFIX}_PKG"
SINGLE_PACKAGE="${PREFIX}_ONE"
SYN_PAGE="${PREFIX}_PAGE"
LOCK_NAME="${PREFIX}_LOCK"
TRIGGER_NAME="${PREFIX}_FAIL_SAVE"
LOCK_PID=""
SINGLE_PID=""
VALID_PID=""
INVALID_PID=""
TEMP_FILES=()
FIXTURES_CREATED=false
TRIGGER_CREATED=false

wait_pool_idle() {
    local response consecutive=0
    for ((i=0; i<300; i++)); do
        response=$(curl -fsS --max-time 2 "${API_URL}/status" 2>/dev/null) || response=""
        if echo "${response}" | jq -e '.dls_compil_pool_busy == 0 and .dls_compil_pool_queued == 0' >/dev/null 2>&1; then
            consecutive=$((consecutive + 1))
            [[ ${consecutive} -ge 2 ]] && return 0
        else
            consecutive=0
        fi
        sleep 0.2
    done
    return 1
}

wait_db_lock() {
    local held
    for ((i=0; i<100; i++)); do
        held=$(db_domain_query "SELECT IS_USED_LOCK('${LOCK_NAME}') IS NOT NULL;")
        [[ "${held}" == "1" ]] && return 0
        sleep 0.1
    done
    return 1
}

wait_pool_state() {
    local busy="$1" queued_min="$2" http_busy_min="${3:-0}"
    local current_busy current_queued current_http_busy
    for ((i=0; i<60; i++)); do
        POOL_STATUS=$(curl -fsS --max-time 2 "${API_URL}/status" 2>/dev/null) || POOL_STATUS=""
        current_busy=$(echo "${POOL_STATUS}" | jq -r '.dls_compil_pool_busy // -1' 2>/dev/null)
        current_queued=$(echo "${POOL_STATUS}" | jq -r '.dls_compil_pool_queued // -1' 2>/dev/null)
        current_http_busy=$(echo "${POOL_STATUS}" | jq -r '.http_pool_busy // -1' 2>/dev/null)
        if [[ "${current_busy}" == "${busy}" && "${current_queued}" =~ ^[0-9]+$ &&
              "${current_http_busy}" =~ ^[0-9]+$ ]] &&
           (( current_queued >= queued_min && current_http_busy >= http_busy_min )); then
            return 0
        fi
        sleep 0.1
    done
    return 1
}

db_root_query() {
    "${DB_CLIENT}" -h "${DB_HOST}" -P "${DB_PORT}" -u root -p"${DB_ROOT_PASS:-rootpass}" \
        --connect-timeout=3 --silent --skip-column-names -e "$1" 2>/dev/null
}

wait_source_updates() {
    local expected="$1"
    local blocked held
    for ((i=0; i<100; i++)); do
        held=$(db_domain_query "SELECT IS_USED_LOCK('${LOCK_NAME}') IS NOT NULL;")
        [[ "${held}" == "1" ]] || return 1
        blocked=$(db_root_query "SELECT COUNT(*) FROM information_schema.PROCESSLIST
                                 WHERE USER='${TEST_DOMAIN_UUID}'
                                 AND INFO LIKE 'UPDATE %syns_motifs%SET used=0 WHERE dls_id=${SOURCE_DLS_ID}%';")
        [[ "${blocked}" == "${expected}" ]] && return 0
        sleep 0.1
    done
    return 1
}

cleanup() {
    [[ -n "${LOCK_PID}" ]] && wait "${LOCK_PID}" 2>/dev/null
    [[ -n "${SINGLE_PID}" ]] && wait "${SINGLE_PID}" 2>/dev/null
    [[ -n "${VALID_PID}" ]] && wait "${VALID_PID}" 2>/dev/null
    [[ -n "${INVALID_PID}" ]] && wait "${INVALID_PID}" 2>/dev/null
    if [[ "${TRIGGER_CREATED}" == true ]]; then
        db_domain_query "DROP TRIGGER IF EXISTS \`${TRIGGER_NAME}\`;" >/dev/null
    fi
    if [[ "${FIXTURES_CREATED}" == true ]]; then
        if wait_pool_idle; then
            db_domain_query "DELETE FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}','${BAD_ID}','${PRIVATE_ID}','${SINGLE_ID}','${SOURCE_ID}');
                             DELETE FROM dls_packages WHERE name IN ('${PACKAGE_NAME}','${SINGLE_PACKAGE}');
                             DELETE FROM syns WHERE page='${SYN_PAGE}';" >/dev/null
        else
            log_warn "Pool encore actif : conservation des fixtures ${PREFIX}"
        fi
    fi
    local file
    for file in "${TEMP_FILES[@]}"; do rm -f -- "${file}"; done
}
trap cleanup EXIT

POOL_STATUS=$(curl -fsS --max-time 2 "${API_URL}/status" 2>/dev/null)
_test_start
if echo "${POOL_STATUS}" | jq -e \
    '.dls_compil_pool_thread_nbr == 2 and
     (.dls_compil_pool_busy | type == "number") and
     (.dls_compil_pool_queued | type == "number")' >/dev/null 2>&1; then
    _test_pass "GET /status expose le pool DLS configure avec 2 workers"
else
    _test_fail "Configuration du pool DLS inattendue" "${POOL_STATUS}"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

if ! wait_pool_idle; then
    _test_start
    _test_fail "Pool DLS disponible avant les tests" "Des compilations restent actives"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

EXISTING=$(db_domain_query "SELECT (SELECT COUNT(*) FROM syns WHERE page='${SYN_PAGE}') +
                                  (SELECT COUNT(*) FROM dls_packages WHERE name IN ('${PACKAGE_NAME}','${SINGLE_PACKAGE}')) +
                                  (SELECT COUNT(*) FROM information_schema.TRIGGERS WHERE TRIGGER_SCHEMA=DATABASE() AND TRIGGER_NAME='${TRIGGER_NAME}') +
                                  (SELECT COUNT(*) FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}','${BAD_ID}','${PRIVATE_ID}','${SINGLE_ID}','${SOURCE_ID}'));")
if [[ "${EXISTING}" != "0" ]]; then
    _test_start
    _test_fail "Creation des fixtures du pool" "La page ${SYN_PAGE} existe deja ou la base est indisponible"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi
FIXTURES_CREATED=true
db_domain_query "INSERT INTO syns (parent_id, libelle, page, access_level)
                 VALUES (1, 'Pool compilation test', '${SYN_PAGE}', 9);
                 INSERT INTO dls_packages (name, description, sourcecode)
                 VALUES ('${PACKAGE_NAME}', 'Pool test', '/* Pool test */'),
                        ('${SINGLE_PACKAGE}', 'Single pool test', '/* Single pool test */');
                 INSERT INTO dls (tech_id, syn_id, name, shortname, package, sourcecode)
                 VALUES ('${PUBLIC_IDS[0]}', 1, 'Pool 1', 'Pool 1', '${PACKAGE_NAME}', '/* Pool test */'),
                        ('${PUBLIC_IDS[1]}', 1, 'Pool 2', 'Pool 2', '${PACKAGE_NAME}', '/* Pool test */'),
                        ('${PUBLIC_IDS[2]}', 1, 'Pool 3', 'Pool 3', '${PACKAGE_NAME}', '/* Pool test */'),
                        ('${PUBLIC_IDS[3]}', 1, 'Pool 4', 'Pool 4', '${PACKAGE_NAME}', '/* Pool test */'),
                        ('${BAD_ID}', 1, 'Pool invalid', 'Pool invalid', 'custom', '@'),
                        ('${PRIVATE_ID}', (SELECT syn_id FROM syns WHERE page='${SYN_PAGE}'), 'Pool private', 'Private', '${PACKAGE_NAME}', '/* Pool test */'),
                        ('${SINGLE_ID}', (SELECT syn_id FROM syns WHERE page='${SYN_PAGE}'), 'Pool single', 'Single', '${SINGLE_PACKAGE}', '/* Pool test */'),
                        ('${SOURCE_ID}', (SELECT syn_id FROM syns WHERE page='${SYN_PAGE}'), 'Pool sources', 'Sources', 'custom', '/* Initial source */');
                 INSERT INTO mnemos_VISUEL (tech_id, acronyme, libelle)
                 VALUES ('${SOURCE_ID}', 'POOL_GUARD', 'Pool source guard');
                 INSERT INTO syns_motifs (dls_id, mnemo_visuel_id)
                 SELECT d.dls_id, m.mnemo_visuel_id FROM dls AS d
                 INNER JOIN mnemos_VISUEL AS m ON m.tech_id=d.tech_id
                 WHERE d.tech_id='${SOURCE_ID}' AND m.acronyme='POOL_GUARD';" >/dev/null
if [[ $? -ne 0 ]]; then
    _test_start
    _test_fail "Creation des fixtures du pool" "Echec SQL"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

PACKAGE_ID=$(db_domain_query "SELECT dls_package_id FROM dls_packages WHERE name='${PACKAGE_NAME}';")
SINGLE_PACKAGE_ID=$(db_domain_query "SELECT dls_package_id FROM dls_packages WHERE name='${SINGLE_PACKAGE}';")
ELIGIBLE=$(db_domain_query "SELECT COUNT(*) FROM dls AS d INNER JOIN syns AS s USING (syn_id) WHERE s.access_level <= 6;")
SYS_BEFORE=$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='SYS';")
TEST_DLS_BEFORE=$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='TEST_DLS';")

RESPONSE=$(api_call POST /dls/compil_all "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')
assert_http_status 403 "POST /dls/compil_all refuse un utilisateur en lecture seule"

db_domain_query "START TRANSACTION; SELECT dls_id FROM dls FOR UPDATE;
                 DO GET_LOCK('${LOCK_NAME}', 1); DO SLEEP(10); COMMIT; DO RELEASE_LOCK('${LOCK_NAME}');" >/dev/null &
LOCK_PID=$!
if ! wait_db_lock; then
    _test_start
    _test_fail "Verrouillage des plugins pour le test de concurrence" "Verrou SQL indisponible"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

RESPONSE=$(api_call POST /dls/compil_all "${USER_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')
assert_http_status 200 "Premier lot complet accepte sans attendre les compilations"
RESPONSE=$(api_call POST /dls/compil_all "${USER_TOKEN}" "${TEST_DOMAIN_UUID}" '{}')
assert_http_status 200 "Deuxieme lot complet accepte sur le meme pool"

_test_start
EXPECTED_QUEUE=$((2 * ELIGIBLE - 2))
if wait_pool_state 2 "${EXPECTED_QUEUE}" &&
   [[ "$(db_domain_query "SELECT IS_USED_LOCK('${LOCK_NAME}') IS NOT NULL;")" == "1" ]]; then
    _test_pass "Deux lots partagent 2 workers et mettent tous les autres plugins en attente"
else
    _test_fail "Limite globale et mise en file des deux lots" "attente >= ${EXPECTED_QUEUE}, status=${POOL_STATUS}"
fi

wait "${LOCK_PID}"
LOCK_PID=""
_test_start
if wait_pool_idle; then
    _test_pass "Les deux lots complets se terminent"
else
    _test_fail "Fin des deux lots complets" "Pool toujours actif"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

PUBLIC_COMPILED=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}')
                                   AND nbr_compil=2 AND compil_status=1 AND error_count=0 AND compil_user='User Test';")
BAD_COMPILED=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id='${BAD_ID}' AND nbr_compil=2 AND error_count>0;")
PRIVATE_UNTOUCHED=$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id IN ('${PRIVATE_ID}','${SINGLE_ID}') AND nbr_compil=0;")
SYS_AFTER=$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='SYS';")
TEST_DLS_AFTER=$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='TEST_DLS';")
_test_start
if [[ "${PUBLIC_COMPILED}" == "4" && "${BAD_COMPILED}" == "1" && "${PRIVATE_UNTOUCHED}" == "2" &&
      "${SYS_AFTER}" == "$((SYS_BEFORE + 2))" && "${TEST_DLS_AFTER}" == "$((TEST_DLS_BEFORE + 2))" ]]; then
    _test_pass "Tous les plugins autorises sont compiles deux fois, meme en presence d'une erreur DLS"
else
    _test_fail "Resultat des deux lots complets" \
        "valides=${PUBLIC_COMPILED}, invalide=${BAD_COMPILED}, prives=${PRIVATE_UNTOUCHED}, SYS=${SYS_AFTER}, TEST_DLS=${TEST_DLS_AFTER}"
fi

RESPONSE=$(api_call POST /dls/compil_all "${USER_TOKEN}" "${TEST_DOMAIN_UUID}" "{\"dls_package_id\":${PACKAGE_ID}}")
assert_http_status 200 "Lot filtre par package accepte"
_test_start
if wait_pool_idle &&
   [[ "$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}') AND nbr_compil=3;")" == "4" &&
      "$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='${PRIVATE_ID}';")" == "0" &&
      "$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='${BAD_ID}';")" == "2" &&
      "$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='SYS';")" == "${SYS_AFTER}" ]]; then
    _test_pass "Le filtre package et les droits limitent les plugins compiles"
else
    _test_fail "Filtre package ou droits incorrects" "Les compteurs de compilation ne correspondent pas"
fi

RESPONSE=$(api_call POST /dls/compil_all "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "{\"dls_package_id\":${PACKAGE_ID}}")
assert_http_status 200 "Lot package administrateur accepte"
_test_start
if wait_pool_idle &&
   [[ "$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}') AND nbr_compil=4;")" == "4" &&
      "$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='${PRIVATE_ID}';")" == "1" ]]; then
    _test_pass "L'administrateur compile aussi le plugin prive du package"
else
    _test_fail "Compilation du package administrateur" "Le plugin prive ou les plugins publics manquent"
fi

RESPONSE=$(api_call POST /dls/compil_all "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" '{"dls_package_id":-1}')
assert_http_status 200 "Lot vide accepte"
_test_start
if wait_pool_idle &&
   [[ "$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='${PRIVATE_ID}';")" == "1" ]]; then
    _test_pass "Un package inexistant ne compile aucun plugin"
else
    _test_fail "Lot vide" "Un plugin a ete compile ou le pool reste actif"
fi

db_domain_query "START TRANSACTION;
                 SELECT dls_id FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}','${SINGLE_ID}') FOR UPDATE;
                 DO GET_LOCK('${LOCK_NAME}', 1); DO SLEEP(10); COMMIT; DO RELEASE_LOCK('${LOCK_NAME}');" >/dev/null &
LOCK_PID=$!
if ! wait_db_lock; then
    _test_start
    _test_fail "Verrouillage des plugins pour les tokens distincts" "Verrou SQL indisponible"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

RESPONSE=$(api_call POST /dls/compil_all "${USER_TOKEN}" "${TEST_DOMAIN_UUID}" "{\"dls_package_id\":${PACKAGE_ID}}")
assert_http_status 200 "Lot utilisateur accepte pendant le verrouillage"
RESPONSE=$(api_call POST /dls/compil_all "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "{\"dls_package_id\":${SINGLE_PACKAGE_ID}}")
assert_http_status 200 "Lot administrateur accepte pendant le verrouillage"
_test_start
if wait_pool_state 2 3 &&
   [[ "$(db_domain_query "SELECT IS_USED_LOCK('${LOCK_NAME}') IS NOT NULL;")" == "1" ]]; then
    _test_pass "Les deux requetes retournent pendant que leurs compilations sont en attente"
else
    _test_fail "Mise en file des requetes de deux utilisateurs" "status=${POOL_STATUS}"
fi

wait "${LOCK_PID}"
LOCK_PID=""
_test_start
if wait_pool_idle &&
   [[ "$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id IN ('${PUBLIC_IDS[0]}','${PUBLIC_IDS[1]}','${PUBLIC_IDS[2]}','${PUBLIC_IDS[3]}') AND nbr_compil=5 AND compil_user='User Test';")" == "4" &&
      "$(db_domain_query "SELECT COUNT(*) FROM dls WHERE tech_id='${SINGLE_ID}' AND nbr_compil=1 AND compil_user='Admin Test';")" == "1" ]]; then
    _test_pass "Chaque tache conserve son plugin et le token de sa requete apres la reponse HTTP"
else
    _test_fail "Conservation des plugins et tokens par tache" "Les resultats ou utilisateurs sauvegardes ne correspondent pas"
fi

SINGLE_BEFORE=$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='${SINGLE_ID}';")
db_domain_query "START TRANSACTION; SELECT dls_id FROM dls WHERE tech_id='${SINGLE_ID}' FOR UPDATE;
                 DO GET_LOCK('${LOCK_NAME}', 1); DO SLEEP(10); COMMIT; DO RELEASE_LOCK('${LOCK_NAME}');" >/dev/null &
LOCK_PID=$!
if ! wait_db_lock; then
    _test_start
    _test_fail "Verrouillage du plugin partage" "Verrou SQL indisponible"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi
RESPONSE=$(api_call POST /dls/compil_all "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "{\"dls_package_id\":${SINGLE_PACKAGE_ID}}")
assert_http_status 200 "Premier lot du plugin partage accepte"
RESPONSE=$(api_call POST /dls/compil_all "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "{\"dls_package_id\":${SINGLE_PACKAGE_ID}}")
assert_http_status 200 "Deuxieme lot du plugin partage accepte"

SINGLE_OUTPUT=$(mktemp)
TEMP_FILES+=("${SINGLE_OUTPUT}")
curl -s --max-time 40 -w '\n%{http_code}\n' -X POST "${API_URL}/dls/compil" \
    -H "Content-Type: application/json" -H "Authorization: Bearer ${ADMIN_TOKEN}" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" -d "{\"tech_id\":\"${SINGLE_ID}\"}" >"${SINGLE_OUTPUT}" &
SINGLE_PID=$!

_test_start
CONCURRENT=false
BLOCKED=0
if wait_pool_state 2 0 2; then
    for ((i=0; i<80; i++)); do
        BLOCKED=$(db_root_query "SELECT COUNT(*) FROM information_schema.PROCESSLIST
                                 WHERE USER='${TEST_DOMAIN_UUID}' AND INFO LIKE
                                 CONCAT('UPDATE dls SET nbr_compil=nbr_compil+1 WHERE tech_id=', CHAR(39), '${SINGLE_ID}', CHAR(39), '%');")
        if [[ "${BLOCKED}" == "3" &&
              "$(db_domain_query "SELECT IS_USED_LOCK('${LOCK_NAME}') IS NOT NULL;")" == "1" ]]; then
            CONCURRENT=true
            break
        fi
        sleep 0.1
    done
fi
if [[ "${CONCURRENT}" == true ]]; then
    _test_pass "Deux lots et une compilation unitaire executent simultanement le meme plugin"
else
    _test_fail "Concurrence du plugin partage" "3 requetes SQL attendues, observees=${BLOCKED}, status=${POOL_STATUS}"
fi

wait "${LOCK_PID}"
LOCK_PID=""
wait "${SINGLE_PID}"
SINGLE_PID=""
_test_start
if [[ "$(tail -n 1 "${SINGLE_OUTPUT}")" == "200" ]] && wait_pool_idle &&
   [[ "$(db_domain_query "SELECT nbr_compil FROM dls WHERE tech_id='${SINGLE_ID}';")" == "$((SINGLE_BEFORE + 3))" ]]; then
    _test_pass "Les deux lots et la compilation unitaire se terminent tous"
else
    _test_fail "Fin des compilations du plugin partage" "$(tail -n 2 "${SINGLE_OUTPUT}")"
fi

SOURCE_DLS_ID=$(db_domain_query "SELECT dls_id FROM dls WHERE tech_id='${SOURCE_ID}';")
db_domain_query "START TRANSACTION; SELECT syn_motif_id FROM syns_motifs WHERE dls_id=${SOURCE_DLS_ID} FOR UPDATE;
                 DO GET_LOCK('${LOCK_NAME}', 1); DO SLEEP(10); COMMIT; DO RELEASE_LOCK('${LOCK_NAME}');" >/dev/null &
LOCK_PID=$!
if ! wait_db_lock; then
    _test_start
    _test_fail "Verrouillage du motif pour les sources concurrentes" "Verrou SQL indisponible"
    print_suite_summary "Suite 23 - Pool de compilation DLS"
    exit 1
fi

VALID_OUTPUT=$(mktemp)
INVALID_OUTPUT=$(mktemp)
TEMP_FILES+=("${VALID_OUTPUT}" "${INVALID_OUTPUT}")
VALID_PAYLOAD=$(jq -cn --arg tech_id "${SOURCE_ID}" '{tech_id:$tech_id,sourcecode:"/* Valid source */\n"}')
INVALID_PAYLOAD=$(jq -cn --arg tech_id "${SOURCE_ID}" '{tech_id:$tech_id,sourcecode:"@"}')

curl -s --max-time 40 -w '\n%{http_code}\n' -X POST "${API_URL}/dls/compil" \
    -H "Content-Type: application/json" -H "Authorization: Bearer ${ADMIN_TOKEN}" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" -d "${VALID_PAYLOAD}" >"${VALID_OUTPUT}" &
VALID_PID=$!
FIRST_SOURCE_BLOCKED=false
if wait_source_updates 1; then FIRST_SOURCE_BLOCKED=true; fi

curl -s --max-time 40 -w '\n%{http_code}\n' -X POST "${API_URL}/dls/compil" \
    -H "Content-Type: application/json" -H "Authorization: Bearer ${ADMIN_TOKEN}" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" -d "${INVALID_PAYLOAD}" >"${INVALID_OUTPUT}" &
INVALID_PID=$!

_test_start
if [[ "${FIRST_SOURCE_BLOCKED}" == true ]] && wait_source_updates 2; then
    _test_pass "Deux sources distinctes du meme plugin sont pretes a etre analysees en parallele"
else
    _test_fail "Preparation simultanee de sources distinctes" "Les deux compilations n'ont pas atteint le meme blocage SQL"
fi

wait "${LOCK_PID}"
LOCK_PID=""
wait "${VALID_PID}"
VALID_PID=""
wait "${INVALID_PID}"
INVALID_PID=""
_test_start
if [[ "$(tail -n 1 "${VALID_OUTPUT}")" == "200" && "$(tail -n 1 "${INVALID_OUTPUT}")" == "200" ]] &&
   sed '$d' "${VALID_OUTPUT}" | jq -e --arg tech_id "${SOURCE_ID}" \
       '.tech_id == $tech_id and .compil_status == true and .error_count == 0' >/dev/null 2>&1 &&
   sed '$d' "${INVALID_OUTPUT}" | jq -e --arg tech_id "${SOURCE_ID}" \
       '.tech_id == $tech_id and .compil_status == true and .error_count > 0' >/dev/null 2>&1; then
    _test_pass "Chaque compilation analyse sa propre source"
else
    _test_fail "Isolation des sources concurrentes" "valide=$(cat "${VALID_OUTPUT}"), invalide=$(cat "${INVALID_OUTPUT}")"
fi

SAVED_SOURCE_OK=$(db_domain_query "SELECT CASE
    WHEN sourcecode=CONCAT('/* Valid source */', CHAR(10)) THEN error_count=0 AND codec LIKE '%void Go%' AND nbr_ligne=2
    WHEN sourcecode='@' THEN error_count>0 AND codec='No CodeC error' AND nbr_ligne=1
    ELSE 0 END FROM dls WHERE tech_id='${SOURCE_ID}';")
_test_start
if [[ "${SAVED_SOURCE_OK}" == "1" ]]; then
    _test_pass "La source et le resultat sauvegardes correspondent a une meme compilation"
else
    _test_fail "Coherence du resultat sauvegarde" "La source, le code genere et les erreurs ne correspondent pas"
fi

EMPTY_PAYLOAD=$(jq -cn --arg tech_id "${SOURCE_ID}" '{tech_id:$tech_id,sourcecode:""}')
RESPONSE=$(api_call POST /dls/compil "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${EMPTY_PAYLOAD}")
assert_http_status 200 "Une source DLS vide peut etre compilee"
_test_start
if echo "${RESPONSE}" | jq -e '.compil_status == true and .error_count == 0' >/dev/null 2>&1 &&
   [[ "$(db_domain_query "SELECT sourcecode='' AND nbr_ligne=1 FROM dls WHERE tech_id='${SOURCE_ID}';")" == "1" ]]; then
    _test_pass "Une source vide est analysee et sauvegardee sans erreur"
else
    _test_fail "Compilation d'une source vide" "${RESPONSE}"
fi

TRIGGER_CREATED=true
_test_start
if db_domain_query "CREATE TRIGGER \`${TRIGGER_NAME}\` BEFORE UPDATE ON dls FOR EACH ROW
                    SET NEW.tech_id=IF(NEW.tech_id='${SOURCE_ID}' AND NEW.sourcecode='/* Reject save */', NULL, NEW.tech_id);" >/dev/null; then
    _test_pass "Injection d'une erreur de sauvegarde SQL"
    FAILURE_PAYLOAD=$(jq -cn --arg tech_id "${SOURCE_ID}" '{tech_id:$tech_id,sourcecode:"/* Reject save */"}')
    RESPONSE=$(api_call POST /dls/compil "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${FAILURE_PAYLOAD}")
    assert_http_status 500 "Une erreur de sauvegarde renvoie HTTP 500"
    _test_start
    if echo "${RESPONSE}" | jq -e \
       '.compil_status == false and .errorlog == "Unable to save compilation results"' >/dev/null 2>&1 &&
       [[ "$(db_domain_query "SELECT sourcecode='' AND nbr_ligne=1 FROM dls WHERE tech_id='${SOURCE_ID}';")" == "1" ]]; then
        _test_pass "Une sauvegarde refusee signale l'erreur et conserve la source precedente"
    else
        _test_fail "Gestion d'une sauvegarde refusee" "${RESPONSE}"
    fi
else
    _test_fail "Injection d'une erreur de sauvegarde SQL" "Creation du trigger impossible"
fi
if db_domain_query "DROP TRIGGER IF EXISTS \`${TRIGGER_NAME}\`;" >/dev/null; then
    TRIGGER_CREATED=false
fi

CONFIG_FILE=$(mktemp)
TEMP_FILES+=("${CONFIG_FILE}")
for invalid in 0 -1 1.5 '"2"' 2147483648; do
    printf '{"dls_compil_pool_thread_nbr":%s,"idp_token_check":false,"db_hostname":"127.0.0.1","db_port":1}\n' "${invalid}" >"${CONFIG_FILE}"
    _test_start
    if ABLS_CONFIG_FILE="${CONFIG_FILE}" "${SCRIPT_DIR}/../../build/abls-habitat-api" >/dev/null 2>&1; then
        _test_fail "Configuration invalide ${invalid} refusee" "L'API a retourne un code de succes"
    else
        _test_pass "Configuration invalide ${invalid} refusee au demarrage"
    fi
done

print_suite_summary "Suite 23 - Pool de compilation DLS"
[[ ${TESTS_FAILED} -eq 0 ]]
