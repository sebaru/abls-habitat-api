/******************************************************************************************************************************/
/* syn_motifs.c       Ajout/retrait de motifs dans la database                                                                */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                      mer 05 mai 2004 12:11:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * syn_motifs.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* Synoptique_auto_create_MOTIF: Création automatique d'un motif depuis la compilation DLS                                    */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Synoptique_auto_create_MOTIF ( struct DOMAIN *domain, JsonNode *plugin, gchar *target_tech_id_src, gchar *target_acronyme_src )
  { gchar *target_tech_id, *target_acro;
    gboolean retour;

/******************************************** Préparation de la base du mnemo *************************************************/
    target_acro = Normaliser_chaine ( target_acronyme_src );                                 /* Formatage correct des chaines */
    if ( !target_acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for target_acro." ); }

    target_tech_id = Normaliser_chaine ( target_tech_id_src );                               /* Formatage correct des chaines */
    if ( !target_tech_id )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for target_tech_id." ); }

    if (target_tech_id && target_acro)
     { retour = DB_Write ( domain,
		                   "INSERT INTO syns_motifs SET "
                           "dls_id='%d', mnemo_visuel_id=(SELECT mnemo_visuel_id FROM mnemos_VISUEL WHERE tech_id='%s' AND acronyme='%s'), "
                           "posx='150', posy='150', angle='0', scale='1', layer=(SELECT MAX(layer) FROM syns_motifs WHERE dls_id='%d')+1 "
                           "ON DUPLICATE KEY UPDATE mnemo_visuel_id=mnemo_visuel_id",
                           Json_get_int ( plugin, "dls_id" ), target_tech_id, target_acro, Json_get_int ( plugin, "dls_id" ) );
     } else retour = FALSE;

    if (target_tech_id) g_free(target_tech_id);
    if (target_acro)    g_free(target_acro);
    return (retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
