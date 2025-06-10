#!/bin/bash

# Utiliser des variables d'environnement
GIT_TOKEN="${GIT_TOKEN}"
GIT_REPO_URL="${GIT_REPO_URL}"
REPO_FILE_PATH="${REPO_FILE_PATH}"
LOCAL_FILE_PATH="${LOCAL_FILE_PATH}"
COMMIT_MESSAGE="${COMMIT_MESSAGE}"

# Vérifier que toutes les variables nécessaires sont définies
if [[ -z "$GIT_TOKEN" ]]; then
  echo "Erreur : GIT_TOKEN n'est pas défini."
  exit 1
fi

if [[ -z "$GIT_REPO_URL" ]]; then
  echo "Erreur : GIT_REPO_URL n'est pas défini."
  exit 1
fi

if [[ -z "$REPO_FILE_PATH" ]]; then
  echo "Erreur : REPO_FILE_PATH n'est pas défini."
  exit 1
fi

if [[ -z "$LOCAL_FILE_PATH" ]]; then
  echo "Erreur : LOCAL_FILE_PATH n'est pas défini."
  exit 1
fi

if [[ -z "$COMMIT_MESSAGE" ]]; then
  echo "Erreur : COMMIT_MESSAGE n'est pas défini."
  exit 1
fi

# Encoder le fichier en base64
FILE_CONTENT_BASE64=$(base64 -w 0 "$LOCAL_FILE_PATH")

# Obtenir le SHA du fichier existant
FILE_SHA=$(curl -s -X GET \
  -H "Authorization: Bearer $GIT_TOKEN" \
  -H "Accept: application/vnd.github.v3+json" \
  "$GIT_REPO_URL/$REPO_FILE_PATH" | jq -r '.sha')

echo "FILE_SHA = $FILE_SHA. Sending to $GIT_REPO_URL/$REPO_FILE_PATH (#$COMMIT_MESSAGE)"

  # Vérifier si le fichier existe déjà
if [ "$FILE_SHA" != "null" ] && [ "$FILE_SHA" != "" ]; then
  # Mettre à jour le fichier existant
  curl -X PUT \
    -H "Accept: application/vnd.github+json" \
    -H "Authorization: Bearer $GIT_TOKEN" \
    -d "{\"message\":\"$COMMIT_MESSAGE\",\"content\":\"$FILE_CONTENT_BASE64\",\"sha\":\"$FILE_SHA\"}" \
    "$GIT_REPO_URL/$REPO_FILE_PATH"
else
  # Créer un nouveau fichier
  curl -X PUT \
    -H "Accept: application/vnd.github+json" \
    -H "Authorization: Bearer $GIT_TOKEN" \
    -d "{\"message\":\"$COMMIT_MESSAGE\",\"content\":\"$FILE_CONTENT_BASE64\"}" \
    "$GIT_REPO_URL/$REPO_FILE_PATH"
fi
exit 0
