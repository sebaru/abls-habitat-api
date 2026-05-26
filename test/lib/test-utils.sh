#!/usr/bin/env bash
# =============================================================================
# test-utils.sh - Utilitaires pour les tests fonctionnels de l'API ABLS-Habitat
# =============================================================================
# Usage: source test/lib/test-utils.sh depuis un fichier de test
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/colors.sh"

# =============================================================================
# Configuration par défaut (overridable via variables d'env)
# =============================================================================
API_URL="${API_URL:-http://localhost:15562}"
DB_HOST="${DB_HOST:-127.0.0.1}"
DB_PORT="${DB_PORT:-13306}"
DB_ARCH_PORT="${DB_ARCH_PORT:-13307}"
DB_USER="${DB_USER:-abls_test}"
DB_PASS="${DB_PASS:-abls_test_pass}"
DB_NAME="${DB_NAME:-master}"
DB_CLIENT="${DB_CLIENT:-}"

# UUIDs de test définis dans test-data.sql
TEST_DOMAIN_UUID="aaaaaaaa-0000-0000-0000-000000000001"
TEST_ADMIN_UUID="bbbbbbbb-0000-0000-0000-000000000001"
TEST_USER_UUID="cccccccc-0000-0000-0000-000000000001"
TEST_READONLY_UUID="dddddddd-0000-0000-0000-000000000001"
TEST_DISABLED_UUID="eeeeeeee-0000-0000-0000-000000000001"

# IDP URL attendu par l'API (doit matcher idp_url dans la config test)
IDP_URL="https://idp.abls-habitat.fr"

# Compteurs globaux de résultats
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Variables pour les derniers résultats d'appels API
LAST_HTTP_CODE=0
LAST_RESPONSE=""

# =============================================================================
# Fonctions d'affichage
# =============================================================================
log_info()  { echo -e "${BLUE}${ICON_INFO}${RESET} $*"; }
log_warn()  { echo -e "${YELLOW}${ICON_WARN}${RESET} $*"; }
log_pass()  { echo -e "${GREEN}${ICON_PASS}${RESET} $*"; }
log_fail()  { echo -e "${RED}${ICON_FAIL}${RESET} $*"; }
log_suite() { echo -e "\n${BOLD}${BLUE}══════════════════════════════════════${RESET}"; \
              echo -e "${BOLD}${BLUE}  $*${RESET}"; \
              echo -e "${BOLD}${BLUE}══════════════════════════════════════${RESET}"; }

detect_db_client() {
    if [[ -n "${DB_CLIENT}" ]] && command -v "${DB_CLIENT}" &>/dev/null; then
        return 0
    fi
    if command -v mariadb &>/dev/null; then
        DB_CLIENT="mariadb"
    elif command -v mysql &>/dev/null; then
        DB_CLIENT="mysql"
    else
        log_fail "Client SQL introuvable (mariadb/mysql)"
        return 1
    fi
}

detect_db_client >/dev/null || true

jwt_profile_defaults() {
    local user_uuid="$1"
    local email="$2"

    case "${user_uuid}:${email}" in
        "${TEST_ADMIN_UUID}:admin@test.abls-habitat.fr")
            printf '%s\t%s\t%s\t%s\n' "Admin Test" "Admin Test" "Admin" "Test"
            ;;
        "${TEST_USER_UUID}:user@test.abls-habitat.fr")
            printf '%s\t%s\t%s\t%s\n' "User Test" "User Test" "User" "Test"
            ;;
        "${TEST_READONLY_UUID}:readonly@test.abls-habitat.fr")
            printf '%s\t%s\t%s\t%s\n' "Readonly Test" "Readonly Test" "Readonly" "Test"
            ;;
        "${TEST_DISABLED_UUID}:disabled@test.abls-habitat.fr")
            printf '%s\t%s\t%s\t%s\n' "Disabled Test" "Disabled Test" "Disabled" "Test"
            ;;
        *)
            printf '%s\t%s\t%s\t%s\n' "${email}" "${email}" "Test" "User"
            ;;
    esac
}

