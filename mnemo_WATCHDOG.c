/******************************************************************************************************************************/
/* mnemo_WATCHDOG.c        Déclaration des fonctions pour la gestion des Watchdogs                                            */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_WATCHDOG.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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
/* Mnemo_auto_create_WATCHDOG_from_agent: Ajoute un mnemonique dans la base via le tech_id depuis une demande d'un agent      */
/* Entrée: le tech_id, l'acronyme, la description agent                                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_WATCHDOG_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info ( __func__, "mnemo", domain->uuid, LOG_ERR, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *description = Normaliser_chaine ( description_src );                              /* Formatage correct des chaines */
    if ( !description )
     { Info ( __func__, "mnemo", domain->uuid, LOG_ERR, "Normalize error for description." );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,                                                                     /* Requete SQL */
                                 "INSERT INTO mnemos_WATCHDOG SET deletable=0, used=1, tech_id='%s', acronyme='%s', "
                                 "libelle='%s', agent_description='%s' "
                                 "ON DUPLICATE KEY UPDATE used=1, agent_description=VALUES(agent_description)",
                                 tech_id, acro, description, description );
    g_free(acro);
    g_free(description);
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_auto_create_WATCHDOG_from_dls: Ajoute un mnemonique dans la base via le tech_id depuis une demande dls               */
/* Entrée: le tech_id, l'acronyme, le libelle et l'unite et l'archivage                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_WATCHDOG_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info ( __func__, "mnemo", domain->uuid, LOG_ERR, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *libelle = Normaliser_chaine ( libelle_src );                                      /* Formatage correct des chaines */
    if ( !libelle )
     { Info ( __func__, "mnemo", domain->uuid, LOG_ERR, "Normalize error for libelle." );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,                                                                     /* Requete SQL */
                                 "INSERT INTO mnemos_WATCHDOG SET deletable=1, used=1, tech_id='%s', acronyme='%s', libelle='%s' "
                                 "ON DUPLICATE KEY UPDATE used=1, libelle=VALUES(libelle)",
                                 tech_id, acro, libelle );
    g_free(acro);
    g_free(libelle);
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_sauver_un_WATCHDOG_by_array: Sauve le décompte restant d'un watchdog en base de données                              */
/* Entrée: le tech_id, l'acronyme et le temps restant (en dixièmes de seconde), dans element                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_sauver_un_WATCHDOG_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    if ( !Json_has_member ( element, "tech_id" ) ) return;
    if ( !Json_has_member ( element, "acronyme" ) ) return;
    if ( !Json_has_member ( element, "restant" ) ) return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string ( element, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string ( element, "acronyme" ) );
    gint   restant  = Json_get_int ( element, "restant" );
    if (restant < 0) restant = 0;
    if (tech_id && acronyme)
     { DB_Write ( domain, "UPDATE mnemos_WATCHDOG SET restant='%d' WHERE tech_id='%s' AND acronyme='%s'",
                  restant, tech_id, acronyme );
     }
    else Info ( __func__, "mnemo", domain->uuid, LOG_ERR, "Normalize error for watchdog." );
    if (tech_id)  g_free(tech_id);
    if (acronyme) g_free(acronyme);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
