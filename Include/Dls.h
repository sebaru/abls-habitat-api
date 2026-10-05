/******************************************************************************************************************************/
/* Include/Dls.h                  Définitions des constantes programme DLS                                                    */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                14.07.2022 21:43:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls.h
 * This file is part of Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

#ifndef _DLS_H_
 #define _DLS_H_

 #define DLS_MONITOR_WATCHER_TTL 300     /* 30 secondes sans keepalive du navigateur et l'observateur est oublié (en top 10Hz) */
 #define ARCHIVE_NONE           0
 #define ARCHIVE_5_SEC          50
 #define ARCHIVE_1_MIN          600
 #define ARCHIVE_1_HEURE        36000
 #define ARCHIVE_1_JOUR         864000

 enum
  { MSG_ETAT,                                                                            /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_NOTIF,
    MSG_DANGER,
    MSG_DERANGEMENT,
    NBR_TYPE_MSG
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern gboolean Dls_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode );
 extern void DLS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_SOURCE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RENAME_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RENAME_BIT_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_ENABLE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RESTART_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_REMAP_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_RELOAD_HORLOGES_TICK_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_MONITOR_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param );
 extern void DLS_MONITOR_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void Dls_monitor_init ( struct DOMAIN *domain );
 extern void Dls_monitor_end ( struct DOMAIN *domain );
 extern gboolean Dls_monitor_check_all_tech_id ( gpointer domain_uuid, gpointer value, gpointer user_data );
 extern void DLS_MONITOR_Handle_one ( struct DOMAIN *domain, gchar *tech_id, JsonNode *request );
 extern void DLS_COMPIL_ALL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void DLS_COMPIL_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request );
 extern void RUN_DLS_PLUGINS_request_get ( struct DOMAIN *domain, gchar *path, struct ABLS_HEADERS *abls_headers, SoupServerMessage *msg, JsonNode *url_param );
 extern void RUN_DLS_LOAD_request_get ( struct DOMAIN *domain, gchar *path, struct ABLS_HEADERS *abls_headers, SoupServerMessage *msg, JsonNode *url_param );
 extern void Dls_Send_Reload_to_master ( struct DOMAIN *domain, gchar *tech_id );
 extern void Dls_Compil_one ( struct DOMAIN *domain, JsonNode *token, JsonNode *plugin );
 extern gboolean Dls_create_agent_plugin ( struct DOMAIN *domain, gchar *tech_id, gchar *description, gchar *agent_classe );
 extern gboolean Dls_remove_plugin ( struct DOMAIN *domain, gchar *tech_id );
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
 extern void Dls_traduire_plugin ( struct DOMAIN *domain, JsonNode *PluginNode );
 extern void Dls_save_plugin ( struct DOMAIN *domain, JsonNode *token, JsonNode *PluginNode );
 extern gint Traduire_DLS( gchar *tech_id );                                                                 /* Dans Interp.c */
 extern gboolean Mnemo_auto_create_BI ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint groupe );
 extern gboolean Mnemo_auto_create_MONO ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AI_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AI_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src,
                                                    gchar *unite_src, gint archivage );
 extern gboolean Mnemo_auto_create_AO_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_AO_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src,
                                                    gchar *unite_src, gint archivage );
 extern gboolean Mnemo_auto_create_DI_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *map_sms_src );
 extern gboolean Mnemo_auto_create_DI_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src );
 extern gboolean Mnemo_auto_create_DO_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_DO_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src, gboolean mono );
 extern gboolean Mnemo_auto_create_HORLOGE_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_HORLOGE_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src );
 extern gboolean Mnemo_auto_create_TEMPO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_WATCHDOG_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_WATCHDOG_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src );
 extern gboolean Mnemo_auto_create_REGISTRE ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Mnemo_auto_create_CI_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Mnemo_auto_create_CI_from_agent ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *description_src,
                                                    gchar *unite_src, gint archivage );
 extern gboolean Mnemo_auto_create_CH ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Mnemo_auto_create_MSG ( struct DOMAIN *domain, gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src,
                                         gint typologie, gint groupe, gint notif_sms, gint notif_chat, gint freeze, gchar *audio_zone );
 extern gboolean Mnemo_auto_create_VISUEL ( struct DOMAIN *domain, JsonNode *plugin, gchar *acronyme, gchar *libelle_src,
                                            gchar *forme_src, gchar *mode_src, gchar *couleur_src,
                                            gdouble min, gdouble max, gdouble seuil_ntb, gdouble seuil_nb, gdouble seuil_nh, gdouble seuil_nth,
                                            gint nb_decimal, gchar *input_tech_id_src, gchar *input_acronyme_src, gint rw );
 extern gboolean Synoptique_auto_create_MOTIF ( struct DOMAIN *domain, JsonNode *plugin, gchar *target_tech_id_src, gchar *target_acronyme_src, gint place );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
