dnl Process this file with autoconf to produce a configure script.

AC_INIT([Abls-Habitat-API], [4.x],
        [sebastien.lefevre@abls-habitat.fr], [Abls-Habitat-API], [https://docs.abls-habitat.fr] )

AC_DEFINE_UNQUOTED([ABLS_API_VERSION], "[m4_esyscmd_s([sh -c "git describe --tags | tr -d '\n'"])]", [Numero de version])

AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([Http.c])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CONFIG_MACRO_DIR([m4])
AC_ISC_POSIX
AM_PROG_CC_C_O

PKG_PROG_PKG_CONFIG

AC_SUBST([systemdsystemunitdir], $($PKG_CONFIG --variable=systemdsystemunitdir systemd) )
AC_SUBST([systemduserunitdir], $($PKG_CONFIG --variable=systemduserunitdir systemd) )
AC_SUBST([sudoersdir], "/etc/sudoers.d" )
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

AM_DISABLE_STATIC
AC_PROG_LIBTOOL

AM_PATH_GLIB_2_0( 2.2.2 )
GLIB_GSETTINGS

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(glib.h fcntl.h sys/file.h sys/time.h syslog.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_CHECK_FUNCS(gettimeofday select socket strdup strerror)

dnl parametres par defaut
CFLAGS="-Wall -Wno-unused-result -Wno-comment -Wno-pointer-sign -Wno-discarded-qualifiers -pipe -g -ggdb -fdiagnostics-color"

ALL_LINGUAS=""

AC_OUTPUT( \
Makefile \
)
