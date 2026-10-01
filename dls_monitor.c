/******************************************************************************************************************************/
/* API/dls_monitor.c      Gestion des observateurs temps réel des bits internes d'un module D.L.S                              */
/* Projet Abls-Habitat                   Gestion d'habitat                                                01.10.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls_monitor.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * Abls-Habitat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Abls-Habitat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Abls-Habitat; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include <string.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;

/******************************************************************************************************************************/
/* Dls_monitor_set: Envoie l'ordre de (dés)activation du monitoring au moteur D.L.S du domaine                                */
/* Entrée: le domaine, le tech_id du plugin et l'état souhaité                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_monitor_set ( struct DOMAIN *domain, gchar *tech_id, gboolean enable )
  { JsonNode *ToAgentNode = Json_create();
    if (!ToAgentNode)
     { Info ( __func__, "dls_monitor", domain->uuid, LOG_ERR, "Memory error, cannot send monitor order" ); return; }

    Json_add_bool   ( ToAgentNode, "enable", enable );
    MQTT_Send_to_domain ( domain, ToAgentNode, "DLS/MONITOR/%s", tech_id );
    Json_unref ( ToAgentNode );
  }
/******************************************************************************************************************************/
/* Dls_monitor_init: Prépare le registre des observateurs d'un domaine                                                        */
/* Entrée: le domaine                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_monitor_init ( struct DOMAIN *domain )
  { g_rw_lock_init ( &domain->Dls_monitor_watchers_sync );
    domain->Dls_monitor_watchers = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free,
                                                          (GDestroyNotify) g_hash_table_destroy );
  }
/******************************************************************************************************************************/
/* Dls_monitor_end: Libère le registre des observateurs d'un domaine                                                          */
/* Entrée: le domaine                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_monitor_end ( struct DOMAIN *domain )
  { if (!domain->Dls_monitor_watchers) return;
    g_rw_lock_writer_lock ( &domain->Dls_monitor_watchers_sync );
    g_hash_table_destroy ( domain->Dls_monitor_watchers );
    domain->Dls_monitor_watchers = NULL;
    g_rw_lock_writer_unlock ( &domain->Dls_monitor_watchers_sync );
    g_rw_lock_clear ( &domain->Dls_monitor_watchers_sync );
  }
/******************************************************************************************************************************/
/* Dls_monitor_count: Nombre d'observateurs encore actifs sur un plugin. A appeler sous mutex                                 */
/* Entrée: le domaine et le tech_id du plugin                                                                                 */
/* Sortie: le nombre d'observateurs                                                                                           */
/******************************************************************************************************************************/
 static guint Dls_monitor_count ( struct DOMAIN *domain, gchar *tech_id )
  { GHashTable *watchers = g_hash_table_lookup ( domain->Dls_monitor_watchers, tech_id );
    return ( watchers ? g_hash_table_size ( watchers ) : 0 );
  }
/******************************************************************************************************************************/
/* Dls_monitor_request_post: Un navigateur s'abonne, se réabonne ou se désabonne des bits internes d'un plugin                */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_MONITOR_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id" ))    return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "watcher_id" )) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "enable" ))     return;

    gchar *tech_id    = Json_get_string ( request, "tech_id" );
    gchar *watcher_id = Json_get_string ( request, "watcher_id" );
    gboolean enable   = Json_get_bool   ( request, "enable" );

    if (!domain->Dls_monitor_watchers)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Watchers not initialized", NULL ); return; }

    g_rw_lock_writer_lock ( &domain->Dls_monitor_watchers_sync );
    guint before = Dls_monitor_count ( domain, tech_id );

    if (enable)
     { GHashTable *watchers = g_hash_table_lookup ( domain->Dls_monitor_watchers, tech_id );
       if (!watchers)
        { watchers = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );
          g_hash_table_insert ( domain->Dls_monitor_watchers, g_strdup(tech_id), watchers );
        }
       g_hash_table_insert ( watchers, g_strdup(watcher_id), GINT_TO_POINTER(Global.Top) );
     }
    else
     { GHashTable *watchers = g_hash_table_lookup ( domain->Dls_monitor_watchers, tech_id );
       if (watchers)
        { g_hash_table_remove ( watchers, watcher_id );
          if (!g_hash_table_size ( watchers )) g_hash_table_remove ( domain->Dls_monitor_watchers, tech_id );
        }
     }

    guint after = Dls_monitor_count ( domain, tech_id );
    g_rw_lock_writer_unlock ( &domain->Dls_monitor_watchers_sync );

    if (!before && after)      Dls_monitor_set ( domain, tech_id, TRUE );
    else if (before && !after) Dls_monitor_set ( domain, tech_id, FALSE );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;
    Json_add_int ( RootNode, "nbr_watchers", after );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Watch updated", RootNode );
  }
