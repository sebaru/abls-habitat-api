/******************************************************************************************************************************/
/* mail.c        Fonctions d'envoi de mail aux utilisateurs                                                                   */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mail.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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
/* Send_mail: Envoi un mail en utilisation le sendmail du system                                                              */
/* Entrée: Le sujet, le destinataire, le corps du message                                                                     */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body )
  { gchar fichier[32], commande[128], chaine[256];
    gint fd;
    g_snprintf( fichier, sizeof(fichier), "WTDMail_XXXXXX" );
    fd = mkstemp ( fichier );
    if (fd==-1)
     { Info_new( __func__, LOG_ERR, NULL, "Mkstemp failed for '%s', '%s': %s", sujet, dest, strerror(errno) );
       return(FALSE);
     }

    g_snprintf( chaine, sizeof(chaine), "From: WatchdogServer\n"
                                        "Subject: %s\n"
                                        "To: %s\n"
                                        "Content-Type: text/html\n"
                                        "MIME-Version: 1.0\n"
                                        "\n", sujet, dest );

    if (write ( fd, chaine, strlen(chaine) ) < 0)
     { Info_new( __func__, LOG_ERR, NULL, "%s: writing header failed for '%s', '%s'", sujet, dest );
       close(fd);
       return(FALSE);
     }


    if (write ( fd, body, strlen(body) ) < 0)
     { Info_new( __func__, LOG_ERR, NULL, "Writing body failed for '%s', '%s'", sujet, dest );
       close(fd);
       return(FALSE);
     }

    close(fd);

    g_snprintf ( commande, sizeof(commande), "cat %s | sendmail -t", fichier );
    system(commande);
    unlink(fichier);
    Info_new( __func__, LOG_NOTICE, NULL, "Mail '%s' sent to '%s'", sujet, dest );
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
