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
     { Info_new( __func__, LOG_WARNING, NULL, "Normaliser_chaine: memory error %s", pre_comment );
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
/* DB_Write: Envoie une requete en parametre au serveur de base de données                                                    */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Write( struct DOMAIN *domain, gchar *format, ... )
  { gboolean retour = FALSE;
    va_list ap;

    if (!domain )
     { Info_new( __func__, LOG_ERR, domain, "Domain not found. Dropping." ); return(FALSE); }

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *requete = g_try_malloc(taille+1);
    if (!requete)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Memory Error for '%s'", requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "Memory Error" );
       return(FALSE);
     }

    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );
    MYSQL *mysql = DB_Pool_take ( domain );
    if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: %s for '%s'", (char *)mysql_error(mysql), requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "%s", (char *)mysql_error(mysql) );
       retour = FALSE;
     }
    else
     { Info_new( __func__, LOG_DEBUG, domain, "DB OK: '%s'", requete );
       retour = TRUE;
     }
    DB_Pool_unlock ( domain, mysql );
    g_free(requete);
    return(retour);
  }
/******************************************************************************************************************************/
/* DB_Arch_Write: Envoie une requete en parametre au serveur d'archive                                                        */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Arch_Write( struct DOMAIN *domain, gchar *format, ... )
  { gboolean retour = FALSE;
    va_list ap;

    if (! (domain && domain->mysql_arch) )
     { Info_new( __func__, LOG_ERR, domain, "Domain not found or not connected. Dropping." ); return(FALSE); }
    MYSQL *mysql = domain->mysql_arch;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *requete = g_try_malloc(taille+1);
    if (!requete)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Memory Error for '%s'", requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "Memory Error" );
       return(FALSE);
     }

    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );
    pthread_mutex_lock ( &domain->mysql_arch_mutex );
    if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: %s for '%s'", (char *)mysql_error(mysql), requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "%s", (char *)mysql_error(mysql) );
       retour = FALSE;
     }
    else
     { Info_new( __func__, LOG_DEBUG, domain, "DB OK: '%s'", requete );
       retour = TRUE;
     }
    pthread_mutex_unlock ( &domain->mysql_arch_mutex );
    g_free(requete);
    return(retour);
  }
/******************************************************************************************************************************/
/* DB_Pool_take: Prend un token dans le pool de connexion base de données                                                     */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 MYSQL *DB_Pool_take ( struct DOMAIN *domain )
  { if (!domain) return(NULL);
    if (!domain->mysql[0])
     { Info_new( __func__, LOG_ERR, domain, "No pool available. Dropping." );
       return(NULL);
     }

encore:
    for (gint i=0; i<DATABASE_POOL_SIZE; i++)
     { if ( pthread_mutex_trylock ( &domain->mysql_mutex[i] ) == 0 ) return(domain->mysql[i]); }

    Info_new( __func__, LOG_ERR, domain, "All pool are busy. waiting before retry." );
    sleep(1);
    goto encore;
  }
/******************************************************************************************************************************/
/* DB_Pool_take: Prend un token dans le pool de connexion base de données                                                     */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 void DB_Pool_unlock ( struct DOMAIN *domain, MYSQL *mysql )
  { if (!domain) return;

    for (gint i=0; i<DATABASE_POOL_SIZE; i++)
     { if ( domain->mysql[i] == mysql ) pthread_mutex_unlock ( &domain->mysql_mutex[i] ); }
  }
/******************************************************************************************************************************/
/* DB_Pool_end: Ferme les connexions aux bases de données du domaine                                                          */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 void DB_Pool_end ( struct DOMAIN *domain )
  { gint i;
    for (i=0; i<DATABASE_POOL_SIZE; i++)
     { if (domain->mysql[i])
        { mysql_close ( domain->mysql[i] );
          pthread_mutex_destroy( &domain->mysql_mutex[i] );
        }
     }
    if (domain->mysql_arch)
     { mysql_close ( domain->mysql_arch );
       pthread_mutex_destroy( &domain->mysql_arch_mutex );
     }
  }
