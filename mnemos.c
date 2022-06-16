/******************************************************************************************************************************/
/* mnemos.c                      Gestion des mnemoss dans l'API HTTP WebService                                               */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemos.c
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
/* MNEMOS_TECH_IDS_request_post: Recherche les tech_id sur la base d'un parametre                                             */
/* Entrées: la connexion browser                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void MNEMOS_TECH_IDS_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "tech_ids", "SELECT DISTINCT tech_id FROM dictionnaire" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List done", NULL );
  }
/******************************************************************************************************************************/
/* MNEMOS_VALIDATE_request_post: Valide les tech_id acronyme an parametre                                                     */
/* Entrées: la connexion browser                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void MNEMOS_VALIDATE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "classe" ))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme" )) return;

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string( request, "acronyme" ) );
    gchar *classe   = Normaliser_chaine ( Json_get_string( request, "classe" ) );

    gboolean retour = DB_Read ( domain, RootNode, "tech_ids_found",
                               "SELECT tech_id, name FROM dls WHERE tech_id LIKE '%%%s%%' ORDER BY tech_id", tech_id );

            retour &= DB_Read ( domain, RootNode, "acronymes_found",
                               "SELECT acronyme,libelle FROM dictionnaire WHERE tech_id='%s' AND acronyme LIKE '%%%s%%' "
                               "AND classe='%s' ORDER BY acronyme",
                                tech_id, acronyme, classe );
    g_free(acronyme);
    g_free(tech_id);
    g_free(classe);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "List done", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
