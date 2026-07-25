/******************************************************************************************************************************/
/* agent.c                      Gestion des agents dans l'API HTTP WebService                                                 */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent.c
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

/******************************************************************************************************************************/
/* Server_load: Charge la configuration d'un server                                                                           */
/* Entrées: le domaine, les headers d'agent et le node de reponse                                                             */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                               */
/******************************************************************************************************************************/
 gboolean Server_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Write ( domain, "INSERT INTO servers SET server_uuid='%s', agent_tech_id='%s' "
                       "ON DUPLICATE KEY UPDATE agent_tech_id=VALUE(agent_tech_id)",
                       abls_headers->server_uuid, abls_headers->agent_tech_id );
    DB_Read ( domain, DstNode, NULL, "SELECT * FROM servers WHERE server_uuid='%s'", abls_headers->server_uuid );
    Json_add_bool ( DstNode, "enable", TRUE );
    if (!Json_has_member ( DstNode, "server_uuid" )) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Check_agent_classe: Vérifie qu'une classe d'agent existe                                                                   */
/* Entrées: la classe a controler                                                                                             */
/* Sortie : NULL si erreur, sinon la classe elle meme                                                                         */
/******************************************************************************************************************************/
 static gchar *Check_agent_classe ( gchar *agent_classe )
  { if (!agent_classe) return(NULL);
         if (!strcasecmp ( agent_classe, "modbus"      )) return ("modbus");
    else if (!strcasecmp ( agent_classe, "audio"       )) return ("audio");
    else if (!strcasecmp ( agent_classe, "imsgs"       )) return ("imsgs");
    else if (!strcasecmp ( agent_classe, "smsg"        )) return ("smsg");
    else if (!strcasecmp ( agent_classe, "ups"         )) return ("ups");
    else if (!strcasecmp ( agent_classe, "teleinfoedf" )) return ("teleinfoedf");
    else if (!strcasecmp ( agent_classe, "meteo"       )) return ("meteo");
    else if (!strcasecmp ( agent_classe, "gpiod"       )) return ("gpiod");
    else if (!strcasecmp ( agent_classe, "shelly"      )) return ("shelly");
    else if (!strcasecmp ( agent_classe, "phidget"     )) return ("phidget");
    else if (!strcasecmp ( agent_classe, "servers"     )) return ("servers");
    return(NULL);
  }
/******************************************************************************************************************************/
/* AGENT_get_classe: Retourne la classe d'un agent a partir de son agent_tech_id                                              */
/* Entrees: le domain et l'agent_tech_id                                                                                      */
/* Sortie : une chaine statique                                                                                               */
/******************************************************************************************************************************/
 gchar *AGENT_get_classe ( struct DOMAIN *domain, gchar *agent_tech_id )
  { if (!domain || !agent_tech_id) return(NULL);
    gboolean retour = FALSE;

    JsonNode *RootNode = Json_create();
    if (!RootNode) return(NULL);

    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    if (agent_tech_id_safe)
     { retour = DB_Read ( domain, RootNode, NULL,
                                   "SELECT agent_classe FROM agents WHERE agent_tech_id='%s' LIMIT 1",
                                   agent_tech_id_safe );
       g_free(agent_tech_id_safe);
     }
    if (!retour || !Json_has_member ( RootNode, "agent_classe" ))
     { Json_unref ( RootNode );
       return(NULL);
     }

    gchar *agent_classe = Check_agent_classe ( Json_get_string ( RootNode, "agent_classe" ) );
    Json_unref ( RootNode );
    return(agent_classe);
  }
