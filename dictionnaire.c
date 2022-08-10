/******************************************************************************************************************************/
/* dictionnaire.c        Déclaration des fonctions pour la gestion du dictionnaire                                            */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dictionnaire.c
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

#ifdef bouh
/******************************************************************************************************************************/
/* Rechercher_type_bit: Recherche le type d'un bit matérialisé par son tech_id:acronyme                                       */
/* Entrée: le tech_id et acronyme                                                                                             */
/* Sortie: -1 si erreur                                                                                                       */
/******************************************************************************************************************************/
 gint Rechercher_DICO_type ( gchar *tech_id, gchar *acronyme )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT classe_int FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(-1);
     }
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       return(-1);
     }
    gint result = atoi(db->row[0]);
    Libere_DB_SQL( &db );
    return(result);
  }
#endif
/******************************************************************************************************************************/
/* Rechercher_DICO: Recherche un bit interne dans le dictionnairepar son tech_id:acronyme                                     */
/* Entrée: le tech_id et acronyme                                                                                             */
/* Sortie: Un JsonNode ou NULL si erreur                                                                                      */
/******************************************************************************************************************************/
 JsonNode *Rechercher_DICO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme )
  { JsonNode *result = Json_node_create ();
    if (!result) return(NULL);

    gboolean retour = DB_Read ( domain, result, NULL,
                                "SELECT * FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme
                              );
    if (!retour)
     { Info_new ( __func__, LOG_ERR, domain, "DB Error for '%s:%s' dans le dictionnaire", tech_id, acronyme );
       json_node_unref(result);
       result = NULL;
     }
    return(result);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
