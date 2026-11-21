/******************************************************************************************************************************/
/* visuel.c                      Gestion des visuels dans l'API HTTP WebService                                               */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * visuel.c
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

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* VISUEL_Lookup: Cherche un visuel dans le Tree                                                                              */
/* Entrées: les tech_id/acrnonyme du visuel sous forme json                                                                   */
/* Sortie : néant. L'appelant doit détenir domain->Visuels_sync (reader ou writer) et tracer lui-même un visuel inconnu       */
/******************************************************************************************************************************/
 static JsonNode *VISUEL_Lookup ( struct DOMAIN *domain, JsonNode *source )
  { if (!source || !domain->Visuels) return(NULL);
    JsonNode *visuel = g_tree_lookup ( domain->Visuels, source );
    if (!visuel)
     { gchar *tech_id  = Json_get_string ( source, "tech_id" );
       gchar *acronyme = Json_get_string ( source, "acronyme" );
            if (!tech_id)  { Info ( __func__, "visuel", domain->uuid, LOG_ERR, "Visuel unknown: tech_id is null." ); }
       else if (!acronyme) { Info ( __func__, "visuel", domain->uuid, LOG_ERR, "Visuel unknown: acronyme is null" ); }
     }
    return(visuel);
  }
/******************************************************************************************************************************/
/* VISUEL_Update_tree_by_array: Met a jour l'arbre des visuels pour le visuel en parametres                                   */
/* Entrées: le tableau, l'index, l'element, le domaine. L'appelant détient domain->Visuels_sync en writer                     */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void VISUEL_Update_tree_by_array ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    if (!domain->Visuels) return;
    JsonNode *dest = VISUEL_Lookup ( domain, element );
    if (!dest)
     { json_node_ref ( element );
       g_tree_insert ( domain->Visuels, element, element );
     }
    else
     { Json_add_string ( dest, "forme",         Json_get_string ( element, "forme" ) );
       Json_add_string ( dest, "mode",          Json_get_string ( element, "mode" ) );
       Json_add_string ( dest, "color",         Json_get_string ( element, "color" ) );
       Json_add_string ( dest, "badge",         Json_get_string ( element, "badge" ) );
       Json_add_bool   ( dest, "cligno",        Json_get_bool   ( element, "cligno" ) );
       Json_add_bool   ( dest, "noshow",        Json_get_bool   ( element, "noshow" ) );
       Json_add_bool   ( dest, "disable",       Json_get_bool   ( element, "disable" ) );
       Json_add_int    ( dest, "nb_decimal",    Json_get_int    ( element, "nb_decimal" ) );
       Json_add_double ( dest, "minimum",       Json_get_double ( element, "minimum" ) );
       Json_add_double ( dest, "maximum",       Json_get_double ( element, "maximum" ) );
       Json_add_double ( dest, "seuil_ntb",     Json_get_double ( element, "seuil_ntb" ) );
       Json_add_double ( dest, "seuil_nb",      Json_get_double ( element, "seuil_nb" ) );
       Json_add_double ( dest, "seuil_nh",      Json_get_double ( element, "seuil_nh" ) );
       Json_add_double ( dest, "seuil_nth",     Json_get_double ( element, "seuil_nth" ) );
       Json_add_string ( dest, "unite",         Json_get_string ( element, "unite" ) );
       Json_add_string ( dest, "libelle",       Json_get_string ( element, "libelle" ) );
       Json_add_string ( dest, "input_libelle", Json_get_string ( element, "input_libelle" ) );
       Json_add_bool   ( dest, "rw",            Json_get_bool   ( element, "rw" ) );
     }
  }