build_test_jwt() {
    local user_uuid="$1"
    local email="$2"
    local email_verified="$3"
    local issuer="$4"
    local exp_epoch="$5"
    local defaults preferred_username full_name given_name family_name

    defaults="$(jwt_profile_defaults "${user_uuid}" "${email}")"
    preferred_username="$(printf '%s' "${defaults}" | cut -f1)"
    full_name="$(printf '%s' "${defaults}" | cut -f2)"
    given_name="$(printf '%s' "${defaults}" | cut -f3)"
    family_name="$(printf '%s' "${defaults}" | cut -f4)"

    local header='{"alg":"none","typ":"JWT"}'
    local payload
    payload=$(cat <<EOF
{"sub":"${user_uuid}","email":"${email}","email_verified":${email_verified},"iss":"${issuer}","exp":${exp_epoch},"iat":$(date +%s),"preferred_username":"${preferred_username}","name":"${full_name}","given_name":"${given_name}","family_name":"${family_name}"}
EOF
)

    local b64_header b64_payload
    b64_header=$(echo -n "${header}"  | base64 | tr '+/' '-_' | tr -d '=\n')
    b64_payload=$(echo -n "${payload}" | base64 | tr '+/' '-_' | tr -d '=\n')

    echo "${b64_header}.${b64_payload}."
}

# =============================================================================
# generate_jwt: Génère un JWT minimal valide (sans signature, pour idp_token_check=false)
#
# Avec idp_token_check=false, l'API décode le token mais ne vérifie pas la signature.
# On peut donc créer un JWT factice mais structurellement valide.
#
# Arguments:
#   $1 = user_uuid (sub)
#   $2 = email
#   $3 = access_level (ignoré ici, l'API le relit depuis la BD)
#
# Retourne le token JWT complet (header.payload.signature)
# =============================================================================
generate_jwt() {
    local user_uuid="$1"
    local email="$2"
    local exp_epoch
    exp_epoch=$(date -d "+2 hours" +%s 2>/dev/null || date -v +2H +%s 2>/dev/null)

    build_test_jwt "${user_uuid}" "${email}" true "${IDP_URL}/realms/Abls-Habitat" "${exp_epoch}"
}

# generate_jwt_exp: comme generate_jwt mais avec une valeur d'expiration personnalisée
# Arguments: $1=user_uuid, $2=email, $3=access_level (ignoré), $4=exp (epoch timestamp)
generate_jwt_exp() {
    local user_uuid="$1"
    local email="$2"
    local exp_epoch="$4"

    build_test_jwt "${user_uuid}" "${email}" true "${IDP_URL}/realms/Abls-Habitat" "${exp_epoch}"
}

generate_jwt_custom() {
    local user_uuid="$1"
    local email="$2"
    local email_verified="$3"
    local issuer="$4"
    local exp_epoch="$5"

    build_test_jwt "${user_uuid}" "${email}" "${email_verified}" "${issuer}" "${exp_epoch}"
}

# =============================================================================
# api_call: Appel HTTP vers l'API avec headers JWT et X-ABLS-DOMAIN
#
# Arguments:
#   $1 = méthode HTTP (GET, POST, DELETE)
#   $2 = path de l'endpoint (ex: /camera/list)
#   $3 = JWT token
#   $4 = domain_uuid (X-ABLS-DOMAIN header)
#   $5 = corps JSON (optionnel)
#
# Retourne: réponse JSON sur stdout + code HTTP sur stderr (via LAST_HTTP_CODE)
# =============================================================================
LAST_HTTP_CODE=0
LAST_HTTP_CODE_FILE="${SCRIPT_DIR}/../results/.last_http_code"

extract_http_response_parts() {
    local full_response="$1"
    local code body

    code="${full_response##*__HTTP_CODE:}"
    code="${code%%$'\n'*}"
    body="${full_response%__HTTP_CODE:*}"
    body="${body%$'\n'}"

    echo "${code}"
    echo "${body}"
}

set_last_http_code() {
    local code="$1"
    LAST_HTTP_CODE="${code}"
    mkdir -p "$(dirname "${LAST_HTTP_CODE_FILE}")" 2>/dev/null || true
    printf "%s" "${code}" > "${LAST_HTTP_CODE_FILE}" 2>/dev/null || true
}

set_last_response() {
    local response="$1"
    LAST_RESPONSE="${response}"
}

refresh_last_http_code() {
    if [[ -f "${LAST_HTTP_CODE_FILE}" ]]; then
        LAST_HTTP_CODE="$(cat "${LAST_HTTP_CODE_FILE}" 2>/dev/null || echo "${LAST_HTTP_CODE}")"
    fi
}

