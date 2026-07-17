/******************************************************************************************************************************/
/* shelly.c                      Gestion des shelly dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                08.03.2024 22:48:23 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * shelly.c
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
/* Shelly_load: Charge la configuration d'un shelly                                                                         */
/* Entrées: le domaine, les headers d'agent et le node de reponse                                                           */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                             */
/******************************************************************************************************************************/
 gboolean Shelly_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM shelly WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    return(TRUE);
  }

/******************************************************************************************************************************/
/* SHELLY_SET_request_post: Appelé depuis libsoup pour éditer ou creer un shelly                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SHELLY_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "string_id" ))           return;

    gchar *agent_tech_id = Json_get_string( request, "agent_tech_id" );
    g_strcanon ( agent_tech_id, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid         = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id_safe  = Normaliser_chaine ( agent_tech_id );
    gchar *hostname            = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description         = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *string_id           = Normaliser_chaine ( Json_get_string( request, "string_id" ) );

    retour = DB_Write ( domain,
                       "INSERT INTO shelly SET "
                       "server_uuid='%s', agent_tech_id='%s', hostname='%s', description='%s', string_id='%s' "
                       "ON DUPLICATE KEY UPDATE server_uuid=VALUE(server_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "string_id=VALUE(string_id) ",
                        server_uuid, agent_tech_id_safe, hostname, description, string_id );

    g_free(server_uuid);
    g_free(agent_tech_id_safe);
    g_free(hostname);
    g_free(description);
    g_free(string_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "SHELLY", "Shelly agent configured: agent=%s, hostname=%s",
                Json_get_string( request, "agent_tech_id" ), Json_get_string( request, "hostname" ) );
    Json_add_string ( request, "thread_classe", "shelly" );
    MQTT_Send_to_domain ( domain, request, "RESTART/AGENT/%s", agent_tech_id );
      Info ( __func__, "shelly", domain->uuid, LOG_NOTICE, "Agent shelly '%s' configured", agent_tech_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
