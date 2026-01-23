/******************************************************************************************************************************/
/* archive.c       Gestion des archives dans l'API                                                                            */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                                                14.05.2022 10:17:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * archive.c
 * This file is part of Watchdog
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

/**************************************************** Chargement des prototypes ***********************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* ARCHIVE_add_one_enreg: Enregistre une nouvelle entrée dans la base d'archive du domain                                     */
/* Entrée: Le domaine, l'élement a archiver                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean ARCHIVE_Handle_one ( struct DOMAIN *domain, JsonNode *element )
  { if (!Json_has_member (element, "tech_id"))   return(FALSE);
    if (!Json_has_member (element, "acronyme"))  return(FALSE);
    if (!Json_has_member (element, "date_sec"))  return(FALSE);
    if (!Json_has_member (element, "date_usec")) return(FALSE);
    if (!Json_has_member (element, "valeur"))    return(FALSE);

    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );

     /* On met la requete en attente dans la table cleanup pour éviter les délais d'insert en cas de sauvegardes des archives */
    DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
               "requete='INSERT INTO histo_bit (tech_id, acronyme, date_time, valeur) "
               "         VALUES(\"%s\", \"%s\", FROM_UNIXTIME(%d.%d),\"%f\") ON DUPLICATE KEY UPDATE valeur=VALUES(valeur)'",
               tech_id, acronyme,
               Json_get_int    ( element, "date_sec" ),
               Json_get_int    ( element, "date_usec" ),
               Json_get_double ( element, "valeur" ) );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* ARCHIVE_DELETE_request: Supprime une table d'archivage                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tablename")) return;
    gchar *tablename_src = Json_get_string ( request, "tablename" );
    if (!g_str_has_prefix ( tablename_src, "histo_bit_" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "tablename is wrong", NULL ); return; }

    gchar *tablename = Normaliser_chaine ( tablename_src );
    gboolean retour = DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete=\"DROP TABLE `%s`\"", tablename );
    g_free( tablename );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Archive deleted", NULL );
  }
/******************************************************************************************************************************/
/* ARCHIVE_SET_request_post: Change la configuration de l'archivage                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "archive_hot_retention")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archive_cold_retention")) return;

    gint archive_hot_retention  = Json_get_int ( request, "archive_hot_retention" );
    gint archive_cold_retention = Json_get_int ( request, "archive_cold_retention" );

    gboolean retour = DB_Write ( DOMAIN_tree_get("master"),
                                "UPDATE domains SET archive_hot_retention='%d', archive_cold_retention='%d' "
                                "WHERE domain_uuid='%s'",
                                archive_hot_retention, archive_cold_retention, Json_get_string ( domain->config, "domain_uuid" ) );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_int ( domain->config, "archive_hot_retention",  archive_hot_retention );
    Json_node_add_int ( domain->config, "archive_cold_retention", archive_cold_retention );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Domain Archive updated", NULL );
  }
/******************************************************************************************************************************/
/* ARCHIVE_REBUILD_request_post: Change la configuration de l'archivage                                                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_REBUILD_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "partname")) return;
    gchar *partname_src = Json_get_string ( request, "partname" );
    if (!g_str_has_prefix ( partname_src, "p_" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "partname is wrong", NULL ); return; }

    gchar *partname = Normaliser_chaine ( partname_src );
    gboolean retour = DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                                         "requete=\"ALTER TABLE histo_bit REBUILD PARTITION %s;\"", partname );
    g_free( partname );

    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Archive deleted", NULL );
  }
/******************************************************************************************************************************/
/* ARCHIVE_STATUS_HOT_request_get: Renvoi la status des tables d'archivages                                                   */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_STATUS_HOT_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    Json_node_add_int    ( RootNode, "archive_hot_retention",  Json_get_int ( domain->config, "archive_hot_retention" ) );
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    DB_Arch_Read ( domain, RootNode, NULL,
                   "SELECT SUM(table_rows) AS nbr_hot_archives, "
                   "ROUND(SUM((DATA_LENGTH + INDEX_LENGTH)) / 1024 / 1024) AS size_hot_archives "
                   "FROM information_schema.tables WHERE table_schema='%s' AND table_name = 'histo_bit'", domain_uuid );

    DB_Arch_Read ( domain, RootNode, "partitions",
                   "SELECT PARTITION_NAME AS partname, TABLE_ROWS AS nbr_archives, "
                   "ROUND((DATA_LENGTH + INDEX_LENGTH) / 1024 / 1024, 2) AS size, "
                   "ROUND((DATA_FREE/(DATA_LENGTH+INDEX_LENGTH))*100, 2) AS fragmentation "
                   "FROM information_schema.partitions WHERE TABLE_SCHEMA='%s' AND TABLE_NAME = 'histo_bit' ORDER BY partname", domain_uuid );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* ARCHIVE_STATUS_COLD_request_get: Renvoi la status des tables d'archivages                                                  */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_STATUS_COLD_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    Json_node_add_int    ( RootNode, "archive_cold_retention", Json_get_int ( domain->config, "archive_cold_retention" ) );
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

    DB_Arch_Read ( domain, RootNode, NULL,
                   "SELECT SUM(table_rows) AS nbr_cold_archives, "
                   "ROUND(SUM((DATA_LENGTH + INDEX_LENGTH)) / 1024 / 1024, 2) AS size_cold_archives "
                   "FROM information_schema.tables WHERE table_schema='%s' AND table_name LIKE 'histo_bit_%%'", domain_uuid );
    if (!Json_has_member ( RootNode, "nbr_cold_archives"))  Json_node_add_int ( RootNode, "nbr_cold_archives", 0 );
    if (!Json_has_member ( RootNode, "size_cold_archives")) Json_node_add_int ( RootNode, "size_cold_archives", 0 );

    DB_Arch_Read ( domain, RootNode, "tables",
                   "SELECT TABLE_NAME AS tablename, TABLE_ROWS AS nbr_archives, ROUND((DATA_LENGTH + INDEX_LENGTH) / 1024 / 1024, 2) AS size "
                   "FROM information_schema.tables where TABLE_SCHEMA='%s' AND TABLE_NAME LIKE 'histo_bit_%%' ORDER BY tablename", domain_uuid );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* ARCHIVE_Daily_update: Lance le menage (pthread) dans les archives du domaine en parametre issu du g_tree                   */