api_call() {
    local method="$1"
    local path="$2"
    local jwt="$3"
    local domain_uuid="${4:-}"
    local data="${5:-}"

    local curl_args=(
        -s
        --max-time 15
        -w "\n__HTTP_CODE:%{http_code}"
        -X "${method}"
        "${API_URL}${path}"
        -H "Content-Type: application/json"
    )

    [[ -n "${jwt}" ]]         && curl_args+=(-H "Authorization: Bearer ${jwt}")
    [[ -n "${domain_uuid}" ]] && curl_args+=(-H "X-ABLS-DOMAIN: ${domain_uuid}")
    [[ -n "${data}" ]]        && curl_args+=(-d "${data}")

    local full_response
    full_response=$(curl "${curl_args[@]}" 2>/dev/null)

    local parsed code body
    parsed="$(extract_http_response_parts "${full_response}")"
    code="$(echo "${parsed}" | sed -n '1p')"
    body="$(echo "${parsed}" | sed -n '2,$p')"
    set_last_http_code "${code}"
    set_last_response "${body}"
    echo "${body}"
}

# =============================================================================
# api_call_no_auth: Appel HTTP sans authentification (pour /ping, /status)
# =============================================================================
api_call_no_auth() {
    local method="$1"
    local path="$2"

    local full_response
    full_response=$(curl -s --max-time 15 -w "\n__HTTP_CODE:%{http_code}" \
        -X "${method}" \
        "${API_URL}${path}" \
        -H "Content-Type: application/json" 2>/dev/null)

    local parsed code body
    parsed="$(extract_http_response_parts "${full_response}")"
    code="$(echo "${parsed}" | sed -n '1p')"
    body="$(echo "${parsed}" | sed -n '2,$p')"
    set_last_http_code "${code}"
    set_last_response "${body}"
    echo "${body}"
}

# =============================================================================
# db_query: Exécute une requête SQL sur la BD MariaDB de test
#
# Arguments:
#   $1 = requête SQL
#   $2 = nom de la base (optionnel, par défaut DB_NAME)
#
# Retourne: résultat tabulaire sur stdout
# =============================================================================
db_query() {
    local sql="$1"
    local db="${2:-${DB_NAME}}"

    "${DB_CLIENT}" \
        -h "${DB_HOST}" \
        -P "${DB_PORT}" \
        -u "${DB_USER}" \
        -p"${DB_PASS}" \
        --connect-timeout=3 \
        --silent \
        --skip-column-names \
        "${db}" \
        -e "${sql}" 2>/dev/null
}

# Alias pour la DB domaine de test
db_domain_query() {
    local sql="$1"
    local domain_db="${TEST_DOMAIN_UUID}"
    db_query "${sql}" "${domain_db}"
}

# =============================================================================
# Fonctions d'assertion
# =============================================================================

# Incrémente le compteur de tests
_test_start() {
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
}

# Marque un test comme réussi
_test_pass() {
    TESTS_PASSED=$((TESTS_PASSED + 1))
    log_pass "$1"
}

# Marque un test comme échoué
_test_fail() {
    TESTS_FAILED=$((TESTS_FAILED + 1))
    log_fail "$1"
    [[ -n "${2:-}" ]] && echo -e "       ${DIM}Détail: $2${RESET}"
}

# Fonction helper pour obtenir le message de statut HTTP
get_http_status_message() {
    local code="$1"
    
    case "${code}" in
        200) echo "OK" ;;
        201) echo "Created" ;;
        204) echo "No Content" ;;
        400) echo "Bad Request" ;;
        401) echo "Unauthorized" ;;
        403) echo "Forbidden" ;;
        404) echo "Not Found" ;;
        405) echo "Method Not Allowed" ;;
        500) echo "Internal Server Error" ;;
        502) echo "Bad Gateway" ;;
        503) echo "Service Unavailable" ;;
        000) echo "No Response" ;;
        *) echo "HTTP ${code}" ;;
    esac
}

# Fonction helper pour extraire le message d'erreur de la réponse JSON
get_error_message_from_response() {
    local response="$1"
    local error_msg
    
    # Essayer d'extraire api_error ou error de la réponse JSON
    error_msg=$(echo "${response}" | jq -r '.api_error // .error // empty' 2>/dev/null)
    
    if [[ -n "${error_msg}" ]]; then
        echo "${error_msg}"
    fi
}

