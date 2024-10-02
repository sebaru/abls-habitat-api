/******************************************************************************************************************************/
/* teleinfoedf.c                      Gestion des teleinfoedf dans l'API HTTP WebService                                      */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * teleinfoedf.c
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
/* TELEINFOEDF_SET_request_post: Appelé depuis libsoup pour éditer ou creer un teleinfoedf                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void TELEINFOEDF_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "port" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "standard" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;

    Json_node_add_string ( request, "thread_classe", "teleinfoedf" );
    gchar *agent_uuid     = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *port           = Normaliser_chaine ( Json_get_string( request, "port" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gboolean standard     = Json_get_bool ( request, "standard" );

    retour = DB_Write ( domain,
                        "INSERT INTO teleinfoedf SET agent_uuid='%s', thread_tech_id=UPPER('%s'), "
                        "port='%s', description='%s', standard='%d' "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), "
                        "port=VALUES(port), description=VALUES(description), standard=VALUES(standard) ",
                        agent_uuid, thread_tech_id, port, description, standard );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(port);
    g_free(description);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    MQTT_Send_to_domain ( domain, "agents", "THREAD_RESTART", request );                           /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
