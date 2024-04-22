/******************************************************************************************************************************/
/* archive.c       Gestion des archives dans l'API                                                                            */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                14.05.2022 10:17:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * archive.c
 * This file is part of Watchdog
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

/**************************************************** Chargement des prototypes ***********************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* ARCHIVE_add_one_enreg: Enregistre une nouvelle entrée dans la base d'archive du domain                                     */
/* Entrée: Le domaine, l'élement a archiver                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean ARCHIVE_add_one_enreg ( struct DOMAIN *domain, JsonNode *element )
  { gchar requete[512];

    if (!Json_has_member (element, "tech_id"))   return(FALSE);
    if (!Json_has_member (element, "acronyme"))  return(FALSE);
    if (!Json_has_member (element, "date_sec"))  return(FALSE);
    if (!Json_has_member (element, "date_usec")) return(FALSE);
    if (!Json_has_member (element, "valeur"))    return(FALSE);

    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO histo_bit (tech_id, acronyme, date_time, valeur) VALUES('%s', '%s', FROM_UNIXTIME(%d.%d),'%f')",
                tech_id, acronyme,
                Json_get_int    ( element, "date_sec" ),
                Json_get_int    ( element, "date_usec" ),
                Json_get_double ( element, "valeur" ) );

    if ( DB_Arch_Write ( domain, requete ) == TRUE ) return(TRUE);                             /* Execution de la requete SQL */

    if (g_str_has_prefix ( domain->mysql_last_error, "Duplicate entry")) return(TRUE);
                                     /* Si erreur, c'est peut etre parce que la table n'existe pas, on tente donc de la créer */
    return(FALSE);
  }
/******************************************************************************************************************************/
/* RUN_ARCHIVE_request_post: Repond aux requests ARCHIVE depuis les agents                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void RUN_ARCHIVE_SAVE_request_post ( struct DOMAIN *domain, gchar *path, gchar *agent_uuid, SoupServerMessage *msg, JsonNode *request )
  { gint retour = TRUE;
    if (Http_fail_if_has_not ( domain, path, msg, request, "archives")) return;

    GList *Archives = json_array_get_elements ( Json_get_array ( request, "archives" ) );
    GList *archives = Archives;
    gint nbr_enreg  = 0;
    gint top = Global.Top;
    while(archives && retour)
     { JsonNode *element = archives->data;
       retour &= ARCHIVE_add_one_enreg ( domain, element );
       nbr_enreg++;
       archives = g_list_next(archives);
     }
    g_list_free(Archives);
    if (retour) Info_new ( __func__, LOG_DEBUG, domain, "%04d enregistrements sauvegardés en %06.1fs", nbr_enreg, (Global.Top-top)/10.0 );
           else Info_new ( __func__, LOG_ERR,   domain, "%04d enregistrements non sauvegardés", nbr_enreg );

    JsonNode *RootNode = Http_json_node_create(msg);
    if (!RootNode) return;
    Json_node_add_int ( RootNode, "nbr_archives_saved", nbr_enreg );

    Http_Send_json_response ( msg, retour, domain->mysql_last_error, RootNode );
  }
/******************************************************************************************************************************/
/* ARCHIVE_DELETE_request: Supprime une table d'archivage                                                                     */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_DELETE_request ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "tech_id")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "acronyme")) return;

    gchar *tech_id  = Normaliser_chaine ( Json_get_string ( request, "tech_id" ) );
    gchar *acronyme = Normaliser_chaine ( Json_get_string ( request, "acronyme" ) );
    gboolean retour = DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                                         "requete=\"DELETE FROM histo_bit WHERE tech_id='%s' AND acronyme='%s'\"", tech_id, acronyme );
    g_free( tech_id );
    g_free( acronyme );

    ARCHIVE_Daily_update( Json_get_string ( domain->config, "domain_uuid" ), domain, NULL );
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

    if (Http_fail_if_has_not ( domain, path, msg, request, "archive_retention")) return;

    gint archive_retention = Json_get_int ( request, "archive_retention" );

    gboolean retour = DB_Write ( DOMAIN_tree_get("master"),
                                "UPDATE domains SET archive_retention='%d' "
                                "WHERE domain_uuid='%s'",
                                archive_retention, Json_get_string ( domain->config, "domain_uuid" ) );
    if (!retour) { Http_Send_json_response ( msg, retour, domain->mysql_last_error, NULL ); return; }

    Json_node_add_int ( domain->config, "archive_retention", archive_retention );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Domain Archive updated", NULL );
  }
