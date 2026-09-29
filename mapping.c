/******************************************************************************************************************************/
/* mapping.c                      Gestion des mappings dans l'API HTTP WebService                                             */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                16.06.2022 08:44:13 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mapping.c
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
/* Mapping_Apply_mapping: Pousse la config IO des agents (description, unite, archivage) dans les tables mnemos               */
/* Entrées: le domain d'application                                                                                           */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Mapping_Apply_mapping ( struct DOMAIN *domain )
  { Modbus_Apply_mapping ( domain );
    Phidget_Apply_mapping ( domain );
    Gpiod_Apply_mapping ( domain );
  }
/******************************************************************************************************************************/
/* Mapping_clear_old_map: Efface l'agent_description du bit DLS actuellement mappé sur un IO agent                            */
/* Entrées: le domain, l'agent_tech_id, l'agent_acronyme                                                                      */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Mapping_Clear_old_map ( struct DOMAIN *domain, gchar *agent_tech_id, gchar *agent_acronyme, gchar *tech_id, gchar *acronyme )
  { if (!domain || !agent_tech_id || !agent_acronyme || !tech_id || !acronyme) return;
    gchar *agent_tech_id_safe  = Normaliser_chaine ( agent_tech_id );
    gchar *agent_acronyme_safe = Normaliser_chaine ( agent_acronyme );
    gchar *tech_id_safe        = Normaliser_chaine ( tech_id );
    gchar *acronyme_safe       = Normaliser_chaine ( acronyme );
    if (agent_tech_id_safe && agent_acronyme_safe && tech_id_safe && acronyme_safe)
     { const gchar *tables[] = { "mnemos_DI", "mnemos_DO", "mnemos_AI", "mnemos_AO" };
       for (guint i=0; i<G_N_ELEMENTS(tables); i++)     /* Supprime les données IO de l'ancien mapping dans les tables mnemos */
        { DB_Write ( domain, "UPDATE %s AS mnemos_dest "
                             "INNER JOIN mappings AS map ON mnemos_dest.tech_id=map.tech_id AND mnemos_dest.acronyme=map.acronyme "
                             "SET mnemos_dest.agent_description='not mapped', mnemos_dest.archivage = 0 "
                             "WHERE map.agent_tech_id='%s' AND map.agent_acronyme='%s'",
                             tables[i], agent_tech_id_safe, agent_acronyme_safe );
        }
       DB_Write ( domain, "DELETE FROM mappings "                                       /* Supprime le mapping de l'ancien IO */
                          "WHERE tech_id = '%s' AND acronyme = '%s'", tech_id_safe, acronyme_safe );

     }
    if (agent_tech_id_safe)  g_free(agent_tech_id_safe);
    if (agent_acronyme_safe) g_free(agent_acronyme_safe);
    if (tech_id_safe)        g_free(tech_id_safe);
    if (acronyme_safe)       g_free(acronyme_safe);
  }
/******************************************************************************************************************************/
/* MAPPING_SET_request_post: Ajoute un mapping                                                                                */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void MAPPING_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" ))        return;

    gchar *agent_tech_id  = Json_get_string( request, "agent_tech_id" );
    gchar *agent_acronyme = Json_get_string( request, "agent_acronyme" );
    gchar *tech_id        = Json_get_string( request, "tech_id" );
    gchar *acronyme       = Json_get_string( request, "acronyme" );

    Mapping_Clear_old_map ( domain, agent_tech_id, agent_acronyme, tech_id, acronyme );

    gchar *agent_tech_id_safe  = Normaliser_chaine ( agent_tech_id );
    gchar *agent_acronyme_safe = Normaliser_chaine ( agent_acronyme );
    gchar *tech_id_safe        = Normaliser_chaine ( tech_id );
    gchar *acronyme_safe       = Normaliser_chaine ( acronyme );

    gboolean retour = FALSE;
    if (agent_tech_id_safe && agent_acronyme_safe && tech_id_safe && acronyme_safe)
     { retour &= DB_Write ( domain,
                            "INSERT INTO mappings SET "
                            "agent_tech_id = UPPER('%s'), agent_acronyme = UPPER('%s'), tech_id = UPPER('%s'), acronyme = '%s' "
                            "ON DUPLICATE KEY UPDATE tech_id=VALUES(tech_id), acronyme=VALUES(acronyme) ",
                            agent_tech_id_safe, agent_acronyme_safe, tech_id_safe, acronyme_safe );

       if (!strcasecmp ( agent_tech_id, "_COMMAND_TEXT" ) )              /* Ajoute un libellé pour les Mappings _COMMAND_TEXT */
        { retour &= DB_Write ( domain,
                               "INSERT INTO mnemos_DI SET "
                               "tech_id = UPPER('%s'), acronyme = UPPER('%s'), "
                               "libelle=CONCAT('TRUE when ', UPPER('%s'), ' is received') "
                               "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle) ",
                               tech_id_safe, acronyme_safe, agent_acronyme_safe );
        }
     }

    if (acronyme_safe)       g_free(acronyme_safe);
    if (tech_id_safe)        g_free(tech_id_safe);
    if (agent_acronyme_safe) g_free(agent_acronyme_safe);
    if (agent_tech_id_safe)  g_free(agent_tech_id_safe);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    MQTT_Send_to_domain ( domain, NULL, "DLS/REMAP" );
    Mapping_Apply_mapping ( domain );

    Audit_log ( domain, token, "MAPPING", "Mapping '%s:%s' <-> '%s:%s' set", agent_tech_id, agent_acronyme, tech_id, acronyme );
    Info ( __func__, "mapping", domain->uuid, LOG_NOTICE, "Mapping '%s:%s' <-> '%s:%s' set",
               agent_tech_id, agent_acronyme, tech_id, acronyme );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping done", NULL );
  }
