/******************************************************************************************************************************/
/* Include/server.h       Déclaration des fonctions servers                                                                   */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                19.07.2026          */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * server.h
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

#ifndef _SERVER_H_
 #define _SERVER_H_

/*************************************************** Définitions des prototypes ***********************************************/
 extern void SERVER_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path,
                                       SoupServerMessage *msg, JsonNode *request );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/