/******************************************************************************************************************************/
/* Include/Http.h        Déclaration structure internes des WebServices                                                       */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                19.02.2022 20:58:34 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Http.h
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

 #define DATABASE_POOL_SIZE   10

 #include "config.h" /* from autotools */
 #include "Domains.h"
 #include "Database.h"
 #include "Json.h"
 #include "Websocket.h"
 #include "Dls.h"
 #include "Erreur.h"

 struct GLOBAL                                                                                    /* zone de mémoire partagée */
  { gboolean Keep_running;
    gint Top;
    pthread_mutex_t nbr_threads_sync;                                                     /* Bit de synchronisation processus */
    gint nbr_threads;                                                                              /* Nombre de request en // */
    JsonNode *config;                                                                              /* Config globale via file */
    GTree *domaines;                                                                                        /* Tree of DOMAIN */
  };


/******************************************************************************************************************************/
/* struct HTTP_CADRAN
  { gchar tech_id[32];
    gchar acronyme[64];
    gchar unite[32];
    gchar classe[12];
    gpointer dls_data;
    gdouble  valeur;
    gboolean in_range;
    gint last_update;
  };*/

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
 extern void Copy_thread_io_to_mnemos_for_classe ( struct DOMAIN *domain, gchar *thread_classe );
 extern void MAPPING_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MAPPING_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MAPPING_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_MAPPING_LIST_request_post ( struct DOMAIN *domain, gchar *path, gchar *mappings_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_MAPPING_SEARCH_TXT_request_post ( struct DOMAIN *domain, gchar *path, gchar *mappings_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void RUN_AGENT_START_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern gboolean AGENT_send_to_agent ( struct DOMAIN *domain, gchar *agent_uuid, gchar *agent_tag, JsonNode *node );
 extern void AGENT_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_SET_MASTER_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_RESET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_UPGRADE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void AGENT_SEND_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void AGENT_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void AGENT_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void VISUEL_Handle_one_by_array ( JsonArray *array, guint index_, JsonNode *source, gpointer user_data );
 extern void VISUELS_Load_all ( struct DOMAIN *domain );
 extern void VISUELS_Unload_all ( struct DOMAIN *domain );
 extern void VISUEL_Add_etat_to_json ( JsonArray *array, guint index, JsonNode *visuel, gpointer user_data);
 extern void VISUELS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

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
 extern void RUN_THREAD_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_LOAD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_CONFIG_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_THREAD_ADD_AI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_AO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_DI_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_DO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_ADD_WATCHDOG_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_THREAD_HEARTBEAT_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void ICONS_request_get ( SoupServer *server, SoupServerMessage *msg, const char *path );

 extern void STATUS_request_get ( SoupServer *server, SoupServerMessage *msg, const char *path );

 extern void DOMAIN_LIST_request_get ( JsonNode *token,  SoupServerMessage *msg );
 extern void DOMAIN_STATUS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DOMAIN_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_IMAGE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_SET_NOTIF_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_IMAGE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DOMAIN_TRANSFER_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_ADD_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DOMAIN_DELETE_request ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern gboolean DOMAIN_Archiver_status ( gpointer key, gpointer value, gpointer data );

 extern void SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );

 extern void HISTO_ALIVE_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void HISTO_SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void HISTO_ACQUIT_request_post ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_HISTO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void MODBUS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MODBUS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_AI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_DI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void MODBUS_SET_DO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_MODBUS_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void PHIDGET_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void PHIDGET_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void PHIDGET_SET_IO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_PHIDGET_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void AUDIO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void IMSGS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void SMSG_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void UPS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void TELEINFOEDF_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void METEO_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );

 extern void MNEMOS_TECH_IDS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void MNEMOS_VALIDATE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_MNEMOS_SAVE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

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
 extern void RUN_ARCHIVE_SAVE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request ) ;
 extern gboolean ARCHIVE_Delete_old_data ( gpointer key, gpointer value, gpointer data );
 extern gboolean ARCHIVE_add_one_enreg ( struct DOMAIN *domain, JsonNode *element );

 extern void DLS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_SOURCE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_DEBUG_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_COMPIL_ALL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_COMPIL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_PLUGINS_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_CREATE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_LOAD_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );

 extern void MESSAGE_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_MESSAGE_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );

 extern void RUN_HORLOGES_LOAD_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_HORLOGE_ADD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_HORLOGE_ADD_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_HORLOGE_DEL_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request );

 extern void ABONNEMENT_Load ( struct DOMAIN *domain );
 extern void ABONNEMENT_Unload ( struct DOMAIN *domain );
 extern void ABONNEMENT_Handle_one_by_array ( JsonArray *array, guint index_, JsonNode *source, gpointer user_data );

 extern void Mnemo_sauver_un_REGISTRE_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_BI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_MONO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_CH_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_CI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_DI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_DO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_AI_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);
 extern void Mnemo_sauver_un_AO_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data);

 extern void Dls_traduire_plugin ( struct DOMAIN *domain, JsonNode *PluginNode );
 extern void Dls_save_plugin ( struct DOMAIN *domain, JsonNode *PluginNode );
 extern JsonNode *Rechercher_DICO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme );

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );
 extern void Audit_log ( struct DOMAIN *domain, JsonNode *token, gchar *classe, gchar *format, ... );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
