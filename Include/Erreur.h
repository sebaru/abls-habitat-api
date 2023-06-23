/******************************************************************************************************************************/
/* Include/Erreur.h      Déclaration des constantes et prototypes de gestion de log                                           */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                19.02.2022 20:58:34 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Erreur.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien LEFEVRE
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

#ifndef _ERREUR_H_
 #define _ERREUR_H_

 #include <glib.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <time.h>
 #include <sys/prctl.h>
 #include <stdarg.h>

 extern void Info_new( gchar *func, guint priority, struct DOMAIN *domain, gchar *format, ... );
 extern void Info_change_log_level( guint new_log_level );
 extern void Info_init( gchar *entete, guint log_level );
#endif
/*--------------------------------------------------------------------------------------------------------*/
