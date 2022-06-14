/******************************************************************************************************************************/
/* Include/Http.h        Déclaration structure internes des WebServices                                                       */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                19.02.2022 20:58:34 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

#ifndef _HTTP_H_
 #define _HTTP_H_

 #include <glib.h>
 #include <openssl/ssl.h>
 #include <openssl/rand.h>
 #include <libsoup/soup.h>
 #include <uuid/uuid.h>
 #include <json-glib/json-glib.h>
 #include <mysql.h>
 #include <syslog.h>
 #include <jwt.h>

 #define DATABASE_POOL_SIZE   5

 #include "config.h" /* from autotools */
 #include "Domains.h"
 #include "Database.h"
 #include "Json.h"
 #include "Websocket.h"
 #include "Erreur.h"

 struct GLOBAL                                                                                    /* zone de mémoire partagée */
  { gint Top;
    pthread_mutex_t nbr_threads_sync;                                                     /* Bit de synchronisation processus */
    gint nbr_threads;                                                                              /* Nombre de request en // */
    JsonNode *config;                                                                              /* Config globale via file */
    GTree *domaines;                                                                                        /* Tree of DOMAIN */
  };


/******************************************************************************************************************************/
/* struct HTTP_CADRAN
  { gchar tech_id[32];
    gchar acronyme[64];
    gchar unite[32];
    gchar classe[12];
    gpointer dls_data;
    gdouble  valeur;
    gboolean in_range;
    gint last_update;
  };*/

/*************************************************** Définitions des prototypes ***********************************************/
 extern JsonNode *Http_Msg_to_Json ( SoupMessage *msg );                                                       /* Dans http.c */
 extern JsonNode *Http_Response_Msg_to_Json ( SoupMessage *msg );
 extern gint Http_Msg_status_code ( SoupMessage *msg );
 extern gchar *Http_Msg_reason_phrase ( SoupMessage *msg );
 extern JsonNode *Http_json_node_create ( SoupMessage *msg );
 extern void Http_Send_json_response ( SoupMessage *msg, gint code, gchar *details, JsonNode *RootNode );
 extern JsonNode *Http_get_token ( gchar *path, SoupMessage *msg );
 extern gboolean Http_fail_if_has_not ( struct DOMAIN *domain, gchar *path, SoupMessage *msg, JsonNode *request, gchar *name );
 extern gboolean Http_is_authorized ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, gint access_level );
 extern void Http_print_request ( struct DOMAIN *domain, JsonNode *token, gchar *path );
 extern void UUID_New ( gchar *target );

/* extern void Http_traiter_map ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data );*/
 extern void RUN_AGENT_START_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request );
 extern gboolean AGENT_send_to_agent ( struct DOMAIN *domain, gchar *agent_uuid, gchar *api_tag, JsonNode *node );
 extern void AGENT_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void AGENT_SET_MASTER_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void AGENT_RESET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void AGENT_UPGRADE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void AGENT_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void RUN_VISUELS_SET_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request );
 extern void VISUELS_Load_all ( struct DOMAIN *domain );
 extern void VISUELS_Unload_all ( struct DOMAIN *domain );

 extern void USER_PROFIL_request_post ( JsonNode *token, SoupMessage *msg, JsonNode *request );
 extern void USER_INVITE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void USER_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void USER_SET_DOMAIN_request_post ( JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void THREAD_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void THREAD_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void THREAD_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void THREAD_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request );
 extern void RUN_THREAD_LOAD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request );
 extern void RUN_THREAD_GET_CONFIG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request );

 extern void ICONS_request_get ( SoupServer *server, SoupMessage *msg, const char *path );

 extern void STATUS_request_get ( SoupServer *server, SoupMessage *msg, const char *path );

 extern void DOMAIN_LIST_request_post ( JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_IMAGE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_STATUS_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_IMAGE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_TRANSFER_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void DOMAIN_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void MODBUS_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void MODBUS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void AUDIO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void IMSGS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void MNEMOS_TECH_IDS_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );

 extern void ARCHIVE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void ARCHIVE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void ARCHIVE_STATUS_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request );
 extern void RUN_ARCHIVE_SAVE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request ) ;
 extern gboolean ARCHIVE_Delete_old_data ( gpointer key, gpointer value, gpointer data );

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
