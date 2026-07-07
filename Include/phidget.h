/******************************************************************************************************************************/
/* Include/phidget.h        Déclaration des fonctions phidget                                                              */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                06.07.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * phidget.h
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

#ifndef _PHIDGET_H_
 #define _PHIDGET_H_

 #include "Http.h"

/*************************************************** Définitions des prototypes ***********************************************/
 extern gboolean Phidget_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode );
 extern void Phidget_Copy_thread_io_to_mnemos ( struct DOMAIN *domain );
 extern void PHIDGET_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void PHIDGET_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void PHIDGET_SET_IO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_PHIDGET_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/