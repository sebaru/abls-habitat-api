/******************************************************************************************************************************/
/* mnemo_VISUEL.c       Ajout/retrait de visuel dans la database                                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                      mer 05 mai 2004 12:11:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_VISUEL.c
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
/* Mnemo_auto_create_VISUEL: Création automatique d'un visuel depuis la compilation DLS                                  */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_VISUEL ( struct DOMAIN *domain, JsonNode *plugin, gchar *acronyme, gchar *libelle_src,
                                     gchar *forme_src, gchar *mode_src, gchar *couleur_src )
  { gchar *acro, *libelle, *forme, *mode, *couleur;
    gboolean retour;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for acronyme." ); }

    libelle    = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." ); }

    forme      = Normaliser_chaine ( forme_src );                                            /* Formatage correct des chaines */
    if ( !forme )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for forme." ); }

    mode       = Normaliser_chaine ( mode_src );                                             /* Formatage correct des chaines */
    if ( !mode )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for mode." ); }

    couleur    = Normaliser_chaine ( couleur_src );                                          /* Formatage correct des chaines */
    if ( !couleur )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for couleur." ); }

    if (acro && libelle && forme && mode && couleur)
     { retour = DB_Write( domain,
		                  "INSERT INTO mnemos_VISUEL SET "
                          "tech_id='%s', acronyme='%s', forme='%s', libelle='%s', mode='%s', color='%s' "
                          "ON DUPLICATE KEY UPDATE forme=VALUES(forme), libelle=VALUES(libelle),"
                          "mode=VALUES(mode), color=VALUES(color)",
                          Json_get_string ( plugin, "tech_id" ), acro, forme, libelle, mode, couleur );
     } else retour = FALSE;

    if (acro)    g_free(acro);
    if (forme)   g_free(forme);
    if (couleur) g_free(couleur);
    if (mode)    g_free(mode);
    if (libelle) g_free(libelle);

    return (retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
