/******************************************************************************************************************************/
/* dls_package.c                      Gestion des package dls dans l'API HTTP WebService                                      */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                30.01.2025 21:05:55 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls_package.c
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
/* DLS_PACKAGE_LIST_request_get: Liste les packages DLS                                                                       */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "dls_packages",
                                "SELECT dls_package_id, name FROM `dls_packages` ORDER BY name" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of Packages", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_PACKAGE_SOURCE_request_get: Renvoie la code source du package DLS                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_SOURCE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "name" )) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *name = Normaliser_chaine ( Json_get_string ( url_param, "name" ) );         /* Formatage correct des chaines */
    if (!name) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT * FROM `dls_packages` WHERE name='%s'", name );
    g_free(name);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Package SourceCode sent", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_PACKAGE_SET_request_post: Appelé depuis libsoup pour éditer ou creer un package                                        */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour = FALSE;
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "sourcecode" )) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *name       = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *sourcecode = Normaliser_chaine ( Json_get_string( request, "sourcecode" ) );
    if (!(name && sourcecode)) goto end;

    if (Json_has_member ( request, "dls_package_id" ) )
     { gint dls_package_id = Json_get_int ( request, "dls_package_id" );
       DB_Read ( domain, RootNode, NULL, "SELECT name FROM dls_packages WHERE dls_package_id='%d'", dls_package_id );

       if ( !Json_has_member ( RootNode, "name" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "DLS unknown", RootNode ); goto end; }

       retour = DB_Write ( domain, "UPDATE dls_packages "
                                   "SET name='%s', sourcecode='%s' WHERE dls_package_id='%d' ",
                                    name, sourcecode, dls_package_id );

       if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); goto end; }
     }
    else                                                                                            /* Ajout d'un package DLS */
     { retour = DB_Write ( domain, "INSERT INTO dls_packages "
                                   "SET name='%s', sourcecode='%s' ",
                                   name, sourcecode );                                          /* Création, sans compilation */
     }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    else Http_Send_json_response ( msg, SOUP_STATUS_OK, "DLS package changed", RootNode );

end:
    if (name)       g_free(name);
    if (sourcecode) g_free(sourcecode);
  }
/******************************************************************************************************************************/
/* DLS_PACKAGE_DELETE_request: Supprime un package DLS                                                                        */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "dls_package_id" )) return;
    gint dls_package_id = Json_get_int ( request, "dls_package_id" );
    gboolean retour = DB_Write ( domain, "DELETE dls_packages WHERE dls_package_id='%d'", dls_package_id );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Package deleted", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
