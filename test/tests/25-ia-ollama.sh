#!/usr/bin/env bash
# =============================================================================
# 25-ia-ollama.sh - Tests du fournisseur IA local Ollama
# =============================================================================
# Endpoints testés: GET /status (ollama_url), POST /domain/set (ia_provider/ia_model), POST /ia/chat
# Un faux serveur Ollama (python3) écoute sur le port de ollama_url de la config test.
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 25 - IA / Ollama"

ADMIN_TOKEN=$(make_admin_token)
OLLAMA_URL=$(jq -r '.ollama_url' "${SCRIPT_DIR}/../config/abls-habitat-api.test.conf")
OLLAMA_PORT=$(echo "${OLLAMA_URL}" | sed -E 's#^https?://[^:/]+:([0-9]+).*#\1#')
TMP_DIR=$(mktemp -d)
restore_provider() {
    api_call POST /domain/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
        "{\"domain_uuid\":\"${TEST_DOMAIN_UUID}\",\"ia_provider\":\"mistral\",\"ia_model\":\"\"}" >/dev/null
}
trap 'restore_provider; [[ -n "${OLLAMA_PID:-}" ]] && kill "${OLLAMA_PID}" 2>/dev/null; rm -rf "${TMP_DIR}"' EXIT

# Faux serveur Ollama: refuse toute requête avec Authorization, vérifie le modèle et le format JSON
cat > "${TMP_DIR}/ollama.py" <<'EOF'
import sys, json
from http.server import BaseHTTPRequestHandler, HTTPServer
class H(BaseHTTPRequestHandler):
    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers.get("Content-Length", 0))) or b"{}")
        ok = (self.path == "/v1/chat/completions" and "Authorization" not in self.headers and
              body.get("model") == "qwen2.5:7b" and body.get("response_format", {}).get("type") == "json_object")
        content = json.dumps({"reply": "Module de test.", "sourcecode": None})
        payload = json.dumps({"choices": [{"message": {"role": "assistant", "content": content}}]} if ok
                             else {"error": {"message": "bad request"}}).encode()
        self.send_response(200 if ok else 400)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)
    def log_message(self, *args): pass
HTTPServer(("127.0.0.1", int(sys.argv[1])), H).serve_forever()
EOF
python3 "${TMP_DIR}/ollama.py" "${OLLAMA_PORT}" &
OLLAMA_PID=$!
sleep 1

CHAT_REQUEST='{"tech_id":"TEST_DLS","action":"explain","message":"Explique ce module","sourcecode":"/* test */"}'

# =============================================================================
# TEST: GET /status expose ollama_url
# =============================================================================
log_info "Test: GET /status ollama_url"
RESPONSE=$(api_call GET /status "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /status → HTTP 200"
assert_json_field "${RESPONSE}" "ollama_url" "${OLLAMA_URL}" "GET /status ollama_url correct"

# =============================================================================
# TEST: POST /domain/set ia_provider=ollama et modèle avec ':'
# =============================================================================
log_info "Test: POST /domain/set ia_provider ollama"
RESPONSE=$(api_call POST /domain/set "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" \
    "{\"domain_uuid\":\"${TEST_DOMAIN_UUID}\",\"ia_provider\":\"ollama\",\"ia_model\":\"qwen2.5:7b\"}")
assert_http_status 200 "POST /domain/set ia_provider ollama → HTTP 200"
assert_master_db_field "domains" "ia_provider" "ollama" "ia_provider ollama en BD" "domain_uuid='${TEST_DOMAIN_UUID}'"
assert_master_db_field "domains" "ia_model" "qwen2.5:7b" "ia_model qwen2.5:7b en BD" "domain_uuid='${TEST_DOMAIN_UUID}'"

# =============================================================================
# TEST: POST /ia/chat via Ollama, sans clé d'API
# =============================================================================
log_info "Test: POST /ia/chat via Ollama"
RESPONSE=$(api_call POST /ia/chat "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${CHAT_REQUEST}")
assert_http_status 200 "POST /ia/chat ollama → HTTP 200"
assert_json_field "${RESPONSE}" "provider" "ollama" "POST /ia/chat provider ollama"
assert_json_field "${RESPONSE}" "model" "qwen2.5:7b" "POST /ia/chat modèle qwen2.5:7b"
assert_json_field "${RESPONSE}" "reply" "Module de test." "POST /ia/chat réponse Ollama"

# =============================================================================
# TEST: POST /ia/chat avec Ollama indisponible
# =============================================================================
log_info "Test: POST /ia/chat Ollama indisponible"
kill "${OLLAMA_PID}" 2>/dev/null; wait "${OLLAMA_PID}" 2>/dev/null; OLLAMA_PID=""
RESPONSE=$(api_call POST /ia/chat "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${CHAT_REQUEST}")
assert_http_status 502 "POST /ia/chat ollama indisponible → HTTP 502"

print_suite_summary "Suite 25 - IA / Ollama"
[[ ${TESTS_FAILED} -eq 0 ]]
