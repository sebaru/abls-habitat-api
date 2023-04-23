/******************************************************************************************************************************/
/* syn_cadrans.c       Ajout/retrait de cadrans dans la database                                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                               26.01.2023 21:40:22  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * syn_cadrans.c
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
/* Synoptique_auto_create_CADRAN: Ajoute un cadran sur le DLS en parametre, qui cible le tech_id:acronyme en parametre        */
/* Entrée: les informations du cadran                                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Synoptique_auto_create_CADRAN ( struct DOMAIN *domain, gint dls_id, gchar *forme_src, gchar *tech_id_src, gchar *acronyme_src,
                                          gdouble min, gdouble max,
                                          gdouble seuil_ntb, gdouble seuil_nb,
                                          gdouble seuil_nh, gdouble seuil_nth,
                                          gint nb_decimal )
  { gchar *target_acro = Normaliser_chaine ( acronyme_src );                                 /* Formatage correct des chaines */
    if ( !target_acro ) { Info_new ( __func__, LOG_ERR, domain, "Normalize error for target_acro." ); }

    gchar *target_tech_id = Normaliser_chaine ( tech_id_src );                               /* Formatage correct des chaines */
    if ( !target_tech_id ) { Info_new ( __func__, LOG_ERR, domain, "Normalize error for target_tech_id." ); }

    gchar *forme = Normaliser_chaine ( forme_src );                                          /* Formatage correct des chaines */
    if ( !forme ) { Info_new ( __func__, LOG_ERR, domain, "Normalisation forme impossible. Cadran NOT added." ); }

    gboolean retour;
    if (target_tech_id && target_acro && forme)
     { retour = DB_Write ( domain,
                          "INSERT INTO syns_cadrans SET "
                          "dls_id=%d, tech_id='%s', acronyme='%s', forme='%s', minimum='%f', maximum='%f', "
                          "seuil_ntb='%f', seuil_nb='%f', seuil_nh='%f', seuil_nth='%f', "
                          "nb_decimal='%d', posx='150', posy='150' "
                          "ON DUPLICATE KEY UPDATE forme=VALUES(forme), "
                          "minimum=VALUES(minimum), maximum=VALUES(maximum), nb_decimal=VALUES(nb_decimal), "
                          "seuil_ntb=VALUES(seuil_ntb), seuil_nb=VALUES(seuil_nb), seuil_nh=VALUES(seuil_nh), seuil_nth=VALUES(seuil_nth)",
                          dls_id, target_tech_id, target_acro, forme, min, max, seuil_ntb, seuil_nb, seuil_nh, seuil_nth, nb_decimal );
     } else retour = FALSE;

    if (forme) g_free(forme);
    if (target_tech_id) g_free(target_tech_id);
    if (target_acro)    g_free(target_acro);
    return (retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
