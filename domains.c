/******************************************************************************************************************************/
/* domains.c                      Gestion des domains dans l'API HTTP WebService                                              */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                16.02.2022 09:42:50 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * domains.c
 * This file is part of Abls-Habitat
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "Http.h"

 extern struct GLOBAL Global;                                                                       /* Configuration de l'API */
 #define DOMAIN_DATABASE_VERSION 63

/******************************************************************************************************************************/
/* DOMAIN_Comparer_tree_clef_for_bit: Compare deux clefs dans un tableau GTree                                                */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 gint DOMAIN_Comparer_tree_clef_for_bit ( JsonNode *node1, JsonNode *node2, gpointer user_data )
  { struct DOMAIN *domain = user_data;
    if (!node1) return(-1);
    if (!node2) return(1);
    gchar *tech_id_1 = Json_get_string ( node1, "tech_id" );
    gchar *tech_id_2 = Json_get_string ( node2, "tech_id" );
    if (!tech_id_1) { Info_new( __func__, LOG_ERR, domain, "tech_id1 is NULL", __func__ ); return(-1); }
    if (!tech_id_2) { Info_new( __func__, LOG_ERR, domain, "tech_id2 is NULL", __func__ ); return(1); }
    gint result = strcasecmp ( tech_id_1, tech_id_2 );
    if (result) return(result);
    gchar *acronyme_1 = Json_get_string ( node1, "acronyme" );
    gchar *acronyme_2 = Json_get_string ( node2, "acronyme" );
    if (!acronyme_1) { Info_new( __func__, LOG_ERR, domain, "acronyme1 is NULL", __func__ ); return(-1); }
    if (!acronyme_2) { Info_new( __func__, LOG_ERR, domain, "acronyme2 is NULL", __func__ ); return(1); }
    return( strcasecmp ( acronyme_1, acronyme_2 ) );
  }