# ----------------------------------------------------------------------------
# assert_http_status: Vérifie que le dernier appel a retourné le bon code HTTP
#
# Usage: assert_http_status <expected_code> <test_name>
# Affiche aussi le message de statut HTTP et le message d'erreur API si présent
# ----------------------------------------------------------------------------
assert_http_status() {
    local expected="$1"
    local test_name="$2"
    _test_start

    refresh_last_http_code

    local status_msg
    status_msg=$(get_http_status_message "${LAST_HTTP_CODE}")

    if [[ "${LAST_HTTP_CODE}" == "${expected}" ]]; then
        _test_pass "${test_name} (HTTP ${LAST_HTTP_CODE} ${status_msg})"
        return 0
    else
        local expected_status_msg
        expected_status_msg=$(get_http_status_message "${expected}")
        local error_detail="attendu: ${expected} ${expected_status_msg}, reçu: ${LAST_HTTP_CODE} ${status_msg}"
        local error_msg
        
        # Essayer d'extraire le message d'erreur de la réponse
        if [[ -n "${LAST_RESPONSE}" ]]; then
            error_msg=$(get_error_message_from_response "${LAST_RESPONSE}")
            if [[ -n "${error_msg}" ]]; then
                error_detail="${error_detail} - ${error_msg}"
            fi
        fi
        
        _test_fail "${test_name}" "${error_detail}"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_json_field: Vérifie qu'un champ JSON a la valeur attendue
#
# Usage: assert_json_field <json_string> <field_path> <expected_value> <test_name>
# La valeur attendue peut être "not_empty" pour vérifier juste la présence
# ----------------------------------------------------------------------------
assert_json_field() {
    local json="$1"
    local field="$2"
    local expected="$3"
    local test_name="$4"
    _test_start

    # Extraire la valeur avec jq
    local actual
    actual=$(echo "${json}" | jq -r "(try .${field} catch empty) | if . == null then empty else . end" 2>/dev/null)

    if [[ "${expected}" == "not_empty" ]]; then
        if [[ -n "${actual}" && "${actual}" != "null" ]]; then
            _test_pass "${test_name} (.${field} = '${actual}')"
            return 0
        else
            _test_fail "${test_name}" ".${field} est vide ou null dans: ${json}"
            return 1
        fi
    elif [[ "${actual}" == "${expected}" ]]; then
        _test_pass "${test_name} (.${field} = '${expected}')"
        return 0
    else
        _test_fail "${test_name}" ".${field}: attendu='${expected}', reçu='${actual}'"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_json_array_not_empty: Vérifie qu'un tableau JSON n'est pas vide
# ----------------------------------------------------------------------------
assert_json_array_not_empty() {
    local json="$1"
    local array_field="$2"
    local test_name="$3"
    _test_start

    local length
    length=$(echo "${json}" | jq ".${array_field} | length" 2>/dev/null)

    if [[ -n "${length}" && "${length}" -gt 0 ]]; then
        _test_pass "${test_name} (${array_field}: ${length} éléments)"
        return 0
    else
        _test_fail "${test_name}" "${array_field} est vide ou absent dans: ${json}"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_db_count: Vérifie le nombre de lignes dans une table
#
# Usage: assert_db_count <table> <expected_count> <test_name> [where_clause]
# ----------------------------------------------------------------------------
assert_db_count() {
    local table="$1"
    local expected="$2"
    local test_name="$3"
    local where="${4:-}"
    _test_start

    local sql="SELECT COUNT(*) FROM \`${table}\`"
    [[ -n "${where}" ]] && sql="${sql} WHERE ${where}"

    local actual
    actual=$(db_domain_query "${sql};")

    if [[ "${actual}" == "${expected}" ]]; then
        _test_pass "${test_name} (${table}: ${expected} ligne(s))"
        return 0
    else
        _test_fail "${test_name}" "${table} COUNT: attendu=${expected}, actuel=${actual}"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_db_field: Vérifie la valeur d'un champ en base de données
#
# Usage: assert_db_field <table> <field> <expected> <test_name> <where_clause>
# ----------------------------------------------------------------------------
assert_db_field() {
    local table="$1"
    local field="$2"
    local expected="$3"
    local test_name="$4"
    local where="$5"
    _test_start

    local actual
    actual=$(db_domain_query "SELECT \`${field}\` FROM \`${table}\` WHERE ${where} LIMIT 1;")

    if [[ "${actual}" == "${expected}" ]]; then
        _test_pass "${test_name} (${table}.${field} = '${expected}')"
        return 0
    else
        _test_fail "${test_name}" "${table}.${field}: attendu='${expected}', actuel='${actual}'"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_db_row_exists: Vérifie qu'une ligne existe en base
# ----------------------------------------------------------------------------
assert_db_row_exists() {
    local table="$1"
    local test_name="$2"
    local where="$3"
    _test_start

    local count
    count=$(db_domain_query "SELECT COUNT(*) FROM \`${table}\` WHERE ${where};")

    if [[ "${count}" -gt 0 ]]; then
        _test_pass "${test_name} (ligne trouvée dans ${table})"
        return 0
    else
        _test_fail "${test_name}" "Aucune ligne trouvée dans ${table} WHERE ${where}"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_db_row_absent: Vérifie qu'une ligne n'existe PAS en base
# ----------------------------------------------------------------------------
assert_db_row_absent() {
    local table="$1"
    local test_name="$2"
    local where="$3"
    _test_start

    local count
    count=$(db_domain_query "SELECT COUNT(*) FROM \`${table}\` WHERE ${where};")

    if [[ "${count}" -eq 0 ]]; then
        _test_pass "${test_name} (ligne absente de ${table}, correct)"
        return 0
    else
        _test_fail "${test_name}" "${count} ligne(s) trouvée(s) dans ${table} WHERE ${where}"
        return 1
    fi
}

# ----------------------------------------------------------------------------
# assert_api_master_db_row_exists: Vérifie dans la DB master (pour user, domains)
# ----------------------------------------------------------------------------
assert_master_db_field() {
    local table="$1"
    local field="$2"
    local expected="$3"
    local test_name="$4"
    local where="$5"
    _test_start

    local actual
    actual=$(db_query "SELECT \`${field}\` FROM \`${table}\` WHERE ${where} LIMIT 1;" "${DB_NAME}")

    if [[ "${actual}" == "${expected}" ]]; then
        _test_pass "${test_name} (master.${table}.${field} = '${expected}')"
        return 0
    else
        _test_fail "${test_name}" "master.${table}.${field}: attendu='${expected}', actuel='${actual}'"
        return 1
    fi
}

# =============================================================================
# wait_for_api: Attend que l'API soit disponible (max N secondes)
# =============================================================================
wait_for_api() {
    local max_wait="${1:-30}"
    local waited=0

    log_info "Attente de l'API sur ${API_URL}..."
    while [[ ${waited} -lt ${max_wait} ]]; do
        if curl -s --max-time 1 "${API_URL}/ping" | grep -q PONG 2>/dev/null; then
            log_info "API disponible après ${waited}s"
            return 0
        fi
        sleep 1
        waited=$((waited + 1))
    done

    log_fail "API non disponible après ${max_wait}s sur ${API_URL}"
    return 1
}

# =============================================================================
# wait_for_db: Attend que MariaDB soit disponible
# =============================================================================
wait_for_db() {
    local max_wait="${1:-60}"
    local waited=0

    log_info "Attente de MariaDB sur ${DB_HOST}:${DB_PORT}..."
    while [[ ${waited} -lt ${max_wait} ]]; do
        if "${DB_CLIENT}" -h "${DB_HOST}" -P "${DB_PORT}" -u "${DB_USER}" -p"${DB_PASS}" \
                 --connect-timeout=2 -e "SELECT 1;" "${DB_NAME}" &>/dev/null; then
            log_info "MariaDB disponible après ${waited}s"
            return 0
        fi
        sleep 2
        waited=$((waited + 2))
    done

    log_fail "MariaDB non disponible après ${max_wait}s"
    return 1
}

# =============================================================================
# print_suite_summary: Affiche le bilan d'une suite de tests
# =============================================================================
print_suite_summary() {
    local suite_name="${1:-Suite}"
    local failed="${2:-${TESTS_FAILED}}"
    local total="${3:-${TESTS_TOTAL}}"
    local passed=$((total - failed))

    echo ""
    if [[ ${failed} -eq 0 ]]; then
        echo -e "${GREEN}${BOLD}✓ ${suite_name}: ${passed}/${total} tests OK${RESET}"
    else
        echo -e "${RED}${BOLD}✗ ${suite_name}: ${passed}/${total} OK, ${failed} ÉCHEC(S)${RESET}"
    fi
    echo "PASSED=${passed}"
    echo "FAILED=${failed}"
    echo "TOTAL=${total}"
}

# =============================================================================
# Token helpers: crée les tokens pour chaque rôle de test
# =============================================================================
make_admin_token()    { generate_jwt "${TEST_ADMIN_UUID}"    "admin@test.abls-habitat.fr"; }
make_user_token()     { generate_jwt "${TEST_USER_UUID}"     "user@test.abls-habitat.fr"; }
make_readonly_token() { generate_jwt "${TEST_READONLY_UUID}" "readonly@test.abls-habitat.fr"; }
make_disabled_token() { generate_jwt "${TEST_DISABLED_UUID}" "disabled@test.abls-habitat.fr"; }
