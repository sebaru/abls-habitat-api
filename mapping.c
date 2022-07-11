/******************************************************************************************************************************/
/* mapping.c                      Gestion des mappings dans l'API HTTP WebService                                             */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.06.2022 08:44:13 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mapping.c
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
/* MAPPING_SET_request_post: Ajoute un mapping                                                                                */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void MAPPING_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" ))        return;

    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *thread_acronyme = Normaliser_chaine ( Json_get_string( request, "thread_acronyme" ) );
    gchar *tech_id         = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme        = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );

    gboolean retour = DB_Write ( domain, "UPDATE mappings SET tech_id = NULL, acronyme = NULL "
                                         "WHERE tech_id = '%s' AND acronyme = '%s'", tech_id, acronyme );

            retour &= DB_Write ( domain,
                                 "INSERT INTO mappings SET "
                                 "thread_tech_id = UPPER('%s'), thread_acronyme = UPPER('%s'), tech_id = UPPER('%s'), acronyme = '%s' "
                                 "ON DUPLICATE KEY SET tech_id=VALUES(tech_id), acronyme=VALUES(acronyme) ",
                                 thread_tech_id, thread_acronyme, tech_id, acronyme );

    g_free(acronyme);
    g_free(tech_id);
    g_free(thread_acronyme);
    g_free(thread_tech_id);

    AGENT_send_to_agent ( domain, NULL, "REMAP", NULL );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping done", NULL );
  }
/******************************************************************************************************************************/
/* RUN_MAPPING_LIST_request_post: Repond aux requests AGENT depuis pour les mappings                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_MAPPING_LIST_request_post ( struct DOMAIN *domain, gchar *path, gchar *mappings_uuid, SoupMessage *msg, JsonNode *request )
  {
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "mappings", "SELECT * FROM mappings WHERE tech_id IS NOT NULL AND acronyme IS NOT NULL" );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping sent", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_MAPPING_LIST_request_post: Repond aux requests AGENT depuis pour les mappings                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_MAPPING_SEARCH_TXT_request_post ( struct DOMAIN *domain, gchar *path, gchar *mappings_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" ))  return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *thread_acronyme = Normaliser_chaine ( Json_get_string ( request, "thread_acronyme" ) );/* Formatage correct des chaines */
    if (!thread_acronyme) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "results",
                                "SELECT * FROM mappings WHERE thread_tech_id='_COMMAND_TEXT' AND thread_acronyme LIKE '%%%s%%'",
                                thread_acronyme );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping sent", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
