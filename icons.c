/******************************************************************************************************************************/
/* icons.c                      Gestion des agents dans l'API HTTP WebService                                           */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * icons.c
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
/* ICONS_request_get: Repond aux requests du domain                                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ICONS_request_get ( SoupServer *server, SoupServerMessage *msg, const char *path )
  { gboolean retour = FALSE;
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;
    retour = DB_Read ( DOMAIN_tree_get("master"), RootNode, "icons", "SELECT * FROM icons" );
    Http_Send_json_response ( msg, retour, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
