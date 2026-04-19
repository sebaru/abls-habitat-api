/******************************************************************************************************************************/
/* syn_cameras.c      Gestion des associations cameras/synoptiques dans l'API HTTP WebService                                 */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                20.03.2026 00:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * syn_cameras.c
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
/* SYN_CAMERA_LIST_request_get: Retourne la liste des cameras associées à un synoptique                                       */
/* Entrées: les elements libsoup                                                                                               */
/* Sortie : néant                                                                                                              */
/******************************************************************************************************************************/
 void SYN_CAMERA_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour;
    if (Json_has_member ( url_param, "syn_id" ))
     { gint syn_id = Json_get_int ( url_param, "syn_id" );
       retour = DB_Read ( domain, RootNode, "cameras",
                          "SELECT sc.syn_camera_id, sc.syn_id, s.page, c.camera_id, c.name AS camera_name, c.url, c.enable FROM syn_cameras AS sc "
                          "INNER JOIN cameras AS c USING(camera_id) "
                          "INNER JOIN syns AS s ON s.syn_id=sc.syn_id "
                          "WHERE sc.syn_id=%d ORDER BY c.name", syn_id );
     }
    else
     { retour = DB_Read ( domain, RootNode, "cameras",
                          "SELECT sc.syn_camera_id, sc.syn_id, s.page, c.camera_id, c.name AS camera_name, c.url, c.enable FROM syn_cameras AS sc "
                          "INNER JOIN cameras AS c USING(camera_id) "
                          "INNER JOIN syns AS s ON s.syn_id=sc.syn_id "
                          "ORDER BY s.page, c.name" );
     }

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* SYN_CAMERA_ADD_request_post: Associe une camera à un synoptique                                                            */
/* Entrées: les elements libsoup                                                                                               */
/* Sortie : néant                                                                                                              */
/******************************************************************************************************************************/
 void SYN_CAMERA_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 4 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id"))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "camera_id")) return;

    gint syn_id   = Json_get_int ( request, "syn_id" );
    gint camera_id = Json_get_int ( request, "camera_id" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Write ( domain,
                                 "INSERT IGNORE INTO syn_cameras SET syn_id=%d, camera_id=%d, date_create=NOW()",
                                 syn_id, camera_id );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

    Audit_log ( domain, token, "SYNOPTIQUE", "Camera %d ajoutée au synoptique %d", camera_id, syn_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Camera added to synoptique", RootNode );
  }
/******************************************************************************************************************************/
/* SYN_CAMERA_DELETE_request: Supprime l'association d'une camera à un synoptique                                             */
/* Entrées: les elements libsoup                                                                                               */
/* Sortie : néant                                                                                                              */
/******************************************************************************************************************************/
 void SYN_CAMERA_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 4 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_camera_id")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gint syn_camera_id = Json_get_int ( request, "syn_camera_id" );
    gboolean retour = DB_Write ( domain, "DELETE FROM syn_cameras WHERE syn_camera_id=%d", syn_camera_id );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }

    Audit_log ( domain, token, "SYNOPTIQUE", "Association camera syn_camera_id=%d supprimée", syn_camera_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Camera removed from synoptique", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
