/******************************************************************************************************************************/
/* database.c          Gestion des connexions à la base de données                                                            */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * db.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

    if (!pre_comment) return(NULL);

    const gchar *end;
    if (!g_utf8_validate( pre_comment, -1, &end ))                                                      /* Validate la chaine */
     { Info_new( __func__, LOG_WARNING, NULL, "Could not validate UTF8 string" );
       Info_new( __func__, LOG_WARNING, NULL, "%s", pre_comment );
       return(NULL);
     }

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
       else if (car =='\n')                                                                        /* Dédoublage du backspace */
        { g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "n", 1 ); cible = g_utf8_next_char( cible );
        }
       else
        { g_utf8_strncpy( cible, source, 1 ); cible = g_utf8_next_char( cible );
        }
       source = g_utf8_next_char(source);
     }
    return(comment);
  }
/******************************************************************************************************************************/
/* DB_Pool_take: Prend un token dans le pool de connexion base de données                                                     */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 static MYSQL *DB_Pool_take ( struct DOMAIN *domain )
  { if (!domain) return(NULL);
    if (!domain->mysql[0])
     { Info_new( __func__, LOG_ERR, domain, "No pool available. Dropping. Starting." );
       if (!DB_Arch_Pool_init ( domain ))
        { Info_new( __func__, LOG_ERR, domain, "Failed to start DB_Pool. Dropping." );
          return(NULL);
        }
     }

encore:
    for (gint i=0; i<DATABASE_POOL_SIZE; i++)
     { if ( pthread_mutex_trylock ( &domain->mysql_mutex[i] ) == 0 ) return(domain->mysql[i]); }

    Info_new( __func__, LOG_ERR, domain, "All pool are busy. waiting before retry." );
    sleep(1);
    goto encore;
  }
/******************************************************************************************************************************/
/* DB_Pool_unlock: Rend un token dans le pool de connexion base de données                                                    */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 static void DB_Pool_unlock ( struct DOMAIN *domain, MYSQL *mysql )
  { if (!domain) return;

    for (gint i=0; i<DATABASE_POOL_SIZE; i++)
     { if ( domain->mysql[i] == mysql ) pthread_mutex_unlock ( &domain->mysql_mutex[i] ); }
  }
/******************************************************************************************************************************/
/* DB_Arch_Pool_take: Prend un token dans le pool de connexion base de données d'archivage                                    */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 static MYSQL *DB_Arch_Pool_take ( struct DOMAIN *domain )
  { if (!domain) return(NULL);
    if (!domain->mysql_arch[0])
     { Info_new( __func__, LOG_ERR, domain, "No pool available. Starting." );
       if (!DB_Arch_Pool_init ( domain ))
        { Info_new( __func__, LOG_ERR, domain, "Failed to start DB_Arch_Pool. Dropping." );
          return(NULL);
        }
     }

encore:
    for (gint i=0; i<DATABASE_POOL_SIZE; i++)
     { if ( pthread_mutex_trylock ( &domain->mysql_arch_mutex[i] ) == 0 ) return(domain->mysql_arch[i]); }

    Info_new( __func__, LOG_ERR, domain, "All pool are busy. waiting before retry." );
    sleep(1);
    goto encore;
  }