/******************************************************************************************************************************/
/* DOMAIN_create_domainDB: Création du schéma de base de données pour le domein_uuid en parametre                             */
/* Entrée: UUID                                                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_create_domainDB ( struct DOMAIN *domain )
  { if (!domain) return;
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    Info_new( __func__, LOG_INFO, domain, "Creating Schema for '%s'", domain_uuid );
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `agents` ("
               "`agent_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`agent_uuid` VARCHAR(37) UNIQUE NOT NULL,"
               "`agent_hostname` VARCHAR(64) NOT NULL,"
               "`headless` BOOLEAN NOT NULL DEFAULT '1',"
               "`is_master` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_msrv` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_bus` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_dls` BOOLEAN NOT NULL DEFAULT 0,"
               "`log_level` INT(11) NOT NULL DEFAULT 6,"
               "`start_time` DATETIME DEFAULT NOW(),"
               "`install_time` DATETIME DEFAULT NOW(),"
               "`heartbeat_time` DATETIME DEFAULT NOW(),"
               "`description` VARCHAR(128) NOT NULL DEFAULT '',"
               "`version` VARCHAR(32) NOT NULL DEFAULT 'none',"
               "`branche` VARCHAR(32) NOT NULL DEFAULT 'none'"
               ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=1;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `teleinfoedf` ("
               "`teleinfoedf_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My Teleinfo EDF',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`port` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`standard` BOOLEAN NOT NULL DEFAULT '0',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `ups` ("
               "`ups_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My UPS',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`host` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
               "`name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
               "`admin_username` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
               "`admin_password` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `meteo` ("
               "`meteo_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My Meteo',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`token` VARCHAR(65) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`code_insee` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `modbus` ("
               "`modbus_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My WAGO',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`watchdog` INT(11) NOT NULL DEFAULT 50,"
               "`max_request_par_sec` INT(11) NOT NULL DEFAULT 50,"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `modbus_DI` ("
               "`modbus_di_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`num` INT(11) NOT NULL DEFAULT 0,"
               "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
               "`flip` BOOLEAN NOT NULL DEFAULT 0,"
               "`archivage` INT(11) NOT NULL DEFAULT 36000,"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `modbus_DO` ("
               "`modbus_do_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`num` INT(11) NOT NULL DEFAULT 0,"
               "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
               "`archivage` INT(11) NOT NULL DEFAULT 36000,"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `modbus_AI` ("
               "`modbus_ai_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`num` INT(11) NOT NULL DEFAULT 0,"
               "`type_borne` INT(11) NOT NULL DEFAULT 0,"
               "`min` FLOAT NOT NULL DEFAULT 0,"
               "`max` FLOAT NOT NULL DEFAULT 100,"
               "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
               "`unite` VARCHAR(32) NOT NULL DEFAULT '',"
               "`archivage` INT(11) NOT NULL DEFAULT '36000',"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `modbus_AO` ("
               "`modbus_ao_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`num` INT(11) NOT NULL DEFAULT 0,"
               "`type_borne` INT(11) NOT NULL DEFAULT 0,"
               "`min` FLOAT NOT NULL DEFAULT 0,"
               "`max` FLOAT NOT NULL DEFAULT 100,"
               "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
               "`unite` VARCHAR(32) NOT NULL DEFAULT '',"
               "`archivage` INT(11) NOT NULL DEFAULT '36000',"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `shelly` ("
               "`shelly_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My new shelly',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`string_id` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My new shelly',"
               "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `smsg` ("
               "`smsg_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`ovh_service_name` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`ovh_application_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`ovh_application_secret` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`ovh_consumer_key` VARCHAR(33) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*--------------------------------------------------------- Audio ------------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `audio` ("
               "`audio_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`language` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fr',"
               "`device` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`volume` INT(11) NOT NULL DEFAULT '100',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `audio_zones` ("
               "`audio_zone_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`audio_zone` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT'"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `audio_map` ("
               "`audio_map_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`audio_zone_id` int(11) NOT NULL,"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "FOREIGN KEY (`audio_zone_id`) REFERENCES `audio_zones` (`audio_zone_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*----------------------------------------------------------- Radio ----------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `radio` ("
               "`radio_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `dmx` ("
               "`dmx_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`device` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT', "
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `imsgs` ("
               "`imsgs_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`jabberid` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`password` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*------------------------------------------------- GPIOD --------------------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `gpiod` ("
               "`gpiod_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "UNIQUE (agent_uuid, thread_tech_id),"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `gpiod_IO` ("
               "`gpiod_io_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`num` INT(11) NOT NULL DEFAULT '0',"
               "`mode_inout` INT(11) NOT NULL DEFAULT '0',"
               "`mode_activelow` BOOLEAN NOT NULL DEFAULT '0',"
               "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
               "UNIQUE (thread_tech_id, thread_acronyme),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `gpiod` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*------------------------------------------------- Phidget ------------------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `phidget` ("
               "`phidget_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`heartbeat_time` DATETIME NOT NULL DEFAULT '0',"
               "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
               "`enable` BOOLEAN NOT NULL DEFAULT '1',"
               "`debug` BOOLEAN NOT NULL DEFAULT 0,"
               "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
               "`password` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`serial` INT(11) UNIQUE NOT NULL DEFAULT '0',"
               "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `phidget_IO` ("
               "`phidget_io_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` datetime NOT NULL DEFAULT NOW(),"
               "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`classe` VARCHAR(8) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`port` int(11) NOT NULL,"
               "`capteur` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
               "`intervalle` INT(11) NOT NULL DEFAULT 0,"
               "`archivage` INT(11) NOT NULL DEFAULT 36000,"
               "UNIQUE (thread_tech_id, port),"
               "FOREIGN KEY (`thread_tech_id`) REFERENCES `phidget` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*------------------------------------------------- D.L.S --------------------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `syns` ("
               "`syn_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`parent_id` INT(11) NOT NULL,"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
               "`image` MEDIUMTEXT COLLATE utf8_unicode_ci NULL,"
               "`page` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
               "`access_level` INT(11) NOT NULL DEFAULT '0',"
               "`mode_affichage` BOOLEAN NOT NULL DEFAULT '0',"
               "FOREIGN KEY (`parent_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "INSERT IGNORE INTO `syns` (`syn_id`, `parent_id`, `libelle`, `page`, `access_level` ) VALUES"
               "(1, 1, 'Accueil', 'DEFAULT_PAGE', 0);");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `dls` ("
               "`dls_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
               "`package` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'custom',"
               "`syn_id` INT(11) NOT NULL DEFAULT '0',"
               "`name` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
               "`shortname` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`enable` BOOLEAN NOT NULL DEFAULT '0',"
               "`compil_date` DATETIME NOT NULL DEFAULT NOW(),"
               "`compil_time` INT(11) NOT NULL DEFAULT '0',"
               "`compil_status` BOOLEAN NOT NULL DEFAULT '0',"
               "`compil_user` VARCHAR(32) NOT NULL DEFAULT '',"
               "`error_count` INT(11) NOT NULL DEFAULT '0',"
               "`warning_count` INT(11) NOT NULL DEFAULT '0',"
               "`nbr_compil` INT(11) NOT NULL DEFAULT '0',"
               "`sourcecode` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',"
               "`codec` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',"
               "`errorlog` TEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No Error',"
               "`nbr_ligne` INT(11) NOT NULL DEFAULT '0',"
               "`debug` BOOLEAN NOT NULL DEFAULT '0',"
               "FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "INSERT IGNORE INTO `dls` (`dls_id`, `syn_id`, `name`, `shortname`, `tech_id`, `enable`, `compil_date`, `compil_status` ) VALUES "
               "(1, 1, 'Système', 'Système', 'SYS', FALSE, 0, FALSE);");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `dls_packages` ("
               "`dls_package_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`name` VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
               "`description` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`sourcecode` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */'"
               ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `dls_params` ("
               "`dls_param_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default libelle',"
               "`valeur`  VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default value',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

/*--------------------------------------------- Mapping ----------------------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mappings` ("
               "`mapping_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`thread_tech_id` VARCHAR(32) NOT NULL,"
               "`thread_acronyme` VARCHAR(64) NOT NULL,"
               "`tech_id` VARCHAR(32) NULL DEFAULT NULL,"
               "`acronyme` VARCHAR(64) NULL DEFAULT NULL,"
               "UNIQUE (`thread_tech_id`,`thread_acronyme`),"
               "UNIQUE (`tech_id`,`acronyme`),"
               "UNIQUE (`thread_tech_id`,`thread_acronyme`,`tech_id`,`acronyme`)"
               ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=1;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_DI` ("
               "`mnemo_di_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 0,"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`etat` BOOLEAN NOT NULL DEFAULT '0',"
               "`archivage` INT(11) NOT NULL DEFAULT '864000'",
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_DO` ("
               "`mnemo_do_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`etat` BOOLEAN NOT NULL DEFAULT '0',"
               "`mono` BOOLEAN NOT NULL DEFAULT '0',"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`archivage` INT(11) NOT NULL DEFAULT '864000'",
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_AI` ("
               "`mnemo_ai_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`valeur` FLOAT NOT NULL DEFAULT '0',"
               "`unite` VARCHAR(32) NOT NULL DEFAULT '',"
               "`archivage` INT(11) NOT NULL DEFAULT '36000',"
               "`in_range` BOOLEAN NOT NULL DEFAULT '0',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_AO` ("
               "`mnemo_ao_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`valeur` FLOAT NOT NULL DEFAULT '0',"
               "`unite` VARCHAR(32) NOT NULL DEFAULT '',"
               "`archivage` INT(11) NOT NULL DEFAULT '36000',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_BI` ("
               "`mnemo_bi_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`etat` BOOLEAN NOT NULL DEFAULT 0,"
               "`groupe` INT(11) NOT NULL DEFAULT 0,"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_MONO` ("
               "`mnemo_mono_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`etat` BOOLEAN NOT NULL DEFAULT 0,"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_WATCHDOG` ("
               "`mnemo_watchdog_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_CI` ("
               "`mnemo_ci_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`etat` BOOLEAN NOT NULL DEFAULT '0',"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`valeur` INT(11) NOT NULL DEFAULT '0',"
               "`unite` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fois',"
               "`archivage` INT(11) NOT NULL DEFAULT '36000',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_CH` ("
               "`mnemo_ch_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`etat` BOOLEAN NOT NULL DEFAULT '0',"
               "`valeur` INT(11) NOT NULL DEFAULT '0',"
               "`archivage` INT(11) NOT NULL DEFAULT '864000',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_TEMPO` ("
               "`mnemo_tempo_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE` ("
               "`mnemo_horloge_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`access_level` INT(11) NOT NULL DEFAULT '0',"
               "`tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE_ticks` ("
               "`mnemo_horloge_tick_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`horloge_id` INT(11) NOT NULL,"
               "`heure` INT(11) NOT NULL DEFAULT '0',"
               "`minute` INT(11) NOT NULL DEFAULT '0',"
               "`lundi` BOOLEAN NOT NULL DEFAULT '0',"
               "`mardi` BOOLEAN NOT NULL DEFAULT '0',"
               "`mercredi` BOOLEAN NOT NULL DEFAULT '0',"
               "`jeudi` BOOLEAN NOT NULL DEFAULT '0',"
               "`vendredi` BOOLEAN NOT NULL DEFAULT '0',"
               "`samedi` BOOLEAN NOT NULL DEFAULT '0',"
               "`dimanche` BOOLEAN NOT NULL DEFAULT '0',"
               "`date_modif` DATETIME NOT NULL DEFAULT NOW(),"
               "FOREIGN KEY (`horloge_id`) REFERENCES `mnemos_HORLOGE` (`mnemo_horloge_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_REGISTRE` ("
               "`mnemo_registre_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,"
               "`valeur` FLOAT NOT NULL DEFAULT '0',"
               "`unite` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`archivage` INT(11) NOT NULL DEFAULT '36000',"
               "`map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`map_reponse_vocale` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun',"
               "UNIQUE (`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `mnemos_VISUEL` ("
               "`mnemo_visuel_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`forme` VARCHAR(80) NOT NULL DEFAULT 'unknown',"
               "`mode`  VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',"
               "`color` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'gray',"
               "`valeur` FLOAT NOT NULL DEFAULT 0,"
               "`cligno` BOOLEAN NOT NULL DEFAULT 0,"
               "`noshow` BOOLEAN NOT NULL DEFAULT 0,"
               "`disable` BOOLEAN NOT NULL DEFAULT 0,"
               "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,"
               "`minimum`   FLOAT NOT NULL DEFAULT '0',"
               "`maximum`   FLOAT NOT NULL DEFAULT '100',"
               "`seuil_ntb` FLOAT NOT NULL DEFAULT '5',"
               "`seuil_nb`  FLOAT NOT NULL DEFAULT '10',"
               "`seuil_nh`  FLOAT NOT NULL DEFAULT '90',"
               "`seuil_nth` FLOAT NOT NULL DEFAULT '95',"
               "`nb_decimal` INT(11) NOT NULL DEFAULT '2',"
               "`input_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`input_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "UNIQUE (`tech_id`, `acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*--------------------------------------------- Elements visuels -------------------------------------------------------------*/
/*    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `syns_liens` ("
               "`syn_lien_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`dls_id` INT(11) NOT NULL DEFAULT 0,"
               "`src_posx` INT(11) NOT NULL DEFAULT 0,"
               "`src_posy` INT(11) NOT NULL DEFAULT 0,"
               "`dst_posx` INT(11) NOT NULL DEFAULT 0,"
               "`dst_posy` INT(11) NOT NULL DEFAULT 0,"
               "`stroke` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'black',"
               "`stroke_dasharray` VARCHAR(32) COLLATE utf8_unicode_ci DEFAULT NULL,"
               "`stroke_width` INT(11) NOT NULL DEFAULT 1,"
               "`stroke_linecap` VARCHAR(32) COLLATE utf8_unicode_ci DEFAULT 'butt',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `syns_rectangles` ("
               "`syn_rect_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`dls_id` INT(11) NOT NULL DEFAULT 0,"
               "`posx` INT(11) NOT NULL DEFAULT 0,"
               "`posy` INT(11) NOT NULL DEFAULT 0,"
               "`width` INT(11) NOT NULL DEFAULT 10,"
               "`height` INT(11) NOT NULL DEFAULT 10,"
               "`rx` INT(11) NOT NULL DEFAULT 0,"
               "`ry` INT(11) NOT NULL DEFAULT 0,"
               "`stroke` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'black',"
               "`color` VARCHAR(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'gray',"
               "`stroke_width` INT(11) NOT NULL DEFAULT 1,"
               "`stroke_dasharray` VARCHAR(32) COLLATE utf8_unicode_ci NULL,"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `syns_camerasup` ("
               "`id` INT(11) NOT NULL AUTO_INCREMENT,"
               "`syn_id` INT(11) NOT NULL,"
               "`camera_src_id` INT(11) NOT NULL,"
               "`posx` INT(11) NOT NULL,"
               "`posy` INT(11) NOT NULL,"
               "PRIMARY KEY (`id`),"
               "FOREIGN KEY (`camera_src_id`) REFERENCES `cameras` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,"
               "FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;"*/

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `syns_motifs` ("
               "`syn_motif_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`mnemo_visuel_id` INT(11) NOT NULL,"
               "`dls_id` INT(11) NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`posx` INT(11) NOT NULL DEFAULT '0',"
               "`posy` INT(11) NOT NULL DEFAULT '0',"
               "`angle` INT(11) NOT NULL DEFAULT '0',"
               "`scale` FLOAT NOT NULL DEFAULT '1.0',"
               "`layer` INT(11) NOT NULL DEFAULT '0',"
               "`place` INT(11) NOT NULL DEFAULT '0',"
               "UNIQUE (`dls_id`, `mnemo_visuel_id`),"
               "FOREIGN KEY (`mnemo_visuel_id`) REFERENCES `mnemos_VISUEL` (`mnemo_visuel_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
               "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`dls_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

/*------------------------------------------------------- Tableaux -----------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `tableau` ("
               "`tableau_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
               "`titre` VARCHAR(128) UNIQUE NOT NULL,"
               "`syn_id` INT(11) NOT NULL,"
               "`mode` INT(11) NOT NULL DEFAULT 0,"
               "FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `tableau_map` ("
               "`tableau_map_id` INT(11) PRIMARY KEY AUTO_INCREMENT ,"
               "`tableau_id` INT(11) NOT NULL,"
               "`tech_id` VARCHAR(32) NOT NULL,"
               "`acronyme` VARCHAR(64) NOT NULL,"
               "`color` VARCHAR(16) NOT NULL DEFAULT 'blue',"
               "FOREIGN KEY (`tableau_id`) REFERENCES `tableau` (`tableau_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ")ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");

/*---------------------------------------------------------- Messages --------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `msgs` ("
               "`msg_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`deletable` BOOLEAN NOT NULL DEFAULT '1',"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`used` BOOLEAN NOT NULL DEFAULT 1,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No libelle',"
               "`typologie` INT(11) NOT NULL DEFAULT '0',"
               "`rate_limit` INT(11) NOT NULL DEFAULT '1',"
               "`notif_sms` INT(11) NOT NULL DEFAULT '-1',"
               "`notif_sms_by_dls` INT(11) NOT NULL DEFAULT '0',"
               "`notif_chat` INT(11) NOT NULL DEFAULT '-1',"
               "`notif_chat_by_dls` INT(11) NOT NULL DEFAULT '0',"
               "`audio_profil` VARCHAR(80) NOT NULL DEFAULT 'P_NONE',"
               "`audio_libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
               "`etat` BOOLEAN NOT NULL DEFAULT '0',"
               "UNIQUE(`tech_id`,`acronyme`),"
               "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
               ") ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;");

    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `histo_msgs` ("
               "`histo_msg_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`syn_page` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`dls_shortname` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`typologie` INT(11) NOT NULL DEFAULT '0',"
               "`nom_ack` VARCHAR(97) COLLATE utf8_unicode_ci DEFAULT NULL,"
               "`date_create` DATETIME(2) NULL,"
               "`date_fixe` DATETIME(2) NULL,"
               "`date_fin` DATETIME(2) NULL,"
               "`libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,"
               "KEY (`date_create`), "
               "KEY (`date_fin`), "
               "UNIQUE (`date_create`,`tech_id`,`acronyme`) "
               ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;");

/*-------------------------------------------------------- Audit log ---------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE TABLE IF NOT EXISTS `audit_log` ("
               "`audit_log_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
               "`username` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
               "`classe` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
               "`message` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,"
               "`date` DATETIME NOT NULL DEFAULT NOW(),"
               "KEY (`date`),"
               "KEY (`username`)"
               ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;");

    DB_Write ( domain, "CREATE TABLE `cleanup`("
                       "`cleanup_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                       "`archive` BOOLEAN NOT NULL DEFAULT '1',"
                       "`requete` VARCHAR(256) NOT NULL"
                       ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000" );

    DB_Arch_Write ( domain, "CREATE TABLE `status`("
                            "`tech_id` VARCHAR(32) NOT NULL,"
                            "`acronyme` VARCHAR(64) NOT NULL,"
                            "`rows` INT(11) NOT NULL DEFAULT 0,"
                            "`date_create` DATETIME(2) NOT NULL DEFAULT NOW(),"
                            "`last_update` DATETIME(2) NOT NULL DEFAULT NOW(),"
                            "UNIQUE (`tech_id`,`acronyme`) "
                            ") ENGINE=ARIA DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" );


    DB_Write ( DOMAIN_tree_get ("master"), "UPDATE domains SET db_version = %d WHERE domain_uuid='%s'", DOMAIN_DATABASE_VERSION, domain_uuid);
    Info_new( __func__, LOG_INFO, domain, "Domain '%s' created with db_version=%d", domain_uuid, DOMAIN_DATABASE_VERSION );
  }
/******************************************************************************************************************************/
/* DOMAIN_update_domainDB: Met a jour la version de database du domaine                                                       */
/* Entrée: le domain                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_update_domainDB ( struct DOMAIN *domain )
  { gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    gint db_version    = Json_get_int    ( domain->config, "db_version" );
    Info_new( __func__, LOG_INFO, domain, "Domain '%s' updating from db_version %d to %d", domain_uuid, db_version, DOMAIN_DATABASE_VERSION );
    if (db_version<1)
     { DB_Write ( domain, "ALTER TABLE `agents` DROP `run_as`" );
       DB_Write ( domain, "ALTER TABLE `agents` ADD  `headless` BOOLEAN NOT NULL DEFAULT '1' AFTER `hostname`");
       DB_Write ( domain, "ALTER TABLE `agents` DROP `use_subdir`" );
     }

    if (db_version<4)
     { DB_Write ( domain, "ALTER TABLE `agents` CHANGE `hostname` `agent_hostname` VARCHAR(64) UNIQUE NOT NULL" ); }

    if (db_version<5)
     { DB_Write ( domain, "ALTER TABLE `smsg` DROP `nbr_sms`" ); }

    if (db_version<6)
     { DB_Write ( domain, "ALTER TABLE `dls` CHANGE `actif` `enable` BOOLEAN NOT NULL DEFAULT '0'" ); }

    if (db_version<7)
     { DB_Write ( domain, "ALTER TABLE `dls` ADD `codec` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */' AFTER `sourcecode`" ); }

    if (db_version<8)
     { DB_Write ( domain, "DROP TABLE `syns_visuels`" ); }

    if (db_version<9)
     { DB_Write ( domain, "ALTER TABLE `dls` ADD `compil_time` INT(11) NOT NULL DEFAULT '0' AFTER `compil_date`"); }

    if (db_version<11)
     { DB_Write ( domain, "ALTER TABLE `dls` ADD `error_count` INT(11) NOT NULL DEFAULT '0' AFTER `compil_status`");
       DB_Write ( domain, "ALTER TABLE `dls` ADD `warning_count` INT(11) NOT NULL DEFAULT '0' AFTER `error_count`");
     }

    if (db_version<12)
     { DB_Write ( domain, "ALTER TABLE `mnemos_DI` ADD `etat` BOOLEAN NOT NULL DEFAULT '0'" ); }

    if (db_version<13)
     { DB_Write ( domain, "ALTER TABLE `histo_msgs` ADD `dls_shortname` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL AFTER `acronyme`"); }

    if (db_version<14)
     { DB_Write ( domain, "ALTER TABLE `histo_msgs` ADD `syn_page` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL AFTER `acronyme`");
       DB_Write ( domain, "ALTER TABLE `histo_msgs` ADD `typologie` INT(11) NOT NULL DEFAULT '0' AFTER `dls_shortname`" );
     }

    if (db_version<15)
     { DB_Write ( domain, "ALTER TABLE `agents` ADD `branche` VARCHAR(32) NOT NULL DEFAULT 'none'" ); }

    if (db_version<16)
     { DB_Write ( domain, "ALTER TABLE `audit_log` ADD `classe` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL AFTER `username`" ); }

    if (db_version<17)
     { DB_Write ( domain, "ALTER TABLE `teleinfoedf` ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `ups`         ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `meteo`       ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `modbus`      ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `smsg`        ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `audio`       ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `radio`       ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `dmx`         ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `imsgs`       ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `gpiod`       ADD `last_comm` DATETIME NULL AFTER `date_create`" );
       DB_Write ( domain, "ALTER TABLE `phidget`     ADD `last_comm` DATETIME NULL AFTER `date_create`" );
     }

    if (db_version<18)
     { DB_Write ( domain, "CREATE TABLE IF NOT EXISTS `syns_cadrans` ("
                          "`syn_cadran_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                          "`dls_id` INT(11) NOT NULL DEFAULT 0,"
                          "`forme` VARCHAR(80) NOT NULL DEFAULT 'unknown',"
                          "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                          "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                          "`groupe` INT(11) NOT NULL DEFAULT '0',"
                          "`posx` INT(11) NOT NULL DEFAULT '0',"
                          "`posy` INT(11) NOT NULL DEFAULT '0',"
                          "`scale` FLOAT NOT NULL DEFAULT '1.0',"
                          "`minimum` FLOAT NOT NULL DEFAULT '0',"
                          "`maximum` FLOAT NOT NULL DEFAULT '100',"
                          "`seuil_ntb` FLOAT NOT NULL DEFAULT '5',"
                          "`seuil_nb` FLOAT NOT NULL DEFAULT '10',"
                          "`seuil_nh` FLOAT NOT NULL DEFAULT '90',"
                          "`seuil_nth` FLOAT NOT NULL DEFAULT '95',"
                          "`angle` INT(11) NOT NULL DEFAULT '0',"
                          "`nb_decimal` INT(11) NOT NULL DEFAULT '2',"
                          "UNIQUE (`dls_id`, `tech_id`, `acronyme`),"
                          "FOREIGN KEY (`dls_id`) REFERENCES `dls` (`dls_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                          ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
     }

    if (db_version<19)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `cligno` BOOLEAN NOT NULL DEFAULT 0 AFTER `color`" ); }

    if (db_version<20)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` DROP `access_level`" ); }

    if (db_version<21)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `disable` BOOLEAN NOT NULL DEFAULT 0 AFTER `cligno`" ); }

    if (db_version<22)
     { DB_Write ( domain, "ALTER TABLE `mappings` DROP `libelle`" ); }

    if (db_version<23)
     { DB_Write ( domain, "CREATE TABLE IF NOT EXISTS `phidget_IO` ("
                       "`phidget_io_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`classe` VARCHAR(8) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`port` INT(11) NOT NULL,"
                       "`capteur` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                       "`libelle` VARCHAR(128) NOT NULL DEFAULT '',"
                       "`intervalle` int(11) NOT NULL DEFAULT 0,"
                       "UNIQUE (thread_tech_id, port),"
                       "FOREIGN KEY (`thread_tech_id`) REFERENCES `phidget` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       DB_Write ( domain, "DROP TABLE `phidget_DI`" );
       DB_Write ( domain, "DROP TABLE `phidget_DO`" );
       DB_Write ( domain, "DROP TABLE `phidget_AI`" );
       DB_Write ( domain, "DROP TABLE `phidget_AO`" );
     }

    if (db_version<24)
     { DB_Write ( domain, "ALTER TABLE `mappings` DROP `classe`" ); }

    if (db_version<25)
     { DB_Write ( domain, "ALTER TABLE `histo_msgs` ADD KEY (`tech_id`,`acronyme`)" ); }

    if (db_version<26)
     { DB_Write ( domain, "ALTER TABLE `mnemos_REGISTRE` CHANGE `archivage` `archivage` INT(11) NOT NULL DEFAULT 0" ); }

    if (db_version<27)
     { DB_Write ( domain, "ALTER TABLE `msgs` CHANGE `rate_limit` `rate_limit` INT(11) NOT NULL DEFAULT '1'");
       DB_Write ( domain, "CREATE TABLE duplicate_histo AS SELECT histo_msg_id FROM histo_msgs GROUP by date_create, tech_id, acronyme HAVING count(*) > 1; ");
       DB_Write ( domain, "DELETE histo_msgs FROM histo_msgs WHERE histo_msgs.histo_msg_id IN (SELECT * FROM duplicate_histo); ");
       DB_Write ( domain, "ALTER TABLE histo_msgs ADD UNIQUE (date_create, tech_id, acronyme); ");
       DB_Write ( domain, "DROP TABLE duplicate_histo; ");
    }

    if (db_version<28)
     { DB_Write ( domain, "ALTER TABLE syns_motifs ADD `layer` INT(11) NOT NULL DEFAULT '0'" );
       DB_Write ( domain, "ALTER TABLE syns_motifs REMOVE `groupe`" );
       DB_Write ( domain, "ALTER TABLE syns_motifs REMOVE `dialog`" );
       DB_Write ( domain, "ALTER TABLE syns_motifs REMOVE `gestion`" );
     }

    if (db_version<29)
     { DB_Write ( domain, "ALTER TABLE `mnemos_DO` ADD `mono` BOOLEAN NOT NULL DEFAULT '0' AFTER `etat`" ); }

    if (db_version<30)
     { DB_Write ( domain, "ALTER TABLE `agents` ADD `log_dls` BOOLEAN NOT NULL DEFAULT 0 AFTER `log_bus`" ); }

    if (db_version<31)
     { DB_Write ( domain, "ALTER TABLE `msgs` CHANGE `sms_notification` `txt_notification` INT(11) NOT NULL DEFAULT '0'" ); }

    if (db_version<32)
     { DB_Write ( domain, "UPDATE `dls` SET sourcecode=REPLACE(`sourcecode`, '=info', '=etat')" );
       DB_Write ( domain, "UPDATE `dls` SET sourcecode=REPLACE(`sourcecode`, '=attente', '=notification')" );
     }

    if (db_version<33)
     { DB_Write ( domain, "ALTER TABLE `dls` ADD `compil_user` VARCHAR(32) NOT NULL DEFAULT '' AFTER `compil_status`" ); }

    if (db_version<34)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `valeur` FLOAT NOT NULL DEFAULT 0 AFTER `color`" ); }

    if (db_version<35)
     { DB_Write ( domain, "ALTER TABLE `syns_motifs` ADD `place` INT(11) NOT NULL DEFAULT 0 AFTER `layer`" ); }

    if (db_version<36)
     { DB_Write ( domain, "CREATE TABLE IF NOT EXISTS `dls_params` ("
                          "`dls_param_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                          "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,"
                          "`acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,"
                          "`libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default_libelle',"
                          "UNIQUE (`tech_id`,`acronyme`),"
                          "FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                          ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
     }

    if (db_version<37)
     { DB_Write ( domain, "ALTER TABLE `modbus_DI` ADD `flip` BOOLEAN NOT NULL DEFAULT 0 AFTER `libelle`" ); }

    if (db_version<38)
     { DB_Write ( domain, "ALTER TABLE `phidget_IO` ADD `archivage` INT(11) NOT NULL DEFAULT 0 AFTER `intervalle`" ); }

    if (db_version<39)
     { DB_Write ( domain, "ALTER TABLE `audio` ADD `volume` INT(11) NOT NULL DEFAULT '100' AFTER `device`" ); }

    if (db_version<40)
     { DB_Write ( domain,
                  "CREATE TABLE IF NOT EXISTS `shelly` ("
                  "`shelly_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                  "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                  "`last_comm` DATETIME NULL,"
                  "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                  "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                  "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My WAGO',"
                  "`enable` BOOLEAN NOT NULL DEFAULT '1',"
                  "`debug` BOOLEAN NOT NULL DEFAULT 0,"
                  "`string_id` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My new shelly',"
                  "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                  "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                  ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
     }

    if (db_version<41)
     { DB_Write ( domain, "ALTER TABLE `modbus_DI` ADD `archivage` INT(11) NOT NULL DEFAULT '36000'" );
       DB_Write ( domain, "ALTER TABLE `modbus_DO` ADD `archivage` INT(11) NOT NULL DEFAULT '36000'" );
       DB_Write ( domain, "ALTER TABLE `mnemos_CH` ADD `archivage` INT(11) NOT NULL DEFAULT '864000'" );
       DB_Write ( domain, "UPDATE modbus_AI  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE modbus_AI  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE modbus_AI  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE modbus_AI  SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE modbus_AO  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE modbus_AO  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE modbus_AO  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE modbus_AO  SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE modbus_DI  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE modbus_DI  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE modbus_DI  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE modbus_DI  SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE modbus_DO  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE modbus_DO  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE modbus_DO  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE modbus_DO  SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE phidget_IO SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE phidget_IO SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE phidget_IO SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE phidget_IO SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE mnemos_CH  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE mnemos_CH  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE mnemos_CH  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE mnemos_CH  SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE mnemos_CI  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE mnemos_CI  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE mnemos_CI  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE mnemos_CI  SET archivage=864000 WHERE archivage=4" );
       DB_Write ( domain, "UPDATE mnemos_REGISTRE SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE mnemos_REGISTRE SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE mnemos_REGISTRE SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE mnemos_REGISTRE SET archivage=864000 WHERE archivage=4" );
     }

    if (db_version<42)
     { DB_Write ( domain, "ALTER TABLE `teleinfoedf` ADD `standard` BOOLEAN NOT NULL DEFAULT '0'" ); }

    if (db_version<43)
     { DB_Write ( domain, "UPDATE mnemos_AI  SET archivage=50     WHERE archivage=1" );
       DB_Write ( domain, "UPDATE mnemos_AI  SET archivage=600    WHERE archivage=2" );
       DB_Write ( domain, "UPDATE mnemos_AI  SET archivage=36000  WHERE archivage=3" );
       DB_Write ( domain, "UPDATE mnemos_AI  SET archivage=864000 WHERE archivage=4" );
     }

    if (db_version<44)
     { DB_Write ( domain, "CREATE TABLE `cleanup`("
                          "`cleanup_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                          "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                          "`archive` BOOLEAN NOT NULL DEFAULT '1',"
                          "`requete` VARCHAR(256) NOT NULL"
                          ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000"
                );
     }

    if (db_version<45)
     { gint top = Global.Top;
       DB_Arch_Write ( domain, "CREATE TABLE `histo_bit`("
                               "`tech_id` VARCHAR(32) NOT NULL,"
                               "`acronyme` VARCHAR(64) NOT NULL,"
                               "`date_time` DATETIME(2) NOT NULL,"
                               "`valeur` FLOAT NOT NULL,"
                               " UNIQUE (tech_id, acronyme, date_time),"
                               " INDEX (tech_id, acronyme),"
                               " INDEX (date_time)"
                               ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
                               "  PARTITION BY HASH (YEARWEEK(`date_time`)) PARTITIONS 52;" );

       DB_Arch_Write ( domain, "CREATE TABLE `status`("
                               "`tech_id` VARCHAR(32) NOT NULL,"
                               "`acronyme` VARCHAR(64) NOT NULL,"
                               "`rows` INT(11) NOT NULL,"
                               "`last_update` DATETIME(2) NOT NULL"
                               ") ENGINE=ARIA DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" );

       JsonNode *RootNode = Json_node_create();
       DB_Read ( domain, RootNode, "bits", "SELECT tech_id, acronyme FROM dictionnaire group by tech_id, acronyme " );
       GList *Requests = json_array_get_elements ( Json_get_array ( RootNode, "bits" ) );
       GList *requests = Requests;
       while(requests)
        { JsonNode *requete = requests->data;
          gchar *tech_id  = Json_get_string ( requete, "tech_id" );
          gchar *acronyme = Json_get_string ( requete, "acronyme" );
          DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete='ALTER TABLE histo_bit_%s_%s ADD `tech_id` VARCHAR(32) NOT NULL FIRST'", tech_id, acronyme );
          DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete='ALTER TABLE histo_bit_%s_%s ADD `acronyme` VARCHAR(64) NOT NULL AFTER `tech_id`'", tech_id, acronyme );
          DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete='UPDATE histo_bit_%s_%s SET tech_id=\"%s\", acronyme=\"%s\"'", tech_id, acronyme, tech_id, acronyme );
          DB_Write ( domain, "INSERT INTO cleanup SET archive = 1, requete='INSERT INTO histo_bit (tech_id, acronyme, date_time, valeur) SELECT tech_id, acronyme, date_time, valeur FROM histo_bit_%s_%s'", tech_id, acronyme );
          requests = g_list_next(requests);
        }
       g_list_free(Requests);
       json_node_unref ( RootNode );

       Info_new ( __func__, LOG_NOTICE, domain, "DATABASE Move Archive table in %f s", ( Global.Top - top ) / 10.0 );
     }

    if (db_version<46)
     { DB_Write ( domain, "ALTER TABLE `agents` ADD `heartbeat_time` DATETIME DEFAULT NOW() AFTER `install_time`" ); }

    if (db_version<47)
     { DB_Write ( domain, "ALTER TABLE `teleinfoedf` CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `meteo`       CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `shelly`      CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `modbus`      CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `smsg`        CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `audio`       CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `radio`       CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `imsgs`       CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `gpiod`       CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `ups`         CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `dmx`         CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
       DB_Write ( domain, "ALTER TABLE `phidget`     CHANGE `last_comm` `heartbeat_time` DATETIME NOT NULL DEFAULT NOW()" );
     }

    if (db_version<48)
     { DB_Write ( domain, "DROP TABLE `syns_cadrans`" ); }

    if (db_version<49)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `minimum`   FLOAT NOT NULL DEFAULT '0'" );
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `maximum`   FLOAT NOT NULL DEFAULT '100'");
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `seuil_ntb` FLOAT NOT NULL DEFAULT '5'");
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `seuil_nb`  FLOAT NOT NULL DEFAULT '10'");
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `seuil_nh`  FLOAT NOT NULL DEFAULT '90'");
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `seuil_nth` FLOAT NOT NULL DEFAULT '95'");
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `nb_decimal` INT(11) NOT NULL DEFAULT '2'");
     }

    if (db_version<50)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` CHANGE `nb_decimal` `decimal` INT(11) NOT NULL DEFAULT '2'"); }

    if (db_version<51)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `input_tech_id`  VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''" );
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `input_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT ''" );
       DB_Write ( domain, "ALTER TABLE `syns_motifs` DROP `rafraich`" );
       DB_Write ( domain, "ALTER TABLE `syns_motifs` DROP `larg`" );
       DB_Write ( domain, "ALTER TABLE `syns_motifs` DROP `haut`" );
       DB_Write ( domain, "ALTER TABLE `syns_motifs` DROP `dialog`" );
       DB_Write ( domain, "ALTER TABLE `syns_motifs` DROP `gestion`" );
       DB_Write ( domain, "ALTER TABLE `syns_motifs` DROP `groupe`" );
       DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` CHANGE `decimal` `nb_decimal` INT(11) NOT NULL DEFAULT '2'");
     }

    if (db_version<52)
     { DB_Arch_Write ( domain, "ALTER TABLE `status` ADD `date_create` DATETIME(2) NOT NULL DEFAULT NOW() AFTER `rows`" ); }

    if (db_version<53)
     { DB_Write ( domain, "ALTER TABLE `syns_motifs` ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `dls_id`" ); }

    if (db_version<54)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " ); }

    if (db_version<55)
     { DB_Write ( domain, "ALTER TABLE `mnemos_BI`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_MONO`     ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_CI`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_CH`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_DI`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_DO`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_AI`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_AO`       ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_HORLOGE`  ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_REGISTRE` ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_WATCHDOG` ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `mnemos_TEMPO`    ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
       DB_Write ( domain, "ALTER TABLE `msgs`            ADD `used` BOOLEAN NOT NULL DEFAULT 1 AFTER `acronyme` " );
     }

    if (db_version<56)
     { DB_Write ( domain, "ALTER TABLE `dls` DROP `debug`" ); }

    if (db_version<57)
     { DB_Write ( domain, "ALTER TABLE `mnemos_VISUEL` ADD `noshow` BOOLEAN NOT NULL DEFAULT 0 AFTER `cligno`" ); }

    if (db_version<58)
     { DB_Write ( domain, "CREATE TABLE `dls_packages` ("
                          "`dls_package_id` INT(11) PRIMARY KEY AUTO_INCREMENT,"
                          "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                          "`name` VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                          "`description` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',"
                          "`sourcecode` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */'"
                          ") ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;");
     }

    if (db_version<59)
     { DB_Write ( domain, "CREATE TABLE IF NOT EXISTS `audio_zones` ("
                          "`audio_zone_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                          "`date_create` datetime NOT NULL DEFAULT NOW(),"
                          "`audio_zone` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                          "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT'"
                          ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
       DB_Write ( domain, "CREATE TABLE IF NOT EXISTS `audio_map` ("
                          "`audio_map_id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                          "`date_create` datetime NOT NULL DEFAULT NOW(),"
                          "`audio_zone_id` int(11) NOT NULL,"
                          "`agent_uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                          "FOREIGN KEY (`audio_zone_id`) REFERENCES `audio_zones` (`audio_zone_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
                          "FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                          ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );
     }

    if (db_version<60)
     { DB_Write ( domain, "ALTER TABLE `dls_params` CHANGE `libelle` `valeur` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default value'" );
       DB_Write ( domain, "ALTER TABLE `dls_params` ADD `libelle` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default libelle'" );
     }

    if (db_version<61)
     { DB_Write ( domain, "ALTER TABLE `msgs` CHANGE `txt_notification` `notif_sms` INT(11) NOT NULL DEFAULT '-1'" );
       DB_Write ( domain, "ALTER TABLE `msgs` ADD `notif_sms_by_dls` INT(11) NOT NULL DEFAULT '0' AFTER `notif_sms`" );
       DB_Write ( domain, "UPDATE `msgs` SET notif_sms = -1 WHERE notif_sms = 0" );
       DB_Write ( domain, "UPDATE `msgs` SET notif_sms = 2 WHERE notif_sms = 3" );
       DB_Write ( domain, "ALTER TABLE `msgs` ADD `notif_chat` INT(11) NOT NULL DEFAULT '-1' AFTER `notif_sms_by_dls`" );
       DB_Write ( domain, "ALTER TABLE `msgs` ADD `notif_chat_by_dls` INT(11) NOT NULL DEFAULT '1' AFTER `notif_chat`" );
       DB_Write ( domain, "ALTER TABLE `msgs` DROP `groupe`" );
       DB_Write ( domain, "ALTER TABLE `mnemos_AI` CHANGE `archivage` INT(11) NOT NULL DEFAULT '36000'" );
       DB_Write ( domain, "ALTER TABLE `mnemos_AO` CHANGE `archivage` INT(11) NOT NULL DEFAULT '36000'" );
       DB_Write ( domain, "ALTER TABLE `mnemos_REGISTRE` CHANGE `archivage` INT(11) NOT NULL DEFAULT '36000'" );
       DB_Write ( domain, "ALTER TABLE `modbus_AI` CHANGE `archivage` INT(11) NOT NULL DEFAULT '36000'" );
       DB_Write ( domain, "ALTER TABLE `modbus_AO` CHANGE `archivage` INT(11) NOT NULL DEFAULT '36000'" );
     }

    if (db_version<62)
     { DB_Write ( domain, "ALTER TABLE `mnemos_CI` DROP `multi`" ); }

    if (db_version<63)
     { DB_Write ( domain, "ALTER TABLE `tableau` ADD `mode` INT(11) NOT NULL DEFAULT 0" ); }

/*---------------------------------------------------------- Views -----------------------------------------------------------*/
    DB_Write ( domain,
               "CREATE OR REPLACE VIEW threads AS "
               "SELECT agent_uuid, 'teleinfoedf' AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM teleinfoedf UNION "
               "SELECT agent_uuid, 'meteo'       AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM meteo UNION "
               "SELECT agent_uuid, 'shelly'      AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM shelly UNION "
               "SELECT agent_uuid, 'modbus'      AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM modbus UNION "
               "SELECT agent_uuid, 'smsg'        AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM smsg UNION "
               "SELECT agent_uuid, 'audio'       AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM audio UNION "
               "SELECT agent_uuid, 'radio'       AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM radio UNION "
               "SELECT agent_uuid, 'imsgs'       AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM imsgs UNION "
               "SELECT agent_uuid, 'gpiod'       AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM gpiod UNION "
               "SELECT agent_uuid, 'phidget'     AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM phidget UNION "
               "SELECT agent_uuid, 'ups'         AS thread_classe, thread_tech_id, enable, debug, description, heartbeat_time >= NOW() - INTERVAL 60 SECOND AS is_alive FROM ups"
             );

    DB_Write ( domain,
               "CREATE OR REPLACE VIEW dictionnaire AS "
               "SELECT dls_id,           'DLS' AS classe,        tech_id,shortname as acronyme,name as libelle, 'none' as unite FROM dls UNION "
               "SELECT syn_id,           'SYNOPTIQUE' AS classe, page as tech_id, NULL as acronyme,libelle, 'none' as unite FROM syns UNION "
               "SELECT mnemo_ai_id,      'AI' AS classe,         tech_id,acronyme,libelle, unite FROM mnemos_AI UNION "
               "SELECT mnemo_di_id,      'DI' AS classe,         tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_DI UNION "
               "SELECT mnemo_do_id,      'DO' AS classe,         tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_DO UNION "
               "SELECT mnemo_ao_id,      'AO' AS classe,         tech_id,acronyme,libelle, unite FROM mnemos_AO UNION "
               "SELECT mnemo_bi_id,      'BI' AS classe,         tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_BI UNION "
               "SELECT mnemo_mono_id,    'MONO' AS classe,       tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_MONO UNION "
               "SELECT mnemo_ch_id,      'CH' AS classe,         tech_id,acronyme,libelle, '1/10 secondes' as unite FROM mnemos_CH UNION "
               "SELECT mnemo_ci_id,      'CI' AS classe,         tech_id,acronyme,libelle, unite FROM mnemos_CI UNION "
               "SELECT mnemo_horloge_id, 'HORLOGE' AS classe,    tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_HORLOGE UNION "
               "SELECT mnemo_tempo_id,   'TEMPO' AS classe,      tech_id,acronyme,libelle, 'boolean' as unite FROM mnemos_TEMPO UNION "
               "SELECT mnemo_registre_id,'REGISTRE' AS classe,   tech_id,acronyme,libelle, unite FROM mnemos_REGISTRE UNION "
               "SELECT mnemo_visuel_id,  'VISUEL' AS classe,     tech_id,acronyme,libelle, 'none' as unite FROM mnemos_VISUEL UNION "
               "SELECT mnemo_watchdog_id,'WATCHDOG' AS classe,   tech_id,acronyme,libelle, '1/10 secondes' as unite FROM mnemos_WATCHDOG UNION "
               "SELECT tableau_id,       'TABLEAU' AS classe,    NULL AS tech_id, NULL AS acronyme, titre AS libelle, 'none' as unite FROM tableau UNION "
               "SELECT msg_id,           'MESSAGE' AS classe,    tech_id,acronyme,libelle, 'none' as unite FROM msgs UNION "
               "SELECT modbus_id,        'MODBUS' AS classe,     thread_tech_id, NULL AS acronyme, description AS libelle, 'none' as unite FROM modbus "
             );

    DB_Write ( domain,
               "CREATE OR REPLACE VIEW domain_status AS SELECT "
               "(SELECT COUNT(*) FROM syns) AS nbr_syns, "
               "(SELECT COUNT(*) FROM syns_motifs) AS nbr_syns_motifs, "
               "(SELECT COUNT(*) FROM dls) AS nbr_dls, "
               "(SELECT COUNT(*) FROM dls WHERE error_count>0) AS nbr_dls_error, "
               "(SELECT COUNT(*) FROM mnemos_DI) AS nbr_dls_di, "
               "(SELECT COUNT(*) FROM mnemos_DO) AS nbr_dls_do, "
               "(SELECT COUNT(*) FROM mnemos_AI) AS nbr_dls_ai, "
               "(SELECT COUNT(*) FROM mnemos_AO) AS nbr_dls_ao, "
               "(SELECT COUNT(*) FROM mnemos_BI) AS nbr_dls_bi, "
               "(SELECT COUNT(*) FROM mnemos_MONO) AS nbr_dls_mono, "
               "(SELECT SUM(dls.nbr_ligne) FROM dls) AS nbr_dls_lignes, "
               "(SELECT SUM(dls.compil_time) FROM dls) AS dls_compil_time, "
               "(SELECT COUNT(*) FROM agents) AS nbr_agents, "
               "(SELECT COUNT(*) FROM threads) AS nbr_threads, "
               "(SELECT COUNT(*) FROM msgs) AS nbr_dls_msgs, "
               "(SELECT COUNT(*) FROM histo_msgs) AS nbr_histo_msgs, "
               "(SELECT COUNT(*) FROM audit_log) AS nbr_audit_log" );

/*---------------------------------------------------------- Triggers --------------------------------------------------------*/
    DB_Arch_Write ( domain, "DROP TRIGGER IF EXISTS update_status" );
    DB_Arch_Write ( domain,
               "CREATE TRIGGER update_status AFTER INSERT ON histo_bit FOR EACH ROW "
               "INSERT INTO status SET tech_id=NEW.tech_id, acronyme=NEW.acronyme, "
               "date_create=NEW.date_time, last_update=NEW.date_time "
               "ON DUPLICATE KEY UPDATE `rows` = `rows` + 1, last_update=NEW.date_time "
             );

/*-------------------------------------------------------- Opérational -------------------------------------------------------*/
    DB_Write ( domain, "INSERT IGNORE INTO syns SET libelle='Accueil', parent_id=1, page='ACCUEIL', image='syn_maison.png', access_level=0" );
    DB_Write ( domain, "INSERT IGNORE INTO dls  SET tech_id='SYS', syn_id=1, name='Système', shortname='Système'" );

                                                 /* Bit de domaine, non archivés par le master mais par l'API, tous les jours */
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_LIGNE_DLS",    "Nombre de lignes total de tous modules D.L.S", "lignes", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_MOTIFS",       "Nombre de motifs total de tous les synoptiques", "motifs", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_AGENTS",       "Nombre d'agents dans le domaine", "agents", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_THREADS",      "Nombre de threads dans le domaine", "threads", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS",          "Nombre de D.L.S dans le domaine", "dls", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS_ERROR",    "Nombre de D.L.S en erreur dans le domaine", "dls", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS_DI",       "Nombre de DI dans le domaine", "DI", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS_DO",       "Nombre de DO dans le domaine", "DO", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS_AI",       "Nombre de AI dans le domaine", "AI", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS_AO",       "Nombre de AO dans le domaine", "AO", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "NBR_DLS_MSGS",     "Nombre de Messages dans le domaine", "msgs", ARCHIVE_NONE );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "DLS_COMPIL_TIME",  "Temps de compilation total", "1/10 s", ARCHIVE_NONE );

                                                                                    /* Bit du Master, archivage par le master */
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "DLS_BIT_PER_SEC",     "Nombre de changements d'etat par seconde", "/s", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "DLS_TOUR_PER_SEC",    "Nombre de tours par seconde", "/s", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "DLS_WAIT",            "Délai d'attente DLS", "ms", ARCHIVE_1_MIN );
    Mnemo_auto_create_AI_from_thread ( domain, "SYS", "MAXRSS",              "Consommation mémoire", "kb", ARCHIVE_1_MIN );

    Mnemo_auto_create_MONO ( domain, FALSE, "SYS", "TOP_1MIN",         "Impulsion toutes les minutes" );
    Mnemo_auto_create_MONO ( domain, FALSE, "SYS", "TOP_1SEC",         "Impulsion toutes les secondes" );
    Mnemo_auto_create_MONO ( domain, FALSE, "SYS", "TOP_5SEC",         "Impulsion toutes les 5 secondes" );
    Mnemo_auto_create_MONO ( domain, FALSE, "SYS", "TOP_10SEC",        "Impulsion toutes les 10 secondes" );
    Mnemo_auto_create_MONO ( domain, FALSE, "SYS", "TOP_2HZ",          "Impulsion toutes les demi-secondes" );
    Mnemo_auto_create_MONO ( domain, FALSE, "SYS", "TOP_5HZ",          "Impulsion toutes les 1/5 secondes" );
    Mnemo_auto_create_BI   ( domain, FALSE, "SYS", "MQTT_CONNECTED",   "TRUE si l'agent est connecté au MQTT", 0 );
    Mnemo_auto_create_BI   ( domain, FALSE, "SYS", "FLIPFLOP_2SEC",    "Creneaux d'une durée de deux secondes", 0 );
    Mnemo_auto_create_BI   ( domain, FALSE, "SYS", "FLIPFLOP_1SEC",    "Creneaux d'une durée d'une seconde", 0 );
    Mnemo_auto_create_BI   ( domain, FALSE, "SYS", "FLIPFLOP_2HZ",     "Creneaux d'une durée d'une demi seconde", 0 );
    Mnemo_auto_create_BI   ( domain, FALSE, "SYS", "FLIPFLOP_5HZ",     "Creneaux d'une durée d'un 5ième de seconde", 0 );

    Mnemo_auto_create_DI_from_thread ( domain, "SYS", "TOP_ALERTE_1",  "Demande d'alerte" );
    Mnemo_auto_create_DI_from_thread ( domain, "SYS", "TOP_ALERTE_2",  "Demande d'alerte" );

    db_version = DOMAIN_DATABASE_VERSION;
    DB_Write ( DOMAIN_tree_get("master"), "UPDATE domains SET db_version=%d WHERE domain_uuid ='%s'", db_version, domain_uuid );
  }
/******************************************************************************************************************************/
/* DOMAIN_tree_get: Recherche la structure domaine en fonction du nom de l'uuid                                               */
/* Entrée: UUID                                                                                                               */
/* Sortie: struct DOMAIN, or NULL si erreur                                                                                   */
/******************************************************************************************************************************/
 struct DOMAIN *DOMAIN_tree_get ( gchar *domain_uuid )
  { return ( g_tree_lookup ( Global.domaines, domain_uuid ) ); }
/******************************************************************************************************************************/
/* DOMAIN_Load_one: Charge un domaine en mémoire depuis la base de données                                                    */
/* Entrée: Le domaine, sous la forme d'un JSON dans un tableau                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_Load_one ( JsonNode *domaine_config )
  {
    if (!domaine_config)
     { Info_new ( __func__, LOG_ERR, NULL, "No Config. Loading Failed" ); return; }

    gchar *domain_uuid = Json_get_string ( domaine_config, "domain_uuid" );
    if (!domain_uuid)
     { Info_new ( __func__, LOG_ERR, NULL, "No domain_uuid. Loading Failed" ); return; }

    Info_new ( __func__, LOG_INFO, NULL, "Loading '%s' domain", domain_uuid );

    struct DOMAIN *domain = g_try_malloc0 ( sizeof(struct DOMAIN) );
    if (!domain)
     { Info_new ( __func__, LOG_ERR, NULL, "Memory Error. Loading Failed" ); return; }

    pthread_mutexattr_t param;                                                                /* Creation du mutex de synchro */
    pthread_mutexattr_init( &param );                                                         /* Creation du mutex de synchro */
    pthread_mutexattr_setpshared( &param, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &domain->synchro, &param );

    domain->config = json_node_copy ( domaine_config );
    g_tree_insert ( Global.domaines, domain_uuid, domain );                         /* Ajout dans l'arbre global des domaines */

    if (!DB_Pool_init ( domain ))                                          /* Activation de la connexion a la base de données */
     { Info_new ( __func__, LOG_ERR, domain, "DB Connect failed. Domain loaded but DB Query will failed" );
       return;
     }

    if (!DB_Arch_Pool_init ( domain ))                                     /* Activation de la connexion a la base de données */
     { Info_new ( __func__, LOG_ERR, domain, "DB Ach Connect failed. Domain loaded but DB Arch Query will failed" );
       return;
     }

    if (strcasecmp ( domain_uuid, "master" ) )                                         /* si pas dans master -> domain normal */
     { if (Json_get_int ( domain->config, "db_version" )==0)
        { DOMAIN_create_domainDB ( domain );                                                           /* Création du domaine */
          Json_node_add_int ( domain->config, "db_version", DOMAIN_DATABASE_VERSION );
        }
       DOMAIN_update_domainDB ( domain );
       VISUELS_Load_all ( domain );
       DB_Write ( DOMAIN_tree_get("master"), "GRANT SELECT ON TABLE master.icons TO '%s'@'%%'", domain_uuid );
       DB_Write ( DOMAIN_tree_get("master"), "GRANT SELECT ON TABLE master.icons_modes TO '%s'@'%%'", domain_uuid );
     }

    Info_new ( __func__, LOG_NOTICE, domain, "Domain '%s' Loaded", domain_uuid );
  }
/******************************************************************************************************************************/
/* DOMAIN_Load_one_by_array: Charge un domaine en mémoire depuis la base de données                                           */
/* Entrée: Le domaine, sous la forme d'un JSON dans un tableau                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_Load_one_by_array ( JsonArray *array, guint index_, JsonNode *domaine_config, gpointer user_data )
  { DOMAIN_Load_one ( domaine_config ); }

/******************************************************************************************************************************/
/* DOMAIN_Load_all: Charge tous les domaines en mémoire depuis la base de données                                             */
/* Entrée: Le domaine, sous la forme d'un JSON dans un tableau                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_Load_all ( void )
  { JsonNode *RootNode = Json_node_create();
    if (!RootNode)
     { Info_new ( __func__, LOG_ERR, NULL, "Unable to load all Domains: Memory Error" );
       return;
     }

    Info_new( __func__, LOG_INFO, NULL, "Loading All Domains" );
    DB_Read ( DOMAIN_tree_get("master"), RootNode, "domains", "SELECT * FROM domains" );
    Json_node_foreach_array_element ( RootNode, "domains", DOMAIN_Load_one_by_array, NULL );
    Info_new( __func__, LOG_INFO, NULL, "%d Domains loaded", Json_get_int ( RootNode, "nbr_domains" ) );
    json_node_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                                           */
/* Entrée: La DB                                                                                                              */
/******************************************************************************************************************************/
 static gboolean DOMAIN_Unload_one ( gpointer domain_uuid, gpointer value, gpointer user_data )
  { struct DOMAIN *domain = value;
    VISUELS_Unload_all ( domain );
    DB_Pool_end ( domain );
    pthread_mutex_destroy( &domain->synchro );
    Info_new( __func__, LOG_INFO, domain, "Disconnected", domain_uuid );
    g_free(domain_uuid);
    g_free(domain);
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
    Info_new( __func__, LOG_INFO, NULL, "All Domains are Disconnected" );
  }
/******************************************************************************************************************************/
/* DOMAIN_Archive_status_thread: Appelé une fois toutes les 10 minutes pour sauver le statut du domain                        */
/* Entrée: le domaine                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void DOMAIN_Archiver_status_thread ( struct DOMAIN *domain )
  { prctl(PR_SET_NAME, "W-ArchDomainSQL", 0, 0, 0 );

    JsonNode *element = Json_node_create();
    if (!element) return;
    DB_Read ( domain, element, NULL, "SELECT * FROM domain_status" );

    JsonNode *arch = Json_node_create ();
    if (arch)
     { struct timeval tv;
       gettimeofday( &tv, NULL );                                                                /* On prend l'heure actuelle */
       Json_node_add_string ( arch, "tech_id",   "SYS" );
       Json_node_add_int    ( arch, "date_sec",  tv.tv_sec );
       Json_node_add_int    ( arch, "date_usec", tv.tv_usec );

       Json_node_add_string ( arch, "acronyme",  "NBR_MOTIFS" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_syns_motifs" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_AGENTS" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_agents" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_THREADS" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_threads" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS_DI" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_di" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS_DO" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_do" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS_AI" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_ai" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS_AO" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_ao" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS_ERROR" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_error" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_DLS_MSGS" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_msgs" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "NBR_LIGNE_DLS" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "nbr_dls_lignes" ) );
       ARCHIVE_Handle_one ( domain, arch );

       Json_node_add_string ( arch, "acronyme",  "DLS_COMPIL_TIME" );
       Json_node_add_double ( arch, "valeur",    1.0*Json_get_int ( element, "dls_compil_time" ) );
       ARCHIVE_Handle_one ( domain, arch );
       json_node_unref(arch);
     }
    json_node_unref(element);
    pthread_exit(0);
  }
/******************************************************************************************************************************/
/* DOMAIN_Archive_status: Lance l'archivage du statut du domain (pthread) en parametre                                        */
/* Entrée: le gtree                                                                                                           */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean DOMAIN_Archiver_status ( gpointer key, gpointer value, gpointer data )
  { pthread_t TID;
    struct DOMAIN *domain = value;

    if(!strcasecmp ( key, "master" )) return(FALSE);                                    /* Pas d'archive sur le domain master */

    if ( pthread_create( &TID, NULL, (void *)DOMAIN_Archiver_status_thread, domain ) )
     { Info_new( __func__, LOG_ERR, domain, "Error while pthreading ARCHIVE_Delete_old_data_thread: %s", strerror(errno) ); }
    return(FALSE); /* False = on continue */
  }
/******************************************************************************************************************************/
/* DOMAIN_LIST_request_get: Envoi la liste des domaines d'un utilisateur                                                      */
/* Entrées: le domain source, le token user, le msg libsoup et la request json                                                */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void DOMAIN_LIST_request_get ( JsonNode *token,  SoupServerMessage *msg )
  { /*if (!Http_is_authorized ( domain, token, path, msg, 0 )) return;*/
    Http_print_request ( NULL, token, "/domain/list" );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gboolean retour = DB_Read ( master, RootNode, "domains",
                                "SELECT domain_uuid, domain_name, image, access_level FROM domains "
                                "INNER JOIN users_grants USING(domain_uuid) "
                                "WHERE user_uuid='%s'",
                                Json_get_string ( token, "sub" )
                              );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* DOMAIN_GET_request_post: Appelé depuis libsoup pour éditer un domaine                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_GET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (Http_fail_if_has_not ( domain, path, msg, url_param, "domain_uuid")) return;

    gchar         *domain_uuid   = Json_get_string ( url_param, "domain_uuid" );
    struct DOMAIN *search_domain = DOMAIN_tree_get ( domain_uuid );
    struct DOMAIN *master        = DOMAIN_tree_get ("master");

    if (!search_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: domain_uuid not found. Bad Request", path );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Domaine non trouvé", NULL );
       return;
     }

    if (!Http_is_authorized ( search_domain, token, path, msg, 6 )) return;
    Http_print_request ( search_domain, token, path );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( master, RootNode, NULL,
                                "SELECT d.domain_uuid, d.domain_name, d.date_create, d.image, d.domain_secret, "
                                "d.debug_dls, d.audio_tech_id, d.notif, g.access_level "
                                "FROM domains AS d INNER JOIN users_grants AS g USING(domain_uuid) "
                                "WHERE g.user_uuid = '%s' AND d.domain_uuid='%s'",
                                Json_get_string ( token, "sub" ), Json_get_string ( search_domain->config, "domain_uuid" ) );
    Json_node_add_string ( RootNode, "api_url", Json_get_string ( Global.config, "api_public_url" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* DOMAIN_SET_request_post: Appelé depuis libsoup pour éditer un domaine                                                      */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_SET_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "domain_uuid"))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "domain_name"))   return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "debug_dls"))     return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "notif"))         return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "audio_tech_id")) return;

    gchar *domain_uuid    = Json_get_string ( request, "domain_uuid" );
    struct DOMAIN *target_domain = DOMAIN_tree_get ( domain_uuid );

    if (!target_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: domain_uuid does not exists or not connected. Bad Request", path );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Domaine non trouvé", NULL );
       return;
     }

    if (!Http_is_authorized ( target_domain, token, path, msg, 8 )) return;
    Http_print_request ( target_domain, token, path );

    gchar *domain_name   = Normaliser_chaine ( Json_get_string ( request, "domain_name" ) );
    gchar *notif         = Normaliser_chaine ( Json_get_string ( request, "notif" ) );
    gchar *audio_tech_id = Normaliser_chaine ( Json_get_string ( request, "audio_tech_id" ) );
    gboolean debug_dls   = Json_get_bool ( request, "debug_dls" );

    gboolean retour = DB_Write ( DOMAIN_tree_get ("master"),
                                 "UPDATE domains SET domain_name='%s', notif='%s', debug_dls=%d, audio_tech_id=UPPER('%s') "
                                 "WHERE domain_uuid='%s'", domain_name, notif, debug_dls, audio_tech_id, domain_uuid );
    g_free(domain_name);
    g_free(notif);
    g_free(audio_tech_id);
                                                                                         /* Recopie en live dans la structure */
    Json_node_add_string ( target_domain->config, "domain_name", Json_get_string ( request, "domain_name" ) );
    Json_node_add_bool   ( target_domain->config, "debug_dls", debug_dls );

    if (!retour) { Http_Send_json_response ( msg, retour, DOMAIN_tree_get("master")->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Domain changed", NULL );
  }
/******************************************************************************************************************************/
/* DOMAIN_ADD_request_post: Créé un domaine à l'utilisateur                                                                   */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_ADD_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  {
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    /* ToDo : vérif le num max de domaine autorisé */
    /*if (!Http_is_authorized ( NULL, token, path, msg, 6 )) return;*/
    Http_print_request ( NULL, token, path );

    gchar new_domain_uuid[37];
    UUID_New ( new_domain_uuid );

    gchar new_password_bin[48];
    RAND_bytes( new_password_bin, sizeof(new_password_bin) );
    gchar new_password[sizeof(new_password_bin)*4/3+1];
    EVP_EncodeBlock ( new_password, new_password_bin, sizeof(new_password_bin) );

    gboolean retour = DB_Write ( master,
                                 "INSERT INTO domains SET domain_uuid = '%s', "
                                 "domain_secret=SHA2(RAND(), 512), "
                                 "mqtt_password=SHA2(RAND(), 512), "
                                 "browser_password=SHA2(RAND(), 512), "
                                 "db_password='%s' ",
                                 new_domain_uuid, new_password );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

    retour = DB_Write ( master,
                        "INSERT INTO users_grants SET domain_uuid = '%s', user_uuid='%s', access_level='9' ",
                        new_domain_uuid, Json_get_string ( token, "sub" ) );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

/************************************************** Create new domain database ************************************************/
    retour = DB_Write ( master, "CREATE DATABASE `%s` CHARACTER SET 'utf8' COLLATE 'utf8_unicode_ci'", new_domain_uuid );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

/************************************************** Create new user of domain database ****************************************/
    retour = DB_Write ( master, "GRANT ALL PRIVILEGES ON `%s`.* TO '%s'@'%' IDENTIFIED BY '%s'",
                        new_domain_uuid, new_domain_uuid, new_password );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

/************************************************** Create new arch database si les serveurs SGBD sont différents *************/
    if ( strcmp ( Json_get_string ( Global.config, "db_hostname" ), Json_get_string ( Global.config, "db_arch_hostname" ) ) ||
         Json_get_int ( Global.config, "db_port" ) != Json_get_int ( Global.config, "db_arch_port" )
       )
     { retour = DB_Arch_Write ( master, "CREATE DATABASE `%s` CHARACTER SET 'utf8' COLLATE 'utf8_unicode_ci'", new_domain_uuid );
       if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
/************************************************** Create new user of arch database ******************************************/
       retour = DB_Arch_Write ( master, "GRANT ALL PRIVILEGES ON `%s`.* TO '%s'@'%' IDENTIFIED BY '%s'",
                                new_domain_uuid, new_domain_uuid, new_password );
       if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
     }

/************************************************** Load new domain ***********************************************************/
    JsonNode *RootNode = Json_node_create ();
    if (!RootNode) return;
    retour = DB_Read ( master, RootNode, NULL, "SELECT * FROM domains WHERE domain_uuid='%s'", new_domain_uuid );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); json_node_unref(RootNode); return; }

    DOMAIN_Load_one ( RootNode );
    MQTT_Allow_one_domain ( DOMAIN_tree_get ( new_domain_uuid ) );

    JsonNode *Response = Http_json_node_create ( msg );
    Json_node_add_string ( Response, "domain_uuid", Json_get_string ( RootNode, "domain_uuid" ) );
    Json_node_add_string ( Response, "domain_name", Json_get_string ( RootNode, "domain_name" ) );
    json_node_unref(RootNode);

    Info_new ( __func__, LOG_NOTICE, NULL, "Domain '%s' created", new_domain_uuid );
    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Domain created", Response );
  }
/******************************************************************************************************************************/
/* DOMAIN_TRANSFER_request_post: Transfert un domain                                                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_TRANSFER_request_post ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( NULL, path, msg, request, "domain_uuid")) return;
    if (Http_fail_if_has_not ( NULL, path, msg, request, "new_owner_email"))    return;

    gchar *domain_uuid    = Json_get_string ( request, "domain_uuid" );
    struct DOMAIN *target_domain = DOMAIN_tree_get ( domain_uuid );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    if (!target_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: target_domain not found.", path );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "Domaine non trouvé", NULL );
       return;
     }

    if (!Http_is_authorized ( target_domain, token, path, msg, 9 )) return;
    Http_print_request ( target_domain, token, path );

    JsonNode *RootNode = Http_json_node_create ( msg );
    if (!RootNode) return;

    gchar *new_owner_email = Normaliser_chaine ( Json_get_string ( request, "new_owner_email" ) );
    gboolean retour = DB_Read ( master, RootNode, NULL,
                                "SELECT user_uuid AS new_user_uuid FROM users WHERE email='%s'", new_owner_email );
    g_free(new_owner_email);
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }

    if (!Json_has_member( RootNode, "new_user_uuid" ))
     { Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "New user not found", RootNode ); return; }
    if (!strcmp ( Json_get_string ( token, "sub" ), Json_get_string ( RootNode, "new_user_uuid" ) ) )
     { Http_Send_json_response ( msg, SOUP_STATUS_BAD_REQUEST, "Vous etes déja le propriétaire de ce domaine", NULL );
       return;
     }
    retour  = DB_Write ( master,
                        "INSERT INTO users_grants SET user_uuid='%s', domain_uuid='%s', access_level=9 "
                        "ON DUPLICATE KEY UPDATE access_level=VALUES(access_level)",
                        Json_get_string ( RootNode, "new_user_uuid" ), domain_uuid );

    retour &= DB_Write ( master,
                        "DELETE FROM users_grants WHERE user_uuid='%s', domain_uuid='%s'",
                        Json_get_string ( token, "sub" ), domain_uuid );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* DOMAIN_DELETE_request: Supprime un domain                                                                                  */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_DELETE_request ( JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( NULL, path, msg, request, "domain_uuid")) return;

    gchar *domain_uuid    = Json_get_string ( request, "domain_uuid" );
    struct DOMAIN *target_domain = DOMAIN_tree_get ( domain_uuid );

    if (!target_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: domain_uuid does not exists or not connected. Bad Request", path );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "target_domain does not exists", NULL );
       return;
     }

    if (!Http_is_authorized ( target_domain, token, path, msg, 9 )) return;
    Http_print_request ( target_domain, token, path );

    struct DOMAIN *master = DOMAIN_tree_get ("master");
    gboolean retour = DB_Write ( master, "DELETE FROM domains WHERE domain_uuid='%s'", domain_uuid );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

/************************************************** Delete domain database ****************************************************/
    retour = DB_Write ( master, "DROP DATABASE IF EXISTS `%s`", domain_uuid );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

/************************************************** Delete domain database ****************************************************/
    retour = DB_Write ( master, "DROP USER IF EXISTS `%s`", domain_uuid );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }

/************************************************** Delete new arch database si les serveurs SGBD sont différents *************/
    if ( strcmp ( Json_get_string ( Global.config, "db_hostname" ), Json_get_string ( Global.config, "db_arch_hostname" ) ) ||
         Json_get_int ( Global.config, "db_port" ) != Json_get_int ( Global.config, "db_arch_port" )
       )
     { retour = DB_Arch_Write ( master, "DROP DATABASE `%s`", domain_uuid );
       if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
/************************************************** Delete  user of arch database *********************************************/
       retour = DB_Arch_Write ( master, "DROP USER IF EXISTS `%s`", domain_uuid );
       if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
     }

    Info_new ( __func__, LOG_NOTICE, NULL, "Domain '%s' deleted", domain_uuid );

    Http_Send_json_response ( msg, SOUP_STATUS_OK, "Domain deleted", NULL );
  }
/******************************************************************************************************************************/
/* DOMAIN_SET_IMAGE_request_post: Change l'image du domain                                                                    */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_SET_IMAGE_request_post ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (Http_fail_if_has_not ( domain, path, msg, request, "domain_uuid")) return;
    if (Http_fail_if_has_not ( domain, path, msg, request, "image"))       return;

    gchar *domain_uuid    = Json_get_string ( request, "domain_uuid" );
    struct DOMAIN *target_domain = DOMAIN_tree_get ( domain_uuid );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    if (!target_domain)
     { Info_new ( __func__, LOG_WARNING, NULL, "%s: domain_uuid does not exists or not connected. Bad Request", path );
       Http_Send_json_response ( msg, SOUP_STATUS_NOT_FOUND, "target domain does not exists", NULL );
       return;
     }

    if (!Http_is_authorized ( target_domain, token, path, msg, 8 )) return;
    Http_print_request ( target_domain, token, path );

    gchar *image = Normaliser_chaine ( Json_get_string ( request, "image" ) );

    gboolean retour = DB_Write ( master,
                                 "UPDATE domains SET image='%s' "
                                 "WHERE domain_uuid='%s'", image, domain_uuid );
    g_free(image);
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, NULL ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, NULL );
  }
/******************************************************************************************************************************/
/* DOMAIN_STATUS_request_post: Appelé depuis libsoup pour l'URI domain_status                                                 */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_STATUS_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *url_param )
  {
    if (!Http_is_authorized ( domain, token, path, msg, 0 )) return;
    Http_print_request ( domain, token, path );
    struct DOMAIN *master = DOMAIN_tree_get ("master");

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    gboolean retour = DB_Read ( domain, RootNode, NULL, "SELECT * FROM domain_status" );
    gchar *domain_uuid = Json_get_string ( domain->config, "domain_uuid" );
    retour &= DB_Read ( master, RootNode, NULL,
                        "SELECT COUNT(*) AS nbr_users FROM users_grants WHERE domain_uuid='%s'", domain_uuid );

    Json_node_add_int    ( RootNode, "nbr_visuels", domain->Nbr_visuels );
    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* DOMAIN_IMAGE_request_get: Retourne l'image d'un domaine, au format base64 en json                                          */
/* Entrée: Les paramètres libsoup                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void DOMAIN_IMAGE_request_get ( struct DOMAIN *domain, JsonNode *token, const char *path, SoupServerMessage *msg, JsonNode *request )
  { if (!Http_is_authorized ( domain, token, path, msg, 0 )) return;
    Http_print_request ( domain, token, path );

    SoupMessageHeaders *headers = soup_server_message_get_response_headers ( msg );
    soup_message_headers_append ( headers, "Cache-Control", "max-age=120, public" );

    JsonNode *RootNode = Http_json_node_create (msg);
    if (!RootNode) return;

    struct DOMAIN *master = DOMAIN_tree_get ("master");
    gboolean retour = DB_Read ( master, RootNode, NULL,
                                "SELECT domain_uuid, image FROM domains WHERE domain_uuid = '%s'",
                                Json_get_string ( domain->config, "domain_uuid" ) );

    if (!retour) { Http_Send_json_response ( msg, retour, master->mysql_last_error, RootNode ); return; }
    Http_Send_json_response ( msg, SOUP_STATUS_OK, NULL, RootNode );
  }
/******************************************************************************************************************************/
/* DOMAIN_Daily_update: Lance le menage (pthread) dans les archives du domaine en parametre issu du g_tree                   */
/* Entrée: le gtree                                                                                                           */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean DOMAIN_Daily_update ( gpointer key, gpointer value, gpointer data )
  { struct DOMAIN *domain = value;

    if(!strcasecmp ( key, "master" )) return(FALSE);                                    /* Pas d'archive sur le domain master */

    gint days = Json_get_int    ( domain->config, "archive_retention" );
    Info_new( __func__, LOG_NOTICE, domain, "Starting ARCHIVE_Daily_update with days=%d", days );

    ARCHIVE_Daily_update ( key, value, data );

    DB_Write ( domain, "INSERT INTO cleanup SET archive = 0, "
                       "requete=\"UPDATE histo_msgs "
                       "LEFT JOIN msgs ON histo_msgs.tech_id = msgs.tech_id AND histo_msgs.acronyme = msgs.acronyme "
                       "SET date_fin=NOW() WHERE histo_msgs.date_fin IS NULL AND msgs.tech_id IS NULL\"" );

    return(FALSE); /* False = on continue */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
