/******************************************************************************************************************************/
/* modbus.c                      Gestion des modbus dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * modbus.c
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

#ifdef bouh
/******************************************************************************************************************************/
/* MODBUS_GET_request_post: Appelé depuis libsoup pour éditer un domaine                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_GET_request_post ( struct DOMAIN *domain, const char *path, SoupMessage *msg, JsonNode *request )
  { JsonNode *token = Http_get_token ( domain, msg );
    if (!token) return;

    if (!Json_has_member ( __func__, request, "search_domain_uuid" ))
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: search_domain_uuid not present. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    if (!Json_has_member ( __func__, request, "search_domain_uuid" ))
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: search_domain_uuid not present. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    gchar *search_domain_uuid    = Json_get_string ( request, "search_domain_uuid" );
    struct DOMAIN *search_domain = MODBUS_tree_get ( search_domain_uuid );

    if (!search_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: search_domain_uuid does not exists or not connected. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    if (!Http_is_authorized ( search_domain, msg, token, 6 )) goto end_request;
    Http_print_request ( search_domain, token, path );

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode) { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" ); goto end_request; }

    DB_Read ( MODBUS_tree_get ("master"), RootNode, NULL,
              "SELECT * FROM modbus WHERE domain_uuid='%s'", search_domain_uuid );

    Http_Send_json_response ( msg, "success", RootNode );

end_request:
    json_node_unref(token);
  }
/******************************************************************************************************************************/
/* MODBUS_SET_request_post: Appelé depuis libsoup pour éditer un domaine                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_request_post ( struct DOMAIN *domain, const char *path, SoupMessage *msg, JsonNode *request )
  { JsonNode *token = Http_get_token ( domain, msg );
    if (!token) return;

    if (!Json_has_member ( __func__, request, "target_domain_uuid" ))
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: target_domain_uuid not present. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    if (!Json_has_member ( __func__, request, "description" ))
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: description not present not present. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    gchar *target_domain_uuid    = Json_get_string ( request, "target_domain_uuid" );
    struct DOMAIN *target_domain = MODBUS_tree_get ( target_domain_uuid );

    if (!target_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: target_domain_uuid does not exists or not connected. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    if (!Http_is_authorized ( target_domain, msg, token, 6 )) goto end_request;
    Http_print_request ( target_domain, token, path );

    gchar *description = Normaliser_chaine ( Json_get_string ( request, "description" ) );

    gboolean retour = DB_Write ( MODBUS_tree_get ("master"),
                                 "UPDATE modbus SET description='%s' "
                                 "WHERE domain_uuid='%s'", description, target_domain_uuid );
    g_free(description);

    Http_Send_json_response ( msg, (retour ? "success" : "failed"), NULL );

end_request:
    json_node_unref(token);
  }
/******************************************************************************************************************************/
/* MODBUS_DELETE_request_post: Supprime un domain                                                                             */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_DELETE_request_post ( struct DOMAIN *domain, const char *path, SoupMessage *msg, JsonNode *request )
  { JsonNode *token = Http_get_token ( domain, msg );
    if (!token) return;

    if (!Json_has_member ( __func__, request, "target_domain_uuid" ))
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: target_domain_uuid not present. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    gchar *target_domain_uuid    = Json_get_string ( request, "target_domain_uuid" );
    struct DOMAIN *target_domain = MODBUS_tree_get ( target_domain_uuid );

    if (!target_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: target_domain_uuid does not exists or not connected. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    if (!Http_is_authorized ( target_domain, msg, token, 6 )) goto end_request;
    Http_print_request ( target_domain, token, path );

    if ( strcmp ( Json_get_string ( token, "email" ), Json_get_string ( target_domain->config, "owner" ) ) )
     { soup_message_set_status (msg, SOUP_STATUS_FORBIDDEN );
       goto end_request;
     }

    gboolean retour = DB_Write ( MODBUS_tree_get ("master"),
                                 "DELETE modbus WHERE domain_uuid='%s'", target_domain_uuid );

    Http_Send_json_response ( msg, (retour ? "success" : "failed"), NULL );

end_request:
    json_node_unref(token);
  }
#endif
/******************************************************************************************************************************/
/* MODBUS_LIST_request_post: Appelé depuis libsoup pour l'URI modbus/list                                                     */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_LIST_request_post ( struct DOMAIN *domain, const char *path, SoupMessage *msg, JsonNode *request )
  { JsonNode *token = Http_get_token ( domain, msg );
    if (!token) return;

    if (!Json_has_member ( __func__, request, "classe" ))
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: classe not present. Bad Request", path );
       soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST );
       goto end_request;
     }

    if (!Http_is_authorized ( domain, msg, token, 6 )) goto end_request;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode) { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" ); goto end_request; }

    gboolean retour = FALSE;
    gchar *classe = Json_get_string ( request, "classe" );
         if (!strcasecmp ( classe, "modbus" )) retour = DB_Read ( domain, RootNode, "modbus", "SELECT * FROM modbus" );
    else if (!strcasecmp ( classe, "AI" ))     retour = DB_Read ( domain, RootNode, "AI", "SELECT * FROM modbus_AI");
    else if (!strcasecmp ( classe, "AO" ))     retour = DB_Read ( domain, RootNode, "AO", "SELECT * FROM modbus_AO");
    else if (!strcasecmp ( classe, "DI" ))     retour = DB_Read ( domain, RootNode, "DI", "SELECT * FROM modbus_DI");
    else if (!strcasecmp ( classe, "DO" ))     retour = DB_Read ( domain, RootNode, "DO", "SELECT * FROM modbus_DO");

    Http_Send_json_response ( msg, (retour ? "success" : "failed"), RootNode );

end_request:
    json_node_unref(token);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
