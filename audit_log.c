/******************************************************************************************************************************/
/* audit_log.c                      Gestion des log user dans l'API HTTP WebService                                           */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                20.12.2022 07:50:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audit_log.c
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
/* Audit_log: Appelé depuis libsoup pour éditer ou creer un audit_log                                                         */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Audit_log ( struct DOMAIN *domain, JsonNode *token, gchar *classe, gchar *format, ... )
  { gchar chaine[512];
    va_list ap;

    if (!domain || !token) return;

    va_start( ap, format );
    g_vsnprintf ( chaine, sizeof(chaine), format, ap );
    va_end ( ap );
    Info_new ( __func__, LOG_NOTICE, domain, "%s", chaine );
    gchar *message = Normaliser_chaine ( chaine );
    if (message)
     { DB_Write ( domain, "INSERT INTO audit_log SET username='%s', classe='%s', access_level='%d', message='%s'",
                  Json_get_string ( token, "email" ), classe, Json_get_int ( token, "access_level" ), message );
     } else Info_new ( __func__, LOG_ERR, domain, "Audit log memory error for normalize message" );
    g_free(message);
  }
/******************************************************************************************************************************/
/* AUDIT_LOG_LIST_request_get: Liste les audit_logs du domaine                                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                               */
/******************************************************************************************************************************/
 void AUDIT_LOG_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gint user_access_level = Json_get_int ( token, "access_level" );
    gboolean retour = DB_Read ( domain, RootNode, "audit_logs",
                                "SELECT * FROM audit_log WHERE access_level<'%d' ORDER BY date DESC LIMIT 500",
                                user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of audit logs", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