/******************************************************************************************************************************/
/* ARCHIVE_STATUS_request_get: Renvoi la status des tables d'archivages                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_STATUS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;
    Json_node_add_int    ( RootNode, "archive_retention", Json_get_int ( domain->config, "archive_retention" ) );

    DB_Arch_Read ( domain, RootNode, NULL,
                   "SELECT SUM(table_rows) AS nbr_all_archives, ROUND(SUM((DATA_LENGTH + INDEX_LENGTH)) / 1024 / 1024) AS database_size "
                   "FROM information_schema.tables WHERE table_schema='%s' "
                   "AND table_name = 'histo_bit'", Json_get_string ( domain->config, "domain_uuid" ) );

    DB_Arch_Read ( domain, RootNode, "tables", "SELECT * FROM status" );

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

    gint days = Json_get_int    ( domain->config, "archive_retention" );
    Info_new( __func__, LOG_NOTICE, domain, "Starting ARCHIVE_Daily_update with days=%d", days );

    DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                       "requete=\"DELETE FROM histo_bit WHERE date_time < NOW() - INTERVAL %d DAY\"", days );
    DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete=\"DELETE FROM status\"" );
    DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, "
                       "requete=\"INSERT INTO status SELECT tech_id, acronyme, count(tech_id), MAX(date_time) "
                                 "FROM histo_bit GROUP BY tech_id, acronyme\"" );

    return(FALSE); /* False = on continue */
  }
/******************************************************************************************************************************/
/* ARCHIVE_GET_request_post: Repond a la requete de calcul des archives                                                       */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void ARCHIVE_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 6 )) return;
    Http_print_request ( domain, token, path );

    if (Http_fail_if_has_not ( domain, path, msg, request, "period")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "courbes")) return;

/************************************************ Préparation du buffer JSON **************************************************/
    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gchar *requete = NULL, chaine[512], *interval, nom_courbe[12];
    gint nbr;

    gchar *period = Normaliser_chaine ( Json_get_string ( request, "period" ) );
    gint periode  = 450;
    interval = " ";
         if (!strcasecmp(period, "HOUR"))  { periode = 150;   interval = " WHERE date_time>=NOW() - INTERVAL 4 HOUR"; }
    else if (!strcasecmp(period, "DAY"))   { periode = 450;   interval = " WHERE date_time>=NOW() - INTERVAL 2 DAY"; }
    else if (!strcasecmp(period, "WEEK"))  { periode = 3600;  interval = " WHERE date_time>=NOW() - INTERVAL 2 WEEK"; }
    else if (!strcasecmp(period, "MONTH")) { periode = 43200; interval = " WHERE date_time>=NOW() - INTERVAL 9 WEEK"; }
    else if (!strcasecmp(period, "YEAR"))  { periode = 86400; interval = " WHERE date_time>=NOW() - INTERVAL 13 MONTH"; }
    g_free(period);

    gint taille_requete = 32;
    requete = g_try_malloc0(taille_requete);
    if (!requete) { Http_Send_json_response ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error", RootNode ); return; }

    g_snprintf( requete, taille_requete, "SELECT * FROM ");

    gint nbr_courbe = json_array_get_length ( Json_get_array ( request, "courbes" ) );
    for (nbr=0; nbr<nbr_courbe; nbr++)
     { g_snprintf( nom_courbe, sizeof(nom_courbe), "courbe%d", nbr+1 );

       JsonNode *courbe = json_array_get_element ( Json_get_array ( request, "courbes" ), nbr );
       gchar *tech_id  = Normaliser_chaine ( Json_get_string ( courbe, "tech_id" ) );
       gchar *acronyme = Normaliser_chaine ( Json_get_string ( courbe, "acronyme" ) );

       g_snprintf( chaine, sizeof(chaine),
                  "%s "
                  "(SELECT FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV %d)*%d) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne%d "
                  " FROM histo_bit %s WHERE tech_id='%s' AND acronyme='%s' GROUP BY date ORDER BY date) AS %s "
                  "%s ",
                  (nbr!=0 ? "INNER JOIN" : ""), periode, periode, nbr+1, interval, tech_id, acronyme, nom_courbe,
                  (nbr!=0 ? "USING(date)" : "") );

       taille_requete += strlen(chaine)+1;
       requete = g_try_realloc ( requete, taille_requete );
       if (requete) g_strlcat ( requete, chaine, taille_requete );

       JsonNode *json_courbe = Json_node_add_objet ( RootNode, nom_courbe );
       DB_Read ( domain, json_courbe, NULL, "SELECT * FROM dictionnaire WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme );

       g_free(tech_id);
       g_free(acronyme);
     }

    DB_Arch_Read ( domain, RootNode, "valeurs", requete );
    g_free(requete);

    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
