/******************************************************************************************************************************/
/* mnemo_HORLOGE.c        Déclaration des fonctions pour la gestion des Horloges                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                03.07.2018 21:25:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_HORLOGE.c
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
/* Mnemo_auto_create_HORLOGE_from_thread: Ajoute un mnemonique dans la base via le tech_id depuis une demande d'un thread     */
/* Entrée: le tech_id, l'acronyme, le libelle et l'unite et l'archivage                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_HORLOGE_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *libelle = Normaliser_chaine ( libelle_src );                                      /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,                                                                     /* Requete SQL */
                                 "INSERT INTO mnemos_HORLOGE SET deletable=0, tech_id='%s', acronyme='%s', libelle='%s' "
                                 "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                                 tech_id, acro, libelle );
    g_free(acro);
    g_free(libelle);
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_auto_create_HORLOGE_from_dls: Ajoute un mnemonique dans la base via le tech_id depuis une demande dls                */
/* Entrée: le tech_id, l'acronyme, le libelle                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_HORLOGE_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *libelle = Normaliser_chaine ( libelle_src );                                      /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,                                                                     /* Requete SQL */
                                 "INSERT INTO mnemos_HORLOGE SET deletable=1, tech_id='%s', acronyme='%s', libelle='%s' "
                                 "ON DUPLICATE KEY UPDATE libelle=IF(deletable=1, VALUES(libelle), libelle)",
                                 tech_id, acro, libelle );
    g_free(acro);
    g_free(libelle);
    return (retour);
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Horloge_del_all_ticks: Retire tous les ticks d'une horloge                                                                 */
/* Entrée: le tech_id/acronyme de l'horloge                                                                                   */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Horloge_del_all_ticks ( gchar *tech_id, gchar *acronyme )
  { return( SQL_Write_new ( "DELETE mnemos_HORLOGE_ticks FROM mnemos_HORLOGE_ticks "
                            "INNER JOIN mnemos_HORLOGE ON mnemos_HORLOGE.id = mnemos_HORLOGE_ticks.horloge_id "
                            "WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme
                          ) );
  }
/******************************************************************************************************************************/
/* Horloge_add_tick: Ajout un tick a heure/minute en parametre pour l'horloge tech_id:acronyme                                */
/* Entrée: le tech_id/acronyme de l'horloge, l'heure et la minute                                                             */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Horloge_add_tick ( gchar *tech_id, gchar *acronyme, gint heure, gint minute )
  { return( SQL_Write_new ( "INSERT INTO mnemos_HORLOGE_ticks SET "
                            "horloge_id = (SELECT id FROM mnemos_HORLOGE WHERE tech_id='%s' AND acronyme='%s'), "
                            "lundi=1, mardi=1, mercredi=1, jeudi=1, vendredi=1, samedi=1, dimanche=1, "
                            "heure = %d, minute = %d", tech_id, acronyme, heure, minute ) );
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
