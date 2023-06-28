/******************************************************************************************************************************/
/* tableaux.c                      Gestion des tableaux dans l'API HTTP WebService                                            */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                27.06.2023 20:32:07 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * tableaux.c
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
/* TABLEAU_SET_request_post: Ajoute ou modifie un tableau                                                                     */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if (Http_fail_if_has_not ( domain, path, msg, request, "titre" ))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" )) return;

    gboolean retour = FALSE;
    gint syn_id = Json_get_int ( request, "syn_id" );
    gchar *titre = Normaliser_chaine ( Json_get_string ( request, "titre" ) );
    if ( Json_has_member ( request, "tableau_id" ) )
     { gint tableau_id = Json_get_int ( request, "tableau_id" );
       retour = DB_Write ( domain, "UPDATE tableau INNER JOIN syns USING(`syn_id`) "
                                   "SET titre='%s', syn_id='%d' WHERE tableau_id='%d' AND access_level<='%d'",
                                   titre, syn_id, tableau_id, user_access_level );
     }
    else
     { retour = DB_Write ( domain, "INSERT INTO tableau SET titre='%s', syn_id='%d'", titre, syn_id ); }
    g_free(titre);
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau Set", NULL );
  }
/******************************************************************************************************************************/
/* TABLEAU_DELETE_request_post: Retire un tableau                                                                             */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tableau_id" )) return;
    gint user_access_level = Json_get_int ( token, "access_level" );

    gint tableau_id = Json_get_int ( request, "tableau_id" );

    gboolean retour = DB_Write ( domain,
                                 "DELETE  tableau FROM tableau INNER JOIN syns USING(`syn_id`) WHERE tableau_id=%d AND access_level<=%d",
                                 tableau_id, user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau deleted", NULL );
  }
/******************************************************************************************************************************/
/* TABLEAU_LIST_request_get: Liste les tableaux accessibles                                                                   */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_LIST_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "tableaux",
                                "SELECT t.*,syn.page FROM tableau AS t INNER JOIN syns AS syn USING(`syn_id`) ",
                                "WHERE syn.access_level<='%d' ORDER BY t.titre", user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Tableau list done", RootNode );
  }
#ifdef bouh

