/******************************************************************************************************************************/
/* Mnemo_DO.c        Déclaration des fonctions pour la gestion des Entrée TOR                                                 */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_DO.c
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
/* Mnemo_auto_create_DO: Ajoute un mnemo DO en base                                                                           */
/* Entrée: les parametres du mnemo                                                                                            */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_DO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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
                                 "INSERT INTO mnemos_DO SET deletable='%d', tech_id='%s',acronyme='%s',libelle='%s' "
                                 " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                                 deletable, tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

    return (retour);
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Updater_confDO: Mise a jour des valeurs des DigitalOutput en base                                                          */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_DO( void )
  { GSList *liste;
    gint cpt;

    cpt = 0;
    liste = Partage->Dls_data_DO;
    while ( liste )
     { struct DLS_DO *dout = liste->data;
       SQL_Write_new( "UPDATE mnemos_DO as m SET etat='%d' "
                      "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                      dout->etat, dout->tech_id, dout->acronyme );
       liste = g_slist_next(liste);
       cpt++;
     }

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d DO updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_DO_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_DO_to_json ( JsonNode *element, struct DLS_DO *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
