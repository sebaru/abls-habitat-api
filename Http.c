/******************************************************************************************************************************/
/* Http.c                      Gestion des connexions HTTP WebService de watchdog                                             */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.c
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
 static gboolean Keep_running = TRUE;
 struct GLOBAL Global;                                                                              /* Configuration de l'API */
#ifdef bouh
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Msg_to_Json ( SoupMessage *msg )
  { GBytes *request_brute;
    gsize taille;
    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( request_brute, &taille ) );
    if ( !request) { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Not a JSON request"); }
    return(request);
  }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Response_Msg_to_Json ( SoupMessage *msg )
  { GBytes *reponse_brute;
    gsize taille;
    g_object_get ( msg, "response-body-data", &reponse_brute, NULL );
    JsonNode *reponse = Json_get_from_string ( g_bytes_get_data ( reponse_brute, &taille ) );
    return(reponse);
  }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 gint Http_Msg_status_code ( SoupMessage *msg )
  { gint status;
    g_object_get ( msg, "status-code", &status, NULL );
    return(status);
  }
/******************************************************************************************************************************/
/* Http_Msg_to_Json: Récupère la partie payload du msg, au format JSON                                                        */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 gchar *Http_Msg_reason_phrase ( SoupMessage *msg )
  { gchar *phrase;
    g_object_get ( msg, "reason-phrase", &phrase, NULL );
    return(phrase);
  }
#endif
/******************************************************************************************************************************/
/* Http_print_request: affiche les données relatives à une requete                                                            */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Http_print_request ( gchar *function, SoupServer *server, SoupMessage *msg, const char *path, SoupClientContext *client )
  { Info_new( function, LOG_INFO, "%s: '%s'", soup_client_context_get_host(client), path ); }
/******************************************************************************************************************************/
/* Http_get_request_parameter: Renvoi un parametre sanitizé                                                                   */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: le tampon sanitizé                                                                                                 */
/******************************************************************************************************************************/
 gchar *Http_get_request_parameter ( GHashTable *query, gchar *name )
  { gchar *valeur = g_hash_table_lookup ( query, name );
    if (!valeur) return(NULL);
    return ( Normaliser_chaine ( valeur ) );
  }
