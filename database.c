/******************************************************************************************************************************/
/* database.c          Gestion des connexions à la base de données                                                            */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * db.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

 #include <glib.h>
 #include <string.h>
 #include <locale.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

/**************************************************** Chargement des prototypes ***********************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */

/******************************************************************************************************************************/
/* Normaliser_chaine: Normalise les chaines ( remplace ' par \', " par "" )                                                   */
/* Entrées: un commentaire (gchar *)                                                                                          */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 gchar *Normaliser_chaine( gchar *pre_comment )
  { gchar *comment, *source, *cible;
    gunichar car;

    g_utf8_validate( pre_comment, -1, NULL );                                                           /* Validate la chaine */
    comment = g_try_malloc0( (2*g_utf8_strlen(pre_comment, -1))*6 + 1 );                  /* Au pire, ts les car sont doublés */
                                                                                                      /* *6 pour gerer l'utf8 */
    if (!comment)
     { Info_new( __func__, LOG_WARNING, "Normaliser_chaine: memory error %s", pre_comment );
       return(NULL);
     }
    source = pre_comment;
    cible  = comment;

    while( (car = g_utf8_get_char( source )) )
     { if ( car == '\'' )                                                                     /* Dédoublage de la simple cote */
        { g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
        }
       else if (car =='\\')                                                                        /* Dédoublage du backspace */
        { g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
        }
       else
        { g_utf8_strncpy( cible, source, 1 ); cible = g_utf8_next_char( cible );
        }
       source = g_utf8_next_char(source);
     }
    return(comment);
  }
/******************************************************************************************************************************/
/* DB_Connected: Renvoi TRUE si la connexion est établie                                                                      */
/* Entrée: le domain en question                                                                                              */
/******************************************************************************************************************************/
 gboolean DB_Connected( gchar *domain_uuid )
  { struct DOMAIN *domain = DOMAIN_tree_get ( domain_uuid );
    if (!domain || !domain->mysql) return(FALSE);
    return (!mysql_ping ( domain->mysql ));
  }
/******************************************************************************************************************************/
/* DB_Write: Envoie une requete en parametre au serveur de base de données                                                    */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Write( gchar *domain_uuid, gchar *format, ... )
  { gboolean retour = FALSE;
    va_list ap;

    struct DOMAIN *domain = DOMAIN_tree_get ( domain_uuid );
    if (! (domain && domain->mysql) )
     { Info_new( __func__, LOG_ERR, "%s: domain not found. Dropping.", domain_uuid ); return(FALSE); }
    MYSQL *mysql = domain->mysql;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *requete = g_try_malloc(taille+1);
    if (!requete)
     { Info_new( __func__, LOG_ERR, "%s: DB FAILED: Memory Error for '%s'", domain_uuid, requete );
       return(FALSE);
     }

    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );
    if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, "%s: DB FAILED: %s for '%s'", domain_uuid, (char *)mysql_error(mysql), requete );
       retour = FALSE;
     }
    else
     { Info_new( __func__, LOG_DEBUG, "%s: DB OK: '%s'", domain_uuid, requete );
      retour = TRUE;
     }
    g_free(requete);
    return(retour);
  }
