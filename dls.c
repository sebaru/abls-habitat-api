/******************************************************************************************************************************/
/* dls.c                      Gestion des dls dans l'API HTTP WebService                                                      */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls.c
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
 struct HTTP_COMPIL_REQUEST
  { struct DOMAIN *domain;
    JsonNode *token;
  };

/******************************************************************************************************************************/
/* DLS_LIST_request_get: Liste les modules DLS                                                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "dls",
                                "SELECT d.dls_id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.enable, "
                                "d.compil_status, d.compil_date, d.compil_user, d.warning_count, d.error_count, "
                                "d.nbr_compil, d.nbr_ligne, d.debug, ps.page as ppage, s.page as page "
                                "FROM dls AS d "
                                "INNER JOIN syns as s  ON d.syn_id = s.syn_id "
                                "INNER JOIN syns as ps ON s.parent_id = ps.syn_id "
                                "WHERE s.access_level<='%d' ORDER BY d.tech_id", user_access_level );

    retour &= DB_Read ( domain, RootNode, NULL,
                        "SELECT agent_hostname AS master_hostname FROM agents WHERE is_master=1" );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of D.L.S", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_SOURCE_request_get: Renvoie la code source DLS d'un module                                                             */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_SOURCE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id" ))   return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( url_param, "tech_id" ) );         /* Formatage correct des chaines */
    if (!tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT d.dls_id, d.tech_id, d.package, d.syn_id, d.name, d.shortname, d.enable, d.compil_status, "
                                "d.error_count, d.warning_count, d.compil_time, "
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
 void DLS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour = FALSE;
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "shortname" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "package" ))   return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id   = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *name      = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *shortname = Normaliser_chaine ( Json_get_string( request, "shortname" ) );
    gchar *package   = Normaliser_chaine ( Json_get_string( request, "package" ) );
    gint   syn_id    = Json_get_int ( request, "syn_id" );

    if (Json_has_member ( request, "dls_id" ) )
     { gint dls_id = Json_get_int ( request, "dls_id" );
       DB_Read ( domain, RootNode, NULL, "SELECT tech_id FROM dls WHERE dls_id='%d'", dls_id );

       if ( !Json_has_member ( RootNode, "tech_id" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "DLS unknown", RootNode ); goto end; }

       retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                   "SET syn_id='%d', tech_id='%s', shortname='%s', name='%s', package='%s' WHERE dls_id='%d' "
                                   "AND syns.access_level <= %d",
                                    syn_id, tech_id, shortname, name, package, dls_id, user_access_level );

       if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); goto end; }

       gchar *old_tech_id = Json_get_string ( RootNode, "tech_id" );
       if ( strcmp ( old_tech_id, tech_id ) )                                       /* Si modification de tech_id -> recompil */
        { DB_Write ( domain, "UPDATE dls SET `sourcecode` = REPLACE(`sourcecode`, '%s:', '%s:')", old_tech_id, tech_id );
          DB_Write ( domain, "UPDATE mappings SET `tech_id` = '%s' WHERE `tech_id` = '%s'", tech_id, old_tech_id );
          DB_Write ( domain, "UPDATE tableau_map SET `tech_id` = '%s' WHERE `tech_id` = '%s'", tech_id, old_tech_id );
          JsonNode *TableNode = Json_node_create();
          DB_Arch_Read ( domain, TableNode, "names",
                         "SELECT table_name FROM information_schema.tables "
                         "WHERE table_name LIKE 'histo_bit_%s_%%'", old_tech_id );
          GList *Names = json_array_get_elements ( Json_get_array ( TableNode, "names" ) );
          GList *names = Names;
          while(names)
           { gchar *name = Json_get_string ( names->data, "table_name" );
             gchar target_name[256];
             g_snprintf ( target_name, sizeof(target_name), "histo_bit_%s_%s", tech_id, name + strlen(old_tech_id) + strlen("histo_bit__") );
             DB_Arch_Write ( domain, "ALTER TABLE %s RENAME %s", name, target_name );
             names = g_list_next(names);
           }
          g_list_free(Names);
          json_node_unref ( TableNode );
          AGENT_send_to_agent ( domain, NULL, "REMAP", NULL );
          DLS_COMPIL_ALL_request_post ( domain, token, path, msg, request );
        }
       else DLS_COMPIL_request_post ( domain, token, path, msg, request );
     }
    else                                                                                             /* Ajout d'un module DLS */
     { DB_Read ( domain, RootNode, NULL, "SELECT access_level FROM syns WHERE syn_id='%d'", syn_id ); /* Droit sur ce level ? */

       if ( !Json_has_member ( RootNode, "access_level" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "SYN unknown", RootNode ); goto end; }
       gint syn_access_level = Json_get_int ( RootNode, "access_level" );
       if ( user_access_level < syn_access_level )
        { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied", RootNode ); goto end; }

       retour = DB_Write ( domain, "INSERT INTO dls SET syn_id='%d', tech_id='%s', shortname='%s', name='%s', package='%s'",
                                   syn_id, tech_id, shortname, name, package );                 /* Création, sans compilation */
     }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    else Http_Send_json_response ( msg, SOUP_STATUS_OK, "DLS changed", RootNode );

end:
    g_free(tech_id);
    g_free(name);
    g_free(shortname);
    g_free(package);
  }
