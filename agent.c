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
/* AGENT_LIST_Add_ws_status: Ajoute le statut du websocket a l'agent en paramete                                              */
/* Entrées: la structure json de l'agent                                                                                      */
/* Sortie : "ws_connected" est mis à jour dans chaque agent                                                                   */
/******************************************************************************************************************************/
 void AGENT_LIST_Add_ws_status ( JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    gchar *agent_uuid = Json_get_string ( element, "agent_uuid" );
    Json_node_add_bool ( element, "ws_connected", FALSE );
    pthread_mutex_lock ( &domain->synchro );
    GSList *liste = domain->ws_agents;
    while(liste)
     { struct WS_AGENT_SESSION *ws_agent = liste->data;
       if (!strcmp ( agent_uuid, ws_agent->agent_uuid ) ) { Json_node_add_bool ( element, "ws_connected", TRUE ); break; }
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock ( &domain->synchro );
  }
/******************************************************************************************************************************/
/* AGENT_LIST_request_post: Repond aux requests depuis les browsers                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "agents", "SELECT * FROM agents" );
    Json_node_foreach_array_element ( RootNode, "agents", AGENT_LIST_Add_ws_status, domain );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* AGENT_send_to_agent: Envoi un json à l'agent en parametre                                                                  */
/* Entrées: le domain, l'agent_uuid, l'api_tag et le json source                                                              */
/* Sortie : FALSE si pas trouvé                                                                                               */
/******************************************************************************************************************************/
 gboolean AGENT_send_to_agent ( struct DOMAIN *domain, gchar *agent_uuid, gchar *api_tag, JsonNode *node )
  { gboolean retour = FALSE;
    pthread_mutex_lock ( &domain->synchro );
    GSList *liste = domain->ws_agents;
    while (liste)
     { struct WS_AGENT_SESSION *ws_agent = liste->data;
       if (agent_uuid == NULL || !strcmp ( agent_uuid, ws_agent->agent_uuid ) )
        { WS_Agent_Send_to_agent ( ws_agent, api_tag, node );
          retour = TRUE;
        }
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock ( &domain->synchro );
    return(retour);
  }
/******************************************************************************************************************************/
/* AGENT_UPGRADE_request_post: Envoi une demande d'upgrade à un agent                                                         */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_UPGRADE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid")) return;

    gchar *agent_uuid = Json_get_string ( request, "agent_uuid" );
    gboolean retour = AGENT_send_to_agent ( domain, agent_uuid, "UPGRADE", NULL );

    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is upgrading", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent is not connected", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_RESET_request_post: Envoi un reset à un agent                                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_RESET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid")) return;

    gchar *agent_uuid = Json_get_string ( request, "agent_uuid" );
    gboolean retour = AGENT_send_to_agent ( domain, agent_uuid, "RESET", NULL );

    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is resetting", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent is not connected", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_SET_request_post: Repond aux requests depuis les browsers                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "description")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid"))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_level"))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_msrv"))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_bus"))     return;

    gint log_target = Json_get_int ( request, "log_level" );
    if (log_target<3 || log_target>7)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log", NULL );
       return;
     }

    gchar *description = Normaliser_chaine ( Json_get_string ( request, "description" ) );
    gchar *agent_uuid  = Normaliser_chaine ( Json_get_string ( request, "agent_uuid" ) );
    gboolean retour = DB_Write ( domain, "UPDATE agents SET log_msrv=%d, log_level=%d, log_bus=%d, description='%s' "
                                "WHERE agent_uuid='%d'",
                                Json_get_bool ( request, "log_msrv" ), Json_get_int ( request, "log_level" ),
                                Json_get_bool ( request, "log_bus" ), description, agent_uuid );
    g_free(agent_uuid);
    g_free(description);

    Json_node_add_string ( request, "agent_tag", "SET_LOG" );
    /* WS_Send_to_agent( domain, agent_uuid, request );*/
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_request_post: Repond aux requests AGENT depuis les agents                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_request_post ( struct DOMAIN *domain, gchar *agent_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  { if ( !strcasecmp ( api_tag, "START" ) )
     {
       if (Http_fail_if_has_not ( domain, "/run/agent", msg, request, "start_time")) return;
       if (Http_fail_if_has_not ( domain, "/run/agent", msg, request, "agent_hostname")) return;
       if (Http_fail_if_has_not ( domain, "/run/agent", msg, request, "version")) return;
       if (Http_fail_if_has_not ( domain, "/run/agent", msg, request, "install_time")) return;

       gchar *agent_hostname = Normaliser_chaine ( Json_get_string ( request, "agent_hostname") );
       gchar *version        = Normaliser_chaine ( Json_get_string ( request, "version") );
       gchar *install_time   = Normaliser_chaine ( Json_get_string ( request, "install_time") );
       DB_Write ( domain,
                  "INSERT INTO agents SET agent_uuid='%s', start_time=FROM_UNIXTIME(%d), agent_hostname='%s', "
                  "version='%s', install_time='%s' "
                  "ON DUPLICATE KEY UPDATE start_time=VALUE(start_time), agent_hostname=VALUE(agent_hostname), version=VALUE(version)",
                  agent_uuid, Json_get_int (request, "start_time"), agent_hostname, version, install_time );
       g_free(agent_hostname);
       g_free(version);
       g_free(install_time);

       JsonNode *RootNode = Http_json_node_create (msg);
       if (!RootNode) return;

       gboolean retour = DB_Read ( domain, RootNode, NULL,
                                   "SELECT * FROM agents WHERE agent_uuid='%s'", agent_uuid );
               retour &= DB_Read ( domain, RootNode, NULL,
                                   "SELECT agent_hostname AS master_hostname FROM agents WHERE is_master=1 LIMIT 1" );

       Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
     }
    else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "api_tag unknown", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
