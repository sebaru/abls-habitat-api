#!/usr/bin/env bash
# =============================================================================
# run-all-tests.sh - Lanceur principal de la suite de tests fonctionnels API
# =============================================================================
# Usage:
#   ./run-all-tests.sh [OPTIONS]
#
# Options:
#   --skip-setup     Ne pas relancer setup.sh (utilise la BD déjà démarrée)
#   --skip-teardown  Ne pas arrêter la BD après les tests
#   --no-start-api   Ne pas lancer l'API automatiquement (comportement inverse)
#   --start-api      Compatibilité: force le lancement automatique de l'API
#   --tests PATTERN  N'exécuter que les suites correspondant au pattern glob
#                    (ex: --tests "02-*" pour n'exécuter que la suite 02)
#   --output FILE    Chemin du fichier rapport (défaut: results/report-<date>.txt)
#   -h, --help       Afficher cette aide
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# -----------------------------------------------------------------------------
# Couleurs et icônes
# -----------------------------------------------------------------------------
source "${SCRIPT_DIR}/lib/colors.sh"

# -----------------------------------------------------------------------------
# Arguments
# -----------------------------------------------------------------------------
SKIP_SETUP=false
SKIP_TEARDOWN=false
START_API=true
TESTS_PATTERN="0*.sh"
OUTPUT_FILE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --skip-setup)     SKIP_SETUP=true    ;;
        --skip-teardown)  SKIP_TEARDOWN=true ;;
        --no-start-api)   START_API=false    ;;
        --start-api)      START_API=true     ;;
        --tests)          shift; TESTS_PATTERN="$1" ;;
        --output)         shift; OUTPUT_FILE="$1"   ;;
        -h|--help)
            sed -n '/^# Usage:/,/^# =====/p' "$0" | sed 's/^# \?//'
            exit 0
            ;;
        *)
            echo "${RED}Argument inconnu: $1${RESET}" >&2
            exit 1
            ;;
    esac
    shift
done

# -----------------------------------------------------------------------------
# Vérification des prérequis
# -----------------------------------------------------------------------------
echo ""
echo "${BOLD}${BLUE}================================================================${RESET}"
echo "${BOLD}${BLUE}  ABLS Habitat API - Suite de tests fonctionnels${RESET}"
echo "${BOLD}${BLUE}================================================================${RESET}"
echo ""

MISSING_DEPS=false
for dep in curl jq; do
    if ! command -v "${dep}" &>/dev/null; then
        echo "${RED}${ICON_FAIL} Prérequis manquant: ${dep}${RESET}"
        MISSING_DEPS=true
    fi
done

if command -v mariadb &>/dev/null; then
    DB_CLIENT="mariadb"
elif command -v mysql &>/dev/null; then
    DB_CLIENT="mysql"
else
    echo "${RED}${ICON_FAIL} Prérequis manquant: client SQL (mariadb/mysql)${RESET}"
    MISSING_DEPS=true
fi

if command -v podman &>/dev/null && podman compose version &>/dev/null; then
    CONTAINER_STACK="podman compose"
elif command -v podman-compose &>/dev/null; then
    CONTAINER_STACK="podman-compose"
elif command -v podman &>/dev/null; then
    CONTAINER_STACK="podman (mode direct)"
elif command -v docker &>/dev/null && docker compose version &>/dev/null; then
    CONTAINER_STACK="docker compose"
else
    echo "${RED}${ICON_FAIL} Prérequis manquant: podman (ou podman compose, ou docker compose)${RESET}"
    MISSING_DEPS=true
fi

if [[ "${MISSING_DEPS}" == true ]]; then
    echo ""
    echo "${RED}Installez les prérequis manquants et relancez.${RESET}"
    exit 1
fi

echo "${GREEN}${ICON_PASS} Runtime conteneur: ${CONTAINER_STACK}${RESET}"
echo "${GREEN}${ICON_PASS} Client SQL: ${DB_CLIENT}${RESET}"

# -----------------------------------------------------------------------------
# Répertoire results/
# -----------------------------------------------------------------------------
mkdir -p "${SCRIPT_DIR}/results"
if [[ -z "${OUTPUT_FILE}" ]]; then
    OUTPUT_FILE="${SCRIPT_DIR}/results/report-$(date +%Y%m%d-%H%M%S).txt"
fi

# -----------------------------------------------------------------------------
# Setup
# -----------------------------------------------------------------------------
if [[ "${SKIP_SETUP}" == false ]]; then
    echo "${YELLOW}▶ Démarrage de l'environnement de test...${RESET}"
    SETUP_ARGS=""
    [[ "${START_API}" == true ]] && SETUP_ARGS="--start-api"
    if ! bash "${SCRIPT_DIR}/setup.sh" ${SETUP_ARGS}; then
        echo "${RED}${ICON_FAIL} setup.sh a échoué — abandon.${RESET}"
        if [[ -f "${SCRIPT_DIR}/results/api-startup.log" ]]; then
            echo "${YELLOW}${ICON_WARN} Dernières lignes de results/api-startup.log:${RESET}"
            tail -n 40 "${SCRIPT_DIR}/results/api-startup.log" || true
        fi
        exit 1
    fi
    echo ""
elif [[ "${START_API}" == true ]]; then
    echo "${YELLOW}${ICON_WARN} --skip-setup actif: l'API n'est pas démarrée automatiquement.${RESET}"
