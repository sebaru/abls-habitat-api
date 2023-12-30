/******************************************************************************************************************************/
/* modbus.c                      Gestion des modbus dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * modbus.c
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
/* Modbus_Copy_thread_io_to_mnemos: Recopie la config IO modbus et met a jour les tables mnemos_xx                            */
/* Entrées: le domaine                                                                                                        */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Modbus_Copy_thread_io_to_mnemos ( struct DOMAIN *domain )
  { gchar requete[512];

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_AI AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_AI AS src ON src.thread_tech_id=map.thread_tech_id AND src.thread_acronyme=map.thread_acronyme "
                 "SET dest.archivage = src.archivage, dest.unite = src.unite, dest.libelle = src.libelle " );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_AO AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_AO AS src ON src.thread_tech_id=map.thread_tech_id AND src.thread_acronyme=map.thread_acronyme "
                 "SET dest.archivage = src.archivage, dest.unite = src.unite, dest.libelle = src.libelle " );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_DI AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_DI AS src ON src.thread_tech_id=map.thread_tech_id AND src.thread_acronyme=map.thread_acronyme "
                 "SET dest.libelle = src.libelle " );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_DO AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_DO AS src ON src.thread_tech_id=map.thread_tech_id AND src.thread_acronyme=map.thread_acronyme "
                 "SET mono=0, dest.libelle = src.libelle " );
    DB_Write ( domain, requete );
  }
/******************************************************************************************************************************/
/* MODBUS_SET_request_post: Appelé depuis libsoup pour éditer ou creer un modbus                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "watchdog" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "max_request_par_sec" )) return;

    gchar *agent_uuid          = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id      = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *hostname            = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description         = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gint   watchdog            = Json_get_int( request, "watchdog" );
    gint   max_request_par_sec = Json_get_int( request, "max_request_par_sec" );

    retour = DB_Write ( domain,
                       "INSERT INTO modbus SET "
                       "agent_uuid='%s', thread_tech_id='%s', hostname='%s', description='%s', watchdog='%d', max_request_par_sec='%d' "
                       "ON DUPLICATE KEY UPDATE agent_uuid=VALUE(agent_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "watchdog=VALUE(watchdog), max_request_par_sec=VALUE(max_request_par_sec) ",
                       agent_uuid, thread_tech_id, hostname, description, watchdog, max_request_par_sec );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(hostname);
    g_free(description);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_string ( request, "thread_classe", "modbus" );
    AGENT_send_to_agent ( domain, NULL, "THREAD_RESTART", request );                               /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* MODBUS_LIST_request_get: Appelé depuis libsoup pour l'URI modbus/list                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (Http_fail_if_has_not ( domain, path, msg, url_param, "classe")) return;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = FALSE;
    gchar *classe = Json_get_string ( url_param, "classe" );
         if (!strcasecmp ( classe, "modbus" ))
          { retour = DB_Read ( domain, RootNode, "modbus", "SELECT modbus.*, agent_hostname FROM modbus INNER JOIN agents USING(agent_uuid)" ); }
    else if (!strcasecmp ( classe, "AI" ))
          { retour = DB_Read ( domain, RootNode, "AI",
                               "SELECT m.*, map.tech_id, map.acronyme FROM modbus_AI AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "AO" ))
          { retour = DB_Read ( domain, RootNode, "AO",
                               "SELECT m.*, map.tech_id, map.acronyme FROM modbus_AO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "DI" ))
          { retour = DB_Read ( domain, RootNode, "DI",
                               "SELECT m.*, map.tech_id, map.acronyme FROM modbus_DI AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "DO" ))
          { retour = DB_Read ( domain, RootNode, "DO",
                               "SELECT m.*, map.tech_id, map.acronyme FROM modbus_DO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* MODBUS_SET_AI_request_post: Change les données d'une analogInput                                                           */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_AI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "modbus_ai_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "min" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "max" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "type_borne" ))   return;

    gint   modbus_ai_id = Json_get_int( request, "modbus_ai_id" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gint   min          = Json_get_int( request, "min" );
    gint   max          = Json_get_int( request, "max" );
    gint   type_borne   = Json_get_int( request, "type_borne" );
    gchar *unite        = Normaliser_chaine ( Json_get_string( request, "unite" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                       "UPDATE modbus_AI SET archivage=%d, min=%d, max=%d, type_borne=%d, unite='%s', libelle='%s' "
                       "WHERE modbus_ai_id=%d", archivage, min, max, type_borne, unite, libelle, modbus_ai_id );

    g_free(libelle);
    g_free(unite);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_classe, thread_tech_id, agent_uuid FROM modbus_AI "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE modbus_ai_id='%d'", modbus_ai_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", RootNode );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Modbus_AI set", NULL );
  }
