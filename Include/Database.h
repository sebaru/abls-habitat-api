/******************************************************************************************************************************/
/* Include/Database.h           Gestion des bases de données des domaines                                                     */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                19.02.2022 20:58:34 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Database.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #ifndef _DB_H_
  #define _DB_H_

 #include <mysql.h>

/************************************* Prototypes de fonctions ********************************************/
 extern gchar *Normaliser_chaine( gchar *pre_comment );
 extern gboolean DB_Write( struct DOMAIN *domain, gchar *format, ... );
 extern gboolean DB_Arch_Write( struct DOMAIN *domain, gchar *format, ... );
 extern gboolean DB_Arch_Pool_init ( struct DOMAIN *domain );
 extern gboolean DB_Pool_init ( struct DOMAIN *domain );
 extern void DB_Pool_end ( struct DOMAIN *domain );
 extern gboolean DB_Master_Update ( void );
 extern gboolean DB_Icons_Update ( void );
 extern gboolean DB_Read ( struct DOMAIN *domain, JsonNode *RootNode, gchar *array_name, gchar *format, ... );
 extern gboolean DB_Arch_Connect ( struct DOMAIN *domain );
 extern gboolean DB_Arch_Read ( struct DOMAIN *domain, JsonNode *RootNode, gchar *array_name, gchar *format, ... );
 extern gboolean DB_Read_from_file ( struct DOMAIN *domain, gchar *file );
 extern gboolean DB_Cleanup ( gpointer key, gpointer value, gpointer data );
 #endif
/*--------------------------------------------------------------------------------------------------------*/
