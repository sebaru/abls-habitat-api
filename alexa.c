/******************************************************************************************************************************/
/* alexa.c                      Gestion des commandes vocales ALEXA dans l'API HTTP WebService                                */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                15.04.2025 21:56:37 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * alexa.c
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
/* ALEXA_request_post: Appelé depuis libsoup pour une commande vocale                                                         */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void ALEXA_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "request" ))  return;

    JsonNode *RootNode = Http_json_node_create(msg);
    if (!RootNode) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", NULL ); return; }
    Json_node_add_string ( RootNode, "version", "1.0" );
    JsonNode *response = Json_node_add_objet ( RootNode, "response" );
    Json_node_add_bool ( response, "shouldEndSession", true );
    JsonNode *outputSpeech = Json_node_add_objet ( response, "outputSpeech" );
    Json_node_add_string ( outputSpeech, "type", "PlainText" );

    JsonNode *request_element = Json_get_object_as_node ( request, "request" );

    gchar *type = Json_get_string ( request_element, "type" );
    if (!type) Info_new ( __func__, LOG_ERR, NULL, "ALEXA: No Type in Alexa Request" );
    else if (!strcmp(type, "LaunchRequest"))
     { Json_node_add_string ( outputSpeech, "text", "Application démarrée." );
       Info_new ( __func__, LOG_NOTICE, NULL, "ALEXA: Démarrage de l'application vocale" );
     }
    else if (!strcmp(type, "IntentRequest"))
     { JsonNode *intent = Json_get_object_as_node ( request_element, "intent" );
       gchar *name = Json_get_string ( intent, "name" );
       gchar chaine [256];
       g_snprintf ( chaine, sizeof(chaine), "J'ai reçu une intention %s", name );
       Info_new ( __func__, LOG_NOTICE, NULL, "ALEXA: %s", name );
       Json_node_add_string ( outputSpeech, "text", chaine );
     }
    else
     { Json_node_add_string ( outputSpeech, "text", "Désolé, je n'ai pas compris." );
       Info_new ( __func__, LOG_NOTICE, NULL, "ALEXA: Intent not recognized" );
     }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "OK", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
