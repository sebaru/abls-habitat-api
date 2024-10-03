/******************************************************************************************************************************/
/* users.c                      Gestion des users dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                09.04.2022 21:33:35 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * users.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
/* User_handle_one_invite: Enregistre une invitation dans le domaine                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void User_handle_one_invite ( JsonArray *array, guint index_, JsonNode *invite, gpointer user_data )
  { /*JsonNode *token = user_data;*/
    struct DOMAIN *master = DOMAIN_tree_get ("master");
    DB_Write ( master, "INSERT INTO users_grants SET user_uuid=(SELECT user_uuid FROM users WHERE email LIKE '%s'), "
                       " domain_uuid='%s', access_level='%d' ON DUPLICATE KEY UPDATE user_uuid=user_uuid",
               Json_get_string ( invite, "email" ), Json_get_string ( invite, "domain_uuid" ), Json_get_int ( invite, "access_level" ) );
  }
/******************************************************************************************************************************/
/* USER_PROFIL_request_get: renvoi le profil utilisateur vis à vis du token recu                                              */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : Le JWT est mis a jour                                                                                             */
/******************************************************************************************************************************/
 void USER_PROFIL_request_get ( JsonNode *token, SoupServerMessage *msg )
  {
    Http_print_request ( NULL, token, "/user/profil" );

    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "name")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "preferred_username")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "given_name")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "family_name")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "email")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "email_verified")) return;

    struct DOMAIN *master = DOMAIN_tree_get ("master");
    JsonNode *RootNode = Http_json_node_create(msg);
    if (!RootNode) return;

    gchar *username  = Normaliser_chaine ( Json_get_string ( token , "preferred_username" ) );
    gchar *email     = Normaliser_chaine ( Json_get_string ( token , "email" ) );
    gchar *user_uuid = Normaliser_chaine ( Json_get_string ( token , "sub" ) );

    gboolean retour = DB_Read ( master, RootNode, NULL,
                               "SELECT u.user_uuid FROM users AS u "
                               "WHERE email='%s' OR username='%s' LIMIT 1", email, username );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); goto end_user; }

    if (!Json_has_member ( RootNode, "user_uuid" ))
     { Info_new ( __func__, LOG_NOTICE, NULL, "First request of a new user '%s'. Creating entry in database", email );
       gchar *user_uuid = Json_get_string ( token, "sub" );
       retour = DB_Write ( master, "INSERT INTO users SET user_uuid='%s', email='%s', username='%s',enable=1", user_uuid, email, username );
       if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); goto end_user; }
       gchar body[2048];
       g_snprintf ( body, sizeof(body), "Bonjour %s, <br>Votre compte a été créé. <br>Cliquez sur le lien ci dessous pour accéder à ABLS-Habitat.", Json_get_string ( token , "name" ) );
       Send_mail ( "Votre compte a été créé.", Json_get_string ( token , "email" ), body );
     }

    retour =  DB_Read ( master, RootNode, "invites", "SELECT * FROM users_invite WHERE email='%s'", email );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); goto end_user; }
    Json_node_foreach_array_element ( RootNode, "invites", User_handle_one_invite, token );
    DB_Write ( master, "DELETE FROM users_invite WHERE email ='%s'", email );

    retour = DB_Read ( master, RootNode, NULL,
                       "SELECT u.user_uuid,u.email,u.username,u.enable, "
                       "u.default_domain_uuid, d.domain_name AS default_domain_name, d.notif AS domain_notification, "
                       "'%s' AS mqtt_hostname, '%d' AS mqtt_port, '%d' AS mqtt_over_ssl, d.browser_password, g.access_level "
                       "FROM users AS u "
                       "LEFT JOIN domains AS d ON (d.domain_uuid = u.default_domain_uuid) "
                       "LEFT JOIN users_grants AS g ON (g.user_uuid = u.user_uuid AND g.domain_uuid = d.domain_uuid) "
                       "WHERE email='%s' OR username='%s' LIMIT 1",
                       Json_get_string ( Global.config, "mqtt_hostname" ),
                       Json_get_int    ( Global.config, "mqtt_port" ) + 1,
                       Json_get_bool   ( Global.config, "mqtt_over_ssl" ),
                       email, username );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); goto end_user; }
    Json_node_add_string ( RootNode, "static_data_url", Json_get_string ( Global.config, "static_data_url" ) );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "this is your profil", RootNode );

end_user:
    g_free(username);
    g_free(email);
    g_free(user_uuid);
  }