/******************************************************************************************************************************/
/* TABLEAU_SHOW_request_get: Envoie les composants d'un tableau                                                         */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void TABLEAU_SHOW_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { gboolean retour = FALSE;
    if (!Http_is_authorized ( domain, token, path, msg, 0 )) return;
    Http_print_request ( domain, token, path );

    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    if (Json_has_member ( url_param, "syn_page" ) )                                /* Récupération du tableau via syn_page */
     { gchar *syn_page = Normaliser_chaine ( Json_get_string ( url_param, "syn_page" ) );
       retour = DB_Read ( domain, RootNode, NULL,
                          "SELECT syn_id, access_level, libelle FROM syns WHERE page='%s' AND access_level <= %d", syn_page, user_access_level );
       g_free(syn_page);
     }
    else                                                                              /* Sinon récupération du tableau n°1 */
     { retour = DB_Read ( domain, RootNode, NULL,
                          "SELECT syn_id, access_level, libelle FROM syns WHERE syn_id=1 AND access_level <= %d", user_access_level );
     }

    if ( !Json_has_member ( RootNode, "access_level" ))                                                      /* Si pas trouvé */
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Syn unknown", RootNode ); return; }
    gint syn_id = Json_get_int ( RootNode, "syn_id" );

/*---------------------------------------------- Lit les données du syn lui-meme ---------------------------------------------*/
    DB_Read ( domain, RootNode, NULL, "SELECT * FROM syns WHERE syn_id='%d' AND access_level<='%d'", syn_id, user_access_level);

/*---------------------------------------------- Envoi les données des tableaux parents -----------------------------------*/
    JsonArray *parents = Json_node_add_array ( RootNode, "parent_syns" );
    gint cur_syn_id = syn_id;
    while ( cur_syn_id != 1 )
     { JsonNode *cur_syn = Json_node_create();
       if (!cur_syn) break;
       DB_Read ( domain, cur_syn, NULL, "SELECT syn_id, parent_id, page, image, libelle FROM syns WHERE syn_id=%d", cur_syn_id );
       Json_array_add_element ( parents, cur_syn );
       cur_syn_id = Json_get_int ( cur_syn, "parent_id" );
     }

/*-------------------------------------------------- Envoi les data des tableaux fils -------------------------------------*/
    DB_Read ( domain, RootNode, "child_syns",
                                "SELECT s.* FROM syns AS s INNER JOIN syns as s2 ON s.parent_id=s2.syn_id "
                                "WHERE s2.syn_id='%d' AND s.syn_id!=1 AND s.access_level<='%d'",
                                syn_id, user_access_level);

/*-------------------------------------------------- Envoi les syn_vars ------------------------------------------------------*/
/*    JsonArray *syn_vars = Json_node_add_array ( tableau, "syn_vars" );
    Dls_foreach_syns ( syn_vars, Dls_syn_vars_to_json );*/

/*-------------------------------------------------- Envoi les liens ---------------------------------------------------------*/
/*    DB_Read ( domain, RootNode, "liens",
                                "SELECT lien.* FROM syns_liens AS lien "
                                "INNER JOIN syns as syn ON lien.syn_id=syn.syn_id "
                                "WHERE lien.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );
/*-------------------------------------------------- Envoi les rectangles ----------------------------------------------------*/
/*    DB_Read ( domain, RootNode, "rectangles",
                                "SELECT rectangle.* FROM syns_rectangles AS rectangle "
                                "INNER JOIN syns as syn ON rectangle.syn_id=syn.syn_id "
                                "WHERE rectangle.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );

/*-------------------------------------------------- Envoi les cameras -------------------------------------------------------*/
/*    DB_Read ( domain, RootNode, "cameras",
                                "SELECT cam.*,src.location,src.libelle FROM syns_camerasup AS cam "
                                "INNER JOIN cameras AS src ON cam.camera_src_id=src.id "
                                "INNER JOIN syns as syn ON cam.syn_id=syn.syn_id "
                                "WHERE cam.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );

/*-------------------------------------------------- Envoi les cadrans de la page --------------------------------------------*/
    DB_Read ( domain, RootNode, "cadrans",
                                "SELECT cadran.*, dico.classe, dico.libelle FROM syns_cadrans AS cadran "
                                "INNER JOIN dls AS dls ON cadran.dls_id=dls.dls_id "
                                "INNER JOIN syns AS syn ON dls.syn_id=syn.syn_id "
                                "INNER JOIN dictionnaire AS dico ON (cadran.tech_id=dico.tech_id AND cadran.acronyme=dico.acronyme) "
                                "WHERE syn.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );
/*-------------------------------------------------- Envoi les tableaux de la page -------------------------------------------*/
    DB_Read ( domain, RootNode, "tableaux",
                                "SELECT tableau.* FROM tableau "
                                "INNER JOIN syns as syn ON tableau.syn_id=syn.syn_id "
                                "WHERE tableau.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );
/*-------------------------------------------------- Envoi les tableaux_map de la page ---------------------------------------*/
    DB_Read ( domain, RootNode, "tableaux_map",
                                "SELECT tableau_map.* FROM tableau_map "
                                "INNER JOIN tableau ON tableau_map.tableau_id=tableau.tableau_id "
                                "INNER JOIN syns as syn ON tableau.syn_id=syn.syn_id "
                                "WHERE tableau.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );

/*-------------------------------------------------- Envoi les visuels de la page --------------------------------------------*/
    DB_Read ( domain, RootNode, "visuels",
                                "SELECT m.*,v.*,i.*,dls.tech_id AS dls_tech_id, dls.shortname AS dls_shortname, dls_owner.shortname AS dls_owner_shortname "
                                "FROM syns_motifs AS m "
                                "INNER JOIN mnemos_VISUEL AS v USING(mnemo_visuel_id) "                 /* du motif au visuel */
                                "INNER JOIN dls USING(dls_id) "                           /* recup du DLS hébergeant le motif */
                                "INNER JOIN syns AS s USING(syn_id) "                       /* Recup du syn hebergeant le dls */
                                "INNER JOIN dls AS dls_owner ON dls_owner.tech_id=v.tech_id "/* Recup du DLS source du visuel */
                                "INNER JOIN master.icons AS i USING(forme) "                            /* Lien avec la forme */
                                "WHERE s.syn_id='%d' AND s.access_level<=%d "
                                "ORDER BY layer",
                                syn_id, user_access_level, user_access_level);

/*------------------------------------------------- Envoi l'état de tous les visuels du tableau ---------------------------*/
    Json_node_foreach_array_element ( RootNode, "visuels", VISUEL_Add_etat_to_json, domain );

/*-------------------------------------------------- Envoi les horloges de la page -------------------------------------------*/
    DB_Read ( domain, RootNode, "horloges",
                                "SELECT DISTINCT horloge.tech_id, dls.name as dls_name FROM mnemos_HORLOGE AS horloge "
                                "INNER JOIN dls ON dls.tech_id=horloge.tech_id "
                                "INNER JOIN syns as syn ON dls.syn_id=syn.syn_id "
                                "WHERE dls.syn_id=%d AND syn.access_level<=%d",
                                syn_id, user_access_level );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn showed", RootNode );
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/