/******************************************************************************************************************************/
/* DB_Arch_Pool_unlock: Rend un token dans le pool de connexion base de données d'archivage                                   */
/* Entrée: le json representant le domaine                                                                                    */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 static void DB_Arch_Pool_unlock ( struct DOMAIN *domain, MYSQL *mysql )
  { if (!domain) return;

    for (gint i=0; i<DATABASE_POOL_SIZE; i++)
     { if ( domain->mysql_arch[i] == mysql ) pthread_mutex_unlock ( &domain->mysql_arch_mutex[i] ); }
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

    gint top = Global.Top;
    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );
    MYSQL *mysql = DB_Pool_take ( domain );
    if (!mysql)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: No pool available for '%s'", requete );
       retour = FALSE;
     }
    else if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: %s for '%s'", (char *)mysql_error(mysql), requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "%s", (char *)mysql_error(mysql) );
       retour = FALSE;
     }
    else
     { Info_new( __func__, LOG_DEBUG, domain, "DB OK in %04.1fs: '%s'", (Global.Top - top) / 10.0, requete );
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

    gint top = Global.Top;
    va_start( ap, format );
    g_vsnprintf ( requete, taille, format, ap );
    va_end ( ap );
    MYSQL *mysql = DB_Arch_Pool_take ( domain );
    if (!mysql)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: No pool available for '%s'", requete );
       retour = FALSE;
     }
    else if ( mysql_query ( mysql, requete ) )
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: %s for '%s'", (char *)mysql_error(mysql), requete );
       g_snprintf ( domain->mysql_last_error, sizeof(domain->mysql_last_error), "%s", (char *)mysql_error(mysql) );
       retour = FALSE;
     }
    else
     { Info_new( __func__, LOG_DEBUG, domain, "DB OK in %04.1fs: '%s'", (Global.Top - top) / 10.0, requete );
       retour = TRUE;
     }
    DB_Arch_Pool_unlock ( domain, mysql );
    g_free(requete);
    return(retour);
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
          domain->mysql[i] = NULL;
        }
     }
    for (i=0; i<DATABASE_POOL_SIZE; i++)
     { if (domain->mysql_arch[i])
        { mysql_close ( domain->mysql_arch[i] );
          pthread_mutex_destroy( &domain->mysql_arch_mutex[i] );
          domain->mysql_arch[i] = NULL;
        }
     }
  }
/******************************************************************************************************************************/
/* DB_Pool_init: Alloue un pool de connexion vers la base de données du domain en parametre                                   */
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
     { Info_new( __func__, LOG_ERR, domain, "Cannot load any DBPool for %s@%s:%d on %s", db_username, db_hostname, db_port, db_database );
       return(FALSE);
     }
    Info_new( __func__, LOG_INFO, domain, "%d Pools OK with %s@%s:%d on %s", i, db_username, db_hostname, db_port, db_database );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* DB_Arch_Pool_init: Alloue un pool de connexion vers la base de données d'archive du domain en parametre                    */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Arch_Pool_init ( struct DOMAIN *domain )
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

    gint i;
    for (i=0; i<DATABASE_POOL_SIZE; i++)
     { domain->mysql_arch[i] = mysql_init(NULL);
       if (!domain->mysql_arch[i])
        { Info_new( __func__, LOG_ERR, domain, "Unable to init domain database pool %d", i );
          break;
        }

       pthread_mutex_init( &domain->mysql_arch_mutex[i], NULL );
       my_bool reconnect = 1;
       mysql_options( domain->mysql_arch[i], MYSQL_OPT_RECONNECT, &reconnect );
       gint timeout = 10;
       mysql_options( domain->mysql_arch[i], MYSQL_OPT_CONNECT_TIMEOUT, &timeout );          /* Timeout en cas de non reponse */
       mysql_options( domain->mysql_arch[i], MYSQL_SET_CHARSET_NAME, (void *)"utf8" );

       if ( ! mysql_real_connect( domain->mysql_arch[i], db_hostname, db_username, db_password, db_database, db_port, NULL, 0 ) )
        { Info_new( __func__, LOG_ERR, domain, "Mysql_real_connect failed to connect to %s@%s:%d/%s for pool %d -> %s",
                   db_username, db_hostname, db_port, db_database, i,
                   (char *) mysql_error(domain->mysql_arch[i]) );
          mysql_close( domain->mysql_arch[i] );
          domain->mysql_arch[i] = NULL;
          return (FALSE);
        }
     }

    if (!i)
     { Info_new( __func__, LOG_ERR, domain, "Cannot load any DBArchPool for %s@%s:%d on %s", db_username, db_hostname, db_port, db_database );
       return(FALSE);
     }

    Info_new( __func__, LOG_INFO, domain, "%d Pools OK with %s@%s:%d on %s", i, db_username, db_hostname, db_port, db_database );
    return(TRUE);
  }

