/******************************************************************************************************************************/
/* gpiod.c                      Gestion des gpiod dans l'API HTTP WebService                                                  */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                13.05.2025 20:28:04 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * gpiod.c
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* GPIOD_SET_request_post: Appelé depuis libsoup pour éditer ou creer un gpiod                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void GPIOD_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))     return;

    gchar *agent_uuid      = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id  = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *description     = Normaliser_chaine ( Json_get_string( request, "description" ) );

    retour = DB_Write ( domain,
                        "INSERT INTO gpiod SET agent_uuid='%s', thread_tech_id=UPPER('%s'), description='%s' "
                        "ON DUPLICATE KEY UPDATE agent_uuid=VALUES(agent_uuid), description=VALUES(description)",
                        agent_uuid, thread_tech_id, description );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(description);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_string ( request, "thread_classe", "gpiod" );
    MQTT_Send_to_domain ( domain, "agents", "THREAD_RESTART", request );                           /* Stop sent to all agents */
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
                          "SELECT m.*, map.tech_id, map.acronyme FROM gpiod_IO AS m "
                          "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme "
                        );
     }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* RUN_GPIOD_ADD_IO_request_post: Ajoute des I/O pour un thread GPIOD                                                         */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_GPIOD_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "nbr_lignes" )) return;

    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );
    gint nbr_lignes = Json_get_int ( request, "nbr_lignes" );

    Info_new ( __func__, LOG_INFO, domain, "%s: Add %d IO", thread_tech_id, nbr_lignes );
    gboolean retour = TRUE;
    for (gint cpt=0; cpt<nbr_lignes; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO gpiod_IO SET "
                                    "thread_tech_id='%s', "
                                    "thread_acronyme='%02d', "
                                    "num='%d', mode_inout='0', mode_activelow='0', "
                                    "libelle='Entrée/Sortie GPIOD N°%d' ",
                                    thread_tech_id, cpt, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='%02d'",
                                    thread_tech_id, cpt );
     }
    g_free(thread_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
