/******************************************************************************************************************************/
/* histo.c              Déclaration des fonctions pour la gestion des historiques                                             */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                06.11.2022 15:22:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * message.c
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
/* HISTO_ACQUIT_request_post: Acquitte un dls depuis un message alive                                                         */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_ACQUIT_request_post ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id")) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *name    = Normaliser_chaine ( Json_get_string ( token, "given_name" ) );

    gboolean retour = DB_Write ( domain, "UPDATE histo_msgs SET date_fixe=NOW(), nom_ack='%s' "
                                 "WHERE tech_id='%s' AND date_fin IS NULL AND nom_ack IS NULL ",
                                 name, tech_id );

    g_free(tech_id);
    g_free(name);
    AGENT_send_to_agent ( domain, NULL, "DLS_ACQUIT", request );
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S acquitté", NULL );
  }
/******************************************************************************************************************************/
/* HISTO_ALIVE_request_get: Renvoi les historiques vivant au user                                                             */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_ALIVE_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupMessage *msg, JsonNode *url_param )
  { /*if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id")) return;*/

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "histo_msgs", "SELECT * FROM histo_msgs WHERE date_fin IS NULL ORDER BY date_create DESC" );
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "you have histo alives", RootNode );
  }
/******************************************************************************************************************************/
/* HISTO_SEARCH_request_get: Renvoi les messages historiques selon un critère de recherche                                    */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupMessage *msg, JsonNode *url_param )
  { if (Http_fail_if_has_not ( domain, path, msg, url_param, "search")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *search = Normaliser_chaine ( Json_get_string ( url_param, "search" ) );
    if (!search) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "histo_msgs", "SELECT * FROM histo_msgs WHERE "
                                "tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' OR "
                                "syn_page LIKE '%%%s%%' OR dls_shortname LIKE '%%%s%%' OR nom_ack LIKE '%%%s%%' "
                                "ORDER BY date_create DESC LIMIT 1000",
                                search, search, search, search, search, search );

    g_free(search);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "you have histo alives", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_HISTO_request_post: Enregistre un historique en base de données                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_HISTO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { gboolean retour;
    if (Http_fail_if_has_not ( domain, path, msg, request, "alive"))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id"))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme"))    return;

    if (Json_get_bool ( request, "alive" ) == FALSE)
     { if (Http_fail_if_has_not ( domain, path, msg, request, "date_fin")) return; }
    else
     { if (Http_fail_if_has_not ( domain, path, msg, request, "libelle"))     return;
       if (Http_fail_if_has_not ( domain, path, msg, request, "date_create")) return;
     }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id     = Normaliser_chaine ( Json_get_string ( request, "tech_id") );
    gchar *acronyme    = Normaliser_chaine ( Json_get_string ( request, "acronyme") );
    if (Json_get_bool ( request, "alive" ) == TRUE)
     { Info_new ( __func__, LOG_DEBUG, domain, "Received MSG '%s:%s' = 1", tech_id, acronyme );
       gchar *libelle     = Normaliser_chaine ( Json_get_string ( request, "libelle") );
       gchar *date_create = Normaliser_chaine ( Json_get_string ( request, "date_create") );
       retour = DB_Write ( domain, "INSERT INTO histo_msgs SET tech_id='%s', acronyme='%s', date_create='%s', libelle='%s',"
                                   "syn_page = (SELECT page FROM syns INNER JOIN dls USING (`syn_id`) WHERE dls.tech_id='%s'), "
                                   "dls_shortname = (SELECT shortname FROM dls WHERE dls.tech_id='%s'), "
                                   "typologie = (SELECT typologie FROM msgs WHERE msgs.tech_id='%s' AND msgs.acronyme='%s')",
                           tech_id, acronyme, date_create, libelle, tech_id, tech_id, tech_id, acronyme );
       if (domain->ws_clients) DB_Read ( domain, request, NULL,
                                         "SELECT * FROM histo_msgs WHERE tech_id='%s' AND acronyme='%s' AND date_fin IS NULL", tech_id, acronyme );
       g_free(date_create);
       g_free(libelle);
     }
    else
     { Info_new ( __func__, LOG_DEBUG, domain, "Received MSG '%s:%s' = 0", tech_id, acronyme );
       gchar *date_fin = Normaliser_chaine ( Json_get_string ( request, "date_fin") );
       retour = DB_Write ( domain, "UPDATE histo_msgs SET date_fin='%s' WHERE tech_id='%s' AND acronyme='%s' AND date_fin IS NULL",
                           date_fin, tech_id, acronyme );
       if (domain->ws_clients) DB_Read ( domain, request, NULL,
                                         "SELECT * FROM histo_msgs WHERE tech_id='%s' AND acronyme='%s' ORDER BY date_fin DESC LIMIT 1", tech_id, acronyme );
       g_free(date_fin);
     }
    g_free(acronyme);
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

    Json_node_add_string ( request, "tag", "DLS_HISTO" );
    WS_Client_send_to_all ( domain, request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Histo saved", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
