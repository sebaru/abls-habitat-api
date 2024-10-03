/******************************************************************************************************************************/
/* smsg.c                      Gestion des smsg dans l'API HTTP WebService                                                    */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * smsg.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
/* SMSG_SET_request_post: Appelé depuis libsoup pour éditer ou creer un smsg                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SMSG_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))             return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_service_name" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_consumer_key" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_application_key" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ovh_application_secret" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))            return;

    gchar *agent_uuid             = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id         = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *description            = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *ovh_service_name       = Normaliser_chaine ( Json_get_string( request, "ovh_service_name" ) );
    gchar *ovh_application_key    = Normaliser_chaine ( Json_get_string( request, "ovh_application_key" ) );
    gchar *ovh_application_secret = Normaliser_chaine ( Json_get_string( request, "ovh_application_secret" ) );
    gchar *ovh_consumer_key       = Normaliser_chaine ( Json_get_string( request, "ovh_consumer_key" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO smsg SET agent_uuid='%s', thread_tech_id=UPPER('%s'), "
                        "ovh_service_name='%s', ovh_application_key='%s', ovh_application_secret='%s', ovh_consumer_key='%s', description='%s' "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), "
                        "ovh_service_name=VALUES(ovh_service_name), ovh_application_key=VALUES(ovh_application_key), "
                        "ovh_application_secret=VALUES(ovh_application_secret), ovh_consumer_key=VALUES(ovh_consumer_key),"
                        "description=VALUES(description)",
                        agent_uuid, thread_tech_id, ovh_service_name, ovh_application_key, ovh_application_secret, ovh_consumer_key ,description );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(description);
    g_free(ovh_service_name);
    g_free(ovh_application_key);
    g_free(ovh_application_secret);
    g_free(ovh_consumer_key);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_string ( request, "thread_classe", "smsg" );
    MQTT_Send_to_domain ( domain, "agents", "THREAD_RESTART", request );                           /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
