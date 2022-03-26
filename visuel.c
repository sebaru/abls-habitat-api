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
 void VISUELS_save_one_visuel ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gchar *domain_uuid = user_data;
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
    DB_Write ( domain_uuid, "UPDATE mnemos_VISUEL SET mode='%s', libelle='%s', color='%s', cligno='%d' "
                            "WHERE tech_id='%s' AND acronyme='%s'", mode, libelle, color, cligno, tech_id, acronyme );
    g_free(mode);
    g_free(libelle);
    g_free(color);
    g_free(tech_id);
    g_free(acronyme);
  }
/******************************************************************************************************************************/
/* VISUEL_request_post: Repond aux requests des visuels                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUELS_request_post ( gchar *domain_uuid, gchar *instance_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  { /*if (!Http_check_request( msg, session, 6 )) return;*/

    if ( !strcasecmp ( api_tag, "SAVE_ALL" ) && Json_has_member ( request, "visuels" ) )
     {
       Json_node_foreach_array_element ( request, "visuels", VISUELS_save_one_visuel, domain_uuid );
       Http_Send_json_response ( msg, "success", NULL );
     }
    else soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
