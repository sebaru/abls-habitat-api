/******************************************************************************************************************************/
/* dls_ârams.c                      Gestion des parametres dls dans l'API HTTP WebService                                     */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                02.02.2025 12:15:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls_params.c
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
/* DLS_PARAMS_request_get: Renvoie les paramètres d'un code DLS                                                               */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PARAMS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, url_param, "tech_id" ))   return;

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *tech_id = Normaliser_chaine ( Json_get_string ( url_param, "tech_id" ) );         /* Formatage correct des chaines */
    if (!tech_id) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    gboolean retour = DB_Read ( domain, RootNode, "params",
                                "SELECT p.dls_param_id, p.acronyme, p.libelle, p.valeur FROM dls_params AS p "
                                "INNER JOIN dls AS d USING(tech_id) "
                                "INNER JOIN syns AS s USING(syn_id) "
                                "WHERE s.access_level<='%d' AND tech_id='%s'"
                                "ORDER BY acronyme",
                                 user_access_level, tech_id );
    g_free(tech_id);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Parameters given", RootNode );
  }
/******************************************************************************************************************************/
/* DLS_PARAMS_SET_request_post: Positionne les paramètres d'un code DLS                                                       */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_PARAMS_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "dls_param_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "valeur" ))       return;

    gchar *valeur = Normaliser_chaine ( Json_get_string ( request, "valeur" ) );           /* Formatage correct des chaines */
    if (!valeur) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", NULL ); return; }

    gint dls_param_id = Json_get_int( request, "dls_param_id" );
    gboolean retour = DB_Write ( domain, "UPDATE dls_params AS p INNER JOIN dls USING (`tech_id`) INNER JOIN syns USING(`syn_id`) "
                                         "SET p.valeur='%s' WHERE p.dls_param_id=%d AND syns.access_level <= %d",
                                         valeur, dls_param_id, user_access_level );
    g_free(valeur);

    if (!retour) { Http_Send_json_response ( msg, FALSE, domain->mysql_last_error, NULL ); return; }
                                                                   /* On récupère le tech_id avant de demander la compilation */
    DB_Read ( domain, request, NULL, "SELECT tech_id FROM dls_params WHERE dls_param_id='%d'", dls_param_id );
    DLS_COMPIL_request_post ( domain, token, path, msg, request );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Parameter setted", NULL );
  }
/******************************************************************************************************************************/
/* Dls_update_one_parameter: Met a jour un parametre dans le sourcecode fourni                                                */
/* Entrée: le sourcecode, le parametre, sa valeur                                                                             */
/* Sortie: le sourcecode mis à jour                                                                                           */
/******************************************************************************************************************************/
 static void Dls_update_one_parameter ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gchar *acronyme = Json_get_string ( element, "acronyme" );
    gchar *valeur   = Json_get_string ( element, "valeur" );
    GString *sourcecode = user_data;
    gchar find [256];
    g_snprintf( find, sizeof(find), "$%s", acronyme );
    g_string_replace ( sourcecode, find, valeur, 0 );
  }
/******************************************************************************************************************************/
/* DLS_Apply_params: Applique les parametres d'un plugin D.L.S                                                                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Apply_params ( struct DOMAIN *domain, JsonNode *PluginNode )
  { if (!Json_has_member ( PluginNode, "tech_id" )) return;
    gchar *tech_id = Json_get_string( PluginNode, "tech_id" );
    Info_new( __func__, LOG_INFO, domain, "'%s': Applying params", tech_id );

    gchar target_string[128];
    JsonNode *ParamsNode = Json_node_create();                                         /* Récupère tous les parameters du DLS */
    DB_Read ( domain, ParamsNode, "params_value", "SELECT acronyme, valeur FROM dls_params WHERE tech_id='%s'", tech_id );
    JsonArray *params_value = Json_get_array ( ParamsNode, "params_value" );

    JsonNode *param_this = Json_node_create();                                                  /* Ajout de $THIS Replacement */
    Json_node_add_string ( param_this, "acronyme", "THIS" );
    Json_node_add_string ( param_this, "libelle", tech_id );
    Json_array_add_element ( params_value, param_this );

    JsonNode *param_tech_id = Json_node_create();                                                  /* Ajout de $THIS Replacement */
    Json_node_add_string ( param_tech_id, "acronyme", "DLS_TECH_ID" );
    Json_node_add_string ( param_tech_id, "libelle", tech_id );
    Json_array_add_element ( params_value, param_tech_id );

    JsonNode *param_dls_id = Json_node_create();                                              /* Ajout de $DLS_ID Replacement */
    Json_node_add_string ( param_dls_id, "acronyme", "DLS_ID" );
    g_snprintf ( target_string, sizeof(target_string), "%d", Json_get_int ( PluginNode, "dls_id" ) );
    Json_node_add_string ( param_dls_id, "libelle", target_string );
    Json_array_add_element ( params_value, param_dls_id );

    JsonNode *param_page = Json_node_create();                                                  /* Ajout de $THIS Replacement */
    Json_node_add_string ( param_page, "acronyme", "SYN_PAGE" );
    Json_node_add_string ( param_page, "libelle", Json_get_string ( PluginNode, "page" ) );
    Json_array_add_element ( params_value, param_page );

    JsonNode *param_syn_id = Json_node_create();                                              /* Ajout de $DLS_ID Replacement */
    Json_node_add_string ( param_syn_id, "acronyme", "SYN_ID" );
    g_snprintf ( target_string, sizeof(target_string), "%d", Json_get_int ( PluginNode, "syn_id" ) );
    Json_node_add_string ( param_syn_id, "libelle", target_string );
    Json_array_add_element ( params_value, param_syn_id );

    GString *sourcecode_string = g_string_new ( Json_get_string ( PluginNode, "sourcecode" ) );     /* Apply all replacements */
    Json_node_foreach_array_element ( ParamsNode, "params_value", Dls_update_one_parameter, sourcecode_string );
    gchar *sourcecode_updated = g_string_free_and_steal ( sourcecode_string );
    Json_node_add_string ( PluginNode, "sourcecode", sourcecode_updated );
    g_free(sourcecode_updated);
    Info_new( __func__, LOG_INFO, domain, "'%s': Parameters set", tech_id );
    json_node_unref ( ParamsNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
