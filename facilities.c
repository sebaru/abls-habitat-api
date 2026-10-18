/******************************************************************************************************************************/
/* facilities.c                 Gestion des facilities de log des agents dans l'API HTTP WebService                           */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                11.10.2026 10:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * facilities.c
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

/******************************************************************************************************************************/
/* FACILITY_add_agent_facilities: Ajoute au RootNode le tableau des noms de facilities forcees pour un agent                   */
/* Entrees: le domaine, le noeud cible et l'agent_tech_id (deja controle)                                                     */
/* Sortie : FALSE si erreur base de donnees                                                                                   */
/******************************************************************************************************************************/
 gboolean FACILITY_add_agent_facilities ( struct DOMAIN *domain, JsonNode *RootNode, gchar *agent_tech_id )
  { return ( DB_Read ( domain, RootNode, "log_facilities",
                       "SELECT lf.log_facility FROM agent_log_facilities AS alf "
                       "INNER JOIN log_facilities AS lf USING (log_facility_id) "
                       "WHERE alf.agent_tech_id='%s' ORDER BY lf.log_facility",
                       agent_tech_id ) );
  }
/******************************************************************************************************************************/
/* LOG_FACILITY_LIST_request_get: Retourne le catalogue des facilities de log trie par nom                                    */
/* Entrees: le domaine, le token et les elements de la requete HTTP                                                           */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void LOG_FACILITY_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    if (!DB_Read ( domain, RootNode, "log_facilities",
                   "SELECT log_facility_id, log_facility FROM log_facilities ORDER BY log_facility" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database error", RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of log facilities", RootNode );
  }
/******************************************************************************************************************************/
/* LOG_FACILITY_GET_request_get: Retourne les facilities de log forcees pour un agent                                         */
/* Entrees: le domaine, le token et les elements de la requete HTTP                                                           */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void LOG_FACILITY_GET_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id" )) return;

    gchar *agent_tech_id = Json_get_string ( url_param, "agent_tech_id" );
    if (!AGENT_get_classe ( domain, agent_tech_id ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", NULL ); return; }

    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    if (!agent_tech_id_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) { g_free(agent_tech_id_safe); return; }

    gboolean retour = DB_Read ( domain, RootNode, "log_facilities",
                                "SELECT lf.log_facility_id, lf.log_facility FROM agent_log_facilities AS alf "
                                "INNER JOIN log_facilities AS lf USING (log_facility_id) "
                                "WHERE alf.agent_tech_id='%s' ORDER BY lf.log_facility",
                                agent_tech_id_safe );
    g_free(agent_tech_id_safe);
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database error", RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List of agent log facilities", RootNode );
  }
/******************************************************************************************************************************/
/* LOG_FACILITY_SET_request_post: Active/desactive le forcage d'une facility de log pour un agent                             */
/* Entrees: le domaine, le token et les elements de la requete HTTP                                                           */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void LOG_FACILITY_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "log_facility_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable" ))          return;

    gchar   *agent_tech_id   = Json_get_string ( request, "agent_tech_id" );
    gint     log_facility_id = Json_get_int    ( request, "log_facility_id" );
    gboolean enable          = Json_get_bool   ( request, "enable" );

    if (!AGENT_get_classe ( domain, agent_tech_id ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Agent not found", NULL ); return; }

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    if (!DB_Read ( domain, RootNode, NULL,
                   "SELECT log_facility_id, log_facility FROM log_facilities WHERE log_facility_id=%d LIMIT 1",
                   log_facility_id ))
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database error", RootNode ); return; }
    if (!Json_has_member ( RootNode, "log_facility" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Unknown log_facility_id", RootNode ); return; }

    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    if (!agent_tech_id_safe)
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Normalize error for agent_tech_id", RootNode ); return; }

    gboolean retour;
    if (enable)
     { retour = DB_Write ( domain, "INSERT IGNORE INTO agent_log_facilities (agent_tech_id, log_facility_id) VALUES ('%s', %d)",
                           agent_tech_id_safe, log_facility_id );
     }
    else
     { retour = DB_Write ( domain, "DELETE FROM agent_log_facilities WHERE agent_tech_id='%s' AND log_facility_id=%d",
                           agent_tech_id_safe, log_facility_id );
     }
    g_free(agent_tech_id_safe);
    if (!retour)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, domain->mysql_last_error, RootNode ); return; }

    gchar *log_facility = Json_get_string ( RootNode, "log_facility" );
    Json_add_bool ( RootNode, "enable", enable );

    JsonNode *MqttNode = Json_create ();
    if (MqttNode)
     { Json_add_string ( MqttNode, (enable ? "debug" : "undebug"), log_facility );
       MQTT_Send_to_domain ( domain, MqttNode, "AGENT/%s/LOG", agent_tech_id );
       Json_unref ( MqttNode );
     }

    Audit_log ( domain, token, "AGENT", "Agent '%s' log facility '%s' %s",
                agent_tech_id, log_facility, (enable ? "enabled" : "disabled") );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Agent log facility updated", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
