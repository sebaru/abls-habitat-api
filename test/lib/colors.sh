#!/usr/bin/env bash
# =============================================================================
# colors.sh - Couleurs ANSI pour l'affichage des tests
# =============================================================================

# Couleurs
RED=$'\033[0;31m'
GREEN=$'\033[0;32m'
YELLOW=$'\033[1;33m'
BLUE=$'\033[0;34m'
BOLD=$'\033[1m'
DIM=$'\033[2m'
RESET=$'\033[0m'

# Icones (fallback texte si le terminal ne supporte pas)
ICON_PASS="[PASS]"
ICON_FAIL="[FAIL]"
ICON_SKIP="[SKIP]"
ICON_INFO="[INFO]"
ICON_WARN="[WARN]"
