/******************************************************************************************************************************/
/* archive.c       Gestion des archives dans l'API                                                                            */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                14.05.2022 10:17:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * archive.c
 * This file is part of Watchdog
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

/**************************************************** Chargement des prototypes ***********************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* ARCHIVE_creer_table: Ajoute une table dans la database d'archive du domaine                                                */
/* Entrée: le domain, l'element a sauvegrder                                                                                  */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 static gboolean ARCHIVE_creer_table ( struct DOMAIN *domain, JsonNode *element )
  { return ( DB_Arch_Write ( domain, "CREATE TABLE `histo_bit_%s_%s`("
                             "`date_time` datetime(2) DEFAULT NULL,"
                             "`valeur` float NOT NULL DEFAULT '0',"
                             "UNIQUE `index_unique` (`date_time`, `valeur`),"
                             "KEY `index_date` (`date_time`)"
                             ") ENGINE=ARIA DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                             "  PARTITION BY LINEAR KEY (date_time) PARTITIONS 12;",
                             Json_get_string ( element, "tech_id" ), Json_get_string ( element, "acronyme" ) ) );
  }
/******************************************************************************************************************************/
/* ARCHIVE_add_one_enreg: Enregistre une nouvelle entrée dans la base d'archive du domain                                     */
/* Entrée: Le domaine, l'élement a archiver                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void ARCHIVE_add_one_enreg ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    gchar requete[512];

    if (!Json_has_member (element, "tech_id"))   return;
    if (!Json_has_member (element, "acronyme"))  return;
    if (!Json_has_member (element, "date_sec"))  return;
    if (!Json_has_member (element, "date_usec")) return;
    if (!Json_has_member (element, "valeur"))    return;

    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO histo_bit_%s_%s(date_time,valeur) VALUES (FROM_UNIXTIME(%d.%d),'%f')",
                tech_id, acronyme,
                Json_get_int    ( element, "date_sec" ),
                Json_get_int    ( element, "date_usec" ),
                Json_get_double ( element, "valeur" ) );

    if ( DB_Arch_Write ( domain, requete ) == FALSE )                                          /* Execution de la requete SQL */
     {                               /* Si erreur, c'est peut etre parce que la table n'existe pas, on tente donc de la créer */
       if ( ARCHIVE_creer_table ( domain, element ) == FALSE)                                  /* Execution de la requete SQL */
        { Info_new( __func__, LOG_ERR, domain, "Creation de la table histo_%s_%s FAILED", tech_id, acronyme );
          return;
        }
       Info_new( __func__, LOG_NOTICE, domain, "Creation de la table histo_%s_%s avant Insert", tech_id, acronyme );

       if ( DB_Arch_Write ( domain, requete ) == FALSE )               /* Une fois la table créé, on peut y stocker l'archive */
        { Info_new( __func__, LOG_ERR, domain, "Ajout (2ième essai) dans la table histo_%s_%s FAILED", tech_id, acronyme );
          return;
        }
     }
  }
/******************************************************************************************************************************/
/* RUN_ARCHIVE_request_post: Repond aux requests ARCHIVE depuis les agents                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_ARCHIVE_request_post ( struct DOMAIN *domain, gchar *agent_uuid, gchar *api_tag, SoupMessage *msg, JsonNode *request )
  { if (!DB_Arch_Connected ( domain ))
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Database backend is not connected", NULL ); return; }

    if (Http_fail_if_has_not ( domain, "/run/archive", msg, request, "archives")) return;

    Json_node_foreach_array_element ( request, "archives", ARCHIVE_add_one_enreg, domain );
    gint nbr_enreg = json_array_get_length ( Json_get_array ( request, "archives" ) );
    Info_new ( __func__, LOG_INFO, domain, "%05d enregistrements sauvegardés", nbr_enreg );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Arch_Update_SQL_Partitions: Appelé une fois par jour pour faire des opérations de menage dans les tables d'archivages      */
/* Entrée: néant                                                                                                              */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 void Arch_Update_SQL_Partitions_thread ( void )
  { gchar requete[512];
	   GSList *Liste_tables;
    struct DB *db;
    prctl(PR_SET_NAME, "W-ArchSQL", 0, 0, 0 );
    Liste_tables = NULL;

    db = Init_ArchDB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_arch, LOG_ERR,
                "%s: Unable to open database %s", __func__, Partage->com_arch.archdb_database );
       return;
     }

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Starting Update SQL Partition on %s with days=%d", __func__,
              Partage->com_arch.archdb_database, Partage->com_arch.retention );
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT table_name FROM information_schema.tables WHERE table_schema='%s' "
                "AND table_name like 'histo_bit_%%'", Partage->com_arch.archdb_database );
    if (Lancer_requete_SQL ( db, requete )==FALSE)                                             /* Execution de la requete SQL */
     { Libere_DB_SQL(&db);
	      Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: Searching table names failed", __func__ );
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { Liste_tables = g_slist_prepend ( Liste_tables, strdup(db->row[0]) ); }
    Libere_DB_SQL(&db);

    db = Init_ArchDB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: Unable to open database %s for deleting", __func__,
                 Partage->com_arch.archdb_database );
     }

    while (db && Liste_tables && Partage->com_arch.Thread_run == TRUE)
     { gchar *table;
       gint top;
	      table = Liste_tables->data;
	      Liste_tables = g_slist_remove ( Liste_tables, table );
       Info_new( Config.log, Config.log_arch, LOG_DEBUG, "%s: Starting Update SQL Partition table %s", __func__, table );
	      g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "DELETE FROM %s WHERE date_time < NOW() - INTERVAL %d DAY", table, Partage->com_arch.retention );
       top = Partage->top;
       if (Lancer_requete_SQL ( db, requete )==FALSE)                                          /* Execution de la requete SQL */
        { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: Unable to delete from table '%s'", __func__, table );
        }
       Info_new( Config.log, Config.log_arch, LOG_NOTICE,
                "%s: Update SQL Partition table %s OK in %05.1fs", __func__, table, (Partage->top-top)/10.0 );
       g_free(table);
     }

    g_slist_foreach( Liste_tables, (GFunc)g_free, NULL );                            /* Vidage de la liste si arret prematuré */
    g_slist_free( Liste_tables );

    Libere_DB_SQL(&db);
    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Update SQL Partition end", __func__ );
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

