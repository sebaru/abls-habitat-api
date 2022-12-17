/******************************************************************************************************************************/
/* mnemo_HORLOGE.c        Déclaration des fonctions pour la gestion des Horloges                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                03.07.2018 21:25:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemo_HORLOGE.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* Mnemo_auto_create_HORLOGE_from_thread: Ajoute un mnemonique dans la base via le tech_id depuis une demande d'un thread     */
/* Entrée: le tech_id, l'acronyme, le libelle et l'unite et l'archivage                                                       */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_HORLOGE_from_thread ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *libelle = Normaliser_chaine ( libelle_src );                                      /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,                                                                     /* Requete SQL */
                                 "INSERT INTO mnemos_HORLOGE SET deletable=0, tech_id='%s', acronyme='%s', libelle='%s' "
                                 "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                                 tech_id, acro, libelle );
    g_free(acro);
    g_free(libelle);
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_auto_create_HORLOGE_from_dls: Ajoute un mnemonique dans la base via le tech_id depuis une demande dls                */
/* Entrée: le tech_id, l'acronyme, le libelle                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_HORLOGE_from_dls ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  {
/******************************************** Préparation de la base du mnemo *************************************************/
    gchar *acro = Normaliser_chaine ( acronyme );                                            /* Formatage correct des chaines */
    if ( !acro )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for acronyme." );
       return(FALSE);
     }

    gchar *libelle = Normaliser_chaine ( libelle_src );                                      /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new ( __func__, LOG_ERR, domain, "Normalize error for libelle." );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour = DB_Write ( domain,                                                                     /* Requete SQL */
                                 "INSERT INTO mnemos_HORLOGE SET deletable=1, tech_id='%s', acronyme='%s', libelle='%s' "
                                 "ON DUPLICATE KEY UPDATE libelle=IF(deletable=1, VALUES(libelle), libelle)",
                                 tech_id, acro, libelle );
    g_free(acro);
    g_free(libelle);
    return (retour);
  }
/******************************************************************************************************************************/
/* RUN_HORLOGES_LOAD_request_get: Repond aux requests du domain                                                               */
/* Entrées: les éléments libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_HORLOGES_LOAD_request_get ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *url_param )
  { JsonNode *RootNode = Http_json_node_create(msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "horloges",
                                "SELECT * "
                                "FROM mnemos_HORLOGE as m "
                                "INNER JOIN mnemos_HORLOGE_ticks AS t ON m.mnemo_horloge_id = t.horloge_id" );

    if (retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "These are Horloges", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_HORLOGE_ADD_request_post: Repond aux requests Thread des agents                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_HORLOGE_ADD_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))         return;
    gchar *tech_id  = Json_get_string ( request, "tech_id" );
    gchar *acronyme = Json_get_string ( request, "acronyme" );
    gchar *libelle         = Json_get_string ( request, "libelle" );

    gboolean retour = Mnemo_auto_create_HORLOGE_from_thread( domain, tech_id, acronyme, libelle );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_HORLOGE_DEL_TICK_request_post: Repond aux requests Thread des agents                                                   */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_HORLOGE_DEL_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" )) return;
    gchar *tech_id  = Json_get_string ( request, "tech_id" );
    gchar *acronyme = Json_get_string ( request, "acronyme" );

    gboolean retour = DB_Write ( domain, "DELETE t FROM mnemos_HORLOGE_ticks AS t INNER JOIN mnemos_HORLOGE AS h ON t.horloge_id = h.mnemo_horloge_id "
                                         " WHERE h.tech_id='%s' AND h.acronyme='%s'", tech_id, acronyme );
    AGENT_send_to_agent ( domain, NULL, "RELOAD_HORLOGE_TICK", NULL );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/******************************************************************************************************************************/
/* RUN_HORLOGE_ADD_TICK_request_post: Repond aux requests Thread des agents                                                   */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_HORLOGE_ADD_TICK_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "heure" ))           return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "minute" ))          return;
    gchar *tech_id  = Json_get_string ( request, "tech_id" );
    gchar *acronyme = Json_get_string ( request, "acronyme" );
    gint heure             = Json_get_int    ( request, "heure" );
    gint minute            = Json_get_int    ( request, "minute" );

    gboolean retour = DB_Write ( domain, "INSERT INTO mnemos_HORLOGE_ticks SET "
                                         "horloge_id=(SELECT mnemo_HORLOGE_id FROM mnemos_HORLOGE WHERE tech_id='%s' AND acronyme='%s'),"
                                         "heure=%d, minute=%d",
                                 tech_id, acronyme, heure, minute );
    AGENT_send_to_agent ( domain, NULL, "RELOAD_HORLOGE_TICK", NULL );
    Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
