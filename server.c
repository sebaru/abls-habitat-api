/******************************************************************************************************************************/
/* server.c                     Gestion des serveurs dans l'API HTTP WebService                                               */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                19.07.2026 18:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * server.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
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
  { DB_Write ( domain, "INSERT INTO agent_server SET server_uuid='%s', agent_tech_id=UPPER('%s') "
                       "ON DUPLICATE KEY UPDATE agent_tech_id=VALUE(agent_tech_id)",
                       abls_headers->server_uuid, abls_headers->agent_tech_id );
    DB_Read ( domain, DstNode, NULL, "SELECT * FROM agent_server WHERE server_uuid='%s'", abls_headers->server_uuid );
    if (!Json_has_member ( DstNode, "server_uuid" )) return(FALSE);

/*-------------------------------------------- Detection de manque du master -------------------------------------------------*/
    JsonNode *MasterNode = Json_create();
    if (!MasterNode) return(FALSE);
    gboolean retour = DB_Read ( domain, MasterNode, NULL, "SELECT server_uuid FROM agent_server WHERE is_master=1 LIMIT 1" );
    if (!retour)
     { Json_unref ( MasterNode );
       return(FALSE);
     }
    if (!Json_has_member ( MasterNode, "server_uuid" ))
     { retour = DB_Write ( domain, "UPDATE agent_server SET is_master=1 WHERE server_uuid='%s'", abls_headers->server_uuid );
       Json_add_bool ( DstNode, "is_master", TRUE );
     }
    Json_unref ( MasterNode );
/*-------------------------------------------- Création Agent DLS sur le master ----------------------------------------------*/
    if (Json_get_bool ( DstNode, "is_master" ))
     { DB_Write ( domain, "INSERT INTO agent_dls SET server_uuid='%s', agent_tech_id='SYS' "
                          "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid)",
                          Json_get_string ( DstNode, "server_uuid" ) );
     }
/*-------------------------------------------- Chargement des agents locaux du serveur ---------------------------------------*/
    DB_Read ( domain, DstNode, "local_agents",                                                /* Chargement des agents locaux */
              "SELECT agent_classe, agent_tech_id, description FROM agents "
              "WHERE enable=1 AND server_uuid='%s' AND agent_classe!='server'",
              abls_headers->server_uuid );
/*-------------------------------------------- Création du DLS Agent_server --------------------------------------------------*/
    Dls_create_agent_plugin ( domain, abls_headers->agent_tech_id, Json_get_string ( DstNode, "description" ), "server" );

    return(TRUE);
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
                                "SELECT * FROM agent_server ORDER BY is_master DESC, agent_tech_id ASC" );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }

/******************************************************************************************************************************/
/* SERVER_GET_request_get: Donne la configuration d'un serveur et ses agents locaux                                           */
/******************************************************************************************************************************/
 void SERVER_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    if (Http_fail_if_has_not ( domain, path, msg, url_param, "server_uuid" )) return;

    gchar *server_uuid = Normaliser_chaine ( Json_get_string ( url_param, "server_uuid" ) );
    if (!server_uuid)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for server_uuid", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode)
     { g_free ( server_uuid ); Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT *, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM agent_server WHERE server_uuid='%s' LIMIT 1", server_uuid );
    if (!retour || !Json_has_member ( RootNode, "server_uuid" ))
     { g_free ( server_uuid ); Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Server not found", RootNode ); return; }

    retour = DB_Read ( domain, RootNode, "local_agents",
                       "SELECT agent_classe, agent_tech_id, description, enable FROM agents "
                       "WHERE server_uuid='%s' AND agent_classe!='server' "
                       "ORDER BY agent_classe, agent_tech_id", server_uuid );
    g_free ( server_uuid );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }

/******************************************************************************************************************************/
/* SERVER_SET_MASTER_request_post: Modifie le flag master d'un serveur                                                        */
/* Entrees: la connexion Websocket                                                                                            */
/* Sortie : neant                                                                                                             */
/******************************************************************************************************************************/
 void SERVER_SET_MASTER_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path,
                                       SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" )) return;
    gchar *server_uuid = Json_get_string ( request, "server_uuid" );

    gboolean has_old_master = FALSE;
    JsonNode *old_master = Json_create();
    if (old_master)
     { DB_Read ( domain, old_master, NULL,
                 "SELECT server_uuid, agent_tech_id FROM agent_server WHERE is_master=1 LIMIT 1" );
     }
    gchar old_server_uuid[37];
    if (Json_has_member ( old_master, "server_uuid" ))
     { g_snprintf ( old_server_uuid, sizeof(old_server_uuid), "%s", Json_get_string ( old_master, "server_uuid" ) );
       has_old_master = TRUE;
     }
    Json_unref ( old_master );

    gchar *server_uuid_safe = Normaliser_chaine ( Json_get_string ( request, "server_uuid" ) );
    if (!server_uuid_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "server_uuid invalide", NULL );
       return;
     }

    gboolean retour = DB_Write ( domain, "UPDATE agent_server SET is_master=0" );
    retour &= DB_Write ( domain, "UPDATE agent_server SET is_master=1 WHERE server_uuid='%s'", server_uuid_safe );
    g_free(server_uuid_safe);
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, domain->mysql_last_error, NULL );
       return;
     }

    if (has_old_master)
     { MQTT_Send_to_domain ( domain, NULL, "SERVER/%s/RESTART", old_server_uuid ); }
    MQTT_Send_to_domain ( domain, NULL, "SERVER/%s/RESTART", server_uuid );
    Audit_log ( domain, token, "SERVER", "Server '%s' updated", server_uuid );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Server updated", NULL );
  }
