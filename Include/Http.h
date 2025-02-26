/******************************************************************************************************************************/
/* Include/Http.h        Déclaration structure internes des WebServices                                                       */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                19.02.2022 20:58:34 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.h
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

#ifndef _HTTP_H_
 #define _HTTP_H_

 #include <glib.h>
 #include <openssl/ssl.h>
 #include <openssl/rand.h>
 #include <libsoup/soup.h>
 #include <uuid/uuid.h>
 #include <json-glib/json-glib.h>
 #include <mysql.h>
 #include <syslog.h>
 #include <jwt.h>
 #include <mosquitto.h>

 #define DATABASE_POOL_SIZE   10

 #include "config.h" /* from autotools */
 #include "Domains.h"
 #include "Database.h"
 #include "Json.h"
 #include "Dls.h"
 #include "Erreur.h"

 struct GLOBAL                                                                                    /* zone de mémoire partagée */
  { gboolean Keep_running;
    gint Top;
    JsonNode *config;                                                                              /* Config globale via file */
    GTree *domaines;                                                                                        /* Tree of DOMAIN */
    struct mosquitto *MQTT_session;                                                            /* Session MQTT vers le broker */
    pthread_mutex_t Nbr_compil_mutex;                                           /* Mutex sur le nombre de compil en parallele */
    gint Nbr_compil;                                                                        /* Nombre de compile en parallele */
   };