/******************************************************************************************************************************/
/* VISUEL_Update_tree_by_tech_id: Met a jour les paramètres statiques des visuels d'un tech_id donné                          */
/* Entrées: le domain, le tech_id (ou NULL si ALL)                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUEL_Update_tree_by_tech_id ( struct DOMAIN *domain, gchar *tech_id_src )
  { JsonNode *RootNode = Json_create ();
    if(!RootNode) return;
    gchar requete[1024];
    g_snprintf ( requete, sizeof(requete),
                "SELECT v.*, d.unite, d.libelle AS input_libelle FROM mnemos_VISUEL AS v "
                "LEFT JOIN dictionnaire AS d ON (v.input_tech_id = d.tech_id AND v.input_acronyme = d.acronyme) " );
    if (tech_id_src)
     { gchar *tech_id  = Normaliser_chaine ( tech_id_src );                                  /* Formatage correct des chaines */
       if ( !tech_id )
        { Info ( __func__, "visuel", domain->uuid, LOG_ERR, "Normalize error for tech_id." );
          Json_unref ( RootNode );
          return;
        }
       g_strlcat ( requete, "WHERE v.tech_id='", sizeof(requete) );
       g_strlcat ( requete, tech_id, sizeof(requete) );
       g_strlcat ( requete, "'", sizeof(requete) );
       g_free(tech_id);
     }
    DB_Read ( domain, RootNode, "visuels", requete );
    g_rw_lock_reader_lock ( &domain->Visuels_sync );
    Json_foreach_array_element ( RootNode, "visuels", VISUEL_Update_tree_by_array, domain );
    g_rw_lock_reader_unlock ( &domain->Visuels_sync );
    Json_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* VISUEL_save_one_to_db: Enregistre un visuel en base                                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean VISUEL_save_one_to_db ( gpointer key, gpointer value, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    JsonNode *visuel = value;
    gchar *tech_id   = Normaliser_chaine ( Json_get_string ( visuel, "tech_id"  ) );
    gchar *acronyme  = Normaliser_chaine ( Json_get_string ( visuel, "acronyme" ) );
    gchar *libelle   = Normaliser_chaine ( Json_get_string ( visuel, "libelle"  ) );
    gchar *mode      = Normaliser_chaine ( Json_get_string ( visuel, "mode"     ) );
    gchar *color     = Normaliser_chaine ( Json_get_string ( visuel, "color"    ) );
    gchar *badge     = Normaliser_chaine ( Json_get_string ( visuel, "badge"    ) );
    gdouble  valeur  = Json_get_double ( visuel, "valeur" );
    gboolean cligno  = Json_get_bool   ( visuel, "cligno" );
    gboolean noshow  = Json_get_bool   ( visuel, "noshow" );
    gboolean disable = Json_get_bool   ( visuel, "disable" );
    DB_Write ( domain, "INSERT INTO mnemos_VISUEL SET tech_id='%s', acronyme='%s', "
                       "libelle='%s', mode='%s', color='%s', valeur='%f', cligno='%d', noshow='%d', disable='%d', "
                       "badge='%s' "
                       "ON DUPLICATE KEY UPDATE libelle=VALUE(libelle), mode=VALUE(mode), color=VALUE(color), "
                       "valeur=VALUE(valeur), cligno=VALUE(cligno), noshow=VALUE(noshow), disable=VALUE(disable), "
                       "badge=VALUE(badge) ",
                       tech_id, acronyme, libelle, mode, color, valeur, cligno, noshow, disable, badge );
    g_free(tech_id);
    g_free(acronyme);
    g_free(libelle);
    g_free(mode);
    g_free(color);
    g_free(badge);
    return(FALSE);
  }
/******************************************************************************************************************************/
/* VISUEL_Load: Charge les visuels d'un domain                                                                                */
/* Entrée: le domaine                                                                                                         */
/******************************************************************************************************************************/
 void VISUEL_Load_all ( struct DOMAIN *domain )
  { if (domain->Visuels)
     { Info ( __func__, "visuel", domain->uuid, LOG_INFO, "Visuels already loaded" );
       return;
     }
    g_rw_lock_writer_lock ( &domain->Visuels_sync );
    domain->Visuels = g_tree_new_full( (GCompareDataFunc) DOMAIN_Comparer_tree_clef_for_bit, domain, NULL, (GDestroyNotify) Json_unref );
    g_rw_lock_writer_unlock ( &domain->Visuels_sync );
    if (!domain->Visuels)
     { Info ( __func__, "visuel", domain->uuid, LOG_ERR, "Unable to load visuels (g_tree error)" );
       return;
     }

    JsonNode *RootNode = Json_create ();
    if (!RootNode)
     { Info ( __func__, "visuel", domain->uuid, LOG_ERR, "Unable to load visuels (JsonNode error)" );
       return;
     }
    VISUEL_Update_tree_by_tech_id ( domain, NULL );                     /* Update de tous les visuels, tous tech_id confondus */
    Json_unref ( RootNode );
    Info ( __func__, "visuel", domain->uuid, LOG_INFO, "%04d visuels loaded", VISUEL_get_count ( domain ) );
  }
/******************************************************************************************************************************/
/* VISUEL_get_count: Renvoie le nombre de visuels chargés en mémoire pour le domaine                                          */
/* Entrée: le domaine                                                                                                         */
/* Sortie: le nombre de visuels (0 si pas d'arbre)                                                                            */
/******************************************************************************************************************************/
 gint VISUEL_get_count ( struct DOMAIN *domain )
  { g_rw_lock_reader_lock ( &domain->Visuels_sync );
    gint nbr_visuels = (domain->Visuels ? g_tree_nnodes ( domain->Visuels ) : 0);
    g_rw_lock_reader_unlock ( &domain->Visuels_sync );
    return(nbr_visuels);
  }
