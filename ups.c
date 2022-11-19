/******************************************************************************************************************************/
/* ups.c                      Gestion des ups dans l'API HTTP WebService                                                      */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                19.06.2022 09:24:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ups.c
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
/* UPS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un ups                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UPS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "host" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "name" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "admin_username" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "admin_password" )) return;

    Json_node_add_string ( request, "thread_classe", "ups" );
    gchar *agent_uuid     = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *host           = Normaliser_chaine ( Json_get_string( request, "host" ) );
    gchar *name           = Normaliser_chaine ( Json_get_string( request, "name" ) );
    gchar *admin_username = Normaliser_chaine ( Json_get_string( request, "admin_username" ) );
    gchar *admin_password = Normaliser_chaine ( Json_get_string( request, "admin_password" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO ups SET agent_uuid='%s', thread_tech_id=UPPER('%s'), "
                        "host='%s', name='%s', admin_username='%s', admin_password='%s' "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), "
                        "host=VALUES(host), name=VALUES(name), admin_username=VALUES(admin_username), admin_password=VALUES(admin_password) ",
                        agent_uuid, thread_tech_id, host, name, admin_username, admin_password );

    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "LOAD",            "Charge onduleur", "%", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "REALPOWER",       "Charge onduleur", "W", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "BATTERY_CHARGE",  "Charge batterie", "%", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "INPUT_VOLTAGE",   "Tension d'entrée", "V", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "BATTERY_RUNTIME", "Durée de batterie restante", "s", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "BATTERY_VOLTAGE", "Tension batterie", "V", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "INPUT_HZ",        "Fréquence d'entrée", "HZ", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "OUTPUT_CURRENT",  "Courant de sortie", "A", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "OUTPUT_HZ",       "Fréquence de sortie", "HZ", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI ( domain, FALSE, thread_tech_id, "OUTPUT_VOLTAGE",  "Tension de sortie", "V", ARCHIVE_1_MIN );

    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "OUTLET_1_STATUS",  "Statut de la prise n°1" );
    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "OUTLET_2_STATUS",  "Statut de la prise n°2" );
    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "UPS_ONLINE",       "UPS Online" );
    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "UPS_CHARGING",     "UPS en charge" );
    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "UPS_ON_BATT",      "UPS sur batterie" );
    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "UPS_REPLACE_BATT", "Batteries UPS a changer" );
    Mnemo_auto_create_DI ( domain, FALSE, thread_tech_id, "UPS_ALARM",        "UPS en alarme !" );

    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "LOAD_OFF",        "Coupe la sortie ondulée" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "LOAD_ON",         "Active la sortie ondulée" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "OUTLET_1_OFF",    "Désactive la prise n°1" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "OUTLET_1_ON",     "Active la prise n°1" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "OUTLET_2_OFF",    "Désactive la prise n°2" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "OUTLET_2_ON",     "Active la prise n°2" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "START_DEEP_BAT",  "Active un test de décharge profond" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "START_QUICK_BAT", "Active un test de décharge léger" );
    Mnemo_auto_create_DO ( domain, FALSE, thread_tech_id, "STOP_TEST_BAT",   "Stop le test de décharge batterie" );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(host);
    g_free(name);
    g_free(admin_username);
    g_free(admin_password);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    AGENT_send_to_agent ( domain, NULL, "THREAD_STOP", request );                                  /* Stop sent to all agents */
    AGENT_send_to_agent ( domain, Json_get_string( request, "agent_uuid" ), "THREAD_START", request );               /* Start */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
