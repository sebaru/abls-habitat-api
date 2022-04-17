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
    if (!request) { soup_message_set_status_full (msg, SOUP_STATUS_BAD_REQUEST, "Not a JSON request"); }
    g_bytes_unref(request_brute);
    return(request);
  }
/******************************************************************************************************************************/
/* Http_Send_json_response: Envoie le json en paramètre en prenant le lead dessus                                             */
/* Entrée: le messages, le buffer json                                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Send_json_response ( SoupMessage *msg, gchar *status, JsonNode *RootNode )
  { if (!RootNode) RootNode = Json_node_create();
    Json_node_add_string ( RootNode, "status", status );
    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
    if (!buf)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Send Json Memory Error");
       return;
     }
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
       case SIGINT:  Info_new( __func__, LOG_INFO, NULL, "Recu SIGINT" );
                     Keep_running = FALSE;                                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGTERM: Info_new( __func__, LOG_INFO, NULL, "Recu SIGTERM" );
                     Keep_running = FALSE;                                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGABRT: Info_new( __func__, LOG_INFO, NULL, "Recu SIGABRT" );
                     break;
       case SIGCHLD: Info_new( __func__, LOG_INFO, NULL, "Recu SIGCHLD" );
                     break;
       case SIGPIPE: Info_new( __func__, LOG_INFO, NULL, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info_new( __func__, LOG_INFO, NULL, "Recu SIGBUS" ); break;
       case SIGIO:   Info_new( __func__, LOG_INFO, NULL, "Recu SIGIO" ); break;
       case SIGUSR1: Info_new( __func__, LOG_INFO, NULL, "Recu SIGUSR1" );
                     break;
       case SIGUSR2: Info_new( __func__, LOG_INFO, NULL, "Recu SIGUSR2" );
                     break;
       default: Info_new( __func__, LOG_NOTICE, NULL, "Recu signal %d", num ); break;
     }
  }
/******************************************************************************************************************************/
/* Http_print_request: affiche une requete authentifiée                                                                       */
/* Entrées: le domain, le token, le path                                                                                      */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_print_request ( struct DOMAIN *domain, JsonNode *token, gchar *path )
  { if (!domain) return;
    if (!token)  return;

    gchar *email = Json_get_string ( token, "email" );
    Info_new( __func__, LOG_INFO, domain, "User '%s': path %s", email, path );
  }
/******************************************************************************************************************************/
/* Http_get_token: Vérifie le token et le renvoi au format JSON                                                               */
/* Entrées: le domain, le message                                                                                             */
/* Sortie: FALSE si non authorisé                                                                                             */
/******************************************************************************************************************************/
 gboolean Http_is_authorized ( struct DOMAIN *domain, SoupMessage *msg, JsonNode *token, gint access_level )
  { gboolean retour = FALSE;
    if (!domain) return(FALSE);
    if (!token)  return(FALSE);

    gchar *email = Json_get_string ( token, "email" );
    gint exp     = Json_get_int ( token, "exp" );
    gint iat     = Json_get_int ( token, "iat" );

    if ( exp + iat < time(NULL) )
     { Info_new( __func__, LOG_ERR, domain, "User '%s': token is expired", email );
       soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN );
       return(FALSE);
     }

    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    GList *domain_list = json_array_get_elements ( Json_get_array ( token, "grants" ) );
    while (domain_list)
     { JsonNode *domain_to_check = domain_list->data;
       gchar *check_uuid         = Json_get_string ( domain_to_check, "domain_uuid" );
       gint   check_access_level = Json_get_int    ( domain_to_check, "access_level" );
       if (!strcmp (check_uuid, domain_uuid))
        { if ( check_access_level >= access_level ) { retour = TRUE; break; } }
       domain_list = g_list_next(domain_list);
     }
    g_list_free(domain_list);
    if (retour == FALSE) soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN );
    return(retour);
  }
