/******************************************************************************************************************************/
/* tableaux.c                      Gestion des tableaux dans l'API HTTP WebService                                            */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                27.06.2023 20:32:07 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * tableaux.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* TABLEAU_SET_request_post: Ajoute ou modifie un tableau                                                                     */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "titre" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" )) return;

    gboolean retour = FALSE;
    gint syn_id = Json_get_int ( request, "syn_id" );
    gchar *titre = Normaliser_chaine ( Json_get_string ( request, "titre" ) );
    if ( Json_has_member ( request, "tableau_id" ) )
     { gint tableau_id = Json_get_int ( request, "tableau_id" );
       retour = DB_Write ( domain, "UPDATE tableau INNER JOIN syns USING(`syn_id`) "
                                   "SET titre='%s', syn_id='%d' WHERE tableau_id='%d' AND access_level<='%d'",
                                   titre, syn_id, tableau_id, user_access_level );
     }
    else
     { retour = DB_Write ( domain, "INSERT INTO tableau SET titre='%s', syn_id='%d'", titre, syn_id ); }
    g_free(titre);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau Set", NULL );
  }
/******************************************************************************************************************************/
/* TABLEAU_DELETE_request_post: Retire un tableau                                                                             */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tableau_id" )) return;
    gint user_access_level = Json_get_int ( token, "access_level" );

    gint tableau_id = Json_get_int ( request, "tableau_id" );

    gboolean retour = DB_Write ( domain,
                                 "DELETE  tableau FROM tableau INNER JOIN syns USING(`syn_id`) WHERE tableau_id=%d AND access_level<=%d",
                                 tableau_id, user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau deleted", NULL );
  }
/******************************************************************************************************************************/
/* TABLEAU_LIST_request_get: Liste les tableaux accessibles                                                                   */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "tableaux",
                                "SELECT t.*,syn.page FROM tableau AS t INNER JOIN syns AS syn USING(`syn_id`) ",
                                "WHERE syn.access_level<='%d' ORDER BY t.titre", user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau list done", RootNode );
  }
/******************************************************************************************************************************/
/* TABLEAU_MAP_LIST_request_get: Liste les map des tableaux                                                                   */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_MAP_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "tableau_id" )) return;
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tableau_id = Normaliser_chaine ( Json_get_string ( url_param, "tableau_id" ) );
    if(!tableau_id) { Http_Send_json_response ( msg, FALSE, "Normalize error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "tableau_map",
                                "SELECT m.*,dico.libelle FROM tableau_map AS m "
                                "INNER JOIN tableau AS t USING(`tableau_id`) "
                                "INNER JOIN syns AS syn USING(`syn_id`) "
                                "LEFT JOIN dictionnaire AS dico ON (m.tech_id=dico.tech_id AND m.acronyme=dico.acronyme) "
                                "WHERE syn.access_level<='%d' AND tableau_id='%s' ORDER BY m.tech_id, m.acronyme",
                                user_access_level, tableau_id );
    g_free(tableau_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau map done", RootNode );
  }
/******************************************************************************************************************************/
/* TABLEAU_MAP_DELETE_request: Retire un tableau map                                                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_MAP_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tableau_map_id" )) return;
    gint user_access_level = Json_get_int ( token, "access_level" );

    gint tableau_map_id = Json_get_int ( request, "tableau_map_id" );

    gboolean retour = DB_Write ( domain,
                                 "DELETE tableau_map FROM tableau_map "
                                 "INNER JOIN tableau AS t USING(`tableau_id`) "
                                 "INNER JOIN syns AS syn USING(`syn_id`) "
                                 "WHERE syn.access_level<='%d' AND tableau_map_id='%d'",
                                 user_access_level, tableau_map_id );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau deleted", NULL );
  }
/******************************************************************************************************************************/
/* TABLEAU_MAP_SET_request_post: Ajoute un tableau_MAP                                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_MAP_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tableau_map_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" ))       return;

    gchar *tech_id      = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *acronyme     = Normaliser_chaine ( Json_get_string ( request, "acronyme" ) );
    gint tableau_map_id = Json_get_int ( request, "tableau_map_id" );
    gboolean retour = DB_Write ( domain, "UPDATE tableau_map INNER JOIN tableau USING(`tableau_id`) INNER JOIN syns USING(`syn_id`) "
                                         "SET tech_id='%s', acronyme='%d' WHERE tableau_map_id='%d' AND access_level<='%d'",
                                         tech_id, acronyme, tableau_map_id, user_access_level );
    g_free(tech_id);
    g_free(acronyme);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "TableauMap Set", NULL );
  }
/******************************************************************************************************************************/
/* TABLEAU_MAP_ADD_request_post: Ajoute un tableau_MAP                                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_MAP_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "tableau_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" ))   return;

    gint tableau_id = Json_get_int ( request, "tableau_id" );
    gchar *tech_id  = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string ( request, "acronyme" ) );
    gboolean retour = DB_Write ( domain, "INSERT INTO tableau_map SET tableau_id='%d', tech_id='%s', acronyme='%s'", tableau_id, tech_id, acronyme );
    g_free(tech_id);
    g_free(acronyme);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "TableauMap Add", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
