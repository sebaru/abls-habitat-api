#******************************************************************************************************************************/
#* Makefile.am          Profil de compilation DLS pour l'interface API ABLS                                                   */
#* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                                    */
#* Auteur: LEFEVRE Sebastien                                                                              15.02.2022 21:20:44 */
#******************************************************************************************************************************/

noinst_LTLIBRARIES    = libtradDLS.la
libtradDLS_la_SOURCES = lignes.y mots.l Interp.c options.c conditions.c actions.c
libtradDLS_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                        `mariadb_config --libs`
libtradDLS_la_CFLAGS  = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                        `mariadb_config --cflags`

AM_LFLAGS = -Cfa
AM_YFLAGS = -p DlsScanner_ -t -v -Wcounterexamples
AM_CPPFLAGS=-I$(top_srcdir)/Include

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = mots.h lignes.h
CLEANFILES = mots.h lignes.h lignes.c mots.c
