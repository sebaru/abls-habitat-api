/******************************************************************************************************************************/
/* meteo.c                      Gestion des meteo dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                22.11.2022 10:10:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * meteo.c
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
/* METEO_LIST_request_get: Donne la liste des agents meteo                                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void METEO_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "meteo",
                                "SELECT m.*, s.agent_tech_id AS server_hostname, "
                                "       m.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM `meteo` AS m INNER JOIN `server` AS s USING (`server_uuid`) "
                                "ORDER BY s.agent_tech_id, m.agent_tech_id" );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* METEO_GET_request_get: Donne la configuration et les mnemoniques d'un agent meteo                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void METEO_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id" )) return;

    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
    if (!agent_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { g_free(agent_tech_id); Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT m.*, s.agent_tech_id AS server_hostname, "
                                "       m.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM `meteo` AS m INNER JOIN `server` AS s USING (`server_uuid`) "
                                "WHERE m.agent_tech_id='%s' LIMIT 1", agent_tech_id );

    if (retour && !Json_has_member ( RootNode, "agent_tech_id" ))
     { g_free(agent_tech_id);
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Meteo agent not found", RootNode );
       return;
     }

    retour &= DB_Read ( domain, RootNode, "IO",
                        "SELECT mnemo_ai_id, tech_id, acronyme, libelle, unite, valeur, archivage, in_range "
                        "FROM `mnemos_AI` WHERE tech_id='%s' ORDER BY acronyme", agent_tech_id );
    g_free ( agent_tech_id );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* METEO_SET_request_post: Appelé depuis libsoup pour éditer ou creer un meteo                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void METEO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "code_insee" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "token" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid    = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id  = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *code_insee     = Normaliser_chaine ( Json_get_string( request, "code_insee" ) );
    gchar *token_insee    = Normaliser_chaine ( Json_get_string( request, "token" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO meteo SET server_uuid='%s', agent_tech_id=UPPER('%s'), token='%s', code_insee='%s', description='%s' "
                        "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid), code_insee=VALUES(code_insee), token=VALUES(token),"
                        "description=VALUES(description)",
                        server_uuid, agent_tech_id, token_insee, code_insee, description );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(description);
    g_free(code_insee);
    g_free(token_insee);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "METEO", "Weather configuration updated: agent=%s, code_insee=%s",
                Json_get_string( request, "agent_tech_id" ), Json_get_string( request, "code_insee" ) );
    Json_add_string ( request, "agent_classe", "meteo" );
    MQTT_Send_to_domain ( domain, request, "AGENT/%s/RESTART", Json_get_string( request, "agent_tech_id" ) );
    Info ( __func__, "meteo", domain->uuid, LOG_NOTICE, "Agent meteo '%s' configured (code_insee='%s')",
           Json_get_string( request, "agent_tech_id" ), Json_get_string( request, "code_insee" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
