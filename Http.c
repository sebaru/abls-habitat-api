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
 struct HTTP_REQUEST
  { SoupServer *server;
    SoupMessage *msg;
    const char *path;
    SoupClientContext *client;
  };

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
    if (!request) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Not a Json Request", NULL ); }
    g_bytes_unref(request_brute);
    return(request);
  }
/******************************************************************************************************************************/
/* Http_Check_Agent_Signature: Vérifie qu'un message est correctement signé                                                   */
/* Entrée: le messages                                                                                                        */
/* Sortie: TRUE si OK                                                                                                         */
/******************************************************************************************************************************/
 gboolean Http_Check_Agent_signature ( gchar *path, SoupMessage *msg, struct DOMAIN **domain_p, gchar **agent_uuid_p )
  { SoupMessageHeaders *headers;
    g_object_get ( msg, "request-headers", &headers, NULL );
    if (!headers)
     { Info_new ( __func__, LOG_ERR, NULL, "%s: No headers provided. Access Denied.", path );
       soup_message_set_status ( msg, SOUP_STATUS_UNAUTHORIZED );
       return(FALSE);
     }

    gchar *origin      = soup_message_headers_get_one ( headers, "Origin" );
    if (!origin)
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, Origin Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *domain_uuid = soup_message_headers_get_one ( headers, "X-ABLS-DOMAIN" );
    if (!domain_uuid)
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, X-ABLS-DOMAIN Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *agent_uuid  = (*agent_uuid_p) = soup_message_headers_get_one ( headers, "X-ABLS-AGENT" );
    if (!agent_uuid)
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, X-ABLS-AGENT Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *timestamp = soup_message_headers_get_one ( headers, "X-ABLS-TIMESTAMP" );
    if (!timestamp)
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, X-ABLS-TIMESTAMP Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    gchar *signature   = soup_message_headers_get_one ( headers, "X-ABLS-SIGNATURE" );
    if (!signature)
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Bad Request, X-ABLS-SIGNATURE Header is missing", path );
       soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST );
       return(FALSE);
     }

    if (!strcasecmp ( domain_uuid, "master" ) )
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Forbidden", path );
       soup_message_set_status ( msg, SOUP_STATUS_FORBIDDEN );
       return(FALSE);
     }

    struct DOMAIN *domain = (*domain_p) = DOMAIN_tree_get ( domain_uuid );
    if( domain == NULL )
     { Info_new ( __func__, LOG_WARNING, domain, "'%s' -> Domain '%s' not found in tree", path, domain_uuid );
       soup_message_set_status ( msg, SOUP_STATUS_NOT_FOUND );
       return(FALSE);
     }

    GBytes *gbytes_body;
    gsize taille_body;
    g_object_get ( msg, "request-body-data", &gbytes_body, NULL );
    gchar *request_body  = g_bytes_get_data ( gbytes_body, &taille_body );
    gchar *domain_secret = Json_get_string ( domain->config, "domain_secret" );

    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, domain_uuid,   strlen(domain_uuid));
    EVP_DigestUpdate(mdctx, agent_uuid,    strlen(agent_uuid));
    EVP_DigestUpdate(mdctx, domain_secret, strlen(domain_secret));
    EVP_DigestUpdate(mdctx, request_body,  taille_body);
    EVP_DigestUpdate(mdctx, timestamp,     strlen(timestamp));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    gchar local_signature[64];
    EVP_EncodeBlock( local_signature, hash_bin, 32 ); /* 256 bits -> 32 bytes */

    gint retour = strcmp ( signature, local_signature );
    g_bytes_unref(gbytes_body);
    if (retour)
     { Info_new ( __func__, LOG_ERR, domain, "%s -> Forbidden, Wrong signature", path );
       soup_message_set_status ( msg, SOUP_STATUS_FORBIDDEN );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_json_node_create: Creer un buffer json de reponse HTTP                                                                */
/* Entrée: le msg source de la reponse                                                                                        */
/* Sortie: le buffer ou null si pb. Dans ce cas, le status est mis à jour                                                     */
/******************************************************************************************************************************/
 JsonNode *Http_json_node_create ( SoupMessage *msg )
  { JsonNode *RootNode = Json_node_create();
    if (!RootNode) soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error");
    return(RootNode);
  }
/******************************************************************************************************************************/
/* Http_Send_json_response: Envoie le json en paramètre en prenant le lead dessus                                             */
/* Entrée: le messages, le success, le details, le buffer json                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Send_json_response ( SoupMessage *msg, gint code, gchar *details, JsonNode *RootNode )
  { if (!RootNode)
     { if ( (RootNode = Http_json_node_create (msg) ) == NULL ) return; }

    if (code == 1) { code = SOUP_STATUS_OK; details = "OK"; }
    if (code == 0) { code = SOUP_STATUS_INTERNAL_SERVER_ERROR; }

    Json_node_add_int ( RootNode, "api_status", code );
    if (details)
     { if (code != SOUP_STATUS_OK) Json_node_add_string ( RootNode, "api_error", details );
                              else Json_node_add_string ( RootNode, "api_result", details );
     }

    gchar *buf = Json_node_to_string ( RootNode );
    json_node_unref ( RootNode );
    if (!buf)
     { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Send Json Memory Error");
       return;
     }
/*************************************************** Envoi au client **********************************************************/
    soup_message_set_status (msg, code );
    soup_message_set_response ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, strlen(buf) );
  }
