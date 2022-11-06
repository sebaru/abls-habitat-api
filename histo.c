/******************************************************************************************************************************/
/* histo.c              Déclaration des fonctions pour la gestion des historiques                                             */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                06.11.2022 15:22:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * message.c
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
/* HISTO_ALIVE_request_get: Renvoi les historiques vivant au user                                                             */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_ALIVE_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupMessage *msg, JsonNode *url_param )
  { /*if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id")) return;*/

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "histo_msgs", "SELECT * FROM histo_msgs WHERE date_fin IS NOT NULL" );
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "you have histo alives", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
