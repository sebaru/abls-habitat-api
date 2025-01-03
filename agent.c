/******************************************************************************************************************************/
/* agent.c                      Gestion des agents dans l'API HTTP WebService                                                 */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
/* AGENT_LIST_request_get: Repond aux requests depuis les browsers                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "agents",
                                "SELECT *, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM agents" );

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
/* AGENT_UPGRADE_request_post: Envoi une demande d'upgrade à un agent                                                         */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_UPGRADE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid")) return;

    gchar *agent_uuid = Json_get_string ( request, "agent_uuid" );
    MQTT_Send_to_domain ( domain, agent_uuid, "UPGRADE", NULL );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is upgrading", NULL );
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
    MQTT_Send_to_domain ( domain, "agents", tag, request );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tag Sent", NULL );
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
    MQTT_Send_to_domain ( domain, agent_uuid, "RESET", NULL );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is resetting", NULL );
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
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_dls"))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "branche"))     return;

    gint log_target = Json_get_int ( request, "log_level" );
    if (log_target<3 || log_target>7)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log", NULL );
       return;
     }

    gchar *description = Normaliser_chaine ( Json_get_string ( request, "description" ) );
    gchar *agent_uuid  = Normaliser_chaine ( Json_get_string ( request, "agent_uuid" ) );
    gboolean retour = DB_Write ( domain,
                                "UPDATE agents SET headless='%d', log_msrv=%d, log_dls='%d', log_level=%d, log_bus=%d, description='%s' "
                                "WHERE agent_uuid='%s'",
                                Json_get_bool ( request, "headless" ), Json_get_bool ( request, "log_msrv" ),
                                Json_get_int ( request, "log_dls" ), Json_get_int ( request, "log_level" ),
                                Json_get_bool ( request, "log_bus" ), description, agent_uuid );
    g_free(agent_uuid);
    g_free(description);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    MQTT_Send_to_domain ( domain, Json_get_string ( request, "agent_uuid" ), "AGENT_SET", request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent updated", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_DELETE_request: supprime un agent de la base de données                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid"))  return;

    gchar *agent_uuid  = Normaliser_chaine ( Json_get_string ( request, "agent_uuid" ) );
    gboolean retour = DB_Write ( domain, "DELETE FROM agents WHERE agent_uuid='%s'", agent_uuid );
    g_free(agent_uuid);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    MQTT_Send_to_domain ( domain, Json_get_string ( request, "agent_uuid" ), "AGENT_DELETE", request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent deleted", NULL );
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

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *agent_hostname = Normaliser_chaine ( Json_get_string ( request, "agent_hostname") );
    gchar *version        = Normaliser_chaine ( Json_get_string ( request, "version") );
    gchar *branche        = Normaliser_chaine ( Json_get_string ( request, "branche") );
    DB_Write ( domain,
               "INSERT INTO agents SET agent_uuid='%s', start_time=FROM_UNIXTIME(%d), agent_hostname='%s', "
               "version='%s', branche='%s', install_time=NOW(), heartbeat_time=NOW() "
               "ON DUPLICATE KEY UPDATE start_time=VALUE(start_time), heartbeat_time=VALUE(heartbeat_time),"
               "agent_hostname=VALUE(agent_hostname), version=VALUE(version), branche=VALUE(branche)",
               agent_uuid, Json_get_int (request, "start_time"), agent_hostname, version, branche );

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

    retour &= DB_Read ( DOMAIN_tree_get ( "master" ), RootNode, NULL,
                       "SELECT mqtt_password FROM domains WHERE domain_uuid='%s'", Json_get_string ( domain->config, "domain_uuid") );

    Json_node_add_string ( RootNode, "mqtt_hostname", Json_get_string ( Global.config, "mqtt_hostname" ) );
    Json_node_add_int    ( RootNode, "mqtt_port",     Json_get_int    ( Global.config, "mqtt_port" ) );
    Json_node_add_bool   ( RootNode, "mqtt_over_ssl", Json_get_bool   ( Global.config, "mqtt_over_ssl" ) );
    Json_node_add_bool   ( RootNode, "mqtt_qos",      Json_get_int    ( Global.config, "mqtt_qos" ) );

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

    MQTT_Send_to_domain ( domain, "agent", "RESET", NULL );                                               /* Reset all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agents resetted", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