/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                                             */
/* Entrée: numero du signal à gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  {
    if (num == SIGALRM) { Global.Top++; return; }

    switch (num)
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
  { if (!token)  return;

    gchar *email = Json_get_string ( token, "email" );
    Info_new( __func__, LOG_INFO, domain, "User '%s': path %s", email, path );
  }
/******************************************************************************************************************************/
/* Http_is_authorized: Vérifie le token et l'access level du user vis à vis du domain en parametre                            */
/* Entrées: le domain, le message, le token, l'access_level attendu                                                           */
/* Sortie: FALSE si non authorisé                                                                                             */
/******************************************************************************************************************************/
 gboolean Http_is_authorized ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, gint min_access_level )
  { if (!domain) return(FALSE);
    if (!token)  return(FALSE);

    gchar *email = Json_get_string ( token, "email" );
    gchar *iss   = Json_get_string ( token, "iss" );
    gint exp     = Json_get_int ( token, "exp" );

    if ( exp <= time(NULL) )
     { Info_new( __func__, LOG_ERR, domain, "%s: User '%s': token has expired", path, email );
       Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Token has expired", NULL );
       return(FALSE);
     }

    if ( !g_str_has_prefix ( iss, Json_get_string ( Global.config, "idp_url" ) ) )
     { Info_new( __func__, LOG_ERR, domain, "%s: User '%s': Wrong IDP Issuer (%s != %s)", path, email, iss, Json_get_string ( Global.config, "idp_url" ) );
       Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Wrong IDP Issuer", NULL );
       return(FALSE);
     }

    gboolean email_verified = Json_get_bool ( token , "email_verified" );
    if (!email_verified)
     { Info_new( __func__, LOG_ERR, domain, "%s: User '%s': Email not verified", path, email );
       Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Email not verified", NULL );
       return(FALSE);
     }

    gboolean retour = DB_Read ( DOMAIN_tree_get("master"), token, NULL,
                                "SELECT enable, access_level FROM users INNER JOIN users_grants USING (user_uuid) "
                                "WHERE domain_uuid='%s' AND user_uuid='%s'",
                                Json_get_string ( domain->config, "domain_uuid" ), Json_get_string ( token, "sub" ) );
    if (!retour) { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Not authorized", NULL ); return(FALSE); }

    if ( Json_get_bool ( token, "enable" ) == FALSE )
     { Http_Send_json_response ( msg, SOUP_STATUS_UNAUTHORIZED, "User not enabled", NULL ); return(FALSE); }

    if (!Json_has_member ( token, "access_level" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "User or domain not found", NULL ); return(FALSE); }

    if ( Json_get_int ( token, "access_level" ) >= min_access_level) return(TRUE);
    Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Opération non autorisée, vous manquez de permissions.", NULL );
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Http_fail_if_has_not: Vérifie la présence d'un champ dans la requete. Si inexistant, positionne le code retour adequat     */
/* Entrées: le domain, le message, le path, la requete, le champ a chercher                                                   */
/* Sortie: TRUE si le champ n'est pas present                                                                                 */
/******************************************************************************************************************************/
 gboolean Http_fail_if_has_not ( struct DOMAIN *domain, gchar *path, SoupMessage *msg, JsonNode *request, gchar *name )
  { if (Json_has_member ( request, name )) return(FALSE);
    gchar chaine[80];
    g_snprintf ( chaine, sizeof(chaine), "%s is missing", name );
    Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, chaine, NULL );
    Info_new ( __func__, LOG_ERR, domain, "%s: %s is missing", path, name );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_get_token: Vérifie le token et le renvoi au format JSON                                                               */
/* Entrées: le path, le msg libsoup                                                                                           */
/* Sortie: NULL si le token n'est pas valide                                                                                  */
/******************************************************************************************************************************/
 JsonNode *Http_get_token ( gchar *path, SoupMessage *msg )
  { SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(msg), "request-headers", &headers, NULL );
    if (!headers)
     { Info_new ( __func__, LOG_ERR, NULL, "%s: No headers provided. Access Denied.", path );
       Http_Send_json_response ( msg, SOUP_STATUS_UNAUTHORIZED, "You are not known", NULL );
       return(NULL);
     }

    gchar *token_char = soup_message_headers_get_one ( headers, "Authorization" );
    if (!token_char)
     { Info_new ( __func__, LOG_ERR, NULL, "%s: No token provided. Access Denied.", path );
       Http_Send_json_response ( msg, SOUP_STATUS_UNAUTHORIZED, "You are not known", NULL );
       return(NULL);
     }

    if (!g_str_has_prefix ( token_char, "Bearer "))
     { Info_new ( __func__, LOG_ERR, NULL, "%s: Token is not Bearer. Access Denied.", path );
       Http_Send_json_response ( msg, SOUP_STATUS_UNAUTHORIZED, "You are not known", NULL );
       return(NULL);
     }
    token_char = token_char + 7; /* swap 'Bearer ' */

    gchar *key = Json_get_string ( Global.config, "idp_public_key" );
    jwt_t *token;
    if ( jwt_decode ( &token, token_char, key, strlen(key) ) )
     { Info_new ( __func__, LOG_ERR, NULL, "%s: Token decode error: %s.", path, g_strerror(errno) );
       Http_Send_json_response ( msg, SOUP_STATUS_UNAUTHORIZED, "You are not known", NULL );
       return(NULL);
     }

    gchar *RootNode_char = jwt_get_grants_json	( token, NULL );                                 /* Convert from token to Json */
    jwt_free (token);
    JsonNode *RootNode = Json_get_from_string ( RootNode_char );
    g_free(RootNode_char);
    return(RootNode);
  }
/******************************************************************************************************************************/
/* PING_request_post: repond à une requete ping                                                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PING_request_post ( JsonNode *token, SoupMessage *msg, JsonNode *request )
  {
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    Json_node_add_string ( RootNode, "result", "PONG" );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* HTTP_Handle_request: Repond aux requests reçues                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void HTTP_Handle_request ( struct HTTP_REQUEST *http_request )
  {
    SoupServer *server = http_request->server;
    SoupMessage *msg   = http_request->msg;
    const char  *path  = http_request->path;
    SoupClientContext *client = http_request->client;
    g_free(http_request);

    SoupMessageHeaders *headers;
    g_object_get ( G_OBJECT(msg), SOUP_MESSAGE_RESPONSE_HEADERS, &headers, NULL );
    soup_message_headers_append ( headers, "Access-Control-Allow-Origin", Json_get_string ( Global.config, "Access-Control-Allow-Origin" ) );
    soup_message_headers_append ( headers, "Access-Control-Allow-Methods", "*" );
    soup_message_headers_append ( headers, "Access-Control-Allow-Headers", "content-type, authorization" );
    soup_message_headers_append ( headers, "Cache-Control", "no-store, must-revalidate" );

/*---------------------------------------------------- OPTIONS ---------------------------------------------------------------*/
    if (msg->method == SOUP_METHOD_OPTIONS)
     { soup_message_headers_append ( headers, "Access-Control-Max-Age", "86400" );
       Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
       goto end_request;
     }
/*------------------------------------------------------ GET -----------------------------------------------------------------*/
    if (msg->method == SOUP_METHOD_GET)
     {      if (!strcasecmp ( path, "/status" )) STATUS_request_get ( server, msg, path );
       else if (!strcasecmp ( path, "/icons" ))  ICONS_request_get ( server, msg, path );
/*------------------------------------------------------ GET WEBSOCKET -------------------------------------------------------*/
       else if (!strcasecmp ( path, "/websocket" ))
        { struct DOMAIN *domain;
          gchar *agent_uuid;

          if (!Http_Check_Agent_signature ( path, msg, &domain, &agent_uuid )) return;
          gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

          if (!soup_websocket_server_check_handshake ( msg, "abls-habitat.fr", NULL, NULL ))
           { soup_message_set_status ( msg, SOUP_STATUS_BAD_REQUEST ); return; }

          struct WS_AGENT_SESSION *ws_agent = g_try_malloc0( sizeof(struct WS_AGENT_SESSION) );
          if(!ws_agent)
           { Info_new( __func__, LOG_ERR, domain, "%s: WebSocket Memory error. Closing '%s'/'%s' !", path, domain_uuid, agent_uuid );
             Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory error", NULL );
             goto end_request;
           }
          ws_agent->context = client;
          ws_agent->domain  = domain;
          g_snprintf( ws_agent->agent_uuid, sizeof(ws_agent->agent_uuid), "%s", agent_uuid );
          pthread_mutex_lock ( &domain->synchro );
          domain->ws_agents = g_slist_append ( domain->ws_agents, ws_agent );
          pthread_mutex_unlock ( &domain->synchro );

          Info_new ( __func__, LOG_NOTICE, domain,
                    "%s: Websocket Access Granted to domain '%s', agent '%s'", path, domain_uuid, agent_uuid );

          soup_websocket_server_process_handshake ( msg, "abls-habitat.fr", NULL );
          g_signal_connect ( msg, "wrote-informational", G_CALLBACK(WS_Agent_Open_CB), ws_agent );
          goto end_request;
        }
       else
        { Info_new ( __func__, LOG_WARNING, NULL, "GET %s -> not found", path );
          Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "URI not found", NULL );
        }
       goto end_request;
     }