/******************************************************************************************************************************/
/* VISUEL_Unload_all: Sauve et Décharge les visuels d'un domain                                                               */
/* Entrée: le domaine                                                                                                         */
/******************************************************************************************************************************/
 void VISUEL_Unload_all ( struct DOMAIN *domain )
  { if (!domain->Visuels) return;
    g_rw_lock_writer_lock ( &domain->Visuels_sync );
    gint nbr_visuels = g_tree_nnodes ( domain->Visuels );
    g_tree_foreach ( domain->Visuels, VISUEL_save_one_to_db, domain );
    g_tree_destroy ( domain->Visuels );
    domain->Visuels = NULL;
    g_rw_lock_writer_unlock ( &domain->Visuels_sync );
    Info ( __func__, "visuel", domain->uuid, LOG_INFO, "%04d visuels saved to DB", nbr_visuels );
  }
/******************************************************************************************************************************/
/* VISUEL_Add_etat_to_json: Ajoute l'état d'un visuel dans le json associé                                                    */
/* Entrées : Le visuel, le domain dans user_data. L'appelant détient domain->Visuels_sync en reader                           */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void VISUEL_Add_etat_to_json ( JsonArray *array, guint index, JsonNode *visuel_dest, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    JsonNode *visuel_source = VISUEL_Lookup ( domain, visuel_dest );
    if (visuel_source)
     { Json_add_string ( visuel_dest, "libelle", Json_get_string ( visuel_source, "libelle" ) );
       Json_add_string ( visuel_dest, "mode",    Json_get_string ( visuel_source, "mode" ) );
       Json_add_string ( visuel_dest, "color",   Json_get_string ( visuel_source, "color" ) );
       Json_add_string ( visuel_dest, "badge",   Json_get_string ( visuel_source, "badge" ) );
       Json_add_double ( visuel_dest, "valeur",  Json_get_double ( visuel_source, "valeur" ) );
       Json_add_bool   ( visuel_dest, "cligno",  Json_get_bool   ( visuel_source, "cligno" ) );
       Json_add_bool   ( visuel_dest, "noshow",  Json_get_bool   ( visuel_source, "noshow" ) );
       Json_add_bool   ( visuel_dest, "disable", Json_get_bool   ( visuel_source, "disable" ) );
     }
  }
/******************************************************************************************************************************/
/* VISUEL_Add_etats_to_json: Ajoute les états de chaque visuel du tableau array_name de RootNode                              */
/* Entrées : le domaine, le RootNode, le nom du tableau des visuels                                                           */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUEL_Add_etats_to_json ( struct DOMAIN *domain, JsonNode *RootNode, gchar *array_name )
  { g_rw_lock_reader_lock ( &domain->Visuels_sync );
    Json_foreach_array_element ( RootNode, array_name, VISUEL_Add_etat_to_json, domain );
    g_rw_lock_reader_unlock ( &domain->Visuels_sync );
  }
