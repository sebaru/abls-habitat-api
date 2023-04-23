/******************************************************************************************************************************/
/* http_websocket.c         Gestion des echanges des elements websocket avec les Users                                        */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http_websocket.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* WS_Client_send_cadran_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void WS_Client_send_cadran_to_all ( struct DOMAIN *domain, JsonNode *node )
  { gchar *tech_id  = Json_get_string ( node, "tech_id" );
    gchar *acronyme = Json_get_string ( node, "acronyme" );
    if (!(tech_id && acronyme)) return;
    gchar *buf      = Json_node_to_string ( node );
    GSList *ws_clients = domain->ws_clients;
    gint cpt=0;
    while (ws_clients)
     { struct WS_CLIENT_SESSION *ws_client = ws_clients->data;
       GList *Cadrans = json_array_get_elements ( Json_get_array ( ws_client->abonnements, "cadrans" ) );
       GList *cadrans = Cadrans;
       while(cadrans)
        { JsonNode *element = cadrans->data;
          if (!strcasecmp ( Json_get_string ( element, "tech_id"  ), tech_id  ) &&
              !strcasecmp ( Json_get_string ( element, "acronyme" ), acronyme ) )
           { soup_websocket_connection_send_text ( ws_client->connexion, buf ); cpt++; }
          cadrans = g_list_next(cadrans);
        }
       g_list_free(Cadrans);
       ws_clients = g_slist_next ( ws_clients );
     }
    Info_new( __func__, LOG_DEBUG, domain, "Cadran %s:%s sent to %d clients :%s", tech_id, acronyme, cpt, buf );
    if (cpt==0)
     { AGENT_send_to_agent ( domain, NULL, "DESABONNER", node );
       g_tree_remove ( domain->abonnements, node );                                           /* node_unref is part of remove */
     }
    g_free(buf);
  }
/******************************************************************************************************************************/
/* WS_Client_send_to_all: Envoi d'un buffer a tous les clients connectés à la websocket                                       */
/* Entrée: Le buffer                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void WS_Client_send_to_all ( struct DOMAIN *domain, JsonNode *node )
  { gchar *buf = Json_node_to_string ( node );
    Info_new( __func__, LOG_DEBUG, domain, "Sending to %d clients :%s", g_slist_length(domain->ws_clients), buf );
    GSList *ws_clients = domain->ws_clients;
    while (ws_clients)
     { struct WS_CLIENT_SESSION *ws_client = ws_clients->data;
       soup_websocket_connection_send_text ( ws_client->connexion, buf );
       ws_clients = g_slist_next ( ws_clients );
     }
    g_free(buf);
  }
/******************************************************************************************************************************/
/* WS_Http_on_message: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée depuis l'ihm              */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void WS_Http_on_message ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { struct WS_CLIENT_SESSION *ws_client = user_data;
    gsize taille;

    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response)
     { Info_new( __func__, LOG_WARNING, ws_client->domain, "WebSocket Message Dropped (not JSON) : %s !", g_bytes_get_data ( message_brut, &taille ) );
       return;
     }

    if (!Json_has_member ( response, "tag" ))
     { Info_new( __func__, LOG_WARNING, ws_client->domain, "WebSocket Message Dropped (no 'tag') !" );
       goto end_request;
     }

    gchar *tag = Json_get_string ( response, "tag" );
    Info_new( __func__, LOG_NOTICE, ws_client->domain, "WebSocket Message Received : '%s'", tag );
    if (!strcasecmp ( tag, "abonner" ) && Json_has_member( response, "syn_id" ) )
     { if (ws_client->abonnements) json_node_unref ( ws_client->abonnements );       /* Normalement ne devrait jamais arriver */
       ws_client->abonnements = Json_node_create();
       gint syn_id = Json_get_int ( response, "syn_id" );
       DB_Read ( ws_client->domain, ws_client->abonnements, "cadrans",
                 "SELECT cadran.tech_id, cadran.acronyme, dico.classe FROM syns_cadrans AS cadran "
                 "INNER JOIN dls AS dls ON cadran.dls_id=dls.dls_id "
                 "INNER JOIN syns AS syn ON dls.syn_id=syn.syn_id "
                 "INNER JOIN dictionnaire AS dico ON (cadran.tech_id=dico.tech_id AND cadran.acronyme=dico.acronyme) "
                 "WHERE syn.syn_id=%d AND syn.access_level<=%d",
                 syn_id, ws_client->user_access_level );
       gint nbr_cadrans = Json_get_int ( ws_client->abonnements, "nbr_cadrans" ) ;
       if (nbr_cadrans)
        { Info_new( __func__, LOG_INFO, ws_client->domain, "Demande d'abonnement sur %d cadrans auprès du master", nbr_cadrans );
          AGENT_send_to_agent ( ws_client->domain, NULL, "ABONNER", ws_client->abonnements );
        }
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
  { struct WS_CLIENT_SESSION *ws_client = user_data;
    Info_new( __func__, LOG_INFO, ws_client->domain, "WebSocket Closed" );
    struct DOMAIN *domain = ws_client->domain;
    pthread_mutex_lock ( &domain->synchro );
    domain->ws_clients = g_slist_remove ( domain->ws_clients, ws_client );
    pthread_mutex_unlock ( &domain->synchro );
    if(ws_client->abonnements) json_node_unref(ws_client->abonnements);
    g_free(ws_client);
  }
 static void WS_Http_on_error ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { struct WS_CLIENT_SESSION *ws_client = user_data;
    Info_new( __func__, LOG_INFO, ws_client->domain, "WebSocket Error" );
  }
/******************************************************************************************************************************/
/* WS_Http_Open_CB: Traite une requete websocket                                                                              */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void WS_Http_Open_CB ( SoupServerMessage *msg, gpointer user_data )
  { struct WS_CLIENT_SESSION *ws_client = user_data;

    SoupMessageHeaders *headers = soup_server_message_get_request_headers ( msg );
    gchar *origin     = soup_message_headers_get_one ( headers, "Origin" );
    GUri  *uri        = soup_server_message_get_uri ( msg );
    GIOStream *stream = soup_server_message_steal_connection ( msg );
    ws_client->connexion = soup_websocket_connection_new ( stream, uri, SOUP_WEBSOCKET_CONNECTION_SERVER, origin, "live-http", NULL );
    soup_websocket_connection_set_keepalive_interval ( ws_client->connexion, 30 );

    g_signal_connect ( ws_client->connexion, "closed",  G_CALLBACK(WS_Http_on_closed), ws_client );
    g_signal_connect ( ws_client->connexion, "error",   G_CALLBACK(WS_Http_on_error), ws_client );
    g_signal_connect ( ws_client->connexion, "message", G_CALLBACK(WS_Http_on_message), ws_client );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
