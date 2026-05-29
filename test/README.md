# Tests fonctionnels API - ABLS Habitat

Suite de tests fonctionnels Bash/curl pour l'API ABLS Habitat. Les tests vérifient le comportement HTTP des endpoints **et** la cohérence de la base de données MariaDB.

## Prérequis

| Outil  | Version min | Usage |
|--------|------------|-------|
| `bash` | 4.x        | Interpréteur des tests |
| `curl` | 7.x        | Appels HTTP vers l'API |
| `jq`   | 1.6        | Parsing JSON |
| `mariadb`| client 11  | Requêtes directes MariaDB |
| `podman` | v4+ | Base de données de test (compose si disponible, sinon mode direct) |

Installation des outils client sur Debian/Ubuntu :

```bash
sudo apt install curl jq mariadb-client podman podman-compose

Note: si `podman compose` n'est pas disponible, les scripts utilisent automatiquement `podman run` en mode direct.
```

## Architecture

```
test/
├── run-all-tests.sh          # Lanceur principal
├── setup.sh                  # Init : Podman + DB + fixtures (+ API optionnelle)
├── teardown.sh               # Nettoyage : arrêt API + Podman
├── config/
│   ├── abls-habitat-api.test.conf  # Config API pour les tests
│   └── test-data.sql               # Fixtures (schéma + jeux de données)
├── lib/
│   ├── colors.sh             # Constantes ANSI couleurs/icônes
│   └── test-utils.sh         # Fonctions communes (JWT, curl, assertions, DB)
├── tests/
│   ├── 01-unauthenticated.sh # Endpoints publics (ping, status)
│   ├── 02-user-auth.sh       # Authentification JWT et profil utilisateur
│   ├── 03-domain-crud.sh     # CRUD domaines
│   ├── 04-camera-crud.sh     # CRUD caméras
    ├── 05-dls.sh             # DLS (scripts de logique + /run/dls/create via agent)
│   ├── 06-data-integrity.sh  # Intégrité BD cross-suite
│   └── 07-error-handling.sh  # Cas d'erreur et limites
└── results/                  # Rapports générés automatiquement
```

## Démarrage rapide

### 1. Mode par défaut (API démarrée/arrêtée automatiquement)

```bash
cd test/
./run-all-tests.sh
```

### 2. Avec une API déjà lancée manuellement

```bash
# Lancer l'API avec la config de test (depuis la racine du projet API)
./build/abls-habitat-api -c test/config/abls-habitat-api.test.conf &

# Initialiser la BD et lancer les tests
cd test/
./run-all-tests.sh --skip-setup --skip-teardown
```

### 3. Désactiver le démarrage automatique de l'API

```bash
cd test/
./run-all-tests.sh --no-start-api
```

### 4. Lancer une seule suite

```bash
cd test/
./setup.sh                         # Démarrer la BD
bash tests/04-camera-crud.sh       # Exécuter la suite 04 uniquement
./teardown.sh                      # Arrêter la BD
```

## Options de `run-all-tests.sh`

| Option | Description |
|--------|-------------|
| `--skip-setup` | Ne pas relancer `setup.sh` (BD déjà démarrée) |
| `--skip-teardown` | Conserver la BD après les tests (pour déboguer) |
| `--no-start-api` | Ne lance pas l'API automatiquement |
| `--start-api` | Compatibilité: force le lancement auto |
| `--tests PATTERN` | Exécuter seulement les suites correspondant au pattern (ex: `"0[12]-*"`) |
| `--output FILE` | Chemin du rapport (défaut: `results/report-<timestamp>.txt`) |
| `-h, --help` | Afficher l'aide |

## Configuration de test

Le fichier [config/abls-habitat-api.test.conf](config/abls-habitat-api.test.conf) configure l'API pour les tests :

- **`idp_token_check: false`** — La signature des JWT n'est pas vérifiée (les tests génèrent des JWT factices)
- **Port MariaDB : 13306** — Évite les conflits avec une instance locale
- **Port API : 15562** — Port dédié aux tests
- **`db_hostname: 127.0.0.1`** — La BD est exposée sur localhost via Podman

## Données de test

Les fixtures sont définies dans [config/test-data.sql](config/test-data.sql) :

| Rôle | UUID (début) | Email | access_level |
|------|-------------|-------|--------------|
| Admin | `bbbbbbbb-…-001` | `admin@test.abls-habitat.fr` | 9 |
| Utilisateur | `cccccccc-…-001` | `user@test.abls-habitat.fr` | 6 |
| Lecture seule | `dddddddd-…-001` | `readonly@test.abls-habitat.fr` | 1 |
| Désactivé | `eeeeeeee-…-001` | `disabled@test.abls-habitat.fr` | 1 (enable=0) |

