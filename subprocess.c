/******************************************************************************************************************************/
/* subprocess.c                      Gestion des subprocess dans l'API HTTP WebService                                        */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * subprocess.c
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
/* SUBPROCESS_request_post: Repond aux requests du domain                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SUBPROCESS_request_post ( gchar *domain_uuid, gchar *instance_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  { /*if (!Http_check_request( msg, session, 6 )) return;*/

    if ( !strcasecmp ( api_tag, "LOAD" ) )
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { DB_Read ( domain_uuid, RootNode, "subprocesses",
                    "SELECT * FROM subprocesses WHERE instance_uuid='%s'", instance_uuid );
          Http_Send_json_response ( msg, "success", RootNode );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
     }
    else if ( !strcasecmp ( api_tag, "GET_CONFIG" ) && Json_has_member ( request, "thread_tech_id" ) )
     { gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );
       JsonNode *Recherche_thread = Json_node_create();
       if (Recherche_thread)
        { g_free(thread_tech_id);
          soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
          return;
        }

       DB_Read ( "master", Recherche_thread, NULL, "SELECT * FROM subprocesses WHERE thread_tech_id ='%s'", thread_tech_id );
       if (!Json_has_member ( Recherche_thread, "thread_name" ))
        { g_free(thread_tech_id);
          soup_message_set_status (msg, SOUP_STATUS_NOT_FOUND);
          return;
        }

       gchar *thread_name = Json_get_string ( Recherche_thread, "thread_name" );
       JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { DB_Read ( domain_uuid, RootNode, NULL,
                    "SELECT * FROM %s WHERE instance_uuid='%s' AND thread_tech_id='%s'", thread_name, instance_uuid, thread_tech_id );
          if (!strcasecmp ( thread_name, "modbus" ) ||
              !strcasecmp ( thread_name, "phidget" ) ||
              !strcasecmp ( thread_name, "gpiod" ) )
           { DB_Read ( domain_uuid, RootNode, "AI",
                       "SELECT * FROM %s_AI WHERE thread_tech_id='%s'", thread_name, thread_tech_id );
             DB_Read ( domain_uuid, RootNode, "AO",
                       "SELECT * FROM %s_AO WHERE thread_tech_id='%s'", thread_name, thread_tech_id );
             DB_Read ( domain_uuid, RootNode, "DI",
                       "SELECT * FROM %s_AI WHERE thread_tech_id='%s'", thread_name, thread_tech_id );
             DB_Read ( domain_uuid, RootNode, "DO",
                       "SELECT * FROM %s_AO WHERE thread_tech_id='%s'", thread_name, thread_tech_id );
           }
          Http_Send_json_response ( msg, "success", RootNode );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       g_free(thread_tech_id);
      }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
