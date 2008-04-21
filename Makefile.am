## Process this file with automake to produce Makefile.in

SUBDIRS = src po help data


INTLTOOL_BUILT = \
        intltool-extract \
        intltool-merge \
        intltool-update

EXTRA_DIST = $(INTLTOOL_BUILT:=.in)

DISTCLEANFILES = $(INTLTOOL_BUILT)