/******************************************************************************************************************************/
/* DLS_DELETE_request: Supprime un module DLS                                                                                 */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))   return;

    gchar *tech_id   = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );

    gboolean retour = DB_Write ( domain, "DELETE dls FROM dls INNER JOIN syns USING(`syn_id`) "
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
 void DLS_DEBUG_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
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
 void DLS_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable" ))   return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gboolean enable = Json_get_bool ( request, "enable" );

    gboolean retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                         "SET enable=%d WHERE dls.tech_id='%s'AND syns.access_level <= %d",
                                         enable, tech_id, user_access_level );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    AGENT_send_to_agent ( domain, NULL, "DLS_SET", request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S enable OK", NULL );
  }
/******************************************************************************************************************************/
/* DLS_PARAMS_request_get: Renvoie les paramètres d'un code DLS                                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PARAMS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id" ))   return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( url_param, "tech_id" ) );         /* Formatage correct des chaines */
    if (!tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "params",
                                "SELECT p.dls_param_id, p.acronyme, p.libelle FROM dls_params AS p "
                                "INNER JOIN dls AS d USING(tech_id) "
                                "INNER JOIN syns AS s USING(syn_id) "
                                "WHERE s.access_level<='%d' AND tech_id='%s'"
                                "ORDER BY acronyme",
                                 user_access_level, tech_id );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Parameters given", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_PARAMS_SET_request_post: Positionne les paramètres d'un code DLS                                                       */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PARAMS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "dls_param_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;

    gchar *libelle = Normaliser_chaine ( Json_get_string ( request, "libelle" ) );         /* Formatage correct des chaines */
    if (!libelle) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", NULL ); return; }

    gint dls_param_id = Json_get_int( request, "dls_param_id" );
    gboolean retour = DB_Write ( domain, "UPDATE dls_params AS p INNER JOIN dls USING (`tech_id`) INNER JOIN syns USING(`syn_id`) "
                                         "SET p.libelle='%s' WHERE p.dls_param_id=%d AND syns.access_level <= %d",
                                         libelle, dls_param_id, user_access_level );
    g_free(libelle);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    DLS_COMPIL_request_post ( domain, token, path, msg, request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Parameter setted", NULL );
  }
/******************************************************************************************************************************/
/* RUN_DLS_CREATE_request_post: Appelé depuis libsoup pour creer un plugin D.L.S                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void RUN_DLS_CREATE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" )) return;

    gchar *tech_id     = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );       /* Formatage correct des chaines */
    gchar *description = Normaliser_chaine ( Json_get_string ( request, "description" ) );   /* Formatage correct des chaines */

    if (!tech_id || !description)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Normalize Error", (tech_id ? tech_id : "unknown") );
       goto end;
     }

    gboolean retour = DB_Write ( domain,
                                 "INSERT INTO dls SET enable=0,"
                                 "tech_id=UPPER('%s'),shortname='Add a shortname',name='%s',package='custom',"
                                 "syn_id=1 "
                                 "ON DUPLICATE KEY UPDATE tech_id=VALUES(tech_id)", tech_id, description );
    Info_new( __func__, LOG_NOTICE, domain, "'%s': D.L.S plugin created ('%s')", tech_id, description );

