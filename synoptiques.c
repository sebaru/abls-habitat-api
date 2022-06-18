/******************************************************************************************************************************/
/* synoptiques.c                      Gestion des synoptiquess dans l'API HTTP WebService                                     */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                17.06.2022 08:32:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * synoptiques.c
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
/* SYNOPTIQUE_SET_request_post: Ajoute un synoptique                                                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if ( Json_has_member ( request, "syn_id" ) )
     { gint syn_id = Json_get_int ( request, "syn_id" );

       if (Json_has_member ( request, "image" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "image" ) );
          gboolean retour = DB_Write ( domain, "UPDATE syns SET image='%s' WHERE syn_id='%d' AND access_level<='%d'",
                                       chaine, syn_id, user_access_level );
          g_free(chaine);
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "libelle" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "libelle" ) );
          gboolean retour = DB_Write ( domain, "UPDATE syns SET libelle='%s' WHERE syn_id='%d' AND access_level<='%d'",
                                       chaine, syn_id, user_access_level );
          g_free(chaine);
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "page" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "page" ) );
          gboolean retour = DB_Write ( domain, "UPDATE syns SET page='%s' WHERE syn_id='%d' AND access_level<='%d'",
                                       chaine, syn_id, user_access_level );
          g_free(chaine);
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "mode_affichage" ) )
        { gboolean retour = DB_Write ( domain, "UPDATE syns SET mode_affichage='%d' WHERE syn_id='%d' AND access_level<='%d'",
                                       Json_get_bool ( request, "mode_affichage" ), syn_id, user_access_level );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "access_level" ) )
        { gboolean retour = DB_Write ( domain, "UPDATE syns SET access_level='%d' WHERE syn_id='%d' AND access_level<='%d'",
                                       Json_get_int ( request, "access_level" ), syn_id, user_access_level );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "parent_id" ) )
        { gboolean retour = DB_Write ( domain, "UPDATE syns SET parent_id='%d' WHERE syn_id='%d' AND access_level<='%d'",
                                       Json_get_int ( request, "parent_id" ),syn_id, user_access_level );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
          /* Recalculer arbre syn */                                                                           /* Relance DLS */
        }
       Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn updated", NULL );
       return;
     }

    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "page" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "parent_id" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "access_level" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "image" ))        return;

    gint access_level = Json_get_int ( request, "access_level" );
    if (access_level>user_access_level)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Permission Denied", NULL ); return; }

    gint   parent_id   = Json_get_int ( request, "parent_id" );
    gchar *libelle     = Normaliser_chaine ( Json_get_string( request, "libelle" ) );
    gchar *page        = Normaliser_chaine ( Json_get_string( request, "page" ) );
    gchar *image       = Normaliser_chaine ( Json_get_string( request, "image" ) );

    gboolean retour = DB_Write ( domain, "INSERT INTO syns SET libelle='%s', parent_id=%d, page='%s', image='%s', "
                                         "access_level='%d'", libelle, parent_id, page, image, access_level );

    g_free(image);
    g_free(page);
    g_free(libelle);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn added", NULL );
  }
/******************************************************************************************************************************/
/* SYNOPTIQUE_SET_request_post: Ajoute un synoptique                                                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" ))  return;

    gint syn_id = Json_get_int ( request, "syn_id" );
    if (syn_id==1)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Synoptique 1 cannot be deleted", NULL ); return; }

    gboolean retour = DB_Write ( domain,
                                 "DELETE FROM syns WHERE syn_id=%d AND access_level<='%d'",
                                 syn_id, Json_get_int ( token, "access_level" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Synoptique deleted", NULL );
  }
/******************************************************************************************************************************/
/* SYNOPTIQUE_LIST_request_post: Liste les synoptiques accessibles                                                            */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "synoptiques",
                                "SELECT syn.*, psyn.page as ppage, psyn.libelle AS plibelle, psyn.syn_id AS pid, "
                                        "(SELECT COUNT(*) FROM dls WHERE dls.syn_id=syn.syn_id) AS dls_count, "
                                        "(SELECT COUNT(*) FROM syns AS sub_syn WHERE syn.syn_id=sub_syn.parent_id) AS subsyn_count "
                                "FROM syns AS syn "
                                "INNER JOIN syns AS psyn ON psyn.syn_id=syn.parent_id "
                                "WHERE syn.access_level<='%d' ORDER BY syn.page", Json_get_int ( token, "access_level" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn list done", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