/*------------------------------------------------------ POST ----------------------------------------------------------------*/
    if (msg->method != SOUP_METHOD_POST && msg->method != SOUP_METHOD_DELETE)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s %s -> not implemented", msg->method, path );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_IMPLEMENTED, "Methode non implémentée", NULL );
       goto end_request;
     }

/*------------------------------------------------ Requetes des agents -------------------------------------------------------*/
    if (msg->method == SOUP_METHOD_POST && g_str_has_prefix ( path, "/run/" ))
     { struct DOMAIN *domain;
       gchar *agent_uuid;
       if (!Http_Check_Agent_signature ( path, msg, &domain, &agent_uuid )) return;

       JsonNode *request = Http_Msg_to_Json ( msg );
       if (!request) goto end_request;

       Info_new ( __func__, LOG_INFO, domain, "%s requested by agent '%s'", path, agent_uuid );

            if (!strcasecmp ( path, "/run/agent/start"       )) RUN_AGENT_START_request_post ( domain, path, agent_uuid, msg, request );
       else if (!strcasecmp ( path, "/run/visuels/set"       )) RUN_VISUELS_SET_request_post ( domain, path, agent_uuid, msg, request );
       else if (!strcasecmp ( path, "/run/archive/save"      )) RUN_ARCHIVE_SAVE_request_post ( domain, path, agent_uuid, msg, request );
       else if (!strcasecmp ( path, "/run/mapping/list"      )) RUN_MAPPING_LIST_request_post ( domain, path, agent_uuid, msg, request );
       else if (!strcasecmp ( path, "/run/thread/load"       )) RUN_THREAD_LOAD_request_post ( domain, path, agent_uuid, msg, request );
       else if (!strcasecmp ( path, "/run/thread/add_io"     )) RUN_THREAD_ADD_IO_request_post ( domain, path, agent_uuid, msg, request );
       else if (!strcasecmp ( path, "/run/thread/get_config" )) RUN_THREAD_GET_CONFIG_request_post ( domain, path, agent_uuid, msg, request );
       else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "URI not found", NULL );
       json_node_unref(request);
       goto end_request;
     }
