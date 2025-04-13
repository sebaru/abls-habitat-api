/******************************************************************************************************************************/
/* histo.c              Déclaration des fonctions pour la gestion des historiques                                             */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                06.11.2022 15:22:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * message.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
 void HISTO_ACQUIT_request_post ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id")) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *name    = Normaliser_chaine ( Json_get_string ( token, "given_name" ) );

    gboolean retour = DB_Write ( domain, "UPDATE histo_msgs SET date_fixe=NOW(), nom_ack='%s' "
                                 "WHERE tech_id='%s' AND date_fin IS NULL AND nom_ack IS NULL ",
                                 name, tech_id );

    g_free(tech_id);
    g_free(name);
    MQTT_Send_to_domain ( domain, "master", "DLS_ACQUIT", request );
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S acquitté", NULL );
  }
/******************************************************************************************************************************/
/* HISTO_ALIVE_request_get: Renvoi les historiques vivant au user                                                             */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_ALIVE_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *url_param )
  { gchar chaine[256];
    /*if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id")) return;*/
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    g_snprintf( chaine, sizeof(chaine), "SELECT * FROM histo_msgs WHERE date_fin IS NULL " );
    if (Json_has_member ( url_param, "syn_page" ) )
     { gchar *syn_page = Normaliser_chaine ( Json_get_string ( url_param, "syn_page" ) );
       gchar complement[64];
       g_snprintf ( complement, sizeof(complement), "AND syn_page='%s' ", syn_page );
       g_free(syn_page);
       g_strlcat ( chaine, complement, sizeof(chaine) );
     }

    g_strlcat ( chaine, "ORDER BY date_create DESC", sizeof(chaine) );
    gboolean retour = DB_Read ( domain, RootNode, "histo_msgs", chaine );

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "you have histo alives", RootNode );
  }
/******************************************************************************************************************************/
/* HISTO_SEARCH_request_get: Renvoi les messages historiques selon un critère de recherche                                    */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *url_param )
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
/* HISTO_Handle_one: Traite un historique recu du Master                                                                      */
/* Entrées: le jsonnode représentant le bit interne et sa valeur                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_Handle_one ( struct DOMAIN *domain, JsonNode *source )
  { if (!Json_has_member(source, "alive"))    return;
    if (!Json_has_member(source, "tech_id"))  return;
    if (!Json_has_member(source, "acronyme")) return;

    if (Json_get_bool ( source, "alive" ) == FALSE)
     { if (!Json_has_member(source, "date_fin")) return; }
    else
     { if (!Json_has_member(source, "libelle"))     return;
       if (!Json_has_member(source, "date_create")) return;
     }

    gchar *tech_id     = Normaliser_chaine ( Json_get_string ( source, "tech_id") );
    gchar *acronyme    = Normaliser_chaine ( Json_get_string ( source, "acronyme") );
    if (Json_get_bool ( source, "alive" ) == TRUE)
     { Info_new ( __func__, LOG_DEBUG, domain, "Received MSG '%s:%s' = 1", tech_id, acronyme );
       DB_Write ( domain, "UPDATE histo_msgs SET date_fin=NOW() WHERE tech_id='%s' AND acronyme='%s' AND date_fin IS NULL", tech_id, acronyme );
       gchar *libelle     = Normaliser_chaine ( Json_get_string ( source, "libelle") );
       gchar *date_create = Normaliser_chaine ( Json_get_string ( source, "date_create") );
       DB_Write ( domain, "INSERT INTO histo_msgs SET tech_id='%s', acronyme='%s', date_create='%s', libelle='%s',"
                          "syn_page = (SELECT page FROM syns INNER JOIN dls USING (`syn_id`) WHERE dls.tech_id='%s'), "
                          "dls_shortname = (SELECT shortname FROM dls WHERE dls.tech_id='%s'), "
                          "typologie = (SELECT typologie FROM msgs WHERE msgs.tech_id='%s' AND msgs.acronyme='%s')",
                           tech_id, acronyme, date_create, libelle, tech_id, tech_id, tech_id, acronyme );
       DB_Read ( domain, source, NULL,
                 "SELECT * FROM histo_msgs WHERE tech_id='%s' AND acronyme='%s' AND date_fin IS NULL", tech_id, acronyme );
       g_free(date_create);
       g_free(libelle);
     }
    else
     { Info_new ( __func__, LOG_DEBUG, domain, "Received MSG '%s:%s' = 0", tech_id, acronyme );
       gchar *date_fin = Normaliser_chaine ( Json_get_string ( source, "date_fin") );
       DB_Write ( domain, "UPDATE histo_msgs SET date_fin='%s' WHERE tech_id='%s' AND acronyme='%s' AND date_fin IS NULL",
                  date_fin, tech_id, acronyme );
       DB_Read ( domain, source, NULL,
                 "SELECT * FROM histo_msgs WHERE tech_id='%s' AND acronyme='%s' ORDER BY date_fin DESC LIMIT 1", tech_id, acronyme );
       g_free(date_fin);
     }
    g_free(acronyme);
    g_free(tech_id);
    MQTT_Send_to_browsers ( domain, "DLS_HISTO", Json_get_string ( source, "syn_page" ), source );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
