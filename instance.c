/******************************************************************************************************************************/
/* instance.c                      Gestion des instances dans l'API HTTP WebService                                           */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * instance.c
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
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
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
         Json_has_member ( request, "start_time" ) && Json_has_member ( request, "hostname" ) &&
         Json_has_member ( request, "version" ) && Json_has_member ( request, "install_time" )
       )
     { gchar *hostname     = Normaliser_chaine ( Json_get_string ( request, "hostname") );
       gchar *version      = Normaliser_chaine ( Json_get_string ( request, "version") );
       gchar *install_time = Normaliser_chaine ( Json_get_string ( request, "install_time") );
       gint retour = DB_Write ( domain_uuid,
                               "INSERT INTO instances SET instance_uuid='%s', start_time=FROM_UNIXTIME(%d), hostname='%s', "
                               "version='%s', install_time='%s' "
                               "ON DUPLICATE KEY UPDATE start_time=VALUE(start_time), hostname=VALUE(hostname), version=VALUE(version)",
                               instance_uuid, Json_get_int (request, "start_time"), hostname, version, install_time );
       g_free(hostname);
       g_free(version);
       g_free(install_time);
       Http_Send_json_response ( msg, (retour ? "success" : "failed"), NULL );
     }
    else if ( !strcasecmp ( api_tag, "GET_CONFIG" ) )
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { DB_Read ( domain_uuid, RootNode, NULL,
                    "SELECT * FROM instances WHERE instance_uuid='%s'", instance_uuid );
          DB_Read ( domain_uuid, RootNode, NULL,
                    "SELECT hostname AS master_hostname FROM instances WHERE is_master=1 LIMIT 1" );

          Http_Send_json_response ( msg, "success", RootNode );
        }
       else soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
      }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
    json_node_unref(request);
  }
/******************************************************************************************************************************/
/* INSTANCE_request: Appeler sur l'URI /instance                                                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void INSTANCE_request ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                         SoupClientContext *client, gpointer user_data )
  {
         if (msg->method == SOUP_METHOD_POST) INSTANCE_request_post ( server, msg, path, query, client, user_data );
    else soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