/******************************************************************************************************************************/
/* Http_Send_json_response: Envoie le json en paramètre en prenant le lead dessus                                             */
/* Entrée: le messages, le buffer json                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Send_json_response ( SoupMessage *msg, JsonNode *RootNode )
  { gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, SOUP_STATUS_OK);
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                                             */
/* Entrée: numero du signal à gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  { switch (num)
     { case SIGQUIT:
       case SIGINT:  Info_new( __func__, LOG_INFO, "Recu SIGINT" );
                     Keep_running = FALSE;                                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGTERM: Info_new( __func__, LOG_INFO, "Recu SIGTERM" );
                     Keep_running = FALSE;                                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGABRT: Info_new( __func__, LOG_INFO, "Recu SIGABRT" );
                     break;
       case SIGCHLD: Info_new( __func__, LOG_INFO, "Recu SIGCHLD" );
                     break;
       case SIGPIPE: Info_new( __func__, LOG_INFO, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info_new( __func__, LOG_INFO, "Recu SIGBUS" ); break;
       case SIGIO:   Info_new( __func__, LOG_INFO, "Recu SIGIO" ); break;
       case SIGUSR1: Info_new( __func__, LOG_INFO, "Recu SIGUSR1" );
                     break;
       case SIGUSR2: Info_new( __func__, LOG_INFO, "Recu SIGUSR2" );
                     break;
       default: Info_new( __func__, LOG_NOTICE, "Recu signal %d", num ); break;
     }
  }
/******************************************************************************************************************************/
/* Http_get_status: fourni des informations sur le status de l'API                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Http_get_status ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                               SoupClientContext *client, gpointer user_data )
  { if (msg->method != SOUP_METHOD_GET)
     { soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
      return;
     }
    JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
       return;
     }
    Json_node_add_string ( RootNode, "version",  ABLS_API_VERSION );
    Json_node_add_string ( RootNode, "product", "ABLS-HABITAT-API" );
    Json_node_add_string ( RootNode, "vendor",  "ABLS-HABITAT" );
    Json_node_add_int    ( RootNode, "nbr_domains", g_tree_nnodes ( Global.domaines) );
    Json_node_add_string ( RootNode, "author",  "Sébastien Lefèvre" );
    Json_node_add_string ( RootNode, "docs",    "https://docs.abls-habitat.fr" );

    Http_Send_json_response( msg, RootNode );
  }
/******************************************************************************************************************************/
/* Keep_running_process: Thread principal                                                                                     */
/* Entrée: néant                                                                                                              */
/* Sortie: -1 si erreur, 0 sinon                                                                                              */
/******************************************************************************************************************************/
 gint main ( void )
  { GError *error = NULL;

    prctl(PR_SET_NAME, "W-GLOBAL-API", 0, 0, 0 );
    Info_init ( "Abls-Habitat-API", LOG_INFO );
    Info_new ( __func__, LOG_INFO, "API %s is starting", ABLS_API_VERSION );
    signal(SIGTERM, Traitement_signaux);                                               /* Activation de la réponse au signaux */
    memset ( &Global, 0, sizeof(struct GLOBAL) );
/******************************************************* Read Config file *****************************************************/
    Global.config = Json_read_from_file ( "/etc/fr-abls-habitat-api.conf" );
    if (!Global.config)
     { Info_new ( __func__, LOG_CRIT, "Unable to read Config file /etc/fr-abls-habitat-api.conf" );
       return(-1);
     }
    Json_node_add_string ( Global.config, "domain_uuid", "master" );
    if (!Json_has_member ( Global.config, "api_port"    )) Json_node_add_int    ( Global.config, "api_port", 5562 );
    if (!Json_has_member ( Global.config, "db_hostname" )) Json_node_add_string ( Global.config, "db_hostname", "localhost" );
    if (!Json_has_member ( Global.config, "db_username" )) Json_node_add_string ( Global.config, "db_username", "dbuser" );
    if (!Json_has_member ( Global.config, "db_password" )) Json_node_add_string ( Global.config, "db_password", "dbpass" );
    if (!Json_has_member ( Global.config, "db_database" )) Json_node_add_string ( Global.config, "db_database", "database" );
    if (!Json_has_member ( Global.config, "db_port"     )) Json_node_add_int    ( Global.config, "db_port", 3306 );

    Global.domaines = g_tree_new ( (GCompareFunc) strcmp );
    DOMAIN_Load ( NULL, 0, Global.config, NULL );
/******************************************************* Connect to DB ********************************************************/
    if ( DB_Connected ( "master" ) == FALSE )
     { Info_new ( __func__, LOG_CRIT, "Unable to connect to database" );
       json_node_unref(Global.config);
       return(-1);
     }

/******************************************************* Update Schema ********************************************************/
    if ( DB_Master_Update () == FALSE )
     { Info_new ( __func__, LOG_ERR, "Unable to update database" ); }

    DOMAIN_Load_all ();                                                                    /* Chargement de tous les domaines */
/********************************************************* Active le serveur HTTP/WS ******************************************/
    SoupServer *socket = soup_server_new( "server-header", "Abls-Habitat API Server", NULL );
    if (!socket)
     { Info_new ( __func__, LOG_CRIT, "Unable to start SoupServer" );
       Keep_running = FALSE;
     }

/************************************************* Declare Handlers ***********************************************************/
    soup_server_add_handler ( socket, "/status", Http_get_status, NULL, NULL );
    soup_server_add_handler ( socket, "/domains", DOMAIN_request, NULL, NULL );
    soup_server_add_handler ( socket, "/instance", INSTANCE_request, NULL, NULL );

    static gchar *protocols[] = { "live-visuels", "live-instances", NULL };
    soup_server_add_websocket_handler ( socket, "/websocket", NULL, protocols, WS_Open_CB, NULL, NULL );

    gint api_port = Json_get_int ( Global.config, "api_port" );
    if (!soup_server_listen_all (socket, api_port, 0/*SOUP_SERVER_LISTEN_HTTPS*/, &error))
     { Info_new ( __func__, LOG_CRIT, "Unable to listen to port %d: %s", api_port, error->message );
       g_error_free(error);
       Keep_running = FALSE;
     }

    Info_new ( __func__, LOG_NOTICE, "API %s started. Waiting for connexions.", ABLS_API_VERSION );

    if (Keep_running)
     { GMainLoop *loop = g_main_loop_new (NULL, TRUE);
       while( Keep_running ) { g_main_context_iteration ( g_main_loop_get_context ( loop ), TRUE ); }
       g_main_loop_unref( loop );
     }

/******************************************************* End of API ***********************************************************/
    if (socket) soup_server_disconnect ( socket );                                              /* Arret du serveur WebSocket */
    DOMAIN_Unload_all();
    json_node_unref(Global.config);
    Info_new ( __func__, LOG_INFO, "API stopped" );
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
