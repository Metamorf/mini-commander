builderdir=@builderdir@
builder_DATA = mini-commander.ui

BITMAPS = 			\
	browser-mini.xpm	\
	history-mini.xpm

desktop_iconsdir = ${datadir}/icons/hicolor/48x48/apps
desktop_icons_DATA = gnome-mini-commander.png


serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_MiniCommanderApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBDIR\@|$(libdir)|" $< > $@

CLEANFILES = $(server_in_files) $(server_DATA) $(schemas_DATA)

schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = mini-commander.schemas.in mini-commander-global.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_MiniCommanderApplet.xml

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA) ;
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(top_builddir)/src/mc-install-default-macros

@INTLTOOL_SERVER_RULE@


EXTRA_DIST =					\
	GNOME_MiniCommanderApplet.server.in.in	\
	$(desktop_icons_DATA)			\
	$(BITMAPS)				\
	$(ui_DATA)				\
	$(builder_DATA)				\
	$(schemas_in_files)
