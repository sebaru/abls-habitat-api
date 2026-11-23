#!/usr/bin/env bash
# =============================================================================
# 24-voice.sh - Tests de l'endpoint de transcription vocale
# =============================================================================
# Endpoints testés: GET /status (whisper_url), POST /assistant/voice
# Un faux serveur Whisper (python3) écoute sur le port de whisper_url de la config test.
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../lib/test-utils.sh"

log_suite "Suite 24 - Voice / Whisper"

ADMIN_TOKEN=$(make_admin_token)
WHISPER_URL=$(jq -r '.whisper_url' "${SCRIPT_DIR}/../config/abls-habitat-api.test.conf")
WHISPER_PORT=$(echo "${WHISPER_URL}" | sed -E 's#^https?://[^:/]+:([0-9]+).*#\1#')
TMP_DIR=$(mktemp -d)
trap '[[ -n "${WHISPER_PID:-}" ]] && kill "${WHISPER_PID}" 2>/dev/null; rm -rf "${TMP_DIR}"' EXIT

# Faux serveur Whisper: vérifie le multipart et renvoie un texte fixe
cat > "${TMP_DIR}/whisper.py" <<'EOF'
import sys, json
from http.server import BaseHTTPRequestHandler, HTTPServer
class H(BaseHTTPRequestHandler):
    def do_POST(self):
        body = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        ok = self.path == "/inference" and b'name="file"' in body and (
            (b'filename="voice.wav"' in body and b"Content-Type: audio/wav" in body and b"RIFF" in body and b"WAVE" in body) or
            (b'filename="voice.webm"' in body and b"Content-Type: audio/webm" in body) or
            (b'filename="voice.mp4"' in body and b"Content-Type: audio/mp4" in body))
        payload = json.dumps({"text": " Allume la lumière du salon. \n"} if ok else {"error": "bad request"}).encode()
        self.send_response(200 if ok else 400)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)
    def log_message(self, *args): pass
HTTPServer(("127.0.0.1", int(sys.argv[1])), H).serve_forever()
EOF
python3 "${TMP_DIR}/whisper.py" "${WHISPER_PORT}" &
WHISPER_PID=$!
sleep 1

# WAV minimal: en-tête RIFF/WAVE 16 kHz mono 16 bits + 0.1s de silence
python3 - "${TMP_DIR}/test.wav" "${TMP_DIR}/test.webm" "${TMP_DIR}/test.mp4" <<'EOF'
import sys, wave
with wave.open(sys.argv[1], "wb") as w:
    w.setnchannels(1); w.setsampwidth(2); w.setframerate(16000); w.writeframes(b"\0\0" * 1600)
for path, data in zip(sys.argv[2:], (b"webm audio data", b"mp4 audio data")):
    with open(path, "wb") as f:
        f.write(data)
EOF

voice_post() {
    local file="$1" token="$2" content_type="${3:-audio/wav}"
    local full parsed
    full=$(curl -s --max-time 30 -w "\n__HTTP_CODE:%{http_code}" -X POST "${API_URL}/assistant/voice" \
                -H "Content-Type: ${content_type}" -H "Authorization: Bearer ${token}" \
                -H "X-ABLS-DOMAIN: ${TEST_DOMAIN_UUID}" --data-binary "@${file}" 2>/dev/null)
    parsed="$(extract_http_response_parts "${full}")"
    set_last_http_code "$(echo "${parsed}" | sed -n '1p')"
    echo "${parsed}" | sed -n '2,$p'
}

# =============================================================================
# TEST: GET /status expose whisper_url
# =============================================================================
log_info "Test: GET /status whisper_url"
RESPONSE=$(api_call GET /status "${ADMIN_TOKEN}" "${TEST_DOMAIN_UUID}")
assert_http_status 200 "GET /status authentifié → HTTP 200"
assert_json_field "${RESPONSE}" "whisper_url" "${WHISPER_URL}" "GET /status whisper_url correct"

# =============================================================================
# TEST: POST /assistant/voice avec un WAV invalide
# =============================================================================
log_info "Test: POST /assistant/voice WAV invalide"
echo '{"not":"a wav"}' > "${TMP_DIR}/bad.wav"
RESPONSE=$(voice_post "${TMP_DIR}/bad.wav" "${ADMIN_TOKEN}")
assert_http_status 400 "POST /assistant/voice WAV invalide → HTTP 400"

# =============================================================================
# TEST: POST /assistant/voice avec un format non pris en charge
# =============================================================================
log_info "Test: POST /assistant/voice format non pris en charge"
RESPONSE=$(voice_post "${TMP_DIR}/test.webm" "${ADMIN_TOKEN}" "application/octet-stream")
assert_http_status 415 "POST /assistant/voice format non pris en charge → HTTP 415"

# =============================================================================
# TEST: POST /assistant/voice sans token
# =============================================================================
log_info "Test: POST /assistant/voice sans token"
RESPONSE=$(voice_post "${TMP_DIR}/test.wav" "")
_test_start
if [[ "${LAST_HTTP_CODE}" == "401" || "${LAST_HTTP_CODE}" == "403" ]]; then
    _test_pass "POST /assistant/voice sans token refusé (HTTP ${LAST_HTTP_CODE})"
else
    _test_fail "POST /assistant/voice sans token" "attendu: 401/403, reçu: ${LAST_HTTP_CODE}"
fi

# =============================================================================
# TEST: POST /assistant/voice avec un WAV valide
# =============================================================================
log_info "Test: POST /assistant/voice WAV valide"
RESPONSE=$(voice_post "${TMP_DIR}/test.wav" "${ADMIN_TOKEN}")
assert_http_status 200 "POST /assistant/voice WAV → HTTP 200"
assert_json_field "${RESPONSE}" "text" "Allume la lumière du salon." "POST /assistant/voice retourne le texte compris par Whisper"

# =============================================================================
# TEST: POST /assistant/voice avec un WebM (converti par Whisper)
# =============================================================================
log_info "Test: POST /assistant/voice WebM valide"
RESPONSE=$(voice_post "${TMP_DIR}/test.webm" "${ADMIN_TOKEN}" "audio/webm;codecs=opus")
assert_http_status 200 "POST /assistant/voice WebM → HTTP 200"

# =============================================================================
# TEST: POST /assistant/voice avec un MP4
# =============================================================================
log_info "Test: POST /assistant/voice MP4 valide"
RESPONSE=$(voice_post "${TMP_DIR}/test.mp4" "${ADMIN_TOKEN}" "audio/mp4")
assert_http_status 200 "POST /assistant/voice MP4 → HTTP 200"

# =============================================================================
# TEST: POST /assistant/voice avec Whisper indisponible
# =============================================================================
log_info "Test: POST /assistant/voice Whisper indisponible"
kill "${WHISPER_PID}" 2>/dev/null; wait "${WHISPER_PID}" 2>/dev/null; WHISPER_PID=""
RESPONSE=$(voice_post "${TMP_DIR}/test.wav" "${ADMIN_TOKEN}")
assert_http_status 502 "POST /assistant/voice Whisper indisponible → HTTP 502"

print_suite_summary "Suite 24 - Voice / Whisper"
[[ ${TESTS_FAILED} -eq 0 ]]
