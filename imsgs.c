/******************************************************************************************************************************/
/* imsgs.c                      Gestion des imsgs dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * imsgs.c
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
/* Imsgs_load: Charge la configuration d'un agent imsgs                                                                       */
/* Entrées: le domaine, les headers d'agent et le node de reponse                                                             */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                               */
/******************************************************************************************************************************/
 gboolean Imsgs_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM imsgs WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* IMSGS_LIST_request_get: Donne la liste des agents imsgs                                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void IMSGS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "imsgs",
                                "SELECT i.*, s.agent_tech_id AS server_hostname, "
                                "       i.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM imsgs AS i INNER JOIN server AS s USING(server_uuid) "
                                "ORDER BY s.agent_tech_id, i.agent_tech_id" );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* IMSGS_GET_request_get: Donne la configuration d'un agent imsgs                                                            */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void IMSGS_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id" )) return;
    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
    if (!agent_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { g_free(agent_tech_id); Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT i.*, s.agent_tech_id AS server_hostname, "
                                "       i.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM imsgs AS i INNER JOIN server AS s USING(server_uuid) "
                                "WHERE i.agent_tech_id='%s' LIMIT 1", agent_tech_id );
    g_free(agent_tech_id);

    if (retour && !Json_has_member ( RootNode, "agent_tech_id" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Imsgs agent not found", RootNode ); return; }
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* IMSGS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un imsgs                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void IMSGS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "jabberid" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "password" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))     return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid     = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id  = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *jabberid       = Normaliser_chaine ( Json_get_string( request, "jabberid" ) );
    gchar *password       = Normaliser_chaine ( Json_get_string( request, "password" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO imsgs SET server_uuid='%s', agent_tech_id=UPPER('%s'), jabberid='%s', password='%s', description='%s' "
                        "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid), jabberid=VALUES(jabberid), password=VALUES(password),"
                        "description=VALUES(description)",
                        server_uuid, agent_tech_id, jabberid, password, description );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(description);
    g_free(jabberid);
    g_free(password);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "IMSGS", "IMSG configuration updated: agent=%s", Json_get_string( request, "agent_tech_id" ) );
    Json_add_string ( request, "agent_classe", "imsgs" );
    MQTT_Send_to_domain ( domain, request, "AGENT/%s/RESTART", Json_get_string( request, "agent_tech_id" ) );
    Info ( __func__, "imsgs", domain->uuid, LOG_NOTICE, "Agent imsgs '%s' configured", Json_get_string( request, "agent_tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
