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
/* Capteur_to_classse: Retourne la classe associée à un capteur donné                                                         */
/* Entrée: Le capteur                                                                                                         */
/* Sortie: la classe, ou NULL si erreur                                                                                       */
/******************************************************************************************************************************/
 static gchar *Capteur_to_classe ( gchar *capteur )
  { if (!capteur)   { return(NULL); }
    if (!strcasecmp ( capteur, "ADP1000-PH"           )) { return("AI"); }
    if (!strcasecmp ( capteur, "ADP1000-ORP"          )) { return("AI"); }
    if (!strcasecmp ( capteur, "TMP1200_0-PT100-3850" )) { return("AI"); }
    if (!strcasecmp ( capteur, "TMP1200_0-PT100-3920" )) { return("AI"); }
    if (!strcasecmp ( capteur, "AC-CURRENT-10A"       )) { return("AI"); }
    if (!strcasecmp ( capteur, "AC-CURRENT-25A"       )) { return("AI"); }
    if (!strcasecmp ( capteur, "AC-CURRENT-50A"       )) { return("AI"); }
    if (!strcasecmp ( capteur, "AC-CURRENT-100A"      )) { return("AI"); }
    if (!strcasecmp ( capteur, "TEMP_1124_0"          )) { return("AI"); }
    if (!strcasecmp ( capteur, "DIGITAL-INPUT"        )) { return("DI"); }
    if (!strcasecmp ( capteur, "REL2001_0"            )) { return("DO"); }
    return(NULL);
  }
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
    else if (!strcasecmp ( classe, "IO" ))
          { retour = DB_Read ( domain, RootNode, "IO",
                               "SELECT m.*, map.tech_id, map.acronyme FROM phidget_IO AS m "
                               "LEFT JOIN mappings AS map ON m.thread_tech_id = map.thread_tech_id AND m.thread_acronyme = map.thread_acronyme "
                             );
          }

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* PHIDGET_SET_IO_request_post: Change les données d'une I/O Phidget                                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_SET_IO_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "phidget_io_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "capteur" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "intervalle" ))    return;
    /*if (Http_fail_if_has_not ( domain, path, msg, request, "archivage" ))     return;*/
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))       return;

    gchar *capteur = Json_get_string( request, "capteur" );
    gchar *classe  = Capteur_to_classe ( capteur );

    if (!classe) { Http_Send_json_response ( msg, FALSE, "Capteur non pris en charge", NULL ); return; }

    gint   phidget_io_id = Json_get_int( request, "phidget_io_id" );
    gint   intervalle    = Json_get_int( request, "intervalle" );
    gchar *libelle       = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
                        "UPDATE phidget_IO SET classe='%s', thread_acronyme=CONCAT(classe,LPAD(port,2,'0')), capteur='%s', libelle='%s', intervalle=%d "
                        "WHERE phidget_io_id=%d", classe, capteur, libelle, intervalle, phidget_io_id );

    g_free(libelle);
    Copy_thread_io_to_mnemos_for_classe ( domain, "phidget" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT thread_tech_id, agent_uuid FROM phidget_IO "
                                      "INNER JOIN threads USING (thread_tech_id) WHERE phidget_io_id='%d'", phidget_io_id );
    AGENT_send_to_agent ( domain, Json_get_string( RootNode, "agent_uuid" ), "THREAD_RESTART", request );/* Stop sent to all agents */
    json_node_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread resetted", NULL );
  }
/******************************************************************************************************************************/
/* RUN_PHIDGET_ADD_IO_request_post: Ajoute des I/O pour un hub phidget                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_PHIDGET_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;

    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );

    Info_new ( __func__, LOG_INFO, domain, "Add 6 IO", thread_tech_id );
    gboolean retour = TRUE;
    for (gint cpt=0; cpt<6; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO phidget_IO SET "
                                    "thread_tech_id='%s', thread_acronyme=CONCAT(classe,LPAD(port,2,'0')), "
                                    "classe='DI', port='%d', "
                                    "capteur='DIGITAL-INPUT', "
                                    "description='Capteur type DIGITAL-INPUT sur port %d' ",
                                    thread_tech_id, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DI%02d'",
                                    thread_tech_id, cpt );
     }
    g_free(thread_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
