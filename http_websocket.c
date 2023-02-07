/******************************************************************************************************************************/
/* http_websocket.c         Gestion des echanges des elements websocket avec les Users                                        */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http_websocket.c
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
/* WS_Http_send_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                         */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void WS_Http_send_to_all ( struct DOMAIN *domain, JsonNode *node )
  { gchar *buf = Json_node_to_string ( node );
    GSList *ws_https = domain->ws_https;
    while (ws_https)
     { struct WS_HTTP_SESSION *ws_http = ws_https->data;
       soup_websocket_connection_send_text ( ws_http->connexion, buf );
       ws_https = g_slist_next ( ws_https );
     }
    g_free(buf);
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Envoyer_un_cadran: Envoi un update cadran au client                                                                        */
/* Entrée: une reference sur la session en cours, et le cadran a envoyer                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void HTTP_CADRAN_to_json ( JsonNode *node, struct HTTP_CADRAN *http_cadran )
  { Json_node_add_string ( node, "tech_id",  http_cadran->tech_id );
    Json_node_add_string ( node, "acronyme", http_cadran->acronyme );
    Json_node_add_string ( node, "classe",   http_cadran->classe );
    Json_node_add_bool   ( node, "in_range", http_cadran->in_range );
    Json_node_add_double ( node, "valeur",   http_cadran->valeur );
    Json_node_add_string ( node, "unite",    http_cadran->unite );
  }
/******************************************************************************************************************************/
/* Http_ws_on_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket                              */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Http_ws_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct WS_CLIENT_SESSION *client = user_data;
    Info_new( Config.log, Config.log_msrv, LOG_INFO, NULL, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( response, "zmq_tag" ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket Message Dropped (no 'zmq_tag') !", __func__ );
       json_node_unref(response);
       return;
     }

    gchar *zmq_tag = Json_get_string( response, "zmq_tag" );

    if(!strcasecmp(zmq_tag,"CONNECT"))
     { if ( ! (Json_has_member( response, "wtd_session") ))
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket without wtd_session !", __func__ ); }
       else
        { gchar *wtd_session = Json_get_string ( response, "wtd_session");
          GSList *liste = Partage->com_http.liste_http_clients;
          while ( liste )                                                      /* Recherche de la session HTTP correspondante */
           { struct HTTP_CLIENT_SESSION *http_session = liste->data;
             if (!strcmp(http_session->wtd_session, wtd_session))
              { client->http_session = http_session;
                pthread_mutex_lock( &Partage->com_http.synchro );
                http_session->liste_ws_clients = g_slist_prepend ( http_session->liste_ws_clients, client );
                pthread_mutex_unlock( &Partage->com_http.synchro );
                Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: session found for '%s' !", __func__, http_session->username );
                break;
              }
             liste = g_slist_next ( liste );
           }
        }
     }

    if (!client->http_session)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Not authorized !", __func__ ); }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_Envoyer_les_cadrans: Envoi les cadrans aux clients                                                                    */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_Envoyer_les_cadrans ( void )
  { pthread_mutex_lock( &Partage->com_http.synchro );
    GSList *sessions = Partage->com_http.liste_http_clients;
    while ( sessions )
     { struct HTTP_CLIENT_SESSION *session = sessions->data;
       GSList *cadrans = session->Liste_bit_cadrans;
       while ( cadrans )
        { struct HTTP_CADRAN *cadran = cadrans->data;
          if (cadran->last_update + 10 <= Partage->top)
           { GSList *clients = session->liste_ws_clients;
             JsonNode *RootNode = Json_node_create();
             if (RootNode)
              { Http_Formater_cadran ( cadran );
                Json_node_add_string ( RootNode, "zmq_tag", "DLS_CADRAN" );
                HTTP_CADRAN_to_json ( RootNode, cadran );
                while (clients)
                 { struct WS_CLIENT_SESSION *client = clients->data;
                   Http_ws_send_to_client ( client, RootNode );
                   clients = g_slist_next(clients);
                 }
                json_node_unref( RootNode );
              }
             cadran->last_update = Partage->top;
           }
          cadrans = g_slist_next(cadrans);
        }
       sessions = g_slist_next(sessions);
     }
    pthread_mutex_unlock( &Partage->com_http.synchro );
  }
#endif
/******************************************************************************************************************************/
/* WS_Http_on_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée depuis l'agent           */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void WS_Http_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct WS_HTTP_SESSION *ws_http = user_data;
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( __func__, LOG_WARNING, ws_http->domain, "WebSocket Message Dropped (not JSON) : %s !", g_bytes_get_data ( message_brut, &taille ) );
       return;
     }

    if (!Json_has_member ( response, "tag" ))
     { Info_new( __func__, LOG_WARNING, ws_http->domain, "WebSocket Message Dropped (no 'tag') !" );
       goto end_request;
     }

    gchar *tag = Json_get_string ( response, "tag" );
    Info_new( __func__, LOG_NOTICE, ws_http->domain, "WebSocket Message Received : '%s'", tag );
    if (!strcasecmp ( tag, "abonner" ))
     { if (ws_http->abonnements) json_node_unref ( ws_http->abonnements );           /* Normalement ne devrait jamais arriver */
       json_node_ref ( response );
       ws_http->abonnements = response;
       AGENT_send_to_agent ( ws_http->domain, NULL, "ABONNER", response );
     }
end_request:
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Http_ws_on_closed: Traite une deconnexion                                                                                  */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void WS_Http_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { struct WS_HTTP_SESSION *ws_http = user_data;
    gchar *hostname = soup_client_context_get_host(ws_http->context);
    Info_new( __func__, LOG_INFO, ws_http->domain, "%s: WebSocket Closed", hostname );
    struct DOMAIN *domain = ws_http->domain;
    pthread_mutex_lock ( &domain->synchro );
    domain->ws_https = g_slist_remove ( domain->ws_https, ws_http );
    pthread_mutex_unlock ( &domain->synchro );
    if(ws_http->abonnements) json_node_unref(ws_http->abonnements);
    g_free(ws_http);
  }
 static void WS_Http_on_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { struct WS_HTTP_SESSION *ws_http = user_data;
    gchar *hostname = soup_client_context_get_host(ws_http->context);
    Info_new( __func__, LOG_INFO, ws_http->domain, "%s: WebSocket Error", hostname );
  }
/******************************************************************************************************************************/
/* WS_Http_Open_CB: Traite une requete websocket                                                                              */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void WS_Http_Open_CB ( SoupMessage *msg, gpointer user_data )
  { struct WS_HTTP_SESSION *ws_http = user_data;

    SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(msg), "request-headers", &headers, NULL );
    gchar *origin     = soup_message_headers_get_one ( headers, "Origin" );
    SoupURI   *uri    = soup_message_get_uri ( msg );
    GIOStream *stream = soup_client_context_steal_connection ( ws_http->context );
    ws_http->connexion = soup_websocket_connection_new ( stream, uri, SOUP_WEBSOCKET_CONNECTION_SERVER, origin, "live-http" );

    g_signal_connect ( ws_http->connexion, "closed",  G_CALLBACK(WS_Http_on_closed), ws_http );
    g_signal_connect ( ws_http->connexion, "error",   G_CALLBACK(WS_Http_on_error), ws_http );
    g_signal_connect ( ws_http->connexion, "message", G_CALLBACK(WS_Http_on_message), ws_http );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
