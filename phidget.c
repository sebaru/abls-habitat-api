/******************************************************************************************************************************/
/* phidget.c                      Gestion des phidget dans l'API HTTP WebService                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                23.04.2023 08:08:46 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * phidget.c
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
/* PHIDGET_SET_request_post: Appelé depuis libsoup pour éditer ou creer un phidget                                              */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_uuid" ))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "password" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "serial" ))         return;

    Json_node_add_string ( request, "thread_classe", "phidget" );
    gchar *agent_uuid     = Normaliser_chaine ( Json_get_string( request, "agent_uuid" ) );
    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string( request, "thread_tech_id" ) );
    gchar *hostname       = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description    = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *password       = Normaliser_chaine ( Json_get_string( request, "password" ) );
    gchar *serial         = Normaliser_chaine ( Json_get_string( request, "serial" ) );

    retour = DB_Write ( domain,
                       "INSERT INTO phidget SET "
                       "agent_uuid='%s', thread_tech_id='%s', hostname='%s', description='%s', password='%s', serial='%s' "
                       "ON DUPLICATE KEY UPDATE agent_uuid=VALUE(agent_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "password=VALUE(password), serial=VALUE(serial) ",
                       agent_uuid, thread_tech_id, hostname, description, password, serial );

    g_free(agent_uuid);
    g_free(thread_tech_id);
    g_free(hostname);
    g_free(description);
    g_free(password);
    g_free(serial);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    AGENT_send_to_agent ( domain, NULL, "THREAD_RESTART", request );                               /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* PHIDGET_LIST_request_get: Appelé depuis libsoup pour l'URI phidget/list                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (Http_fail_if_has_not ( domain, path, msg, url_param, "classe")) return;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = FALSE;
    gchar *classe = Json_get_string ( url_param, "classe" );
         if (!strcasecmp ( classe, "phidget" ))
          { retour = DB_Read ( domain, RootNode, "phidget", "SELECT phidget.*, agent_hostname FROM phidget INNER JOIN agents USING(agent_uuid)" ); }
    else if (!strcasecmp ( classe, "AI" ))
          { retour = DB_Read ( domain, RootNode, "AI",
                               "SELECT * FROM phidget_AI AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "AO" ))
          { retour = DB_Read ( domain, RootNode, "AO",
                               "SELECT * FROM phidget_AO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "DI" ))
          { retour = DB_Read ( domain, RootNode, "DI",
                               "SELECT * FROM phidget_DI AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }
    else if (!strcasecmp ( classe, "DO" ))
          { retour = DB_Read ( domain, RootNode, "DO",
                               "SELECT * FROM phidget_DO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme");
          }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* PHIDGET_SET_AI_request_post: Change les données d'une analogInput                                                           */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_SET_AI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "phidget_ai_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "min" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "max" ))          return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "unite" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "type_borne" ))   return;

    gint   phidget_ai_id = Json_get_int( request, "phidget_ai_id" );
    gint   archivage    = Json_get_int( request, "archivage" );
    gint   min          = Json_get_int( request, "min" );
    gint   max          = Json_get_int( request, "max" );
    gint   type_borne   = Json_get_int( request, "type_borne" );
    gchar *unite        = Normaliser_chaine ( Json_get_string( request, "unite" ) );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                       "UPDATE phidget_AI SET archivage=%d, min=%d, max=%d, type_borne=%d, unite='%s', libelle='%s' "
                       "WHERE phidget_ai_id=%d", archivage, min, max, type_borne, unite, libelle, phidget_ai_id );

    g_free(libelle);
    g_free(unite);
    Copy_thread_io_to_mnemos_for_classe ( domain, "phidget" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_tech_id, agent_uuid FROM phidget_AI "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE phidget_ai_id='%d'", phidget_ai_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", request );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread resetted", NULL );
  }
/******************************************************************************************************************************/
/* PHIDGET_SET_DI_request_post: Change les données d'une DigitalInput                                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_SET_DI_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "phidget_di_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "classe" ))        return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "capteur" ))       return;

#ifdef bouh
    gchar *phidget_classe = NULL;
    if (!strcasecmp ( capteur, "DIGITAL-INPUT" )) phidget_classe="DigitalInput";

    if (!phidget_classe) { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Capteur is unknown", NULL ); return; }

    gint   phidget_di_id = Json_get_int( request, "phidget_di_id" );
    gchar *libelle       = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                       "UPDATE phidget_DI SET hub_id=%d, port=%d, classe='%s', capteur='%s', libelle='%s' "
                       "WHERE phidget_di_id=%d", hub_id, port, phidget_classe, capteur, libelle, phidget_di_id );


    retour = DB_Write ( domain, "INSERT INTO phidget_DI SET hub_id=%d, port=%d, classe='%s', capteur='%s', libelle='%s' "
                                "ON DUPLICATE KEY UPDATE "
                                "classe=VALUES(classe), capteur=VALUES(capteur), port=VALUES(port), hub_id=VALUES(hub_id)",
                                hub_id, port, phidget_classe, capteur, libelle );

       SQL_Write_new ( "UPDATE mnemos_DI SET map_thread='PHIDGET', "
                       "map_thread_tech_id=CONCAT ( (SELECT thread_tech_id FROM phidget WHERE id=%d), '_P%d') "
                       "WHERE thread_tech_id='%s' AND acronyme='%s'",
                       hub_id, port, thread_tech_id, acronyme
                     );

       if (SQL_Write_new ( "INSERT INTO phidget_DI SET hub_id=%d, port=%d, classe='%s', capteur='%s' "
                           "ON DUPLICATE KEY UPDATE "
                           "classe=VALUES(classe),capteur=VALUES(capteur), port=VALUES(port), hub_id=VALUES(hub_id)",
                           hub_id, port, phidget_classe, capteur

    g_free(libelle);
    Copy_thread_io_to_mnemos_for_classe ( domain, "phidget" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_tech_id, agent_uuid FROM phidget_DI "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE phidget_di_id='%d'", phidget_di_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", request );/* Stop sent to all agents */
    json_node_unref(RootNode);
#endif
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread resetted", NULL );
  }
/******************************************************************************************************************************/
/* PHIDGET_SET_DO_request_post: Change les données d'une DigitalInput                                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_SET_DO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "phidget_do_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;

    gint   phidget_do_id = Json_get_int( request, "phidget_do_id" );
    gchar *libelle      = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain, "UPDATE phidget_DO SET libelle='%s' WHERE phidget_do_id=%d", libelle, phidget_do_id );

    g_free(libelle);
    Copy_thread_io_to_mnemos_for_classe ( domain, "phidget" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_tech_id, agent_uuid FROM phidget_DO "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE phidget_do_id='%d'", phidget_do_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", request );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread resetted", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
