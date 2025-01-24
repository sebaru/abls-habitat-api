/******************************************************************************************************************************/
/* mnemo_CH.c      Déclaration des fonctions pour la gestion des cpt_h                                                        */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                      mar 14 fév 2006 15:03:51 CET  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_CH.c
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* Mnemo_auto_create_CH: Ajout ou modifie le mnemo en parametre                                                               */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_CH ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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

    gboolean retour = DB_Write ( domain,
                                 "INSERT INTO mnemos_CH SET used=1, tech_id='%s',acronyme='%s',libelle='%s' "
                                 " ON DUPLICATE KEY UPDATE used=1, libelle=VALUES(libelle)",
                                 tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_CH: Sauve un CH en base de données                                                                         */
/* Entrée: le domain, le CI                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_CH ( struct DOMAIN *domain, JsonNode *element )
  { if ( !Json_has_member ( element, "tech_id" ) ) return;
    if ( !Json_has_member ( element, "acronyme" ) ) return;
    if ( !Json_has_member ( element, "valeur" ) ) return;
    if ( !Json_has_member ( element, "etat" ) ) return;
    DB_Write ( domain, "UPDATE mnemos_CH as m SET etat='%d', valeur='%d' "
                       "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                       Json_get_bool ( element, "etat" ), Json_get_int ( element, "valeur" ),
                       Json_get_string ( element, "tech_id" ), Json_get_string( element, "acronyme" ) );
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_CH_by_array: Sauve un bistable en base de données                                                          */
/* Entrée: le tech_id, l'acronyme, valeur, dans element                                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_CH_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    Mnemo_sauver_un_CH ( domain, element );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