/* Entrée: le gtree                                                                                                           */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean ARCHIVE_Daily_update ( gpointer key, gpointer value, gpointer data )
  { struct DOMAIN *domain = value;

    if(!strcasecmp ( key, "master" )) return(FALSE);                                    /* Pas d'archive sur le domain master */
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

    gint hot_retention  = Json_get_int ( domain->config, "archive_hot_retention" );
    if (hot_retention<1) hot_retention = 1;
    gint cold_retention = Json_get_int ( domain->config, "archive_cold_retention" );
    Info_new( __func__, LOG_NOTICE, domain, "Starting with hot=%d months, cold=%d years", hot_retention, cold_retention );

/*------------------------------------- Création de la nouvelle partition du mois --------------------------------------------*/
    if (Global.Top_localtime.tm_mday == 1)                                                         /* Si premier jour du mois */
     { struct tm prev;
       Get_previous_time ( &prev, 1 );
       gint now_year = Global.Top_localtime.tm_year + 1900;
       gint now_mon  = Global.Top_localtime.tm_mon;
       Info_new( __func__, LOG_NOTICE, domain, "First Day of month, create partition 'p_%d%02d'", prev.tm_year+1900, prev.tm_mon+1 );
       DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                          "requete=\"ALTER TABLE histo_bit REORGANIZE PARTITION p_new INTO ( "
                          "PARTITION p_%d%02d VALUES LESS THAN (TO_DAYS('%d-%02d-01')), "
                          "PARTITION p_new    VALUES LESS THAN MAXVALUE )\"",
                          prev.tm_year+1900, prev.tm_mon+1, now_year, now_mon+1 );
     }
