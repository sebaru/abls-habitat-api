/******************************************************************************************************************************/
/* teleinfoedf.c                      Gestion des teleinfoedf dans l'API HTTP WebService                                      */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * teleinfoedf.c
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
/* Teleinfoedf_load: Charge la configuration d'un agent Téléinfo EDF                                                        */
/* Entrées: le domaine, les headers d'agent et le node de réponse                                                             */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                               */
/******************************************************************************************************************************/
 gboolean Teleinfoedf_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM teleinfoedf WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    return(TRUE);
  }

/******************************************************************************************************************************/
/* TELEINFOEDF_SET_request_post: Appelé depuis libsoup pour éditer ou creer un teleinfoedf                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void TELEINFOEDF_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "port" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "standard" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid    = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id  = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *port           = Normaliser_chaine ( Json_get_string( request, "port" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gboolean standard     = Json_get_bool ( request, "standard" );

    retour = DB_Write ( domain,
                        "INSERT INTO teleinfoedf SET server_uuid='%s', agent_tech_id=UPPER('%s'), "
                        "port='%s', description='%s', standard='%d' "
                        "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid), "
                        "port=VALUES(port), description=VALUES(description), standard=VALUES(standard) ",
                        server_uuid, agent_tech_id, port, description, standard );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(port);
    g_free(description);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "TELEINFOEDF", "Teleinfo EDF configuration updated: agent=%s, port=%s",
                Json_get_string( request, "agent_tech_id" ), Json_get_string( request, "port" ) );
    Json_add_string ( request, "agent_classe", "teleinfoedf" );
    MQTT_Send_to_domain ( domain, request, "THREAD/RESTART" );                          /* Stop sent to all agents */
      Info ( __func__, "teleinfoedf", domain->uuid, LOG_NOTICE, "Agent teleinfoedf '%s' configured", Json_get_string( request, "agent_tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
