#!/usr/bin/env bash
# =============================================================================
# 25-ia-ollama.sh - Tests du fournisseur IA local Ollama
# =============================================================================
# Endpoints testés: GET /status (ollama_url), POST /domain/set (ia_provider/ia_model), POST /assistant/ia/chat
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

# Faux serveur Ollama en streaming SSE: refuse toute requête avec Authorization, vérifie le modèle et stream=true.
# Le mode est choisi par un mot-clé dans le dernier message: MODE_SOURCE, MODE_SLOW (12s de silence), MODE_HANG (sans fin)
cat > "${TMP_DIR}/ollama.py" <<'EOF'
import sys, json, time, os
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
class H(BaseHTTPRequestHandler):
    def chunk(self, text, finish=None):
        data = {"choices": [{"index": 0, "delta": {"content": text}, "finish_reason": finish}]}
        self.wfile.write(("data: " + json.dumps(data) + "\n\n").encode())
        self.wfile.flush()
    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers.get("Content-Length", 0))) or b"{}")
        ok = (self.path == "/v1/chat/completions" and "Authorization" not in self.headers and
              body.get("model") == "qwen2.5:7b" and body.get("stream") is True and "response_format" not in body)
        if not ok:
            payload = json.dumps({"error": {"message": "bad request"}}).encode()
            self.send_response(400)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
            return
        prompt = body["messages"][-1]["content"]
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.end_headers()
        try:
            if "MODE_HANG" in prompt:
                while True:
                    time.sleep(1)
                    self.wfile.write(b": keepalive\n\n")
                    self.wfile.flush()
            if "MODE_SLOW" in prompt:
                time.sleep(12)
            if "MODE_SOURCE" in prompt:
                self.chunk("Voici la correction.\n```dls-source\n")
                self.chunk("/* corrige */\n")
                self.chunk("```\n")
                self.chunk("Fin.", "stop")
            else:
                self.chunk("Module ")
                self.chunk("de test.", "stop")
            self.wfile.write(b"data: [DONE]\n\n")
        except (BrokenPipeError, ConnectionResetError):
            open(os.path.join(sys.argv[2], "cancelled"), "w").close()
    def log_message(self, *args): pass
ThreadingHTTPServer(("127.0.0.1", int(sys.argv[1])), H).serve_forever()
EOF
python3 "${TMP_DIR}/ollama.py" "${OLLAMA_PORT}" "${TMP_DIR}" &
OLLAMA_PID=$!
sleep 1

chat_request() {
    echo "{\"tech_id\":\"TEST_DLS\",\"action\":\"$1\",\"message\":\"$2\",\"sourcecode\":\"/* test */\"}"
}
# stream_call <fichier entêtes> <max-time> <requête>: corps SSE sur stdout
stream_call() {
    curl -s -N --max-time "$2" -D "$1" -X POST "${API_URL}/assistant/ia/chat" \
        -H "Content-Type: application/json" -H "Authorization: Bearer ${ADMIN_TOKEN}" \
        -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" -d "$3"
}
# done_event <corps SSE>: données JSON de l'événement 'done'
done_event() { echo "$1" | sed -n '/^event: done$/{n;s/^data: //p}'; }
# check <code retour> <nom du test>
check() {
    _test_start
    if [[ "$1" -eq 0 ]]; then _test_pass "$2"; else _test_fail "$2" "condition fausse"; fi
}
CHAT_REQUEST=$(chat_request explain "Explique ce module")

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
# TEST: POST /assistant/ia/chat via Ollama, sans clé d'API, en SSE
# =============================================================================
log_info "Test: POST /assistant/ia/chat via Ollama (SSE)"
BODY=$(stream_call "${TMP_DIR}/h1" 30 "${CHAT_REQUEST}")
grep -q '^HTTP/1.1 200' "${TMP_DIR}/h1"; check $? "POST /assistant/ia/chat ollama → HTTP 200"
grep -qi '^Content-Type: text/event-stream' "${TMP_DIR}/h1"; check $? "POST /assistant/ia/chat → text/event-stream"
[[ $(grep -c '^event: delta$' <<< "${BODY}") -eq 2 ]]; check $? "POST /assistant/ia/chat relaie les 2 fragments"
DONE=$(done_event "${BODY}")
assert_json_field "${DONE}" "provider" "ollama" "POST /assistant/ia/chat provider ollama"
assert_json_field "${DONE}" "model" "qwen2.5:7b" "POST /assistant/ia/chat modèle qwen2.5:7b"
assert_json_field "${DONE}" "reply" "Module de test." "POST /assistant/ia/chat réponse Ollama"
[[ $(jq -r '.sourcecode' <<< "${DONE}") == null ]]; check $? "POST /assistant/ia/chat sans source proposé"

