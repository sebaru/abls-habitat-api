/******************************************************************************************************************************/
/* dls.c                      Gestion des dls dans l'API HTTP WebService                                                      */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls.c
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
 #include <sys/sysinfo.h>
 #include <sys/prctl.h>
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */
 struct HTTP_COMPIL_REQUEST
  { struct DOMAIN *domain;
    JsonNode *token;
    gint dls_package_id;
  };

 struct DLS_COMPIL_THREAD_INFO
  { struct DOMAIN *domain;
    JsonNode *pluginNode;
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
                                "d.compil_status, d.compil_date, d.compil_time, d.compil_user, d.warning_count, d.error_count, "
                                "d.nbr_compil, d.nbr_ligne, ps.page as ppage, s.page as page "
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
                                "d.nbr_compil, d.nbr_ligne, d.compil_date, ps.page as ppage, s.page as page, "
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
/* DLS_RENAME_request_post: Appelé depuis libsoup pour renommer un tech_id                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_RENAME_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;

    if (Http_fail_if_has_not ( domain, path, msg, request, "old_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "new_tech_id" )) return;

    if (!strcasecmp ( Json_get_string ( request, "old_tech_id" ), "SYS" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "SYS can not be renamed", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *old_tech_id_safe = Normaliser_chaine ( Json_get_string ( request, "old_tech_id" ) );
    gchar *new_tech_id_safe = Normaliser_chaine ( Json_get_string ( request, "new_tech_id" ) );
    if (!(old_tech_id_safe && new_tech_id_safe))
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize error", NULL );
       goto end;
     }

    DB_Read ( domain, RootNode, NULL, "SELECT tech_id FROM dls WHERE tech_id='%s'", new_tech_id_safe );
    if ( Json_has_member ( RootNode, "tech_id" ) )
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Target tech_id already exists", NULL ); goto end; }

    DB_Write ( domain, "UPDATE dls SET `sourcecode` = REPLACE(`sourcecode`, '%s:', '%s:')", old_tech_id_safe, new_tech_id_safe );
    DB_Write ( domain, "UPDATE dls SET `tech_id` = '%s' WHERE `tech_id` = '%s'", new_tech_id_safe, old_tech_id_safe );
    DB_Write ( domain, "UPDATE mappings SET `tech_id` = '%s' WHERE `tech_id` = '%s'", new_tech_id_safe, old_tech_id_safe );
    DB_Write ( domain, "UPDATE tableau_map SET `tech_id` = '%s' WHERE `tech_id` = '%s'", new_tech_id_safe, old_tech_id_safe );
    DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete='UPDATE histo_bit SET `tech_id` = \"%s\" WHERE `tech_id` = \"%s\"'",
               new_tech_id_safe, old_tech_id_safe );
    MQTT_Send_to_domain ( domain, "master", "REMAP", NULL );
    DLS_COMPIL_ALL_request_post ( domain, token, path, msg, request );                  /* Positionne Http_Send_json_response */
end:
    json_node_unref ( RootNode );
    if (old_tech_id_safe) g_free(old_tech_id_safe);
    if (new_tech_id_safe) g_free(new_tech_id_safe);
  }
/******************************************************************************************************************************/
/* DLS_RENAME_BIT_request_post: Appelé depuis libsoup pour renommer un bit interne                                            */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_RENAME_BIT_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id"  )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "old_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "new_acronyme" )) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id_safe  = Normaliser_chaine ( Json_get_string ( request, "tech_id"  ) );
    gchar *old_acronyme_safe = Normaliser_chaine ( Json_get_string ( request, "old_acronyme" ) );
    gchar *new_acronyme_safe = Normaliser_chaine ( Json_get_string ( request, "new_acronyme" ) );
    if (!(tech_id_safe && old_acronyme_safe && new_acronyme_safe))
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize error", NULL );
       goto end;
     }

    DB_Write ( domain, "UPDATE dls SET `sourcecode` = REPLACE(`sourcecode`, '%s:%s', '%s:%s')",
               tech_id_safe, old_acronyme_safe, tech_id_safe, new_acronyme_safe );
    DB_Write ( domain, "UPDATE dls SET `sourcecode` = REPLACE(`sourcecode`, '%s', '%s') WHERE tech_id='%s'",
               old_acronyme_safe, new_acronyme_safe, tech_id_safe );
    DB_Write ( domain, "UPDATE mappings SET `acronyme` ='%s' WHERE `tech_id` = '%s' AND `acronyme` = '%s'",
               new_acronyme_safe, tech_id_safe, old_acronyme_safe );
    DB_Write ( domain, "UPDATE tableau_map SET `acronyme` ='%s' WHERE `tech_id` = '%s' AND `acronyme` = '%s'",
               new_acronyme_safe, tech_id_safe, old_acronyme_safe );
    DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete='UPDATE histo_bit SET `acronyme` = \"%s\" "
                       "WHERE `tech_id` = \"%s\" AND `acronyme` = \"%s\"'",
               new_acronyme_safe, tech_id_safe, old_acronyme_safe );
    MQTT_Send_to_domain ( domain, "master", "REMAP", NULL );
    DLS_COMPIL_ALL_request_post ( domain, token, path, msg, request );                  /* Positionne Http_Send_json_response */
