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
/* VISUEL_save_one_visuel: Enregistre un visuel en base                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void VISUELS_save_one_visuel ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    if ( !Json_has_member ( element, "mode"     ) ) return;
    if ( !Json_has_member ( element, "libelle"  ) ) return;
    if ( !Json_has_member ( element, "color"    ) ) return;
    if ( !Json_has_member ( element, "cligno"   ) ) return;
    if ( !Json_has_member ( element, "tech_id"  ) ) return;
    if ( !Json_has_member ( element, "acronyme" ) ) return;
    gchar *mode     = Normaliser_chaine ( Json_get_string ( element, "mode" ) );
    gchar *libelle  = Normaliser_chaine ( Json_get_string ( element, "libelle" ) );
    gchar *color    = Normaliser_chaine ( Json_get_string ( element, "color" ) );
    gchar *tech_id  = Normaliser_chaine ( Json_get_string ( element, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string ( element, "acronyme" ) );
    gboolean cligno = Json_get_bool   ( element, "cligno" );
    DB_Write ( domain, "UPDATE mnemos_VISUEL SET mode='%s', libelle='%s', color='%s', cligno='%d' "
                            "WHERE tech_id='%s' AND acronyme='%s'", mode, libelle, color, cligno, tech_id, acronyme );
    g_free(mode);
    g_free(libelle);
    g_free(color);
    g_free(tech_id);
    g_free(acronyme);
  }
/******************************************************************************************************************************/
/* VISUELS_Comparer_clef_thread: Compare deux clef thread dans l'arbre des visuels                                            */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 gint VISUELS_Comparer_clef_thread ( JsonNode *node1, JsonNode *node2, gpointer data )
  { if (!node1) return(-1);
    if (!node2) return(1);
    gchar *tech_id_1 = Json_get_string ( node1, "tech_id" );
    gchar *tech_id_2 = Json_get_string ( node2, "tech_id" );
    if (!tech_id_1) { Info_new( __func__, LOG_ERR, "tech_id1 is NULL", __func__ ); return(-1); }
    if (!tech_id_2) { Info_new( __func__, LOG_ERR, "tech_id2 is NULL", __func__ ); return(1); }
    gint result = strcasecmp ( tech_id_1, tech_id_2 );
    if (result) return(result);
    gchar *acronyme_1 = Json_get_string ( node1, "acronyme" );
    gchar *acronyme_2 = Json_get_string ( node2, "acronyme" );
    if (!acronyme_1) { Info_new( __func__, LOG_ERR, "acronyme1 is NULL", __func__ ); return(-1); }
    if (!acronyme_2) { Info_new( __func__, LOG_ERR, "acronyme2 is NULL", __func__ ); return(1); }
    return( strcasecmp ( acronyme_1, acronyme_2 ) );
  }
/******************************************************************************************************************************/
/* VISUELS_new_to_domain: Ajoute un visuel dans l'arbre des visuels du domaine                                                */
/* Entrée: le domain et le visuel                                                                                             */
/* Sortie: le visuel au format json, or NULL si erreur                                                                        */
/******************************************************************************************************************************/
 static JsonNode *VISUELS_new_to_domain ( struct DOMAIN *domain, JsonNode *element )
  { JsonNode *visuel = Json_node_create();
    if(!visuel) return(NULL);
    Json_node_add_string ( visuel, "tech_id", Json_get_string  ( element, "tech_id" ) );
    Json_node_add_string ( visuel, "acronyme", Json_get_string ( element, "acronyme" ) );
    pthread_mutex_lock ( &domain->synchro );
    g_tree_insert ( domain->Visuels, visuel, visuel );
    domain->Nbr_visuels++;
    pthread_mutex_unlock ( &domain->synchro );
    return(visuel);
  }
/******************************************************************************************************************************/
/* VISUELS_get_from_domain: Recherche un visuel dans l'arbre des visuels                                                      */
/* Entrée: un buffer json avec tech_id/acronyme                                                                               */
/* Sortie: le visuel au format json, or NULL si erreur                                                                        */
/******************************************************************************************************************************/
 static JsonNode *VISUELS_get_from_domain ( struct DOMAIN *domain, JsonNode *source )
  { return ( g_tree_lookup ( domain->Visuels, source ) ); }
/******************************************************************************************************************************/
/* VISUELS_set_one_visuel: Enregistre un visuel en mémoire                                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean VISUELS_set_one_visuel ( struct DOMAIN *domain, JsonNode *element )
  { if ( !Json_has_member ( element, "mode"     ) ) return(FALSE);
    if ( !Json_has_member ( element, "libelle"  ) ) return(FALSE);
    if ( !Json_has_member ( element, "color"    ) ) return(FALSE);
    if ( !Json_has_member ( element, "cligno"   ) ) return(FALSE);
    if ( !Json_has_member ( element, "tech_id"  ) ) return(FALSE);
    if ( !Json_has_member ( element, "acronyme" ) ) return(FALSE);

    gchar *mode     = Json_get_string ( element, "mode" );
    gchar *libelle  = Json_get_string ( element, "libelle" );
    gchar *color    = Json_get_string ( element, "color" );
    gboolean cligno = Json_get_bool   ( element, "cligno" );

    JsonNode *visuel = VISUELS_get_from_domain ( domain, element );
    if (!visuel) { visuel = VISUELS_new_to_domain ( domain, element ); }
    if (!visuel) { Info_new ( __func__, LOG_ERR, "Unable to add new visuel" ); return(FALSE); }

    Json_node_add_string ( visuel, "mode",     mode );
    Json_node_add_string ( visuel, "libelle",  libelle );
    Json_node_add_string ( visuel, "color",    color );
    Json_node_add_bool   ( visuel, "cligno",   cligno );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* VISUEL_request_post: Repond aux requests des visuels                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUELS_request_post ( struct DOMAIN *domain, gchar *instance_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  { /*if (!Http_check_request( msg, session, 6 )) return;*/

    if ( !strcasecmp ( api_tag, "SAVE_ALL" ) && Json_has_member ( request, "visuels" ) )
     { Json_node_foreach_array_element ( request, "visuels", VISUELS_save_one_visuel, domain );
       Http_Send_json_response ( msg, "success", NULL );
     }
    else if ( !strcasecmp ( api_tag, "SET_VISUEL" ) )
     { gboolean retour = VISUELS_set_one_visuel ( domain, request );
       Http_Send_json_response ( msg, (retour ? "success" : "failed"), NULL );
     }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