end:
    if (tech_id)     g_free(tech_id);
    if (description) g_free(description);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S created", NULL );
  }
/******************************************************************************************************************************/
/* Dls_save_plugin: Sauvegarde les buffers de la traduction du plugin                                                         */
/* Entrée: le domaine d'application, l'utilisateur générateur de l'évènement et le PluginNode                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_save_plugin ( struct DOMAIN *domain, JsonNode *token, JsonNode *PluginNode )
  { gchar *errorlog = NULL, *codec = NULL;

    if (Json_has_member ( PluginNode, "errorlog" )) errorlog = Normaliser_chaine ( Json_get_string ( PluginNode, "errorlog" ) );
    if (Json_has_member ( PluginNode, "codec" ))    codec    = Normaliser_chaine ( Json_get_string ( PluginNode, "codec" ) );

    DB_Write ( domain, "UPDATE dls SET compil_status='%d', compil_date = NOW(), compil_time = '%d', compil_user='%s', "
                       "nbr_ligne = LENGTH(`sourcecode`)-LENGTH(REPLACE(`sourcecode`,'\n',''))+1, codec='%s', errorlog='%s', "
                       "error_count='%d', warning_count='%d' "
                       "WHERE tech_id='%s'",
               Json_get_bool ( PluginNode, "compil_status" ),
               Json_get_int  ( PluginNode, "compil_time" ),
               (token ? Json_get_string ( token, "preferred_username" ) : "système"),
               (codec ? codec : "No CodeC error"),
               (errorlog ? errorlog : "No ErrorLog error"),
               Json_get_int ( PluginNode, "error_count" ),
               Json_get_int ( PluginNode, "warning_count" ),
               Json_get_string ( PluginNode, "tech_id" ) );
    g_free(errorlog);
    g_free(codec);
  }
/******************************************************************************************************************************/
/* DLS_COMPIL_ALL_request_post: Traduction de tous les DLS du domain vers le langage C                                        */
/* Entrée: les elements libsoup                                                                                               */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 static void DLS_COMPIL_ALL_CB ( struct HTTP_COMPIL_REQUEST *http_request )
  { struct DOMAIN *domain = http_request->domain;
    JsonNode *token       = http_request->token;
    g_free(http_request);

    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *pluginsNode = Json_node_create();
    if (!pluginsNode) { return; }

    gboolean retour = DB_Read ( domain, pluginsNode, "plugins",
                                "SELECT dls_id, tech_id, access_level, sourcecode, debug FROM dls "
                                "INNER JOIN syns USING(`syn_id`) "
                                "WHERE syns.access_level <= %d ORDER BY tech_id", user_access_level );
    if (!retour)
     { json_node_unref ( pluginsNode );
       return;
     }
    gint nbr_plugin = Json_get_int ( pluginsNode, "nbr_plugins" );

    gint compil_time = 0;
    JsonNode *ToAgentNode = Json_node_create();
    if (!ToAgentNode)
     { json_node_unref ( pluginsNode );
       return;
     }
    Json_node_add_bool ( ToAgentNode, "dls_reset", TRUE );                           /* On demande le reset des bits internes */

    GList *PluginsArray = json_array_get_elements ( Json_get_array ( pluginsNode, "plugins" ) );
    GList *plugins = PluginsArray;
    while(plugins)
     { JsonNode *plugin = plugins->data;
       gchar *tech_id = Json_get_string ( plugin, "tech_id" );
       Dls_traduire_plugin ( domain, plugin );
       compil_time += Json_get_int ( plugin, "compil_time" );
       Dls_save_plugin ( domain, token, plugin );
       if (Json_get_bool ( plugin, "compil_status" ) && Json_get_int ( plugin, "error_count" ) == 0 )
        { Info_new( __func__, LOG_NOTICE, domain, "'%s': Parsing OK, sending Compil Order to Master Agent", tech_id );
          Json_node_add_string ( ToAgentNode, "tech_id", Json_get_string ( plugin, "tech_id" ) );
          AGENT_send_to_agent ( domain, NULL, "DLS_COMPIL", ToAgentNode );                           /* Envoi du code C aux agents */
        } else Info_new( __func__, LOG_ERR, domain, "'%s': Parsing Failed. Compil aborted", tech_id );
       plugins = g_list_next(plugins);
     }
    g_list_free(PluginsArray);
    json_node_unref ( pluginsNode );
    json_node_unref ( ToAgentNode );
    json_node_unref ( token );
    Info_new( __func__, LOG_INFO, domain, "Compil all %03d plugins in %06.1fs", nbr_plugin, compil_time/10.0 );
  }
