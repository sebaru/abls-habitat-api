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
/* DOMAIN_request_get: Appelé depuis libsoup                                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void INSTANCE_request_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                  SoupClientContext *client, gpointer user_data )
  {
    Http_print_request ( __func__, server, msg, path, client );
    gchar *domain_uuid   = Http_get_request_parameter ( query, "domain_uuid" );
    if (!domain_uuid)
     { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "domain_uuid missing" );
       return;
     }

    gchar *instance_uuid = Http_get_request_parameter ( query, "instance_uuid" );
    if (!domain_uuid || !instance_uuid)
     { g_free(domain_uuid);
       soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "instance_uuid missing" );
       return;
     }


    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { DB_Read ( DOMAIN_tree_get ( domain_uuid ), RootNode, "instance",
                 "SELECT * FROM instances WHERE instance_uuid='%s'", instance_uuid );
       Http_Send_json_response ( msg, RootNode );
     }
    else { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" ); }

    g_free(domain_uuid);
    g_free(instance_uuid);
  }
/******************************************************************************************************************************/
/* DOMAIN_request: Appeler sur l'URI /domain                                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void INSTANCE_request ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                         SoupClientContext *client, gpointer user_data )
  {
    if (msg->method == SOUP_METHOD_GET) INSTANCE_request_get ( server, msg, path, query, client, user_data );
    else	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
