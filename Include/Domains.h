/******************************************************************************************************************************/
/* Include/Domains.h        Déclaration structure internes des domaines                                                       */
/* Projet Abls-Habitat version 4.x       Gestion d'habitat                                                19.02.2022 20:58:23 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Domains.h
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


#ifndef _DOMAIN_H_
 #define _DOMAIN_H_

 struct DOMAIN                                                                                           /* Zone des domaines */
  { JsonNode *config;
    pthread_mutex_t synchro;
    MYSQL *mysql[DATABASE_POOL_SIZE];
    pthread_mutex_t mysql_mutex[DATABASE_POOL_SIZE];                                      /* Bit de synchronisation processus */
    MYSQL *mysql_arch[DATABASE_POOL_SIZE];
    pthread_mutex_t mysql_arch_mutex[DATABASE_POOL_SIZE];                                 /* Bit de synchronisation processus */
    gchar mysql_last_error[256];
    GTree *Visuels;
    gint Nbr_visuels;
    GSList *ws_agents;                                                               /* Liste des agents connectés au domaine */
    GSList *ws_clients;                                                          /* Les des clients web connectées au domaine */
    GTree *abonnements;
    pthread_mutex_t abonnements_synchro;
  };

/*************************************************** Définitions des prototypes ***********************************************/
 extern gint DOMAIN_Comparer_tree_clef_for_bit ( JsonNode *node1, JsonNode *node2, gpointer user_data );
 extern struct DOMAIN *DOMAIN_tree_get ( gchar *domain_uuid );
 extern void DOMAIN_Load ( JsonArray *array, guint index_, JsonNode *domaine_config, gpointer user_data );
 extern void DOMAIN_Load_all ( void );
 extern void DOMAIN_Unload_all( void );
 extern gboolean DOMAIN_Archiver_status ( gpointer key, gpointer value, gpointer data );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
