/******************************************************************************************************************************/
/* gpiod.c                      Gestion des gpiod dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                13.05.2025 20:28:04 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gpiod.c
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
/* Gpiod_load: Charge la configuration d'un agent GPIOD                                                                       */
/* Entrées: le domaine, les headers d'agent et le node de réponse                                                             */
/* Sortie : FALSE si l'agent n'a pas été trouvé                                                                               */
/******************************************************************************************************************************/
 gboolean Gpiod_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM gpiod WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    DB_Read ( domain, DstNode, "IO", "SELECT * FROM gpiod_IO WHERE agent_tech_id='%s'", abls_headers->agent_tech_id );
    return(TRUE);
  }

/******************************************************************************************************************************/
/* GPIOD_SET_request_post: Appelé depuis libsoup pour éditer ou creer un gpiod                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void GPIOD_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))     return;

    g_strcanon ( Json_get_string( request, "agent_tech_id" ), "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );

    gchar *server_uuid     = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id  = Normaliser_chaine ( Json_get_string( request, "agent_tech_id" ) );
    gchar *description     = Normaliser_chaine ( Json_get_string( request, "description" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO gpiod SET server_uuid='%s', agent_tech_id=UPPER('%s'), description='%s' "
                        "ON DUPLICATE KEY UPDATE server_uuid=VALUES(server_uuid), description=VALUES(description)",
                        server_uuid, agent_tech_id, description );

    g_free(server_uuid);
    g_free(agent_tech_id);
    g_free(description);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "GPIO", "GPIO thread configured: agent=%s, description=%s", 
          Json_get_string( request, "agent_tech_id" ), description );
    Json_add_string ( request, "agent_classe", "gpiod" );
    MQTT_Send_to_domain ( domain, request, "THREAD/RESTART" );                          /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* GPIOD_LIST_request_get: Appelé depuis libsoup pour l'URI gpiod/list                                                        */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void GPIOD_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (Http_fail_if_has_not ( domain, path, msg, url_param, "classe"  )) return;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = FALSE;
    gchar *classe = Json_get_string ( url_param, "classe" );
    if (!strcasecmp ( classe, "IO" ))
     { retour = DB_Read ( domain, RootNode, "IO",
                          "SELECT m.*, map.tech_id, map.acronyme, map.mapping_id FROM gpiod_IO AS m "
                          "LEFT JOIN mappings AS map ON m.agent_tech_id = map.agent_tech_id AND m.agent_acronyme = map.agent_acronyme "
                        );
     }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* GPIOD_SET_IO_request_post: Change les données d'une I/O GPIOD                                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void GPIOD_SET_IO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "gpiod_io_id"    )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle"        )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "mode_inout"     )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "mode_activelow" )) return;

    gchar *libelle        = Normaliser_chaine ( Json_get_string( request, "libelle" ) );
    gint   gpiod_io_id    = Json_get_int( request, "gpiod_io_id" );
    gint   mode_inout     = Json_get_int( request, "mode_inout" );
    gint   mode_activelow = Json_get_int( request, "mode_activelow" );

    retour = DB_Write ( domain,
                        "UPDATE gpiod_IO SET mode_inout='%d', mode_activelow='%d', libelle='%s' "
                        "WHERE gpiod_io_id=%d", mode_inout, mode_activelow, libelle, gpiod_io_id );

    g_free(libelle);
    /*Gpoid_Copy_thread_io_to_mnemos ( domain );*/

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "GPIO", "GPIO IO configured: mode_inout=%d, mode_activelow=%d", mode_inout, mode_activelow );
    JsonNode *RootNode = Json_create();
    DB_Read ( domain, RootNode, NULL, "SELECT 'gpiod' AS agent_classe, gpiod.agent_tech_id, gpiod.server_uuid "
              "FROM gpiod_IO INNER JOIN gpiod USING (agent_tech_id) WHERE gpiod_io_id='%d'", gpiod_io_id );
    MQTT_Send_to_domain ( domain, RootNode, "%s/THREAD_RESTART", Json_get_string( RootNode, "server_uuid" ) );/* Stop sent to all agents */
    Json_unref(RootNode);

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Gpiod_IO set", NULL );
  }
/******************************************************************************************************************************/
/* RUN_GPIOD_ADD_IO_request_post: Ajoute des I/O pour un thread GPIOD                                                         */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_GPIOD_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_lignes" )) return;

    gchar *agent_tech_id = Normaliser_chaine ( Json_get_string ( request, "agent_tech_id" ) );
    gint nbr_lignes = Json_get_int ( request, "nbr_lignes" );

    Info ( __func__, "gpio", domain->uuid, LOG_INFO, "%s: Add %d IO", agent_tech_id, nbr_lignes );
    gboolean retour = TRUE;
    for (gint cpt=0; cpt<nbr_lignes; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO gpiod_IO SET "
                                    "agent_tech_id='%s', "
                                    "agent_acronyme='IO%02d', "
                                    "num='%d', mode_inout='0', mode_activelow='0', "
                                    "libelle='Entrée/Sortie GPIOD N°%d' ",
                                    agent_tech_id, cpt, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET agent_tech_id='%s', agent_acronyme='%02d'",
                                    agent_tech_id, cpt );
     }
    g_free(agent_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
