#******************************************************************************************************************************/
#* Makefile.am          Profil de compilation de l'interface API Abls                                                         */
#* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                                    */
#* Auteur: LEFEVRE Sebastien                                                                              15.02.2022 21:20:44 */
#******************************************************************************************************************************/

SUBDIRS = TraductionDLS

bin_PROGRAMS = abls-habitat-api
abls_habitat_api_SOURCES = Http.c Erreur.c json.c database.c domains.c agent.c icons.c \
                           status.c visuel.c thread.c audit_log.c tableaux.c shelly.c mqtt.c \
                           users.c uuid.c mnemos.c archive.c imsgs.c smsg.c dls.c dls_package.c dls_params.c \
                           message.c dictionnaire.c histo.c meteo.c \
                           mnemo_BI.c mnemo_MONO.c mnemo_DI.c mnemo_HORLOGE.c mnemo_DO.c mnemo_AI.c mnemo_AO.c \
                           mnemo_TEMPO.c mnemo_REGISTRE.c mnemo_WATCHDOG.c mnemo_CI.c mnemo_CH.c mnemo_VISUEL.c \
                           modbus.c phidget.c mail.c audio.c mapping.c synoptiques.c syn_motifs.c ups.c teleinfoedf.c

abls_habitat_api_LDADD   = TraductionDLS/libtradDLS.la

abls_habitat_api_LDFLAGS = `pkg-config --libs glib-2.0 openssl gthread-2.0 json-glib-1.0 libsoup-3.0 libjwt uuid libmosquitto` \
                           `mariadb_config --libs`

abls_habitat_api_CFLAGS = `pkg-config --cflags glib-2.0 openssl gthread-2.0 json-glib-1.0 libsoup-3.0 libjwt uuid libmosquitto` \
                          `mariadb_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include -DABLS_API_PKGDATADIR=\"$(pkgdatadir)\"
dist_systemdsystemunit_DATA = abls-habitat-api.service

EXTRA_DIST = abls-habitat-api.service abls-habitat-api.conf.sample
pkgdata_DATA = abls-habitat-api.service abls-habitat-api.conf.sample mosquitto.conf
MAINTAINERCLEANFILES = Makefile.in
ACLOCAL_AMFLAGS = -I m4
