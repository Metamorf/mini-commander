# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(MINI_COMMANDER, 2.23.0, email@michaelwood.me.uk)
AC_CONFIG_SRCDIR([src/macro.h])
AC_CONFIG_MACRO_DIR(m4)
AM_GCONF_SOURCE_2

AC_CONFIG_HEADER([config.h])

pkgdatadir="${datadir}/mini_commander"
AC_SUBST(pkgdatadir)
builderdir="${pkgdatadir}/builder"
AC_SUBST(builderdir)


AC_DEFINE_DIR(DATADIR, "${datadir}", [datadir])
AC_DEFINE_DIR(GNOMELOCALEDIR, "${datadir}/locale", [locale directory])
AC_DEFINE_DIR(SYSCONFDIR, "${sysconfdir}", [sysconfdir])
AC_DEFINE_DIR(LIBDIR, "${libdir}", [libdir])
AC_DEFINE_DIR(PREFIX, "$prefix", [install prefix])
AC_DEFINE_DIR(GTK_BUILDERDIR, "${builderdir}", [GtkBuilder Resources Directory])


AM_INIT_AUTOMAKE()

IT_PROG_INTLTOOL([0.35.0])

dnl i18n support
GETTEXT_PACKAGE=mini-commander
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AC_SUBST(LIBDIR)
AM_GLIB_GNU_GETTEXT

# Checks for programs.
AC_PROG_CC

AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
if test x"$GCONFTOOL" = xno; then
  AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([regcomp strstr])



GTK_DOC_CHECK([1.4])
PKG_CHECK_MODULES(MINI_COMMANDER, gtk+-2.0 >= 2.6.0 gconf-2.0 libpanelapplet-2.0 >= 2.13.4 libgnomeui-2.0)

AC_CONFIG_FILES([
	Makefile
	data/Makefile
	help/Makefile
	po/Makefile.in
	src/Makefile
	])
AC_OUTPUT