/*------------------------------------------- Création de la partition cold --------------------------------------------------*/
    if (Global.Top_localtime.tm_mday == 1 && cold_retention)                                       /* Si premier jour du mois */
     { gchar src_partname[16], dst_tablename[16];
       struct tm oldest;
       Get_previous_time ( &oldest, hot_retention+1 );
       JsonNode *RootNode = Json_node_create ();
       if (!RootNode)
        { Info_new( __func__, LOG_INFO, domain, "Memory Error when deleting old cold tables" ); }
       else
        { DB_Arch_Read ( domain, RootNode, "partitions",                              /* Recherche des partitions a supprimer */
                         "SELECT CAST(SUBSTRING(PARTITION_NAME, 3, 4) AS UNSIGNED) AS annee, "
                         "       CAST(SUBSTRING(PARTITION_NAME, 7, 2) AS UNSIGNED) AS mois "
                         "FROM INFORMATION_SCHEMA.PARTITIONS "
                         "WHERE TABLE_SCHEMA='%s' AND TABLE_NAME = 'histo_bit' "
                         "AND CAST(SUBSTRING(PARTITION_NAME, 3) AS UNSIGNED) <= '%d%02d' "
                         "AND PARTITION_NAME != 'p_new' ",
                         domain_uuid, oldest.tm_year+1900, oldest.tm_mon+1 );

          Info_new( __func__, LOG_NOTICE, domain, "Move '%d' partitions to cold table", Json_get_int ( RootNode, "nbr_partitions" ) );
          GList *Parts = json_array_get_elements ( Json_get_array ( RootNode, "partitions" ) );
          GList *parts = Parts;
          while(parts)
           { JsonNode *element = parts->data;
             gint annee = Json_get_int ( element, "annee" );
             gint mois  = Json_get_int ( element, "mois" );
             g_snprintf ( src_partname,  sizeof(src_partname),  "p_%d%02d", annee, mois );
             g_snprintf ( dst_tablename, sizeof(dst_tablename), "histo_bit_%d", oldest.tm_year+1900 );

             Info_new( __func__, LOG_NOTICE, domain, "Create cold table '%s'", dst_tablename );
             DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete=\"CREATE TABLE IF NOT EXISTS `%s` LIKE `histo_bit`\"", dst_tablename );

             Info_new( __func__, LOG_NOTICE, domain, "Hot to Cold: move partition '%s' to table '%s'", src_partname, dst_tablename );
             DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                                "requete=\"INSERT INTO `%s` SELECT * FROM histo_bit PARTITION(%s)\"", dst_tablename, src_partname );
             Info_new( __func__, LOG_NOTICE, domain, "Delete old partition '%s'", src_partname );
             DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                                "requete=\"ALTER TABLE `histo_bit` DROP PARTITION %s;\"", src_partname );
             parts = g_list_next(parts);
           }
          g_list_free(Parts);
          json_node_unref ( RootNode );
       }
     }
/*------------------------------------------- Delete old cold table ----------------------------------------------------------*/
    if (Global.Top_localtime.tm_mday == 1)                                                         /* Si premier jour du mois */
     { struct tm prev;
       Get_previous_time ( &prev, hot_retention + cold_retention*12 );                           /* Conversion: mois -> année */
       JsonNode *RootNode = Json_node_create ();
       if (!RootNode)
        { Info_new( __func__, LOG_INFO, domain, "Memory Error when deleting old cold tables" ); }
       else
        { DB_Arch_Read ( domain, RootNode, "tables",                                      /* Recherche des tables a supprimer */
                         "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES "
                         "WHERE TABLE_SCHEMA='%s' AND TABLE_NAME LIKE 'histo_bit_%%' "
                         "AND CAST(SUBSTRING(TABLE_NAME, 11) AS UNSIGNED) < '%d'", domain_uuid, prev.tm_year+1900 );

          Info_new( __func__, LOG_NOTICE, domain, "Delete '%d' old cold tables", Json_get_int ( RootNode, "nbr_tables" ) );

          GList *Tables = json_array_get_elements ( Json_get_array ( RootNode, "tables" ) );
          GList *tables = Tables;
          while(tables)
           { JsonNode *element = tables->data;
             gchar *tablename = Json_get_string ( element, "TABLE_NAME" );
             Info_new( __func__, LOG_NOTICE, domain, "Delete old cold table '%s'", tablename );
             DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete=\"DROP TABLE `%s`\"", tablename );
             tables = g_list_next(tables);
           }
          g_list_free(Tables);
          json_node_unref ( RootNode );
       }
     }