fi

# -----------------------------------------------------------------------------
# Exécution des suites
# -----------------------------------------------------------------------------
GLOBAL_PASSED=0
GLOBAL_FAILED=0
GLOBAL_TOTAL=0
FAILED_SUITES=()

echo "${BOLD}Exécution des suites de tests (pattern: ${TESTS_PATTERN})${RESET}"
echo "──────────────────────────────────────────────────────────────"

# Collect test files sorted
mapfile -t TEST_FILES < <(find "${SCRIPT_DIR}/tests" -maxdepth 1 -name "${TESTS_PATTERN}" -type f | sort)

if [[ ${#TEST_FILES[@]} -eq 0 ]]; then
    echo "${YELLOW}Aucun fichier de test trouvé pour le pattern: ${TESTS_PATTERN}${RESET}"
    exit 0
fi

SUITE_RESULTS=()

for test_file in "${TEST_FILES[@]}"; do
    suite_name=$(basename "${test_file}" .sh)
    echo ""
    echo "${BOLD}${BLUE}┌─ ${suite_name}${RESET}"

    # Exécuter la suite dans un sous-shell pour isoler les compteurs
    set +e
    SUITE_OUTPUT=$(bash "${test_file}" 2>&1)
    SUITE_EXIT_CODE=$?
    set -e

    # Extraire les compteurs depuis la sortie de la suite
    SUITE_PASSED=$(echo "${SUITE_OUTPUT}" | grep -oP '(?<=PASSED=)\d+' | tail -1 || echo 0)
    SUITE_FAILED=$(echo "${SUITE_OUTPUT}" | grep -oP '(?<=FAILED=)\d+' | tail -1 || echo 0)
    SUITE_TOTAL=$(echo "${SUITE_OUTPUT}" | grep -oP '(?<=TOTAL=)\d+' | tail -1 || echo 0)

    # Afficher la sortie de la suite avec indentation
    echo "${SUITE_OUTPUT}" | sed 's/^/│  /'
    echo "${BOLD}${BLUE}└──────────────────────────────────────────────────────────${RESET}"

    GLOBAL_PASSED=$(( GLOBAL_PASSED + SUITE_PASSED ))
    GLOBAL_FAILED=$(( GLOBAL_FAILED + SUITE_FAILED ))
    GLOBAL_TOTAL=$(( GLOBAL_TOTAL  + SUITE_TOTAL  ))

    if [[ ${SUITE_EXIT_CODE} -ne 0 ]] || [[ "${SUITE_FAILED}" -gt 0 ]]; then
        FAILED_SUITES+=("${suite_name}")
        SUITE_RESULTS+=("${RED}${ICON_FAIL}${RESET} ${suite_name} (${SUITE_FAILED}/${SUITE_TOTAL} échecs)")
    else
        SUITE_RESULTS+=("${GREEN}${ICON_PASS}${RESET} ${suite_name} (${SUITE_TOTAL}/${SUITE_TOTAL} réussis)")
    fi
done

# -----------------------------------------------------------------------------
# Rapport global
# -----------------------------------------------------------------------------
echo ""
echo "${BOLD}================================================================${RESET}"
echo "${BOLD}  RAPPORT GLOBAL${RESET}"
echo "${BOLD}================================================================${RESET}"
echo ""
for result in "${SUITE_RESULTS[@]}"; do
    echo "  ${result}"
done
echo ""
echo "  Total : ${GLOBAL_TOTAL} tests"
echo "  ${GREEN}Réussis : ${GLOBAL_PASSED}${RESET}"
if [[ ${GLOBAL_FAILED} -gt 0 ]]; then
    echo "  ${RED}Échoués : ${GLOBAL_FAILED}${RESET}"
else
    echo "  Échoués : 0"
fi
echo ""

# Écrire le rapport dans le fichier
{
    echo "ABLS Habitat API - Rapport de test fonctionnel"
    echo "Date : $(date '+%Y-%m-%d %H:%M:%S')"
    echo "================================================================"
    echo ""
    for result in "${SUITE_RESULTS[@]}"; do
        # Supprimer les codes couleur pour le fichier
        echo "  ${result}" | sed 's/\x1B\[[0-9;]*m//g'
    done
    echo ""
    echo "Total   : ${GLOBAL_TOTAL}"
    echo "Réussis : ${GLOBAL_PASSED}"
    echo "Échoués : ${GLOBAL_FAILED}"
} > "${OUTPUT_FILE}"

echo "  Rapport écrit dans : ${OUTPUT_FILE}"
echo ""

# -----------------------------------------------------------------------------
# Teardown
# -----------------------------------------------------------------------------
if [[ "${SKIP_TEARDOWN}" == false ]]; then
    echo "${YELLOW}▶ Arrêt de l'environnement de test...${RESET}"
    bash "${SCRIPT_DIR}/teardown.sh" || true
fi

# -----------------------------------------------------------------------------
# Code de sortie
# -----------------------------------------------------------------------------
if [[ ${GLOBAL_FAILED} -gt 0 ]]; then
    echo "${RED}${ICON_FAIL} ${GLOBAL_FAILED} test(s) en échec.${RESET}"
    exit 1
else
    echo "${GREEN}${ICON_PASS} Tous les tests ont réussi.${RESET}"
    exit 0
fi
