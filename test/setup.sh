#!/usr/bin/env bash
# =============================================================================
# setup.sh - Prépare l'environnement de test ABLS-Habitat
# =============================================================================
# Usage: bash test/setup.sh [--start-api]
#
# Prérequis: podman (compose ou mode direct), ou docker compose, client MariaDB/MySQL, curl, jq
# =============================================================================

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib/colors.sh"

API_PORT="${API_PORT:-15562}"
DB_PORT="${DB_PORT:-13306}"
DB_ARCH_PORT="${DB_ARCH_PORT:-13307}"
MQTT_PORT="${MQTT_PORT:-11883}"
START_API=false
DB_CLIENT="${DB_CLIENT:-}"

log_info()  { echo -e "${BLUE}[INFO]${RESET} $*"; }
log_ok()    { echo -e "${GREEN}[ OK ]${RESET} $*"; }
log_error() { echo -e "${RED}[ERRO]${RESET} $*" >&2; }

COMPOSE_CMD=()
CONTAINER_RUNTIME=""
PODMAN_MASTER_CONTAINER="abls-test-mariadb"
PODMAN_ARCH_CONTAINER="abls-test-mariadb-arch"
PODMAN_MQTT_CONTAINER="abls-test-mqtt"
PODMAN_MASTER_VOLUME="abls-test-data"
PODMAN_ARCH_VOLUME="abls-test-arch-data"

detect_compose_cmd() {
    # Priorité à Podman Compose, puis fallback Docker Compose.
    if command -v podman &>/dev/null && podman compose version &>/dev/null; then
        COMPOSE_CMD=(podman compose)
        return 0
    fi
    if command -v podman-compose &>/dev/null; then
        COMPOSE_CMD=(podman-compose)
        return 0
    fi
    if command -v docker &>/dev/null && docker compose version &>/dev/null; then
        COMPOSE_CMD=(docker compose)
        return 0
    fi
    return 1
}

compose() {
    "${COMPOSE_CMD[@]}" "$@"
}

detect_db_client() {
    if [[ -n "${DB_CLIENT}" ]] && command -v "${DB_CLIENT}" &>/dev/null; then
        return 0
    fi
    if command -v mariadb &>/dev/null; then
        DB_CLIENT="mariadb"
    elif command -v mysql &>/dev/null; then
        DB_CLIENT="mysql"
    else
        return 1
    fi
}

start_mariadb_with_podman() {
    log_info "Démarrage de MariaDB (podman direct)..."

    podman rm -f "${PODMAN_MASTER_CONTAINER}" "${PODMAN_ARCH_CONTAINER}" &>/dev/null || true
    podman volume create "${PODMAN_MASTER_VOLUME}" &>/dev/null || true
    podman volume create "${PODMAN_ARCH_VOLUME}" &>/dev/null || true

    podman run -d \
        --name "${PODMAN_MASTER_CONTAINER}" \
        -p "${DB_PORT}:3306" \
        -e MARIADB_ROOT_PASSWORD=rootpass \
        -e MARIADB_DATABASE=abls_master \
        -e MARIADB_USER=abls_test \
        -e MARIADB_PASSWORD=abls_test_pass \
        -v "${PODMAN_MASTER_VOLUME}:/var/lib/mysql" \
        docker.io/library/mariadb:11 &>/dev/null

    podman run -d \
        --name "${PODMAN_ARCH_CONTAINER}" \
        -p "${DB_ARCH_PORT}:3306" \
        -e MARIADB_ROOT_PASSWORD=rootpass \
        -e MARIADB_DATABASE=abls_arch \
        -e MARIADB_USER=abls_test \
        -e MARIADB_PASSWORD=abls_test_pass \
        -v "${PODMAN_ARCH_VOLUME}:/var/lib/mysql" \
        docker.io/library/mariadb:11 &>/dev/null
}

start_mqtt_with_podman() {
    log_info "Démarrage de MQTT (podman direct)..."
    podman rm -f "${PODMAN_MQTT_CONTAINER}" &>/dev/null || true
    podman run -d \
        --name "${PODMAN_MQTT_CONTAINER}" \
        -p "${MQTT_PORT}:1883" \
        -v "${SCRIPT_DIR}/config/mosquitto-test.conf:/mosquitto/config/mosquitto.conf:ro" \
        docker.io/library/eclipse-mosquitto:2 &>/dev/null
}