end:
    json_node_unref ( RootNode );
    if (tech_id_safe)      g_free(tech_id_safe);
    if (old_acronyme_safe) g_free(old_acronyme_safe);
    if (new_acronyme_safe) g_free(new_acronyme_safe);
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
    if (!(tech_id && name && shortname && package)) goto end;

    gint   syn_id    = Json_get_int ( request, "syn_id" );

    if (Json_has_member ( request, "dls_id" ) )
     { gint dls_id = Json_get_int ( request, "dls_id" );
       DB_Read ( domain, RootNode, NULL, "SELECT tech_id FROM dls WHERE dls_id='%d'", dls_id );

       if ( !Json_has_member ( RootNode, "tech_id" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "DLS unknown", RootNode ); goto end; }

       retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                   "SET syn_id='%d', shortname='%s', name='%s', package='%s' WHERE dls_id='%d' "
                                   "AND syns.access_level <= %d",
                                    syn_id, shortname, name, package, dls_id, user_access_level );

       if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); goto end; }
       DLS_COMPIL_request_post ( domain, token, path, msg, request );
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
    if (tech_id)   g_free(tech_id);
    if (name)      g_free(name);
    if (shortname) g_free(shortname);
    if (package)   g_free(package);
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
/* DLS_RUN_request_get: Renvoie les internals du dls en parametre                                                             */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_RUN_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { gchar *table = NULL;
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, url_param, "classe" ))  return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *classe = Json_get_string ( url_param, "classe" );                            /* Récupération de la classe demandée */
         if ( ! strcasecmp ( classe, "DI" ) )       table = "mnemos_DI";
    else if ( ! strcasecmp ( classe, "AI" ) )       table = "mnemos_AI";
    else if ( ! strcasecmp ( classe, "DO" ) )       table = "mnemos_DO";
    else if ( ! strcasecmp ( classe, "AO" ) )       table = "mnemos_AO";
    else if ( ! strcasecmp ( classe, "CI" ) )       table = "mnemos_CI";
    else if ( ! strcasecmp ( classe, "CH" ) )       table = "mnemos_CH";
    else if ( ! strcasecmp ( classe, "MONO" ) )     table = "mnemos_MONO";
    else if ( ! strcasecmp ( classe, "BI" ) )       table = "mnemos_BI";
    else if ( ! strcasecmp ( classe, "REGISTRE" ) ) table = "mnemos_REGISTRE";
    else if ( ! strcasecmp ( classe, "VISUEL" ) )   table = "mnemos_VISUEL";
    else if ( ! strcasecmp ( classe, "MSG" ) )      table = "msgs";
    else { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Wrong Class", RootNode ); return; }

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( url_param, "tech_id" ) );         /* Formatage correct des chaines */
    if (!tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, classe,
                                "SELECT m.*, map.thread_tech_id, map.thread_acronyme FROM %s AS m "
                                "INNER JOIN dls AS d USING(tech_id) "
                                "INNER JOIN syns AS s USING(syn_id) "
                                "LEFT JOIN mappings AS map ON (map.tech_id=m.tech_id AND map.acronyme=m.acronyme) "
                                "WHERE s.access_level<='%d' AND m.tech_id='%s' "
                                "ORDER BY acronyme",
                                 table, user_access_level, tech_id );
    g_free(tech_id);

    Json_node_add_bool ( url_param, "debug", TRUE );
    MQTT_Send_to_domain ( domain, "master", "DLS_SET", url_param );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Internals given", RootNode );
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
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable" ))  return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gboolean enable = Json_get_bool ( request, "enable" );

    gboolean retour = DB_Write ( domain, "UPDATE dls INNER JOIN syns USING(`syn_id`) "
                                         "SET enable=%d WHERE dls.tech_id='%s'AND syns.access_level <= %d",
                                         enable, tech_id, user_access_level );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    MQTT_Send_to_domain ( domain, "master", "DLS_SET", request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S enable OK", NULL );
  }
