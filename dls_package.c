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
                                "SELECT dls_package_id, name, description FROM `dls_packages` ORDER BY name" );

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
/* DLS_PACKAGE_SET_request_post: Appelé depuis libsoup pour éditer un package                                                 */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour = TRUE;
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "dls_package_id" )) return;
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gint dls_package_id = Json_get_int ( request, "dls_package_id" );

    if (Json_has_member ( request, "name" ))
     { gchar *name = Normaliser_chaine ( Json_get_string( request, "name" ) );
       if (name)
        { retour &= DB_Write ( domain, "UPDATE dls_packages SET name='%s' WHERE dls_package_id='%d'", name, dls_package_id );
          g_free(name);
        }
       else Info_new( __func__, LOG_ERR, domain, "'%05d': Normalize Name Error", dls_package_id );
     }

    if (Json_has_member ( request, "description" ))
     { gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
       if (description)
        { retour &= DB_Write ( domain, "UPDATE dls_packages SET description='%s' WHERE dls_package_id='%d'", description, dls_package_id );
          g_free(description);
        }
       else Info_new( __func__, LOG_ERR, domain, "'%05d': Normalize Description Error", dls_package_id );
     }

    if (Json_has_member ( request, "sourcecode" ))
     { gchar *sourcecode = Normaliser_chaine ( Json_get_string( request, "sourcecode" ) );
       if (sourcecode)
        { retour &= DB_Write ( domain, "UPDATE dls_packages SET description='%s' WHERE dls_package_id='%d'", sourcecode, dls_package_id );
          g_free(sourcecode);
        }
       else Info_new( __func__, LOG_ERR, domain, "'%05d': Normalize Sourcecode Error", dls_package_id );
     }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    else Http_Send_json_response ( msg, SOUP_STATUS_OK, "DLS package changed", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_PACKAGE_SAVE_request_post: Appelé depuis libsoup pour éditer un package                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_SAVE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "dls_package_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "sourcecode" ))     return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gint dls_package_id = Json_get_int ( request, "dls_package_id" );
    gchar *sourcecode = Normaliser_chaine ( Json_get_string ( request, "sourcecode" ) );
    if (sourcecode)
     { gboolean retour = DB_Write ( domain, "UPDATE dls_packages SET sourcecode='%s' WHERE dls_package_id=%d", sourcecode, dls_package_id );
       if (!retour)
          { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
       else Http_Send_json_response ( msg, SOUP_STATUS_OK, "DLS package changed", RootNode );
     }
    else Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode );
    if (sourcecode) g_free(sourcecode);
  }
/******************************************************************************************************************************/
/* DLS_PACKAGE_ADD_request_post: Appelé depuis libsoup pour creer un package                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PACKAGE_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" )) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *name        = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *description = Normaliser_chaine ( Json_get_string( request, "description" ) );
    if (name && description)
     { gboolean retour = DB_Write ( domain, "INSERT INTO dls_packages SET name='%s', description='%s'",           /* Création */
                                    name, description );
       if (!retour)
          { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
       else Http_Send_json_response ( msg, SOUP_STATUS_OK, "DLS package changed", RootNode );
     }
    else Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode );
    if (name)        g_free(name);
    if (description) g_free(description);
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
    gboolean retour = DB_Write ( domain, "DELETE FROM dls_packages WHERE dls_package_id='%d'", dls_package_id );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Package deleted", NULL );
  }
