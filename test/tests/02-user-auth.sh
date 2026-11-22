#!/usr/bin/env bash
# =============================================================================
# 02-user-auth.sh - Tests d'authentification et endpoints utilisateurs
# =============================================================================
# Endpoints testés: GET /status, GET /user/profil, GET /user/list, POST /user/set,
#                   POST /user/get, POST /user/set_gps, POST /user/invite,
#                   POST /user/set_domain
# JWT requis pour tous les endpoints. X-ABLS-DOMAIN est requis uniquement pour les endpoints dans un domaine.
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 02 - Authentification et endpoints utilisateurs"

# Préparation des tokens
ADMIN_TOKEN=$(make_admin_token)
USER_TOKEN=$(make_user_token)
READONLY_TOKEN=$(make_readonly_token)
DISABLED_TOKEN=$(make_disabled_token)

# =============================================================================
# TEST: GET /status (access_level ≥ 6 dans le domaine)
# =============================================================================
log_info "Test: GET /status - authentification et droits"
RESPONSE=$(api_call GET /status "" "${TEST_DOMAIN_UUID}")
assert_http_status 401 "GET /status sans token avec domaine → HTTP 401"

RESPONSE=$(api_call GET /status "${ADMIN_TOKEN}" "")
assert_http_status 400 "GET /status sans domaine → HTTP 400"