wait_for_tcp_port() {
    local host="$1"
    local port="$2"
    local max_wait="$3"
    local waited=0
    while ! (echo > "/dev/tcp/${host}/${port}") >/dev/null 2>&1; do
        sleep 1
        waited=$((waited + 1))
        if [[ ${waited} -ge ${max_wait} ]]; then
            return 1
        fi
    done
    return 0
}

# Parse args
for arg in "$@"; do
    [[ "${arg}" == "--start-api" ]] && START_API=true
done

# =============================================================================
# Vérification des prérequis
# =============================================================================
log_info "Vérification des prérequis..."

for cmd in curl jq; do
    if ! command -v "${cmd}" &>/dev/null; then
        log_error "Commande manquante: ${cmd}"
        exit 1
    fi
done

if ! detect_db_client; then
    log_error "Commande manquante: client SQL (mariadb/mysql)"
    exit 1
fi

if detect_compose_cmd; then
    CONTAINER_RUNTIME="compose"
elif command -v podman &>/dev/null; then
    CONTAINER_RUNTIME="podman-direct"
else
    log_error "Runtime conteneur manquant (podman ou docker compose)"
    exit 1
fi

if [[ "${CONTAINER_RUNTIME}" == "compose" ]]; then
    log_ok "Tous les prérequis sont présents (${COMPOSE_CMD[*]}, ${DB_CLIENT})"
else
    log_ok "Tous les prérequis sont présents (podman direct, ${DB_CLIENT})"
fi

# =============================================================================
# Démarrage de MariaDB
# =============================================================================
cd "${SCRIPT_DIR}"
if [[ "${CONTAINER_RUNTIME}" == "compose" ]]; then
    log_info "Démarrage de MariaDB (${COMPOSE_CMD[*]})..."
    compose up -d mariadb mariadb-arch mqtt
else
    start_mariadb_with_podman
    start_mqtt_with_podman
fi

# Attente readiness MariaDB (master)
log_info "Attente de MariaDB (master) sur port ${DB_PORT}..."
MAX_WAIT=60
waited=0
until "${DB_CLIENT}" -h 127.0.0.1 -P "${DB_PORT}" -u abls_test -pabls_test_pass \
            --connect-timeout=3 abls_master -e "SELECT 1;" &>/dev/null; do
    sleep 2
    waited=$((waited + 2))
    if [[ ${waited} -ge ${MAX_WAIT} ]]; then
        log_error "MariaDB master non disponible après ${MAX_WAIT}s"
        if [[ "${CONTAINER_RUNTIME}" == "compose" ]]; then
            compose logs mariadb
        else
            podman logs "${PODMAN_MASTER_CONTAINER}" || true
        fi
        exit 1
    fi
done
log_ok "MariaDB master disponible"

# Attente readiness MariaDB (archive)
log_info "Attente de MariaDB (archive) sur port ${DB_ARCH_PORT}..."
waited=0
until "${DB_CLIENT}" -h 127.0.0.1 -P "${DB_ARCH_PORT}" -u abls_test -pabls_test_pass \
            --connect-timeout=3 abls_arch -e "SELECT 1;" &>/dev/null; do
    sleep 2
    waited=$((waited + 2))
    if [[ ${waited} -ge ${MAX_WAIT} ]]; then
        log_error "MariaDB archive non disponible après ${MAX_WAIT}s"
        if [[ "${CONTAINER_RUNTIME}" == "compose" ]]; then
            compose logs mariadb-arch
        else
            podman logs "${PODMAN_ARCH_CONTAINER}" || true
        fi
        exit 1
    fi
done
log_ok "MariaDB archive disponible"

# Attente readiness MQTT
log_info "Attente de MQTT (test) sur port ${MQTT_PORT}..."
if ! wait_for_tcp_port "127.0.0.1" "${MQTT_PORT}" 30; then
    log_error "MQTT non disponible après 30s"
    if [[ "${CONTAINER_RUNTIME}" == "compose" ]]; then
        compose logs mqtt || true
    else
        podman logs "${PODMAN_MQTT_CONTAINER}" || true
    fi
    exit 1
fi
log_ok "MQTT test disponible"

# =============================================================================
# Chargement des fixtures
# =============================================================================
log_info "Chargement des fixtures (test-data.sql)..."