/******************************************************************************************************************************/
/* VISUEL_Handle_one: Traite un visuel recu du Master                                                                         */
/* Entrées: le jsonnode représentant le bit interne et sa valeur                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void VISUEL_Handle_one ( struct DOMAIN *domain, JsonNode *visuel_source )
  { if ( !Json_has_member ( visuel_source, "tech_id"  ) ) return;
    if ( !Json_has_member ( visuel_source, "acronyme" ) ) return;

    gchar *tech_id   = Json_get_string ( visuel_source, "tech_id" );
    gchar *acronyme  = Json_get_string ( visuel_source, "acronyme" );

    g_rw_lock_reader_lock ( &domain->Visuels_sync );
    JsonNode *visuel_in_tree = VISUEL_Lookup ( domain, visuel_source );                /* Récupération du visuel dans l'arbre */
    if (!visuel_in_tree)
     { g_rw_lock_reader_unlock ( &domain->Visuels_sync );
       Info ( __func__, "visuel", domain->uuid, LOG_ERR, "Visuel '%s:%s' unknown.", tech_id, acronyme );
       return;
     }
                                                                /* Recopie des champs du visuel_source dans le visuel_in_tree */
    if ( Json_has_member ( visuel_source, "libelle"  ) )
     { Json_add_string ( visuel_in_tree, "libelle",  Json_get_string ( visuel_source, "libelle" ) ); }

    if ( Json_has_member ( visuel_source, "mode"     ) )
     { Json_add_string ( visuel_in_tree, "mode",  Json_get_string ( visuel_source, "mode" ) ); }

    if ( Json_has_member ( visuel_source, "color"    ) )
     { Json_add_string ( visuel_in_tree, "color",  Json_get_string ( visuel_source, "color" ) ); }

    if ( Json_has_member ( visuel_source, "badge"    ) )
     { Json_add_string ( visuel_in_tree, "badge",  Json_get_string ( visuel_source, "badge" ) ); }

    if ( Json_has_member ( visuel_source, "valeur"   ) )
     { Json_add_double ( visuel_in_tree, "valeur", Json_get_double ( visuel_source, "valeur" ) ); }

    if ( Json_has_member ( visuel_source, "cligno"   ) )
     { Json_add_bool ( visuel_in_tree, "cligno", Json_get_bool ( visuel_source, "cligno" ) ); }

    if ( Json_has_member ( visuel_source, "noshow"   ) )
     { Json_add_bool ( visuel_in_tree, "noshow", Json_get_bool ( visuel_source, "noshow" ) ); }

    if ( Json_has_member ( visuel_source, "disable"  ) )
     { Json_add_bool ( visuel_in_tree, "disable", Json_get_bool ( visuel_source, "disable" ) ); }

    JsonNode *visuel_to_send = Json_create ();/* Préparation du */
    if (!visuel_to_send)
     { g_rw_lock_reader_unlock ( &domain->Visuels_sync );
       Info ( __func__, "visuel", domain->uuid, LOG_ALERT, "Visuel '%s:%s': memory error.", tech_id, acronyme );
       return;
     }

    Info ( __func__, "visuel", domain->uuid, LOG_DEBUG,
           "Visuel '%s:%s' set to '%s' '%s' %f %s, decimal=%d, cligno=%d, noshow=%d, '%s', disable=%d badge='%s'",
           Json_get_string ( visuel_in_tree, "tech_id" ), Json_get_string ( visuel_in_tree, "acronyme" ),
           Json_get_int ( visuel_in_tree, "mode" ), Json_get_string ( visuel_in_tree, "color" ),
           Json_get_double ( visuel_in_tree, "valeur" ), Json_get_string ( visuel_in_tree, "unite" ),
           Json_get_int ( visuel_in_tree, "nb_decimal" ),
           Json_get_bool ( visuel_in_tree, "cligno" ), Json_get_bool ( visuel_in_tree, "noshow" ),
           Json_get_string ( visuel_in_tree, "libelle" ), Json_get_bool ( visuel_in_tree, "disable" ),
           Json_get_string ( visuel_in_tree, "badge" ) );

    JsonNode *RootNode = Json_create ();                                  /* Recherche les pages avant d'envoyer aux browsers */
    if(RootNode)
     { DB_Read ( domain, RootNode, "pages",
                 "SELECT DISTINCT syns.page FROM syns "
                 "INNER JOIN dls USING(syn_id) "
                 "INNER JOIN syns_motifs USING(dls_id) "
                 "INNER JOIN mnemos_VISUEL AS v USING(mnemo_visuel_id) "
                 "WHERE v.tech_id='%s' AND v.acronyme='%s'", tech_id, acronyme );
       GList *Pages = json_array_get_elements ( Json_get_array ( RootNode, "pages" ) );
       GList *pages = Pages;
       while(pages)
        { JsonNode *element = pages->data;
          gchar *page = Json_get_string ( element, "page" );
          MQTT_Send_to_browsers ( domain, "DLS_VISUEL", page, visuel_in_tree );                         /* Envoi aux browsers */
          pages = g_list_next(pages);
        }
       g_list_free(Pages);
       Json_unref ( RootNode );
     } else Info ( __func__, "visuel", domain->uuid, LOG_ALERT, "Visuel '%s:%s': memory error. Cannot send to browsers.",
                   Json_get_string ( visuel_in_tree, "tech_id" ), Json_get_string ( visuel_in_tree, "acronyme" ) );
    g_rw_lock_reader_unlock ( &domain->Visuels_sync );
  }
/******************************************************************************************************************************/
/* VISUEL_DELETE_request: Supprime les visuels en mémoire                                                                     */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void VISUEL_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (!domain->Visuels) return;
    g_rw_lock_writer_lock ( &domain->Visuels_sync );
    gint nbr_visuels = g_tree_nnodes ( domain->Visuels );
    g_tree_foreach ( domain->Visuels, VISUEL_save_one_to_db, domain );
    g_tree_remove_all ( domain->Visuels );
    g_rw_lock_writer_unlock ( &domain->Visuels_sync );
    Info ( __func__, "visuel", domain->uuid, LOG_INFO, "%04d visuels cleared", nbr_visuels );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Visuels cleared", NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
