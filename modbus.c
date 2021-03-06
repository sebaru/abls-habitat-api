/******************************************************************************************************************************/
/* modbus.c                      Gestion des modbus dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * modbus.c
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
/* MODBUS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un modbus                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "watchdog" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "max_request_par_sec" )) return;

    Json_node_add_string ( request, "thread_classe", "modbus" );
    gchar *agent_uuid          = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id      = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *hostname            = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description         = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gint   watchdog            = Json_get_int( request, "watchdog" );
    gint   max_request_par_sec = Json_get_int( request, "max_request_par_sec" );

    retour = DB_Write ( domain,
                       "INSERT INTO modbus SET "
                       "agent_uuid='%s', thread_tech_id='%s', hostname='%s', description='%s', watchdog='%d', max_request_par_sec='%d' "
                       "ON DUPLICATE KEY UPDATE agent_uuid=VALUE(agent_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "watchdog=VALUE(watchdog), max_request_par_sec=VALUE(max_request_par_sec) ",
                       agent_uuid, thread_tech_id, hostname, description, watchdog, max_request_par_sec );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(hostname);
    g_free(description);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    AGENT_send_to_agent ( domain, NULL, "THREAD_STOP", request );                                  /* Stop sent to all agents */
    AGENT_send_to_agent ( domain, Json_get_string( request, "agent_uuid" ), "THREAD_START", request );               /* Start */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* MODBUS_LIST_request_post: Appelé depuis libsoup pour l'URI modbus/list                                                     */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "classe")) return;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = FALSE;
    gchar *classe = Json_get_string ( request, "classe" );
         if (!strcasecmp ( classe, "modbus" ))
          { retour = DB_Read ( domain, RootNode, "modbus", "SELECT modbus.*, agent_hostname FROM modbus INNER JOIN agents USING(agent_uuid)" ); }
    else if (!strcasecmp ( classe, "AI" ))
          { retour = DB_Read ( domain, RootNode, "AI",
                               "SELECT * FROM modbus_AI AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "AO" ))
          { retour = DB_Read ( domain, RootNode, "AO",
                               "SELECT * FROM modbus_AO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "DI" ))
          { retour = DB_Read ( domain, RootNode, "DI",
                               "SELECT * FROM modbus_DI AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "DO" ))
          { retour = DB_Read ( domain, RootNode, "DO",
                               "SELECT * FROM modbus_DO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
