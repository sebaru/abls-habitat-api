/******************************************************************************************************************************/
/* audit_log.c                      Gestion des log user dans l'API HTTP WebService                                           */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                20.12.2022 07:50:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audit_log.c
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
     { DB_Write ( domain, "INSERT INTO audit_log SET username='%s', classe='%s', message='%s'",
                  Json_get_string ( token, "email" ), classe, message );
     } else Info_new ( __func__, LOG_ERR, domain, "Audit log memory error for normalize message" );
    g_free(message);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