/******************************************************************************************************************************/
/* Http_get_token: Vérifie le token et le renvoi au format JSON                                                               */
/* Entrées: le domain, le message                                                                                             */
/* Sortie: NULL si le token n'est pas valide                                                                                  */
/******************************************************************************************************************************/
 JsonNode *Http_get_token ( struct DOMAIN *domain, SoupMessage *msg )
  { SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(msg), "request-headers", &headers, NULL );
    if (!headers)
     { Info_new ( __func__, LOG_ERR, domain, "No headers provided. Access Denied." );
       soup_message_set_status ( msg, SOUP_STATUS_UNAUTHORIZED );
       return(NULL);
     }

    gchar *token_char = soup_message_headers_get_one ( headers, "Authorization" );
    if (!token_char)
     { Info_new ( __func__, LOG_ERR, domain, "No token provided. Access Denied." );
       soup_message_set_status ( msg, SOUP_STATUS_UNAUTHORIZED );
       return(NULL);
     }

    if (!g_str_has_prefix ( token_char, "Bearer "))
     { Info_new ( __func__, LOG_ERR, domain, "Token is not Bearer. Access Denied." );
       soup_message_set_status ( msg, SOUP_STATUS_UNAUTHORIZED );
       return(NULL);
     }
    token_char = token_char + 7; /* swap 'Bearer ' */

    gchar *key = Json_get_string ( Global.config, "JWT_SECRET_KEY" );
    jwt_t *token;
    if ( jwt_decode ( &token, token_char, key, strlen(key) ) )
     { Info_new ( __func__, LOG_ERR, domain, "Token decode error : %s.", g_strerror(errno) );
       soup_message_set_status ( msg, SOUP_STATUS_UNAUTHORIZED );
       return(NULL);
     }

    gchar *RootNode_char = jwt_get_grants_json	( token, NULL );                                 /* Convert from token to Json */
    jwt_free (token);
    JsonNode *RootNode = Json_get_from_string ( RootNode_char );
    g_free(RootNode_char);
    return(RootNode);
  }
/******************************************************************************************************************************/
/* HTTP_Handle_request: Repond aux requests reçues                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HTTP_Handle_request ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  {
    SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(msg), SOUP_MESSAGE_RESPONSE_HEADERS, &headers, NULL );
    soup_message_headers_append ( headers, "Access-Control-Allow-Origin", Json_get_string ( Global.config, "Access-Control-Allow-Origin" ) );
    soup_message_headers_append ( headers, "Access-Control-Allow-Methods", "*" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Headers", "content-type, authorization" );

/*---------------------------------------------------- OPTIONS ---------------------------------------------------------------*/
    if (msg->method == SOUP_METHOD_OPTIONS)
     { soup_message_set_status ( msg, SOUP_STATUS_OK );
       return;
     }
/*------------------------------------------------------ GET -----------------------------------------------------------------*/
    else if (msg->method == SOUP_METHOD_GET)
     {      if (!strcasecmp ( path, "/status" )) STATUS_request_get ( server, msg, path, query, client, user_data );
       else if (!strcasecmp ( path, "/icons" ))  ICONS_request_get ( server, msg, path, query, client, user_data );
       else
        { Info_new ( __func__, LOG_WARNING, NULL, "GET %s -> not found", path );
          soup_message_set_status ( msg, SOUP_STATUS_NOT_FOUND );
        }
    /*soup_server_add_handler ( socket, "/domains", DOMAIN_request, NULL, NULL );*/
       return;
     }
