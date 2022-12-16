/******************************************************************************************************************************/
/* thread.c                      Gestion des thread dans l'API HTTP WebService                                        */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.c
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
/* THREAD_SEND_request_post: Envoi un requete à un thread                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void THREAD_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tag")) return;


    gboolean retour = AGENT_send_to_agent ( domain, NULL, "THREAD_SEND", request );                     /* Send to all agents */
    if (retour) Http_Send_json_response ( msg, SOUP_STATUS_OK, "Command sent", NULL );
           else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agents are not connected", NULL );
  }
/******************************************************************************************************************************/
/* THREAD_ENABLE_request_post: Envoi une demande d'activation ou desactivation ud Thread                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void THREAD_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable")) return;

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean enable        = Json_get_bool ( request, "enable" );
    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT agent_uuid, thread_tech_id, thread_classe "
                                                        "FROM threads WHERE thread_tech_id='%s'", thread_tech_id );
    g_free(thread_tech_id);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

           thread_tech_id  = Json_get_string( RootNode, "thread_tech_id" );
    gchar *thread_classe   = Json_get_string( RootNode, "thread_classe" );
    gchar *agent_uuid      = Json_get_string( RootNode, "agent_uuid" );

    if (!thread_tech_id || !thread_classe || !agent_uuid)
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Tech_id, Agent or Class not found", RootNode ); return; }

    retour = DB_Write ( domain,"UPDATE %s SET enable='%d' WHERE thread_tech_id='%s'", thread_classe, enable, thread_tech_id );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

    AGENT_send_to_agent ( domain, agent_uuid, "THREAD_STOP",  RootNode );
    if (enable) AGENT_send_to_agent ( domain, agent_uuid, "THREAD_START", RootNode );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread reloaded", RootNode );
  }
/******************************************************************************************************************************/
/* THREAD_DELETE_request: Appelé depuis libsoup pour supprimer un thread                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void THREAD_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT agent_uuid, thread_tech_id, thread_classe "
                                                        "FROM threads WHERE thread_tech_id='%s'", thread_tech_id );
    g_free(thread_tech_id);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

           thread_tech_id  = Json_get_string( RootNode, "thread_tech_id" );
    gchar *thread_classe   = Json_get_string( RootNode, "thread_classe" );
    gchar *agent_uuid      = Json_get_string( RootNode, "agent_uuid" );

    if (!thread_tech_id || !thread_classe || !agent_uuid)
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Tech_id, Agent or Class not found", RootNode ); return; }

    retour = DB_Write ( domain,"DELETE FROM %s WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

    AGENT_send_to_agent ( domain, agent_uuid, "THREAD_STOP", RootNode );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread deleted", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_LOAD_request_post: Repond aux requests Thread des agents                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_LOAD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;
    gboolean retour = DB_Read ( domain, RootNode, "threads",
                                "SELECT * FROM threads WHERE agent_uuid='%s'", agent_uuid );
    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_IO_request_post: Repond aux requests Thread des agents                                                      */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_classe" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    gchar *thread_classe  = Json_get_string (request, "thread_classe");

    if (!strcasecmp ( thread_classe, "modbus" ))
     { if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_entree_ana" )) return;
       if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_entree_tor" )) return;
       if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_sortie_tor" )) return;
       if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_sortie_tor" )) return;

       gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );

       gint nbr_entree_ana = Json_get_int ( request, "nbr_entree_ana" );
       gint nbr_entree_tor = Json_get_int ( request, "nbr_entree_tor" );
       gint nbr_sortie_ana = Json_get_int ( request, "nbr_sortie_ana" );
       gint nbr_sortie_tor = Json_get_int ( request, "nbr_sortie_tor" );
       Info_new ( __func__, LOG_INFO, domain, "'%s': Get %03d DI, %03d DO, %03d AI, %03d AO", thread_tech_id,
                      __func__, nbr_entree_tor, nbr_sortie_tor, nbr_entree_ana, nbr_sortie_ana );
       gboolean retour = TRUE;
       for (gint cpt=0; cpt<nbr_entree_ana; cpt++)
        { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_AI SET thread_tech_id='%s', thread_acronyme='AI%03d', num=%d",
                               thread_tech_id, cpt, cpt );
          retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='AI%03d'",
                               thread_tech_id, cpt );
        }
       for (gint cpt=0; cpt<nbr_sortie_ana; cpt++)
        { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_AO SET thread_tech_id='%s', thread_acronyme='AO%03d', num=%d",
                               thread_tech_id, cpt, cpt );
          retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='AO%03d'",
                               thread_tech_id, cpt );
        }
       for (gint cpt=0; cpt<nbr_entree_tor; cpt++)
        { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_DI SET thread_tech_id='%s', thread_acronyme='DI%03d', num=%d",
                               thread_tech_id, cpt, cpt );
          retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DI%03d'",
                               thread_tech_id, cpt );
        }
       for (gint cpt=0; cpt<nbr_sortie_tor; cpt++)
        { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_DO SET thread_tech_id='%s', thread_acronyme='DO%03d', num=%d",
                               thread_tech_id, cpt, cpt );
          retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DO%03d'",
                               thread_tech_id, cpt );
        }
       g_free(thread_tech_id);
       Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
     }
    else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "thread_classe unknown", NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_AI_request_post: Repond aux requests Thread des agents                                                      */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_AI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))       return;

    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );
    gchar *unite           = Json_get_string ( request, "unite" );
    gint   archivage       = Json_get_int    ( request, "archivage" );

    gboolean retour = Mnemo_auto_create_AI_from_thread ( domain, thread_tech_id, thread_acronyme, libelle, unite, archivage );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%s'",
                         thread_tech_id, thread_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_AO_request_post: Repond aux requests Thread des agents                                                      */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_AO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))       return;

    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );
    gchar *unite           = Json_get_string ( request, "unite" );
    gint   archivage       = Json_get_int    ( request, "archivage" );

    gboolean retour = Mnemo_auto_create_AO_from_thread ( domain, thread_tech_id, thread_acronyme, libelle, unite, archivage );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%s'",
                         thread_tech_id, thread_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_DI_request_post: Repond aux requests Thread des agents                                                      */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_DI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_DI_from_thread( domain, thread_tech_id, thread_acronyme, libelle );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%s'",
                         thread_tech_id, thread_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_HORLOGE_request_post: Repond aux requests Thread des agents                                                 */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_HORLOGE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_HORLOGE_from_thread( domain, thread_tech_id, thread_acronyme, libelle );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%s'",
                         thread_tech_id, thread_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_HORLOGE_TICK_request_post: Repond aux requests Thread des agents                                            */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_HORLOGE_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "heure" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "minute" ))          return;
    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gint heure             = Json_get_int    ( request, "heure" );
    gint minute            = Json_get_int    ( request, "minute" );

    gboolean retour = DB_Write ( domain, "INSERT INTO mnemos_HORLOGE_ticks SET thread_tech_id='%s', thread_acronyme='%s',"
                                         "heure=%d, minute=%d",
                                 thread_tech_id, thread_acronyme, heure, minute );
    AGENT_send_to_agent ( domain, NULL, "RELOAD_HORLOGE_TICK", NULL );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_WATCHDOG_request_post: Repond aux requests Thread des agents                                                */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_WATCHDOG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_WATCHDOG_from_thread( domain, thread_tech_id, thread_acronyme, libelle );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%s'",
                         thread_tech_id, thread_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_DO_request_post: Repond aux requests Thread des agents                                                      */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_DO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
    gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_DO_from_thread ( domain, thread_tech_id, thread_acronyme, libelle );
    retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%s'",
                         thread_tech_id, thread_acronyme );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_request_post: Repond aux requests du domain                                                                     */