/*************************************************** Définitions des prototypes ***********************************************/
 extern JsonNode *Http_Msg_to_Json ( SoupServerMessage *msg );                                                 /* Dans http.c */
 extern JsonNode *Http_Response_Msg_to_Json ( SoupServerMessage *msg );
 extern gint Http_Msg_status_code ( SoupServerMessage *msg );
 extern gchar *Http_Msg_reason_phrase ( SoupServerMessage *msg );
 extern JsonNode *Http_json_node_create ( SoupServerMessage *msg );
 extern void Http_Send_json_response ( SoupServerMessage *msg, gint code, gchar *details, JsonNode *RootNode );
 extern gboolean Http_fail_if_has_not ( struct DOMAIN *domain, gchar *path, SoupServerMessage *msg, JsonNode *request, gchar *name );
 extern gboolean Http_is_authorized ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, gint access_level );
 extern void Http_print_request ( struct DOMAIN *domain, JsonNode *token, gchar *path );
 extern void UUID_New ( gchar *target );

 extern void Copy_thread_io_to_mnemos ( struct DOMAIN *domain );
 extern void MAPPING_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MAPPING_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MAPPING_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_MAPPING_LIST_request_post ( struct DOMAIN *domain, gchar *path, gchar *mappings_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_MAPPING_SEARCH_TXT_request_post ( struct DOMAIN *domain, gchar *path, gchar *mappings_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void RUN_AGENT_START_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_SET_MASTER_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_RESET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_UPGRADE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void AGENT_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void AGENT_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void VISUEL_Handle_one ( struct DOMAIN *domain, JsonNode *source );
 extern void VISUELS_Load_all ( struct DOMAIN *domain );
 extern void VISUELS_Unload_all ( struct DOMAIN *domain );
 extern void VISUEL_Add_etat_to_json ( JsonArray *array, guint index, JsonNode *visuel, gpointer user_data);
 extern void VISUELS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void VISUEL_Update_params ( struct DOMAIN *domain, gchar *tech_id_src, gchar *acronyme_src );

 extern void USER_PROFIL_request_get ( JsonNode *token, SoupServerMessage *msg );
 extern void USER_INVITE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void USER_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void USER_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void USER_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void USER_SET_DOMAIN_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_USERS_WANNA_BE_NOTIFIED_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_USER_CAN_SEND_TXT_CDE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void THREAD_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void THREAD_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void THREAD_DEBUG_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void THREAD_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void THREAD_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void THREAD_HEARTBEAT_set ( struct DOMAIN *domain, JsonNode *request );
 extern void RUN_THREAD_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_LOAD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_CONFIG_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_THREAD_ADD_AI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_AO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_DI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_DO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_WATCHDOG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void ICONS_request_get ( SoupServer *server, SoupServerMessage *msg, const char *path );

 extern void STATUS_request_get ( SoupServer *server, SoupServerMessage *msg, const char *path );

 extern void DOMAIN_LIST_request_get ( JsonNode *token,  SoupServerMessage *msg );
 extern void DOMAIN_STATUS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DOMAIN_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_IMAGE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_IMAGE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DOMAIN_TRANSFER_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_ADD_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_DELETE_request ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern gboolean DOMAIN_Daily_update ( gpointer key, gpointer value, gpointer data );

 extern void SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );

 extern void HISTO_ALIVE_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void HISTO_SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void HISTO_ACQUIT_request_post ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *request );
 extern void HISTO_Handle_one ( struct DOMAIN *domain, JsonNode *source );

 extern void Modbus_Copy_thread_io_to_mnemos ( struct DOMAIN *domain );
 extern void MODBUS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MODBUS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_AI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_AO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_DI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_DO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_MODBUS_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void Phidget_Copy_thread_io_to_mnemos ( struct DOMAIN *domain );
 extern void PHIDGET_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void PHIDGET_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void PHIDGET_SET_IO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_PHIDGET_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void AUDIO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AUDIO_ZONE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );

 extern void IMSGS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void SMSG_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void SHELLY_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void UPS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void TELEINFOEDF_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void METEO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void TABLEAU_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void TABLEAU_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void TABLEAU_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void TABLEAU_MAP_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void TABLEAU_MAP_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void TABLEAU_MAP_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void TABLEAU_MAP_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void MNEMOS_TECH_IDS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MNEMOS_VALIDATE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_MNEMOS_SAVE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void MNEMOS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MNEMOS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void SYNOPTIQUE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void SYNOPTIQUE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void SYNOPTIQUE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void SYNOPTIQUE_SHOW_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void SYNOPTIQUE_SAVE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void SYNOPTIQUE_CLIC_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void SYNOPTIQUE_ACK_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void ARCHIVE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void ARCHIVE_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void ARCHIVE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void ARCHIVE_STATUS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern gboolean ARCHIVE_Daily_update ( gpointer key, gpointer value, gpointer data );
 extern gboolean ARCHIVE_Handle_one ( struct DOMAIN *domain, JsonNode *element );

 extern void DLS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_SOURCE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RENAME_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RENAME_BIT_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RESTART_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RUN_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_COMPIL_ALL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_COMPIL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_PLUGINS_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_CREATE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_LOAD_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );
 extern void Dls_Compil_one ( struct DOMAIN *domain, JsonNode *token, JsonNode *plugin );

 extern void DLS_PARAMS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_PARAMS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void Dls_Apply_params ( struct DOMAIN *domain, JsonNode *PluginNode );

 extern void DLS_PACKAGE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_PACKAGE_SOURCE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_PACKAGE_SAVE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_PACKAGE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_PACKAGE_ADD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_PACKAGE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern gboolean Dls_Apply_package ( struct DOMAIN *domain, JsonNode *PluginNode );

 extern void MESSAGE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MESSAGE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void RUN_HORLOGES_LOAD_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_HORLOGE_ADD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_HORLOGE_ADD_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_HORLOGE_DEL_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void Mnemo_sauver_un_REGISTRE ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_REGISTRE_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_BI ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_BI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_MONO ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_MONO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_CH ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_CH_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_CI ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_CI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_DI ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_DI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_DO ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_DO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_AI ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_AI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_AO ( struct DOMAIN *domain, JsonNode *element );
 extern void Mnemo_sauver_un_AO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);

 extern void Dls_traduire_plugin ( struct DOMAIN *domain, JsonNode *PluginNode );
 extern void Dls_save_plugin ( struct DOMAIN *domain, JsonNode *token, JsonNode *PluginNode );
 extern JsonNode *Rechercher_DICO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme );

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );
 extern void Audit_log ( struct DOMAIN *domain, JsonNode *token, gchar *classe, gchar *format, ... );

 extern void MQTT_Send_to_domain ( struct DOMAIN *domain, gchar *dest, gchar *tag, JsonNode *node );
 extern void MQTT_Send_to_browsers ( struct DOMAIN *domain, gchar *dest, gchar *tag, JsonNode *node );
 extern void MQTT_Allow_one_domain ( struct DOMAIN *domain );
 extern gboolean MQTT_Allow_one_domain_by_tree ( gpointer key, gpointer value, gpointer data );
 extern gboolean MQTT_Start ( void );
 extern void MQTT_Stop ( void );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