/******************************************************************************************************************************/
/* DLS_COMPIL_ALL_request_post: Traduction de tous les DLS du domain vers le langage C                                        */
/* Entrée: les elements libsoup                                                                                               */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 void DLS_COMPIL_ALL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    struct HTTP_COMPIL_REQUEST *http_request = g_try_malloc( sizeof(struct HTTP_COMPIL_REQUEST) );
    if (!http_request) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not enough memory", NULL ); return; }
    http_request->domain  = domain;
    json_node_ref ( token );                                   /* Sera utilisé par le thread, il faut donc ref+1 la structure */
    http_request->token   = token;

    pthread_t TID;
    pthread_create( &TID, NULL, (void *)DLS_COMPIL_ALL_CB, http_request );
    pthread_detach( TID );                                           /* On le detache pour qu'il puisse se terminer tout seul */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Compiling All D.L.S", NULL );
  }
/******************************************************************************************************************************/
/* Dls_update_one_parameter: Met a jour un parametre dans le sourcecode fourni                                                */
/* Entrée: le sourcecode, le parametre, sa valeur                                                                             */
/* Sortie: le sourcecode mis à jour                                                                                           */
/******************************************************************************************************************************/
 static void Dls_update_one_parameter ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gchar *acronyme = Json_get_string ( element, "acronyme" );
    gchar *libelle  = Json_get_string ( element, "libelle" );
    GString *sourcecode = user_data;
    gchar find [256];
    g_snprintf( find, sizeof(find), "$%s", acronyme );
    g_string_replace ( sourcecode, find, libelle, 0 );
  }