# =============================================================================
# TEST: extraction du bloc dls-source
# =============================================================================
log_info "Test: POST /assistant/ia/chat bloc dls-source"
BODY=$(stream_call "${TMP_DIR}/h2" 30 "$(chat_request fix MODE_SOURCE)")
DONE=$(done_event "${BODY}")
assert_json_field "${DONE}" "sourcecode" "/* corrige */" "POST /assistant/ia/chat source extrait du bloc"
[[ "$(jq -r '.reply' <<< "${DONE}")" == $'Voici la correction.\n\nFin.' ]]; check $? "POST /assistant/ia/chat réponse sans le bloc"

# =============================================================================
# TEST: ping SSE pendant un silence du fournisseur, puis 429 si Ollama est occupé
# =============================================================================
log_info "Test: POST /assistant/ia/chat ping et verrou Ollama"
stream_call "${TMP_DIR}/h3" 40 "$(chat_request chat MODE_SLOW)" > "${TMP_DIR}/slow.txt" &
SLOW_PID=$!
sleep 2
RESPONSE=$(api_call POST /assistant/ia/chat "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${CHAT_REQUEST}")
assert_http_status 429 "POST /assistant/ia/chat pendant une génération → HTTP 429"
wait "${SLOW_PID}"
grep -q '^: ping$' "${TMP_DIR}/slow.txt"; check $? "POST /assistant/ia/chat ping pendant le silence"
assert_json_field "$(done_event "$(cat "${TMP_DIR}/slow.txt")")" "reply" "Module de test." "POST /assistant/ia/chat réponse après le silence"

# =============================================================================
# TEST: déconnexion du client → appel Ollama annulé et verrou libéré
# =============================================================================
log_info "Test: POST /assistant/ia/chat annulé à la déconnexion du client"
stream_call "${TMP_DIR}/h4" 3 "$(chat_request chat MODE_HANG)" >/dev/null
for _ in $(seq 1 30); do [[ -f "${TMP_DIR}/cancelled" ]] && break; sleep 1; done
[[ -f "${TMP_DIR}/cancelled" ]]; check $? "POST /assistant/ia/chat connexion Ollama fermée"
sleep 1
BODY=$(stream_call "${TMP_DIR}/h5" 30 "${CHAT_REQUEST}")
assert_json_field "$(done_event "${BODY}")" "reply" "Module de test." "POST /assistant/ia/chat verrou libéré après annulation"

# =============================================================================
# TEST: POST /assistant/ia/chat avec Ollama indisponible
# =============================================================================
log_info "Test: POST /assistant/ia/chat Ollama indisponible"
kill "${OLLAMA_PID}" 2>/dev/null; wait "${OLLAMA_PID}" 2>/dev/null; OLLAMA_PID=""
RESPONSE=$(api_call POST /assistant/ia/chat "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}" "${CHAT_REQUEST}")
assert_http_status 502 "POST /assistant/ia/chat ollama indisponible → HTTP 502"
[[ "$(jq -r '.api_error' <<< "${RESPONSE}")" == "IA provider request failed (ollama:"* ]]; check $? "POST /assistant/ia/chat ollama indisponible → erreur JSON"

print_suite_summary "Suite 25 - IA / Ollama"
[[ ${TESTS_FAILED} -eq 0 ]]
