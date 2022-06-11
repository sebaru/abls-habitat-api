/******************************************************************************************************************************/
/* users.c                      Gestion des users dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                09.04.2022 21:33:35 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * users.c
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
/* USER_PROFIL_request_post: renvoi le profil utilisateur vis à vis du token recu                                             */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : Le JWT est mis a jour                                                                                             */
/******************************************************************************************************************************/
 void USER_PROFIL_request_post ( JsonNode *token, SoupMessage *msg, JsonNode *request )
  {
    Http_print_request ( NULL, token, "/user/profil" );

    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "name")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "preferred_username")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "given_name")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "family_name")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "email")) return;
    if (Http_fail_if_has_not ( NULL, "/user/profil", msg, token, "email_verified")) return;

    gchar *username  = Normaliser_chaine ( Json_get_string ( token , "preferred_username" ) );
    gchar *email     = Normaliser_chaine ( Json_get_string ( token , "email" ) );
    gchar *user_uuid = Normaliser_chaine ( Json_get_string ( token , "sub" ) );

    struct DOMAIN *master = DOMAIN_tree_get ("master");
    JsonNode *RootNode = Json_node_create();
encore:
    gboolean retour = DB_Read ( master, RootNode, NULL,
                                "SELECT u.user_uuid,u.email,u.username,u.enable, "
                                "u.default_domain_uuid, d.domain_name AS default_domain_name, g.access_level "
                                "FROM users AS u "
                                "LEFT JOIN domains AS d ON (d.domain_uuid = u.default_domain_uuid) "
                                "LEFT JOIN users_grants AS g ON (g.user_uuid = u.user_uuid AND g.domain_uuid = d.domain_uuid) "
                                "WHERE email='%s' OR username='%s' LIMIT 1", email, username );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); goto end_user; }

    if (!Json_has_member ( RootNode, "user_uuid" ))
     { Info_new ( __func__, LOG_NOTICE, NULL, "First request of a new user '%s'. Creating entry in database", email );
       gchar *user_uuid = Json_get_string ( token, "sub" );
       retour = DB_Write ( master, "INSERT INTO users SET user_uuid='%s', email='%s', username='%s',enable=1", user_uuid, email, username );
       if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); goto end_user; }
       gchar body[2048];
       g_snprintf ( body, sizeof(body), "Bonjour %s, <br>Votre compte a été créé. <br>Cliquez sur le lien ci dessous pour accéder à ABLS-Habitat.", Json_get_string ( token , "name" ) );
       Send_mail ( "Votre compte a été créé.", Json_get_string ( token , "email" ), body );
       goto encore;
     }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "this is your profil", RootNode );

end_user:
    g_free(username);
    g_free(email);
    g_free(user_uuid);

  }
/******************************************************************************************************************************/
/* USER_LIST_request_post: affiche les utilisateurs d'un domain                                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    if (Http_fail_if_has_not ( domain, path, msg, request, "user_uuid")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "domain_uuid")) return;

    gchar *login = Normaliser_chaine ( Json_get_string ( request, "login" ) );               /* Formatage correct des chaines */
    if (!login) { soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR ); return; }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour =  DB_Read ( master, RootNode, "users",
                                "SELECT u.user_uuid, u.username, u.email, u.enable, u.can_send_txt, u.can_recv_sms, u.xmpp, u.phone, g.access_level "
                                "FROM users_grants AS g INNER JOIN users AS u USING(user_uuid) "
                                "WHERE g.domain_uuid='%s' AND g.access_level<=%d ORDER BY g.access_level",
                                Json_get_string ( domain->config, "domain_uuid" ), Json_get_int ( token, "access_level" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* USER_INVITE_request_post: Invite un utilisateur sur le domaine source                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_INVITE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    if (Http_fail_if_has_not ( domain, path, msg, request, "friend_email")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "friend_level")) return;

    gchar *email = Normaliser_chaine ( Json_get_string ( request, "friend_email" ) );               /* Formatage correct des chaines */
    if (!email) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", NULL ); return; }

    gint friend_level = Json_get_int ( request, "friend_level" );
    if (friend_level >= Json_get_int ( token, "access_level" )) friend_level = Json_get_int ( token, "access_level" ) - 1;

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

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "User invited", NULL );
  }
/******************************************************************************************************************************/
/* USER_SET_DOMAIN_request_post: Change le domaine par défaut d'un utilisateur                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void USER_SET_DOMAIN_request_post ( JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    /*if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;*/
    Http_print_request ( NULL, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    if (Http_fail_if_has_not ( NULL, path, msg, request, "target_domain_uuid")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *target_domain_uuid = Normaliser_chaine ( Json_get_string ( request, "target_domain_uuid" ) );   /* Formatage correct des chaines */

    gboolean retour =  DB_Write ( master,
                                  "UPDATE users AS u SET u.default_domain_uuid = '%s' WHERE u.user_uuid = '%s'",
                                  target_domain_uuid, Json_get_string ( token, "sub" ) );

    g_free(target_domain_uuid);

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
