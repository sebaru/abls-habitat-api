/******************************************************************************************************************************/
/* ups.c                      Gestion des ups dans l'API HTTP WebService                                                      */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ups.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* UPS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un ups                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UPS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "host" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "admin_username" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "admin_password" )) return;

    Json_node_add_string ( request, "thread_classe", "ups" );
    gchar *agent_uuid     = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *host           = Normaliser_chaine ( Json_get_string( request, "host" ) );
    gchar *name           = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *admin_username = Normaliser_chaine ( Json_get_string( request, "admin_username" ) );
    gchar *admin_password = Normaliser_chaine ( Json_get_string( request, "admin_password" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO ups SET agent_uuid='%s', thread_tech_id=UPPER('%s'), "
                        "host='%s', name='%s', admin_username='%s', admin_password='%s' "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), "
                        "host=VALUES(host), name=VALUES(name), admin_username=VALUES(admin_username), admin_password=VALUES(admin_password) ",
                        agent_uuid, thread_tech_id, host, name, admin_username, admin_password );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(host);
    g_free(name);
    g_free(admin_username);
    g_free(admin_password);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    AGENT_send_to_agent ( domain, NULL, "THREAD_RESTART", request );                               /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