/******************************************************************************************************************************/
/* DB_Pool_init: Alloue un pool de connexion vers la base de données du domain en parametrz                                   */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Pool_init ( struct DOMAIN *domain )
  { if (!domain)
     { Info_new( __func__, LOG_ERR, NULL, "No domain selected, DBConnect failed" );
       return(FALSE);
     }

    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

    if (!Json_has_member ( domain->config, "db_password" ))
     { Info_new( __func__, LOG_ERR, domain, "db_password is missing. DBConnect failed." );
       return(FALSE);
     }

    gchar *db_hostname = Json_get_string ( Global.config, "db_hostname" );
    gchar *db_database = domain_uuid;
    gint   db_port     = Json_get_int    ( Global.config, "db_port" );
    gchar *db_username, *db_password;
    if (!strcasecmp ( domain_uuid, "master" ) )
     { db_username = "root";
       db_password = Json_get_string ( Global.config, "db_password" );
     }
    else
     { db_username = domain_uuid;
       db_password = Json_get_string ( domain->config, "db_password" );
     }

    gint i;
    for (i=0; i<DATABASE_POOL_SIZE; i++)
     { domain->mysql[i] = mysql_init(NULL);
       if (!domain->mysql[i])
        { Info_new( __func__, LOG_ERR, domain, "Unable to init domain database pool %d", i );
          break;
        }

       pthread_mutex_init( &domain->mysql_mutex[i], NULL );

       my_bool reconnect = 1;
       mysql_options( domain->mysql[i], MYSQL_OPT_RECONNECT, &reconnect );
       gint timeout = 10;
       mysql_options( domain->mysql[i], MYSQL_OPT_CONNECT_TIMEOUT, &timeout );               /* Timeout en cas de non reponse */
       mysql_options( domain->mysql[i], MYSQL_SET_CHARSET_NAME, (void *)"utf8" );

       if ( ! mysql_real_connect( domain->mysql[i], db_hostname, db_username, db_password, db_database, db_port, NULL, 0 ) )
        { Info_new( __func__, LOG_ERR, domain, "Mysql_real_connect failed to connect to %s@%s:%d/%s for pool %d -> %s",
                    db_username, db_hostname, db_port, db_database, i,
                    (char *) mysql_error(domain->mysql[i]) );
          mysql_close( domain->mysql[i] );
          domain->mysql[i] = NULL;
          break;
        }
     }

    if (!i)
     { Info_new( __func__, LOG_NOTICE, domain, "Cannot load any DBPool for %s@%s:%d on %s", db_username, db_hostname, db_port, db_database );
       return(FALSE);
     }
    Info_new( __func__, LOG_NOTICE, domain, "%d Pools OK with %s@%s:%d on %s", i+1, db_username, db_hostname, db_port, db_database );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* DB_Connect: essai de connexion vers le DB dont les parametre sont dans config                                              */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Arch_Connect ( struct DOMAIN *domain )
  { if (!domain)
     { Info_new( __func__, LOG_ERR, NULL, "No domain selected, DBConnect failed" );
       return(FALSE);
     }

    if (!Json_has_member ( domain->config, "db_password" ))
     { Info_new( __func__, LOG_ERR, domain, "db_password is missing. DBArchConnect failed." );
       return(FALSE);
     }

    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );

    gchar *db_hostname = Json_get_string ( Global.config, "db_arch_hostname" );
    gchar *db_database = domain_uuid;
    gint   db_port     = Json_get_int    ( Global.config, "db_arch_port" );
    gchar *db_username, *db_password;
    if (!strcasecmp ( domain_uuid, "master" ) )
     { db_username = "root";
       db_password = Json_get_string ( Global.config, "db_password" );
     }
    else
     { db_username = domain_uuid;
       db_password = Json_get_string ( domain->config, "db_password" );
     }

    domain->mysql_arch = mysql_init(NULL);
    if (!domain->mysql_arch)
     { Info_new( __func__, LOG_ERR, domain, "Unable to init Archive database" );
       return(FALSE);
     }
    pthread_mutex_init( &domain->mysql_arch_mutex, NULL );
    my_bool reconnect = 1;
    mysql_options( domain->mysql_arch, MYSQL_OPT_RECONNECT, &reconnect );
    gint timeout = 10;
    mysql_options( domain->mysql_arch, MYSQL_OPT_CONNECT_TIMEOUT, &timeout );                /* Timeout en cas de non reponse */
    mysql_options( domain->mysql_arch, MYSQL_SET_CHARSET_NAME, (void *)"utf8" );

    if ( ! mysql_real_connect( domain->mysql_arch, db_hostname, db_username, db_password, db_database, db_port, NULL, 0 ) )
     { Info_new( __func__, LOG_ERR, domain, "Mysql_real_connect failed to connect to %s@%s:%d/%s -> %s",
                db_username, db_hostname, db_port, db_database,
                (char *) mysql_error(domain->mysql_arch) );
       mysql_close( domain->mysql_arch );
       domain->mysql_arch = NULL;
       return (FALSE);
     }

    Info_new( __func__, LOG_NOTICE, domain, "DB Archive Connect OK with %s@%s:%d on %s", db_username, db_hostname, db_port, db_database );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* DB_Update: Met a jour le schema de database                                                                                */
/* Entrée: aucune. Tout se fait sur le domain 'master'                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Master_Update ( void )
  { struct DOMAIN *master = DOMAIN_tree_get ( "master" );
    DB_Write ( master, "CREATE TABLE IF NOT EXISTS database_version ("
                       "`date` DATETIME NOT NULL DEFAULT NOW(),"
                       "`version` INT(11) NULL"
                       ") ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS domains ("
                       "`domain_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`domain_uuid` VARCHAR(37) UNIQUE NOT NULL,"
                       "`domain_secret` VARCHAR(128) NOT NULL,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`domain_name` VARCHAR(256) NOT NULL DEFAULT 'My new domain',"
                       "`db_password` VARCHAR(64) NULL,"
                       "`db_version` INT(11) NOT NULL DEFAULT '0',"
                       "`archive_retention` INT(11) NOT NULL DEFAULT 700,"
                       "`image` MEDIUMTEXT NULL"
                       ") ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `icons` ("
                       "`icon_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`categorie` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                       "`forme` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`extension` VARCHAR(4) NOT NULL DEFAULT 'svg',"
                       "`ihm_affichage` VARCHAR(32) NOT NULL DEFAULT 'static',"
                       "`layer` INT(11) NOT NULL DEFAULT '100',"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW()"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;");

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `users` ("
                       "`user_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`user_uuid` VARCHAR(37) UNIQUE NOT NULL,"
                       "`default_domain_uuid` VARCHAR(37) UNIQUE NULL,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`date_inhib` DATETIME NULL DEFAULT NULL,"
                       "`email` VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`username` VARCHAR(64) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`salt` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`hash` VARCHAR(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`phone` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`xmpp` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`enable` BOOLEAN NOT NULL DEFAULT '0',"
                       "`enable_token` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
                       "CONSTRAINT `key_default_domain_uuid` FOREIGN KEY (`default_domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE SET NULL ON UPDATE CASCADE"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `users_grants` ("
                       "`grant_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`user_uuid` VARCHAR(37) NOT NULL,"
                       "`domain_uuid` VARCHAR(37) NOT NULL,"
                       "`access_level` INT(11) NOT NULL DEFAULT '6',"
                       "`can_send_txt` BOOLEAN NOT NULL DEFAULT '0',"
                       "`can_recv_sms` BOOLEAN NOT NULL DEFAULT '0',"
                       "UNIQUE (`user_uuid`,`domain_uuid`),"
                       "CONSTRAINT `key_user_uuid`   FOREIGN KEY (`user_uuid`)   REFERENCES `users`   (`user_uuid`) ON DELETE CASCADE ON UPDATE CASCADE,"
                       "CONSTRAINT `key_domain_uuid` FOREIGN KEY (`domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return(FALSE);
    DB_Read ( master, RootNode, NULL, "SELECT * FROM database_version ORDER BY date DESC LIMIT 1" );
    gint version = Json_get_int ( RootNode, "version" );
    json_node_unref(RootNode);

    if (version < 1)
     { DB_Write ( master, "ALTER TABLE domains ADD `description` VARCHAR(256) NOT NULL DEFAULT 'My domain' AFTER `email`" ); }

    if (version < 2)
     { DB_Write ( master, "ALTER TABLE domains ADD `db_version` INT(11) NOT NULL DEFAULT '0' AFTER `db_port`" ); }

    if (version < 3)
     { DB_Write ( master, "ALTER TABLE domains ADD `domain_secret` VARCHAR(128) NOT NULL AFTER `domain_uuid`" ); }

    if (version < 4)
     { DB_Write ( master, "ALTER TABLE domains ADD `image` MEDIUMTEXT NULL" ); }

    if (version < 5)
     { DB_Write ( master, "ALTER TABLE domains CHANGE `username` `owner` VARCHAR(256) NOT NULL" ); }

    if (version < 6)
     { DB_Write ( master, "ALTER TABLE domains CHANGE `owner` `owner_uuid` VARCHAR(37) NOT NULL" ); }

    if (version < 7)
     { DB_Write ( master, "ALTER TABLE users ADD `default_domain_uuid` VARCHAR(37) NOT NULL AFTER `user_uuid`" ); }

    if (version < 8)
     { DB_Write ( master, "ALTER TABLE domains CHANGE `description` `domain_name` VARCHAR(256) NOT NULL DEFAULT 'My new domain'" ); }

    if (version < 9)
     { DB_Write ( master, "ALTER TABLE domains DROP `owner_uuid`" ); }

    if (version < 10)
     { DB_Write ( master, "ALTER TABLE domains DROP `db_database`" );
       DB_Write ( master, "ALTER TABLE domains DROP `db_username`" );
       DB_Write ( master, "ALTER TABLE domains DROP `db_arch_username`" );
       DB_Write ( master, "ALTER TABLE domains DROP `db_arch_database`" );
     }

    if (version < 11)
     { DB_Write ( master, "ALTER TABLE domains ADD `archive_retention` INT(11) NOT NULL DEFAULT 700 AFTER `db_arch_port`" ); }

    if (version < 12)
     { DB_Write ( master, "ALTER TABLE domains DROP `db_hostname`" );
       DB_Write ( master, "ALTER TABLE domains DROP `db_arch_username`" );
     }

    if (version < 13)
     { DB_Write ( master, "ALTER TABLE domains DROP `db_port`" );
       DB_Write ( master, "ALTER TABLE domains DROP `db_arch_port`" );
     }

    if (version < 14)
     { DB_Write ( master, "ALTER TABLE domains DROP `db_arch_password`" ); }

    if (version < 15)
     { DB_Write ( master, "ALTER TABLE users DROP create_domain" );
       DB_Write ( master, "ALTER TABLE users DROP comment" );
     }

    if (version < 16)
     { DB_Write ( master, "ALTER TABLE users CHANGE `default_domain_uuid` `default_domain_uuid` VARCHAR(37) UNIQUE NULL" );
       DB_Write ( master, "ALTER TABLE users ADD CONSTRAINT `key_default_domain_uuid` "
                          "FOREIGN KEY (`default_domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE SET NULL ON UPDATE CASCADE" );
     }

    version = 16;
    DB_Write ( master, "INSERT INTO database_version SET version='%d'", version );

    Info_new( __func__, LOG_INFO, NULL, "Master Schema Updated" );
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
/* DB_Arch_Read: Envoie une requete en parametre au serveur de base de données spécifique aux archivages                      */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 static gboolean DB_Read_query ( struct DOMAIN *domain, MYSQL *mysql, JsonNode *RootNode, gchar *array_name, gchar *requete )
  { if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED (%s) for '%s'", (char *)mysql_error(mysql), requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "%s", (char *)mysql_error(mysql) );
       if (array_name)
        { gchar chaine[80];
          g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
          Json_node_add_int  ( RootNode, chaine, 0 );
          Json_node_add_array( RootNode, array_name );                            /* Ajoute un array vide en cas d'erreur SQL */
        }
       return(FALSE);
     }
    else Info_new( __func__, LOG_DEBUG, domain, "DB OK for '%s'", requete );

    MYSQL_RES *result = mysql_store_result ( mysql );
    if ( ! result )
     { Info_new( __func__, LOG_WARNING, domain, "Store_result failed (%s)", (char *) mysql_error(mysql) );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "%s", (char *)mysql_error(mysql) );
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
/******************************************************************************************************************************/
/* DB_Read: Envoie une requete en parametre au serveur de base de données                                                     */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Read ( struct DOMAIN *domain, JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    if (!domain)
     { Info_new( __func__, LOG_ERR, domain, "Domain not found. Dropping." ); return(FALSE); }

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *requete = g_try_malloc(taille+1);
    if (!requete)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Memory Error for '%s'", requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "Memory Error" );
       return(FALSE);
     }

    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );

    MYSQL *mysql = DB_Pool_take ( domain );
    gboolean retour = DB_Read_query ( domain, mysql, RootNode, array_name, requete );
    DB_Pool_unlock ( domain, mysql );
    g_free(requete);
    return(retour);
  }
/******************************************************************************************************************************/
/* DB_Arch_Read: Envoie une requete en parametre au serveur de base de données spécifique aux archivages                      */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Arch_Read ( struct DOMAIN *domain, JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    if (! (domain && domain->mysql_arch) )
     { Info_new( __func__, LOG_ERR, domain, "Domain not found or not connected. Dropping." ); return(FALSE); }
    MYSQL *mysql = domain->mysql_arch;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *requete = g_try_malloc(taille+1);
    if (!requete)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Memory Error for '%s'", requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "Memory Error" );
       return(FALSE);
     }

    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );

    gboolean retour = DB_Read_query ( domain, mysql, RootNode, array_name, requete );
    g_free(requete);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
