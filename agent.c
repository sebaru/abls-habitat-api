/******************************************************************************************************************************/
/* agent.c                      Gestion des agents dans l'API HTTP WebService                                           */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent.c
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* AGENT_LIST_request_post: Repond aux requests depuis les browsers                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_LIST_request_post ( struct DOMAIN *domain, const char *path, SoupMessage *msg, JsonNode *request )
  { JsonNode *token = Http_get_token ( domain, msg );
    if (!token) return;

    if (!Http_is_authorized ( domain, msg, token, 6 )) goto end_request;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode) { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" ); goto end_request; }

    DB_Read ( domain, RootNode, "agents", "SELECT * FROM agents" );
    Http_Send_json_response ( msg, "success", RootNode );
end_request:
    json_node_unref(token);
  }
/******************************************************************************************************************************/
/* AGENT_SET_request_post: Repond aux requests depuis les browsers                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_SET_request_post ( struct DOMAIN *domain, const char *path, SoupMessage *msg, JsonNode *request )
  { JsonNode *token = Http_get_token ( domain, msg );
    if (!token) return;

    if (!Http_is_authorized ( domain, msg, token, 6 )) goto end_request;
    Http_print_request ( domain, token, path );

    if ( ! (Json_has_member ( __func__, request, "log_level" )   && Json_has_member ( __func__, request, "log_msrv" ) &&
            Json_has_member ( __func__, request, "log_bus" )     &&
            Json_has_member ( __func__, request, "description" ) && Json_has_member ( __func__, request, "agent_id" )
           )
       )
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais parametres");
       goto end_request;
     }

    gint log_target = Json_get_int ( request, "log_level" );
    if (log_target<3 || log_target>7)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log");
       goto end_request;
     }

    gchar *description = Normaliser_chaine ( Json_get_string ( request, "description" ) );
    gint   agent_id    = Json_get_int ( request, "agent_id" );
    DB_Write ( domain, "UPDATE agents SET log_msrv=%d, log_level=%d, log_bus=%d, description='%s' "
                       "WHERE agent_id='%d'",
                       Json_get_bool ( request, "log_msrv" ), Json_get_int ( request, "log_level" ),
                       Json_get_bool ( request, "log_bus" ), description, agent_id );
    g_free(description);

    Json_node_add_string ( request, "agent_tag", "SET_LOG" );
    /* WS_Send_to_agent( domain, agent_uuid, request );*/
    Http_Send_json_response ( msg, "success", NULL );
end_request:
    json_node_unref(token);
  }
/******************************************************************************************************************************/
/* RUN_AGENT_request_post: Repond aux requests AGENT depuis les agents                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_request_post ( struct DOMAIN *domain, gchar *agent_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  { if ( !strcasecmp ( api_tag, "START" ) &&
         Json_has_member ( __func__, request, "start_time" ) && Json_has_member ( __func__, request, "hostname" ) &&
         Json_has_member ( __func__, request, "version" ) && Json_has_member ( __func__, request, "install_time" )
       )
     { gchar *hostname     = Normaliser_chaine ( Json_get_string ( request, "hostname") );
       gchar *version      = Normaliser_chaine ( Json_get_string ( request, "version") );
       gchar *install_time = Normaliser_chaine ( Json_get_string ( request, "install_time") );
       DB_Write ( domain,
                  "INSERT INTO agents SET agent_uuid='%s', start_time=FROM_UNIXTIME(%d), hostname='%s', "
                  "version='%s', install_time='%s', ws_password=(SELECT SUBSTRING(MD5(RAND()),1,32)) "
                  "ON DUPLICATE KEY UPDATE start_time=VALUE(start_time), hostname=VALUE(hostname), version=VALUE(version),"
                  " ws_password=(SELECT SUBSTRING(MD5(RAND()),1,32)) ",
                  agent_uuid, Json_get_int (request, "start_time"), hostname, version, install_time );
       g_free(hostname);
       g_free(version);
       g_free(install_time);

       JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { DB_Read ( domain, RootNode, NULL,
                    "SELECT * FROM agents WHERE agent_uuid='%s'", agent_uuid );
          DB_Read ( domain, RootNode, NULL,
                    "SELECT hostname AS master_hostname FROM agents WHERE is_master=1 LIMIT 1" );

          Http_Send_json_response ( msg, "success", RootNode );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
      }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
