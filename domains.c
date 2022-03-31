/******************************************************************************************************************************/
/* domains.c                      Gestion des domains dans l'API HTTP WebService                                              */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * domains.c
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
/* DOMAIN_create_domainDB: Création du schéma de base de données pour le domein_uuid en parametre                             */
/* Entrée: UUID                                                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_create_domainDB ( gchar *domain_uuid )
  { DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `instances` ("
               "`id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`instance_uuid` VARCHAR(37) UNIQUE NOT NULL,"
               "`hostname` VARCHAR(64) UNIQUE NOT NULL,"
               "`headless` BOOLEAN NOT NULL DEFAULT '1',"
               "`is_master` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_msrv` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_db` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_bus` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_trad` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_level` INT(11) NOT NULL DEFAULT 6,"
               "`start_time` DATETIME DEFAULT NOW(),"
               "`install_time` DATETIME DEFAULT NOW(),"
               "`description` VARCHAR(128) NOT NULL DEFAULT '',"
               "`version` VARCHAR(128) NOT NULL"
               ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=1;" );

   DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `teleinfoedf` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My Teleinfo EDF',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`port` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

   DB_Write ( domain_uuid,
              "CREATE TABLE IF NOT EXISTS `ups` ("
              "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
              "`date_create` datetime NOT NULL DEFAULT NOW(),"
              "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
              "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
              "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My UPS',"
              "`enable` BOOLEAN NOT NULL DEFAULT '1',"
              "`debug` BOOLEAN NOT NULL DEFAULT 0,"
              "`host` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
              "`name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
              "`admin_username` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
              "`admin_password` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
              "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
              ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );

   DB_Write ( domain_uuid,
              "CREATE TABLE IF NOT EXISTS `meteo` ("
              "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
              "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
              "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
              "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
              "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My Meteo',"
              "`enable` BOOLEAN NOT NULL DEFAULT '1',"
              "`debug` BOOLEAN NOT NULL DEFAULT 0,"
              "`token` VARCHAR(65) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
              "`code_insee` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
              "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
              ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

   DB_Write ( domain_uuid,
              "CREATE TABLE IF NOT EXISTS `modbus` ("
              "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
              "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
              "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
              "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
              "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My WAGO',"
              "`enable` BOOLEAN NOT NULL DEFAULT '1',"
              "`debug` BOOLEAN NOT NULL DEFAULT 0,"
              "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
              "`watchdog` INT(11) NOT NULL DEFAULT 50,"
              "`max_request_par_sec` INT(11) NOT NULL DEFAULT 50,"
              "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
              ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

   DB_Write ( domain_uuid,
              "CREATE TABLE IF NOT EXISTS `modbus_DI` ("
              "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
              "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
              "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
              "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
              "`num` INT(11) NOT NULL DEFAULT 0,"
              "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
              "UNIQUE (thread_tech_id, thread_acronyme),"
              "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
              ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

   DB_Write ( domain_uuid,
              "CREATE TABLE IF NOT EXISTS `modbus_DO` ("
              "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
              "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
              "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
              "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
              "`num` INT(11) NOT NULL DEFAULT 0,"
              "UNIQUE (thread_tech_id, thread_acronyme),"
              "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
              ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

   DB_Write ( domain_uuid,
              "CREATE TABLE IF NOT EXISTS `modbus_AI` ("
              "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
              "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
              "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
              "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
              "`num` INT(11) NOT NULL DEFAULT 0,"
              "`type_borne` INT(11) NOT NULL DEFAULT 0,"
              "`min` FLOAT NOT NULL DEFAULT 0,"
              "`max` FLOAT NOT NULL DEFAULT 100,"
              "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
              "`unite` VARCHAR(32) NOT NULL DEFAULT '',"
              "`archivage` INT(11) NOT NULL DEFAULT 0,"
              "UNIQUE (thread_tech_id, thread_acronyme),"
              "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
              ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `smsg` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`ovh_service_name` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`ovh_application_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`ovh_application_secret` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`ovh_consumer_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`nbr_sms` int(11) NOT NULL DEFAULT 0,"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `audio` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`language` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fr',"
               "`device` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `radio` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `dmx` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`device` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT', "
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `imsgs` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`jabberid` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`password` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `gpiod` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `gpiod_IO` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`num` INT(11) NOT NULL DEFAULT '0',"
               "`mode_inout` INT(11) NOT NULL DEFAULT '0',"
               "`mode_activelow` BOOLEAN NOT NULL DEFAULT '0',"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `phidget` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );


    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `phidget` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`instance_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`password` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`serial` INT(11) UNIQUE NOT NULL DEFAULT '0',"
               "FOREIGN KEY (`instance_uuid`) REFERENCES `instances` (`instance_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `phidget_AI` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`port` int(11) NOT NULL,"
               "`classe` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`capteur` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`intervalle` int(11) NOT NULL,"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `phidget` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `phidget_DI` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`port` int(11) NOT NULL,"
               "`classe` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`capteur` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `phidget` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE TABLE IF NOT EXISTS `phidget_DO` ("
               "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`port` int(11) NOT NULL,"
               "`classe` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`capteur` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `phidget` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain_uuid,
               "CREATE OR REPLACE VIEW subprocesses AS "
               "SELECT instance_uuid, 'teleinfoedf' AS thread_name, thread_tech_id, description FROM teleinfoedf UNION "
               "SELECT instance_uuid, 'meteo'       AS thread_name, thread_tech_id, description FROM meteo UNION "
               "SELECT instance_uuid, 'modbus'      AS thread_name, thread_tech_id, description FROM modbus UNION "
               "SELECT instance_uuid, 'smsg'        AS thread_name, thread_tech_id, description FROM smsg UNION "
               "SELECT instance_uuid, 'audio'       AS thread_name, thread_tech_id, description FROM audio UNION "
               "SELECT instance_uuid, 'radio'       AS thread_name, thread_tech_id, description FROM radio UNION "
               "SELECT instance_uuid, 'imsgs'       AS thread_name, thread_tech_id, description FROM imsgs UNION "
               "SELECT instance_uuid, 'gpiod'       AS thread_name, thread_tech_id, description FROM gpiod UNION "
               "SELECT instance_uuid, 'phidget'     AS thread_name, thread_tech_id, description FROM phidget UNION "
               "SELECT instance_uuid, 'ups'         AS thread_name, thread_tech_id, description FROM ups"
             );
  }
/******************************************************************************************************************************/
/* DOMAIN_update_domainDB: Met a jour la version de database du domaine                                                       */
/* Entrée: le domain                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_update_domainDB ( struct DOMAIN *domain )
  { gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    gint db_version    = Json_get_int ( domain->config, "db_version" );
    if (db_version<1)
     { DB_Write ( domain_uuid, "ALTER TABLE `instances` DROP `run_as`" );
       DB_Write ( domain_uuid, "ALTER TABLE `instances` ADD  `headless` BOOLEAN NOT NULL DEFAULT '1' AFTER `hostname`");
       DB_Write ( domain_uuid, "ALTER TABLE `instances` DROP `use_subdir`" );
     }
    db_version = 1;
    DB_Write ( "master", "UPDATE domains SET db_version=%d WHERE domain_uuid ='%s'", db_version, domain_uuid );
  }
/******************************************************************************************************************************/
/* DOMAIN_tree_get: Recherche la structure domaine en fonction du nom de l'uuid                                               */
/* Entrée: UUID                                                                                                               */
/* Sortie: struct DOMAIN, or NULL si erreur                                                                                   */
/******************************************************************************************************************************/
 struct DOMAIN *DOMAIN_tree_get ( gchar *domain_uuid )
  { return ( g_tree_lookup ( Global.domaines, domain_uuid ) ); }