/******************************************************************************************************************************/
/* DB_Connect: essai de connexion vers le DB dont les parametre sont dans config                                              */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Connect ( struct DOMAIN *domain )
  { if (!Json_has_member ( domain->config, "domain_uuid" ))
     { Info_new( __func__, LOG_ERR, "No domain_uuid selected, DBConnect failed" );
       return(FALSE);
     }

    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

    if (!Json_has_member ( domain->config, "db_hostname" ))
     { Json_node_add_string ( domain->config, "db_hostname", Json_get_string ( Global.config, "db_hostname" ) ); }

    if (!Json_has_member ( domain->config, "db_username" ))
     { Json_node_add_string ( domain->config, "db_username", domain_uuid ); }

    if (!Json_has_member ( domain->config, "db_database" ))
     { Json_node_add_string ( domain->config, "db_database", domain_uuid ); }

    if (!Json_has_member ( domain->config, "db_port" ))
     { Json_node_add_int ( domain->config, "db_port", 3306 ); }

    if (!Json_has_member ( domain->config, "db_password" ))
     { Info_new( __func__, LOG_ERR, "Connect parameter are missing. DBConnect failed." );
       return(FALSE);
     }

    gchar *db_hostname = Json_get_string ( domain->config, "db_hostname" );
    gchar *db_database = Json_get_string ( domain->config, "db_database" );
    gchar *db_username = Json_get_string ( domain->config, "db_username" );
    gchar *db_password = Json_get_string ( domain->config, "db_password" );
    gint   db_port     = Json_get_int    ( domain->config, "db_port" );

    domain->mysql = mysql_init(NULL);
    if (!domain->mysql)
     { Info_new( __func__, LOG_ERR, "Unable to init database for domain '%s'", domain_uuid );
       return(FALSE);
     }

    my_bool reconnect = 1;
    mysql_options( domain->mysql, MYSQL_OPT_RECONNECT, &reconnect );
    gint timeout = 10;
    mysql_options( domain->mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout );                     /* Timeout en cas de non reponse */
    mysql_options( domain->mysql, MYSQL_SET_CHARSET_NAME, (void *)"utf8" );

    if ( ! mysql_real_connect( domain->mysql, db_hostname, db_username, db_password, db_database, db_port, NULL, 0 ) )
     { Info_new( __func__, LOG_ERR, "Mysql_real_connect failed (%s) for domain '%s'", (char *) mysql_error(domain->mysql), domain_uuid );
       mysql_close( domain->mysql );
       domain->mysql = NULL;
       return (FALSE);
     }

    Info_new( __func__, LOG_INFO, "DB Connect OK for domain '%s' with %s@%s:%d on %s",
              domain_uuid, db_username, db_hostname, db_port, db_database );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* DB_Update: Met a jour le schema de database                                                                                */
/* Entrée: aucune. Tout se fait sur le domain 'master'                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Master_Update ( void )
  { DB_Write ( "master", "CREATE TABLE IF NOT EXISTS domains ("
                         "`domain_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                         "`domain_uuid` VARCHAR(37) UNIQUE NOT NULL,"
                         "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                         "`email` VARCHAR(256) NOT NULL,"
                         "`db_hostname` VARCHAR(64) NULL,"
                         "`db_database` VARCHAR(64) NULL,"
                         "`db_username` VARCHAR(64) NULL,"
                         "`db_password` VARCHAR(64) NULL,"
                         "`db_port` INT(11) NULL,"
                         "`db_arch_hostname` VARCHAR(64) NULL,"
                         "`db_arch_database` VARCHAR(64) NULL,"
                         "`db_arch_username` VARCHAR(64) NULL,"
                         "`db_arch_password` VARCHAR(64) NULL,"
                         "`db_arch_port` INT(11) NULL"
                         ") ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);

    /*SQL_Read ( NULL, RootNode, NULL, "SELECT version FROM database_version" ); */

    json_node_unref(RootNode);
    Info_new( __func__, LOG_INFO, "Master Schema Updated" );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Field_to_Json : Intéègre un MYSQL_FIELD dans une structure JSON                                                        */
/* Entrée: Le JsonNode et le MYSQL_FIELD                                                                                      */
/* Sortie: le jsonnode est mis a jour                                                                                         */
/******************************************************************************************************************************/
 static void SQL_Field_to_Json ( JsonNode *node, MYSQL_FIELD *field, gchar *chaine )
  { if ( field->type == MYSQL_TYPE_FLOAT || field->type==MYSQL_TYPE_DOUBLE )
     { if (chaine) Json_node_add_double( node, field->name, atof(chaine) );
              else Json_node_add_null  ( node, field->name );
     }
    else if ( field->type == MYSQL_TYPE_TINY )
     { if (chaine) Json_node_add_bool ( node, field->name, atoi(chaine) );
              else Json_node_add_null ( node, field->name );
     }
    else if ( IS_NUM(field->type) )
     { if (chaine) Json_node_add_int  ( node, field->name, atoi(chaine) );
              else Json_node_add_null ( node, field->name );
     }
    else
     { Json_node_add_string( node, field->name, chaine ); }
  }
/******************************************************************************************************************************/
/* SQL_Write_new: Envoie une requete en parametre au serveur de base de données                                               */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Read ( gchar *domain_uuid, JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    struct DOMAIN *domain = DOMAIN_tree_get ( domain_uuid );
    if (! (domain && domain->mysql) )
     { Info_new( __func__, LOG_ERR, "%s: domain not found. Dropping.", domain_uuid ); return(FALSE); }
    MYSQL *mysql = domain->mysql;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *requete = g_try_malloc(taille+1);
    if (!requete)
     { Info_new( __func__, LOG_ERR, "%s: DB FAILED: Memory Error for '%s'", domain_uuid, requete );
       return(FALSE);
     }

    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );

    if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, "%s: DB FAILED (%s) for '%s'", domain_uuid, (char *)mysql_error(mysql), requete );
       if (array_name)
        { gchar chaine[80];
          g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
          Json_node_add_int  ( RootNode, chaine, 0 );
          Json_node_add_array( RootNode, array_name );                            /* Ajoute un array vide en cas d'erreur SQL */
        }
       g_free(requete);
       return(FALSE);
     }
    else Info_new( __func__, LOG_DEBUG, "%s: DB OK for '%s'", domain_uuid, requete );

    g_free(requete);
    MYSQL_RES *result = mysql_store_result ( mysql );
    if ( ! result )
     { Info_new( __func__, LOG_WARNING, "%s: store_result failed (%s)", domain_uuid, (char *) mysql_error(mysql) );
       return(FALSE);
     }

    MYSQL_ROW row;
    if (array_name)
     { gchar chaine[80];
       g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
       Json_node_add_int ( RootNode, chaine, mysql_num_rows ( result ));
       JsonArray *array = Json_node_add_array( RootNode, array_name );
       while ( (row = mysql_fetch_row(result)) != NULL )
        { JsonNode *element = Json_node_create();
          for (gint cpt=0; cpt<mysql_num_fields(result); cpt++)
           { MYSQL_FIELD *field = mysql_fetch_field_direct(result, cpt);
             SQL_Field_to_Json ( element, field, row[cpt] );
           }
          Json_array_add_element ( array, element );
        }
     }
    else
     { while ( (row = mysql_fetch_row(result)) != NULL )
        { for (gint cpt=0; cpt<mysql_num_fields(result); cpt++)
           { MYSQL_FIELD *field = mysql_fetch_field_direct(result, cpt);
             SQL_Field_to_Json ( RootNode, field, row[cpt] );
           }
        }
     }
    mysql_free_result( result );
    return(TRUE);
  }

#ifdef bouh

/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Arch_to_json_node ( JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *chaine = g_try_malloc(taille+1);
    if (chaine)
     { va_start( ap, format );
       g_vsnprintf ( chaine, taille, format, ap );
       va_end ( ap );

       gboolean retour = SQL_Select_to_json_node_reel ( TRUE, RootNode, array_name, chaine );
       g_free(chaine);
       return(retour);
     }
    return(FALSE);
  }

/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 gboolean SQL_Arch_Write ( gchar *requete )
  { struct DB *db = Init_ArchDB_SQL ();
    if (!db)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: Init DB FAILED for '%s'", __func__, requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR, "%s: FAILED (%s) for '%s'", __func__, (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_db, LOG_DEBUG, "%s: DB OK for '%s'", __func__, requete );

    Libere_DB_SQL ( &db );
    return(TRUE);
  }
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