- **Domaine de test** : `aaaaaaaa-0000-0000-0000-000000000001` ("Domaine de Test Principal")
- **BD domaine** : `abls_test_aaaaaaaa_0000_0000_0000_000000000001`
- **Agent de test** : `ffffffff-0000-0000-0000-000000000001` (`TEST_AGENT_UUID`), hostname `test-agent-host`
- **Secret du domaine** : `test-domain-secret-001` (`TEST_DOMAIN_SECRET`) — utilisé pour la signature des appels agent
- **Caméras** : `Camera-Test-01` (rtsp://192.168.1.100), `Camera-Test-02`
- **DLS** : `SYS` (système), `TEST_DLS` (test)

## Ajouter une suite de tests

1. Créer `tests/08-mon-endpoint.sh` (respecter le préfixe numérique pour l'ordre)
2. Sourcer les utilitaires en début de fichier :

```bash
#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 08 - Mon endpoint"

ADMIN_TOKEN=$(make_admin_token)

log_info "Test: GET /mon/endpoint → 200"
RESPONSE=$(api_call GET /mon/endpoint "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
_test_start
if [[ "${LAST_HTTP_CODE}" == "200" ]]; then
    _test_pass "GET /mon/endpoint → 200"
else
    _test_fail "GET /mon/endpoint a échoué" "HTTP=${LAST_HTTP_CODE}"
fi

print_suite_summary "Suite 08 - Mon endpoint"
[[ ${TESTS_FAILED} -eq 0 ]]
```

## Fonctions utilitaires

Voir [lib/test-utils.sh](lib/test-utils.sh) pour la liste complète. Les principales :

| Fonction | Description |
|----------|-------------|
| `make_admin_token` | JWT admin (access_level=9) |
| `make_user_token` | JWT utilisateur standard |
| `make_readonly_token` | JWT lecture seule |
| `make_disabled_token` | JWT utilisateur désactivé |
| `api_call METHOD PATH TOKEN DOMAIN [DATA]` | Appel curl avec JWT, positionne `LAST_HTTP_CODE` |
| `api_call_agent METHOD PATH DOMAIN AGENT SECRET [DATA]` | Appel curl avec signature HMAC-SHA256 agent (`/run/*`) |
| `api_call_no_auth METHOD PATH` | Appel curl sans authentification |
| `db_query SQL [DB]` | Requête SQL directe (défaut: `abls_master`) |
| `db_domain_query SQL` | Requête sur la BD du domaine de test |
| `assert_http_status EXPECTED NAME` | Vérifie `LAST_HTTP_CODE` |
| `assert_json_field JSON FIELD EXPECTED NAME` | Vérifie un champ JSON via `jq` |
| `assert_db_count TABLE EXPECTED NAME [WHERE]` | Vérifie `COUNT(*)` en BD |
| `assert_db_field TABLE FIELD EXPECTED NAME [WHERE]` | Vérifie une valeur en BD |

## Authentification agent (`/run/*`)

Les endpoints `/run/*` ne sont pas protégés par JWT mais par une signature HMAC-SHA256 calculée par l'agent Watchdogd :

```
SHA256(domain_uuid + agent_uuid + domain_secret + request_body + timestamp)
```

Le résultat est encodé en base64 standard et transmis dans le header `X-ABLS-SIGNATURE`. Les tests utilisent `api_call_agent` qui recalcule automatiquement cette signature à partir du `TEST_DOMAIN_SECRET` défini dans les fixtures.

Headers requis : `Origin`, `X-ABLS-DOMAIN`, `X-ABLS-AGENT`, `X-ABLS-TIMESTAMP`, `X-ABLS-SIGNATURE`.

## Dépannage

### La BD ne démarre pas

```bash
podman compose -f test/docker-compose.yml logs mariadb
```

### Les tests échouent avec "connexion refusée"

Vérifier que l'API est bien lancée et écoute sur le port 15562 :

```bash
curl http://127.0.0.1:15562/ping
```

### Inspecter la BD de test après un échec

```bash
./run-all-tests.sh --skip-teardown
# puis :
mariadb -u abls_test -pabls_test_password -h 127.0.0.1 -P 13306 abls_master
mariadb -u abls_test -pabls_test_password -h 127.0.0.1 -P 13306 abls_test_aaaaaaaa_0000_0000_0000_000000000001
```

### Réinitialiser la BD sans tout relancer

```bash
./teardown.sh --keep-db  # Arrête l'API mais garde Podman
# Recharger les fixtures uniquement :
mariadb -u root -prootpass -h 127.0.0.1 -P 13306 < config/test-data.sql
```
