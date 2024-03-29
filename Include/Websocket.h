/******************************************************************************************************************************/
/* Include/Http.h        Déclaration structure internes des WebServices                                                       */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                19.02.2022 20:58:34 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

#ifndef _WEBSOCKET_H_
 #define _WEBSOCKET_H_

 struct WS_AGENT_SESSION
  { SoupWebsocketConnection *connexion;
    struct DOMAIN *domain;
    gchar agent_uuid[37];
  };

 struct WS_CLIENT_SESSION
  { SoupWebsocketConnection *connexion;
    struct DOMAIN *domain;
    JsonNode *abonnements;
    gint user_access_level;
  };

/*************************************************** Définitions des prototypes ***********************************************/
 extern void WS_Agent_Open_CB ( SoupServerMessage *msg, gpointer user_data );
 extern void WS_Http_Open_CB ( SoupServerMessage *msg, gpointer user_data );
 extern void WS_Client_send_cadran_to_all ( struct DOMAIN *domain, JsonNode *node );
 extern void WS_Client_send_to_all ( struct DOMAIN *domain, JsonNode *node );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
