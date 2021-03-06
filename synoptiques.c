/******************************************************************************************************************************/
/* synoptiques.c                      Gestion des synoptiquess dans l'API HTTP WebService                                     */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                17.06.2022 08:32:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * synoptiques.c
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
/* SYNOPTIQUE_SET_request_post: Ajoute un synoptique                                                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );
    gint user_access_level = Json_get_int ( token, "access_level" );

    if ( Json_has_member ( request, "syn_id" ) )
     { gint syn_id = Json_get_int ( request, "syn_id" );

       if (Json_has_member ( request, "image" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "image" ) );
          gboolean retour = DB_Write ( domain, "UPDATE syns SET image='%s' WHERE syn_id='%d' AND access_level<='%d'",
                                       chaine, syn_id, user_access_level );
          g_free(chaine);
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "libelle" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "libelle" ) );
          gboolean retour = DB_Write ( domain, "UPDATE syns SET libelle='%s' WHERE syn_id='%d' AND access_level<='%d'",
                                       chaine, syn_id, user_access_level );
          g_free(chaine);
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "page" ) )
        { gchar *chaine = Normaliser_chaine ( Json_get_string ( request, "page" ) );
          gboolean retour = DB_Write ( domain, "UPDATE syns SET page='%s' WHERE syn_id='%d' AND access_level<='%d'",
                                       chaine, syn_id, user_access_level );
          g_free(chaine);
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "mode_affichage" ) )
        { gboolean retour = DB_Write ( domain, "UPDATE syns SET mode_affichage='%d' WHERE syn_id='%d' AND access_level<='%d'",
                                       Json_get_bool ( request, "mode_affichage" ), syn_id, user_access_level );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "access_level" ) )
        { gboolean retour = DB_Write ( domain, "UPDATE syns SET access_level='%d' WHERE syn_id='%d' AND access_level<='%d'",
                                       Json_get_int ( request, "access_level" ), syn_id, user_access_level );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
        }

       if ( Json_has_member ( request, "parent_id" ) )
        { gboolean retour = DB_Write ( domain, "UPDATE syns SET parent_id='%d' WHERE syn_id='%d' AND access_level<='%d'",
                                       Json_get_int ( request, "parent_id" ),syn_id, user_access_level );
          if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
          /* Recalculer arbre syn */                                                                           /* Relance DLS */
        }
       Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn updated", NULL );
       return;
     }

    if (Http_fail_if_has_not ( domain, path, msg, request, "libelle" ))      return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "page" ))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "parent_id" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "access_level" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "image" ))        return;

    gint access_level = Json_get_int ( request, "access_level" );
    if (access_level>user_access_level)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Permission Denied", NULL ); return; }

    gint   parent_id   = Json_get_int ( request, "parent_id" );
    gchar *libelle     = Normaliser_chaine ( Json_get_string( request, "libelle" ) );
    gchar *page        = Normaliser_chaine ( Json_get_string( request, "page" ) );
    gchar *image       = Normaliser_chaine ( Json_get_string( request, "image" ) );

    gboolean retour = DB_Write ( domain, "INSERT INTO syns SET libelle='%s', parent_id=%d, page='%s', image='%s', "
                                         "access_level='%d'", libelle, parent_id, page, image, access_level );

    g_free(image);
    g_free(page);
    g_free(libelle);

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn added", NULL );
  }