/* Entrées: les elments libsoup                                                                                               */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_GET_CONFIG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { JsonNode *Recherche_thread = Http_json_node_create(msg);
    if (!Recherche_thread) return;

    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;

    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );
    DB_Read ( domain, Recherche_thread, NULL, "SELECT * FROM threads WHERE thread_tech_id ='%s'", thread_tech_id );

    if (!Json_has_member ( Recherche_thread, "thread_classe" ))
     { Info_new ( __func__, LOG_ERR, domain, "Thread_classe not found for thread_tech_id '%s'", thread_tech_id );
       g_free(thread_tech_id);
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "thread_tech_id is unknown", NULL );
       return;
     }

    gchar *thread_classe = Json_get_string ( Recherche_thread, "thread_classe" );
    JsonNode *RootNode = Http_json_node_create (msg);
    if (RootNode)
     { Json_node_add_string ( RootNode, "thread_classe", thread_classe );
       gboolean retour = DB_Read ( domain, RootNode, NULL,
                                  "SELECT * FROM %s WHERE agent_uuid='%s' AND thread_tech_id='%s'",
                                   thread_classe, agent_uuid, thread_tech_id );
       if (!strcasecmp ( thread_classe, "modbus" ) ||
           !strcasecmp ( thread_classe, "phidget" ) )
        { retour &= DB_Read ( domain, RootNode, "AI",
                             "SELECT * FROM %s_AI WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
          retour &= DB_Read ( domain, RootNode, "AO",
                             "SELECT * FROM %s_AO WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
          retour &= DB_Read ( domain, RootNode, "DI",
                              "SELECT * FROM %s_DI WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
          retour &= DB_Read ( domain, RootNode, "DO",
                              "SELECT * FROM %s_DO WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
        }
       else if (!strcasecmp ( thread_classe, "gpiod" ) )
        { retour &= DB_Read ( domain, RootNode, "IO",
                              "SELECT * FROM %s_IO WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
        }
       Json_node_add_bool ( RootNode, "api_cache", TRUE );                                  /* Active la cache sur les agents */
       Info_new ( __func__, LOG_INFO, domain, "Thread config '%s' sent", thread_tech_id );
       Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
     }
    else Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not enought Memory", NULL );
    g_free(thread_tech_id);
  }
/******************************************************************************************************************************/
/* THREAD_LIST_request_get: Liste les configs des thread de classe en parametre                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void THREAD_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (!Json_has_member ( url_param, "classe" ))                                                  /* Liste globale des threads */
     { JsonNode *RootNode = Http_json_node_create (msg);
       if (!RootNode) return;

       gboolean retour = DB_Read ( domain, RootNode, "threads",
                                  "SELECT t.*, agent_hostname FROM threads AS t INNER JOIN agents USING(agent_uuid)" );
       if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
       Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of threads", RootNode );
       return;
     }

    gchar *classe = Json_get_string ( url_param, "classe" );                           /* Focus sur une classe en particulier */
         if (!strcasecmp ( classe, "modbus"      )) classe = "modbus";
    else if (!strcasecmp ( classe, "audio"       )) classe = "audio";
    else if (!strcasecmp ( classe, "imsgs"       )) classe = "imsgs";
    else if (!strcasecmp ( classe, "smsg"        )) classe = "smsg";
    else if (!strcasecmp ( classe, "ups"         )) classe = "ups";
    else if (!strcasecmp ( classe, "teleinfoedf" )) classe = "teleinfoedf";
    else if (!strcasecmp ( classe, "meteo"       )) classe = "meteo";
    else { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "classe not found", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, classe,
                               "SELECT %s.*, agent_hostname FROM %s INNER JOIN agents USING(agent_uuid)", classe, classe );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of threads", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
