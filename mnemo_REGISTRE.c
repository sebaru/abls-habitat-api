/******************************************************************************************************************************/
/* mnemo_REGISTRE.c              Déclaration des fonctions pour la gestion des registre.c                                     */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                22.03.2017 10:29:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_TEMPO.c
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
/* Mnemo_auto_create_REGISTRE: Ajout ou modifie le mnemo en parametre                                                         */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_REGISTRE ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src )
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

    gchar *unite = Normaliser_chaine ( unite_src );                                          /* Formatage correct des chaines */
    if ( !unite )
      { Info_new ( __func__, LOG_ERR, domain, "Normalize error for unite." );
        g_free(libelle);
        g_free(acro);
        return(FALSE);
      }

    gboolean retour = DB_Write ( domain, 
                                 "INSERT INTO mnemos_R SET tech_id='%s',acronyme='%s',libelle='%s',unite='%s' "
                                 "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), unite=VALUES(unite) ",
                                 tech_id, acro, libelle, unite );
    g_free(libelle);
    g_free(acro);
    g_free(unite);
    return (retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