/******************************************************************************************************************************/
/* Dls_Apply_package: Applique le package au tech_id en parametre                                                             */
/* Entrée: Le domain et le tech_id du plugin, dans le jsonnode                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Dls_Apply_package ( struct DOMAIN *domain, JsonNode *PluginNode )
  { if (!Json_has_member ( PluginNode, "tech_id" )) return(FALSE);
    gchar *tech_id = Json_get_string( PluginNode, "tech_id" );
    Info_new( __func__, LOG_INFO, domain, "'%s': Searching for a package", tech_id );

    gboolean retour = DB_Read ( domain, PluginNode, NULL,
                                "SELECT package, enable, syn_id, page FROM dls "
                                "INNER JOIN syns USING(`syn_id`) "
                                "WHERE tech_id='%s'", tech_id );
    if (!retour)
    { Info_new( __func__, LOG_ERR, domain, "'%s': DB read error", tech_id );
      return(FALSE);
    }

    if (!Json_has_member( PluginNode, "package" ))
    { Info_new( __func__, LOG_ERR, domain, "'%s': tech_id not not found", tech_id );
      return(FALSE);
    }

    gchar *package = Json_get_string ( PluginNode, "package" );                                     /* S'agit-il d'un package */
    if ( ! (package && strlen(package) && strcasecmp ( package, "custom" ) ) )                     /* Ce n'est pas un package */
     { if (Json_has_member ( PluginNode, "sourcecode" ) )
        { Info_new( __func__, LOG_INFO, domain, "'%s': is not a package. Using embedded sourcecode from jsonnode", tech_id );
          return(TRUE);
        }
       Info_new( __func__, LOG_INFO, domain, "'%s': is not a package. Using sourcecode from database", tech_id );
       DB_Read ( domain, PluginNode, NULL, "SELECT sourcecode FROM dls WHERE tech_id='%s'", tech_id );
       if (!Json_has_member( PluginNode, "sourcecode" ))
        { Info_new( __func__, LOG_ERR, domain, "'%s': sourcode not not found in database.", tech_id );
          return(FALSE);
        }
       return(TRUE);
     }

    Info_new( __func__, LOG_INFO, domain, "'%s': Applying package '%s'", tech_id, package );

/**************************************** Essaie avec un package local s'il existe ********************************************/
    gchar *name = Normaliser_chaine ( package );
    if (!name)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Memory error", tech_id );
       return(FALSE);
     }

    retour = DB_Read ( domain, PluginNode, NULL, "SELECT sourcecode FROM `dls_packages` WHERE name='%s'", name );
    g_free(name);
    if (!retour)
     { Info_new( __func__, LOG_ERR, domain, "'%s': DB Sourcecode error", tech_id );
       return(FALSE);
     }

    if ( Json_has_member ( PluginNode, "sourcecode" ) ) return(TRUE);            /* Si le sourcecode est trouvé, on a terminé */

/************************************ Sinon on essaie de le télécharger depuis le site static *********************************/
    SoupSession *session = soup_session_new();
    gchar package_query[256];
    GError *error = NULL;

    g_snprintf( package_query, sizeof(package_query), "https://static.abls-habitat.fr/package/%s.dls", package );
    SoupMessage *soup_msg = soup_message_new ( "GET", package_query );
    GBytes *response      = soup_session_send_and_read ( session, soup_msg, NULL, &error ); /* SYNC */
    gchar *reason_phrase  = soup_message_get_reason_phrase(soup_msg);
    gint   status_code    = soup_message_get_status(soup_msg);
    retour = FALSE;

    if (error)
     { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
       Info_new( __func__, LOG_ERR, domain, "'%s': Unable to retrieve Package '%s': error %s", tech_id, package_query, error->message );
       g_free(uri);
       g_error_free ( error );
     }
    else if (status_code==200)
     { gsize taille;
       gchar *buffer_unsafe = g_bytes_get_data ( response, &taille );
       gchar *buffer_safe   = g_try_malloc0 ( taille + 1 );
       if (taille && buffer_safe)
        { memcpy ( buffer_safe, buffer_unsafe, taille );                                        /* Copy with \0 end of string */
          Json_node_add_string ( PluginNode, "sourcecode", buffer_safe );             /* Recopie dans la structure de travail */
          g_free(buffer_safe);
          retour = TRUE;
        }
     }
    else
     { Info_new( __func__, LOG_ERR, domain, "Unable to retrieve Package '%s': %s", package_query, reason_phrase ); }
    g_object_unref( soup_msg );
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
