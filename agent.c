/******************************************************************************************************************************/
/* agent.c                      Gestion des agents dans l'API HTTP WebService                                                 */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* AGENT_LIST_request_get: Repond aux requests depuis les browsers                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "agents", "SELECT * FROM agents" );
    Json_node_foreach_array_element ( RootNode, "agents", AGENT_LIST_Add_ws_status, domain );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* AGENT_GET_request_get: Repond aux requests depuis les browsers                                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_uuid")) return;

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gchar *agent_uuid = Normaliser_chaine ( Json_get_string ( url_param, "agent_uuid" ) );
    gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT * FROM agents WHERE agent_uuid='%s'", agent_uuid );
    retour &= DB_Read ( DOMAIN_tree_get("master"), RootNode, NULL,
                        "SELECT domain_secret FROM domains WHERE domain_uuid='%s'", Json_get_string ( domain->config, "domain_uuid" ) );
    Json_node_add_string ( RootNode, "api_url", Json_get_string ( Global.config, "api_public_url" ) );
    g_free(agent_uuid);

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* AGENT_send_to_agent: Envoi un json à l'agent en parametre                                                                  */
/* Entrées: le domain, l'agent_uuid, l'tag et le json source                                                              */
/* Sortie : FALSE si pas trouvé                                                                                               */
/******************************************************************************************************************************/
 gboolean AGENT_send_to_agent ( struct DOMAIN *domain, gchar *agent_uuid, gchar *agent_tag, JsonNode *node )
  { gboolean retour = FALSE, free_node = FALSE;

    if (!node) { node = Json_node_create(); free_node = TRUE; }
    Json_node_add_string ( node, "agent_tag", agent_tag );

    gchar *buf = Json_node_to_string ( node );
    if (!buf) goto end;

    pthread_mutex_lock ( &domain->synchro );
    GSList *liste = domain->ws_agents;
    while (liste)
     { struct WS_AGENT_SESSION *ws_agent = liste->data;
       if (agent_uuid == NULL || !strcmp ( agent_uuid, ws_agent->agent_uuid ) )
        { soup_websocket_connection_send_text ( ws_agent->connexion, buf );
          Info_new ( __func__, LOG_INFO, domain, "'%s' sent to agent '%s'", agent_tag, ws_agent->agent_uuid );
          retour = TRUE;
        }
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock ( &domain->synchro );
    g_free(buf);
end:
    if (free_node) json_node_unref ( node );
    return(retour);
  }
/******************************************************************************************************************************/
/* AGENT_UPGRADE_request_post: Envoi une demande d'upgrade à un agent                                                         */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_UPGRADE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid")) return;

    gchar *agent_uuid = Json_get_string ( request, "agent_uuid" );
    gboolean retour = AGENT_send_to_agent ( domain, agent_uuid, "UPGRADE", NULL );

    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is upgrading", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent is not connected", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_SEND_request_post: Envoi un tag aux agents (ex: remap, reload horloge)                                               */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tag" )) return;

    gchar *tag = Json_get_string ( request, "tag" );
    gboolean retour = AGENT_send_to_agent ( domain, NULL, tag, request );

    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tag Sent", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent is not connected", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_RESET_request_post: Envoi un reset à un agent                                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_RESET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
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
 void AGENT_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "description")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid"))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "headless"))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_level"))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_msrv"))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_bus"))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "branche"))     return;

    gint log_target = Json_get_int ( request, "log_level" );
    if (log_target<3 || log_target>7)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log", NULL );
       return;
     }

    gchar *description = Normaliser_chaine ( Json_get_string ( request, "description" ) );
    gchar *agent_uuid  = Normaliser_chaine ( Json_get_string ( request, "agent_uuid" ) );
    gboolean retour = DB_Write ( domain,
                                "UPDATE agents SET headless='%d', log_msrv=%d, log_level=%d, log_bus=%d, description='%s' "
                                "WHERE agent_uuid='%s'",
                                Json_get_bool ( request, "headless" ),
                                Json_get_bool ( request, "log_msrv" ), Json_get_int ( request, "log_level" ),
                                Json_get_bool ( request, "log_bus" ), description, agent_uuid );
    g_free(agent_uuid);
    g_free(description);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    retour = AGENT_send_to_agent ( domain, Json_get_string ( request, "agent_uuid" ), "AGENT_SET", request );

    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent updated", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent is not connected", NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_START_request_post: Repond aux requests AGENT depuis les agents                                                  */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_START_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "start_time")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_hostname")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "version")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "branche")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "install_time")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *agent_hostname = Normaliser_chaine ( Json_get_string ( request, "agent_hostname") );
    gchar *version        = Normaliser_chaine ( Json_get_string ( request, "version") );
    gchar *branche        = Normaliser_chaine ( Json_get_string ( request, "branche") );
    gchar *install_time   = Normaliser_chaine ( Json_get_string ( request, "install_time") );
    DB_Write ( domain,
               "INSERT INTO agents SET agent_uuid='%s', start_time=FROM_UNIXTIME(%d), agent_hostname='%s', "
               "version='%s', branche='%s', install_time='%s' "
               "ON DUPLICATE KEY UPDATE start_time=VALUE(start_time), "
               "agent_hostname=VALUE(agent_hostname), version=VALUE(version), branche=VALUE(branche)",
               agent_uuid, Json_get_int (request, "start_time"), agent_hostname, version, branche, install_time );

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT * FROM agents WHERE agent_uuid='%s'", agent_uuid );
            retour &= DB_Read ( domain, RootNode, NULL,
                                "SELECT agent_hostname AS master_hostname FROM agents WHERE is_master=1 LIMIT 1" );
    if (!Json_has_member ( RootNode, "master_hostname" ))           /* Si pas de master, le premier agent connecté le devient */
     { Json_node_add_bool ( RootNode, "is_master", TRUE );
       DB_Write ( domain, "UPDATE agents SET is_master = 1 WHERE agent_hostname = '%s'", agent_hostname );
     }
    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */

    g_free(agent_hostname);
    g_free(version);
    g_free(branche);
    g_free(install_time);

    Info_new ( __func__, LOG_INFO, domain, "Agent '%s' (%s) is started", agent_uuid, Json_get_string ( request, "agent_hostname") );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
            else { Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent start OK", RootNode ); }
  }
/******************************************************************************************************************************/
/* AGENT_SET_MASTER_request_post: Promouvoie un agent en tant que master                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_SET_MASTER_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid")) return;

    gchar *agent_uuid  = Normaliser_chaine ( Json_get_string ( request, "agent_uuid" ) );

    gboolean retour  = DB_Write ( domain, "UPDATE agents SET is_master=0" );
             retour &= DB_Write ( domain, "UPDATE agents SET is_master=1 WHERE agent_uuid='%s'", agent_uuid );

    g_free(agent_uuid);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Info_new ( __func__, LOG_INFO, domain, "Agent '%s' is new master", agent_uuid );

    retour = AGENT_send_to_agent ( domain, NULL, "RESET", NULL );                                         /* Reset all agents */

    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agents resetted", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agents are not connected", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