/*------------------------------------------------------ POST ----------------------------------------------------------------*/
    else if (msg->method == SOUP_METHOD_POST)
     { JsonNode *request = Http_Msg_to_Json ( msg );
       if (!request)
        { Info_new ( __func__, LOG_WARNING, NULL, "POST %s -> Request is empty. Bad request.", path );
          soup_message_set_status_full ( msg, SOUP_STATUS_BAD_REQUEST, "Not a JSON request" );
          return;
        }
/*------------------------------------------------ Requetes des users --------------------------------------------------------*/
       if (!strcasecmp ( path, "/user/register" ))
        { USER_REGISTER_request_post ( msg, request );
          goto end_post;
        }
       else if (!strcasecmp ( path, "/user/add" ))
        { USER_ADD_request_post ( msg, request );
          goto end_post;
        }
/*------------------------------------------------ Requetes des browsers -----------------------------------------------------*/
       if (!Json_has_member ( __func__, request, "domain_uuid"))
        { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, domain_uuid is missing", path );
          soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
          goto end_post;
        }

       gchar *domain_uuid   = Json_get_string ( request, "domain_uuid" );
       if (!strcasecmp ( domain_uuid, "master" ) )
        { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Forbidden", path );
          soup_message_set_status ( msg, SOUP_STATUS_FORBIDDEN );
          goto end_post;
        }

       struct DOMAIN *domain = DOMAIN_tree_get ( domain_uuid );
       if( domain == NULL )
        { Info_new ( __func__, LOG_WARNING, domain, "'%s' -> Domain '%s' not found in tree", path, domain_uuid );
          soup_message_set_status ( msg, SOUP_STATUS_NOT_FOUND );
          goto end_post;
        }

       if (DB_Connected(domain)==FALSE)
        { Info_new ( __func__, LOG_WARNING, domain, "'%s' -> Domain not connected", path );
          soup_message_set_status ( msg, SOUP_STATUS_NOT_FOUND );
          goto end_post;
        }

       if (!strcasecmp ( path, "/domain/status" )) { DOMAIN_STATUS_request_post ( domain, msg, request ); goto end_post; }

/*------------------------------------------------ Requetes des agents -------------------------------------------------------*/
       if (!Json_has_member ( __func__, request, "instance_uuid" ))
        { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, instance_uuid is missing", path );
          soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
          goto end_post;
        }

       if (!Json_has_member ( __func__, request, "api_tag" ))
        { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, api_tag is missing", path );
          soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
          goto end_post;
        }

       gchar *instance_uuid = Json_get_string ( request, "instance_uuid" );
       gchar *api_tag       = Json_get_string ( request, "api_tag" );

       Info_new ( __func__, LOG_INFO, domain, "'%s', instance '%s', tag '%s'", path, instance_uuid, api_tag );

            if (!strcasecmp ( path, "/instance"   )) INSTANCE_request_post ( domain, instance_uuid, api_tag, msg, request );
       else if (!strcasecmp ( path, "/visuels"    )) VISUELS_request_post ( domain, instance_uuid, api_tag, msg, request );
       else if (!strcasecmp ( path, "/subprocess" )) SUBPROCESS_request_post ( domain, instance_uuid, api_tag, msg, request );
       else soup_message_set_status ( msg, SOUP_STATUS_NOT_FOUND );
end_post:
       json_node_unref(request);
       return;
     }
    Info_new ( __func__, LOG_WARNING, NULL, "%s %s -> not implemented", msg->method, path );
    soup_message_set_status ( msg, SOUP_STATUS_NOT_IMPLEMENTED );
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
    Info_new ( __func__, LOG_INFO, NULL, "API %s is starting", ABLS_API_VERSION );
    signal(SIGTERM, Traitement_signaux);                                               /* Activation de la réponse au signaux */
    memset ( &Global, 0, sizeof(struct GLOBAL) );
