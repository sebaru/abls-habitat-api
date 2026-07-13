/******************************************************************************************************************************/
/* phidget.c                      Gestion des phidget dans l'API HTTP WebService                                              */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                23.04.2023 08:08:46 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * phidget.c
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
 #include "phidget.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* Phidget_load: Charge la configuration d'un phidget                                                                         */
/* Entrées: le domaine, l'agent_tech_id et le node de reponse                                                                 */
/* Sortie : FALSE si l'agent_tech_id n'a pas été trouvé                                                                       */
/******************************************************************************************************************************/
 gboolean Phidget_load ( struct DOMAIN *domain, struct ABLS_HEADERS *abls_headers, JsonNode *DstNode )
  { DB_Read ( domain, DstNode, NULL, "SELECT * FROM phidget WHERE server_uuid='%s' AND agent_tech_id='%s'",
              abls_headers->server_uuid, abls_headers->agent_tech_id );
    if (!Json_has_member ( DstNode, "agent_tech_id" )) return(FALSE);
    DB_Read ( domain, DstNode, "IO", "SELECT * FROM phidget_IO WHERE agent_tech_id='%s'", abls_headers->agent_tech_id );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Phidget_Copy_thread_io_to_mnemos: Recopie la config IO phidget et met a jour les tables mnemos_xx                          */
/* Entrées: le domaine                                                                                                        */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Phidget_Copy_thread_io_to_mnemos ( struct DOMAIN *domain )
  { gchar requete[512];

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_AI AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN phidget_IO AS src ON src.agent_tech_id=map.thread_tech_id AND src.agent_acronyme=map.thread_acronyme "
                 "SET dest.archivage = src.archivage, dest.unite = src.unite, dest.libelle = src.libelle "
                 "WHERE src.classe='AI'" );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_AO AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN phidget_IO AS src ON src.agent_tech_id=map.thread_tech_id AND src.agent_acronyme=map.thread_acronyme "
                 "SET dest.archivage = src.archivage, dest.unite = src.unite, dest.libelle = src.libelle "
                 "WHERE src.classe='AO'" );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_DI AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN phidget_IO AS src ON src.agent_tech_id=map.thread_tech_id AND src.agent_acronyme=map.thread_acronyme "
                 "SET dest.libelle = src.libelle "
                 "WHERE src.classe='DI'" );
    DB_Write ( domain, requete );

    g_snprintf ( requete, sizeof(requete),
                 "UPDATE mnemos_DO AS dest "
                 "INNER JOIN mappings AS map ON dest.tech_id = map.tech_id AND dest.acronyme=map.acronyme "
                 "INNER JOIN phidget_IO AS src ON src.agent_tech_id=map.thread_tech_id AND src.agent_acronyme=map.thread_acronyme "
                 "SET dest.libelle = src.libelle "
                 "WHERE src.classe='DO'" );
    DB_Write ( domain, requete );
  }
/******************************************************************************************************************************/
/* Capteur_to_classe: Retourne la classe associée à un capteur donné                                                          */
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
    if (!strcasecmp ( capteur, "1130-PH"              )) { return("AI"); }
    if (!strcasecmp ( capteur, "1130-ORP"             )) { return("AI"); }
    if (!strcasecmp ( capteur, "DIGITAL-INPUT"        )) { return("DI"); }
    if (!strcasecmp ( capteur, "REL2001_0"            )) { return("DO"); }
    return(NULL);
  }
/******************************************************************************************************************************/
/* PHIDGET_SET_request_post: Appelé depuis libsoup pour éditer ou creer un phidget                                            */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { gboolean retour;

    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "server_uuid" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "agent_tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "hostname" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "description" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "password" ))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "serial" ))         return;

    gchar *agent_tech_id      = Json_get_string( request, "agent_tech_id" );
    g_strcanon ( agent_tech_id, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_", '_' );
    gchar *server_uuid_safe   = Normaliser_chaine ( Json_get_string( request, "server_uuid" ) );
    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );
    gchar *hostname_safe      = Normaliser_chaine ( Json_get_string( request, "hostname" ) );
    gchar *description_safe   = Normaliser_chaine ( Json_get_string( request, "description" ) );
    gchar *password_safe      = Normaliser_chaine ( Json_get_string( request, "password" ) );
    gint  serial              = Json_get_int( request, "serial" );

    retour = DB_Write ( domain,
                       "INSERT INTO phidget SET "
                       "server_uuid='%s', agent_tech_id='%s', hostname='%s', description='%s', password='%s', serial='%d' "
                       "ON DUPLICATE KEY UPDATE server_uuid=VALUE(server_uuid), hostname=VALUE(hostname), description=VALUE(description),"
                       "password=VALUE(password), serial=VALUE(serial) ",
                       server_uuid_safe, agent_tech_id_safe, hostname_safe, description_safe, password_safe, serial );

    g_free(server_uuid_safe);
    g_free(agent_tech_id_safe);
    g_free(hostname_safe);
    g_free(description_safe);
    g_free(password_safe);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "PHIDGET", "Phidget thread configured: agent=%s, description=%s, hostname=%s, serial=%d",
                Json_get_string( request, "agent_tech_id" ), Json_get_string( request, "description" ), Json_get_string( request, "hostname" ), serial );
    Json_add_string ( request, "thread_classe", "phidget" );
    MQTT_Send_to_domain ( domain, request, "RESTART/AGENT/%s", agent_tech_id );                    /* Stop sent to all agents */
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Thread changed", NULL );
  }
