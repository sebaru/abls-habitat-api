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
    DB_Read ( domain, RootNode, NULL, "SELECT agent_uuid, thread_tech_id, thread_classe "
                                      "FROM threads WHERE thread_tech_id='%s'", thread_tech_id );
    g_free(thread_tech_id);

           thread_tech_id  = Json_get_string( RootNode, "thread_tech_id" );
    gchar *thread_classe   = Json_get_string( RootNode, "thread_classe" );
    gchar *agent_uuid      = Json_get_string( RootNode, "agent_uuid" );

    if (thread_tech_id && thread_classe && agent_uuid)
     { gboolean retour = DB_Write ( domain,"UPDATE %s SET enable='%d' WHERE thread_tech_id='%s'", thread_classe, enable, thread_tech_id );
       if (retour) AGENT_send_to_agent ( domain, agent_uuid, (enable ? "THREAD_ENABLE" : "THREAD_DISABLE"), NULL );
       Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
     }
    else { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Tech_id, Agent or Class not found", NULL ); }
    json_node_unref(RootNode);
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
    DB_Read ( domain, RootNode, NULL, "SELECT agent_uuid, thread_tech_id, thread_classe "
                                      "FROM threads WHERE thread_tech_id='%s'", thread_tech_id );
    g_free(thread_tech_id);

           thread_tech_id  = Json_get_string( RootNode, "thread_tech_id" );
    gchar *thread_classe   = Json_get_string( RootNode, "thread_classe" );
    gchar *agent_uuid      = Json_get_string( RootNode, "agent_uuid" );

    if (thread_tech_id && thread_classe && agent_uuid)
     { gboolean retour = DB_Write ( domain,"DELETE FROM %s WHERE thread_tech_id='%s'", thread_classe, thread_tech_id );
       if (retour) AGENT_send_to_agent ( domain, agent_uuid, "THREAD_DELETE", RootNode );
       Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
     }
    else { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Tech_id, Agent or Class not found", NULL ); }
    json_node_unref(RootNode);
  }
/******************************************************************************************************************************/
/* THREAD_request_post: Repond aux requests du domain                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_THREAD_request_post ( struct DOMAIN *domain, gchar *agent_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  {
/*------------------------------------------------ Loading on threads ---------------------------------------------------*/
    if ( !strcasecmp ( api_tag, "LOAD" ) )
     { JsonNode *RootNode = Http_json_node_create (msg);
       if (!RootNode) return;
       gboolean retour = DB_Read ( domain, RootNode, "threads",
                                   "SELECT * FROM threads WHERE agent_uuid='%s'", agent_uuid );
       Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
     }
/*------------------------------------------------ IO Detection On MODBUS ----------------------------------------------------*/
    else if ( !strcasecmp ( api_tag, "ADD_IO" ) )
     { if (Http_fail_if_has_not ( domain, "/run/thread", msg, request, "thread_classe" )) return;
       if (Http_fail_if_has_not ( domain, "/run/thread", msg, request, "thread_tech_id" )) return;
       gchar *thread_classe  = Json_get_string (request, "thread_classe");

       if (!strcasecmp ( thread_classe, "modbus" ))
        { if (Http_fail_if_has_not ( domain, "/run/thread", msg, request, "nbr_entree_ana" )) return;
          if (Http_fail_if_has_not ( domain, "/run/thread", msg, request, "nbr_entree_tor" )) return;
          if (Http_fail_if_has_not ( domain, "/run/thread", msg, request, "nbr_sortie_tor" )) return;
          if (Http_fail_if_has_not ( domain, "/run/thread", msg, request, "nbr_sortie_tor" )) return;

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
/*------------------------------------------------ Send config of one thread -------------------------------------------------*/
    else if ( !strcasecmp ( api_tag, "GET_CONFIG" ) )
     { JsonNode *Recherche_thread = Http_json_node_create(msg);
       if (!Recherche_thread) return;

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
        { gboolean retour = DB_Read ( domain, RootNode, NULL,
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
          Info_new ( __func__, LOG_INFO, domain, "Subprocess config '%s' sent", thread_tech_id );
          Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
        }
       g_free(thread_tech_id);
      }
    else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "api_tag unknown", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
