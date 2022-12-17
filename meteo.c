/******************************************************************************************************************************/
/* meteo.c                      Gestion des meteo dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                22.11.2022 10:10:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * meteo.c
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
/* METEO_SET_request_post: Appelé depuis libsoup pour éditer ou creer un meteo                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void METEO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "code_insee" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "token" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;

    Json_node_add_string ( request, "thread_classe", "meteo" );
    gchar *agent_uuid     = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *code_insee     = Normaliser_chaine ( Json_get_string( request, "code_insee" ) );
    gchar *token_insee    = Normaliser_chaine ( Json_get_string( request, "token" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO meteo SET agent_uuid='%s', thread_tech_id=UPPER('%s'), token='%s', code_insee='%s', description='%s' "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), code_insee=VALUES(code_insee), token=VALUES(token),"
                        "description=VALUES(description)",
                        agent_uuid, thread_tech_id, token_insee, code_insee, description );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(description);
    g_free(code_insee);
    g_free(token_insee);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    AGENT_send_to_agent ( domain, NULL, "THREAD_STOP", request );                                  /* Stop sent to all agents */
    AGENT_send_to_agent ( domain, Json_get_string( request, "agent_uuid" ), "THREAD_START", request );               /* Start */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
