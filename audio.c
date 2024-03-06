/******************************************************************************************************************************/
/* audio.c                      Gestion des audio dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                14.06.2022 21:02:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audio.c
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
    AGENT_send_to_agent ( domain, NULL, "THREAD_RESTART", request );                               /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