/******************************************************************************************************************************/
/* USER_SET_request_post: Modifie les paramètres d'un utilisateur d'un domain                                                 */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 2 )) return;
    Http_print_request ( domain, token, path );
    if (Http_fail_if_has_not ( domain, path, msg, request, "user_uuid")) return;

    struct DOMAIN *master  = DOMAIN_tree_get ("master");
    gint user_access_level = Json_get_int ( token, "access_level" );
    gchar *domain_uuid     = Json_get_string ( domain->config, "domain_uuid" );

    JsonNode *Target_user = Json_node_create ();
    if (!Target_user) return;

    gchar *target_user_uuid = Normaliser_chaine ( Json_get_string ( request, "user_uuid" ) );
    DB_Read ( master, Target_user, NULL,
              "SELECT * FROM users INNER JOIN users_grants USING(user_uuid) WHERE users.user_uuid='%s' AND domain_uuid='%s'",
              target_user_uuid, domain_uuid );
    g_free(target_user_uuid);

    if (!Json_has_member ( Target_user, "user_uuid" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "User not found", NULL ); json_node_unref(Target_user); return; }

    gboolean set=FALSE;
    gchar requete[1024];
    g_snprintf ( requete, sizeof( requete ), "UPDATE users INNER JOIN users_grants USING(user_uuid) SET user_id=user_id " );

    if (Json_has_member ( request, "enable" ))
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",enable=%d", Json_get_bool ( request, "enable" ) );
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change enable to '%d'",
                   Json_get_string ( Target_user, "email" ), Json_get_bool ( request, "enable" ) );
     }

    if (Json_has_member ( request, "can_send_txt_cde" ))
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",can_send_txt_cde=%d", Json_get_bool ( request, "can_send_txt_cde" ) );
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change can_send_txt_cde to '%d'",
                   Json_get_string ( Target_user, "email" ), Json_get_bool ( request, "can_send_txt_cde" ) );
     }

    if (Json_has_member ( request, "wanna_be_notified" ))
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",wanna_be_notified=%d", Json_get_bool ( request, "wanna_be_notified" ) );
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change wanna_be_notified to '%d'",
                   Json_get_string ( Target_user, "email" ), Json_get_bool ( request, "wanna_be_notified" ) );
     }

    if (Json_has_member ( request, "access_level" ))
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",access_level=%d", Json_get_int ( request, "access_level" ) );
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change access_level to '%d'",
                   Json_get_string ( Target_user, "email" ), Json_get_int ( request, "access_level" ) );
     }

    if (Json_has_member ( request, "xmpp" ))
     { gchar add[128];
       gchar *parametre = Normaliser_chaine ( Json_get_string ( request, "xmpp" ) );
       g_snprintf( add, sizeof(add), ",xmpp='%s'", parametre );
       g_free(parametre);
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change xmpp to '%s'",
                   Json_get_string ( Target_user, "email" ), Json_get_string ( request, "xmpp" ) );
     }

    if (Json_has_member ( request, "free_sms_api_user" ))
     { gchar add[128];
       gchar *parametre = Normaliser_chaine ( Json_get_string ( request, "free_sms_api_user" ) );
       g_snprintf( add, sizeof(add), ",free_sms_api_user='%s'", parametre );
       g_free(parametre);
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change free_sms_api_user to '%s'",
                   Json_get_string ( Target_user, "email" ), Json_get_string ( request, "free_sms_api_user" ) );
     }

    if (Json_has_member ( request, "free_sms_api_key" ))
     { gchar add[128];
       gchar *parametre = Normaliser_chaine ( Json_get_string ( request, "free_sms_api_key" ) );
       g_snprintf( add, sizeof(add), ",free_sms_api_key='%s'", parametre );
       g_free(parametre);
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change free_sms_api_key to '%s'",
                   Json_get_string ( Target_user, "email" ), Json_get_string ( request, "free_sms_api_key" ) );
     }

    if (Json_has_member ( request, "phone" ))
     { gchar add[128];
       gchar *parametre = Normaliser_chaine ( Json_get_string ( request, "phone" ) );
       g_snprintf( add, sizeof(add), ",phone='%s'", parametre );
       g_free(parametre);
       g_strlcat ( requete, add, sizeof(requete) );
       set = TRUE;
       Audit_log ( domain, token, "USER", "User '%s' change phone to '%s'",
                   Json_get_string ( Target_user, "email" ), Json_get_string ( request, "phone" ) );
     }

    if (!set) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "no parameter to set", NULL ); return; }

    gchar add[512];
    g_snprintf( add, sizeof(add), " WHERE users.user_uuid='%s' AND domain_uuid='%s' AND (access_level < %d OR users.user_uuid='%s')",
                Json_get_string ( Target_user, "user_uuid" ), Json_get_string ( domain->config, "domain_uuid" ), user_access_level, Json_get_string ( token, "sub" ) );
    g_strlcat ( requete, add, sizeof(requete) );
    json_node_unref (Target_user);

    gboolean retour =  DB_Write ( master, requete );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "User modified", NULL );
  }
