/******************************************************************************************************************************/
/* voice.c                      Transcription vocale via le serveur Whisper                                                   */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                         22.11.2026 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * voice.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

 #define VOICE_MAX_SIZE (10*1024*1024)

/******************************************************************************************************************************/
/* VOICE_get_extension: Détermine l'extension du fichier audio à transmettre à Whisper                                        */
/* Entrée: type MIME audio sans paramètres, ou NULL                                                                           */
/* Sortie: extension statique, ou NULL si le type n'est pas pris en charge                                                   */
/******************************************************************************************************************************/
 static const gchar *VOICE_get_extension ( const gchar *content_type )
  { if (!content_type) return NULL;
    if (!g_ascii_strcasecmp ( content_type, "audio/wav" ) || !g_ascii_strcasecmp ( content_type, "audio/x-wav" )) return "wav";
    if (!g_ascii_strcasecmp ( content_type, "audio/webm" )) return "webm";
    if (!g_ascii_strcasecmp ( content_type, "audio/mp4" )) return "mp4";
    return NULL;
  }

/******************************************************************************************************************************/
/* VOICE_request_post: Reçoit un enregistrement audio, l'envoie au serveur Whisper et logue le texte compris                   */
/* Entrées: domaine, token utilisateur, chemin HTTP, message client (body = audio brut)                                       */
/* Sortie: réponse JSON { "text": "..." }                                                                                     */
/******************************************************************************************************************************/
 void VOICE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg )
  { if (!Http_is_authorized ( domain, token, path, msg, 1 )) return;
    Http_print_request ( domain, token, (gchar *)path );

    SoupMessageHeaders *headers = soup_server_message_get_request_headers ( msg );
    const gchar *content_type = soup_message_headers_get_content_type ( headers, NULL );
    const gchar *extension = VOICE_get_extension ( content_type );
    if (!extension)
     { Info ( __func__, "voice", domain->uuid, LOG_ERR, "%s: unsupported audio format '%s'", path, content_type ? content_type : "(missing)" );
       Http_Send_json_response ( msg, SOUP_STATUS_UNSUPPORTED_MEDIA_TYPE, "Unsupported audio format", NULL );
       return;
     }

    GBytes *audio = soup_message_body_flatten ( soup_server_message_get_request_body ( msg ) );
    gsize audio_size = 0;
    const guchar *audio_data = g_bytes_get_data ( audio, &audio_size );
    gboolean is_wav = !g_strcmp0 ( extension, "wav" );
    if (!audio_size || audio_size > VOICE_MAX_SIZE ||
        (is_wav && (audio_size < 44 || memcmp ( audio_data, "RIFF", 4 ) || memcmp ( audio_data+8, "WAVE", 4 ))))
     { g_bytes_unref ( audio );
       Info ( __func__, "voice", domain->uuid, LOG_ERR, "%s: invalid audio payload (%" G_GSIZE_FORMAT " bytes, %s)", path, audio_size, content_type );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Invalid audio payload", NULL );
       return;
     }

    gchar *whisper_url = Json_get_string ( Global.config, "whisper_url" );
    SoupMultipart *multipart = soup_multipart_new ( SOUP_FORM_MIME_TYPE_MULTIPART );
    gchar *filename = g_strdup_printf ( "voice.%s", extension );
    soup_multipart_append_form_file   ( multipart, "file", filename, content_type, audio );
    g_free ( filename );
    soup_multipart_append_form_string ( multipart, "response_format", "json" );
    soup_multipart_append_form_string ( multipart, "language", "fr" );
    SoupMessage *whisper_msg = soup_message_new_from_multipart ( whisper_url, multipart );
    soup_multipart_free ( multipart );
    g_bytes_unref ( audio );
    if (!whisper_msg)
     { Info ( __func__, "voice", domain->uuid, LOG_ERR, "Unable to prepare Whisper request to '%s'", whisper_url );
       Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Unable to prepare Whisper request", NULL );
       return;
     }

    SoupSession *session = soup_session_new();
    soup_session_set_timeout ( session, 60 );
    GError *error = NULL;
    GBytes *response = soup_session_send_and_read ( session, whisper_msg, Global.http_cancellable, &error );
    gint status = soup_message_get_status ( whisper_msg );

    if (!response || status < 200 || status >= 300)
     { Info ( __func__, "voice", domain->uuid, LOG_ERR, "Whisper request to '%s' failed (status %d): %s",
              whisper_url, status, error ? error->message : "no response" );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_GATEWAY, "Whisper request failed", NULL );
     }
    else
     { gsize size = 0;
       const gchar *data = g_bytes_get_data ( response, &size );
       gchar *response_text = g_strndup ( data, size );
       JsonNode *whisper_root = Json_get_from_string ( response_text );
       gchar *text = whisper_root ? Json_get_string ( whisper_root, "text" ) : NULL;
       if (!text)
        { Info ( __func__, "voice", domain->uuid, LOG_ERR, "Whisper returned an invalid response: %s", response_text );
          Http_Send_json_response ( msg, SOUP_STATUS_BAD_GATEWAY, "Whisper returned an invalid response", NULL );
        }
       else
        { gchar *texte = g_strstrip ( g_strdup ( text ) );
          Info ( __func__, "voice", domain->uuid, LOG_NOTICE, "Whisper a compris: '%s'", texte );
          JsonNode *RootNode = Json_create();
          if (RootNode)
           { Json_add_string ( RootNode, "text", texte );
             Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
           }
          else Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory error", NULL );
          g_free ( texte );
        }
       if (whisper_root) Json_unref ( whisper_root );
       g_free ( response_text );
     }
    if (response) g_bytes_unref ( response );
    if (error) g_error_free ( error );
    g_object_unref ( whisper_msg );
    soup_session_abort ( session );
    g_object_unref ( session );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
