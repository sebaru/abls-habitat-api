/******************************************************************************************************************************/
/* mnemo_AI.c        Déclaration des fonctions pour la gestion des Analog Input                                               */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                      sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_AI.c
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
/* Mnemo_auto_create_AI: Ajoute un mnemonique dans la base via le tech_id                                                     */
/* Entrée: le tech_id, l'acronyme, le libelle et l'unite                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_AI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src,
                                 gchar *unite_src, gint archivage )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *libelle = NULL;
    if (libelle_src)
     { libelle = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
       if ( !libelle )
        { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
          g_free(acro);
          return(FALSE);
        }
     }

    gchar *unite = NULL;
    if (unite_src)
     { unite = Normaliser_chaine ( unite_src );                                              /* Formatage correct des chaines */
       if ( !unite )
        { Info_new ( __func__, LOG_ERR, domain, "Normalize error for unite." );
          g_free(libelle);
          g_free(acro);
          return(FALSE);
        }
     }

    gchar requete[512];
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_AI SET deletable='%d', tech_id='%s',acronyme='%s', archivage='%d' ",
                deletable, tech_id, acro, archivage );
    g_free(acro);

    if (libelle)
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",libelle='%s'", libelle );
       g_strlcat ( requete, add, sizeof(requete) );
     }

    if (unite)
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",unite='%s'", unite );
       g_strlcat ( requete, add, sizeof(requete) );
     }

    g_strlcat ( requete, " ON DUPLICATE KEY UPDATE acronyme=VALUES(acronyme) ", sizeof(requete) );

    if (unite)
     { g_strlcat ( requete, ",unite=VALUES(unite)", sizeof(requete) );
       g_free(unite);
     }

    if (libelle)
     { g_strlcat ( requete, ",libelle=VALUES(libelle)", sizeof(requete) );
       g_free(libelle);
     }

    gboolean retour = DB_Write ( domain, requete );
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_AI_by_array: Sauve un bit en base de données                                                               */
/* Entrée: le tech_id, l'acronyme, valeur, dans element                                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_AI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    if ( !Json_has_member ( element, "tech_id" ) ) return;
    if ( !Json_has_member ( element, "acronyme" ) ) return;
    if ( !Json_has_member ( element, "valeur" ) ) return;
    DB_Write ( domain, "UPDATE mnemos_AI as m SET valeur='%f' "
                       "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                       Json_get_double ( element, "valeur" ),
                       Json_get_string ( element, "tech_id" ), Json_get_string( element, "acronyme" ) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