RESPONSE=$(api_call GET /status "${DISABLED_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 401 "GET /status utilisateur désactivé → HTTP 401"

NO_GRANT_PAYLOAD=$(jq -cn \
    --arg iss "${IDP_URL}/realms/Abls-Habitat" \
    --argjson exp "$(( $(date +%s) + 7200 ))" \
    '{sub:"11111111-0000-0000-0000-000000000001", email:"no-grant@test.invalid",
      email_verified:true, iss:$iss, exp:$exp, access_level:9, enable:true}' \
    | base64 | tr '+/' '-_' | tr -d '=\n')
NO_GRANT_TOKEN="${ADMIN_TOKEN%%.*}.${NO_GRANT_PAYLOAD}."
RESPONSE=$(api_call GET /status "${NO_GRANT_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 401 "GET /status sans droit en base refuse les droits fournis dans le JWT"
RESPONSE=$(api_call GET /status "${NO_GRANT_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 401 "GET /status répété sans droit en base reste refusé"

RESPONSE=$(api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_READONLY_UUID}\",\"access_level\":6}")
assert_http_status 200 "Préparation du niveau 6"
RESPONSE=$(api_call GET /status "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /status niveau 6 avant révocation → HTTP 200"

RESPONSE=$(api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_READONLY_UUID}\",\"access_level\":5}")
assert_http_status 200 "Préparation du niveau 5"
assert_master_db_field "users_grants" "access_level" "5" "Niveau 5 enregistré" \
    "user_uuid='${TEST_READONLY_UUID}' AND domain_uuid='${TEST_DOMAIN_UUID}'"
RESPONSE=$(api_call GET /status "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /status niveau 5 → HTTP 403"
RESPONSE=$(api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_READONLY_UUID}\",\"access_level\":1}")
assert_http_status 200 "Restauration du niveau en lecture seule"
assert_master_db_field "users_grants" "access_level" "1" "Niveau en lecture seule restauré" \
    "user_uuid='${TEST_READONLY_UUID}' AND domain_uuid='${TEST_DOMAIN_UUID}'"

RESPONSE=$(api_call GET /status "${USER_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /status niveau 6 → HTTP 200"
assert_json_field "${RESPONSE}" "product" "ABLS-HABITAT-API" "GET /status product correct"
assert_json_field "${RESPONSE}" "version" "not_empty" "GET /status version présente"
assert_json_field "${RESPONSE}" "vendor" "ABLS-HABITAT" "GET /status vendor correct"
assert_json_field "${RESPONSE}" "api_status" "not_empty" "GET /status api_status présent"
assert_json_field "${RESPONSE}" "idp_realm" "not_empty" "GET /status idp_realm présent"

STATUS_START_TIME=$(echo "${RESPONSE}" | jq -r '.start_time // 0' 2>/dev/null)
STATUS_UPTIME=$(echo "${RESPONSE}" | jq -r '.uptime // -1' 2>/dev/null)
_test_start
if [[ "${STATUS_START_TIME}" =~ ^[0-9]+$ && "${STATUS_START_TIME}" -gt 0 && "${STATUS_UPTIME}" =~ ^[0-9]+$ ]]; then
    _test_pass "GET /status start_time et uptime présents (uptime=${STATUS_UPTIME}s)"
else
    _test_fail "GET /status start_time/uptime invalides" "start_time=${STATUS_START_TIME}, uptime=${STATUS_UPTIME}"
fi

_test_start
if echo "${RESPONSE}" | jq -e '(.mqtt_connected|type)=="boolean" and (.db_cache_enabled|type)=="boolean"' >/dev/null 2>&1; then
    _test_pass "GET /status mqtt_connected et db_cache_enabled sont des booléens"
else
    _test_fail "GET /status mqtt_connected/db_cache_enabled absents ou non booléens" "${RESPONSE}"
fi

_test_start
if echo "${RESPONSE}" | jq -e '(.master_cache_hits|type)=="number" and (.master_cache_misses|type)=="number" and (.master_cache_errors|type)=="number" and (.master_cache_hit_ratio|type)=="number"' >/dev/null 2>&1; then
    _test_pass "GET /status compteurs du cache master présents"
else
    _test_fail "GET /status compteurs du cache master absents ou non numériques" "${RESPONSE}"
fi

NBR_DOMAINS=$(echo "${RESPONSE}" | jq -r '.nbr_domains // 0' 2>/dev/null)
EXPECTED_DOMAINS=$(db_query "SELECT COUNT(*) FROM domains;" master)
EXPECTED_DOMAINS=$((EXPECTED_DOMAINS + 1))
_test_start
if [[ "${NBR_DOMAINS}" == "${EXPECTED_DOMAINS}" ]]; then
    _test_pass "GET /status nbr_domains cohérent avec BD (${NBR_DOMAINS})"
else
    _test_fail "GET /status nbr_domains incohérent" "réponse=${NBR_DOMAINS}, BD=${EXPECTED_DOMAINS}"
fi

RESPONSE=$(api_call GET /status "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /status niveau 9 → HTTP 200"

# =============================================================================
# TEST: Accès sans token
# =============================================================================
log_info "Test: GET /user/profil sans token"
RESPONSE=$(curl -s -w "\n__HTTP_CODE:%{http_code}" \
    -X GET "${API_URL}/user/profil" \
    -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" 2>/dev/null)
LAST_HTTP_CODE="${RESPONSE##*__HTTP_CODE:}"
set_last_http_code "${LAST_HTTP_CODE}"
assert_http_status 401 "GET /user/profil sans token → HTTP 401"

# =============================================================================
# TEST: Accès sans header X-ABLS-DOMAIN sur /user/profil
# =============================================================================
log_info "Test: GET /user/profil sans X-ABLS-DOMAIN"
RESPONSE=$(curl -s -w "\n__HTTP_CODE:%{http_code}" \
    -X GET "${API_URL}/user/profil" \
    -H "Authorization: Bearer ${ADMIN_TOKEN}" 2>/dev/null)
LAST_HTTP_CODE="${RESPONSE##*__HTTP_CODE:}"
set_last_http_code "${LAST_HTTP_CODE}"
# /user/profil est un endpoint authentifié hors domaine
assert_http_status 200 "GET /user/profil sans X-ABLS-DOMAIN → HTTP 200"
assert_json_field "${RESPONSE}" "user_uuid" "${TEST_ADMIN_UUID}" "GET /user/profil sans X-ABLS-DOMAIN retourne le bon user_uuid"

# =============================================================================
# TEST: GET /user/profil avec admin valide
# =============================================================================
log_info "Test: GET /user/profil - admin"
RESPONSE=$(api_call GET /user/profil "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /user/profil admin → HTTP 200"
assert_json_field "${RESPONSE}" "user_uuid" "${TEST_ADMIN_UUID}" "GET /user/profil user_uuid correct"
assert_json_field "${RESPONSE}" "email" "admin@test.abls-habitat.fr" "GET /user/profil email correct"
assert_json_field "${RESPONSE}" "enable" "true" "GET /user/profil enable=true"

# Vérifier que l'access_level retourné correspond à la BD
ADMIN_LEVEL_DB=$(db_query "SELECT access_level FROM users_grants WHERE user_uuid='${TEST_ADMIN_UUID}' AND domain_uuid='${TEST_DOMAIN_UUID}';" master)
ADMIN_LEVEL_API=$(echo "${RESPONSE}" | jq -r '.access_level // empty' 2>/dev/null)
_test_start
if [[ "${ADMIN_LEVEL_API}" == "${ADMIN_LEVEL_DB}" ]]; then
    _test_pass "GET /user/profil access_level cohérent avec BD (${ADMIN_LEVEL_DB})"
else
    _test_fail "GET /user/profil access_level incohérent" "API=${ADMIN_LEVEL_API}, BD=${ADMIN_LEVEL_DB}"
fi

# =============================================================================
# TEST: GET /user/profil avec user standard
# =============================================================================
log_info "Test: GET /user/profil - user standard (level 6)"
RESPONSE=$(api_call GET /user/profil "${USER_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /user/profil user standard → HTTP 200"
assert_json_field "${RESPONSE}" "user_uuid" "${TEST_USER_UUID}" "GET /user/profil user UUID correct"
assert_json_field "${RESPONSE}" "access_level" "6" "GET /user/profil access_level=6"

# =============================================================================
# TEST: GET /user/profil avec user désactivé
# =============================================================================
log_info "Test: GET /user/profil - user désactivé"
RESPONSE=$(api_call GET /user/profil "${DISABLED_TOKEN}" "${TEST_DOMAIN_UUID}")

# /user/profil expose le profil courant, même si l'utilisateur est désactivé
assert_http_status 200 "GET /user/profil user désactivé → HTTP 200"
assert_json_field "${RESPONSE}" "enable" "false" "GET /user/profil user désactivé retourne enable=false"

# =============================================================================
# TEST: GET /user/list (access_level ≥ 6)
# =============================================================================
log_info "Test: GET /user/list - admin"
RESPONSE=$(api_call GET /user/list "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 200 "GET /user/list admin → HTTP 200"
assert_json_array_not_empty "${RESPONSE}" "users" "GET /user/list contient des utilisateurs"

# L'utilisateur admin doit être dans la liste
USER_IN_LIST=$(echo "${RESPONSE}" | jq -r '.users[] | select(.user_uuid == "'${TEST_ADMIN_UUID}'") | .user_uuid' 2>/dev/null)
_test_start
if [[ "${USER_IN_LIST}" == "${TEST_ADMIN_UUID}" ]]; then
    _test_pass "GET /user/list contient l'admin Test"
else
    _test_fail "GET /user/list ne contient pas l'admin Test" "réponse: ${RESPONSE}"
fi

# =============================================================================
# TEST: Tokens JWT invalides
# =============================================================================
log_info "Test: Token JWT expiré (exp dans le passé) → 401/403"
EXPIRED_TOKEN=$(generate_jwt_exp \
    "${TEST_ADMIN_UUID}" \
    "admin@test.abls-habitat.fr" \
    9 \
    1)

RESPONSE=$(api_call GET /status "${EXPIRED_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "GET /status avec token expiré → HTTP 403"

RESPONSE=$(api_call GET /user/list "${EXPIRED_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "Token expiré refusé → HTTP 403"

log_info "Test: Token JWT avec issuer invalide → 401/403"
BAD_ISSUER_TOKEN=$(generate_jwt_custom \
    "${TEST_ADMIN_UUID}" \
    "admin@test.abls-habitat.fr" \
    true \
    "https://evil.attacker.com" \
    9999999999)

RESPONSE=$(api_call GET /user/list "${BAD_ISSUER_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "Token avec issuer invalide refusé → HTTP 403"

log_info "Test: Token JWT avec email_verified=false → 401/403"
UNVERIFIED_EMAIL_TOKEN=$(generate_jwt_custom \
    "${TEST_ADMIN_UUID}" \
    "admin@test.abls-habitat.fr" \
    false \
    "${IDP_URL}/realms/Abls-Habitat" \
    9999999999)

RESPONSE=$(api_call GET /user/list "${UNVERIFIED_EMAIL_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 403 "Token avec email non vérifié refusé → HTTP 403"

# =============================================================================
# TEST: GET /user/list avec user readonly (access_level=1, requis ≥ 6)
# =============================================================================
log_info "Test: GET /user/list - readonly (access insuffisant)"
RESPONSE=$(api_call GET /user/list "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}")

assert_http_status 403 "GET /user/list readonly → HTTP 403 (accès refusé)"

# =============================================================================
# TEST: POST /user/set - Modifier le téléphone de l'utilisateur standard
# =============================================================================
log_info "Test: POST /user/set - modification phone"
RESPONSE=$(api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_USER_UUID}\",\"phone\":\"+33123450001\"}")

assert_http_status 200 "POST /user/set → HTTP 200"

# Vérifier la modification en base
assert_master_db_field "users" "phone" "+33123450001" \
    "POST /user/set mise à jour en BD" \
    "user_uuid='${TEST_USER_UUID}'"

# Remettre la valeur initiale
api_call POST /user/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_USER_UUID}\",\"phone\":\"\"}" >/dev/null

# =============================================================================
# TEST: POST /user/set - sans accès suffisant (readonly, level 1)
# =============================================================================
log_info "Test: POST /user/set - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /user/set "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"user_uuid\":\"${TEST_USER_UUID}\",\"phone\":\"+33999999999\"}")

assert_http_status 403 "POST /user/set readonly → HTTP 403"

# Vérification que la BD n'a PAS été modifiée
assert_master_db_field "users" "phone" "" \
    "POST /user/set non autorisé n'a pas modifié la BD" \
    "user_uuid='${TEST_USER_UUID}'"

# =============================================================================
# TEST: POST /user/get - Récupérer les détails d'un utilisateur
# =============================================================================
log_info "Test: POST /user/get - admin"
RESPONSE=$(api_call POST /user/get "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"target_user_uuid\":\"${TEST_USER_UUID}\"}")

assert_http_status 200 "POST /user/get admin → HTTP 200"
assert_json_field "${RESPONSE}" "user_uuid" "${TEST_USER_UUID}" "POST /user/get user_uuid correct"
assert_json_field "${RESPONSE}" "email" "user@test.abls-habitat.fr" "POST /user/get email correct"

log_info "Test: POST /user/get - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /user/get "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"target_user_uuid\":\"${TEST_USER_UUID}\"}")
assert_http_status 403 "POST /user/get readonly → HTTP 403"

# =============================================================================
# TEST: POST /user/set_gps - Mise à jour de la position GPS
# =============================================================================
log_info "Test: POST /user/set_gps - user standard"
# L'API enregistre la position de l'utilisateur du token (sub)
RESPONSE=$(api_call POST /user/set_gps "${USER_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"latitude":48.8566,"longitude":2.3522}')

assert_http_status 200 "POST /user/set_gps → HTTP 200"

# Vérifier l'upsert en BD
GPS_LAT=$(db_query "SELECT ROUND(latitude, 2) FROM users_gps WHERE user_uuid='${TEST_USER_UUID}' ORDER BY date_time DESC LIMIT 1;" master)
_test_start
if [[ "${GPS_LAT}" == "48.86" ]]; then
    _test_pass "POST /user/set_gps latitude correcte en BD (${GPS_LAT})"
else
    _test_fail "POST /user/set_gps latitude incorrecte en BD" "attendu=48.86, reçu=${GPS_LAT}"
fi

# =============================================================================
# TEST: POST /user/invite - Inviter un utilisateur dans le domaine
# =============================================================================
log_info "Test: POST /user/invite - admin"
INVITES_BEFORE=$(db_query "SELECT COUNT(*) FROM users_invite WHERE domain_uuid='${TEST_DOMAIN_UUID}';" master)

RESPONSE=$(api_call POST /user/invite "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"friend_email":"invite-test@test.abls-habitat.fr","friend_level":6}')

assert_http_status 200 "POST /user/invite → HTTP 200"

INVITES_AFTER=$(db_query "SELECT COUNT(*) FROM users_invite WHERE domain_uuid='${TEST_DOMAIN_UUID}';" master)
_test_start
if [[ "$((INVITES_BEFORE + 1))" == "${INVITES_AFTER}" ]]; then
    _test_pass "POST /user/invite a créé 1 invitation en BD"
else
    _test_fail "POST /user/invite n'a pas créé d'invitation en BD" \
        "avant=${INVITES_BEFORE}, après=${INVITES_AFTER}"
fi

# Nettoyage
db_query "DELETE FROM users_invite WHERE email='invite-test@test.abls-habitat.fr' AND domain_uuid='${TEST_DOMAIN_UUID}';" master >/dev/null 2>&1 || true

log_info "Test: POST /user/invite - readonly (accès insuffisant)"
RESPONSE=$(api_call POST /user/invite "${READONLY_TOKEN}" "${TEST_DOMAIN_UUID}" \
    '{"friend_email":"invite-fail@test.abls-habitat.fr","friend_level":6}')
assert_http_status 403 "POST /user/invite readonly → HTTP 403"

# =============================================================================
# TEST: POST /user/set_domain - Changer le domaine par défaut
# =============================================================================
log_info "Test: POST /user/set_domain - user standard"
RESPONSE=$(api_call POST /user/set_domain "${USER_TOKEN}" "" \
    "{\"domain_uuid\":\"${TEST_DOMAIN_UUID}\"}")

assert_http_status 200 "POST /user/set_domain → HTTP 200"

DEFAULT_DOMAIN_DB=$(db_query "SELECT default_domain_uuid FROM users WHERE user_uuid='${TEST_USER_UUID}';" master)
_test_start
if [[ "${DEFAULT_DOMAIN_DB}" == "${TEST_DOMAIN_UUID}" ]]; then
    _test_pass "POST /user/set_domain default_domain_uuid mis à jour en BD"
else
    _test_fail "POST /user/set_domain default_domain_uuid non mis à jour" \
        "BD=${DEFAULT_DOMAIN_DB}, attendu=${TEST_DOMAIN_UUID}"
fi

print_suite_summary "Suite 02 - User Auth"
[[ ${TESTS_FAILED} -eq 0 ]]
