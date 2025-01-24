/******************************************************************************************************************************/
/* dictionnaire.c        Déclaration des fonctions pour la gestion du dictionnaire                                            */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dictionnaire.c
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
/* Rechercher_DICO: Recherche un bit interne dans le dictionnairepar son tech_id:acronyme                                     */
/* Entrée: le tech_id et acronyme                                                                                             */
/* Sortie: Un JsonNode ou NULL si erreur                                                                                      */
/******************************************************************************************************************************/
 JsonNode *Rechercher_DICO ( struct DOMAIN *domain, gchar *tech_id, gchar *acronyme )
  { JsonNode *result = Json_node_create ();
    if (!result) return(NULL);

    gboolean retour = DB_Read ( domain, result, NULL,
                                "SELECT * FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme
                              );
    if (!retour)
     { Info_new ( __func__, LOG_ERR, domain, "DB Error for '%s:%s' dans le dictionnaire", tech_id, acronyme );
       json_node_unref(result);
       result = NULL;
     }
    return(result);
  }
/******************************************************************************************************************************/
/* SEARCH_request_get: Liste le dictionnaire                                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void SEARCH_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    /*gint user_access_level = Json_get_int ( token, "access_level" );*/

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = FALSE;
    if ( Json_has_member ( url_param, "search" ) )
     { gchar *search = Normaliser_chaine ( Json_get_string ( url_param, "search" ) );
       retour = DB_Read ( domain, RootNode, "results",
                    "SELECT * FROM dictionnaire "
                    "WHERE classe LIKE '%%%s%%' OR tech_id LIKE '%%%s%%' OR acronyme LIKE '%%%s%%' OR libelle LIKE '%%%s%%' "
                    "LIMIT 200", search, search, search, search );
       g_free(search);
     }
    else retour = DB_Read ( domain, RootNode, "results", "SELECT * FROM dictionnaire LIMIT 200" );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Results of search", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
