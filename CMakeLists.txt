##################################################################################################################################
# CMakeLists.txt    Profil de compilation de l'interface API Abls
# Projet Abls-Habitat version 4.7      Gestion d'habitat
# Auteur: LEFEVRE Sebastien                                                                              26.02.2026
##################################################################################################################################

cmake_minimum_required(VERSION 3.16)

# Récupération de la version depuis git (git describe renvoie
# quelque chose comme "v4.7-3-g9f2d1a").
# Si la commande échoue on retombe sur le numéro figé dans PROJECT_VERSION.
execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ABLS_API_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

# le projet doit connaître sa version dès l'appel à project().
# on se sert de la valeur récupérée ou, si elle est vide, de celle
# fournie en dur ci‑dessous.
if(ABLS_API_VERSION MATCHES "^v?([0-9]+\\.[0-9]+)")
  set(ABLS_API_VERSION_NUMBER "${CMAKE_MATCH_1}")
else()
  set(ABLS_API_VERSION_NUMBER "unknown")
endif()

project(Abls-Habitat-API
  VERSION ${ABLS_API_VERSION_NUMBER}
  DESCRIPTION "Abls Habitat API"
  HOMEPAGE_URL "https://docs.abls-habitat.fr"
  LANGUAGES C CXX
)

# Flags de compilation — seront appliquées par cible en fonction du compilateur
# (globales précédemment, mais cela polluait d'autres projets et n'était pas
# portable). Le drapeau -g/-ggdb est géré par la configuration Debug.

add_compile_definitions(
  ABLS_API_VERSION="${ABLS_API_VERSION}"
  ABLS_API_PKGDATADIR="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}"
)

include_directories(${CMAKE_SOURCE_DIR}/Include)

# pkg-config
find_package(PkgConfig REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

pkg_check_modules(GLIB       REQUIRED glib-2.0)
pkg_check_modules(OPENSSL    REQUIRED openssl)
pkg_check_modules(GTHREAD    REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB  REQUIRED json-glib-1.0)
pkg_check_modules(LIBSOUP    REQUIRED libsoup-3.0)
pkg_check_modules(LIBJWT     REQUIRED libjwt)
pkg_check_modules(UUID       REQUIRED uuid)
pkg_check_modules(MOSQUITTO  REQUIRED libmosquitto)
pkg_check_modules(MEMCACHED  REQUIRED libmemcached)

# mariadb
find_program(MARIADB_CONFIG mariadb_config REQUIRED)
execute_process(COMMAND ${MARIADB_CONFIG} --libs OUTPUT_VARIABLE MARIADB_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${MARIADB_CONFIG} --cflags OUTPUT_VARIABLE MARIADB_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(MARIADB_LIBS_LIST   UNIX_COMMAND "${MARIADB_LIBS}")
separate_arguments(MARIADB_CFLAGS_LIST UNIX_COMMAND "${MARIADB_CFLAGS}")

# Sous-répertoire TraductionDLS (produit la lib statique tradDLS)
add_subdirectory(TraductionDLS)

# Sources de l'exécutable principal
add_executable(abls-habitat-api
  Http.c
  Erreur.c
  json.c
  database.c
  domains.c
  agent.c
  icons.c
  status.c
  visuel.c
  thread.c
  audit_log.c
  tableaux.c
  shelly.c
  mqtt.c
  users.c
  uuid.c
  mnemos.c
  archive.c
  imsgs.c
  smsg.c
  dls.c
  dls_package.c
  dls_params.c
  message.c
  dictionnaire.c
  histo.c
  meteo.c
  alexa.c
  gpiod.c
  mnemo_BI.c
  mnemo_MONO.c
  mnemo_DI.c
  mnemo_HORLOGE.c
  mnemo_DO.c
  mnemo_AI.c
  mnemo_AO.c
  mnemo_TEMPO.c
  mnemo_REGISTRE.c
  mnemo_WATCHDOG.c
  mnemo_CI.c
  mnemo_CH.c
  mnemo_VISUEL.c
  modbus.c
  phidget.c
  mail.c
  audio.c
  mapping.c
  synoptiques.c
  syn_motifs.c
  ups.c
  teleinfoedf.c
)

target_include_directories(abls-habitat-api PRIVATE
  ${GLIB_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIRS}
  ${GTHREAD_INCLUDE_DIRS}
  ${JSON_GLIB_INCLUDE_DIRS}
  ${LIBSOUP_INCLUDE_DIRS}
  ${LIBJWT_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
  ${MOSQUITTO_INCLUDE_DIRS}
  ${MEMCACHED_INCLUDE_DIRS}
)

target_compile_options(abls-habitat-api PRIVATE
  ${MARIADB_CFLAGS_LIST}
  -Wno-discarded-qualifiers
)

target_link_libraries(abls-habitat-api PRIVATE
  tradDLS
  ${GLIB_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
  ${LIBSOUP_LIBRARIES}
  ${LIBJWT_LIBRARIES}
  ${UUID_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
  ${MEMCACHED_LIBRARIES}
  ${MARIADB_LIBS_LIST}
)

# Installation
include(GNUInstallDirs)

install(TARGETS abls-habitat-api
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(PROGRAMS abls-commit-plugin.sh
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
  abls-habitat-api.service
  abls-habitat-api.conf.sample
  mosquitto.conf
  abls-commit-plugin.sh
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
)

install(FILES abls-habitat-api.service
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system
)

# ldconfig post-install
install(CODE "execute_process(COMMAND /sbin/ldconfig)")
