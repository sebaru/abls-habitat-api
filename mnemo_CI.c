/******************************************************************************************************************************/
/* mnemo_CI.c      Déclaration des fonctions pour la gestion des compteurs d'impulsions                                       */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                     mar. 07 déc. 2010 17:26:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_CI.c
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
/* Mnemo_auto_create_CI: Ajout ou modifie le mnemo en parametre                                                               */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_CI ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src, gdouble multi )
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
                                 "INSERT INTO mnemos_CI SET tech_id='%s',acronyme='%s',libelle='%s',unite='%s',multi='%f' "
                                 " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), unite=VALUES(unite), multi=VALUES(multi)",
                                 tech_id, acro, libelle, unite, multi );
    g_free(unite);
    g_free(libelle);
    g_free(acro);
    return(retour);
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_CI_by_array: Sauve un bistable en base de données                                                          */
/* Entrée: le tech_id, l'acronyme, valeur, dans element                                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_CI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    if ( !Json_has_member ( element, "tech_id"  ) ) return;
    if ( !Json_has_member ( element, "acronyme" ) ) return;
    if ( !Json_has_member ( element, "etat"     ) ) return;
    if ( !Json_has_member ( element, "valeur"   ) ) return;
    DB_Write ( domain, "UPDATE mnemos_CI as m SET etat='%d', valeur='%d' "
                       "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                       Json_get_bool ( element, "etat" ), Json_get_int ( element, "valeur" ),
                       Json_get_string ( element, "tech_id" ), Json_get_string( element, "acronyme" ) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