/******************************************************************************************************************************/
/* MODBUS_SET_AO_request_post: Change les données d'une analogInput                                                           */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_AO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "modbus_ao_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "min" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "max" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "type_borne" ))   return;

    gint   modbus_ao_id = Json_get_int( request, "modbus_ao_id" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gint   min          = Json_get_int( request, "min" );
    gint   max          = Json_get_int( request, "max" );
    gint   type_borne   = Json_get_int( request, "type_borne" );
    gchar *unite        = Normaliser_chaine ( Json_get_string( request, "unite" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                       "UPDATE modbus_AO SET archivage=%d, min=%d, max=%d, type_borne=%d, unite='%s', libelle='%s' "
                       "WHERE modbus_ao_id=%d", archivage, min, max, type_borne, unite, libelle, modbus_ao_id );

    g_free(libelle);
    g_free(unite);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_classe, thread_tech_id, agent_uuid FROM modbus_AO "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE modbus_ao_id='%d'", modbus_ao_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", RootNode );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Modbus_AO set", NULL );
  }
/******************************************************************************************************************************/
/* MODBUS_SET_DI_request_post: Change les données d'une DigitalInput                                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_DI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "modbus_di_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "flip" ))         return;

    gint   modbus_di_id = Json_get_int( request, "modbus_di_id" );
    gboolean flip       = Json_get_bool( request, "flip" );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain, "UPDATE modbus_DI SET libelle='%s', flip='%d' WHERE modbus_di_id=%d", libelle, flip, modbus_di_id );

    g_free(libelle);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_classe, thread_tech_id, agent_uuid FROM modbus_DI "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE modbus_di_id='%d'", modbus_di_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", RootNode );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Modbus_DI set", NULL );
  }
/******************************************************************************************************************************/
/* MODBUS_SET_DO_request_post: Change les données d'une DigitalInput                                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_SET_DO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "modbus_do_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;

    gint   modbus_do_id = Json_get_int( request, "modbus_do_id" );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain, "UPDATE modbus_DO SET libelle='%s' WHERE modbus_do_id=%d", libelle, modbus_do_id );

    g_free(libelle);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_classe, thread_tech_id, agent_uuid FROM modbus_DO "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE modbus_do_id='%d'", modbus_do_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", RootNode );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Modbus_DO set", NULL );
  }
/******************************************************************************************************************************/
/* RUN_MODBUS_ADD_IO_request_post: Ajoute des I/O pour un wago détecté                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_MODBUS_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;

    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_entree_ana" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_entree_tor" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_sortie_ana" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_sortie_tor" )) return;

    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );

    gint nbr_entree_ana = Json_get_int ( request, "nbr_entree_ana" );
    gint nbr_entree_tor = Json_get_int ( request, "nbr_entree_tor" );
    gint nbr_sortie_ana = Json_get_int ( request, "nbr_sortie_ana" );
    gint nbr_sortie_tor = Json_get_int ( request, "nbr_sortie_tor" );
    Info_new ( __func__, LOG_INFO, domain, "Get %03d DI, %03d DO, %03d AI, %03d AO",
               nbr_entree_tor, nbr_sortie_tor, nbr_entree_ana, nbr_sortie_ana );
    gboolean retour = TRUE;
    for (gint cpt=0; cpt<nbr_entree_ana; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_AI SET thread_tech_id='%s', thread_acronyme='AI%03d', num=%d",
                            thread_tech_id, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='AI%03d'",
                            thread_tech_id, cpt );
     }
    for (gint cpt=0; cpt<nbr_sortie_ana; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_AO SET thread_tech_id='%s', thread_acronyme='AO%03d', num=%d",
                            thread_tech_id, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='AO%03d'",
                            thread_tech_id, cpt );
     }
    for (gint cpt=0; cpt<nbr_entree_tor; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_DI SET thread_tech_id='%s', thread_acronyme='DI%03d', num=%d",
                            thread_tech_id, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DI%03d'",
                            thread_tech_id, cpt );
     }
    for (gint cpt=0; cpt<nbr_sortie_tor; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_DO SET thread_tech_id='%s', thread_acronyme='DO%03d', num=%d",
                            thread_tech_id, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DO%03d'",
                            thread_tech_id, cpt );
     }
    g_free(thread_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
