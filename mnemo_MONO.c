/******************************************************************************************************************************/
/* Mnemo_MONO.c            Déclaration des fonctions pour la gestion des booleans                                             */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                24.06.2019 22:07:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_MONO.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
/* Mnemo_auto_create_MONO: Ajoute un mnemonique dans la base via le tech_id                                                   */
/* Entrée: le tech_id, l'acronyme, le libelle                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_MONO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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
                                 "INSERT INTO mnemos_MONO SET deletable='%d', used=1, tech_id='%s',acronyme='%s', libelle='%s' "
                                 "ON DUPLICATE KEY UPDATE used=1, libelle=VALUES(libelle)",
                                 deletable, tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_MONO: Sauve un bit en base de données                                                                      */
/* Entrée: le tech_id, l'acronyme, valeur, dans element                                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_MONO ( struct DOMAIN *domain, JsonNode *element )
  { if ( !Json_has_member ( element, "tech_id" ) ) return;
    if ( !Json_has_member ( element, "acronyme" ) ) return;
    if ( !Json_has_member ( element, "etat" ) ) return;
    DB_Write ( domain, "UPDATE mnemos_MONO as m SET etat='%d' "
                       "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                       Json_get_bool ( element, "etat" ),
                       Json_get_string ( element, "tech_id" ), Json_get_string( element, "acronyme" ) );
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_MONO_by_array: Sauve un bit en base de données                                                             */
/* Entrée: le tech_id, l'acronyme, valeur, dans element                                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_MONO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    Mnemo_sauver_un_MONO ( domain, element );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
