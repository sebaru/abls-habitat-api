/******************************************************************************************************************************/
/* camera.c                      Gestion des cameras dans l'API HTTP WebService                                              */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                16.03.2026 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * camera.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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
/* CAMERA_LIST_request_get: Repond aux requests depuis les browsers                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void CAMERA_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gint user_access_level = Json_get_int ( token, "access_level" );
    gboolean retour = DB_Read ( domain, RootNode, "cameras",
                                "SELECT * FROM cameras WHERE access_level <= %d ORDER BY name", user_access_level );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* CAMERA_GET_request_get: Repond aux requests depuis les browsers                                                            */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void CAMERA_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "camera_id")) return;

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gint camera_id = Json_get_int ( url_param, "camera_id" );
    gint user_access_level = Json_get_int ( token, "access_level" );

    gboolean retour = DB_Read ( domain, RootNode, NULL,
                                "SELECT camera_id, name, url, access_level FROM cameras "
                                "WHERE camera_id=%d", camera_id );
    
    if (!retour || !Json_has_member ( RootNode, "camera_id" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Camera not found", NULL );
       return;
     }

    gint camera_access_level = Json_get_int ( RootNode, "access_level" );
    if (user_access_level < camera_access_level)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied", NULL );
       return;
     }

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* CAMERA_ADD_request_post: Ajoute une camera                                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void CAMERA_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 8 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "name")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "url")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "access_level")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable")) return;

    gint access_level = Json_get_int ( request, "access_level" );
    if (access_level < 0 || access_level > 9)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Invalid access_level (must be 0-9)", NULL );
       return;
     }

    gboolean enable = Json_get_bool ( request, "enable" );
    gchar *name = Normaliser_chaine ( Json_get_string ( request, "name" ) );
    gchar *url  = Normaliser_chaine ( Json_get_string ( request, "url" ) );
    gboolean retour = DB_Write ( domain,
                                "INSERT INTO cameras SET name='%s', url='%s', date_create=NOW(), access_level=%d, enable=%d",
                                name, url, access_level, enable );
    g_free(name);
    g_free(url);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "CAMERA", "Camera '%s' added with access_level %d, enable %d", Json_get_string ( request, "name" ), access_level, enable );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Camera added successfully", NULL );
  }
/******************************************************************************************************************************/
/* CAMERA_SET_request_post: Modifie une camera                                                                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void CAMERA_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 8 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "camera_id")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "name")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "url")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "access_level")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable")) return;

    gint camera_id         = Json_get_int ( request, "camera_id" );
    gint user_access_level = Json_get_int ( token, "access_level" );
    gint new_access_level  = Json_get_int ( request, "access_level" );
    gboolean new_enable    = Json_get_bool ( request, "enable" );

    if (new_access_level < 0 || new_access_level > 9)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Invalid access_level (must be 0-9)", NULL );
       return;
     }

     /* Vérifier que la camera existe et que l'utilisateur a le droit de la modifier */
    JsonNode *Camera = Http_json_node_create ( msg );
    if (!Camera) return;

    DB_Read ( domain, Camera, NULL, "SELECT * FROM cameras WHERE camera_id=%d", camera_id );
    if (!Json_has_member ( Camera, "access_level" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Camera not found", NULL );
       return;
     }

    gint camera_access_level = Json_get_int ( Camera, "access_level" );
    if (user_access_level < camera_access_level)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied - cannot modify this camera", NULL );
       return;
     }

    gchar *name = Normaliser_chaine ( Json_get_string ( request, "name" ) );
    gchar *url  = Normaliser_chaine ( Json_get_string ( request, "url" ) );
    gboolean retour = DB_Write ( domain,
                                "UPDATE cameras SET name='%s', url='%s', access_level=%d, enable=%d WHERE camera_id=%d",
                                name, url, new_access_level, new_enable, camera_id );
    g_free(name);
    g_free(url);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "CAMERA", "Camera %d updated - name: '%s', access_level: %d, enable: %d", camera_id, Json_get_string ( request, "name" ), new_access_level, new_enable );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Camera updated successfully", NULL );
  }
/******************************************************************************************************************************/
/* CAMERA_DELETE_request: Supprime une camera                                                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void CAMERA_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 8 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "camera_id")) return;

    gint camera_id = Json_get_int ( request, "camera_id" );
    gint user_access_level = Json_get_int ( token, "access_level" );

    /* Vérifier que la camera existe et que l'utilisateur a le droit de la supprimer */
    JsonNode *Camera = Http_json_node_create ( msg );
    if (!Camera) return;

    DB_Read ( domain, Camera, NULL, "SELECT * FROM cameras WHERE camera_id=%d", camera_id );
    if (!Json_has_member ( Camera, "access_level" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Camera not found", NULL );
       return;
     }

    gint camera_access_level = Json_get_int ( Camera, "access_level" );

    if (user_access_level < camera_access_level)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access denied - cannot delete this camera", NULL );
       return;
     }

    gboolean retour = DB_Write ( domain, "DELETE FROM cameras WHERE camera_id=%d", camera_id );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); 
                   return; 
                 }

    Audit_log ( domain, token, "CAMERA", "Camera '%s' (id: %d) deleted", Json_get_string ( Camera, "name" ), camera_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Camera deleted successfully", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
