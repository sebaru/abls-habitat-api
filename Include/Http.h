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
 #include <libsoup/soup.h>
 #include <uuid/uuid.h>
 #include <json-glib/json-glib.h>
 #include <mysql.h>
 #include <syslog.h>
 #include <jwt.h>

 #include "config.h" /* from autotools */
 #include "Json.h"
 #include "Domains.h"
 #include "Database.h"
 #include "Websocket.h"
 #include "Erreur.h"

 struct GLOBAL                                                                                    /* zone de mémoire partagée */
  { JsonNode *config;                                                                              /* Config globale via file */
    GTree *domaines;                                                                                       /* Tree of DOMAIN */
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
 extern void Http_Send_json_response ( SoupMessage *msg, gchar *status, JsonNode *RootNode );
 extern JsonNode *Http_get_token ( struct DOMAIN *domain, SoupMessage *msg );
 extern gboolean Http_is_authorized ( struct DOMAIN *domain, SoupMessage *msg, JsonNode *token, gint access_level );
 extern void Http_print_request ( struct DOMAIN *domain, JsonNode *token, gchar *path );
 extern void UUID_New ( gchar *target );

/* extern void Http_traiter_map ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                SoupClientContext *client, gpointer user_data );*/
 extern void INSTANCE_request_post ( struct DOMAIN *domain, gchar *instance_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request );

 extern void VISUELS_request_post ( struct DOMAIN *domain, gchar *instance_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request );
 extern void VISUELS_Load_all ( struct DOMAIN *domain );
 extern void VISUELS_Unload_all ( struct DOMAIN *domain );

 extern void USER_REGISTER_request_post ( SoupMessage *msg, JsonNode *request );
 extern void USER_ADD_request_post ( SoupMessage *msg, JsonNode *request );

 extern void SUBPROCESS_request_post ( struct DOMAIN *domain, gchar *instance_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request );

 extern void ICONS_request_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                 SoupClientContext *client, gpointer user_data );

 extern void STATUS_request_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                  SoupClientContext *client, gpointer user_data );

 extern void DOMAIN_STATUS_request_post ( struct DOMAIN *domain, SoupMessage *msg, JsonNode *request );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