/******************************************************************************************************************************/
/* DLS_RESTART_request_post: Appelé depuis libsoup pour restarter un module D.L.S                                             */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_RESTART_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    JsonNode *ToAgentNode = Json_node_create();
    if (ToAgentNode)
     { Json_node_add_string ( ToAgentNode, "tech_id", tech_id );
       MQTT_Send_to_domain ( domain, "master", "DLS_RESTART", ToAgentNode );                    /* Envoi du restart au master */
       json_node_unref( ToAgentNode );
     }
    g_free(tech_id);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S restarted", NULL );
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
    gchar *package_src = Json_get_string ( request, "package" );
    if (!package_src) package_src = "";
    gchar *package     = Normaliser_chaine ( package_src );

    if (!tech_id || !description || !package)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Normalize Error", (tech_id ? tech_id : "unknown") );
       goto end;
     }

    gboolean retour = DB_Write ( domain,
                                 "INSERT INTO dls SET enable=0,"
                                 "tech_id=UPPER('%s'), shortname='Add a shortname', name='%s', package='%s',"
                                 "syn_id=1 "
                                 "ON DUPLICATE KEY UPDATE tech_id=VALUES(tech_id)",
                                 tech_id, description, package );
    Info_new( __func__, LOG_NOTICE, domain, "'%s': D.L.S plugin created ('%s')", tech_id, description );

end:
    if (tech_id)     g_free(tech_id);
    if (description) g_free(description);
    if (package)     g_free(package);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "D.L.S created", NULL );
  }
/******************************************************************************************************************************/
/* Dls_save_plugin: Sauvegarde les buffers de la traduction du plugin                                                         */
/* Entrée: le domaine d'application, l'utilisateur générateur de l'évènement et le PluginNode                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_save_plugin ( struct DOMAIN *domain, JsonNode *token, JsonNode *PluginNode )
  { gchar *tech_id = Json_get_string ( PluginNode, "tech_id" );
    if (!tech_id) return;

    if (Json_has_member ( PluginNode, "sourcecode" ))
     { gchar *sourcecode = Normaliser_chaine ( Json_get_string ( PluginNode, "sourcecode" ) );
       DB_Write ( domain, "UPDATE dls SET sourcecode='%s' WHERE tech_id='%s'", (sourcecode ? sourcecode : "Memory error"), tech_id );
       g_free(sourcecode);
     }

    gchar *errorlog = NULL, *codec = NULL;
    if (Json_has_member ( PluginNode, "errorlog" ))   errorlog   = Normaliser_chaine ( Json_get_string ( PluginNode, "errorlog" ) );
    if (Json_has_member ( PluginNode, "codec" ))      codec      = Normaliser_chaine ( Json_get_string ( PluginNode, "codec" ) );

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
               tech_id );
    if (errorlog) g_free(errorlog);
    if (codec)    g_free(codec);
    Info_new( __func__, LOG_INFO, domain, "'%s': New source code saved in database", tech_id );
  }
/******************************************************************************************************************************/
/* Dls_Send_compil_to_master: Demande au Master de recompilé et updater un plugin                                             */
/* Entrée: le domain et le tech_id                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Send_compil_to_master ( struct DOMAIN *domain, gchar *tech_id )
  { if (!tech_id) return;
    gchar *tech_id_safe = Normaliser_chaine ( tech_id );
    if (!tech_id_safe)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Error normalize tech_id. Dropping.", tech_id ); return; }

    JsonNode *ToAgentNode = Json_node_create();
    if (ToAgentNode)
     { Json_node_add_string ( ToAgentNode, "tech_id", tech_id_safe );
       MQTT_Send_to_domain ( domain, "master", "DLS_RELOAD", ToAgentNode );        /* Envoi de la demande de reload au master */
       json_node_unref( ToAgentNode );
       DB_Write ( domain, "UPDATE histo_msgs SET date_fin=NOW() WHERE tech_id='%s' AND date_fin IS NULL", tech_id );/* RAZ FdL */
     }
    g_free(tech_id_safe);
  }