/*--------------------------------------------- Requetes des browsers --------------------------------------------------------*/
    JsonNode *request = Http_Msg_to_Json ( msg );
    if (!request) goto end_request;

/*------------------------------------------ Recupération du token IDP -------------------------------------------------------*/
    JsonNode *token = Http_get_token ( path, msg );                                             /* Récupération du token user */
    if (!token)
     { json_node_unref(request);
       goto end_request;
     }

/*---------------------------------- Requetes authentifiées des users (hors domaine) -----------------------------------------*/
    if (msg->method == SOUP_METHOD_POST)
     {      if (!strcasecmp ( path, "/user/profil" ))      { USER_PROFIL_request_post ( token, msg, request );  goto end_post; }
       else if (!strcasecmp ( path, "/ping" ))             { PING_request_post ( token, msg, request );         goto end_post; }
       else if (!strcasecmp ( path, "/domain/list" ))      { DOMAIN_LIST_request_post ( token, path, msg, request );      goto end_post; }
       else if (!strcasecmp ( path, "/user/set_domain" ))  { USER_SET_DOMAIN_request_post  ( token, path, msg, request ); goto end_post; }
     }

/*------------------------------------------------ Requetes dans le domaine --------------------------------------------------*/
    if (Http_fail_if_has_not ( NULL, path, msg, request, "domain_uuid")) goto end_post;

    gchar *domain_uuid   = Json_get_string ( request, "domain_uuid" );
    if (!strcasecmp ( domain_uuid, "master" ) )
     { Info_new ( __func__, LOG_ERR, NULL, "'%s' -> Forbidden", path );
       Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Domain master forbidden", NULL );
       goto end_post;
     }
    struct DOMAIN *domain = DOMAIN_tree_get ( domain_uuid );                                   /* Quel domaine de requetage ? */