/*---------------------------------------------- Defragmentation des partitions ----------------------------------------------*/
    JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new( __func__, LOG_INFO, domain, "Memory Error when defragmenting tables" ); }
    else
     { DB_Arch_Read ( domain, RootNode, NULL,
                      "SELECT PARTITION_NAME AS part_name, (DATA_FREE/(DATA_LENGTH+INDEX_LENGTH))*100 AS pct_unused "
                      "FROM INFORMATION_SCHEMA.PARTITIONS "
                      "WHERE TABLE_SCHEMA = '%s' "
                      "AND   TABLE_NAME = 'histo_bit' "
                      "AND   DATA_LENGTH + INDEX_LENGTH >= 100000000 "               /* Uniquement pour les tables de + 100Mb */
                      "ORDER BY pct_unused DESC LIMIT 1",                                                 /* Une par jour max */
                      domain_uuid
                    );

       gchar *partition  = Json_get_string ( RootNode, "part_name" );
       gint   pct_unused = Json_get_double ( RootNode, "pct_unused" );
       if (pct_unused > 5)                                                       /* Pour toute table fragmentée de plus de 5% */
        { Info_new( __func__, LOG_NOTICE, domain, "Rebuilding partition '%s' with pct_unused=%d%%", partition, pct_unused );
          DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                             "requete=\"ALTER TABLE histo_bit REBUILD PARTITION %s;\"", partition );
        }
       json_node_unref ( RootNode );
     }

    return(FALSE); /* False = on continue */
  }