/******************************************************************************************************************************/
/* Dls_Compil_one: Traduction d'un module                                                                                     */
 /* Entrée: les elements de la traduction                                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Compil_one ( struct DOMAIN *domain, JsonNode *token, JsonNode *plugin )
  { gchar *tech_id   = Json_get_string ( plugin, "tech_id" );

    if (Dls_Apply_package ( domain, plugin ) == FALSE)
     { Info_new( __func__, LOG_ERR, domain, "'%s': Error when applying package / sourcode. Dropping.", tech_id ); return; }

    Dls_Apply_params ( domain, plugin );
    Dls_traduire_plugin ( domain, plugin );                /* Le résultat de la traduction est dans le pluginNode directement */
    Dls_save_plugin ( domain, token, plugin );                              /* On sauve le codec, l'errorlog et le sourcecode */

    if (Json_get_bool ( plugin, "compil_status" ) && Json_get_int ( plugin, "error_count" ) == 0 )
     { Info_new( __func__, LOG_NOTICE, domain, "'%s': Parsing OK, sending Compil Order to Master Agent", tech_id );
       Dls_Send_compil_to_master ( domain, tech_id );
     } else Info_new( __func__, LOG_ERR, domain, "'%s': Parsing Failed.", tech_id );
  }
/******************************************************************************************************************************/
/* DLS_COMPIL_one_thread: Traduction d'un module dans un _thread spécifique                                                   */
/* Entrée: les elements de l'array                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void *DLS_COMPIL_one_by_thread ( struct DLS_COMPIL_THREAD_INFO *info )
  { struct DOMAIN *domain = info->domain;
    JsonNode *plugin      = info->pluginNode;
    JsonNode *token       = info->token;
    g_free(info);
    gchar *tech_id   = Json_get_string ( plugin, "tech_id" );
    gchar chaine[128];
    g_snprintf( chaine, sizeof(chaine), "W-CC-%s", tech_id );                                      /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );

    Dls_Compil_one ( domain, token, plugin );
    pthread_mutex_lock ( &Global.Nbr_compil_mutex );            /* Increment le nombre de thread de compilation en parallelle */
    Global.Nbr_compil--;
    pthread_mutex_unlock ( &Global.Nbr_compil_mutex );
    return(NULL);
  }
