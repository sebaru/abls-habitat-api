/******************************************************************************************************************************/
/* message.c        Déclaration des fonctions pour la gestion des messages                                                    */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                     jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * message.c
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
/* Mnemo_auto_create_MSG: Ajout ou edition d'un message                                                                       */
/* Entrée: les parametres du message                                                                                          */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_MSG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme,
                                  gchar *libelle_src, gint typologie, gint notif_sms, gint notif_chat )
  {
    gchar *libelle = Normaliser_chaine ( libelle_src );                                      /* Formatage correct des chaines */
    if (!libelle)
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,
                                 "INSERT INTO msgs SET deletable='%d', used=1, tech_id='%s', acronyme='%s', libelle='%s', "
                                 "audio_libelle='%s', typologie='%d', "
                                 "notif_sms='-1', notif_sms_by_dls='%d', "
                                 "notif_chat='-1', notif_chat_by_dls='%d' "
                                 " ON DUPLICATE KEY UPDATE used=1, "
                                 "libelle=VALUES(libelle), typologie=VALUES(typologie), "
                                 "notif_sms_by_dls=VALUES(notif_sms_by_dls), "
                                 "notif_chat_by_dls=VALUES(notif_chat_by_dls) ",
                                 deletable, tech_id, acronyme, libelle, libelle, typologie, notif_sms, notif_chat
                               );
    g_free(libelle);
    return(retour);
  }
/******************************************************************************************************************************/
/* MESSAGE_LIST_request_get: Liste les messages du domaine                                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MESSAGE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar requete[512];
    g_snprintf ( requete, sizeof(requete),
                 "SELECT m.* FROM msgs AS m "
                 "INNER JOIN dls USING(`tech_id`) "
                 "INNER JOIN syns AS s USING(`syn_id`) "
                 "WHERE s.access_level<='%d' ", user_access_level );

    if (Json_has_member ( url_param, "tech_id" ))
     { gchar *tech_id = Normaliser_chaine ( Json_get_string ( url_param, "tech_id" ) );
       if(tech_id)
        { g_strlcat ( requete, "AND tech_id='", sizeof(requete) );
          g_strlcat ( requete, tech_id, sizeof(requete) );
          g_strlcat ( requete, "' ", sizeof(requete) );
          g_free(tech_id);
        }
     }

    g_strlcat ( requete, "ORDER BY m.tech_id, m.acronyme", sizeof(requete) );
    gboolean retour = DB_Read ( domain, RootNode, "messages", requete );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of Messages", RootNode );
  }
/******************************************************************************************************************************/
/* MESSAGE_SET_request_post: Appelé depuis libsoup pour éditer un message                                                     */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MESSAGE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "rate_limit" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "audio_libelle" ))    return;

    if (Http_fail_if_has_not ( domain, path, msg, request, "notif_sms" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "notif_chat" )) return;

    gchar *tech_id         = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme        = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );
    gchar *audio_libelle   = Normaliser_chaine ( Json_get_string( request, "audio_libelle" ) );
    gint  notif_sms        = Json_get_int( request, "notif_sms" );
    gint  notif_chat       = Json_get_int( request, "notif_chat" );
    gint  rate_limit       = Json_get_int( request, "rate_limit" );

    retour = DB_Write ( domain,
                        "UPDATE msgs SET audio_libelle='%s', notif_sms=%d, notif_chat=%d, rate_limit=%d "
                        "WHERE tech_id='%s' AND acronyme='%s'",
                        audio_libelle, notif_sms, notif_chat, rate_limit, tech_id, acronyme );

    g_free(tech_id);
    g_free(acronyme);
    g_free(audio_libelle);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Dls_Send_compil_to_master ( domain, Json_get_string( request, "tech_id" ) );
    JsonNode *RootNode = Json_node_create ();
    Json_node_add_string ( RootNode, "tech_id", Json_get_string( request, "tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Message changed", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
