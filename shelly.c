/******************************************************************************************************************************/
/* shelly.c                      Gestion des shelly dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                08.03.2024 22:48:23 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * shelly.c
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
/* SHELLY_SET_request_post: Appelé depuis libsoup pour éditer ou creer un shelly                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SHELLY_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "string_id" ))           return;

    gchar *agent_uuid          = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id      = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *hostname            = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description         = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *string_id           = Normaliser_chaine ( Json_get_string( request, "string_id" ) );

    retour = DB_Write ( domain,
                       "INSERT INTO shelly SET "
                       "agent_uuid='%s', thread_tech_id='%s', hostname='%s', description='%s', string_id='%s' "
                       "ON DUPLICATE KEY UPDATE agent_uuid=VALUE(agent_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "string_id=VALUE(string_id) ",
                       agent_uuid, thread_tech_id, hostname, description, string_id );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(hostname);
    g_free(description);
    g_free(string_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_string ( request, "thread_classe", "shelly" );
    MQTT_Send_to_domain ( domain, "agents", "THREAD_RESTART", request );                           /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
