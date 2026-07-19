/******************************************************************************************************************************/
/* server.c                     Gestion des serveurs dans l'API HTTP WebService                                               */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                19.07.2026 18:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * server.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
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
/* SERVER_SET_request_post: Modifie la configuration d'un serveur                                                            */
/* Entrees: la connexion Websocket                                                                                            */
/* Sortie : neant                                                                                                             */
/******************************************************************************************************************************/
 void SERVER_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "headless" ))    return;

    gchar *server_uuid = Normaliser_chaine ( Json_get_string ( request, "server_uuid" ) );
    if (!server_uuid)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "server_uuid invalide", NULL );
       return;
     }

    gboolean retour = DB_Write ( domain,
                  "UPDATE servers SET headless='%d' WHERE server_uuid='%s'",
                  Json_get_bool ( request, "headless" ), server_uuid );
    g_free(server_uuid);
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, domain->mysql_last_error, NULL );
       return;
     }

    MQTT_Send_to_domain ( domain, request, "SERVER/%s/RESTART", Json_get_string ( request, "server_uuid" ) );
    Audit_log ( domain, token, "SERVER", "Server '%s' updated", Json_get_string ( request, "server_uuid" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Server updated", NULL );
  }