# Donner les droits root pour les GRANT dans le SQL
"${DB_CLIENT}" -h 127.0.0.1 -P "${DB_PORT}" \
      -u root -prootpass \
      < "${SCRIPT_DIR}/config/test-data.sql"

log_ok "Fixtures chargées"

# Vérification: compter les enregistrements attendus
USERS_COUNT=$("${DB_CLIENT}" -h 127.0.0.1 -P "${DB_PORT}" -u abls_test -pabls_test_pass \
              --silent --skip-column-names abls_master \
              -e "SELECT COUNT(*) FROM users;" 2>/dev/null)
DOMAINS_COUNT=$("${DB_CLIENT}" -h 127.0.0.1 -P "${DB_PORT}" -u abls_test -pabls_test_pass \
               --silent --skip-column-names abls_master \
               -e "SELECT COUNT(*) FROM domains;" 2>/dev/null)

log_ok "Fixtures: ${USERS_COUNT} users, ${DOMAINS_COUNT} domaines en base"

# =============================================================================
# Démarrage optionnel de l'API
# =============================================================================
if [[ "${START_API}" == true ]]; then
    API_BINARY="${SCRIPT_DIR}/../build/abls-habitat-api"
    API_CONF="${SCRIPT_DIR}/config/abls-habitat-api.test.conf"

    if [[ ! -x "${API_BINARY}" ]]; then
        log_error "Binaire API non trouvé: ${API_BINARY}"
        log_error "Compilez d'abord avec: cd API && bash build.sh"
        exit 1
    fi

    if [[ ! -f "${API_CONF}" ]]; then
        log_error "Config API de test non trouvée: ${API_CONF}"
        exit 1
    fi

    # Le binaire lit /etc/abls-habitat-api.conf; on surcharge via ABLS_*.
    while IFS=$'\t' read -r key value; do
        [[ -z "${key}" ]] && continue
        env_key="ABLS_$(echo "${key}" | tr '[:lower:]' '[:upper:]')"
        export "${env_key}=${value}"
    done < <(jq -r 'to_entries[] | "\(.key)\t\(.value|tostring)"' "${API_CONF}")

    log_info "Démarrage de l'API (port ${API_PORT})..."
    "${API_BINARY}" &
    API_PID=$!
    echo "${API_PID}" > "${SCRIPT_DIR}/results/.api.pid"

    # Attente que l'API réponde. /ping nécessite un token, on utilise /status.
    waited=0
    until [[ "$(curl -s -o /dev/null -w "%{http_code}" --max-time 2 "http://localhost:${API_PORT}/status" 2>/dev/null)" =~ ^(200|500)$ ]]; do
        if ! kill -0 "${API_PID}" 2>/dev/null; then
            log_error "Le processus API s'est arrêté prématurément"
            exit 1
        fi
        sleep 1
        waited=$((waited + 1))
        if [[ ${waited} -ge 30 ]]; then
            log_error "API non disponible après 30s"
            kill "${API_PID}" 2>/dev/null || true
            exit 1
        fi
    done
    log_ok "API démarrée (PID: ${API_PID}, port: ${API_PORT})"
fi

# =============================================================================
# Résumé
# =============================================================================
echo ""
echo -e "${GREEN}${BOLD}══════════════════════════════════════════════${RESET}"
echo -e "${GREEN}${BOLD}  Environnement de test prêt !${RESET}"
echo -e "${GREEN}${BOLD}══════════════════════════════════════════════${RESET}"
echo -e "  MariaDB master: ${BOLD}127.0.0.1:${DB_PORT}${RESET} (abls_master)"
echo -e "  MariaDB arch:   ${BOLD}127.0.0.1:${DB_ARCH_PORT}${RESET} (abls_arch)"
echo -e "  MQTT test:      ${BOLD}127.0.0.1:${MQTT_PORT}${RESET}"
echo -e "  API attendue:   ${BOLD}http://localhost:${API_PORT}${RESET}"
echo ""
echo -e "  Identifiants BD: abls_test / abls_test_pass"
echo ""
echo -e "  Pour démarrer l'API manuellement:"
echo -e "  ${DIM}./build/abls-habitat-api${RESET}"
echo ""
echo -e "  Pour lancer les tests:"
echo -e "  ${DIM}bash test/run-all-tests.sh${RESET}"
echo ""
