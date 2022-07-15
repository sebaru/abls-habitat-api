/******************************************************************************************************************************/
/* dls.c                      Gestion des dls dans l'API HTTP WebService                                                      */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls.c
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
/* DLS_LIST_request_post: Liste les modules DLS                                                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "dls",
                                "SELECT d.dls_id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.actif, d.compil_status, "
                                "d.nbr_compil, d.nbr_ligne, d.compil_date, d.debug, ps.page as ppage, s.page as page "
                                "FROM dls AS d "
                                "INNER JOIN syns as s  ON d.syn_id = s.syn_id "
                                "INNER JOIN syns as ps ON s.parent_id = ps.syn_id "
                                "WHERE s.access_level<='%d' ORDER BY d.tech_id", user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of D.L.S", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_SOURCE_request_post: Renvoie la code source DLS d'un module                                                            */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_SOURCE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))   return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );               /* Formatage correct des chaines */
    if (!tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT d.dls_id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.actif, d.compil_status, "
                                "d.nbr_compil, d.nbr_ligne, d.compil_date, d.debug, ps.page as ppage, s.page as page, "
                                "d.sourcecode, d.errorlog "
                                "FROM dls AS d "
                                "INNER JOIN syns as s  ON d.syn_id = s.syn_id "
                                "INNER JOIN syns as ps ON s.parent_id = ps.syn_id "
                                "WHERE s.access_level<='%d' AND tech_id='%s'", user_access_level, tech_id );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "SourceCode sent", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un dls                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { gboolean retour = FALSE;
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "shortname" )) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id   = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *name      = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *shortname = Normaliser_chaine ( Json_get_string( request, "shortname" ) );
    gint   syn_id    = Json_get_int ( request, "syn_id" );

    if (Json_has_member ( request, "dls_id" ) )
     { gint dls_id = Json_get_int ( request, "dls_id" );
       DB_Read ( domain, RootNode, NULL, "SELECT tech_id FROM dls WHERE dls_id='%d'", dls_id );

       if ( !Json_has_member ( RootNode, "tech_id" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "DLS unknown", RootNode ); goto end; }

       retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                   "SET syn_id='%d', tech_id='%s', shortname='%s', name='%s' WHERE dls_id='%d' "
                                   "AND syns.access_level <= %d",
                                    syn_id, tech_id, shortname, name, dls_id, user_access_level );

       if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); goto end; }

       gchar *old_tech_id = Json_get_string ( RootNode, "tech_id" );
       if ( strcmp ( old_tech_id, tech_id ) )                                       /* Si modification de tech_id -> recompil */
        { DB_Write ( domain, "UPDATE dls SET `sourcecode` = REPLACE(`sourcecode`, '%s:', '%s:')", old_tech_id, tech_id );
          /*Partage->com_dls.Thread_reload_with_recompil = TRUE;                             /* Relance DLS avec recompil */
        }
       /*else Partage->com_dls.Thread_reload = TRUE;          /* Relance DLS sans recompil si les tech_id sont identiques */
     }
    else
     { DB_Read ( domain, RootNode, NULL, "SELECT access_level FROM syns WHERE syn_id='%d'", syn_id );

       if ( !Json_has_member ( RootNode, "access_level" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "SYN unknown", RootNode ); goto end; }
       gint syn_access_level = Json_get_int ( RootNode, "access_level" );
       if ( user_access_level < syn_access_level )
        { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied", RootNode ); goto end; }

       retour = DB_Write ( domain, "INSERT INTO dls SET syn_id='%d', tech_id='%s', shortname='%s', name='%s'",
                                   syn_id, tech_id, shortname, name );
       /* Partage->com_dls.Thread_reload = TRUE;                                                              /* Relance DLS */
     }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    else Http_Send_json_response ( msg, SOUP_STATUS_OK, "DLS changed", RootNode );

end:
    g_free(tech_id);
    g_free(name);
    g_free(shortname);

/*    AGENT_send_to_agent ( domain, Json_get_string( request, "agent_uuid" ), "THREAD_START", request );               /* Start */
  }
/******************************************************************************************************************************/
/* DLS_DELETE_request: Supprime un module DLS                                                                                 */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))   return;

    gchar *tech_id   = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );

    gboolean retour = DB_Write ( domain, "DELETE FROM dls INNER JOIN syns USING(`syn_id`) "
                                         "WHERE tech_id='%s' AND syns.access_level <= %d",
                                         tech_id, user_access_level );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S deleted", NULL );
  }
/******************************************************************************************************************************/
/* DLS_DEBUG_request_post: Appelé depuis libsoup pour modifier la notion de debug DLS                                         */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_DEBUG_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "debug" ))   return;

    gchar *tech_id   = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gboolean debug = Json_get_bool ( request, "debug" );

    gboolean retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                         "SET debug=%d WHERE dls.tech_id='%s'AND syns.access_level <= %d",
                                         debug, tech_id, user_access_level );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    AGENT_send_to_agent ( domain, NULL, "DLS_SET", request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S debug OK", NULL );
  }
/******************************************************************************************************************************/
/* DLS_ENABLE_request_post: Appelé depuis libsoup pour activer ou desactiver un module D.L.S                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable" ))   return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gboolean enable = Json_get_bool ( request, "debug" );

    gboolean retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                         "SET enable=%d WHERE dls.tech_id='%s'AND syns.access_level <= %d",
                                         enable, tech_id, user_access_level );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    AGENT_send_to_agent ( domain, NULL, "DLS_SET", request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S enable OK", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