/******************************************************************************************************************************/
/* DOMAIN_Load: Charge un domaine en mémoire depuis la base de données                                                        */
/* Entrée: Le domaine, sous la forme d'un JSON dans un tableau                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_Load ( JsonArray *array, guint index_, JsonNode *domaine_config, gpointer user_data )
  {
    if (!domaine_config)
     { Info_new ( __func__, LOG_ERR, "No Config. Loading Failed" ); return; }

    gchar *domain_uuid = Json_get_string ( domaine_config, "domain_uuid" );
    if (!domain_uuid)
     { Info_new ( __func__, LOG_ERR, "No domain_uuid. Loading Failed" ); return; }

    struct DOMAIN *domain = g_try_malloc0 ( sizeof(struct DOMAIN) );
    if (!domain)
     { Info_new ( __func__, LOG_ERR, "Memory Error. Loading Failed" ); return; }

    domain->config = json_node_copy ( domaine_config );
    g_tree_insert ( Global.domaines, domain_uuid, domain );                         /* Ajout dans l'arbre global des domaines */

    if (!DB_Connect ( domain ))                                            /* Activation de la connexion a la base de données */
     { Info_new ( __func__, LOG_ERR, "DB Connect failed. domain '%s' loaded but DB Query will failed", domain_uuid );
       return;
     }

    if (strcasecmp ( domain_uuid, "master" ) )
     { DOMAIN_create_domainDB ( domain_uuid );                                                         /* Création du domaine */
       DOMAIN_update_domainDB ( domain );
     }
    Info_new ( __func__, LOG_INFO, "Domain '%s' Loaded", domain_uuid );
  }
/******************************************************************************************************************************/
/* DOMAIN_Load: Charge un domaine en mémoire depuis la base de données                                                        */
/* Entrée: Le domaine, sous la forme d'un JSON dans un tableau                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_Load_all ( void )
  { JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { Info_new ( __func__, LOG_ERR, "Unable to load all Domains: Memory Error" );
       return;
     }

    Info_new( __func__, LOG_INFO, "Loading All Domains" );
    DB_Read ( "master", RootNode, "domains", "SELECT * FROM domains" );
    Json_node_foreach_array_element ( RootNode, "domains", DOMAIN_Load, NULL );
    Info_new( __func__, LOG_INFO, "%d Domains loaded", Json_get_int ( RootNode, "nbr_domains" ) );
    json_node_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                                           */
/* Entrée: La DB                                                                                                              */
/******************************************************************************************************************************/
 static gboolean DOMAIN_Unload_one ( gpointer key, gpointer value, gpointer data )
  { struct DOMAIN *domain = value;
    mysql_close ( domain->mysql );
    Info_new( __func__, LOG_INFO, "DOMAIN '%s' Disconnected", key );
    return(FALSE);
  }
/******************************************************************************************************************************/
/* DOMAIN_Unload_all : Se deconnecte d'une base de données en parametre                                                       */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_Unload_all( void )
  { g_tree_foreach ( Global.domaines, DOMAIN_Unload_one, NULL );
    g_tree_destroy ( Global.domaines );
    Global.domaines = NULL;
    Info_new( __func__, LOG_INFO, "All Domains are Disconnected" );
  }
/******************************************************************************************************************************/
/* DOMAIN_request_get: Appelé depuis libsoup                                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_request_get ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                                  SoupClientContext *client, gpointer user_data )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode) { soup_message_set_status_full (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" ); }

    DB_Read ( "master", RootNode, "domains", "SELECT * FROM domains" );
    Http_Send_json_response ( msg, "success", RootNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
