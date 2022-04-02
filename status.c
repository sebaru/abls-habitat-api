/******************************************************************************************************************************/
/* status.c                      Gestion des requetes /status du WebService de watchdog                                       */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * status.c
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
/* Http_get_status: fourni des informations sur le status de l'API                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void STATUS_request_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                           SoupClientContext *client, gpointer user_data )
  { JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }
    Json_node_add_string ( RootNode, "version",  ABLS_API_VERSION );
    Json_node_add_string ( RootNode, "product", "ABLS-HABITAT-API" );
    Json_node_add_string ( RootNode, "vendor",  "ABLS-HABITAT" );
    Json_node_add_int    ( RootNode, "nbr_domains", g_tree_nnodes (Global.domaines) );
    Json_node_add_string ( RootNode, "author",  "Sébastien Lefèvre" );
    Json_node_add_string ( RootNode, "docs",    "https://docs.abls-habitat.fr" );
    DB_Read ( DOMAIN_tree_get("master"), RootNode, NULL, "SELECT count(*) AS nbr_icons FROM icons" );
    Http_Send_json_response( msg, "success", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
