/******************************************************************************************************************************/
/* audio.c                      Gestion des audio dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                14.06.2022 21:02:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audio.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
/* AUDIO_SET_request_post: Appelé depuis libsoup pour éditer ou creer un audio                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid"     ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "language"       ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "device"         ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description"    ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "volume"         ))  return;

    g_strcanon ( Json_get_string( request, "thread_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *agent_uuid      = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *language        = Normaliser_chaine ( Json_get_string( request, "language" ) );
    gchar *device          = Normaliser_chaine ( Json_get_string( request, "device" ) );
    gchar *description     = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gint   volume          = Json_get_int( request, "volume" );

    retour = DB_Write ( domain,
                        "INSERT INTO audio SET agent_uuid='%s', thread_tech_id=UPPER('%s'), language='%s', device='%s', description='%s', "
                        "volume=%d "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), language=VALUES(language), device=VALUES(device),"
                        "description=VALUES(description), volume=VALUES(volume)",
                        agent_uuid, thread_tech_id, language, device, description, volume );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(description);
    g_free(device);
    g_free(language);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_string ( request, "thread_classe", "audio" );
    MQTT_Send_to_domain ( domain, "agents", "THREAD_RESTART", request );                           /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* AUDIO_ZONES_LIST_request_get: Liste les zones audio du domaine                                                             */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_ZONES_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "audio_zones", "SELECT * FROM audio_zones " );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* AUDIO_ZONES_SET_request_post: Appelé depuis libsoup pour éditer ou creer une zone audio                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_ZONES_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "audio_zone_name" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description"     ))  return;

    gchar *audio_zone_name = Normaliser_chaine ( Json_get_string( request, "audio_zone_name" ) );
    gchar *description     = Normaliser_chaine ( Json_get_string( request, "description" ) );

    if (Json_has_member ( request, "audio_zone_id" ) )
     { gint audio_zone_id = Json_get_int ( request, "audio_zone_id" );
       DB_Read ( domain, request, NULL, "SELECT 1 AS found FROM audio_zones WHERE audio_zone_id='%d'", audio_zone_id );

       if ( !Json_has_member ( request, "found" ) )
        { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Zone Audio unknown", NULL ); }
       else
        { gboolean retour = DB_Write ( domain, "UPDATE audio_zones SET audio_zone_name='%s', description='%s' WHERE audio_zone_id='%d'",
                                       audio_zone_name, description, audio_zone_id );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); }
          else Http_Send_json_response ( msg, SOUP_STATUS_OK, "Zone Audio updated", NULL );
        }
     }
    else
     { gboolean retour = DB_Write ( domain, "INSERT INTO audio_zones SET audio_zone_name='%s', description='%s'",
                                    audio_zone_name, description );                                               /* Création */
       if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); }
       else Http_Send_json_response ( msg, SOUP_STATUS_OK, "Zone Audio created", NULL );
     }

    g_free(audio_zone_name);
    g_free(description);
  }
/******************************************************************************************************************************/
/* AUDIO_ZONES_DELETE_request: Supprime une zone audio                                                                        */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_ZONES_DELETE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "audio_zone_name" )) return;

    gchar *audio_zone_name = Normaliser_chaine ( Json_get_string ( request, "audio_zone_name" ) );
    if (!audio_zone_name) return;

    gboolean retour = DB_Read ( domain, request, NULL, "SELECT 1 AS found FROM audio_zones WHERE audio_zone_name='%s'", audio_zone_name );

    if ( ! (retour && Json_has_member ( request, "found" )) )
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Zone Audio not found", NULL ); goto end; }

    retour &= DB_Write ( domain, "UPDATE msgs SET audio_zone_name = 'DEFAULT' WHERE audio_zone_name='%s'", audio_zone_name );
    retour &= DB_Write ( domain, "DELETE FROM audio_zones WHERE audio_zone_name='%s'", audio_zone_name );
    if (!retour)
         { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); }
    else { Http_Send_json_response ( msg, SOUP_STATUS_OK, "Zone audio deleted", NULL ); }

end:
    g_free(audio_zone_name);
  }
/******************************************************************************************************************************/
/* AUDIO_ZONE_GET_request_get: Donne les thread_tech_id associés à une zone de diffusion                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_ZONE_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/
    if (Http_fail_if_has_not ( domain, path, msg, url_param, "audio_zone_name" ))   return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *audio_zone_name = Normaliser_chaine ( Json_get_string ( url_param, "audio_zone_name" ) );
    if (!audio_zone_name)
     { Http_Send_json_response ( msg, FALSE, "Memory error", RootNode ); return; }
    gboolean retour = DB_Read ( domain, RootNode, "audio_zone_map",
                                "SELECT m.audio_zone_map_id, m.thread_tech_id, "
                                "       z.audio_zone_id, z.audio_zone_name, "
                                "       t.description AS thread_description, a.agent_hostname "
                                "FROM `audio_zone_map` AS m "
                                "INNER JOIN `audio_zones` AS z USING (`audio_zone_id`) "
                                "INNER JOIN `threads` AS t USING (`thread_tech_id`) "
                                "INNER JOIN `agents` AS a USING (`agent_uuid`) "
                                "WHERE audio_zone_name='%s'", audio_zone_name );
    g_free(audio_zone_name);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* AUDIO_ZONE_ADD_request_post: Appelé depuis libsoup pour ajouter un thread dans une zone de diffusion                       */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_ZONE_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "audio_zone_name" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id"  ))  return;

    gchar *audio_zone_name = Normaliser_chaine ( Json_get_string( request, "audio_zone_name" ) );
    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );

    gboolean retour = DB_Write ( domain, "INSERT INTO audio_zone_map SET "
                                 "audio_zone_id=(SELECT audio_zone_id FROM audio_zones WHERE audio_zone_name='%s'), "
                                 "thread_tech_id='%s'",
                                 audio_zone_name, thread_tech_id );                                               /* Création */
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); }
    else Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread added", NULL );

    g_free(audio_zone_name);
    g_free(thread_tech_id);
  }
/******************************************************************************************************************************/
/* AUDIO_ZONE_DELETE_request: Supprime un mapping audio                                                                       */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void AUDIO_ZONE_DELETE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "audio_zone_map_id" )) return;

    gint audio_zone_map_id = Json_get_int ( request, "audio_zone_map_id" );
    gboolean retour = DB_Write ( domain, "DELETE FROM audio_zone_map WHERE audio_zone_map_id='%d'", audio_zone_map_id );
    if (!retour)
         { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); }
    else { Http_Send_json_response ( msg, SOUP_STATUS_OK, "Zone audio deleted", NULL ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