/******************************************************************************************************************************/
/* DB_Load_modes_for_icon: Met a jour la base des modes pour l'icone en parametre                                             */
/* Entrée: un mode en tant qu'element, et l'icone au format json an tant que user_data                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DB_Load_modes_for_icon ( JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *master = DOMAIN_tree_get ( "master" );
    JsonNode *icone = user_data;

    gchar *forme = Normaliser_chaine ( Json_get_string ( icone, "forme" ) );
    gchar *mode  = Normaliser_chaine ( json_node_get_string ( element ) );
    DB_Write ( master, "INSERT INTO `icons_modes` SET forme='%s', mode='%s' ON DUPLICATE KEY UPDATE icon_mode_id=icon_mode_id", forme, mode );
    g_free(mode);
    g_free(forme);
  }
/******************************************************************************************************************************/
/* DB_Load_one_icon: Met a jour la base des icones                                                                            */
/* Entrée: un element recu de la part de l'inventaire, au format json                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DB_Load_one_icon ( JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct DOMAIN *master = DOMAIN_tree_get ( "master" );
    gchar *categorie     = Normaliser_chaine ( Json_get_string ( element, "categorie" ) );
    gchar *forme         = Normaliser_chaine ( Json_get_string ( element, "forme" ) );
    gchar *extension     = Normaliser_chaine ( Json_get_string ( element, "extension" ) );
    gchar *controle      = Normaliser_chaine ( Json_get_string ( element, "controle" ) );
    gchar *default_mode  = Normaliser_chaine ( Json_get_string ( element, "default_mode" ) );
    gchar *default_color = Normaliser_chaine ( Json_get_string ( element, "default_color" ) );

    if ( categorie && forme && extension && controle )
     { DB_Write ( master,
                  "INSERT INTO icons SET "
                  "categorie='%s', forme='%s', extension='%s', controle='%s', default_mode='%s', default_color='%s' "
                  "ON DUPLICATE KEY UPDATE categorie=VALUE(categorie), extension=VALUE(extension), controle=VALUE(controle), "
                  "default_mode=VALUE(default_mode), default_color=VALUE(default_color)",
                  categorie, forme, extension, controle, default_mode, default_color );
       if (Json_has_member ( element, "modes" ))
        { DB_Write ( master, "DELETE FROM icons_modes WHERE forme='%s'", forme );
          Json_node_foreach_array_element ( element, "modes", DB_Load_modes_for_icon, element );
        }
       Info_new( __func__, LOG_INFO, master, "Icon '%s:%s' control '%s' imported", categorie, forme, controle );
     }
    else Info_new( __func__, LOG_ERR, master, "Error when importing icon '%s'", Json_get_string ( element, "forme" ) );

    g_free(categorie);
    g_free(forme);
    g_free(extension);
    g_free(controle);
    g_free(default_mode);
    g_free(default_color);
  }
/******************************************************************************************************************************/
/* DB_Icons_Update: Met a jour la base des icones                                                                             */
/* Entrée: aucune. Tout se fait sur le domain 'master'                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean DB_Icons_Update ( void )
  { gboolean retour = FALSE;
    gchar icon_query[256];                                   /* Récupération de l'inventaire json sur static.abls-habitat.fr  */
    g_snprintf( icon_query, sizeof(icon_query), "%s/inventory.json", Json_get_string ( Global.config, "static_data_url" ) );
    SoupSession *session  = soup_session_new();
    SoupMessage *soup_msg = soup_message_new ( "GET", icon_query );
    GError *error    = NULL;
    GBytes *response = soup_session_send_and_read ( session, soup_msg, NULL, &error ); /* SYNC */

    gchar *reason_phrase = soup_message_get_reason_phrase(soup_msg);
    gint   status_code   = soup_message_get_status(soup_msg);

    if (error)
     { gchar *uri = g_uri_to_string(soup_message_get_uri(soup_msg));
       Info_new( __func__, LOG_ERR, NULL, "Unable to retrieve ICON INVENTORY on %s: error %s", icon_query, error->message );
       g_free(uri);
       g_error_free ( error );
     }
    else if (status_code==200)
     { gsize taille;
       gchar *buffer_unsafe = g_bytes_get_data ( response, &taille );
       gchar *buffer_safe   = g_try_malloc0 ( taille + 1 );
       if (taille && buffer_safe)
        { memcpy ( buffer_safe, buffer_unsafe, taille );                                        /* Copy with \0 end of string */
          JsonNode *ResponseNode = Json_get_from_string ( buffer_safe );

          if (!ResponseNode)
           { Info_new( __func__, LOG_ERR, NULL, "Unable to retrieve ICON INVENTORY on %s: 'inventory.json' not json", icon_query ); }
          else if (!Json_has_member ( ResponseNode, "icons" ))
           { Info_new( __func__, LOG_ERR, NULL, "Unable to retrieve ICON INVENTORY on %s: 'inventory.json' do not have 'icons' array", icon_query ); }
          else
           { Json_node_foreach_array_element ( ResponseNode, "icons", DB_Load_one_icon, NULL );
             Info_new( __func__, LOG_NOTICE, NULL, "ICON INVENTORY loaded from %s", icon_query );
             retour = TRUE;
           }
          g_free(buffer_safe);
          json_node_unref ( ResponseNode );
        }
     }
    else Info_new( __func__, LOG_CRIT, NULL, "Unable to retrieve ICON INVENTORY on %s: error %s", icon_query, reason_phrase );
    if (response) g_bytes_unref ( response );
    g_object_unref( soup_msg );
    soup_session_abort ( session );
    g_object_unref( session );
    return(retour);
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
                       "`mqtt_password` VARCHAR(128) NOT NULL,"
                       "`browser_password` VARCHAR(128) NOT NULL,"
                       "`archive_retention` INT(11) NOT NULL DEFAULT 700,"
                       "`image` MEDIUMTEXT NULL,"
                       "`notif` VARCHAR(256) NOT NULL DEFAULT ''"
                       ") ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `icons` ("
                       "`icon_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`categorie` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                       "`forme` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`extension` VARCHAR(4) NOT NULL DEFAULT 'svg',"
                       "`controle` VARCHAR(32) NOT NULL DEFAULT 'static',"
                       "`default_mode` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                       "`default_color` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW()"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;");

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `icons_modes` ("
                       "`icon_mode_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`forme` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`mode` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "CONSTRAINT `key_icons_modes_forme` FOREIGN KEY (`forme`) REFERENCES `icons` (`forme`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;");

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `users` ("
                       "`user_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`user_uuid` VARCHAR(37) UNIQUE NOT NULL,"
                       "`default_domain_uuid` VARCHAR(37) NULL,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`date_inhib` DATETIME NULL DEFAULT NULL,"
                       "`email` VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`username` VARCHAR(64) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                       "`phone` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`xmpp` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`enable` BOOLEAN NOT NULL DEFAULT '0',"
                       "`free_sms_api_user` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`free_sms_api_key` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "CONSTRAINT `key_default_domain_uuid` FOREIGN KEY (`default_domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE SET NULL ON UPDATE CASCADE"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `users_invite` ("
                       "`user_invite_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`email` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
                       "`domain_uuid` VARCHAR(37) NOT NULL,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`access_level` INT(11) NOT NULL DEFAULT '1',"
                       "UNIQUE (`email`, `domain_uuid`),"
                       "CONSTRAINT `key_users_invite_domain_uuid` FOREIGN KEY (`domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( master, "CREATE TABLE IF NOT EXISTS `users_grants` ("
                       "`grant_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`user_uuid` VARCHAR(37) NOT NULL,"
                       "`domain_uuid` VARCHAR(37) NOT NULL,"
                       "`access_level` INT(11) NOT NULL DEFAULT '6',"
                       "`can_send_txt_cde` BOOLEAN NOT NULL DEFAULT '0',"
                       "`wanna_be_notified` BOOLEAN NOT NULL DEFAULT '0',"
                       "UNIQUE (`user_uuid`,`domain_uuid`),"
                       "CONSTRAINT `key_users_grants_user_uuid`   FOREIGN KEY (`user_uuid`)   REFERENCES `users`   (`user_uuid`)   ON DELETE CASCADE ON UPDATE CASCADE,"
                       "CONSTRAINT `key_users_grants_domain_uuid` FOREIGN KEY (`domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Icons_Update();

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

    if (version < 17)
     { DB_Write ( master, "ALTER TABLE users DROP `salt`" );
       DB_Write ( master, "ALTER TABLE users DROP `hash`" );
     }

    if (version < 18)
     { DB_Write ( master, "ALTER TABLE users DROP `enable_token`" ); }

    if (version < 19)
     { DB_Write ( master, "ALTER TABLE users_grants CHANGE `can_recv_sms` `wanna_be_notified` BOOLEAN NOT NULL DEFAULT '0'" ); }

    if (version < 20)
     { DB_Write ( master, "ALTER TABLE users_grants CHANGE `can_send_txt` `can_send_txt_cde` BOOLEAN NOT NULL DEFAULT '0'" ); }

    if (version < 21)
     { DB_Write ( master, "ALTER TABLE icons ADD `default_mode` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL AFTER `ihm_affichage`" );
       DB_Write ( master, "ALTER TABLE icons ADD `default_color` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL AFTER `default_mode`" );
     }

    if (version < 22)
     { DB_Write ( master, "ALTER TABLE domains ADD `notif` VARCHAR(256) NOT NULL DEFAULT ''" ); }

    if (version < 23)
     { DB_Write ( master, "ALTER TABLE `icons` DROP `layer`"); }

    if (version < 24)
     { DB_Write ( master, "ALTER TABLE `icons` CHANGE `ihm_affichage` `controle` VARCHAR(32) NOT NULL DEFAULT 'static'" ); }

    if (version < 25)
     { DB_Write ( master, "ALTER TABLE domains ADD `bus_is_ssl` BOOLEAN NOT NULL DEFAULT '0'" ); }

    if (version < 26)
     { DB_Write ( master, "ALTER TABLE domains DROP `bus_is_ssl`" ); }

    if (version < 27)
     { DB_Write ( master, "ALTER TABLE `users` ADD `free_sms_api_user` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''" );
       DB_Write ( master, "ALTER TABLE `users` ADD `free_sms_api_key` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''" );
     }

    if (version < 28)
     { DB_Write ( master, "ALTER TABLE domains ADD `mqtt_password` VARCHAR(128) NOT NULL AFTER `db_version`" );
       DB_Write ( master, "UPDATE domains SET `mqtt_password`=SHA2(RAND(), 512)" );
     }

    if (version < 29)
     { DB_Write ( master, "ALTER TABLE domains ADD `browser_password` VARCHAR(128) NOT NULL AFTER `mqtt_password`" );
       DB_Write ( master, "UPDATE domains SET `browser_password`=SHA2(RAND(), 512)" );
     }

    version = 29;
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

    gboolean retour = FALSE;
    MYSQL *mysql = DB_Pool_take ( domain );
    if (!mysql)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: No pool available for '%s'", requete ); goto end; }
    retour = DB_Read_query ( domain, mysql, RootNode, array_name, requete );
    DB_Pool_unlock ( domain, mysql );
end:
    g_free(requete);
    return(retour);
  }
/******************************************************************************************************************************/
/* DB_Arch_Read: Envoie une requete en parametre au serveur de base de données spécifique aux archivages                      */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean DB_Arch_Read ( struct DOMAIN *domain, JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

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

    MYSQL *mysql = DB_Arch_Pool_take ( domain );
    gboolean retour = DB_Read_query ( domain, mysql, RootNode, array_name, requete );
    DB_Arch_Pool_unlock ( domain, mysql );
    g_free(requete);
    return(retour);
  }
/******************************************************************************************************************************/
/* DB_Read_from_file: Execute une requete SQL chargée depuis un fichier sur disque                                            */
/* Entrée: le nom de fichier relatif                                                                                          */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean DB_Read_from_file ( struct DOMAIN *domain, gchar *file )
  { struct stat stat_buf;
    Info_new( __func__, LOG_INFO, domain, "Loading DB file '%s'", file );
    gchar filename[256];
    g_snprintf ( filename, sizeof(filename), "%s/%s", ABLS_API_PKGDATADIR, file );

    if (stat ( filename, &stat_buf)==-1)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Stat DB Error for '%s'", filename );
       return(FALSE);
     }

    gchar *db_content = g_try_malloc0 ( stat_buf.st_size+1 );
    if (!db_content)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Memory DB Error for %s", filename );
       return(FALSE);
     }

    gint fd = open ( filename, O_RDONLY );
    if (!fd)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Open DB Error for %s", filename );
       g_free(db_content);
       return(FALSE);
     }
    if (read ( fd, db_content, stat_buf.st_size ) != stat_buf.st_size)
     { Info_new( __func__, LOG_ERR, domain, "DB FAILED: Read DB Error for '%s'", filename );
       g_free(db_content);
       close(fd);
       return(FALSE);
     }
    close(fd);
    gboolean retour = DB_Write ( domain, db_content );
    g_free(db_content);
    Info_new( __func__, LOG_NOTICE, domain, "DB file '%s' loaded", file );
    return ( retour );
  }
/******************************************************************************************************************************/
/* DB_Cleanup_thread: Appelé une fois par domaine pour faire le menage dans les tables d'archivage                            */
/* Entrée: le domaine                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DB_Cleanup_thread ( struct DOMAIN *domain )
  { prctl(PR_SET_NAME, "W-CleanSQL", 0, 0, 0 );
    Info_new( __func__, LOG_NOTICE, domain, "Starting DB_Cleanup_thread" );

encore:
    gboolean traite = FALSE;
    JsonNode *RootNode = Json_node_create();
    DB_Read ( domain, RootNode, NULL, "SELECT * FROM cleanup ORDER BY cleanup_id ASC LIMIT 1" );
    if (Json_has_member ( RootNode, "requete" ))
     { if (Json_get_bool ( RootNode, "archive" )) { DB_Arch_Write ( domain, "%s", Json_get_string ( RootNode, "requete" ) ); }
                                             else { DB_Write      ( domain, "%s", Json_get_string ( RootNode, "requete" ) ); }
       DB_Write ( domain, "DELETE FROM cleanup WHERE cleanup_id='%d'", Json_get_int ( RootNode, "cleanup_id" ) );
       traite = TRUE;
     }
    json_node_unref ( RootNode );
    if (traite) goto encore;
    domain->database_cleanup_TID = 0;
    pthread_exit(0);
  }
/******************************************************************************************************************************/
/* DB_Cleanup: Lance le menage (pthread) dans les databases du domaine en parametre issu du g_tree                            */
/* Entrée: le gtree                                                                                                           */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean DB_Cleanup ( gpointer key, gpointer value, gpointer data )
  { struct DOMAIN *domain = value;

    if(!strcasecmp ( key, "master" )) return(FALSE);                                    /* Pas d'archive sur le domain master */

    if(domain->database_cleanup_TID == 0)
     { if ( pthread_create( &domain->database_cleanup_TID, NULL, (void *)DB_Cleanup_thread, domain ) )
       { Info_new( __func__, LOG_ERR, domain, "Error while pthreading DB_Cleanup: %s", strerror(errno) ); }
     }

    return(FALSE); /* False = on continue */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
