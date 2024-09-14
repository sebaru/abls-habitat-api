/******************************************************************************************************************************/
/* visuel.c                      Gestion des visuels dans l'API HTTP WebService                                               */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * visuel.c
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
/* ABONNEMENT_Load: Prepare l'arbre des abonnements d'un domaine                                                              */
/* Entrée: le domaine                                                                                                         */
/******************************************************************************************************************************/
 void ABONNEMENT_Load ( struct DOMAIN *domain )
  { domain->abonnements = g_tree_new_full( (GCompareDataFunc) DOMAIN_Comparer_tree_clef_for_bit, domain, NULL, (GDestroyNotify) json_node_unref );
    if (!domain->abonnements)
     { Info_new ( __func__, LOG_ERR, domain, "Unable to load abonnement (g_tree error)" );
       return;
     }
  }
/******************************************************************************************************************************/
/* ABONNEMENT_Unload_all: Décharge les abonnements d'un domaine                                                               */
/* Entrée: le domaine                                                                                                         */
/******************************************************************************************************************************/
 void ABONNEMENT_Unload ( struct DOMAIN *domain )
  { if (!domain->abonnements) return;
    pthread_mutex_lock ( &domain->abonnements_synchro );
    g_tree_destroy ( domain->abonnements );
    domain->abonnements = NULL;
    pthread_mutex_unlock ( &domain->abonnements_synchro );
  }
/******************************************************************************************************************************/
/* ABONNEMENT_Handle_one: Traite un abonnement recu du Master                                                                 */
/* Entrées: le jsonnode représentant le bit interne et sa valeur                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ABONNEMENT_Handle_one ( struct DOMAIN *domain, JsonNode *source )
  { if (!Json_has_member ( source, "classe"  )) return;
    if (!Json_has_member ( source, "tech_id" )) return;
    if (!Json_has_member ( source, "acronyme")) return;

    gchar *classe   = Json_get_string ( source, "classe" );
    gchar *tech_id  = Json_get_string ( source, "tech_id" );
    gchar *acronyme = Json_get_string ( source, "acronyme" );

    pthread_mutex_lock ( &domain->abonnements_synchro );
    JsonNode *element = g_tree_lookup ( domain->abonnements, source );
    if (!element)
     { element = Json_node_create ();
       Json_node_add_string ( element, "classe",    Json_get_string ( source, "classe" ) );
       Json_node_add_string ( element, "tech_id",   Json_get_string ( source, "tech_id" ) );
       Json_node_add_string ( element, "acronyme",  Json_get_string ( source, "acronyme" ) );
       Json_node_add_string ( element, "tag", "DLS_CADRAN" );
       Info_new ( __func__, LOG_INFO, domain, "Abonnement '%s:%s' classe '%s' added in tree", tech_id, acronyme, classe );
       g_tree_insert ( domain->abonnements, element, element );
     }
    pthread_mutex_unlock ( &domain->abonnements_synchro );

    if(!strcasecmp ( classe, "AI" ))
     { if (Json_has_member ( source, "valeur" ) && Json_has_member ( source, "in_range" ) &&
           Json_has_member ( source, "unite" )  && Json_has_member ( source, "libelle" ) )
        { Json_node_add_double ( element, "valeur",   Json_get_double ( source, "valeur"   ) );
          Json_node_add_bool   ( element, "in_range", Json_get_bool   ( source, "in_range" ) );
          Json_node_add_string ( element, "unite",    Json_get_string ( source, "unite"    ) );
          Json_node_add_string ( element, "libelle",  Json_get_string ( source, "libelle"  ) );
          Info_new ( __func__, LOG_DEBUG, domain, "Abonnement '%s:%s' classe '%s' set to %f %s (%s) in_range=%d", tech_id, acronyme, classe,
                     Json_get_double ( element, "valeur" ), Json_get_string ( element, "unite" ),
                     Json_get_string ( element, "libelle" ), Json_get_bool( element, "in_range" )
                   );
        } else Info_new ( __func__, LOG_WARNING, domain, "Abonnement AI '%s:%s': parameter is missing", tech_id, acronyme );
     }
    else if(!strcasecmp ( classe, "CI" ))
     { if (Json_has_member ( source, "valeur" ) && Json_has_member ( source, "multi" ) &&
           Json_has_member ( source, "unite" )  && Json_has_member ( source, "libelle" ) )
        { Json_node_add_int    ( element, "valeur",   Json_get_int    ( source, "valeur"  ) );
          Json_node_add_double ( element, "multi",    Json_get_double ( source, "multi"   ) );
          Json_node_add_string ( element, "unite",    Json_get_string ( source, "unite"   ) );
          Json_node_add_string ( element, "libelle",  Json_get_string ( source, "libelle" ) );
          Info_new ( __func__, LOG_DEBUG, domain, "Abonnement '%s:%s' classe '%s' set to %d*%f %s (%s)", tech_id, acronyme, classe,
                     Json_get_int ( element, "valeur" ), Json_get_double ( element, "multi" ), Json_get_string ( element, "unite" ),
                     Json_get_string ( element, "libelle" )
                   );
        } else Info_new ( __func__, LOG_WARNING, domain, "Abonnement AI '%s:%s': parameter is missing", tech_id, acronyme );
     }
    else if(!strcasecmp ( classe, "CH" ))
     { if (Json_has_member ( source, "valeur" ) && Json_has_member ( source, "etat" ) && Json_has_member ( source, "libelle" ) )
        { Json_node_add_int    ( element, "valeur",   Json_get_int    ( source, "valeur"  ) );
          Json_node_add_bool   ( element, "etat",     Json_get_bool   ( source, "etat"    ) );
          Json_node_add_string ( element, "libelle",  Json_get_string ( source, "libelle" ) );
          Info_new ( __func__, LOG_DEBUG, domain, "Abonnement '%s:%s' classe '%s' set to %d (etat=%d) (%s)", tech_id, acronyme, classe,
                     Json_get_int ( element, "valeur" ), Json_get_bool ( element, "etat" ),
                     Json_get_string ( element, "libelle" )
                   );
        } else Info_new ( __func__, LOG_WARNING, domain, "Abonnement AI '%s:%s': parameter is missing", tech_id, acronyme );
     }
    else if(!strcasecmp ( classe, "REGISTRE" ))
     { if (Json_has_member ( source, "valeur" ) &&
           Json_has_member ( source, "unite" )  && Json_has_member ( source, "libelle" ) )
        { Json_node_add_double ( element, "valeur",   Json_get_double ( source, "valeur"  ) );
          Json_node_add_string ( element, "unite",    Json_get_string ( source, "unite"   ) );
          Json_node_add_string ( element, "libelle",  Json_get_string ( source, "libelle" ) );
          Info_new ( __func__, LOG_DEBUG, domain, "Abonnement '%s:%s' classe '%s' set to %f %s (%s)", tech_id, acronyme, classe,
                     Json_get_double ( element, "valeur" ), Json_get_string ( element, "unite" ),
                     Json_get_string ( element, "libelle" )
                   );
        } else Info_new ( __func__, LOG_WARNING, domain, "Abonnement AI '%s:%s': parameter is missing", tech_id, acronyme );
     }
    else Info_new ( __func__, LOG_WARNING, domain, "Abonnement classe '%s' for '%s:%s' is not known", classe, tech_id, acronyme );
#warning a voir
/*    WS_Client_send_cadran_to_all ( domain, element );*/
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
