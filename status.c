/******************************************************************************************************************************/
/* status.c                      Gestion des requetes /status du WebService de watchdog                                       */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * status.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

    Json_add_string ( RootNode, "version",  ABLS_API_VERSION );
    Json_add_string ( RootNode, "product", "ABLS-HABITAT-API" );
    Json_add_string ( RootNode, "vendor",  "ABLS-HABITAT" );
    Json_add_string ( RootNode, "idp_url",   Json_get_string ( Global.config, "idp_url" ) );
    Json_add_string ( RootNode, "idp_realm", Json_get_string ( Global.config, "idp_realm" ) );
    Json_add_int    ( RootNode, "nbr_domains", DOMAIN_get_count() );
    gint64 now = g_get_real_time() / G_USEC_PER_SEC;
    Json_add_int    ( RootNode, "start_time", Global.start_time );
    Json_add_int    ( RootNode, "uptime", now - Global.start_time );
    Json_add_bool   ( RootNode, "mqtt_connected", g_atomic_int_get ( &Global.MQTT_connected ) );
    Json_add_bool   ( RootNode, "db_cache_enabled", (Global.memcached_pool != NULL) );
    Json_add_int    ( RootNode, "http_pool_thread_nbr", Global.http_pool_thread_nbr );
    Json_add_int    ( RootNode, "http_pool_queue_max",  Global.http_pool_queue_max );
    Json_add_int    ( RootNode, "http_pool_queued", Run_thread_pool_unprocessed ( Global.http_pool ) );
    Json_add_int    ( RootNode, "http_pool_busy", g_atomic_int_get ( &Global.http_busy ) );
    Json_add_int    ( RootNode, "dls_compil_pool_thread_nbr", Json_get_int ( Global.config, "dls_compil_pool_thread_nbr" ) );
    Json_add_int    ( RootNode, "dls_compil_pool_queued", Run_thread_pool_unprocessed ( Global.dls_compil_pool ) );
    Json_add_int    ( RootNode, "dls_compil_pool_busy", g_atomic_int_get ( &Global.dls_compil_pool_busy ) );
    Json_add_string ( RootNode, "docs",    "https://docs.abls-habitat.fr" );
    gboolean retour = DB_Read_with_cache ( DOMAIN_tree_get("master"), DB_CACHE_TTL_STATIC, RootNode, NULL,
                                           "SELECT count(*) AS nbr_icons FROM icons" );
    if (!retour)
     { Json_add_int  ( RootNode, "nbr_icons", 0 ); }
    Json_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active le cache sur les agents */
    Http_Send_json_response( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