/******************************************************************************************************************************/
/* MAPPING_DELETE_request: Retire un mapping                                                                                  */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void MAPPING_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "mapping_id" ))  return;

    gint mapping_id = Json_get_int ( request, "mapping_id" );
    gboolean retour = DB_Write ( domain, "DELETE FROM mappings WHERE mapping_id=%d", mapping_id );
    MQTT_Send_to_domain ( domain, NULL, "DLS/REMAP" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Info ( __func__, "mapping", domain->uuid, LOG_NOTICE, "Mapping mapping_id=%d deleted", mapping_id );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping deleted", NULL );
  }
/******************************************************************************************************************************/
/* MAPPING_LIST_request_post: Repond aux requests des users pour les mappings                                                 */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void MAPPING_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { gchar chaine[256];

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    g_snprintf( chaine, sizeof(chaine), "SELECT * FROM mappings" );

    if ( Json_has_member ( url_param, "agent_tech_id" ) )
     { gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( url_param, "agent_tech_id" ) );
       if (!agent_tech_id)
        { Info ( __func__, "mapping", domain->uuid, LOG_ERR, "Normalize error for agent_tech_id" );
          Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Normalize error", RootNode );
          return;
        }
       g_strlcat ( chaine, " WHERE agent_tech_id='", sizeof(chaine) );
       g_strlcat ( chaine, agent_tech_id, sizeof(chaine) );
       g_strlcat ( chaine, "'", sizeof(chaine) );
       g_free(agent_tech_id);
     }

    gboolean retour = DB_Read ( domain, RootNode, "mappings", "%s", chaine );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping sent", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_MAPPING_LIST_request_post: Repond aux requests AGENT depuis pour les mappings                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_MAPPING_LIST_request_get ( struct DOMAIN *domain, gchar *path, struct ABLS_HEADERS *abls_headers, SoupServerMessage *msg, JsonNode *url_param )
  { JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "mappings", "SELECT * FROM mappings WHERE tech_id IS NOT NULL AND acronyme IS NOT NULL" );
    Json_add_bool ( RootNode, "api_cache", TRUE );                                     /* Active la cache sur les agents */
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping sent", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_MAPPING_LIST_request_post: Repond aux requests AGENT depuis pour les mappings                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_MAPPING_SEARCH_TXT_request_post ( struct DOMAIN *domain, gchar *path, struct ABLS_HEADERS *abls_headers, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_acronyme" ))  return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *agent_acronyme = Normaliser_chaine ( Json_get_string ( request, "agent_acronyme" ) );/* Formatage correct des chaines */
    if (!agent_acronyme) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "results",
                                "SELECT * FROM mappings WHERE agent_tech_id='_COMMAND_TEXT' AND agent_acronyme=TRIM('%s')",
                                agent_acronyme );

    if (retour ==FALSE || Json_get_int ( RootNode, "nbr_results" ) == 0)
     { retour &= DB_Read ( domain, RootNode, "results",
                           "SELECT * FROM mappings WHERE agent_tech_id='_COMMAND_TEXT' AND agent_acronyme LIKE '%%%s%%'",
                           agent_acronyme );
     }

    g_free(agent_acronyme);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Mapping sent", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