/******************************************************************************************************************************/
/* SYNOPTIQUE_SET_request_post: Ajoute un synoptique                                                                          */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" ))  return;

    gint syn_id = Json_get_int ( request, "syn_id" );
    if (syn_id==1)
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Synoptique 1 cannot be deleted", NULL ); return; }

    gboolean retour = DB_Write ( domain,
                                 "DELETE FROM syns WHERE syn_id=%d AND access_level<='%d'",
                                 syn_id, Json_get_int ( token, "access_level" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Synoptique deleted", NULL );
  }
/******************************************************************************************************************************/
/* SYNOPTIQUE_LIST_request_post: Liste les synoptiques accessibles                                                            */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_LIST_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, "synoptiques",
                                "SELECT syn.*, psyn.page as ppage, psyn.libelle AS plibelle, psyn.syn_id AS pid, "
                                        "(SELECT COUNT(*) FROM dls WHERE dls.syn_id=syn.syn_id) AS dls_count, "
                                        "(SELECT COUNT(*) FROM syns AS sub_syn WHERE syn.syn_id=sub_syn.parent_id) AS subsyn_count "
                                "FROM syns AS syn "
                                "INNER JOIN syns AS psyn ON psyn.syn_id=syn.parent_id "
                                "WHERE syn.access_level<='%d' ORDER BY syn.page", Json_get_int ( token, "access_level" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Syn list done", RootNode );
  }
/******************************************************************************************************************************/
/* SYNOPTIQUE_SHOW_request_post: Envoie les composants d'un synoptique                                                        */
/* Entrées: les elements libsoup                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void SYNOPTIQUE_SHOW_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupMessage *msg, JsonNode *request )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 0 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "syn_id" ))  return;
    gint syn_id = Json_get_int ( request, "syn_id" );
    gint user_access_level = Json_get_int ( token, "access_level" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT access_level,libelle FROM syns WHERE syn_id=%d", syn_id );
    if ( !(Json_has_member ( RootNode, "access_level" ) && Json_has_member ( RootNode, "libelle" )) )
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Syn unknown", RootNode ); return; }

    if ( user_access_level < Json_get_int ( RootNode, "access_level" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_FORBIDDEN, "Access Denied", RootNode ); return; }

/*---------------------------------------------- Envoi les données -----------------------------------------------------------*/
    JsonArray *parents = Json_node_add_array ( RootNode, "parent_syns" );
    gint cur_syn_id = syn_id;
    while ( cur_syn_id != 1 )
     { JsonNode *cur_syn = Json_node_create();
       if (!cur_syn) break;
       DB_Read ( domain, cur_syn, NULL, "SELECT syn_id, parent_id, image, libelle FROM syns WHERE syn_id=%d", cur_syn_id );
       Json_array_add_element ( parents, cur_syn );
       cur_syn_id = Json_get_int ( cur_syn, "parent_id" );
     }

    DB_Read ( domain, RootNode, NULL, "SELECT * FROM syns WHERE syn_id='%d' AND access_level<='%d'", syn_id, user_access_level);

    gint full_syn = Json_get_int ( RootNode, "mode_affichage" );
/*-------------------------------------------------- Envoi les data des synoptiques fils -------------------------------------*/
    DB_Read ( domain, RootNode, "child_syns",
                                "SELECT s.* FROM syns AS s INNER JOIN syns as s2 ON s.parent_id=s2.syn_id "
                                "WHERE s2.syn_id='%d' AND s.syn_id!=1 AND s.access_level<='%d'",
                                syn_id, user_access_level);

/*-------------------------------------------------- Envoi les syn_vars ------------------------------------------------------*/
/*    JsonArray *syn_vars = Json_node_add_array ( synoptique, "syn_vars" );
    Dls_foreach_syns ( syn_vars, Dls_syn_vars_to_json );*/

/*-------------------------------------------------- Envoi les passerelles ---------------------------------------------------*/
    if (full_syn)
     { DB_Read ( domain, RootNode, "passerelles",
                                   "SELECT pass.*,syn.page,syn.libelle FROM syns_pass as pass "
                                   "INNER JOIN syns as syn ON pass.syn_cible_id=syn.syn_id "
                                   "WHERE pass.syn_id=%d AND syn.access_level<=%d",
                                    syn_id, user_access_level );
     }
/*-------------------------------------------------- Envoi les liens ---------------------------------------------------------*/
    if (full_syn)
     { DB_Read ( domain, RootNode, "liens",
                                   "SELECT lien.* FROM syns_liens AS lien "
                                   "INNER JOIN syns as syn ON lien.syn_id=syn.syn_id "
                                   "WHERE lien.syn_id=%d AND syn.access_level<=%d",
                                   syn_id, user_access_level );
     }
/*-------------------------------------------------- Envoi les rectangles ----------------------------------------------------*/
    if (full_syn)
     { DB_Read ( domain, RootNode, "rectangles",
                                   "SELECT rectangle.* FROM syns_rectangles AS rectangle "
                                   "INNER JOIN syns as syn ON rectangle.syn_id=syn.syn_id "
                                   "WHERE rectangle.syn_id=%d AND syn.access_level<=%d",
                                   syn_id, user_access_level );
     }
/*-------------------------------------------------- Envoi les commennts -----------------------------------------------------*/
    if (full_syn)
     { DB_Read ( domain, RootNode, "comments",
                                   "SELECT comment.* FROM syns_comments AS comment "
                                   "INNER JOIN syns as syn ON comment.syn_id=syn.syn_id "
                                   "WHERE comment.syn_id=%d AND syn.access_level<=%d",
                                   syn_id, user_access_level );
     }
/*-------------------------------------------------- Envoi les cameras -------------------------------------------------------*/
    DB_Read ( domain, RootNode, "cameras",
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

    /*Json_node_foreach_array_element ( RootNode, "cadrans", Http_abonner_cadran, session );*/
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
    if (full_syn)
     { DB_Read ( domain, RootNode, "visuels",
                                   "SELECT v.*,m.*,i.*,dls.shortname AS dls_shortname, "
                                   "  IF(i.layer IS NULL, 200, i.layer) AS layer,"
                                   "  IF(m.tech_id IS NULL, v.tech_id, m.tech_id) AS tech_id,"
                                   "  IF(m.acronyme IS NULL, v.acronyme, m.acronyme) AS acronyme, "
                                   "  IF(m.color IS NULL, v.color, m.color) AS color "
                                   "FROM syns_visuels AS v "
                                   "LEFT JOIN mnemos_VISUEL AS m ON v.mnemo_id = m.id "
                                   "LEFT JOIN dls ON dls.dls_id=v.dls_id "
                                   "LEFT JOIN icone AS i ON i.forme=m.forme "
                                   "LEFT JOIN syns AS s ON dls.syn_id=s.syn_id "
                                   "WHERE (s.syn_id='%d' AND s.access_level<=%d AND m.access_level<=%d) OR v.syn_id='%d' "
                                   "ORDER BY layer",
                                    syn_id, user_access_level, user_access_level, syn_id);
     }
    else
     { DB_Read ( domain, RootNode, "visuels",
                                   "SELECT v.*,m.*,i.*,dls.tech_id AS dls_tech_id, dls.shortname AS dls_shortname, dls_owner.shortname AS dls_owner_shortname "
                                   "FROM syns_visuels AS v "
                                   "INNER JOIN mnemos_VISUEL AS m ON v.mnemo_id = m.id "
                                   "INNER JOIN dls ON dls.dls_id=v.dls_id "
                                   "INNER JOIN icone AS i ON i.forme=m.forme "
                                   "INNER JOIN syns AS s ON dls.syn_id=s.syn_id "
                                   "INNER JOIN dls AS dls_owner ON dls_owner.tech_id=m.tech_id "
                                   "WHERE s.syn_id='%d' AND s.access_level<=%d AND m.access_level<=%d "
                                   "ORDER BY layer",
                                   syn_id, user_access_level, user_access_level);
     }
/*------------------------------------------------- Envoi l'état de tous les visuels du synoptique ---------------------------*/
    /*Json_node_foreach_array_element ( RootNode, "visuels", Http_add_etat_visuel_to_json, NULL );*/

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
/*----------------------------------------------------------------------------------------------------------------------------*/
