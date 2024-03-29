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
/* VISUEL_copy_in_tree: Enregistre un visuel dans l'arbre des visuels                                                         */
/* Entrées: le visuel au format Json                                                                                          */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static JsonNode *VISUELS_copy_in_tree ( struct DOMAIN *domain, JsonNode *element )
  { if ( !Json_has_member ( element, "tech_id"  ) ) return(NULL);
    if ( !Json_has_member ( element, "acronyme" ) ) return(NULL);
    if ( !Json_has_member ( element, "libelle"  ) ) return(NULL);
    if ( !Json_has_member ( element, "mode"     ) ) return(NULL);
    if ( !Json_has_member ( element, "color"    ) ) return(NULL);
    if ( !Json_has_member ( element, "cligno"   ) ) return(NULL);
    if ( !Json_has_member ( element, "valeur"   ) ) return(NULL);
    if ( !Json_has_member ( element, "disable"  ) ) return(NULL);

    JsonNode *visuel = Json_node_create();
    if (!visuel) return(NULL);

    Json_node_add_string ( visuel, "tech_id",  Json_get_string ( element, "tech_id" ) );
    Json_node_add_string ( visuel, "acronyme", Json_get_string ( element, "acronyme" ) );
    Json_node_add_string ( visuel, "libelle",  Json_get_string ( element, "libelle" ) );
    Json_node_add_string ( visuel, "mode",     Json_get_string ( element, "mode" ) );
    Json_node_add_string ( visuel, "color",    Json_get_string ( element, "color" ) );
    Json_node_add_double ( visuel, "valeur",   Json_get_double ( element, "valeur" ) );
    Json_node_add_bool   ( visuel, "cligno",   Json_get_bool   ( element, "cligno" ) );
    Json_node_add_bool   ( visuel, "disable",  Json_get_bool   ( element, "disable" ) );
    pthread_mutex_lock ( &domain->synchro );
    g_tree_insert ( domain->Visuels, visuel, visuel );
    pthread_mutex_unlock ( &domain->synchro );
    domain->Nbr_visuels++;
    return(visuel);
  }
/******************************************************************************************************************************/
/* VISUEL_save_one_visuel: Enregistre un visuel en base                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void VISUELS_copy_in_tree_by_array ( JsonArray *array, guint index_, JsonNode *visuel, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    VISUELS_copy_in_tree ( domain, visuel );
  }
/******************************************************************************************************************************/
/* VISUELS_save_one_to_db: Enregistre un visuel en base                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean VISUELS_save_one_to_db ( gpointer key, gpointer value, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    JsonNode *visuel = value;
    gchar *tech_id   = Normaliser_chaine ( Json_get_string ( visuel, "tech_id"  ) );
    gchar *acronyme  = Normaliser_chaine ( Json_get_string ( visuel, "acronyme" ) );
    gchar *libelle   = Normaliser_chaine ( Json_get_string ( visuel, "libelle"  ) );
    gchar *mode      = Normaliser_chaine ( Json_get_string ( visuel, "mode"     ) );
    gchar *color     = Normaliser_chaine ( Json_get_string ( visuel, "color"    ) );
    gdouble  valeur  = Json_get_double ( visuel, "valeur" );
    gboolean cligno  = Json_get_bool   ( visuel, "cligno" );
    gboolean disable = Json_get_bool   ( visuel, "disable" );
    DB_Write ( domain, "INSERT INTO mnemos_VISUEL SET tech_id='%s', acronyme='%s', "
                       "libelle='%s', mode='%s', color='%s', valeur='%f', cligno='%d', disable='%d' "
                       "ON DUPLICATE KEY UPDATE libelle=VALUE(libelle), mode=VALUE(mode), color=VALUE(color), "
                       "valeur=VALUE(valeur), cligno=VALUE(cligno), disable=VALUE(disable) ",
                       tech_id, acronyme, libelle, mode, color, valeur, cligno, disable );
    g_free(tech_id);
    g_free(acronyme);
    g_free(libelle);
    g_free(mode);
    g_free(color);
    return(FALSE);
  }
/******************************************************************************************************************************/
/* VISUELS_Load: Charge les visuels d'un domain                                                                               */
/* Entrée: le domaine                                                                                                         */
/******************************************************************************************************************************/
 void VISUELS_Load_all ( struct DOMAIN *domain )
  { domain->Visuels = g_tree_new_full( (GCompareDataFunc) DOMAIN_Comparer_tree_clef_for_bit, domain, NULL, (GDestroyNotify) json_node_unref );
    if (!domain->Visuels)
     { Info_new ( __func__, LOG_ERR, domain, "Unable to load visuels (g_tree error)" );
       return;
     }
    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new ( __func__, LOG_ERR, domain, "Unable to load visuels (JsonNode error)" );
       return;
     }
    DB_Read ( domain, RootNode, "visuels", "SELECT * FROM mnemos_VISUEL" );
    Json_node_foreach_array_element ( RootNode, "visuels", VISUELS_copy_in_tree_by_array, domain );
    json_node_unref ( RootNode );
    Info_new ( __func__, LOG_INFO, domain, "%04d visuels loaded", domain->Nbr_visuels );
  }