/******************************************************************************************************************************/
/* Dls_monitor_check_all_watchers: Détermine si un observateur a dépassé son délai de validité                                */
/* Entrée: l'identifiant de l'observateur, sa dernière activité et la table des observateurs                                  */
/* Sortie: TRUE pour supprimer l'observateur périmé, FALSE pour le conserver                                                  */
/******************************************************************************************************************************/
 static gboolean Dls_monitor_check_all_watchers ( gpointer watcher_id, gpointer value, gpointer user_data )
  { GHashTable *watchers = user_data;
    gpointer last_seen = g_hash_table_lookup ( watchers, watcher_id );
    return ( Global.Top - GPOINTER_TO_INT(last_seen) > DLS_MONITOR_WATCHER_TTL );
  }
/******************************************************************************************************************************/
/* Dls_monitor_check_one_tech_id: Purge les observateurs périmés d'un plugin et relance l'ordre de monitoring si besoin       */
/* Entrée: le tech_id (clé), la table des observateurs (valeur), le domaine                                                   */
/* Sortie: TRUE si plus aucun observateur, pour retrait de la table parente                                                   */
/******************************************************************************************************************************/
 static gboolean Dls_monitor_check_one_tech_id ( gpointer tech_id, gpointer value, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    GHashTable *watchers  = value;
    guint expired = g_hash_table_foreach_remove ( watchers, Dls_monitor_check_all_watchers, watchers );
    if (expired) Info ( __func__, "dls_monitor", domain->uuid, LOG_INFO, "'%s': %u watcher(s) expired", (gchar *)tech_id, expired );

    if (!g_hash_table_size ( watchers ))
     { Dls_monitor_set ( domain, tech_id, FALSE );
       return(TRUE);
     }
    Dls_monitor_set ( domain, tech_id, TRUE );                     /* Rappel périodique: nourrit le chien de garde de l'agent */
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Dls_monitor_check_all_tech_id: Appelé périodiquement depuis la boucle principale, pour chaque domaine                      */
/* Entrée: le domain_uuid (clé), le domaine (valeur)                                                                          */
/* Sortie: FALSE pour continuer le parcours de l'arbre des domaines                                                           */
/******************************************************************************************************************************/
 gboolean Dls_monitor_check_all_tech_id ( gpointer domain_uuid, gpointer value, gpointer user_data )
  { struct DOMAIN *domain = value;

    if (!domain->Dls_monitor_watchers) return(FALSE);

    g_rw_lock_writer_lock ( &domain->Dls_monitor_watchers_sync );                           /* Pour tous les tech_id observés */
    g_hash_table_foreach_remove ( domain->Dls_monitor_watchers, Dls_monitor_check_one_tech_id, domain ); /* Controle des observateurs */
    g_rw_lock_writer_unlock ( &domain->Dls_monitor_watchers_sync );
    return(FALSE);
  }
/******************************************************************************************************************************/
/* DLS_MONITOR_Handle_one: Relaie aux navigateurs un lot de bits internes remonté par le moteur D.L.S                         */
/* Entrée: le domaine, le tech_id du plugin et le message reçu                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DLS_MONITOR_Handle_one ( struct DOMAIN *domain, gchar *tech_id, JsonNode *request )
  { if (!domain->Dls_monitor_watchers) return;

    g_rw_lock_reader_lock ( &domain->Dls_monitor_watchers_sync );
    guint nbr_watchers = Dls_monitor_count ( domain, tech_id );
    g_rw_lock_reader_unlock ( &domain->Dls_monitor_watchers_sync );

    if (!nbr_watchers)                                        /* Agent bavard: personne ne regarde, on lui demande de stopper */
     { Info ( __func__, "dls_monitor", domain->uuid, LOG_INFO, "'%s': no watcher, asking agent to stop monitoring", tech_id );
       Dls_monitor_set ( domain, tech_id, FALSE );
       return;
     }

    MQTT_Send_to_browsers ( domain, "DLS_MONITOR", tech_id, request );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
