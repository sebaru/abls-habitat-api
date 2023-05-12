/******************************************************************************************************************************/
/* thread.c                      Gestion des thread dans l'API HTTP WebService                                        */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.c
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
/* Check_thread_classe: Vérifie qu'une classe de thread existe                                                                */
/* Entrées: la classe a controler                                                                                             */
/* Sortie : NULL si erreur, sinon la classe elle meme                                                                         */
/******************************************************************************************************************************/
 static gchar *Check_thread_classe ( gchar *thread_classe )
  { if (!thread_classe) return(NULL);
         if (!strcasecmp ( thread_classe, "modbus"      )) return ("modbus");
    else if (!strcasecmp ( thread_classe, "audio"       )) return ("audio");
    else if (!strcasecmp ( thread_classe, "imsgs"       )) return ("imsgs");
    else if (!strcasecmp ( thread_classe, "smsg"        )) return ("smsg");
    else if (!strcasecmp ( thread_classe, "ups"         )) return ("ups");
    else if (!strcasecmp ( thread_classe, "teleinfoedf" )) return ("teleinfoedf");
    else if (!strcasecmp ( thread_classe, "meteo"       )) return ("meteo");
    else if (!strcasecmp ( thread_classe, "phidget"     )) return ("phidget");
    else if (!strcasecmp ( thread_classe, "gpiod"       )) return ("gpiod");
    return(NULL);
  }
/******************************************************************************************************************************/
/* THREAD_SEND_request_post: Envoi un requete à un thread                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void THREAD_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
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
 void THREAD_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
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

    AGENT_send_to_agent ( domain, agent_uuid, "THREAD_RESTART", RootNode );                        /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread reloaded", RootNode );
  }
/******************************************************************************************************************************/
/* THREAD_DELETE_request: Appelé depuis libsoup pour supprimer un thread                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void THREAD_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
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
 void RUN_THREAD_LOAD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;
    gboolean retour = DB_Read ( domain, RootNode, "threads",
                                "SELECT * FROM threads WHERE agent_uuid='%s'", agent_uuid );
    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* RUN_THREAD_HEARTBEAT_request_post: Repond aux requests HeartBeat des threads                                               */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_HEARTBEAT_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_classe" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "io_comm" )) return;

    gchar *thread_classe  = Check_thread_classe ( Json_get_string (request, "thread_classe") );
    if (!thread_classe) { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "thread_classe unknown", NULL ); return; }
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string (request, "thread_tech_id") );
    gboolean retour = DB_Write ( domain, "UPDATE `%s` SET last_comm = %s WHERE thread_tech_id='%s' AND agent_uuid='%s'",
                                 thread_classe, (Json_get_bool ( request, "io_comm" ) ? "NOW()" : "NULL"), thread_tech_id, agent_uuid );
    g_free(thread_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL);
  }
/******************************************************************************************************************************/
/* RUN_THREAD_ADD_AI_request_post: Repond aux requests Thread des agents                                                      */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_AI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
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
 void RUN_THREAD_ADD_AO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
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
 void RUN_THREAD_ADD_DI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
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
/* RUN_THREAD_ADD_WATCHDOG_request_post: Repond aux requests Thread des agents                                                */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_ADD_WATCHDOG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
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
 void RUN_THREAD_ADD_DO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
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
/* RUN_THREAD_CONFIG_request_post: Donne la config d'un thread aux agents                                                     */
/* Entrées: les elments libsoup                                                                                               */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_CONFIG_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param )
  { if (Http_fail_if_has_not ( domain, path, msg, url_param, "thread_tech_id" )) return;

    JsonNode *Recherche_thread = Json_node_create();
    if (!Recherche_thread) return;

    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "thread_tech_id" ) );
    DB_Read ( domain, Recherche_thread, NULL, "SELECT * FROM threads WHERE thread_tech_id ='%s' AND agent_uuid='%s'",
                                              thread_tech_id, agent_uuid );

    if (!Json_has_member ( Recherche_thread, "thread_classe" ))
     { Info_new ( __func__, LOG_ERR, domain, "Thread_classe not found for thread_tech_id '%s' on agent '%s'", thread_tech_id, agent_uuid );
       g_free(thread_tech_id);
       json_node_unref ( Recherche_thread );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Thread_classe not found", NULL );
       return;
     }

    gchar *thread_classe = Check_thread_classe ( Json_get_string ( Recherche_thread, "thread_classe" ) );
    if (!thread_classe)
     { Info_new ( __func__, LOG_ERR, domain, "Thread_classe unknown for thread_tech_id '%s' on agent '%s'", thread_tech_id, agent_uuid );
       g_free(thread_tech_id);
       json_node_unref ( Recherche_thread );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Thread_classe unknown", NULL );
       return;
     }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (RootNode)
     { Json_node_add_string ( RootNode, "thread_classe", thread_classe );
       gboolean retour = DB_Read ( domain, RootNode, NULL,
                                  "SELECT * FROM %s WHERE agent_uuid='%s' AND thread_tech_id='%s'",
                                   thread_classe, agent_uuid, thread_tech_id );
       if (!strcasecmp ( thread_classe, "modbus" ))
        { retour &= DB_Read ( domain, RootNode, "AI",
                              "SELECT * FROM modbus_AI WHERE thread_tech_id='%s'", thread_tech_id );
          retour &= DB_Read ( domain, RootNode, "AO",
                              "SELECT * FROM modbus_AO WHERE thread_tech_id='%s'", thread_tech_id );
          retour &= DB_Read ( domain, RootNode, "DI",
                              "SELECT * FROM modbus_DI WHERE thread_tech_id='%s'", thread_tech_id );
          retour &= DB_Read ( domain, RootNode, "DO",
                              "SELECT * FROM modbus_DO WHERE thread_tech_id='%s'", thread_tech_id );
        }
       else if (!strcasecmp ( thread_classe, "phidget" ) )
        { retour &= DB_Read ( domain, RootNode, "IO",
                              "SELECT * FROM phidget_IO WHERE thread_tech_id='%s'", thread_tech_id );
        }
       else if (!strcasecmp ( thread_classe, "gpiod" ) )
        { retour &= DB_Read ( domain, RootNode, "IO",
                              "SELECT * FROM %s_IO WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
        }
       Json_node_add_bool ( RootNode, "api_cache", TRUE );                                  /* Active le cache sur les agents */
       Info_new ( __func__, LOG_INFO, domain, "Thread config '%s' sent", thread_tech_id );
       Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
     }
    else Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not enought Memory", NULL );
    g_free(thread_tech_id);
    json_node_unref ( Recherche_thread );
  }
/******************************************************************************************************************************/
/* THREAD_LIST_request_get: Liste les configs des thread de classe en parametre                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void THREAD_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
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

    gchar *classe = Check_thread_classe ( Json_get_string ( url_param, "classe" ) );   /* Focus sur une classe en particulier */
    if (!classe) { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "classe not found", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, classe,
                               "SELECT %s.*, agent_hostname FROM %s INNER JOIN agents USING(agent_uuid)", classe, classe );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of threads", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
