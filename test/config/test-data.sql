-- =============================================================================
-- ABLS-Habitat API - Fixtures de test
-- Base principale: master
-- =============================================================================
-- Utilisation: mariadb -h 127.0.0.1 -P 13306 -u abls_test -pabls_test_pass master < test-data.sql
-- =============================================================================

CREATE DATABASE IF NOT EXISTS `master` CHARACTER SET utf8 COLLATE utf8_unicode_ci;
USE master;

-- Droits niveau master pour l'user de test
GRANT ALL PRIVILEGES ON `master`.* TO 'abls_test'@'%';
GRANT SUPER ON *.* TO 'abls_test'@'%';
FLUSH PRIVILEGES;

-- =============================================================================
-- TABLE: database_version (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `database_version` (
  `date`    DATETIME NOT NULL DEFAULT NOW(),
  `version` INT(11)  NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

INSERT IGNORE INTO `database_version` (`version`) VALUES (100);

-- =============================================================================
-- TABLE: domains (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `domains` (
  `domain_id`             INT(11)       PRIMARY KEY AUTO_INCREMENT,
  `domain_uuid`           VARCHAR(37)   UNIQUE NOT NULL,
  `domain_secret`         VARCHAR(128)  NOT NULL DEFAULT 'secret',
  `date_create`           DATETIME      NOT NULL DEFAULT NOW(),
  `domain_name`           VARCHAR(256)  NOT NULL DEFAULT 'My new domain',
  `db_password`           VARCHAR(64)   NULL,
  `db_version`            INT(11)       NOT NULL DEFAULT '0',
  `mqtt_password`         VARCHAR(128)  NOT NULL DEFAULT 'passwd',
  `browser_password`      VARCHAR(128)  NOT NULL DEFAULT 'passwd',
  `archive_hot_retention` INT(11)       NOT NULL DEFAULT 18,
  `archive_cold_retention`INT(11)       NOT NULL DEFAULT 2,
  `debug_dls`             BOOLEAN       NOT NULL DEFAULT 0,
  `audio_tech_id`         VARCHAR(32)   NOT NULL DEFAULT 'AUDIO',
  `git_repo_url`          VARCHAR(256)  NOT NULL DEFAULT '',
  `git_repo_token`        VARCHAR(128)  NOT NULL DEFAULT '',
  `mistral_api_key`       VARCHAR(128)  NOT NULL DEFAULT '',
  `image`                 MEDIUMTEXT    NULL,
  `notif_info`            VARCHAR(256)  NOT NULL DEFAULT '',
  `notif_warning`         VARCHAR(256)  NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- Domaine de test
INSERT IGNORE INTO `domains`
  (`domain_uuid`, `domain_secret`, `domain_name`, `db_password`, `db_version`, `mqtt_password`, `browser_password`)
VALUES
  ('aaaaaaaa-0000-0000-0000-000000000001', 'test-domain-secret-001', 'Domaine de Test Principal', 'test-domain-db-pass-001', 91, 'mqtt_test_001', 'browser_test_001');

-- =============================================================================
-- TABLE: users (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `users` (
  `user_uuid`           VARCHAR(37)   PRIMARY KEY,
  `default_domain_uuid` VARCHAR(37)   NULL,
  `date_create`         DATETIME      NOT NULL DEFAULT NOW(),
  `email`               VARCHAR(128)  COLLATE utf8_unicode_ci NOT NULL,
  `username`            VARCHAR(64)   COLLATE utf8_unicode_ci NOT NULL,
  `phone`               VARCHAR(80)   COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `xmpp`                VARCHAR(80)   COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `enable`              BOOLEAN       NOT NULL DEFAULT '0',
  `free_sms_api_user`   VARCHAR(64)   COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `free_sms_api_key`    VARCHAR(64)   COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  CONSTRAINT `fk_default_domain_uuid` FOREIGN KEY (`default_domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- User admin (access_level=9)
INSERT IGNORE INTO `users` (`user_uuid`, `default_domain_uuid`, `email`, `username`, `enable`)
VALUES ('bbbbbbbb-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 'admin@test.abls-habitat.fr', 'Admin Test', 1);

-- User standard (access_level=6)
INSERT IGNORE INTO `users` (`user_uuid`, `default_domain_uuid`, `email`, `username`, `enable`)
VALUES ('cccccccc-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 'user@test.abls-habitat.fr', 'User Test', 1);

-- User sans droits (access_level=1)
INSERT IGNORE INTO `users` (`user_uuid`, `default_domain_uuid`, `email`, `username`, `enable`)
VALUES ('dddddddd-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 'readonly@test.abls-habitat.fr', 'Readonly Test', 1);

-- User désactivé
INSERT IGNORE INTO `users` (`user_uuid`, `default_domain_uuid`, `email`, `username`, `enable`)
VALUES ('eeeeeeee-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 'disabled@test.abls-habitat.fr', 'Disabled Test', 0);

-- =============================================================================
-- TABLE: users_invite (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `users_invite` (
  `user_invite_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `email`          VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,
  `domain_uuid`    VARCHAR(37)  NOT NULL,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `access_level`   INT(11)      NOT NULL DEFAULT '1',
  UNIQUE (`email`, `domain_uuid`),
  CONSTRAINT `fk_users_invite_domain_uuid` FOREIGN KEY (`domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: users_grants (master) - droits par domaine
-- =============================================================================
CREATE TABLE IF NOT EXISTS `users_grants` (
  `grant_id`           INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `user_uuid`          VARCHAR(37) NOT NULL,
  `domain_uuid`        VARCHAR(37) NOT NULL,
  `access_level`       INT(11)     NOT NULL DEFAULT '6',
  `can_send_txt_cde`   BOOLEAN     NOT NULL DEFAULT '0',
  `wanna_be_notified`  BOOLEAN     NOT NULL DEFAULT '0',
  UNIQUE (`user_uuid`, `domain_uuid`),
  CONSTRAINT `fk_users_grants_user_uuid`   FOREIGN KEY (`user_uuid`)   REFERENCES `users`   (`user_uuid`)   ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_users_grants_domain_uuid` FOREIGN KEY (`domain_uuid`) REFERENCES `domains` (`domain_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

INSERT IGNORE INTO `users_grants` (`user_uuid`, `domain_uuid`, `access_level`)
VALUES ('bbbbbbbb-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 9);  -- admin
INSERT IGNORE INTO `users_grants` (`user_uuid`, `domain_uuid`, `access_level`)
VALUES ('cccccccc-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 6);  -- user
INSERT IGNORE INTO `users_grants` (`user_uuid`, `domain_uuid`, `access_level`)
VALUES ('dddddddd-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 1);  -- readonly
INSERT IGNORE INTO `users_grants` (`user_uuid`, `domain_uuid`, `access_level`)
VALUES ('eeeeeeee-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 6);  -- disabled user

-- =============================================================================
-- TABLE: icons et icons_modes (master, minimaliste)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `icons` (
  `icon_id`       INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `categorie`     VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `forme`         VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `extension`     VARCHAR(4)  NOT NULL DEFAULT 'svg',
  `controle`      VARCHAR(32) NOT NULL DEFAULT 'static',
  `default_mode`  VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `default_color` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `date_create`   DATETIME    NOT NULL DEFAULT NOW()
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `icons_modes` (
  `icon_mode_id` INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `forme`        VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `mode`         VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `date_create`  DATETIME    NOT NULL DEFAULT NOW(),
  CONSTRAINT `fk_icons_modes_forme` FOREIGN KEY (`forme`) REFERENCES `icons` (`forme`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: users_gps (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `users_gps` (
  `user_gps_id` INT(11)     NOT NULL AUTO_INCREMENT PRIMARY KEY,
  `user_uuid`   VARCHAR(37) NOT NULL,
  `latitude`    FLOAT       NOT NULL DEFAULT 0,
  `longitude`   FLOAT       NOT NULL DEFAULT 0,
  `date_time`   DATETIME    NOT NULL DEFAULT NOW(),
  CONSTRAINT `fk_users_gps_user_uuid` FOREIGN KEY (`user_uuid`) REFERENCES `users` (`user_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- BASE DOMAINE: aaaaaaaa-0000-0000-0000-000000000001
-- (l'API créé la DB du domaine avec le nom basé sur domain_uuid)
-- =============================================================================
CREATE DATABASE IF NOT EXISTS `aaaaaaaa-0000-0000-0000-000000000001`
  CHARACTER SET utf8 COLLATE utf8_unicode_ci;

-- L'API se connecte au domaine avec user=domain_uuid, password=domains.db_password
CREATE USER IF NOT EXISTS 'aaaaaaaa-0000-0000-0000-000000000001'@'%' IDENTIFIED BY 'test-domain-db-pass-001';

-- Permettre l'accès complet à cette BD domaine
GRANT ALL ON `aaaaaaaa-0000-0000-0000-000000000001`.* TO 'abls_test'@'%';
GRANT ALL ON `aaaaaaaa-0000-0000-0000-000000000001`.* TO 'aaaaaaaa-0000-0000-0000-000000000001'@'%';
FLUSH PRIVILEGES;

USE `aaaaaaaa-0000-0000-0000-000000000001`;

-- Table principale des synoptiques (nécessaire pour les contraintes FK de dls)
CREATE TABLE IF NOT EXISTS `syns` (
  `syn_id`                 INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`            DATETIME     NOT NULL DEFAULT NOW(),
  `parent_id`              INT(11)      NOT NULL,
  `libelle`                VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,
  `image`                  MEDIUMTEXT   COLLATE utf8_unicode_ci NULL,
  `page`                   VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `place`                  INT(11)      NOT NULL DEFAULT '0',
  `access_level`           INT(11)      NOT NULL DEFAULT '0',
  `mode_affichage`         BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSA_DEFAUT`           BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSA_DEFAUT_FIXE`      BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSA_ALARME`           BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSA_ALARME_FIXE`      BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSB_ALERTE`          BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSB_ALERTE_FIXE`     BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSB_VEILLE`          BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSP_DANGER`          BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSP_DANGER_FIXE`     BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSP_DERANGEMENT`     BOOLEAN      NOT NULL DEFAULT '0',
  `MEMSSP_DERANGEMENT_FIXE` BOOLEAN     NOT NULL DEFAULT '0',
  CONSTRAINT `fk_syns_parent_id` FOREIGN KEY (`parent_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT IGNORE INTO `syns` (`syn_id`, `parent_id`, `libelle`, `page`, `access_level`)
VALUES (1, 1, 'Accueil', 'HOME', 0);

-- =============================================================================
-- TABLE: agents (domaine) - schema issu de domains.c
-- =============================================================================
CREATE TABLE IF NOT EXISTS `agents` (
  `agent_id`       INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `agent_uuid`     VARCHAR(37)  UNIQUE NOT NULL,
  `agent_hostname` VARCHAR(64)  NOT NULL,
  `headless`       BOOLEAN      NOT NULL DEFAULT '1',
  `is_master`      BOOLEAN      NOT NULL DEFAULT 0,
  `log_msrv`       BOOLEAN      NOT NULL DEFAULT 0,
  `log_bus`        BOOLEAN      NOT NULL DEFAULT 0,
  `log_dls`        BOOLEAN      NOT NULL DEFAULT 0,
  `log_level`      INT(11)      NOT NULL DEFAULT 6,
  `start_time`     DATETIME     DEFAULT NOW(),
  `install_time`   DATETIME     DEFAULT NOW(),
  `heartbeat_time` DATETIME     DEFAULT NOW(),
  `description`    VARCHAR(128) NOT NULL DEFAULT '',
  `version`        VARCHAR(32)  NOT NULL DEFAULT 'none',
  `branche`        VARCHAR(32)  NOT NULL DEFAULT 'none'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1;

INSERT IGNORE INTO `agents` (`agent_uuid`, `agent_hostname`, `description`)
VALUES ('ffffffff-0000-0000-0000-000000000001', 'test-agent-host', 'Agent de test fonctionnel');

-- =============================================================================
-- TABLE: servers
-- =============================================================================
CREATE TABLE IF NOT EXISTS `servers` (
  `server_uuid`     VARCHAR(37)  PRIMARY KEY NOT NULL,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `server_hostname` VARCHAR(64)  NOT NULL,
  `headless`        BOOLEAN      NOT NULL DEFAULT '1',
  `is_master`       BOOLEAN      NOT NULL DEFAULT 0,
  `description`     VARCHAR(128) NOT NULL DEFAULT '',
  `heartbeat_time`  DATETIME     DEFAULT NOW(),
  `start_time`      DATETIME     DEFAULT NOW(),
  `version`         VARCHAR(32)  NOT NULL DEFAULT 'none'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci;

INSERT IGNORE INTO `servers`
  (`server_uuid`, `date_create`, `server_hostname`, `headless`, `is_master`,
   `description`, `heartbeat_time`, `start_time`, `version`)
SELECT `agent_uuid`, IFNULL(`install_time`, NOW()), `agent_hostname`, `headless`, `is_master`,
       `description`, IFNULL(`heartbeat_time`, NOW()), IFNULL(`start_time`, NOW()), `version`
FROM `agents`;

-- =============================================================================
-- TABLE: log_facilities + agent_log_facilities
-- =============================================================================
CREATE TABLE IF NOT EXISTS `log_facilities` (
  `log_facility_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `log_facility`    VARCHAR(64)  COLLATE utf8_unicode_ci UNIQUE NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

INSERT IGNORE INTO `log_facilities` (`log_facility`) VALUES
('api_config'),
('http'),
('json'),
('local_config'),
('log'),
('mnemo'),
('mqtt'),
('mqtt_api'),
('mqtt_local');

CREATE TABLE IF NOT EXISTS `agent_log_facilities` (
  `agent_log_facility_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `agent_tech_id`         VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `log_facility`          VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  UNIQUE (`agent_tech_id`, `log_facility`),
  CONSTRAINT `fk_agent_log_facilities_log_facility` FOREIGN KEY (`log_facility`) REFERENCES `log_facilities` (`log_facility`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: teleinfoedf
-- =============================================================================
CREATE TABLE IF NOT EXISTS `teleinfoedf` (
  `teleinfoedf_id`  INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time`  DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected`  BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`      VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`     VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My Teleinfo EDF',
  `enable`          BOOLEAN      NOT NULL DEFAULT '1',
  `debug`           BOOLEAN      NOT NULL DEFAULT 0,
  `port`            VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `standard`        BOOLEAN      NOT NULL DEFAULT '0',
  CONSTRAINT `fk_teleinfoedf_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: ups
-- =============================================================================
CREATE TABLE IF NOT EXISTS `ups` (
  `ups_id`          INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time`  DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected`  BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`      VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`     VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My UPS',
  `enable`          BOOLEAN      NOT NULL DEFAULT '1',
  `debug`           BOOLEAN      NOT NULL DEFAULT 0,
  `host`            VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `name`            VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `admin_username`  VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `admin_password`  VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  CONSTRAINT `fk_ups_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1;

-- =============================================================================
-- TABLE: meteo
-- =============================================================================
CREATE TABLE IF NOT EXISTS `meteo` (
  `meteo_id`       INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My Meteo',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  `token`          VARCHAR(65)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `code_insee`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  CONSTRAINT `fk_meteo_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: modbus + sous-tables
-- =============================================================================
CREATE TABLE IF NOT EXISTS `modbus` (
  `modbus_id`           INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`         DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time`      DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected`      BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`          VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id`      VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`         VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My WAGO',
  `enable`              BOOLEAN      NOT NULL DEFAULT '1',
  `debug`               BOOLEAN      NOT NULL DEFAULT 0,
  `hostname`            VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `watchdog`            INT(11)      NOT NULL DEFAULT 50,
  `max_request_par_sec` INT(11)      NOT NULL DEFAULT 50,
  CONSTRAINT `fk_modbus_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `modbus_DI` (
  `modbus_di_id`    INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `thread_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `thread_acronyme` VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `num`             INT(11)      NOT NULL DEFAULT 0,
  `libelle`         VARCHAR(128) NOT NULL DEFAULT '',
  `borne`           VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `ed`              VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `flip`            BOOLEAN      NOT NULL DEFAULT 0,
  `archivage`       INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`thread_tech_id`, `thread_acronyme`),
  CONSTRAINT `fk_modbus_di_thread_tech_id` FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `modbus_DO` (
  `modbus_do_id`    INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `thread_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `thread_acronyme` VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `num`             INT(11)      NOT NULL DEFAULT 0,
  `libelle`         VARCHAR(128) NOT NULL DEFAULT '',
  `borne`           VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `ed`              VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `archivage`       INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`thread_tech_id`, `thread_acronyme`),
  CONSTRAINT `fk_modbus_do_thread_tech_id` FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `modbus_AI` (
  `modbus_ai_id`    INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `thread_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `thread_acronyme` VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `num`             INT(11)      NOT NULL DEFAULT 0,
  `type_borne`      INT(11)      NOT NULL DEFAULT 0,
  `min`             FLOAT        NOT NULL DEFAULT 0,
  `max`             FLOAT        NOT NULL DEFAULT 100,
  `libelle`         VARCHAR(128) NOT NULL DEFAULT '',
  `borne`           VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `ed`              VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `unite`           VARCHAR(32)  NOT NULL DEFAULT '',
  `archivage`       INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`thread_tech_id`, `thread_acronyme`),
  CONSTRAINT `fk_modbus_ai_thread_tech_id` FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `modbus_AO` (
  `modbus_ao_id`    INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `thread_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `thread_acronyme` VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `num`             INT(11)      NOT NULL DEFAULT 0,
  `type_borne`      INT(11)      NOT NULL DEFAULT 0,
  `min`             FLOAT        NOT NULL DEFAULT 0,
  `max`             FLOAT        NOT NULL DEFAULT 100,
  `libelle`         VARCHAR(128) NOT NULL DEFAULT '',
  `borne`           VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `ed`              VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `unite`           VARCHAR(32)  NOT NULL DEFAULT '',
  `archivage`       INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`thread_tech_id`, `thread_acronyme`),
  CONSTRAINT `fk_modbus_ao_thread_tech_id` FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: shelly
-- =============================================================================
CREATE TABLE IF NOT EXISTS `shelly` (
  `shelly_id`      INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My new shelly',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  `string_id`      VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'My new shelly',
  `hostname`       VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  CONSTRAINT `fk_shelly_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: smsg
-- =============================================================================
CREATE TABLE IF NOT EXISTS `smsg` (
  `smsg_id`                INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`            DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time`         DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected`         BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`             VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id`         VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`            VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `enable`                 BOOLEAN      NOT NULL DEFAULT '1',
  `debug`                  BOOLEAN      NOT NULL DEFAULT 0,
  `ovh_service_name`       VARCHAR(16)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `ovh_application_key`    VARCHAR(33)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `ovh_application_secret` VARCHAR(33)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `ovh_consumer_key`       VARCHAR(33)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  CONSTRAINT `fk_smsg_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: audio + audio_zones + audio_zone_map
-- =============================================================================
CREATE TABLE IF NOT EXISTS `audio` (
  `audio_id`       INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  `language`       VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fr',
  `device`         VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `volume`         INT(11)      NOT NULL DEFAULT 100,
  CONSTRAINT `fk_audio_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `audio_zones` (
  `audio_zone_id`   INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `audio_zone_name` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `description`     VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

INSERT IGNORE INTO `audio_zones` (`audio_zone_id`, `audio_zone_name`, `description`)
VALUES (1, 'ZD_NONE', 'Par défaut, zone sans aucune diffusion');

CREATE TABLE IF NOT EXISTS `audio_zone_map` (
  `audio_zone_map_id` INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `date_create`       DATETIME    NOT NULL DEFAULT NOW(),
  `audio_zone_id`     INT(11)     NOT NULL,
  `thread_tech_id`    VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  UNIQUE `uk_audio_zone_id_thread_tech_id` (`audio_zone_id`, `thread_tech_id`),
  CONSTRAINT `fk_audio_zone_map_audio_zone_id`  FOREIGN KEY (`audio_zone_id`)  REFERENCES `audio_zones` (`audio_zone_id`)  ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_audio_zone_map_thread_tech_id` FOREIGN KEY (`thread_tech_id`) REFERENCES `audio`       (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: radio
-- =============================================================================
CREATE TABLE IF NOT EXISTS `radio` (
  `radio_id`       INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  CONSTRAINT `fk_radio_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: dmx
-- =============================================================================
CREATE TABLE IF NOT EXISTS `dmx` (
  `dmx_id`         INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  `device`         VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  CONSTRAINT `fk_dmx_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: imsgs
-- =============================================================================
CREATE TABLE IF NOT EXISTS `imsgs` (
  `imsgs_id`       INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  `jabberid`       VARCHAR(80)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `password`       VARCHAR(80)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  CONSTRAINT `fk_imsgs_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: gpiod + gpiod_IO
-- =============================================================================
CREATE TABLE IF NOT EXISTS `gpiod` (
  `gpiod_id`       INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `agent_uuid`     VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `thread_tech_id` VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `debug`          BOOLEAN      NOT NULL DEFAULT 0,
  UNIQUE (`agent_uuid`, `thread_tech_id`),
  CONSTRAINT `fk_gpiod_agent_uuid` FOREIGN KEY (`agent_uuid`) REFERENCES `agents` (`agent_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `gpiod_IO` (
  `gpiod_io_id`     INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME    NOT NULL DEFAULT NOW(),
  `thread_tech_id`  VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `num`             INT(11)     NOT NULL DEFAULT 0,
  `mode_inout`      INT(11)     NOT NULL DEFAULT 0,
  `mode_activelow`  BOOLEAN     NOT NULL DEFAULT '0',
  `libelle`         VARCHAR(128) NOT NULL DEFAULT '',
  UNIQUE (`thread_tech_id`, `thread_acronyme`),
  CONSTRAINT `fk_gpiod_io_thread_tech_id` FOREIGN KEY (`thread_tech_id`) REFERENCES `gpiod` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: phidget + phidget_IO
-- =============================================================================
CREATE TABLE IF NOT EXISTS `phidget` (
  `phidget_id`     INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `server_uuid`    VARCHAR(37)  COLLATE utf8_unicode_ci NOT NULL,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `agent_tech_id`  VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `headless`       BOOLEAN      NOT NULL DEFAULT '1',
  `mqtt_connected` BOOLEAN      NOT NULL DEFAULT 0,
  `is_master`      BOOLEAN      NOT NULL DEFAULT 0,
  `description`    VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `heartbeat_time` DATETIME     NOT NULL DEFAULT NOW(),
  `start_time`     DATETIME     DEFAULT NOW(),
  `version`        VARCHAR(32)  NOT NULL DEFAULT 'none',
  `enable`         BOOLEAN      NOT NULL DEFAULT '1',
  `log_level`      INT(11)      NOT NULL DEFAULT 6,
  `hostname`       VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `password`       VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `serial`         INT(11)      UNIQUE NOT NULL DEFAULT 0,
  CONSTRAINT `fk_phidget_server_uuid` FOREIGN KEY (`server_uuid`) REFERENCES `servers` (`server_uuid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `phidget_IO` (
  `phidget_io_id`   INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `agent_tech_id`   VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `thread_acronyme` VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `classe`          VARCHAR(8)   COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `port`            INT(11)      NOT NULL,
  `capteur`         VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `libelle`         VARCHAR(128) NOT NULL DEFAULT '',
  `intervalle`      INT(11)      NOT NULL DEFAULT 5000,
  `archivage`       INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`agent_tech_id`, `port`),
  CONSTRAINT `fk_phidget_io_agent_tech_id` FOREIGN KEY (`agent_tech_id`) REFERENCES `phidget` (`agent_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: dls (avec colonne debug)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `dls` (
  `dls_id`        INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `date_create`   DATETIME    NOT NULL DEFAULT NOW(),
  `tech_id`       VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `package`       VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'custom',
  `syn_id`        INT(11)     NOT NULL DEFAULT '0',
  `name`          VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,
  `shortname`     VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `enable`        BOOLEAN      NOT NULL DEFAULT '0',
  `compil_date`   DATETIME     NOT NULL DEFAULT NOW(),
  `compil_time`   INT(11)      NOT NULL DEFAULT '0',
  `compil_status` BOOLEAN      NOT NULL DEFAULT '0',
  `compil_user`   VARCHAR(64)  NOT NULL DEFAULT '',
  `error_count`   INT(11)      NOT NULL DEFAULT '0',
  `warning_count` INT(11)      NOT NULL DEFAULT '0',
  `nbr_compil`    INT(11)      NOT NULL DEFAULT '0',
  `sourcecode`    MEDIUMTEXT   COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',
  `codec`         MEDIUMTEXT   COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',
  `errorlog`      TEXT         COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No Error',
  `nbr_ligne`     INT(11)      NOT NULL DEFAULT '0',
  `debug`         BOOLEAN      NOT NULL DEFAULT '0',
  CONSTRAINT `fk_dls_syn_id` FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

INSERT IGNORE INTO `dls` (`dls_id`, `syn_id`, `name`, `shortname`, `tech_id`, `enable`, `compil_status`)
VALUES (1, 1, 'Système', 'Système', 'SYS', 0, 0);

INSERT IGNORE INTO `dls` (`dls_id`, `syn_id`, `name`, `shortname`, `tech_id`, `enable`, `compil_status`, `sourcecode`)
VALUES (10000, 1, 'Programme de Test', 'TestDLS', 'TEST_DLS', 0, 0, '/* Programme de test fonctionnel */');

-- =============================================================================
-- TABLE: dls_packages + dls_params
-- =============================================================================
CREATE TABLE IF NOT EXISTS `dls_packages` (
  `dls_package_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`    DATETIME     NOT NULL DEFAULT NOW(),
  `name`           VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `description`    VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `sourcecode`     MEDIUMTEXT   COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `dls_params` (
  `dls_param_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `tech_id`      VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`     VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `libelle`      VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default libelle',
  `valeur`       VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default value',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_dls_params_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: mappings
-- =============================================================================
CREATE TABLE IF NOT EXISTS `mappings` (
  `mapping_id`      INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `thread_tech_id`  VARCHAR(32) NOT NULL,
  `thread_acronyme` VARCHAR(64) NOT NULL,
  `tech_id`         VARCHAR(32) NULL DEFAULT NULL,
  `acronyme`        VARCHAR(64) NULL DEFAULT NULL,
  UNIQUE (`thread_tech_id`, `thread_acronyme`),
  UNIQUE (`tech_id`, `acronyme`),
  UNIQUE (`thread_tech_id`, `thread_acronyme`, `tech_id`, `acronyme`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1;

-- =============================================================================
-- TABLE: mnemos_*
-- =============================================================================
CREATE TABLE IF NOT EXISTS `mnemos_DI` (
  `mnemo_di_id`  INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`    BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`      VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`     VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`         BOOLEAN      NOT NULL DEFAULT 0,
  `libelle`      VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat`         BOOLEAN      NOT NULL DEFAULT '0',
  `archivage`    INT(11)      NOT NULL DEFAULT 864000,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_di_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_DO` (
  `mnemo_do_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`   BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`    VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`        BOOLEAN      NOT NULL DEFAULT 1,
  `etat`        BOOLEAN      NOT NULL DEFAULT '0',
  `mono`        BOOLEAN      NOT NULL DEFAULT '0',
  `libelle`     VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `archivage`   INT(11)      NOT NULL DEFAULT 864000,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_do_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_AI` (
  `mnemo_ai_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`   BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`    VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`        BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`     VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `valeur`      FLOAT        NOT NULL DEFAULT '0',
  `unite`       VARCHAR(32)  NOT NULL DEFAULT '',
  `archivage`   INT(11)      NOT NULL DEFAULT 36000,
  `in_range`    BOOLEAN      NOT NULL DEFAULT '0',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_ai_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_AO` (
  `mnemo_ao_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`   BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`    VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`        BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`     VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `valeur`      FLOAT        NOT NULL DEFAULT '0',
  `unite`       VARCHAR(32)  NOT NULL DEFAULT '',
  `archivage`   INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_ao_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_BI` (
  `mnemo_bi_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`   BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`    VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`        BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`     VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat`        BOOLEAN      NOT NULL DEFAULT 0,
  `groupe`      INT(11)      NOT NULL DEFAULT 0,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_bi_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_MONO` (
  `mnemo_mono_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`     BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`       VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`      VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`          BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`       VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat`          BOOLEAN      NOT NULL DEFAULT 0,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_mono_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_WATCHDOG` (
  `mnemo_watchdog_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`         BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`           VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`          VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`              BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`           VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_watchdog_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_CI` (
  `mnemo_ci_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`   BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`    VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`        BOOLEAN      NOT NULL DEFAULT 1,
  `etat`        BOOLEAN      NOT NULL DEFAULT '0',
  `libelle`     VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `valeur`      INT(11)      NOT NULL DEFAULT '0',
  `unite`       VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fois',
  `archivage`   INT(11)      NOT NULL DEFAULT 36000,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_ci_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_CH` (
  `mnemo_ch_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `tech_id`     VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`    VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`        BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`     VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat`        BOOLEAN      NOT NULL DEFAULT '0',
  `valeur`      INT(11)      NOT NULL DEFAULT '0',
  `archivage`   INT(11)      NOT NULL DEFAULT 864000,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_ch_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_TEMPO` (
  `mnemo_tempo_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `tech_id`        VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`       VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`           BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`        VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_tempo_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE` (
  `mnemo_horloge_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`        BOOLEAN      NOT NULL DEFAULT '1',
  `access_level`     INT(11)      NOT NULL DEFAULT '0',
  `tech_id`          VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`         VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`             BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`          VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_horloge_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE_ticks` (
  `mnemo_horloge_tick_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `horloge_id`            INT(11) NOT NULL,
  `heure`                 INT(11) NOT NULL DEFAULT '0',
  `minute`                INT(11) NOT NULL DEFAULT '0',
  `lundi`                 BOOLEAN NOT NULL DEFAULT '0',
  `mardi`                 BOOLEAN NOT NULL DEFAULT '0',
  `mercredi`              BOOLEAN NOT NULL DEFAULT '0',
  `jeudi`                 BOOLEAN NOT NULL DEFAULT '0',
  `vendredi`              BOOLEAN NOT NULL DEFAULT '0',
  `samedi`                BOOLEAN NOT NULL DEFAULT '0',
  `dimanche`              BOOLEAN NOT NULL DEFAULT '0',
  `date_modif`            DATETIME NOT NULL DEFAULT NOW(),
  CONSTRAINT `fk_mnemos_horloge_ticks_horloge_id` FOREIGN KEY (`horloge_id`) REFERENCES `mnemos_HORLOGE` (`mnemo_horloge_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_REGISTRE` (
  `mnemo_registre_id`  INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `tech_id`            VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`           VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`               BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`            VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,
  `valeur`             FLOAT        NOT NULL DEFAULT '0',
  `unite`              VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `archivage`          INT(11)      NOT NULL DEFAULT 36000,
  `map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_reponse_vocale`  VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_registre_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `mnemos_VISUEL` (
  `mnemo_visuel_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `tech_id`         VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `acronyme`        VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `used`            BOOLEAN      NOT NULL DEFAULT 1,
  `forme`           VARCHAR(80)  NOT NULL DEFAULT 'unknown',
  `mode`            VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `color`           VARCHAR(16)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'gray',
  `badge`           VARCHAR(16)  COLLATE utf8_unicode_ci NOT NULL DEFAULT 'none',
  `valeur`          FLOAT        NOT NULL DEFAULT 0,
  `cligno`          BOOLEAN      NOT NULL DEFAULT 0,
  `noshow`          BOOLEAN      NOT NULL DEFAULT 0,
  `disable`         BOOLEAN      NOT NULL DEFAULT 0,
  `libelle`         VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,
  `minimum`         FLOAT        NOT NULL DEFAULT '0',
  `maximum`         FLOAT        NOT NULL DEFAULT '100',
  `seuil_ntb`       FLOAT        NOT NULL DEFAULT '5',
  `seuil_nb`        FLOAT        NOT NULL DEFAULT '10',
  `seuil_nh`        FLOAT        NOT NULL DEFAULT '90',
  `seuil_nth`       FLOAT        NOT NULL DEFAULT '95',
  `nb_decimal`      INT(11)      NOT NULL DEFAULT '2',
  `input_tech_id`   VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `input_acronyme`  VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `rw`              BOOLEAN      NOT NULL DEFAULT 0,
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_mnemos_visuel_tech_id` FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: syns_motifs
-- =============================================================================
CREATE TABLE IF NOT EXISTS `syns_motifs` (
  `syn_motif_id`    INT(11) PRIMARY KEY AUTO_INCREMENT,
  `mnemo_visuel_id` INT(11) NOT NULL,
  `dls_id`          INT(11) NOT NULL,
  `used`            BOOLEAN NOT NULL DEFAULT 1,
  `posx`            INT(11) NOT NULL DEFAULT '0',
  `posy`            INT(11) NOT NULL DEFAULT '0',
  `angle`           INT(11) NOT NULL DEFAULT '0',
  `scale`           FLOAT   NOT NULL DEFAULT '1.0',
  `layer`           INT(11) NOT NULL DEFAULT '0',
  `place`           INT(11) NOT NULL DEFAULT '0',
  UNIQUE (`dls_id`, `mnemo_visuel_id`),
  CONSTRAINT `fk_syns_motifs_visuel_id` FOREIGN KEY (`mnemo_visuel_id`) REFERENCES `mnemos_VISUEL` (`mnemo_visuel_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_syns_motifs_dls_id`    FOREIGN KEY (`dls_id`)          REFERENCES `dls`           (`dls_id`)          ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: syns_cadrans (upgrade db_version<18)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `syns_cadrans` (
  `syn_cadran_id` INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `dls_id`        INT(11)     NOT NULL DEFAULT 0,
  `forme`         VARCHAR(80) NOT NULL DEFAULT 'unknown',
  `tech_id`       VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `acronyme`      VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `groupe`        INT(11)     NOT NULL DEFAULT '0',
  `posx`          INT(11)     NOT NULL DEFAULT '0',
  `posy`          INT(11)     NOT NULL DEFAULT '0',
  `scale`         FLOAT       NOT NULL DEFAULT '1.0',
  `minimum`       FLOAT       NOT NULL DEFAULT '0',
  `maximum`       FLOAT       NOT NULL DEFAULT '100',
  `seuil_ntb`     FLOAT       NOT NULL DEFAULT '5',
  `seuil_nb`      FLOAT       NOT NULL DEFAULT '10',
  `seuil_nh`      FLOAT       NOT NULL DEFAULT '90',
  `seuil_nth`     FLOAT       NOT NULL DEFAULT '95',
  `angle`         INT(11)     NOT NULL DEFAULT '0',
  `nb_decimal`    INT(11)     NOT NULL DEFAULT '2',
  UNIQUE (`dls_id`, `tech_id`, `acronyme`),
  CONSTRAINT `fk_syns_cadrans_dls_id` FOREIGN KEY (`dls_id`) REFERENCES `dls` (`dls_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: tableau + tableau_map
-- =============================================================================
CREATE TABLE IF NOT EXISTS `tableau` (
  `tableau_id`  INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create` DATETIME     NOT NULL DEFAULT NOW(),
  `titre`       VARCHAR(128) UNIQUE NOT NULL,
  `syn_id`      INT(11)      NOT NULL,
  `mode`        INT(11)      NOT NULL DEFAULT 0,
  `periode`     VARCHAR(64)  NOT NULL DEFAULT 'BY_10_MINUTE_ON_3_DAYS',
  `period_lock` BOOLEAN      NOT NULL DEFAULT '0',
  CONSTRAINT `fk_tableau_syn_id` FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

CREATE TABLE IF NOT EXISTS `tableau_map` (
  `tableau_map_id` INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `tableau_id`     INT(11)     NOT NULL,
  `tech_id`        VARCHAR(32) NOT NULL,
  `acronyme`       VARCHAR(64) NOT NULL,
  `color`          VARCHAR(16) NOT NULL DEFAULT 'blue',
  `multi`          FLOAT       NOT NULL DEFAULT 1,
  `offset`         FLOAT       NOT NULL DEFAULT 0,
  `methode`        VARCHAR(24) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'AVG',
  CONSTRAINT `fk_tableau_map_tableau_id` FOREIGN KEY (`tableau_id`) REFERENCES `tableau` (`tableau_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: msgs (messages d'alarme/alerte)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `msgs` (
  `msg_id`            INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `deletable`         BOOLEAN      NOT NULL DEFAULT '1',
  `tech_id`           VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`          VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `used`              BOOLEAN      NOT NULL DEFAULT 1,
  `libelle`           VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No libelle',
  `typologie`         INT(11)      NOT NULL DEFAULT '0',
  `groupe`            INT(11)      NOT NULL DEFAULT '0',
  `rate_limit`        INT(11)      NOT NULL DEFAULT '1',
  `freeze`            INT(11)      NOT NULL DEFAULT '0',
  `notif_sms`         INT(11)      NOT NULL DEFAULT '-1',
  `notif_sms_by_dls`  INT(11)      NOT NULL DEFAULT '0',
  `notif_chat`        INT(11)      NOT NULL DEFAULT '-1',
  `notif_chat_by_dls` INT(11)      NOT NULL DEFAULT '0',
  `audio_zone_name`   VARCHAR(32)  NOT NULL DEFAULT 'ZD_NONE',
  `audio_zone_by_dls` VARCHAR(32)  NOT NULL DEFAULT '',
  `audio_libelle`     VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `etat`              BOOLEAN      NOT NULL DEFAULT '0',
  UNIQUE (`tech_id`, `acronyme`),
  CONSTRAINT `fk_msgs_tech_id`          FOREIGN KEY (`tech_id`)         REFERENCES `dls`         (`tech_id`)         ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_msgs_audio_zone_name`  FOREIGN KEY (`audio_zone_name`) REFERENCES `audio_zones` (`audio_zone_name`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: histo_msgs
-- =============================================================================
CREATE TABLE IF NOT EXISTS `histo_msgs` (
  `histo_msg_id`  INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `tech_id`       VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `acronyme`      VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `syn_page`      VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `dls_shortname` VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `typologie`     INT(11)      NOT NULL DEFAULT '0',
  `nom_ack`       VARCHAR(97)  COLLATE utf8_unicode_ci DEFAULT NULL,
  `date_create`   DATETIME(2)  NULL,
  `date_fixe`     DATETIME(2)  NULL,
  `date_fin`      DATETIME(2)  NULL,
  `libelle`       VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,
  KEY (`date_create`),
  KEY (`date_fin`),
  UNIQUE (`date_create`, `tech_id`, `acronyme`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: audit_log (domaine)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `audit_log` (
  `audit_log_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `username`     VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `access_level` INT(11)      NOT NULL DEFAULT 0,
  `classe`       VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `message`      VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,
  `date`         DATETIME     NOT NULL DEFAULT NOW(),
  KEY (`date`),
  KEY (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: cleanup
-- =============================================================================
CREATE TABLE IF NOT EXISTS `cleanup` (
  `cleanup_id`  INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create` DATETIME     NOT NULL DEFAULT NOW(),
  `archive`     BOOLEAN      NOT NULL DEFAULT '1',
  `requete`     VARCHAR(256) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- =============================================================================
-- TABLE: cameras (avec url et enable — db_version 87+91)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `cameras` (
  `camera_id`    INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`  DATETIME     NOT NULL DEFAULT NOW(),
  `name`         VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT 'NewCamera',
  `url`          VARCHAR(128) NOT NULL DEFAULT '',
  `access_level` INT(11)      NOT NULL DEFAULT '0',
  `enable`       BOOLEAN      NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

INSERT IGNORE INTO `cameras` (`name`, `url`, `access_level`, `enable`)
VALUES ('Camera-Test-01', 'rtsp://192.168.1.100:554/stream', 0, 1);
INSERT IGNORE INTO `cameras` (`name`, `url`, `access_level`, `enable`)
VALUES ('Camera-Test-02', 'http://192.168.1.101/video.mjpeg', 6, 1);

-- =============================================================================
-- TABLE: syn_cameras
-- =============================================================================
CREATE TABLE IF NOT EXISTS `syn_cameras` (
  `syn_camera_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `date_create`   DATETIME NOT NULL DEFAULT NOW(),
  `syn_id`        INT(11)  NOT NULL,
  `camera_id`     INT(11)  NOT NULL,
  UNIQUE KEY `uk_syn_cameras` (`syn_id`, `camera_id`),
  CONSTRAINT `fk_syn_cameras_syn_id`    FOREIGN KEY (`syn_id`)    REFERENCES `syns`    (`syn_id`)    ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_syn_cameras_camera_id` FOREIGN KEY (`camera_id`) REFERENCES `cameras` (`camera_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- =============================================================================
-- TABLE ARCHIVE: histo_bit (chargée aussi sur le serveur d'archive port 13307)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `histo_bit` (
  `tech_id`          VARCHAR(32)  NOT NULL,
  `acronyme`         VARCHAR(64)  NOT NULL,
  `date_time`        DATETIME(2)  NOT NULL,
  `date_time_year`   INT GENERATED ALWAYS AS ( YEAR(`date_time`)    ) STORED,
  `date_time_month`  INT GENERATED ALWAYS AS ( MONTH(`date_time`)   ) STORED,
  `date_time_week`   INT GENERATED ALWAYS AS ( WEEK(`date_time`, 1) ) STORED,
  `date_time_day`    INT GENERATED ALWAYS AS ( DAY(`date_time`)     ) STORED,
  `date_time_hour`   INT GENERATED ALWAYS AS ( HOUR(`date_time`)    ) STORED,
  `date_time_min`    INT GENERATED ALWAYS AS ( MINUTE(`date_time`)  ) STORED,
  `valeur`           FLOAT        NOT NULL,
  UNIQUE `idx_tech_id_acronyme_date_time` (`tech_id`, `acronyme`, `date_time`),
  INDEX `idx_group_by` (`tech_id`, `acronyme`, `date_time_year`, `date_time_month`, `date_time_day`, `date_time_hour`, `date_time_min`),
  INDEX `idx_group_by_week` (`tech_id`, `acronyme`, `date_time_year`, `date_time_week`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- =============================================================================
-- DONNÉES DE TEST SUPPLÉMENTAIRES (threads, mnémos, historique, etc.)
-- =============================================================================

USE `aaaaaaaa-0000-0000-0000-000000000001`;

-- ---- threads: modbus -------------------------------------------------------
INSERT IGNORE INTO `modbus`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `hostname`, `watchdog`, `max_request_par_sec`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_MODBUS', 'Automate de test', 1, 0, '192.168.1.200', 50, 50);

-- ---- threads: phidget ------------------------------------------------------
INSERT IGNORE INTO `phidget`
  (`server_uuid`, `agent_tech_id`, `description`, `enable`, `log_level`, `hostname`, `password`, `serial`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_PHIDGET', 'Phidget de test', 1, 6, '192.168.1.201', '', 12345);

-- ---- threads: gpiod --------------------------------------------------------
INSERT IGNORE INTO `gpiod`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_GPIOD', 'GPIO de test', 1, 0);

-- ---- threads: audio --------------------------------------------------------
INSERT IGNORE INTO `audio`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `language`, `device`, `volume`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_AUDIO', 'Audio de test', 1, 0, 'fr', 'default', 80);

-- ---- threads: imsgs --------------------------------------------------------
INSERT IGNORE INTO `imsgs`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `jabberid`, `password`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_IMSGS', 'XMPP de test', 1, 0, 'test@xmpp.test', 'testpass');

-- ---- threads: smsg ---------------------------------------------------------
INSERT IGNORE INTO `smsg`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`,
   `ovh_service_name`, `ovh_application_key`, `ovh_application_secret`, `ovh_consumer_key`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_SMSG', 'SMS de test', 1, 0,
   'svc-test', 'appkey000000000000000000000000000', 'appsecret0000000000000000000000000', 'consumerkey00000000000000000000000');

-- ---- threads: shelly -------------------------------------------------------
INSERT IGNORE INTO `shelly`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `string_id`, `hostname`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_SHELLY', 'Shelly de test', 1, 0, 'shellypro2-aabbccddeeff', '192.168.1.202');

-- ---- threads: meteo --------------------------------------------------------
INSERT IGNORE INTO `meteo`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `token`, `code_insee`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_METEO', 'Météo de test', 1, 0, 'fake_meteo_token_001', '75056');

-- ---- threads: ups ----------------------------------------------------------
INSERT IGNORE INTO `ups`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `host`, `name`, `admin_username`, `admin_password`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_UPS', 'UPS de test', 1, 0, '192.168.1.203', 'UPS-TEST', 'admin', 'upspass');

-- ---- threads: teleinfoedf --------------------------------------------------
INSERT IGNORE INTO `teleinfoedf`
  (`agent_uuid`, `thread_tech_id`, `description`, `enable`, `debug`, `port`, `standard`)
VALUES
  ('ffffffff-0000-0000-0000-000000000001', 'TEST_TELEINFO', 'Téléinfo EDF de test', 1, 0, '/dev/ttyUSB0', 0);

-- ---- sous-tables modbus ----------------------------------------------------
INSERT IGNORE INTO `modbus_DI`
  (`thread_tech_id`, `thread_acronyme`, `num`, `libelle`, `borne`, `ed`, `flip`, `archivage`)
VALUES
  ('TEST_MODBUS', 'MOD_DI_01', 0, 'Entrée digitale test 01', 'I1', 'DI', 0, 36000);

INSERT IGNORE INTO `modbus_DO`
  (`thread_tech_id`, `thread_acronyme`, `num`, `libelle`, `borne`, `ed`, `archivage`)
VALUES
  ('TEST_MODBUS', 'MOD_DO_01', 0, 'Sortie digitale test 01', 'Q1', 'DO', 36000);

INSERT IGNORE INTO `modbus_AI`
  (`thread_tech_id`, `thread_acronyme`, `num`, `type_borne`, `min`, `max`, `libelle`, `borne`, `ed`, `unite`, `archivage`)
VALUES
  ('TEST_MODBUS', 'MOD_AI_01', 0, 0, 0, 100, 'Entrée analogique test 01', 'IW1', 'AI', '%', 36000);

INSERT IGNORE INTO `modbus_AO`
  (`thread_tech_id`, `thread_acronyme`, `num`, `type_borne`, `min`, `max`, `libelle`, `borne`, `ed`, `unite`, `archivage`)
VALUES
  ('TEST_MODBUS', 'MOD_AO_01', 0, 0, 0, 100, 'Sortie analogique test 01', 'QW1', 'AO', '%', 36000);

-- ---- sous-tables gpiod -----------------------------------------------------
INSERT IGNORE INTO `gpiod_IO`
  (`thread_tech_id`, `thread_acronyme`, `num`, `mode_inout`, `mode_activelow`, `libelle`)
VALUES
  ('TEST_GPIOD', 'GPIO_01', 0, 0, 0, 'GPIO test 01');

-- ---- sous-tables phidget ---------------------------------------------------
INSERT IGNORE INTO `phidget_IO`
  (`thread_tech_id`, `thread_acronyme`, `classe`, `port`, `capteur`, `libelle`, `intervalle`, `archivage`)
VALUES
  ('TEST_PHIDGET', 'PHI_IO_01', 'DI', 0, '', 'Entrée phidget test 01', 5000, 36000);

-- ---- zone audio supplémentaire ---------------------------------------------
INSERT IGNORE INTO `audio_zones` (`audio_zone_name`, `description`)
VALUES ('ZD_TEST', 'Zone audio de test');

INSERT IGNORE INTO `audio_zone_map` (`audio_zone_id`, `thread_tech_id`)
SELECT az.audio_zone_id, 'TEST_AUDIO'
FROM `audio_zones` AS az
WHERE az.audio_zone_name='ZD_TEST'
LIMIT 1;

-- ---- dls_packages ----------------------------------------------------------
INSERT IGNORE INTO `dls_packages` (`name`, `description`, `sourcecode`)
VALUES ('TEST_PACKAGE', 'Package de test fonctionnel', '/* Package source de test */');

-- ---- synoptique enfant de test ---------------------------------------------
INSERT IGNORE INTO `syns` (`syn_id`, `parent_id`, `libelle`, `page`, `access_level`)
VALUES (2, 1, 'Synoptique Test', 'TEST_SYN', 6);

-- ---- tableau ---------------------------------------------------------------
INSERT IGNORE INTO `tableau` (`titre`, `syn_id`, `mode`, `periode`, `period_lock`)
VALUES ('Test-Tableau-01', 1, 0, 'BY_10_MINUTE_ON_3_DAYS', 0);

-- ---- mnémos pour tableau_map -----------------------------------------------
INSERT IGNORE INTO `mnemos_AI`
  (`tech_id`, `acronyme`, `libelle`, `valeur`, `unite`, `archivage`, `deletable`)
VALUES
  ('TEST_DLS', 'TEST_AI', 'Mnémo AI de test', 0.0, 'kWh', 36000, 1);

INSERT IGNORE INTO `mnemos_DI`
  (`tech_id`, `acronyme`, `libelle`, `etat`, `archivage`, `deletable`)
VALUES
  ('TEST_DLS', 'TEST_DI', 'Mnémo DI de test', 0, 36000, 1);

INSERT IGNORE INTO `mnemos_VISUEL`
  (`tech_id`, `acronyme`, `forme`, `libelle`, `used`)
VALUES
  ('TEST_DLS', 'TEST_VISUEL', 'rectangle', 'Mnémo VISUEL de test', 1);

-- ---- tableau_map -----------------------------------------------------------
INSERT IGNORE INTO `tableau_map` (`tableau_id`, `tech_id`, `acronyme`, `color`, `multi`, `offset`, `methode`)
SELECT t.tableau_id, 'TEST_DLS', 'TEST_AI', 'blue', 1.0, 0.0, 'AVG'
FROM `tableau` t WHERE t.titre='Test-Tableau-01' LIMIT 1;

-- ---- msgs (message d'alarme) -----------------------------------------------
INSERT IGNORE INTO `msgs`
  (`tech_id`, `acronyme`, `libelle`, `typologie`, `groupe`, `audio_zone_name`, `etat`, `deletable`)
VALUES
  ('TEST_DLS', 'TEST_MSG', 'Message de test fonctionnel', 0, 0, 'ZD_NONE', 0, 1);

-- ---- histo_msgs (2 entrées actives pour tester /histo/alive + /histo/search)
INSERT IGNORE INTO `histo_msgs`
  (`tech_id`, `acronyme`, `syn_page`, `dls_shortname`, `typologie`, `date_create`, `date_fin`, `libelle`)
VALUES
  ('TEST_DLS', 'TEST_MSG', 'HOME', 'TestDLS', 0, DATE_SUB(NOW(), INTERVAL 10 MINUTE), NULL, 'Alarme test active 1'),
  ('TEST_DLS', 'TEST_MSG', 'HOME', 'TestDLS', 0, DATE_SUB(NOW(), INTERVAL 5 MINUTE),  NULL, 'Alarme test active 2');

-- ---- mappings --------------------------------------------------------------
INSERT IGNORE INTO `mappings` (`thread_tech_id`, `thread_acronyme`, `tech_id`, `acronyme`)
VALUES ('TEST_MODBUS', 'MOD_AI_01', 'TEST_DLS', 'TEST_AI');

-- ---- syn_cameras -----------------------------------------------------------
INSERT IGNORE INTO `syn_cameras` (`syn_id`, `camera_id`)
SELECT 1, c.camera_id FROM `cameras` c WHERE c.name = 'Camera-Test-01' LIMIT 1;

-- =============================================================================
-- Revenir sur la base master
-- =============================================================================
USE master;