/******************************************************* Read Config file *****************************************************/
    Global.config = Json_read_from_file ( "/etc/abls-habitat-api.conf" );
    if (!Global.config)
     { Info_new ( __func__, LOG_CRIT, NULL, "Unable to read Config file /etc/abls-habitat-api.conf" );
       return(-1);
     }
    Json_node_add_string ( Global.config, "domain_uuid", "master" );
    if (!Json_has_member ( __func__, Global.config, "Access-Control-Allow-Origin" )) Json_node_add_string ( Global.config, "Access-Control-Allow-Origin", "*" );
    if (!Json_has_member ( __func__, Global.config, "api_port"    )) Json_node_add_int    ( Global.config, "api_port", 5562 );
    if (!Json_has_member ( __func__, Global.config, "db_hostname" )) Json_node_add_string ( Global.config, "db_hostname", "localhost" );
    if (!Json_has_member ( __func__, Global.config, "db_username" )) Json_node_add_string ( Global.config, "db_username", "dbuser" );
    if (!Json_has_member ( __func__, Global.config, "db_password" )) Json_node_add_string ( Global.config, "db_password", "dbpass" );
    if (!Json_has_member ( __func__, Global.config, "db_database" )) Json_node_add_string ( Global.config, "db_database", "database" );
    if (!Json_has_member ( __func__, Global.config, "db_port"     )) Json_node_add_int    ( Global.config, "db_port", 3306 );
    if (!Json_has_member ( __func__, Global.config, "JWT_ALG"     )) Json_node_add_string ( Global.config, "JWT_ALG", "HS256" );
    if (!Json_has_member ( __func__, Global.config, "JWT_SECRET_KEY" )) Json_node_add_string ( Global.config, "JWT_SECRET_KEY", "has-to-be-changed-now-!" );
    if (!Json_has_member ( __func__, Global.config, "JWT_PUBLIC_KEY" )) Json_node_add_string ( Global.config, "JWT_PUBLIC_KEY", "has-to-be-changed-now-!" );

    Global.domaines = g_tree_new ( (GCompareFunc) strcmp );
    DOMAIN_Load ( NULL, 0, Global.config, NULL );
/******************************************************* Connect to DB ********************************************************/
    struct DOMAIN *master = DOMAIN_tree_get ( "master" );
    if ( master == NULL )
     { Info_new ( __func__, LOG_CRIT, NULL, "Master is not loaded" );
       json_node_unref(Global.config);
       DOMAIN_Unload_all();
       return(-1);
     }

    if ( DB_Connected ( master ) == FALSE )
     { Info_new ( __func__, LOG_CRIT, NULL, "Unable to connect to database" );
       json_node_unref(Global.config);
       DOMAIN_Unload_all();
       return(-1);
     }

/******************************************************* Update Schema ********************************************************/
    if ( DB_Master_Update () == FALSE )
     { Info_new ( __func__, LOG_ERR, NULL, "Unable to update database" ); }

    DOMAIN_Load_all ();                                                                    /* Chargement de tous les domaines */
/********************************************************* Active le serveur HTTP/WS ******************************************/
    SoupServer *socket = soup_server_new( "server-header", "Abls-Habitat API Server", NULL );
    if (!socket)
     { Info_new ( __func__, LOG_CRIT, NULL, "Unable to start SoupServer" );
       Keep_running = FALSE;
     }

/************************************************* Declare Handlers ***********************************************************/
    soup_server_add_handler ( socket, "/", HTTP_Handle_request, NULL, NULL );

    static gchar *protocols[] = { "live-visuels", "live-instances", NULL };
    soup_server_add_websocket_handler ( socket, "/websocket", NULL, protocols, WS_Open_CB, NULL, NULL );

    gint api_port = Json_get_int ( Global.config, "api_port" );
    if (!soup_server_listen_all (socket, api_port, 0/*SOUP_SERVER_LISTEN_HTTPS*/, &error))
     { Info_new ( __func__, LOG_CRIT, NULL, "Unable to listen to port %d: %s", api_port, error->message );
       g_error_free(error);
       Keep_running = FALSE;
     }

    Info_new ( __func__, LOG_NOTICE, NULL, "API %s started. Waiting for connexions.", ABLS_API_VERSION );

    GMainLoop *loop = g_main_loop_new (NULL, TRUE);
    while( Keep_running ) { g_main_context_iteration ( g_main_loop_get_context ( loop ), TRUE ); }
    g_main_loop_unref( loop );

/******************************************************* End of API ***********************************************************/
    if (socket)                                                                                 /* Arret du serveur WebSocket */
     { soup_server_disconnect ( socket );
       g_object_unref ( socket );
     }
    DOMAIN_Unload_all();
    json_node_unref(Global.config);
    Info_new ( __func__, LOG_INFO, NULL, "API stopped" );
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
