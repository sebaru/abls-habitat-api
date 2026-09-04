/******************************************************************************************************************************/
/* modbus.c                      Gestion des modbus dans l'API HTTP WebService                                                */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                29.04.2022 20:46:47 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * modbus.c
 * This file is part of Abls-Habitat
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* Modbus_load: Charge la configuration d'un agent modbus                                                                     */
/* Entrées: le domaine, les headers d'agent et le node de reponse                                                             */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                               */
/******************************************************************************************************************************/
 gboolean Modbus_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM modbus WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    return(TRUE);
  }
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
                 "INNER JOIN modbus_AI AS src ON src.agent_tech_id=map.agent_tech_id AND src.agent_acronyme=map.agent_acronyme "
                 "SET dest.archivage = src.archivage, dest.unite = src.unite, dest.libelle = src.libelle " );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_AO AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_AO AS src ON src.agent_tech_id=map.agent_tech_id AND src.agent_acronyme=map.agent_acronyme "
                 "SET dest.archivage = src.archivage, dest.unite = src.unite, dest.libelle = src.libelle " );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_DI AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_DI AS src ON src.agent_tech_id=map.agent_tech_id AND src.agent_acronyme=map.agent_acronyme "
                 "SET dest.archivage = src.archivage, dest.libelle = src.libelle " );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_DO AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN modbus_DO AS src ON src.agent_tech_id=map.agent_tech_id AND src.agent_acronyme=map.agent_acronyme "
                 "SET mono=0, dest.archivage = src.archivage, dest.libelle = src.libelle " );
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

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "watchdog" ))            return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "max_request_par_sec" )) return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid         = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id       = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *hostname            = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description         = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gint   watchdog            = Json_get_int( request, "watchdog" );
    gint   max_request_par_sec = Json_get_int( request, "max_request_par_sec" );

    retour = DB_Write ( domain,
                       "INSERT INTO modbus SET "
                       "server_uuid='%s', agent_tech_id=UPPER('%s'), hostname='%s', description='%s', watchdog='%d', max_request_par_sec='%d' "
                       "ON DUPLICATE KEY UPDATE server_uuid=VALUE(server_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "watchdog=VALUE(watchdog), max_request_par_sec=VALUE(max_request_par_sec) ",
                       server_uuid, agent_tech_id, hostname, description, watchdog, max_request_par_sec );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(hostname);
    g_free(description);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "MODBUS", "Modbus thread configured: agent=%s, hostname=%s",
           Json_get_string( request, "agent_tech_id" ),
                Json_get_string( request, "hostname" ) );
    Json_add_string ( request, "agent_classe", "modbus" );
    MQTT_Send_to_domain ( domain, request, "AGENT/%s/RESTART", Json_get_string( request, "agent_tech_id" ) );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* MODBUS_LIST_request_get: Appelé depuis libsoup pour l'URI modbus/list                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void MODBUS_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

        gboolean retour = DB_Read ( domain, RootNode, "modbus",
                "SELECT m.*, s.agent_tech_id AS server_hostname, "
                "       m.heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive "
                "FROM modbus AS m INNER JOIN server AS s USING(server_uuid) "
                "ORDER BY s.agent_tech_id, m.agent_tech_id" );
        Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
      }
    /******************************************************************************************************************************/
    /* MODBUS_GET_request_get: Donne la configuration et les I/O d'un agent modbus                                               */
    /* Entrée: Les paramètres libsoup                                                                                             */
    /* Sortie: néant                                                                                                              */
    /******************************************************************************************************************************/
     void MODBUS_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
      { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
        Http_print_request ( domain, token, path );

        if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id" )) return;
        gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
        if (!agent_tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", NULL ); return; }

        JsonNode *RootNode = Http_json_node_create (msg);
        if (!RootNode) { g_free(agent_tech_id); Http_Send_json_response ( msg, FALSE, "Memory error", NULL ); return; }

        gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT * FROM modbus WHERE agent_tech_id='%s' LIMIT 1", agent_tech_id );
        if (retour && !Json_has_member ( RootNode, "agent_tech_id" ))
         { g_free(agent_tech_id);
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Modbus agent not found", RootNode );
       return;
         }

        retour &= DB_Read ( domain, RootNode, "AI", "SELECT m.*, map.tech_id, map.acronyme, map.mapping_id FROM modbus_AI AS m LEFT JOIN mappings AS map ON m.agent_tech_id=map.agent_tech_id AND m.agent_acronyme=map.agent_acronyme WHERE m.agent_tech_id='%s'", agent_tech_id );
        retour &= DB_Read ( domain, RootNode, "AO", "SELECT m.*, map.tech_id, map.acronyme, map.mapping_id FROM modbus_AO AS m LEFT JOIN mappings AS map ON m.agent_tech_id=map.agent_tech_id AND m.agent_acronyme=map.agent_acronyme WHERE m.agent_tech_id='%s'", agent_tech_id );
        retour &= DB_Read ( domain, RootNode, "DI", "SELECT m.*, map.tech_id, map.acronyme, map.mapping_id FROM modbus_DI AS m LEFT JOIN mappings AS map ON m.agent_tech_id=map.agent_tech_id AND m.agent_acronyme=map.agent_acronyme WHERE m.agent_tech_id='%s'", agent_tech_id );
        retour &= DB_Read ( domain, RootNode, "DO", "SELECT m.*, map.tech_id, map.acronyme, map.mapping_id FROM modbus_DO AS m LEFT JOIN mappings AS map ON m.agent_tech_id=map.agent_tech_id AND m.agent_acronyme=map.agent_acronyme WHERE m.agent_tech_id='%s'", agent_tech_id );
        g_free(agent_tech_id);

        Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
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
    if (Http_fail_if_has_not ( domain, path, msg, request, "borne" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ed" ))           return;

    gint   modbus_ai_id = Json_get_int( request, "modbus_ai_id" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gint   min          = Json_get_int( request, "min" );
    gint   max          = Json_get_int( request, "max" );
    gint   type_borne   = Json_get_int( request, "type_borne" );
    gchar *borne        = Normaliser_chaine ( Json_get_string( request, "borne" ) );
    gchar *ed           = Normaliser_chaine ( Json_get_string( request, "ed" ) );
    gchar *unite        = Normaliser_chaine ( Json_get_string( request, "unite" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                       "UPDATE modbus_AI SET archivage=%d, min=%d, max=%d, type_borne=%d, borne='%s', ed='%s', unite='%s', libelle='%s' "
                       "WHERE modbus_ai_id=%d", archivage, min, max, type_borne, borne, ed, unite, libelle, modbus_ai_id );

    g_free(libelle);
    g_free(unite);
    g_free(borne);
    g_free(ed);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "MODBUS", "Modbus AI configured: min=%d, max=%d, archivage=%d", min, max, archivage );
    JsonNode *RootNode = Json_create();
    DB_Read ( domain, RootNode, NULL, "SELECT agent_tech_id FROM modbus_AI WHERE modbus_ai_id='%d'", modbus_ai_id );
    MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/RESTART", Json_get_string( RootNode, "agent_tech_id" ) );
    Json_unref(RootNode);
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
    if (Http_fail_if_has_not ( domain, path, msg, request, "borne" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ed" ))           return;

    gint   modbus_ao_id = Json_get_int( request, "modbus_ao_id" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gint   min          = Json_get_int( request, "min" );
    gint   max          = Json_get_int( request, "max" );
    gint   type_borne   = Json_get_int( request, "type_borne" );
    gchar *borne        = Normaliser_chaine ( Json_get_string( request, "borne" ) );
    gchar *ed           = Normaliser_chaine ( Json_get_string( request, "ed" ) );
    gchar *unite        = Normaliser_chaine ( Json_get_string( request, "unite" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                       "UPDATE modbus_AO SET archivage=%d, min=%d, max=%d, type_borne=%d, borne='%s', ed='%s', unite='%s', libelle='%s' "
                       "WHERE modbus_ao_id=%d", archivage, min, max, type_borne, borne, ed, unite, libelle, modbus_ao_id );

    g_free(libelle);
    g_free(unite);
    g_free(borne);
    g_free(ed);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "MODBUS", "Modbus AO configured: min=%d, max=%d, archivage=%d", min, max, archivage );
    JsonNode *RootNode = Json_create();
    DB_Read ( domain, RootNode, NULL, "SELECT agent_tech_id FROM modbus_AO WHERE modbus_ao_id='%d'", modbus_ao_id );
    MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/RESTART", Json_get_string( RootNode, "agent_tech_id" ) );
    Json_unref(RootNode);
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
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "flip" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "borne" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ed" ))           return;

    gint   modbus_di_id = Json_get_int( request, "modbus_di_id" );
    gboolean flip       = Json_get_bool( request, "flip" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gchar *borne        = Normaliser_chaine ( Json_get_string( request, "borne" ) );
    gchar *ed           = Normaliser_chaine ( Json_get_string( request, "ed" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain, "UPDATE modbus_DI SET archivage=%d, borne='%s', ed='%s', libelle='%s', flip='%d' "
                                "WHERE modbus_di_id=%d", archivage, borne, ed, libelle, flip, modbus_di_id );

    g_free(libelle);
    g_free(borne);
    g_free(ed);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "MODBUS", "Modbus DI configured: archivage=%d, flip=%d", archivage, flip );
    JsonNode *RootNode = Json_create();
    DB_Read ( domain, RootNode, NULL, "SELECT agent_tech_id FROM modbus_DI WHERE modbus_di_id='%d'", modbus_di_id );
    MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/RESTART", Json_get_string( RootNode, "agent_tech_id" ) );
    Json_unref(RootNode);
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
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "borne" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "ed" ))           return;

    gint   modbus_do_id = Json_get_int( request, "modbus_do_id" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gchar *borne        = Normaliser_chaine ( Json_get_string( request, "borne" ) );
    gchar *ed           = Normaliser_chaine ( Json_get_string( request, "ed" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain, "UPDATE modbus_DO SET archivage=%d, borne='%s', ed='%s', libelle='%s' "
                                "WHERE modbus_do_id=%d", archivage, borne, ed, libelle, modbus_do_id );

    g_free(libelle);
    g_free(borne);
    g_free(ed);
    Modbus_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "MODBUS", "Modbus DO configured: archivage=%d", archivage );
    JsonNode *RootNode = Json_create();
    DB_Read ( domain, RootNode, NULL, "SELECT agent_tech_id FROM modbus_DO WHERE modbus_do_id='%d'", modbus_do_id );
    MQTT_Send_to_domain ( domain, RootNode, "AGENT/%s/RESTART", Json_get_string( RootNode, "agent_tech_id" ) );
    Json_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Modbus_DO set", NULL );
  }
/******************************************************************************************************************************/
/* RUN_MODBUS_ADD_IO_request_post: Ajoute des I/O pour un wago détecté                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_MODBUS_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;

    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_entree_ana" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_entree_tor" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_sortie_ana" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_sortie_tor" )) return;

    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( request, "agent_tech_id" ) );

    gint nbr_entree_ana = Json_get_int ( request, "nbr_entree_ana" );
    gint nbr_entree_tor = Json_get_int ( request, "nbr_entree_tor" );
    gint nbr_sortie_ana = Json_get_int ( request, "nbr_sortie_ana" );
    gint nbr_sortie_tor = Json_get_int ( request, "nbr_sortie_tor" );
    Info ( __func__, "modbus", domain->uuid, LOG_INFO, "Get %03d DI, %03d DO, %03d AI, %03d AO",
               nbr_entree_tor, nbr_sortie_tor, nbr_entree_ana, nbr_sortie_ana );
    gboolean retour = TRUE;
    for (gint cpt=0; cpt<nbr_entree_ana; cpt++)
    { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_AI SET agent_tech_id='%s', agent_acronyme='AI%03d', num=%d",
               agent_tech_id, cpt, cpt );
      retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='AI%03d'",
               agent_tech_id, cpt );
     }
    for (gint cpt=0; cpt<nbr_sortie_ana; cpt++)
    { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_AO SET agent_tech_id='%s', agent_acronyme='AO%03d', num=%d",
               agent_tech_id, cpt, cpt );
      retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='AO%03d'",
               agent_tech_id, cpt );
     }
    for (gint cpt=0; cpt<nbr_entree_tor; cpt++)
    { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_DI SET agent_tech_id='%s', agent_acronyme='DI%03d', num=%d",
               agent_tech_id, cpt, cpt );
      retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='DI%03d'",
               agent_tech_id, cpt );
     }
    for (gint cpt=0; cpt<nbr_sortie_tor; cpt++)
    { retour &= DB_Write ( domain, "INSERT IGNORE INTO modbus_DO SET agent_tech_id='%s', agent_acronyme='DO%03d', num=%d",
               agent_tech_id, cpt, cpt );
      retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='DO%03d'",
               agent_tech_id, cpt );
     }
    g_free(agent_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
