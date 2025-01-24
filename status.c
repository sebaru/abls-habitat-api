/******************************************************************************************************************************/
/* status.c                      Gestion des requetes /status du WebService de watchdog                                       */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * status.c
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
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* Http_get_status: fourni des informations sur le status de l'API                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void STATUS_request_get ( SoupServer *server, SoupServerMessage *msg, const char *path )
  { JsonNode *RootNode = Http_json_node_create(msg);
    if (!RootNode) return;

    Json_node_add_string ( RootNode, "version",  ABLS_API_VERSION );
    Json_node_add_string ( RootNode, "product", "ABLS-HABITAT-API" );
    Json_node_add_string ( RootNode, "vendor",  "ABLS-HABITAT" );
    Json_node_add_int    ( RootNode, "nbr_domains", g_tree_nnodes (Global.domaines) );
    Json_node_add_string ( RootNode, "author",  "Sébastien Lefèvre" );
    Json_node_add_string ( RootNode, "docs",    "https://docs.abls-habitat.fr" );
    gboolean retour = DB_Read ( DOMAIN_tree_get("master"), RootNode, NULL, "SELECT count(*) AS nbr_icons FROM icons" );
    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    Http_Send_json_response( msg, retour, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
