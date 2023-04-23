/******************************************************************************************************************************/
/* mail.c        Fonctions d'envoi de mail aux utilisateurs                                                                   */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mail.c
 * This file is part of Watchdog
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

 gchar *mail_header="<!DOCTYPE html><html lang='fr'><head>"
    "<meta charset='utf-8'> <!-- utf-8 works for most cases -->"
    "<meta name='viewport' content='width=device-width'> "
    "<meta http-equiv='X-UA-Compatible' content='IE=edge'> <!-- Use the latest (edge) version of IE rendering engine -->"
    "<meta name='x-apple-disable-message-reformatting'>  <!-- Disable auto-scale in iOS 10 Mail entirely -->"
    "<title>Mail from ABLS-Habitat !</title> <!-- The title tag shows in email notifications, like Android 4.4. -->"
    "<link href='https://fonts.googleapis.com/css?family=Josefin+Sans:300,400,600,700|Lato:300,400,700' rel='stylesheet'>"
    "<style>"
    "html,body {"
    "margin: 0 auto !important;"
    "padding: 0 !important;"
    "height: 100% !important;"
    "width: 100% !important;"
    "background: #f1f1f1;"
	   "font-family: Arial, sans-serif;"
    "font-weight: 400;"
	   "font-size: 15px;"
	   "line-height: 1.8;"
    "}"
    "</style>"
    "</head>"
    "<body style='max-width: 600px; background-color: #FFF;'>"
    "<center><a class='' href='https://home.abls-habitat.fr'><img src='https://static.abls-habitat.fr/img/fond_home.jpg' alt='ABLS Login' width=600></a></center>"
    "<h1> <center>Une information de la part d'Abls-Habitat</center> </h1>";

 gchar *mail_footer=
    "<center><a class='' href='https://home.abls-habitat.fr'><img src='https://static.abls-habitat.fr/img/abls.svg' alt='ABLS Logo' width=50></a></center>"
    "<p>Bonne journée, <br>L'équipe ABLS-Habitat."
    "<hr>"
    "<h6> Pour nous contacter: contact@abls-habitat.fr</h6>"
    "</body></html>";
/******************************************************************************************************************************/
/* Send_mail: Envoi un mail en utilisation le sendmail du system                                                              */
/* Entrée: Le sujet, le destinataire, le corps du message                                                                     */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body )
  { gchar fichier[32], commande[128], chaine[256];
    gint fd;
    g_snprintf( fichier, sizeof(fichier), "/tmp/ABLSAPIMail_XXXXXX" );
    fd = mkstemp ( fichier );
    if (fd==-1)
     { Info_new( __func__, LOG_ERR, NULL, "Mkstemp failed for '%s', '%s': %s", sujet, dest, strerror(errno) );
       return(FALSE);
     }

    g_snprintf( chaine, sizeof(chaine), "From: no-reply@abls-habitat.fr\n"
                                        "Subject: %s\n"
                                        "To: %s\n"
                                        "Content-Type: text/html\n"
                                        "MIME-Version: 1.0\n"
                                        "\n", sujet, dest );

    if (write ( fd, chaine, strlen(chaine) ) < 0)
     { Info_new( __func__, LOG_ERR, NULL, "%s: writing smtp header failed for '%s', '%s'", sujet, dest );
       close(fd);
       return(FALSE);
     }

    if (write ( fd, mail_header, strlen(mail_header) ) < 0)
     { Info_new( __func__, LOG_ERR, NULL, "%s: writing mail header failed for '%s', '%s'", sujet, dest );
       close(fd);
       return(FALSE);
     }

    if (write ( fd, body, strlen(body) ) < 0)
     { Info_new( __func__, LOG_ERR, NULL, "Writing body failed for '%s', '%s'", sujet, dest );
       close(fd);
       return(FALSE);
     }

    if (write ( fd, mail_footer, strlen(mail_footer) ) < 0)
     { Info_new( __func__, LOG_ERR, NULL, "%s: writing mail footer failed for '%s', '%s'", sujet, dest );
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
