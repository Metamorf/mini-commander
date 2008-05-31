## Process this file with automake to produce Makefile.in

INCLUDES = 				\
	-I.				\
	-I$(srcdir)			\
	$(MINI_COMMANDER_CFLAGS)	\
	$(WARN_CFLAGS)			\
	$(GNOME_INCLUDEDIR)

libexec_PROGRAMS = mini_commander_applet

tooldir = $(libdir)/gnome-applets
tool_PROGRAMS = mc-install-default-macros

mini_commander_applet_SOURCES =			\
	about.c					\
	about.h					\
	cmd_completion.c			\
	cmd_completion.h			\
	command_line.c				\
	command_line.h				\
	exec.c					\
	exec.h					\
	mini-commander_applet.c			\
	mini-commander_applet.h			\
	help.c					\
	help.h					\
	history.c				\
	history.h				\
	macro.c					\
	macro.h					\
	preferences.c				\
	preferences.h				\
	mc-default-macros.h				\
	icon-entry.c				\
	icon-entry.h


mini_commander_applet_LDADD =	\
	$(MINI_COMMANDER_LIBS)

mc_install_default_macros_SOURCES =	\
	mc-install-default-macros.c	\
	mc-default-macros.h

mc_install_default_macros_LDADD =	\
	$(MINI_COMMANDER_LIBS)


##mw added
libexecdir	= $(libdir)/gnome-applets
##mw 

