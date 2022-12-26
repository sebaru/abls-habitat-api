/******************************************************************************************************************************/
/* websocket.c         Gestion des echanges des elements dynamique vers les clients et agents                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * websocket.c
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
/* WS_Agent_on_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée depuis l'agent           */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void WS_Agent_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct WS_AGENT_SESSION *ws_agent = user_data;
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( __func__, LOG_WARNING, ws_agent->domain, "WebSocket Message Dropped (not JSON) : %s !", g_bytes_get_data ( message_brut, &taille ) );
       return;
     }

    if (!Json_has_member ( response, "tag" ))
     { Info_new( __func__, LOG_WARNING, ws_agent->domain, "WebSocket Message Dropped (no 'tag') !" );
       goto end_request;
     }

    gchar *tag = Json_get_string ( response, "tag" );
    Info_new( __func__, LOG_NOTICE, ws_agent->domain, "WebSocket Message Received : '%s'", tag );
end_request:
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_ws_on_closed: Traite une deconnexion                                                                                  */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void WS_Agent_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct WS_AGENT_SESSION *ws_agent = user_data;
    gchar *hostname = soup_client_context_get_host(ws_agent->context);
    Info_new( __func__, LOG_INFO, ws_agent->domain, "%s: WebSocket Closed", hostname );
    struct DOMAIN *domain = ws_agent->domain;
    pthread_mutex_lock ( &domain->synchro );
    domain->ws_agents = g_slist_remove ( domain->ws_agents, ws_agent );
    pthread_mutex_unlock ( &domain->synchro );
    g_free(ws_agent);
  }
 static void WS_Agent_on_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { struct WS_AGENT_SESSION *ws_agent = user_data;
    gchar *hostname = soup_client_context_get_host(ws_agent->context);
    Info_new( __func__, LOG_INFO, ws_agent->domain, "%s: WebSocket Error", hostname );
  }
/******************************************************************************************************************************/
/* Http_traiter_websocket: Traite une requete websocket                                                                       */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void WS_Agent_Open_CB ( SoupMessage *msg, gpointer user_data )
  { struct WS_AGENT_SESSION *ws_agent = user_data;

    SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(msg), "request-headers", &headers, NULL );
    gchar *origin     = soup_message_headers_get_one ( headers, "Origin" );
    SoupURI   *uri    = soup_message_get_uri ( msg );
    GIOStream *stream = soup_client_context_steal_connection ( ws_agent->context );
    ws_agent->connexion = soup_websocket_connection_new ( stream, uri, SOUP_WEBSOCKET_CONNECTION_SERVER, origin, "live-agent" );

    g_signal_connect ( ws_agent->connexion, "closed",  G_CALLBACK(WS_Agent_on_closed), ws_agent );
    g_signal_connect ( ws_agent->connexion, "error",   G_CALLBACK(WS_Agent_on_error), ws_agent );
    g_signal_connect ( ws_agent->connexion, "message", G_CALLBACK(WS_Agent_on_message), ws_agent );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/