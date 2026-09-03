/******************************************************************************************************************************/
/* ups.c                      Gestion des ups dans l'API HTTP WebService                                                      */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ups.c
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
/* UPS_LIST_request_get: Donne la liste des agents ups                                                                        */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UPS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "ups",
                                "SELECT u.*, s.agent_tech_id AS server_hostname, "
                                "       u.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM `ups` AS u INNER JOIN `server` AS s USING (`server_uuid`) "
                                "ORDER BY s.agent_tech_id, u.agent_tech_id" );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* UPS_GET_request_get: Donne la configuration et les mnemoniques d'un agent ups                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UPS_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id" )) return;

    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
    if (!agent_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { g_free(agent_tech_id); Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT u.*, s.agent_tech_id AS server_hostname, "
                                "       u.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM `ups` AS u INNER JOIN `server` AS s USING (`server_uuid`) "
                                "WHERE u.agent_tech_id='%s' LIMIT 1", agent_tech_id );

    if (retour && !Json_has_member ( RootNode, "agent_tech_id" ))
     { g_free(agent_tech_id);
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Ups agent not found", RootNode );
       return;
     }

    retour &= DB_Read ( domain, RootNode, "IO",                     /* Le thread onduleur publie des AI et des DI */
                        "SELECT mnemo_ai_id AS mnemo_id, 'AI' AS classe, tech_id, acronyme, libelle, unite, "
                        "       valeur, archivage FROM `mnemos_AI` WHERE tech_id='%s' "
                        "UNION ALL "
                        "SELECT mnemo_di_id AS mnemo_id, 'DI' AS classe, tech_id, acronyme, libelle, '' AS unite, "
                        "       etat AS valeur, archivage FROM `mnemos_DI` WHERE tech_id='%s' "
                        "ORDER BY classe, acronyme", agent_tech_id, agent_tech_id );
    g_free ( agent_tech_id );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* UPS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un ups                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UPS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "host" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "admin_username" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "admin_password" )) return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid    = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id  = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *host           = Normaliser_chaine ( Json_get_string( request, "host" ) );
    gchar *name           = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *admin_username = Normaliser_chaine ( Json_get_string( request, "admin_username" ) );
    gchar *admin_password = Normaliser_chaine ( Json_get_string( request, "admin_password" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO ups SET server_uuid='%s', agent_tech_id=UPPER('%s'), description='%s', "
                        "host='%s', name='%s', admin_username='%s', admin_password='%s' "
                        "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid), description=VALUES(description), "
                        "host=VALUES(host), name=VALUES(name), admin_username=VALUES(admin_username), admin_password=VALUES(admin_password) ",
                        server_uuid, agent_tech_id, description, host, name, admin_username, admin_password );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(description);
    g_free(host);
    g_free(name);
    g_free(admin_username);
    g_free(admin_password);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "UPS", "UPS configuration updated: agent=%s, host=%s",
                Json_get_string( request, "agent_tech_id" ), Json_get_string( request, "host" ) );
    Json_add_string ( request, "agent_classe", "ups" );
    MQTT_Send_to_domain ( domain, request, "AGENT/%s/RESTART", Json_get_string( request, "agent_tech_id" ) );
    Info ( __func__, "ups", domain->uuid, LOG_NOTICE, "Agent ups '%s' configured", Json_get_string( request, "agent_tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