/*--------------------------------------------- Requetes des users (dans un domaine) -----------------------------------------*/
    if( domain == NULL )
     { Info_new ( __func__, LOG_WARNING, domain, "'%s' -> Domain '%s' not found in tree", path, domain_uuid );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Domain not found", NULL );
       goto end_post;
     }

    if (msg->method == SOUP_METHOD_POST)
     {      if (!strcasecmp ( path, "/domain/status" ))    DOMAIN_STATUS_request_post    ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/domain/image" ))     DOMAIN_IMAGE_request_post     ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/domain/get" ))       DOMAIN_GET_request_post       ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/domain/set" ))       DOMAIN_SET_request_post       ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/domain/set_image" )) DOMAIN_SET_IMAGE_request_post ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/domain/transfer" ))  DOMAIN_TRANSFER_request_post  ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/domain/add" ))       DOMAIN_ADD_request_post       ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/user/list" ))        USER_LIST_request_post        ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/user/invite" ))      USER_INVITE_request_post      ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/archive/status" ))   ARCHIVE_STATUS_request_post   ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/archive/set" ))      ARCHIVE_SET_request_post      ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/modbus/list" ))      MODBUS_LIST_request_post      ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/modbus/set" ))       MODBUS_SET_request_post       ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/imsgs/set" ))        IMSGS_SET_request_post        ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/smsg/set" ))         SMSG_SET_request_post         ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/audio/set" ))        AUDIO_SET_request_post        ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/agent/list" ))       AGENT_LIST_request_post       ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/agent/set" ))        AGENT_SET_request_post        ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/agent/set_master" )) AGENT_SET_MASTER_request_post ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/agent/reset" ))      AGENT_RESET_request_post      ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/agent/upgrade" ))    AGENT_UPGRADE_request_post    ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/mnemos/tech_ids" ))  MNEMOS_TECH_IDS_request_post  ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/thread/list" ))      THREAD_LIST_request_post      ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/thread/enable" ))    THREAD_ENABLE_request_post    ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/thread/send" ))      THREAD_SEND_request_post      ( domain, token, path, msg, request );
       else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Path not found", NULL );
     }
    else if (msg->method == SOUP_METHOD_DELETE)
     {      if (!strcasecmp ( path, "/domain/delete" ))    DOMAIN_DELETE_request         ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/thread/delete" ))    THREAD_DELETE_request         ( domain, token, path, msg, request );
       else if (!strcasecmp ( path, "/archive/delete" ))   ARCHIVE_DELETE_request        ( domain, token, path, msg, request );
       else Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Path not found", NULL );
     }