/******************************************************************************************************************************/
/* RUN_AGENT_CONFIG_request_post: Donne la config d'un agent lors de son demarrage                                            */
/* Entrees: les elements libsoup                                                                                              */
/* Sortie : neant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_CONFIG_request_post ( struct DOMAIN *domain, gchar *path, struct ABLS_HEADERS *abls_headers, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_classe" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "version" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "start_time" ))    return;

    gchar *agent_classe  = Json_get_string ( request, "agent_classe" );
    agent_classe = Check_agent_classe ( agent_classe );
    if (!agent_classe)
     { Info ( __func__, "http", domain->uuid, LOG_ERR, "Unknown agent class '%s'", Json_get_string ( request, "agent_classe" ) );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Unknown agent class", NULL );
       return;
     }

    gchar *agent_tech_id = Json_get_string ( request, "agent_tech_id" );
    gint   start_time    = Json_get_int    ( request, "start_time" );

    if (strcmp ( abls_headers->agent_tech_id, agent_tech_id ))
     { Info ( __func__, "http", domain->uuid, LOG_WARNING, "tech_id mismatch '%s'!='%s'", abls_headers->agent_tech_id, agent_tech_id );
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "tech_id mismatch", NULL );
       return;
     }

    JsonNode *RootNode = Json_create();
    if (!RootNode)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not enought Memory", NULL ); return; }

    gboolean found = FALSE;
    if ( !strcasecmp ( agent_classe, "phidget" ) )                                         /* Chargement des infos de l'agent */
     { found = Phidget_load ( domain, abls_headers, RootNode ); }
    else if ( !strcasecmp ( agent_classe, "shelly" ) )
     { found = Shelly_load ( domain, abls_headers, RootNode ); }
    else if ( !strcasecmp ( agent_classe, "servers" ) )
     { found = Server_load ( domain, abls_headers, RootNode ); }
    else
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Unknown agent class", RootNode ); return; }

    if (!found)
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", RootNode ); return; }

    gchar *version_safe = Normaliser_chaine ( Json_get_string ( request, "version" ) );
    DB_Write ( domain, "UPDATE %s SET start_time=FROM_UNIXTIME(%d), heartbeat_time=FROM_UNIXTIME(%d), version='%s', "
                       "agent_status='Initializing' WHERE agent_tech_id='%s'",
                       agent_classe, start_time, start_time, version_safe, agent_tech_id );
    g_free(version_safe);

    gboolean retour = DB_Read ( DOMAIN_tree_get ( "master" ), RootNode, NULL,
                               "SELECT mqtt_password FROM domains WHERE domain_uuid='%s'",
                                domain->uuid );
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database error", RootNode ); return; }

/**************************************************** Ajout du l'agent Master *************************************************/
    retour = DB_Read ( domain, RootNode, NULL,
                       "SELECT agent_tech_id AS master_hostname FROM servers WHERE is_master=1 LIMIT 1" );
    if (!Json_has_member ( RootNode, "master_hostname" ))           /* Si pas de master, le premier agent connecté le devient */
     { Json_add_bool ( RootNode, "is_master", TRUE );
       DB_Write ( domain, "UPDATE servers SET is_master = 1 WHERE server_uuid = '%s'", abls_headers->server_uuid );
     }
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database error", RootNode ); return; }
/**************************************************** Ajout des logs facilities ***********************************************/
    retour = DB_Read ( domain, RootNode, "log_facilities",
                      "SELECT log_facility FROM agent_log_facilities "
                      "WHERE agent_tech_id='%s'",
                       agent_tech_id );
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database error", RootNode ); return; }

    Json_add_string ( RootNode, "mqtt_hostname", Json_get_string ( Global.config, "mqtt_hostname" ) );
    Json_add_int    ( RootNode, "mqtt_port",     Json_get_int    ( Global.config, "mqtt_port" ) );
    Json_add_bool   ( RootNode, "mqtt_over_ssl", Json_get_bool   ( Global.config, "mqtt_over_ssl" ) );
    Json_add_int    ( RootNode, "mqtt_qos",      Json_get_int    ( Global.config, "mqtt_qos" ) );
    Json_add_bool   ( RootNode, "api_cache", TRUE );

    Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent config '%s/%s' loaded (v%s, start_time=%d)",
           agent_classe, agent_tech_id, Json_get_string ( request, "version" ), Json_get_int ( request, "start_time" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent Config loaded", RootNode );
  }
/******************************************************************************************************************************/
/* AGENT_LIST_request_get: Repond aux requests depuis les browsers                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "agents",
                                "SELECT agent.*, servers.agent_tech_id AS server_hostname "
                                "FROM agents AS agent INNER JOIN servers USING(server_uuid)" );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* SERVERS_LIST_request_get: Repond aux requests depuis les browsers                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SERVERS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "servers",
                                "SELECT * FROM servers ORDER BY is_master DESC, agent_tech_id ASC" );
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

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id")) return;

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
    if (!agent_tech_id)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                               "SELECT a.*, s.agent_tech_id AS server_hostname FROM agents AS a INNER JOIN servers AS s USING (server_uuid) "
                                "WHERE a.agent_tech_id='%s' LIMIT 1",
                                agent_tech_id );
    g_free(agent_tech_id);

    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, domain->mysql_last_error, RootNode ); return; }

    if (!Json_has_member ( RootNode, "agent_tech_id" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", RootNode ); return; }

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

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id")) return;

    gchar *agent_tech_id = Json_get_string ( request, "agent_tech_id" );
    MQTT_Send_to_domain ( domain, NULL, "AGENT/%s/UPGRADE", agent_tech_id );
    Audit_log ( domain, token, "AGENT", "Upgrade for '%s' requested", agent_tech_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is upgrading", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_RESTART_request_post: Envoi une demande de redémarrage à un agent                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_RESTART_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id")) return;

    gchar *agent_tech_id = Json_get_string ( request, "agent_tech_id" );
    MQTT_Send_to_domain ( domain, NULL, "AGENT/%s/RESTART", agent_tech_id );
    Audit_log ( domain, token, "AGENT", "Restart for '%s' requested", agent_tech_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is restarting", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_START_request_post: Envoi une demande de redémarrage à un agent                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_START_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id")) return;

    gchar *agent_tech_id = Json_get_string ( request, "agent_tech_id" );
    MQTT_Send_to_domain ( domain, NULL, "AGENT/%s/START", agent_tech_id );
    Audit_log ( domain, token, "AGENT", "Start for '%s' requested", agent_tech_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is starting", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_STOP_request_post: Envoi une demande d'arrêt à un agent                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_STOP_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id")) return;

    gchar *agent_tech_id = Json_get_string ( request, "agent_tech_id" );
    MQTT_Send_to_domain ( domain, NULL, "AGENT/%s/STOP", agent_tech_id );
    Audit_log ( domain, token, "AGENT", "Stop for '%s' requested", agent_tech_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent is stopping", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_TEST_request_post: Envoi un requete de test à un agent                                                               */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
