/******************************************************************************************************************************/
/* Instance.c                      Gestion des instances dans l'API HTTP WebService                                           */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Config.c
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
/* INSTANCE_request_post: Repond aux requests du domain                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void INSTANCE_request_post ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                              SoupClientContext *client, gpointer user_data )
  { JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) return;
    /*if (!Http_check_request( msg, session, 6 )) return;*/

    gchar *domain_uuid   = Json_get_string ( request, "domain_uuid" );
    gchar *instance_uuid = Json_get_string ( request, "instance_uuid" );
    gchar *api_tag       = Json_get_string ( request, "api_tag" );
    Info_new ( __func__, LOG_INFO, "Domain '%s', instance '%s', tag='%s'", domain_uuid, instance_uuid, api_tag );

    if ( !strcasecmp ( api_tag, "START" ) &&
         Json_has_member ( request, "start_time" ) && Json_has_member ( request, "hostname" ) && Json_has_member ( request, "version" )
       )
     { gchar *hostname = Normaliser_chaine ( Json_get_string ( request, "hostname") );
       gchar *version  = Normaliser_chaine ( Json_get_string ( request, "version") );
       DB_Write ( domain_uuid, "INSERT INTO instances SET instance_uuid='%s', start_time=FROM_UNIXTIME(%d), hostname='%s', "
                               "version='%s' "
                               "ON DUPLICATE KEY UPDATE start_time=VALUE(start_time), hostname=VALUE(hostname), version=VALUE(version)",
                               instance_uuid, Json_get_int (request, "start_time"), hostname, version );
       g_free(hostname);
       g_free(version);
       Http_Send_simple_response ( msg, "updated" );
     }
    else if ( !strcasecmp ( api_tag, "GET_CONFIG" ) )
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { DB_Read ( domain_uuid, RootNode, NULL,
                    "SELECT * FROM instances WHERE instance_uuid='%s'", instance_uuid );
          DB_Read ( domain_uuid, RootNode, NULL,
                    "SELECT hostname AS master_hostname FROM instances WHERE is_master=1 LIMIT 1" );

          Http_Send_json_response ( msg, RootNode );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
      }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* DOMAIN_request: Appeler sur l'URI /domain                                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void INSTANCE_request ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                         SoupClientContext *client, gpointer user_data )
  {
         if (msg->method == SOUP_METHOD_POST) INSTANCE_request_post ( server, msg, path, query, client, user_data );
    else	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
