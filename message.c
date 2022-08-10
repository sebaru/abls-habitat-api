/******************************************************************************************************************************/
/* message.c        Déclaration des fonctions pour la gestion des messages                                                    */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                     jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * message.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* Mnemo_auto_create_MSG: Ajout ou edition d'un message                                                                       */
/* Entrée: les parametres du message                                                                                          */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_MSG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint typologie, gint groupe )
  { 
    gchar *libelle = Normaliser_chaine ( libelle_src );                                             /* Formatage correct des chaines */
    if (!libelle)
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain, 
                                 "INSERT INTO msgs SET deletable='%d', tech_id='%s', acronyme='%s', libelle='%s', "
                                 "audio_libelle='%s', typologie='%d', sms_notification='0', groupe='%d' "
                                 " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), typologie=VALUES(typologie), groupe=VALUES(groupe)",
                                 deletable, tech_id, acronyme, libelle, libelle, typologie, groupe
                               );
    g_free(libelle);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