/******************************************************************************************************************************/
/* USER_GET_request_post: affiche un utilisateur d'un domain                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "target_user_uuid")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *target_user_uuid = Normaliser_chaine ( Json_get_string ( request, "target_user_uuid" ) );
    gboolean retour =  DB_Read ( master, RootNode, NULL,
                                "SELECT u.user_uuid, u.username, u.email, u.enable, u.xmpp, u.phone, "
                                "u.free_sms_api_user, u.free_sms_api_key, "
                                "g.can_send_txt_cde, g.wanna_be_notified, g.access_level "
                                "FROM users_grants AS g INNER JOIN users AS u USING(user_uuid) "
                                "WHERE g.domain_uuid='%s' AND u.user_uuid='%s' AND (g.access_level<%d OR u.user_uuid='%s') ORDER BY g.access_level",
                                Json_get_string ( domain->config, "domain_uuid" ), target_user_uuid,
                                user_access_level, Json_get_string ( token, "sub" ) );
    g_free(target_user_uuid);

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* USER_LIST_request_get: affiche les utilisateurs d'un domain                                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour =  DB_Read ( master, RootNode, "users",
                                "SELECT u.user_uuid, u.username, u.email, u.enable, g.access_level "
                                "FROM users_grants AS g INNER JOIN users AS u USING(user_uuid) "
                                "WHERE g.domain_uuid='%s' AND g.access_level<=%d ORDER BY g.access_level",
                                Json_get_string ( domain->config, "domain_uuid" ), user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* USER_INVITE_request_post: Invite un utilisateur sur le domaine source                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_INVITE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "friend_email")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "friend_level")) return;

    if (!strcasecmp ( Json_get_string ( token, "email" ), Json_get_string ( request, "friend_email" ) ))
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Could not invite yourself", NULL ); return; }

    gchar *email = Normaliser_chaine ( Json_get_string ( request, "friend_email" ) );               /* Formatage correct des chaines */
    if (!email) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", NULL ); return; }

    gint friend_level = Json_get_int ( request, "friend_level" );
    if (friend_level >= Json_get_int ( token, "access_level" )) friend_level = user_access_level - 1;

    gboolean retour =  DB_Write ( master,
                                 "INSERT INTO users_invite SET email = '%s', domain_uuid='%s', access_level='%d' "
                                 "ON DUPLICATE KEY UPDATE access_level=VALUE(access_level) ",
                                  email, Json_get_string ( domain->config, "domain_uuid" ), friend_level );
    gchar body[256];
    g_snprintf ( body, sizeof(body), "<strong>%s</strong> vous invite sur son domaine <strong>%s</strong>.<br>"
                                     "Cliquez sur le lien ci dessous pour acceder à ce domaine.",
                                     Json_get_string ( token, "given_name" ), Json_get_string ( domain->config, "domain_name" ) );
    Send_mail ( "Vous êtes invité sur Abls-Habitat.fr", email, body );
    g_free(email);
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
    Audit_log ( domain, token, "USER", "Invite '%s' with level %d'",
                Json_get_string ( request, "friend_email" ), Json_get_string ( request, "friend_level" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "User invited", NULL );
  }
/******************************************************************************************************************************/
/* USER_SET_DOMAIN_request_post: Change le domaine par défaut d'un utilisateur                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_SET_DOMAIN_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    /*if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;*/
    Http_print_request ( NULL, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");
    if (Http_fail_if_has_not ( NULL, path, msg, request, "domain_uuid")) return;
    struct DOMAIN *domain = DOMAIN_tree_get ( Json_get_string ( request, "domain_uuid" ) );

    if (!domain) { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Domain not found", NULL ); return; }
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    gchar *domain_name = Json_get_string ( domain->config, "domain_name" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;


    gboolean retour =  DB_Write ( master,
                                  "UPDATE users AS u SET u.default_domain_uuid = '%s' WHERE u.user_uuid = '%s'",
                                  domain_uuid, Json_get_string ( token, "sub" ) );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Audit_log ( domain, token, "USER", "Change default domain to '%s' (%s)", domain_uuid, domain_name );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* RUN_USERS_WANNA_BE_NOTIFIED: Renvoi la liste des user destinataires des notifications depuis les agents                    */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_USERS_WANNA_BE_NOTIFIED_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param )
  { JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    struct DOMAIN *master = DOMAIN_tree_get ("master");

    gboolean retour = DB_Read ( master, RootNode, "recipients",
                                "SELECT email, username, phone, free_sms_api_user, free_sms_api_key, xmpp "
                                "FROM users INNER JOIN users_grants USING (user_uuid) "
                                "WHERE enable=1 AND wanna_be_notified=1 AND domain_uuid='%s'", Json_get_string ( domain->config, "domain_uuid" ) );

    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Recipients List OK", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_USER_CAN_SEND_TXT_CDE_request_post: Renvoi si un user peut envoyer une commande textuelle                              */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_USER_CAN_SEND_TXT_CDE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  {
    gchar *critere = NULL;
         if ( Json_has_member ( request, "phone" ) ) critere = "phone";
    else if ( Json_has_member ( request, "xmpp" ) )  critere = "xmpp";
    else { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "No phone or xmpp provided", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *critere_value = Normaliser_chaine ( Json_get_string ( request, critere ) );       /* Formatage correct des chaines */
    if (!critere_value) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    struct DOMAIN *master = DOMAIN_tree_get ("master");

    gboolean retour = DB_Read ( master, RootNode, NULL,
                                "SELECT email, username, xmpp, phone, can_send_txt_cde FROM users INNER JOIN users_grants USING (user_uuid) "
                                "WHERE enable=1 AND domain_uuid='%s' AND %s='%s'",
                                Json_get_string ( domain->config, "domain_uuid" ), critere, critere_value );
    g_free(critere_value);
    Json_node_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "User can_send_txt sent", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
