#******************************************************************************************************************************/
#* Makefile.am          Profil de compilation de l'interface API Abls                                                         */
#* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                                    */
#* Auteur: LEFEVRE Sebastien                                                                              15.02.2022 21:20:44 */
#******************************************************************************************************************************/

abls-habitat-api($EXEEXT): libtradDLS.la abls-habitat-api

noinst_LTLIBRARIES    = libtradDLS.la
libtradDLS_la_SOURCES = lignes.h lignes.y mots.l Interp.c
libtradDLS_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4` \
                        `mysql_config --libs`
libtradDLS_la_CFLAGS  = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-2.4` \
                        `mysql_config --cflags`

bin_PROGRAMS = abls-habitat-api
abls_habitat_api_SOURCES = libtradDLS.la \
                           Http.c Erreur.c json.c database.c domains.c websocket.c agent.c icons.c status.c visuel.c thread.c \
                           users.c uuid.c mnemos.c archive.c imsgs.c smsg.c dls.c \
                           mnemo_BI.c mnemo_MONO.c mnemo_DI.c mnemo_HORLOGE.c mnemo_DO.c mnemo_AI.c mnemo_AO.c \
                           mnemo_TEMPO.c mnemo_REGISTRE.c \
                           modbus.c mail.c audio.c mapping.c synoptiques.c ups.c teleinfoedf.c
#nodist_abls_habitat_api_SOURCES = lignes.h
#abls_habitat_api_LDADD   = libtradDLS.la            

abls_habitat_api_LDFLAGS = `pkg-config --libs glib-2.0 openssl gthread-2.0 json-glib-1.0 libsoup-2.4 libjwt uuid` \
                           `mysql_config --libs`

abls_habitat_api_CFLAGS = `pkg-config --cflags glib-2.0 openssl gthread-2.0 json-glib-1.0 libsoup-2.4 libjwt uuid` \
                          `mysql_config --cflags`

AM_LFLAGS = -Cfa
AM_YFLAGS = -p DlsScanner_ -t -v -Wcounterexamples
AM_CPPFLAGS=-I$(top_srcdir)/Include
#            -I$(top_srcdir)/Watchdogd/Include \
#            -DWTD_PKGDATADIR=\"$(pkgdatadir)\"

#sysconf_DATA = watchdogd.abls.conf.sample
#pkgdata_DATA = init_db.sql base_icones.sql
dist_systemdsystemunit_DATA = abls-habitat-api.service

EXTRA_DIST = abls-habitat-api.service abls-habitat-api.conf.sample
MAINTAINERCLEANFILES = Makefile.in
ACLOCAL_AMFLAGS = -I m4
CLEANFILES = mots.h lignes.h lignes.c mots.c