/******************************************************************************************************************************/
/* VISUELS_Load: Sauve et Décharge les visuels d'un domain                                                                    */
/* Entrée: le domaine                                                                                                         */
/******************************************************************************************************************************/
 void VISUELS_Unload_all ( struct DOMAIN *domain )
  { if (!domain->Visuels) return;
    pthread_mutex_lock ( &domain->synchro );
    g_tree_foreach ( domain->Visuels, VISUELS_save_one_to_db, domain );
    Info_new ( __func__, LOG_INFO, domain, "%04d visuels saved to DB", domain->Nbr_visuels );
    g_tree_destroy ( domain->Visuels );
    domain->Visuels = NULL;
    domain->Nbr_visuels = 0;
    pthread_mutex_unlock ( &domain->synchro );
  }

/******************************************************************************************************************************/
/* VISUEL_Add_etat_to_json: Ajoute les états de chaque visuels dans le json associé                                           */
/* Entrées : Le visuel, le domain dans user_data                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUEL_Add_etat_to_json ( JsonArray *array, guint index, JsonNode *visuel, gpointer user_data)
  { struct DOMAIN *domain = user_data;
    JsonNode *visuel_source = g_tree_lookup ( domain->Visuels, visuel );
    if (visuel_source)
     { Json_node_add_string ( visuel, "libelle", Json_get_string ( visuel_source, "libelle" ) );
       Json_node_add_string ( visuel, "mode",    Json_get_string ( visuel_source, "mode" ) );
       Json_node_add_string ( visuel, "color",   Json_get_string ( visuel_source, "color" ) );
       Json_node_add_double ( visuel, "valeur",  Json_get_double ( visuel_source, "valeur" ) );
       Json_node_add_bool   ( visuel, "cligno",  Json_get_bool   ( visuel_source, "cligno" ) );
       Json_node_add_bool   ( visuel, "disable", Json_get_bool   ( visuel_source, "disable" ) );
     }
  }
/******************************************************************************************************************************/
/* VISUEL_Handle_one_by_array: Traite un visuel recu du Master                                                                */
/* Entrées: le jsonnode représentant le bit interne et sa valeur                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUEL_Handle_one_by_array ( JsonArray *array, guint index_, JsonNode *source, gpointer user_data )
  { struct WS_AGENT_SESSION *ws_agent = user_data;
    struct DOMAIN *domain = ws_agent->domain;

    if ( !Json_has_member ( source, "tech_id"  ) ) return;
    if ( !Json_has_member ( source, "acronyme" ) ) return;
    if ( !Json_has_member ( source, "libelle"  ) ) return;
    if ( !Json_has_member ( source, "mode"     ) ) return;
    if ( !Json_has_member ( source, "color"    ) ) return;
    if ( !Json_has_member ( source, "valeur"   ) ) return;
    if ( !Json_has_member ( source, "cligno"   ) ) return;
    if ( !Json_has_member ( source, "disable"  ) ) return;

    gchar *tech_id   = Json_get_string ( source, "tech_id" );
    gchar *acronyme  = Json_get_string ( source, "acronyme" );
    gchar *mode      = Json_get_string ( source, "mode" );
    gchar *color     = Json_get_string ( source, "color" );
    gdouble valeur   = Json_get_double ( source, "valeur" );
    gchar *libelle   = Json_get_string ( source, "libelle" );
    gboolean cligno  = Json_get_bool   ( source, "cligno" );
    gboolean disable = Json_get_bool   ( source, "disable" );

    JsonNode *visuel = g_tree_lookup ( domain->Visuels, source );
    if (visuel)
     { Json_node_add_string ( visuel, "libelle",  libelle );
       Json_node_add_string ( visuel, "mode",     mode );
       Json_node_add_string ( visuel, "color",    color );
       Json_node_add_double ( visuel, "valeur",   valeur );
       Json_node_add_bool   ( visuel, "cligno",   cligno );
       Json_node_add_bool   ( visuel, "disable",  disable );
       Info_new ( __func__, LOG_DEBUG, domain, "Visuel '%s:%s' set to '%s' '%s' %f '%d' '%s', disable=%d",
                  tech_id, acronyme, mode, color, valeur, cligno, libelle, disable );
     }
    else
     { Info_new ( __func__, LOG_INFO, domain, "Visuel '%s:%s' unknown. Adding to tree", tech_id, acronyme );
       visuel = VISUELS_copy_in_tree ( domain, source );
     }
    Json_node_add_string ( visuel, "tag", "DLS_VISUEL" );
    WS_Client_send_to_all ( domain, visuel );                                                     /* Envoi a tous les clients */
  }
/******************************************************************************************************************************/
/* VISUELS_DELETE_request: Supprime les visuels en mémoire                                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void VISUELS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    pthread_mutex_lock ( &domain->synchro );
    g_tree_foreach ( domain->Visuels, VISUELS_save_one_to_db, domain );
    Info_new ( __func__, LOG_INFO, domain, "%04d visuels cleared", domain->Nbr_visuels );
    g_tree_remove_all ( domain->Visuels );
    domain->Nbr_visuels = 0;
    pthread_mutex_unlock ( &domain->synchro );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Visuels deleted", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