/******************************************************************************************************************************/
/* SERVER_DELETE_request: supprime un serveur et tous les agents qui en dependent                                             */
/* Entrees: la connexion Websocket                                                                                            */
/* Sortie : neant                                                                                                             */
/******************************************************************************************************************************/
 void SERVER_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path,
                              SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 8 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" )) return;
    gchar *server_uuid = Json_get_string ( request, "server_uuid" );

    gchar *server_uuid_safe = Normaliser_chaine ( server_uuid );
    if (!server_uuid_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "server_uuid invalide", NULL );
       return;
     }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode)
     { g_free(server_uuid_safe);
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Memory error", NULL );
       return;
     }

    if (!DB_Read ( domain, RootNode, NULL, "SELECT agent_tech_id AS server_tech_id, is_master FROM agent_server WHERE server_uuid='%s'",
                   server_uuid_safe ))
     { g_free(server_uuid_safe);
       Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, domain->mysql_last_error, RootNode );
       return;
     }

    if (!Json_has_member ( RootNode, "server_tech_id" ))
     { g_free(server_uuid_safe);
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Server not found", RootNode );
       return;
     }

    if (Json_get_bool ( RootNode, "is_master" ))
     { g_free(server_uuid_safe);
       Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Suppression du serveur master interdite", RootNode );
       return;
     }

    gboolean retour = DB_Read ( domain, RootNode, "local_agents",
                                "SELECT agent_classe, agent_tech_id, description FROM agents "
                                "WHERE server_uuid='%s' AND agent_classe!='server'", server_uuid_safe );
                                                   /* Les plugins D.L.S ne sont pas en cascade sur la suppression du serveur */
    JsonArray *local_agents = Json_get_array ( RootNode, "local_agents" );
    GList *Agents = (local_agents ? json_array_get_elements ( local_agents ) : NULL);
    for (GList *agents = Agents; agents; agents = g_list_next(agents))
     { retour &= Dls_remove_plugin ( domain, Json_get_string ( agents->data, "agent_tech_id" ) ); }
    g_list_free(Agents);
    retour &= Dls_remove_plugin ( domain, Json_get_string ( RootNode, "server_tech_id" ) );
    retour &= DB_Write ( domain, "DELETE FROM agent_server WHERE server_uuid='%s'", server_uuid_safe );
    g_free(server_uuid_safe);
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, domain->mysql_last_error, RootNode );
       return;
     }

    MQTT_Send_to_domain ( domain, NULL, "SERVER/%s/STOP", server_uuid );
    Audit_log ( domain, token, "SERVER", "Server '%s' (%s) deleted", Json_get_string ( RootNode, "server_tech_id" ), server_uuid );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Server deleted", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