/******************************************************************************************************************************/
/* PHIDGET_LIST_request_get: Appelé depuis libsoup pour l'URI phidget/list                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void PHIDGET_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "agent_tech_id")) return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *agent_tech_id      = Json_get_string ( url_param, "agent_tech_id" );
    gchar *agent_tech_id_safe = Normaliser_chaine ( agent_tech_id );

    gboolean retour = DB_Read ( domain, RootNode, "IO",
                                "SELECT m.*, map.tech_id, map.acronyme, map.mapping_id FROM phidget_IO AS m "
                                "LEFT JOIN mappings AS map ON m.agent_tech_id = map.thread_tech_id AND m.agent_acronyme = map.thread_acronyme "
                                "WHERE m.agent_tech_id='%s'",
                                agent_tech_id_safe
                              );

    g_free ( agent_tech_id_safe );

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
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))       return;

    gchar *capteur = Json_get_string( request, "capteur" );
    gchar *classe  = Capteur_to_classe ( capteur );

    if (!classe) { Http_Send_json_response ( msg, FALSE, "Capteur non pris en charge", NULL ); return; }

    gint   phidget_io_id = Json_get_int( request, "phidget_io_id" );
    gint   intervalle    = Json_get_int( request, "intervalle" );
    gchar *libelle_safe  = Normaliser_chaine ( Json_get_string( request, "libelle" ) );

    retour = DB_Write ( domain,
              "UPDATE phidget_IO SET classe='%s', agent_acronyme=CONCAT(classe,LPAD(port,2,'0')), capteur='%s', libelle='%s', intervalle=%d "
              "WHERE phidget_io_id=%d", classe, capteur, libelle_safe, intervalle, phidget_io_id );

    if (Json_has_member ( request, "unite" ))
     { unite_safe = Normaliser_chaine ( Json_get_string( request, "unite" ) ); }
       retour &= DB_Write ( domain, "UPDATE phidget_IO SET unite='%s' WHERE phidget_io_id=%d", unite_safe, phidget_io_id );
       g_free(unite_safe);
     };

    if (Json_has_member ( request, "archivage" ))
     { retour &= DB_Write ( domain, "UPDATE phidget_IO SET archivage=%d WHERE phidget_io_id=%d",
                                    Json_get_int ( request, "archivage" ), phidget_io_id );
     };


    g_free(libelle_safe);
    Phidget_Copy_thread_io_to_mnemos ( domain );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Audit_log ( domain, token, "PHIDGET", "Phidget IO configured: capteur=%s, intervalle=%d", Json_get_string( request, "capteur" ), intervalle );
    JsonNode *RootNode = Json_create();
    DB_Read ( domain, RootNode, NULL, "SELECT t.thread_classe, t.thread_tech_id, t.agent_uuid FROM phidget_IO AS p "
                      "INNER JOIN threads AS t ON t.thread_tech_id = p.agent_tech_id WHERE p.phidget_io_id='%d'", phidget_io_id );
    MQTT_Send_to_domain ( domain, RootNode, "%s/THREAD_RESTART", Json_get_string( RootNode, "agent_uuid" ) );/* Stop sent to all agents */
    Json_unref(RootNode);
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Phidget_IO set", NULL );
  }
/******************************************************************************************************************************/
/* RUN_PHIDGET_ADD_IO_request_post: Ajoute des I/O pour un hub phidget                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_PHIDGET_ADD_IO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "thread_tech_id" )) return;

    gchar *thread_tech_id = Normaliser_chaine ( Json_get_string ( request, "thread_tech_id" ) );

    Info ( __func__, "phidget", domain->uuid, LOG_INFO, "%s: Add 6 IO", thread_tech_id );
    gboolean retour = TRUE;
    for (gint cpt=0; cpt<6; cpt++)
     { retour &= DB_Write ( domain, "INSERT IGNORE INTO phidget_IO SET "
                                    "agent_tech_id='%s', classe='DI', port='%d', "
                                    "thread_acronyme=CONCAT(classe,LPAD(port,2,'0')), "
                                    "capteur='DIGITAL-INPUT', "
                                    "libelle='Capteur type DIGITAL-INPUT sur port %d' ",
                                    thread_tech_id, cpt, cpt );
       retour &= DB_Write ( domain, "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DI%02d'",
                                    thread_tech_id, cpt );
     }
    g_free(thread_tech_id);
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
