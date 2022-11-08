/******************************************************************************************************************************/
/* histo.c              Déclaration des fonctions pour la gestion des historiques                                             */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                06.11.2022 15:22:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * message.c
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
/* HISTO_ALIVE_request_get: Renvoi les historiques vivant au user                                                             */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void HISTO_ALIVE_request_get ( struct DOMAIN *domain, JsonNode *token, gchar *path, SoupMessage *msg, JsonNode *url_param )
  { /*if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id")) return;*/

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "histo_msgs", "SELECT * FROM histo_msgs WHERE date_fin IS NULL ORDER BY date_create DESC" );
    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "you have histo alives", RootNode );
  }
/******************************************************************************************************************************/
/* RUN_HISTO_request_post: Enregistre un historique en base de données                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_HISTO_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupMessage *msg, JsonNode *request )
  { gboolean retour;
    if (Http_fail_if_has_not ( domain, path, msg, request, "alive"))       return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id"))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme"))    return;

    if (Json_get_bool ( request, "alive" ) == FALSE)
     { if (Http_fail_if_has_not ( domain, path, msg, request, "date_fin")) return; }
    else
     { if (Http_fail_if_has_not ( domain, path, msg, request, "libelle"))     return;
       if (Http_fail_if_has_not ( domain, path, msg, request, "date_create")) return;
     }

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id     = Normaliser_chaine ( Json_get_string ( request, "tech_id") );
    gchar *acronyme    = Normaliser_chaine ( Json_get_string ( request, "acronyme") );
    if (Json_get_bool ( request, "alive" ) == TRUE)
     { gchar *libelle     = Normaliser_chaine ( Json_get_string ( request, "libelle") );
       gchar *date_create = Normaliser_chaine ( Json_get_string ( request, "date_create") );
       retour = DB_Write ( domain, "INSERT INTO histo_msgs SET tech_id='%s', acronyme='%s', date_create='%s', libelle='%s',"
                                   "dls_shortname = (SELECT shortname FROM dls WHERE dls.tech_id='%s')",
                           tech_id, acronyme, date_create, libelle, tech_id );
       g_free(date_create);
       g_free(libelle);
     }
    else
     { gchar *date_fin = Normaliser_chaine ( Json_get_string ( request, "date_fin") );
       retour = DB_Write ( domain, "UPDATE histo_msgs SET date_fin='%s' WHERE tech_id='%s' AND acronyme='%s' AND date_fin IS NULL",
                           date_fin, tech_id, acronyme );
       g_free(date_fin);
     }
    g_free(acronyme);
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Histo saved", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
