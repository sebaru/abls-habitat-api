#!/usr/bin/env bash
# =============================================================================
# teardown.sh - Nettoie l'environnement de test ABLS-Habitat
# =============================================================================
# Usage: bash test/teardown.sh [--keep-db]
# =============================================================================

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib/colors.sh"

KEEP_DB=false
log_info()  { echo -e "${BLUE}[INFO]${RESET} $*"; }
log_ok()    { echo -e "${GREEN}[ OK ]${RESET} $*"; }

COMPOSE_CMD=()
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

for arg in "$@"; do
    [[ "${arg}" == "--keep-db" ]] && KEEP_DB=true
done

detect_compose_cmd || true

# Arrêter le processus API s'il a été démarré par setup.sh
PID_FILE="${SCRIPT_DIR}/results/.api.pid"
if [[ -f "${PID_FILE}" ]]; then
    API_PID=$(cat "${PID_FILE}")
    if kill -0 "${API_PID}" &>/dev/null; then
        log_info "Arrêt de l'API (PID: ${API_PID})..."
        kill "${API_PID}" && log_ok "API arrêtée"
    fi
    rm -f "${PID_FILE}"
fi

# Arrêter Compose
if [[ "${KEEP_DB}" == false ]]; then
    log_info "Arrêt et suppression des containers MariaDB..."
    cd "${SCRIPT_DIR}"
    if [[ ${#COMPOSE_CMD[@]} -gt 0 ]]; then
        compose down -v 2>/dev/null || true
    elif command -v podman &>/dev/null; then
        podman rm -f "${PODMAN_MASTER_CONTAINER}" "${PODMAN_ARCH_CONTAINER}" "${PODMAN_MQTT_CONTAINER}" &>/dev/null || true
        podman volume rm -f "${PODMAN_MASTER_VOLUME}" "${PODMAN_ARCH_VOLUME}" &>/dev/null || true
    fi
    log_ok "Containers et volumes supprimés"
else
    log_info "Conservation des containers MariaDB (--keep-db)"
    if [[ ${#COMPOSE_CMD[@]} -gt 0 ]]; then
        compose stop 2>/dev/null || true
    elif command -v podman &>/dev/null; then
        podman stop "${PODMAN_MASTER_CONTAINER}" "${PODMAN_ARCH_CONTAINER}" "${PODMAN_MQTT_CONTAINER}" &>/dev/null || true
    fi
fi

log_ok "Environnement de test nettoyé"