void AGENT_TEST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id")) return;

    MQTT_Send_to_domain ( domain, request, "AGENT/%s/TEST", Json_get_string ( request, "agent_tech_id" ) );                                             /* Send to all agents */
    Audit_log ( domain, token, "AGENT", "Test sent to agent '%s'", Json_get_string ( request, "agent_tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Command sent", NULL );
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
    MQTT_Send_to_domain ( domain, request, "agents/%s", tag );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tag Sent", NULL );
  }
/******************************************************************************************************************************/
/* AGENT_LOG_LEVEL_request_post: Met a jour le niveau de log d'un agent via son agent_tech_id                                 */
/* Entrees: la connexion Websocket                                                                                            */
/* Sortie : neant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_LOG_LEVEL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_level" ))     return;

    gint log_level = Json_get_int ( request, "log_level" );
    if (log_level < LOG_EMERG || log_level > LOG_DEBUG)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Mauvais niveau de log", NULL );
       return;
     }

    gchar *agent_classe = AGENT_get_classe ( domain, Json_get_string ( request, "agent_tech_id" ) );
    if (!agent_classe)
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", NULL );
       return;
     }

    gchar *agent_tech_id      = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    if (!agent_tech_id_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "agent_tech_id invalide", NULL );
       return;
     }

    gboolean retour = DB_Write ( domain, "UPDATE %s SET log_level=%d WHERE agent_tech_id='%s'",
                                 agent_classe, log_level, agent_tech_id_safe );
    g_free(agent_tech_id_safe);
    if (!retour)
     { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
       return;
     }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Memory error", NULL );
       return;
     }

    Json_add_int ( RootNode, "log_level", log_level );

    MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/LOG", agent_tech_id );
    Audit_log ( domain, token, "AGENT", "Agent '%s' log_level set to %d", agent_tech_id, log_level );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent log level updated", RootNode );
  }
/******************************************************************************************************************************/
/* AGENT_ENABLE_request_post: Active/desactive un agent via son agent_tech_id                                                 */
/* Entrees: la connexion Websocket                                                                                            */
/* Sortie : neant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable" ))        return;

    gchar *agent_classe = AGENT_get_classe ( domain, Json_get_string ( request, "agent_tech_id" ) );
    if (!agent_classe)
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", NULL );
       return;
     }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Memory error", NULL );
       return;
     }
    Json_add_string ( RootNode, "agent_classe",  agent_classe );

    gchar *agent_tech_id      = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    if (!agent_tech_id_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "agent_tech_id invalide", RootNode );
       return;
     }

    gboolean enable = Json_get_bool ( request, "enable" );
    gboolean retour = DB_Write ( domain, "UPDATE %s SET enable=%d WHERE agent_tech_id='%s'",
                                 agent_classe, enable, agent_tech_id_safe );
    retour &= DB_Read ( domain, RootNode, NULL, "SELECT server_uuid FROM %s WHERE agent_tech_id='%s'", agent_classe, agent_tech_id_safe );
    g_free(agent_tech_id_safe);
    if (!retour)
     { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
       return;
     }

    if (enable) MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/START", agent_tech_id );
           else MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/STOP",  agent_tech_id );

    Audit_log ( domain, token, "AGENT", "Agent '%s' %s", agent_tech_id, enable ? "started" : "stopped" );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent enable set", RootNode );
  }
/******************************************************************************************************************************/
/* AGENT_DELETE_request: supprime un agent de la base de données                                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void AGENT_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;

    gchar *agent_classe = AGENT_get_classe ( domain, Json_get_string ( request, "agent_tech_id" ) );
    if (!agent_classe)
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", NULL );
       return;
     }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Memory error", NULL );
       return;
     }
    Json_add_string ( RootNode, "agent_classe",  agent_classe );

    gchar *agent_tech_id      = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    if (!agent_tech_id_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "agent_tech_id invalide", RootNode );
       return;
     }


    gboolean retour = DB_Write ( domain, "DELETE FROM %s WHERE agent_tech_id='%s'", agent_classe, agent_tech_id_safe );
    retour &= DB_Write ( domain, "DELETE FROM dls WHERE tech_id='%s'", agent_tech_id_safe );
    retour &= DB_Read ( domain, RootNode, NULL, "SELECT server_uuid FROM %s WHERE agent_tech_id='%s'", agent_classe, agent_tech_id_safe );
    g_free(agent_tech_id_safe);
    if (!retour)
     { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
       return;
     }

    MQTT_Send_to_domain ( domain, RootNode, "AGENT/STOP/%s", agent_tech_id );
    Audit_log ( domain, token, "AGENT", "Agent '%s' (class '%s') deleted", agent_tech_id, agent_classe );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent deleted", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_ADD_AI_request_post: Repond aux requests AGENT des agents                                                       */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_ADD_AI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))      return;

    gchar *agent_tech_id  = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string ( request, "agent_acronyme" );
    gchar *libelle        = Json_get_string ( request, "libelle" );
    gchar *unite          = Json_get_string ( request, "unite" );
    gint   archivage      = Json_get_int    ( request, "archivage" );

    gboolean retour = Mnemo_auto_create_AI_from_thread ( domain, agent_tech_id, agent_acronyme, libelle, unite, archivage );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='%s'",
                         agent_tech_id, agent_acronyme );
    if (retour) Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent created bit AI '%s/%s'", agent_tech_id, agent_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_ADD_AO_request_post: Repond aux requests AGENT des agents                                                       */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_ADD_AO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))      return;

    gchar *agent_tech_id  = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string ( request, "agent_acronyme" );
    gchar *libelle        = Json_get_string ( request, "libelle" );
    gchar *unite          = Json_get_string ( request, "unite" );
    gint   archivage      = Json_get_int    ( request, "archivage" );

    gboolean retour = Mnemo_auto_create_AO_from_thread ( domain, agent_tech_id, agent_acronyme, libelle, unite, archivage );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='%s'",
                         agent_tech_id, agent_acronyme );
    if (retour) Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent created bit AO '%s/%s'", agent_tech_id, agent_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_ADD_DI_request_post: Repond aux requests AGENT des agents                                                       */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_ADD_DI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))        return;
    gchar *agent_tech_id  = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string ( request, "agent_acronyme" );
    gchar *libelle        = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_DI_from_thread( domain, agent_tech_id, agent_acronyme, libelle );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='%s'",
                         agent_tech_id, agent_acronyme );
    if (retour) Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent created bit DI '%s/%s'", agent_tech_id, agent_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_ADD_CI_request_post: Repond aux requests AGENT des agents                                                       */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_ADD_CI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))      return;
    gchar *agent_tech_id  = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string ( request, "agent_acronyme" );
    gchar *libelle        = Json_get_string ( request, "libelle" );
    gchar *unite          = Json_get_string ( request, "unite" );
    gint   archivage      = Json_get_int    ( request, "archivage" );

    gboolean retour = Mnemo_auto_create_CI_from_thread ( domain, agent_tech_id, agent_acronyme, libelle, unite, archivage );
    if (retour) Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent created bit CI '%s/%s'", agent_tech_id, agent_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_ADD_WATCHDOG_request_post: Repond aux requests AGENT des agents                                                 */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_ADD_WATCHDOG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))        return;
    gchar *agent_tech_id  = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string ( request, "agent_acronyme" );
    gchar *libelle        = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_WATCHDOG_from_thread( domain, agent_tech_id, agent_acronyme, libelle );
    if (retour) Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent created bit WATCHDOG '%s/%s'", agent_tech_id, agent_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_AGENT_ADD_DO_request_post: Repond aux requests AGENT des agents                                                       */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_AGENT_ADD_DO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "mono" ))           return;
    gchar *agent_tech_id  = Json_get_string ( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string ( request, "agent_acronyme" );
    gchar *libelle        = Json_get_string ( request, "libelle" );
    gboolean mono         = Json_get_bool   ( request, "mono" );

    gboolean retour = Mnemo_auto_create_DO_from_thread ( domain, agent_tech_id, agent_acronyme, libelle, mono );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='%s'",
                         agent_tech_id, agent_acronyme );
    if (retour) Info ( __func__, "agent", domain->uuid, LOG_INFO, "Agent created bit DO '%s/%s'", agent_tech_id, agent_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
