/******************************************************************************************************************************/
/* visuel.c                      Gestion des visuels dans l'API HTTP WebService                                               */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * visuel.c
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
/* RUN_ABONNEMENT_request_post: Enregistre un bit en mémoire                                                                  */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_ABONNEMENT_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "classe"  )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme")) return;

    gchar *classe = Json_get_string ( request, "classe" );
    gchar *tech_id = Json_get_string ( request, "tech_id" );
    gchar *acronyme = Json_get_string ( request, "acronyme" );

    pthread_mutex_lock ( &domain->abonnements_synchro );
    JsonNode *element = g_tree_lookup ( domain->abonnements, request );
    if (element)
     { if(!strcasecmp ( classe, "AI" ))
        { if (Json_has_member ( request, "valeur" ) && Json_has_member ( request, "in_range" ))
           { Json_node_add_double ( element, "valeur",    Json_get_double ( request, "valeur"   ) );
             Json_node_add_bool   ( element, "in_range",  Json_get_bool   ( request, "in_range" ) );
           } else Info_new ( __func__, LOG_WARNING, domain, "Abonnement AI %s:%s: valeur or in_range is missing", tech_id, acronyme );
        } else Info_new ( __func__, LOG_WARNING, domain, "Abonnement classe '%s' is not known", classe );
     }
    else
     { element = request;
       json_node_ref ( request );
       Json_node_add_string ( element, "tag", "DLS_CADRAN" );
       Info_new ( __func__, LOG_INFO, domain, "Abonnement '%s:%s' classe '%s' added in tree", tech_id, acronyme, classe );
       g_tree_insert ( domain->abonnements, element, element );
     }
    pthread_mutex_unlock ( &domain->abonnements_synchro );
    WS_Client_send_to_all ( domain, element  );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
