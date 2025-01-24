/******************************************************************************************************************************/
/* Erreur.c         Gestion des logs systemes                                                                                 */
/* Projet Abls-Habitat version 4.3   Gestion d'habitat                                                    16.02.2022 09:43:52 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Erreur.c
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

 #include "Http.h"

/******************************************************************************************************************************/
/* Info_new: on informe le sous systeme syslog en affichant un nombre aléatoire de paramètres                                 */
/* Entrée: le niveau, le texte, et la chaine à afficher                                                                       */
/******************************************************************************************************************************/
 void Info_new( gchar *function, guint priority, struct DOMAIN *domain, gchar *format, ... )
  { gchar chaine[512], nom_thread[32], *domain_uuid;
    va_list ap;

    prctl( PR_GET_NAME, &nom_thread, 0, 0, 0);
    if (domain) domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
           else domain_uuid = "master";
    g_snprintf( chaine, sizeof(chaine), "{ \"domain_uuid\": \"%s\", \"thread\":\"%s\", \"function\":\"%s\", \"message\":\"%s\" }",
                domain_uuid, nom_thread, function, format );

    va_start( ap, format );
    vsyslog ( priority, chaine, ap );
    va_end ( ap );
  }
/******************************************************************************************************************************/
/* Info_change_log_level: Change le niveau de logur                                                                           */
/* Entrée: Le nouveau niveau de debuggage                                                                                     */
/******************************************************************************************************************************/
 void Info_change_log_level( guint new_log_level )
  { setlogmask( LOG_UPTO(new_log_level) );
    Info_new( __func__, LOG_INFO, NULL, "Log level set to %d", new_log_level );
  }
/******************************************************************************************************************************/
/* Info_stop: Arret de la fonctionnalité de logging                                                                           */
/* Entrée: non utilisé                                                                                                        */
/******************************************************************************************************************************/
 static void Info_stop( int code_retour, void *data )
  { Info_new( __func__, LOG_INFO, NULL, "End of logs" ); }
/******************************************************************************************************************************/
/* Info_init: Initialisation du traitement d'erreur                                                                           */
/* Entrée: L'entete souhaité, le niveau de log                                                                                */
/******************************************************************************************************************************/
 void Info_init( gchar *entete, guint log_level )
  { on_exit( Info_stop, NULL );
    openlog( entete, LOG_CONS | LOG_PID, LOG_USER );
    Info_new( __func__, LOG_INFO, NULL, "Start of logs" );
    Info_change_log_level( log_level );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
