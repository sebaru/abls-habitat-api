/******************************************************************************************************************************/
/* smsg.c                      Gestion des smsg dans l'API HTTP WebService                                                    */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * smsg.c
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
/* Smsg_load: Charge la configuration d'un agent smsg                                                                         */
/* Entrées: le domaine, les headers d'agent et le node de reponse                                                             */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                               */
/******************************************************************************************************************************/
 gboolean Smsg_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM smsg WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SMSG_LIST_request_get: Donne la liste des agents smsg                                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SMSG_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "smsg",
                                "SELECT s.*, server.agent_tech_id AS server_hostname, "
                                "       s.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                                "FROM smsg AS s INNER JOIN server USING(server_uuid) "
                                "ORDER BY server.agent_tech_id, s.agent_tech_id" );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* SMSG_GET_request_get: Donne la configuration et les mnemoniques d'un agent smsg                                           */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SMSG_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id" )) return;
    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
    if (!agent_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) { g_free(agent_tech_id); Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT * FROM smsg WHERE agent_tech_id='%s' LIMIT 1", agent_tech_id );
    if (retour && !Json_has_member ( RootNode, "agent_tech_id" ))
     { g_free(agent_tech_id);
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Smsg agent not found", RootNode );
       return;
     }

    retour &= DB_Read ( domain, RootNode, "AI", "SELECT mnemo_ai_id AS mnemo_id, 'AI' AS classe, tech_id, acronyme, libelle, unite, valeur, archivage FROM mnemos_AI WHERE tech_id='%s'", agent_tech_id );
    retour &= DB_Read ( domain, RootNode, "CI", "SELECT mnemo_ci_id AS mnemo_id, 'CI' AS classe, tech_id, acronyme, libelle, unite, valeur, archivage FROM mnemos_CI WHERE tech_id='%s'", agent_tech_id );
    g_free(agent_tech_id);

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* SMSG_SET_request_post: Appelé depuis libsoup pour éditer ou creer un smsg                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SMSG_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_service_name" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_consumer_key" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_application_key" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_application_secret" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))            return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid            = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id          = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *description            = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *ovh_service_name       = Normaliser_chaine ( Json_get_string( request, "ovh_service_name" ) );
    gchar *ovh_application_key    = Normaliser_chaine ( Json_get_string( request, "ovh_application_key" ) );
    gchar *ovh_application_secret = Normaliser_chaine ( Json_get_string( request, "ovh_application_secret" ) );
    gchar *ovh_consumer_key       = Normaliser_chaine ( Json_get_string( request, "ovh_consumer_key" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO smsg SET server_uuid='%s', agent_tech_id=UPPER('%s'), "
                        "ovh_service_name='%s', ovh_application_key='%s', ovh_application_secret='%s', ovh_consumer_key='%s', description='%s' "
                        "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid), "
                        "ovh_service_name=VALUES(ovh_service_name), ovh_application_key=VALUES(ovh_application_key), "
                        "ovh_application_secret=VALUES(ovh_application_secret), ovh_consumer_key=VALUES(ovh_consumer_key),"
                        "description=VALUES(description)",
                        server_uuid, agent_tech_id, ovh_service_name, ovh_application_key, ovh_application_secret, ovh_consumer_key ,description );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(description);
    g_free(ovh_service_name);
    g_free(ovh_application_key);
    g_free(ovh_application_secret);
    g_free(ovh_consumer_key);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "SMSG", "SMS gateway configuration updated: agent=%s, service=%s",
          Json_get_string( request, "agent_tech_id" ),
                Json_get_string( request, "ovh_service_name" ) );
    Json_add_string ( request, "agent_classe", "smsg" );
    MQTT_Send_to_domain ( domain, request, "AGENT/%s/RESTART", Json_get_string( request, "agent_tech_id" ) );
    Info ( __func__, "smsg", domain->uuid, LOG_NOTICE, "Agent smsg '%s' configured", Json_get_string( request, "agent_tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