/******************************************************************************************************************************/
/* ARCHIVE_GET_request_post: Repond a la requete de calcul des archives                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 0 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "period"))  return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "courbes")) return;

/************************************************ Check Periode ***************************************************************/
    gchar *period_src = Json_get_string ( request, "period" );
    gchar *group_by = NULL, *fenetre = NULL;

    if (!strcasecmp(period_src, "BY_MINUTE"))
     { group_by = "date_time_year, date_time_month, date_time_day, date_time_hour, date_time_min";
       fenetre = "2 HOUR";
     }
    else if (!strcasecmp(period_src, "BY_HOUR"))
     { group_by = "date_time_year, date_time_month, date_time_day, date_time_hour";
       fenetre = "1 DAY";
     }
    else if (!strcasecmp(period_src, "BY_DAY"))
     { group_by = "date_time_year, date_time_month, date_time_day";
       fenetre = "2 MONTH";
     }
    else if (!strcasecmp(period_src, "BY_WEEK"))
     { group_by = "date_time_year, date_time_week";
       fenetre = "4 MONTH";
     }
    else if (!strcasecmp(period_src, "BY_MONTH"))
     { group_by = "date_time_year, date_time_month";
       fenetre = "13 MONTH";
     }
    else if (!strcasecmp(period_src, "BY_YEAR"))
     { group_by = "date_time_year";
       fenetre = "2 YEAR";
     }
    else
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Period Error", NULL ); return; }

    gint nbr_courbe = json_array_get_length ( Json_get_array ( request, "courbes" ) );
    if (! (1 <= nbr_courbe && nbr_courbe<=8) )
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Wrong nbr_courbe", NULL ); return; }

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode)
     { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", NULL ); return; }

    soup_server_message_pause (  msg );

    gint taille_requete = 1;
    gchar *requete      = g_try_malloc0(taille_requete);
    gchar chaine[512], nom_courbe[12];

/*-------------------------------------------------- base Select -------------------------------------------------------------*/
    g_snprintf ( chaine, sizeof(chaine), "SELECT " );
    taille_requete += strlen(chaine)+1;
    requete = g_try_realloc ( requete, taille_requete );
    if (requete) g_strlcat ( requete, chaine, taille_requete );

    if (!strcasecmp(period_src, "BY_MINUTE"))
     { g_snprintf ( chaine, sizeof(chaine), "CONCAT ( date_time_year, '-', LPAD(date_time_month, 2, '0'), '-', "
                                            "         LPAD(date_time_day, 2, '0'), ' ', "
                                            "         LPAD(date_time_hour, 2, '0'), ':', LPAD(date_time_min, 2, '0'), ':00' )" );
     }
    else if (!strcasecmp(period_src, "BY_HOUR"))
     { g_snprintf ( chaine, sizeof(chaine), "CONCAT ( date_time_year, '-', LPAD(date_time_month, 2, '0'), '-', "
                                            "         LPAD(date_time_day, 2, '0'), ' ', "
                                            "         LPAD(date_time_hour, 2, '0'), ':00:00' )" );
     }
    else if (!strcasecmp(period_src, "BY_DAY"))
     { g_snprintf ( chaine, sizeof(chaine), "CONCAT ( date_time_year, '-', LPAD(date_time_month, 2, '0'), '-', "
                                            "         LPAD(date_time_day, 2, '0') )" );
     }
    else if (!strcasecmp(period_src, "BY_WEEK"))
     { g_snprintf ( chaine, sizeof(chaine), "CONCAT ( date_time_year, '/', LPAD(date_time_week, 2, '0') )" );
     }
    else if (!strcasecmp(period_src, "BY_MONTH"))
     { g_snprintf ( chaine, sizeof(chaine), "CONCAT ( date_time_year, '-', LPAD(date_time_month, 2, '0'), '-01' )" );
     }
    else if (!strcasecmp(period_src, "BY_YEAR"))
     { g_snprintf ( chaine, sizeof(chaine), "date_time_year" ); }

    taille_requete += strlen(chaine)+1;
    requete = g_try_realloc ( requete, taille_requete );
    if (requete) g_strlcat ( requete, chaine, taille_requete );

    g_snprintf ( chaine, sizeof(chaine), " AS date, COALESCE(valeur1, 0) AS valeur1" );
    taille_requete += strlen(chaine)+1;
    requete = g_try_realloc ( requete, taille_requete );
    if (requete) g_strlcat ( requete, chaine, taille_requete );
/*------------------------------------------------- Courbe Select -------------------------------------------------------------*/
    gint nbr;
    for (nbr=2; nbr<=nbr_courbe; nbr++)
     { g_snprintf ( chaine, sizeof(chaine), ", COALESCE(valeur%d, 0) AS valeur%d", nbr, nbr );
       taille_requete += strlen(chaine)+1;
       requete = g_try_realloc ( requete, taille_requete );
       if (requete) g_strlcat ( requete, chaine, taille_requete );
     }

/*------------------------------------------------- Courbe Select -------------------------------------------------------------*/
    gboolean first = TRUE;
    for (nbr=1; nbr<=nbr_courbe; nbr++)
     { g_snprintf( nom_courbe, sizeof(nom_courbe), "courbe%d", nbr );
       JsonNode *json_courbe = Json_node_add_objet ( RootNode, nom_courbe );

       JsonNode *courbe = json_array_get_element ( Json_get_array ( request, "courbes" ), nbr-1 );
       gchar *tech_id  = Normaliser_chaine ( Json_get_string ( courbe, "tech_id" ) );
       gchar *acronyme = Normaliser_chaine ( Json_get_string ( courbe, "acronyme" ) );
       gchar *methode_src = Json_get_string ( courbe, "methode" );
       gchar *methode = "AVG";
       if(methode_src)
        {      if (!strcasecmp(methode_src, "MIN")) { methode="MIN"; }
          else if (!strcasecmp(methode_src, "MAX")) { methode="MAX"; }
          else if (!strcasecmp(methode_src, "AVG")) { methode="AVG"; }
          else if (!strcasecmp(methode_src, "SUM")) { methode="SUM"; }
        }
       DB_Read ( domain, json_courbe, NULL, "SELECT classe, tech_id, acronyme, libelle, unite "
                                            "FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme );

       g_snprintf ( chaine, sizeof(chaine),
                    " %s ( SELECT %s, %s(valeur) AS valeur%d FROM `histo_bit` "
                    "      WHERE tech_id = '%s' AND acronyme = '%s' AND date_time > NOW() - INTERVAL %s "
                    "      GROUP BY %s "
                    "    ) AS courbe%d ",
                    (first ? "FROM" : "INNER JOIN"), group_by, methode, nbr, tech_id, acronyme, fenetre, group_by, nbr );
       taille_requete += strlen(chaine)+1;
       requete = g_try_realloc ( requete, taille_requete );
       if (requete) g_strlcat ( requete, chaine, taille_requete );

       if (first == FALSE)
        { g_snprintf ( chaine, sizeof(chaine), "USING(%s) ", group_by );
          taille_requete += strlen(chaine)+1;
          requete = g_try_realloc ( requete, taille_requete );
          if (requete) g_strlcat ( requete, chaine, taille_requete );
        }

       first = FALSE;

       g_free(tech_id);
       g_free(acronyme);
     }

    g_snprintf ( chaine, sizeof(chaine), "ORDER BY %s", group_by );
    taille_requete += strlen(chaine)+1;
    requete = g_try_realloc ( requete, taille_requete );
    if (requete) g_strlcat ( requete, chaine, taille_requete );

    DB_Arch_Read ( domain, RootNode, "valeurs", requete );
    g_free(requete);

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
    soup_server_message_unpause ( msg );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