/******************************************************************************************************************************/
/* DLS_COMPIL_request_post: Traduction du fichier en paramètre du langage DLS vers le langage C                               */
/* Entrée: les elements libsoup                                                                                               */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 void DLS_COMPIL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *PluginNode = Json_node_create();
    if (!PluginNode) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );

    retour = DB_Read ( domain, PluginNode, NULL,
                       "SELECT dls_id, tech_id, access_level, sourcecode, debug, package FROM dls "
                       "INNER JOIN syns USING(`syn_id`) "
                       "WHERE tech_id='%s' AND syns.access_level <= %d", tech_id, user_access_level );
    retour&= DB_Read ( domain, PluginNode, "params",
                       "SELECT acronyme, libelle FROM dls_params "
                       "WHERE tech_id='%s'", tech_id );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); goto end; }
    if (!Json_has_member ( PluginNode, "dls_id" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Plugin not found", NULL ); goto end; }
    if ( user_access_level < Json_get_int ( PluginNode, "access_level" ) )
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied", NULL ); goto end; }

    tech_id = Json_get_string ( PluginNode, "tech_id" );

/************************************************** S'agit-il d'un package ? **************************************************/
    gchar *package = Json_get_string ( PluginNode, "package" );
    if ( strcasecmp ( package, "custom" ) && strlen(package) )                                      /* S'agit-il d'un package */
     { gchar package_query[256];
       GError *error = NULL;
       SoupSession *session  = soup_session_new();

       g_snprintf( package_query, sizeof(package_query), "https://static.abls-habitat.fr/package/%s.dls", package );
       SoupMessage *soup_msg = soup_message_new ( "GET", package_query );
       GBytes *response      = soup_session_send_and_read ( session, soup_msg, NULL, &error ); /* SYNC */
       gchar *reason_phrase  = soup_message_get_reason_phrase(soup_msg);
       gint   status_code    = soup_message_get_status(soup_msg);

       if (error)
        { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
          Info_new( __func__, LOG_ERR, domain, "Unable to retrieve Package '%s': error %s", package_query, error->message );
          g_free(uri);
          g_error_free ( error );
        }
       else if (status_code==200)
        { gsize taille;
          gchar *buffer_unsafe = g_bytes_get_data ( response, &taille );
          gchar *buffer_safe   = g_try_malloc0 ( taille + 1 );
          if (taille && buffer_safe)
           { memcpy ( buffer_safe, buffer_unsafe, taille );                                        /* Copy with \0 end of string */
             Json_node_add_string ( PluginNode, "sourcecode", buffer_safe );
             g_free(buffer_safe);
           }
        }
       else Info_new( __func__, LOG_CRIT, domain, "Unable to retrieve Package '%s': %s", package_query, reason_phrase );
       g_object_unref( soup_msg );

/*********************************************** Téléchargement des parametres du package *************************************/
       g_snprintf( package_query, sizeof(package_query),
                   "https://static.abls-habitat.fr/package/%s.params", package );
       soup_msg      = soup_message_new ( "GET", package_query );
       response      = soup_session_send_and_read ( session, soup_msg, NULL, &error ); /* SYNC */
       reason_phrase = soup_message_get_reason_phrase(soup_msg);
       status_code   = soup_message_get_status(soup_msg);

       if (error)
        { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
          Info_new( __func__, LOG_ERR, domain, "Unable to retrieve Package Parameters '%s': error %s", package_query, error->message );
          g_free(uri);
          g_error_free ( error );
        }
       else if (status_code==200) /************************** Update les paramètres *******************************************/
        { gsize taille;
          gchar *buffer_unsafe = g_bytes_get_data ( response, &taille );
          gchar *buffer_safe   = g_try_malloc0 ( taille + 1 );
          if (taille && buffer_safe)
           { memcpy ( buffer_safe, buffer_unsafe, taille );                                     /* Copy with \0 end of string */
             JsonNode *ResponseNode = Json_get_from_string ( buffer_safe );
             g_free(buffer_safe);

             GList *Results = json_array_get_elements ( Json_get_array ( ResponseNode, "params" ) );
             GList *results = Results;
             while(results)
              { JsonNode *element = results->data;
                gchar *acronyme = Json_get_string ( element, "acronyme" );
                gchar *defaut   = Json_get_string ( element, "defaut" );
                DB_Write ( domain, "INSERT IGNORE INTO dls_params SET tech_id='%s', acronyme='%s', libelle='%s' ",
                           tech_id, acronyme, defaut );
                results = g_list_next(results);
              }
             g_list_free(Results);

             GString *sourcecode_string = g_string_new ( Json_get_string ( PluginNode, "sourcecode" ) );
             DB_Read ( domain, ResponseNode, "params_value", "SELECT * FROM dls_params WHERE tech_id='%s'", tech_id );
             Json_node_foreach_array_element ( ResponseNode, "params_value", Dls_update_one_parameter, sourcecode_string );
             gchar *sourcecode_updated = g_string_free_and_steal ( sourcecode_string );
             Json_node_add_string ( PluginNode, "sourcecode", sourcecode_updated );
             g_free(sourcecode_updated);
             Info_new( __func__, LOG_INFO, domain, "'%s': Parameters set", tech_id );
             json_node_unref ( ResponseNode );
           }
        }
       else Info_new( __func__, LOG_CRIT, domain, "Unable to retrieve Package '%s': %s", package_query, reason_phrase );
       g_object_unref( soup_msg );

       soup_session_abort ( session );
       g_object_unref( session );
     }
/************************************************** Non, c'est un module custom ***********************************************/
    else if (Json_has_member ( request, "sourcecode" ))                                              /* new custom sourcecode */
     { gchar *sourcecode = Json_get_string ( request, "sourcecode" );
       Json_node_add_string ( PluginNode, "sourcecode", sourcecode );
     }

    gchar *new_sourcecode = Normaliser_chaine ( Json_get_string ( PluginNode, "sourcecode" ) );
    DB_Write ( domain, "UPDATE dls SET sourcecode='%s' WHERE tech_id='%s'", (new_sourcecode ? new_sourcecode : "Memory error"), tech_id );
    g_free(new_sourcecode);
    Info_new( __func__, LOG_INFO, domain, "'%s': New source code saved", tech_id );

    Dls_traduire_plugin ( domain, PluginNode );            /* Le résultat de la traduction est dans le pluginNode directement */
    Dls_save_plugin ( domain, token, PluginNode );

    JsonNode *RootNode = Http_json_node_create( msg );                                               /* RootNode for response */
    if (!RootNode) goto end;
    gboolean compil_status = Json_get_bool ( PluginNode, "compil_status" );
    gint     compil_time   = Json_get_int  ( PluginNode, "compil_time" );

    Json_node_add_string ( RootNode, "tech_id", tech_id );
    Json_node_add_bool   ( RootNode, "compil_status", compil_status );
    Json_node_add_int    ( RootNode, "compil_time",   compil_time );
    Json_node_add_string ( RootNode, "errorlog",      Json_get_string ( PluginNode, "errorlog" ) );
    Json_node_add_int    ( RootNode, "error_count",   Json_get_int    ( PluginNode, "error_count" ) );
    Json_node_add_int    ( RootNode, "warning_count", Json_get_int    ( PluginNode, "warning_count" ) );

    if (!compil_status)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Compil Failed", RootNode ); goto end; }

    if (Json_get_int ( PluginNode, "error_count" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_OK, "Error found", RootNode ); goto end; }

    Info_new( __func__, LOG_NOTICE, domain, "'%s': Parsing OK (in %06.1fs), sending Compil Order to Master Agent", tech_id, compil_time/10.0 );
    DB_Write ( domain, "UPDATE histo_msgs SET date_fin=NOW() WHERE tech_id='%s' AND date_fin IS NULL", tech_id );  /* RAZ FdL */
    Json_node_add_bool  ( RootNode, "dls_reset", TRUE );                             /* On demande le reset des bits internes */
    AGENT_send_to_agent ( domain, NULL, "DLS_COMPIL", RootNode );                             /* Envoi de la notif aux agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK,
                              ( Json_get_int ( PluginNode, "warning_count" ) ? "Warning found" : "Traduction OK" ),
                              RootNode );
end:
    json_node_unref ( PluginNode );
  }
/******************************************************************************************************************************/
/* RUN_DLS_PLUGINS_request_post: Repond aux requests DLS_PLUGINS depuis les agents                                            */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_DLS_PLUGINS_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  {
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "plugins",
                                "SELECT tech_id, shortname, name FROM dls" );
    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "dls plugins sent", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_DLS_LOAD_request_get: Repond aux requests DLS_LOAD depuis les agents                                                   */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_DLS_LOAD_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param )
  { if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string ( url_param, "tech_id" ) );

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                               "SELECT tech_id, shortname, name, codec, debug, enable FROM dls WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_BI",       "SELECT * FROM mnemos_BI WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_MONO",     "SELECT * FROM mnemos_MONO WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_DI",       "SELECT * FROM mnemos_DI WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_DO",       "SELECT * FROM mnemos_DO WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_AI",       "SELECT * FROM mnemos_AI WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_AO",       "SELECT * FROM mnemos_AO WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_CI",       "SELECT * FROM mnemos_CI WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_CH",       "SELECT * FROM mnemos_CH WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_REGISTRE", "SELECT * FROM mnemos_REGISTRE WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_WATCHDOG", "SELECT * FROM mnemos_WATCHDOG WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_MESSAGE",  "SELECT msgs.*, d.shortname AS dls_shortname FROM msgs "
                                                                     "INNER JOIN dls AS d USING(`tech_id`) WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_TEMPO",    "SELECT * FROM mnemos_TEMPO WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "mnemos_VISUEL",   "SELECT * FROM mnemos_VISUEL WHERE tech_id='%s'", tech_id );
            retour &= DB_Read ( domain, RootNode, "thread_tech_ids", "SELECT DISTINCT(thread_tech_id) FROM mappings "
                                                                     "WHERE tech_id='%s' AND thread_tech_id NOT LIKE '_%%'", tech_id );
    g_free(tech_id);

    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "dls internals sent", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