end_post:
    json_node_unref(token);
    json_node_unref(request);

end_request:
    pthread_mutex_lock( &Global.nbr_threads_sync );
    Global.nbr_threads--;
    pthread_mutex_unlock( &Global.nbr_threads_sync );
    soup_server_unpause_message ( server, msg );
  }
/******************************************************************************************************************************/
/* HTTP_Handle_request: Repond aux requests reçues                                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void HTTP_Handle_request_CB ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                      SoupClientContext *client, gpointer user_data )
  { pthread_t TID;

    pthread_mutex_lock( &Global.nbr_threads_sync );
    Global.nbr_threads++;
    pthread_mutex_unlock( &Global.nbr_threads_sync );

    struct HTTP_REQUEST *request = g_try_malloc( sizeof(struct HTTP_REQUEST) );
    if (!request) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Not enough memory", NULL ); return; }
    request->server = server;
    request->msg    = msg;
    request->path   = path;
    request->client = client;
    soup_server_pause_message ( server, msg );
    pthread_create( &TID, NULL, (void *)HTTP_Handle_request, request );
    pthread_detach( TID );                                           /* On le detache pour qu'il puisse se terminer tout seul */
  }
/******************************************************************************************************************************/
/* main: Fonction principale de l'API                                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: -1 si erreur, 0 sinon                                                                                              */
/******************************************************************************************************************************/
 gint main ( void )
  { struct itimerval timer;
    GError *error = NULL;

    prctl(PR_SET_NAME, "W-GLOBAL-API", 0, 0, 0 );
    Info_init ( "Abls-Habitat-API", LOG_INFO );
    Info_new ( __func__, LOG_INFO, NULL, "API %s is starting", ABLS_API_VERSION );
    memset ( &Global, 0, sizeof(struct GLOBAL) );
    pthread_mutex_init( &Global.nbr_threads_sync, NULL );
    signal(SIGTERM, Traitement_signaux);                                               /* Activation de la réponse au signaux */
    signal(SIGALRM, Traitement_signaux);                                               /* Activation de la réponse au signaux */
    timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;                                       /* Tous les 100 millisecondes */
    timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                                    /* = 10 fois par secondes */
    setitimer( ITIMER_REAL, &timer, NULL );                                                                /* Active le timer */
/******************************************************* Read Config file *****************************************************/
    Global.config = Json_read_from_file ( "/etc/abls-habitat-api.conf" );
    if (!Global.config)
     { Info_new ( __func__, LOG_CRIT, NULL, "Unable to read Config file /etc/abls-habitat-api.conf" );
       return(-1);
     }
    Json_node_add_string ( Global.config, "domain_uuid", "master" );
    if (!Json_has_member ( Global.config, "Access-Control-Allow-Origin" )) Json_node_add_string ( Global.config, "Access-Control-Allow-Origin", "*" );
    if (!Json_has_member ( Global.config, "api_port"    )) Json_node_add_int    ( Global.config, "api_port", 5562 );
    if (!Json_has_member ( Global.config, "idp_url"     )) Json_node_add_string ( Global.config, "idp_url", "https://idp.abls-habitat.fr" );
    if (!Json_has_member ( Global.config, "idp_realm"   )) Json_node_add_string ( Global.config, "idp_realm", "abls-habitat" );

    if (!Json_has_member ( Global.config, "db_hostname" )) Json_node_add_string ( Global.config, "db_hostname", "localhost" );
    if (!Json_has_member ( Global.config, "db_password" )) Json_node_add_string ( Global.config, "db_password", "changeme" );
    if (!Json_has_member ( Global.config, "db_port"     )) Json_node_add_int    ( Global.config, "db_port", 3306 );

    if (!Json_has_member ( Global.config, "db_arch_hostname" ))
     { Json_node_add_string ( Global.config, "db_arch_hostname", Json_get_string ( Global.config, "db_hostname" ) ); }
    if (!Json_has_member ( Global.config, "db_arch_port"     ))
     { Json_node_add_int    ( Global.config, "db_arch_port", Json_get_int    ( Global.config, "db_port" ) ); }

/****************************************** Récupération de la clef public de l'IDP *******************************************/
    gchar idp_query[256];
    g_snprintf( idp_query, sizeof(idp_query), "%s/realms/%s", Json_get_string ( Global.config, "idp_url" ),
                                                              Json_get_string ( Global.config, "idp_realm" ) );
    SoupSession *idp      = soup_session_new();
    SoupMessage *soup_msg = soup_message_new ( "GET", idp_query );
    soup_session_send_message (idp, soup_msg); /* SYNC */
    gint status_code;
    g_object_get ( soup_msg, "status-code", &status_code, NULL );

    if (status_code==200)
     { GBytes *reponse_brute;
       gsize taille;
       g_object_get ( soup_msg, "response-body-data", &reponse_brute, NULL );
       JsonNode *reponse = Json_get_from_string ( g_bytes_get_data ( reponse_brute, &taille ) );
       gchar *pem_key = g_strconcat ("-----BEGIN PUBLIC KEY-----\n", Json_get_string ( reponse, "public_key" ), "\n-----END PUBLIC KEY-----\n", NULL);
       Json_node_add_string ( Global.config, "idp_public_key", pem_key );
       g_free(pem_key);
       Info_new( __func__, LOG_NOTICE, NULL, "IDP PUBLIC KEY loaded from %s: %s", idp_query, Json_get_string ( Global.config, "idp_public_key" ) );
       json_node_unref ( reponse );
     }
    else Info_new( __func__, LOG_CRIT, NULL, "Unable to retrieve IDP PUBLIC KEY on %s", idp_query );
    g_object_unref( soup_msg );
    soup_session_abort ( idp );
    g_object_unref( idp );

/*--------------------------------------------- Chargement du domaine Master -------------------------------------------------*/
    Global.domaines = g_tree_new ( (GCompareFunc) strcmp );
    DOMAIN_Load ( NULL, 0, Global.config, NULL );

/******************************************************* Connect to DB ********************************************************/
    struct DOMAIN *master = DOMAIN_tree_get ( "master" );
    if ( master == NULL )
     { Info_new ( __func__, LOG_CRIT, NULL, "Master is not loaded" );
       DOMAIN_Unload_all();
       json_node_unref(Global.config);
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
    soup_server_add_handler ( socket, "/", HTTP_Handle_request_CB, NULL, NULL );
 /*   static gchar *protocols[] = { "live-visuel", "live-agent", NULL };
    soup_server_add_websocket_handler ( socket, "/websocket", NULL, protocols, WS_Open_CB, NULL, NULL );*/

    gint api_port = Json_get_int ( Global.config, "api_port" );
    if (!soup_server_listen_all (socket, api_port, 0/*SOUP_SERVER_LISTEN_HTTPS*/, &error))
     { Info_new ( __func__, LOG_CRIT, NULL, "Unable to listen to port %d: %s", api_port, error->message );
       g_error_free(error);
       Keep_running = FALSE;
     }

    Info_new ( __func__, LOG_NOTICE, NULL, "API %s started. Waiting for connexions.", ABLS_API_VERSION );

    GMainLoop *loop = g_main_loop_new (NULL, TRUE);
    gint last_top_day = 0;
    while( Keep_running )
     { g_main_context_iteration ( g_main_loop_get_context ( loop ), TRUE );
       if (last_top_day + 864000 <= Global.Top)
        { g_tree_foreach ( Global.domaines, ARCHIVE_Delete_old_data, NULL );
          last_top_day = Global.Top;
        }
     }
    g_main_loop_unref( loop );

/******************************************************* End of API ***********************************************************/
    if (socket)                                                                                 /* Arret du serveur WebSocket */
     { soup_server_disconnect ( socket );
       g_object_unref ( socket );
     }
    Info_new ( __func__, LOG_INFO, NULL, "Waiting for all requests to be handled before unload domains." );
    while ( Global.nbr_threads != 0 ) sleep(1);
    DOMAIN_Unload_all();
    json_node_unref(Global.config);
    pthread_mutex_destroy( &Global.nbr_threads_sync );
    Info_new ( __func__, LOG_INFO, NULL, "API stopped" );
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