/******************************************************************************************************************************/
/* DLS_COMPIL_ALL_request_post: Traduction de tous les DLS du domain vers le langage C                                        */
/* Entrée: les elements libsoup                                                                                               */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 static void DLS_COMPIL_ALL_CB ( struct HTTP_COMPIL_REQUEST *http_request )
  { pthread_t TID;
    struct DOMAIN *domain = http_request->domain;
    JsonNode *token       = http_request->token;
    gint dls_package_id   = http_request->dls_package_id;
    g_free(http_request);

    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *pluginsNode = Json_node_create();
    if (!pluginsNode)
     { Info_new( __func__, LOG_ERR, domain, "Memory Error for pluginsNode. Compil_all aborted." );
       goto end;
     }

    gboolean retour = DB_Read ( domain, pluginsNode, "plugins",
                                "SELECT dls_id, tech_id, access_level FROM dls AS d "
                                "INNER JOIN syns AS s USING(`syn_id`)  "
                                "LEFT JOIN dls_packages AS p ON d.package = p.name "
                                "WHERE s.access_level <= %d "
                                "AND (%d = 0 OR p.dls_package_id = %d) "
                                "ORDER BY d.tech_id", user_access_level, dls_package_id, dls_package_id );
    if (!retour)
     { Info_new( __func__, LOG_ERR, domain, "Database Error searching for plugins. Compil_all aborted." );
       goto end;
     }

    pthread_attr_t attr;                                                      /* Attribut de thread pour parametrer le module */
    if ( pthread_attr_init(&attr) )                                                 /* Initialisation des attributs du thread */
     { Info_new( __func__, LOG_ERR, domain, "pthread_attr_init failed." );
       goto end;
     }

    if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) )                       /* On le laisse joinable au boot */
     { Info_new( __func__, LOG_ERR, domain, "pthread_setdetachstate failed." );
       goto end;
     }

    gint nbr_plugin = Json_get_int ( pluginsNode, "nbr_plugins" );
    Info_new( __func__, LOG_NOTICE, domain, "Start compiling %03d plugins (with dls_package_id=%d).", nbr_plugin, dls_package_id );
    gint compil_top = Global.Top;

    GList *PluginsArray = json_array_get_elements ( Json_get_array ( pluginsNode, "plugins" ) );
    GList *plugins = PluginsArray;
    while(plugins)
     { JsonNode *plugin = plugins->data;
       gchar *tech_id = Json_get_string ( plugin, "tech_id" );
       while (Global.Nbr_compil >= get_nprocs()) sched_yield();                    /* Attente libération d'un slot processeur */
       struct DLS_COMPIL_THREAD_INFO *info = g_try_malloc0 ( sizeof ( struct DLS_COMPIL_THREAD_INFO ) );
       if (info)
        { info->domain     = domain;
          info->pluginNode = plugin;
          info->token      = token;
          pthread_mutex_lock ( &Global.Nbr_compil_mutex );      /* Increment le nombre de thread de compilation en parallelle */
          Global.Nbr_compil++;                      /* fait +1 avant de lancer le pthread pour etre sur que le thread demarre */
          if ( pthread_create( &TID, &attr, (void *)DLS_COMPIL_one_by_thread, info ) )
           { Info_new( __func__, LOG_ERR, domain, "'%s': pthread_create failed.", tech_id );
             Global.Nbr_compil--; /* Démarrage failed */
             g_free(info);
           }
          pthread_mutex_unlock ( &Global.Nbr_compil_mutex );
        }
       else { Info_new( __func__, LOG_ERR, domain, "'%s': memory error.", tech_id ); }
       plugins = g_list_next(plugins);
     }
    g_list_free(PluginsArray);

    while(Global.Nbr_compil) sched_yield();                                             /* Attente de toutes les compilations */
    Info_new( __func__, LOG_INFO, domain, "Compil all %03d plugins in %06.1fs", nbr_plugin, (Global.Top - compil_top)/10.0 );
end:
    if (pluginsNode) json_node_unref ( pluginsNode );
    json_node_unref ( token );
  }
/******************************************************************************************************************************/
/* DLS_COMPIL_ALL_request_post: Traduction de tous les DLS du domain vers le langage C                                        */
/* Entrée: les elements libsoup                                                                                               */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 void DLS_COMPIL_ALL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    struct HTTP_COMPIL_REQUEST *http_request = g_try_malloc0( sizeof(struct HTTP_COMPIL_REQUEST) );
    if (!http_request) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not enough memory", NULL ); return; }
    json_node_ref ( token );                                   /* Sera utilisé par le thread, il faut donc ref+1 la structure */
    http_request->token          = token;
    http_request->domain         = domain;
    http_request->dls_package_id = Json_get_int ( request, "dls_package_id" );

    pthread_t TID;
    pthread_create( &TID, NULL, (void *)DLS_COMPIL_ALL_CB, http_request );
    pthread_detach( TID );                                           /* On le detache pour qu'il puisse se terminer tout seul */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Compiling All D.L.S", NULL );
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
                       "SELECT dls_id, tech_id, access_level FROM dls "
                       "INNER JOIN syns USING(`syn_id`) "
                       "WHERE tech_id='%s' AND syns.access_level <= %d", tech_id, user_access_level );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); goto end; }
    if (!Json_has_member ( PluginNode, "dls_id" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Plugin not found", NULL ); goto end; }
    if ( user_access_level < Json_get_int ( PluginNode, "access_level" ) )
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied", NULL ); goto end; }


/********************************************* Compilation du plugin **********************************************************/
    tech_id = Json_get_string ( PluginNode, "tech_id" );
    if (Json_has_member ( request, "sourcecode" ))                                       /* Recopie du sourcecode s'il existe */
     { Json_node_add_string ( PluginNode, "sourcecode", Json_get_string ( request, "sourcecode" ) ); }
    Dls_Compil_one ( domain, token, PluginNode );                                       /* Compilation du plugin en parametre */

/************************************************** S'agit-il d'un package ? **************************************************/
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
                               "SELECT tech_id, shortname, name, codec, enable FROM dls WHERE tech_id='%s'", tech_id );
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
