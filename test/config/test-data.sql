-- =============================================================================
-- ABLS-Habitat API - Fixtures de test
-- Base principale: abls_master
-- =============================================================================
-- Utilisation: mariadb -h 127.0.0.1 -P 13306 -u abls_test -pabls_test_pass abls_master < test-data.sql
-- =============================================================================

USE abls_master;

-- Droits niveau master pour l'user de test
GRANT ALL PRIVILEGES ON `abls_test_%`.* TO 'abls_test'@'%';
GRANT SUPER ON *.* TO 'abls_test'@'%';
FLUSH PRIVILEGES;

-- =============================================================================
-- TABLE: database_version (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `database_version` (
  `date`    DATETIME NOT NULL DEFAULT NOW(),
  `version` INT(11)  NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT IGNORE INTO `database_version` (`version`) VALUES (100);

-- =============================================================================
-- TABLE: domains (master)
-- =============================================================================
CREATE TABLE IF NOT EXISTS `domains` (
  `domain_id`             INT(11)       PRIMARY KEY AUTO_INCREMENT,
  `domain_uuid`           VARCHAR(37)   UNIQUE NOT NULL,
  `domain_secret`         VARCHAR(128)  NOT NULL DEFAULT 'test-secret',
  `date_create`           DATETIME      NOT NULL DEFAULT NOW(),
  `domain_name`           VARCHAR(256)  NOT NULL DEFAULT 'My new domain',
  `db_password`           VARCHAR(64)   NULL,
  `db_version`            INT(11)       NOT NULL DEFAULT '91',
  `mqtt_password`         VARCHAR(128)  NOT NULL DEFAULT 'mqtt_pass',
  `browser_password`      VARCHAR(128)  NOT NULL DEFAULT 'browser_pass',
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Domaine de test
INSERT IGNORE INTO `domains`
  (`domain_uuid`, `domain_secret`, `domain_name`, `db_version`, `mqtt_password`, `browser_password`)
VALUES
  ('aaaaaaaa-0000-0000-0000-000000000001', 'test-domain-secret-001', 'Domaine de Test Principal', 91, 'mqtt_test_001', 'browser_test_001');

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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `icons_modes` (
  `icon_mode_id` INT(11)     PRIMARY KEY AUTO_INCREMENT,
  `forme`        VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `mode`         VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `date_create`  DATETIME    NOT NULL DEFAULT NOW(),
  CONSTRAINT `fk_icons_modes_forme` FOREIGN KEY (`forme`) REFERENCES `icons` (`forme`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- =============================================================================
-- BASE DOMAINE: abls_test_aaaaaaaa_0000_0000_0000_000000000001
-- (l'API créé la DB du domaine avec le nom basé sur domain_uuid)
-- =============================================================================
CREATE DATABASE IF NOT EXISTS `abls_test_aaaaaaaa_0000_0000_0000_000000000001`
  CHARACTER SET utf8 COLLATE utf8_unicode_ci;

-- Permettre l'accès complet à cette BD domaine
GRANT ALL ON `abls_test_aaaaaaaa_0000_0000_0000_000000000001`.* TO 'abls_test'@'%';
FLUSH PRIVILEGES;

USE `abls_test_aaaaaaaa_0000_0000_0000_000000000001`;

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

-- Table des DLS
CREATE TABLE IF NOT EXISTS `dls` (
  `dls_id`          INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `tech_id`         VARCHAR(32)  COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `package`         VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'custom',
  `syn_id`          INT(11)      NOT NULL DEFAULT '1',
  `name`            VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL,
  `shortname`       VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `enable`          BOOLEAN      NOT NULL DEFAULT '0',
  `compil_date`     DATETIME     NOT NULL DEFAULT NOW(),
  `compil_time`     INT(11)      NOT NULL DEFAULT '0',
  `compil_status`   BOOLEAN      NOT NULL DEFAULT '0',
  `compil_user`     VARCHAR(64)  NOT NULL DEFAULT '',
  `error_count`     INT(11)      NOT NULL DEFAULT '0',
  `warning_count`   INT(11)      NOT NULL DEFAULT '0',
  `nbr_compil`      INT(11)      NOT NULL DEFAULT '0',
  `sourcecode`      MEDIUMTEXT   COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',
  `codec`           MEDIUMTEXT   COLLATE utf8_unicode_ci NOT NULL DEFAULT '/* Default ! */',
  `errorlog`        TEXT         COLLATE utf8_unicode_ci NOT NULL DEFAULT 'No Error',
  `nbr_ligne`       INT(11)      NOT NULL DEFAULT '0',
  CONSTRAINT `fk_dls_syn_id` FOREIGN KEY (`syn_id`) REFERENCES `syns` (`syn_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- DLS Système (requis)
INSERT IGNORE INTO `dls` (`dls_id`, `syn_id`, `name`, `shortname`, `tech_id`, `enable`, `compil_status`)
VALUES (1, 1, 'Système', 'Système', 'SYS', 0, 0);

-- DLS de test
INSERT IGNORE INTO `dls` (`dls_id`, `syn_id`, `name`, `shortname`, `tech_id`, `enable`, `compil_status`, `sourcecode`)
VALUES (10000, 1, 'Programme de Test', 'TestDLS', 'TEST_DLS', 0, 0, '/* Programme de test fonctionnel */');

-- Table des cameras
CREATE TABLE IF NOT EXISTS `cameras` (
  `camera_id`    INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `date_create`  DATETIME     NOT NULL DEFAULT NOW(),
  `name`         VARCHAR(128) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `url`          VARCHAR(128) NOT NULL DEFAULT '',
  `access_level` INT(11)      NOT NULL DEFAULT '0',
  `enable`       BOOLEAN      NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Cameras de test initiaux (pour tester list/get)
INSERT IGNORE INTO `cameras` (`name`, `url`, `access_level`, `enable`)
VALUES ('Camera-Test-01', 'rtsp://192.168.1.100:554/stream', 0, 1);
INSERT IGNORE INTO `cameras` (`name`, `url`, `access_level`, `enable`)
VALUES ('Camera-Test-02', 'http://192.168.1.101/video.mjpeg', 6, 1);

-- Table syn_cameras
CREATE TABLE IF NOT EXISTS `syn_cameras` (
  `syn_camera_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `date_create`   DATETIME NOT NULL DEFAULT NOW(),
  `syn_id`        INT(11)  NOT NULL,
  `camera_id`     INT(11)  NOT NULL,
  UNIQUE KEY `uk_syn_cameras` (`syn_id`, `camera_id`),
  CONSTRAINT `fk_syn_cameras_syn_id`    FOREIGN KEY (`syn_id`)    REFERENCES `syns`    (`syn_id`)    ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_syn_cameras_camera_id` FOREIGN KEY (`camera_id`) REFERENCES `cameras` (`camera_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Table audit_log
CREATE TABLE IF NOT EXISTS `audit_log` (
  `audit_log_id` INT(11)      PRIMARY KEY AUTO_INCREMENT,
  `username`     VARCHAR(64)  COLLATE utf8_unicode_ci NOT NULL,
  `classe`       VARCHAR(32)  COLLATE utf8_unicode_ci NOT NULL,
  `access_level` INT(11)      NOT NULL DEFAULT 0,
  `message`      VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,
  `date`         DATETIME     NOT NULL DEFAULT NOW(),
  KEY (`date`),
  KEY (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Table agents (nécessaire pour les contraintes FK de certains threads)
CREATE TABLE IF NOT EXISTS `agents` (
  `agent_uuid`      VARCHAR(37)  PRIMARY KEY,
  `date_create`     DATETIME     NOT NULL DEFAULT NOW(),
  `install_time`    DATETIME     NULL,
  `heartbeat_time`  DATETIME     DEFAULT NOW(),
  `domain_uuid`     VARCHAR(37)  NOT NULL,
  `agent_hostname`  VARCHAR(64)  UNIQUE NOT NULL,
  `description`     VARCHAR(128) NOT NULL DEFAULT '',
  `headless`        BOOLEAN      NOT NULL DEFAULT '1',
  `log_bus`         BOOLEAN      NOT NULL DEFAULT 0,
  `log_dls`         BOOLEAN      NOT NULL DEFAULT 0,
  `branche`         VARCHAR(32)  NOT NULL DEFAULT 'none'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Agent de test
INSERT IGNORE INTO `agents` (`agent_uuid`, `domain_uuid`, `agent_hostname`, `description`)
VALUES ('ffffffff-0000-0000-0000-000000000001', 'aaaaaaaa-0000-0000-0000-000000000001', 'test-agent-host', 'Agent de test fonctionnel');

-- =============================================================================
-- Revenir sur la base master
-- =============================================================================
USE abls_master;
