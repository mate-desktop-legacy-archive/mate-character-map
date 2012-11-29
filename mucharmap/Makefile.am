## Copyright © 2003  Noah Levitt
## Copyright © 2008  Christian Persch
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
##

NULL =

lib_LTLIBRARIES = libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@.la

BUILT_SOURCES = \
	mucharmap-marshal.c \
	mucharmap-marshal.h \
	mucharmap-type-builtins.c \
	mucharmap-type-builtins.h \
	$(NULL)

libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@_la_SOURCES = \
	mucharmap-block-chapters-model.c \
	mucharmap-block-chapters-model.h \
	mucharmap-block-codepoint-list.c \
	mucharmap-block-codepoint-list.h \
	mucharmap-chapters-model.c \
	mucharmap-chapters-model.h \
	mucharmap-chapters-view.c \
	mucharmap-chapters-view.h \
	mucharmap-charmap.c \
	mucharmap-charmap.h \
	mucharmap-chartable-accessible.c \
	mucharmap-chartable-accessible.h \
	mucharmap-chartable-cell-accessible.c \
	mucharmap-chartable-cell-accessible.h \
	mucharmap-chartable.c \
	mucharmap-chartable.h \
	mucharmap-codepoint-list.c \
	mucharmap-codepoint-list.h \
	mucharmap-init.c \
	mucharmap-private.h \
	mucharmap-script-chapters-model.c \
	mucharmap-script-chapters-model.h \
	mucharmap-script-codepoint-list.c \
	mucharmap-script-codepoint-list.h \
	mucharmap-unicode-info.c \
	mucharmap-unicode-info.h \
	unicode-blocks.h \
	unicode-categories.h \
	unicode-names.h \
	unicode-nameslist.h \
	unicode-scripts.h \
	unicode-unihan.h \
	unicode-versions.h \
	$(NULL)

nodist_libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@_la_SOURCES = $(BUILT_SOURCES)

libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@_la_CPPFLAGS = \
	-I$(top_srcdir) \
	-DMUCHARMAP_COMPILATION \
	-DDATADIR=\"$(datadir)\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DHELPDIR=\"$(HELP_DIR)\" \
	-DG_DISABLE_SINGLE_INCLUDES \
	-DPANGO_DISABLE_SINGLE_INCLUDES \
	-DATK_DISABLE_SINGLE_INCLUDES \
	-DGDK_DISABLE_SINGLE_INCLUDES \
	-DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES \
	-DGTK_DISABLE_SINGLE_INCLUDES \
	$(DISABLE_DEPRECATED) \
	$(AM_CPPFLAGS)

libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@_la_CFLAGS = \
	$(GTK_CFLAGS)	\
	$(WARN_CFLAGS)	\
	$(AM_CFLAGS)

libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@_la_LDFLAGS = \
	-version-info $(LIBMUCHARMAP_LT_VERSION)	\
	-export-symbols-regex "^mucharmap_*"		\
	$(AM_LDFLAGS)

libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@_la_LIBADD = \
	$(GTK_LIBS)	\
	$(INTLLIBS)

libmucharmapincludedir = $(includedir)/mucharmap-$(MUCHARMAP_API_VERSION)/mucharmap
libmucharmapinclude_HEADERS = \
	mucharmap-block-chapters-model.h \
	mucharmap-block-codepoint-list.h \
	mucharmap-chapters-model.h \
	mucharmap-chapters-view.h \
	mucharmap-charmap.h \
	mucharmap-chartable.h \
	mucharmap-codepoint-list.h \
	mucharmap.h \
	mucharmap-script-chapters-model.h \
	mucharmap-script-codepoint-list.h \
	mucharmap-unicode-info.h \
	$(NULL)

nodist_libmucharmapinclude_HEADERS = \
	mucharmap-type-builtins.h \
	$(NULL)

if ENABLE_CHARMAP
bin_PROGRAMS = mucharmap
else
noinst_PROGRAMS = mucharmap
endif

mucharmap_SOURCES = \
	mucharmap-mini-fontsel.c \
	mucharmap-mini-fontsel.h \
	mucharmap-print-operation.c \
	mucharmap-print-operation.h \
	mucharmap-search-dialog.c \
	mucharmap-search-dialog.h \
	mucharmap-settings.c \
	mucharmap-settings.h \
	mucharmap-window.c \
 	mucharmap-window.h \
	main.c \
	$(NULL)

mucharmap_CPPFLAGS = \
	-I$(top_srcdir) \
	-DDATADIR=\"$(datadir)\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DHELPDIR=\"$(HELP_DIR)\" \
	-DG_DISABLE_SINGLE_INCLUDES \
	-DPANGO_DISABLE_SINGLE_INCLUDES \
	-DATK_DISABLE_SINGLE_INCLUDES \
	-DGDK_DISABLE_SINGLE_INCLUDES \
	-DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES \
	-DGTK_DISABLE_SINGLE_INCLUDES \
	$(DISABLE_DEPRECATED) \
	$(AM_CPPFLAGS)

mucharmap_CFLAGS = \
	$(GTK_CFLAGS)	\
	$(MATECONF_CFLAGS)	\
	$(WARN_CFLAGS)	\
	$(AM_CFLAGS)

mucharmap_LDFLAGS = \
	$(AM_LDFLAGS)

mucharmap_LDADD = \
	libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@.la	\
	$(GTK_LIBS)	\
	$(MATECONF_LIBS)	\
	$(INTL_LIBS)

if OS_WIN32
mucharmap_LDADD += mucharmaprc.o
mucharmaprc.o: mucharmaprc.rc
	$(AM_V_GEN) windres -i $< -o $@
endif

mucharmap-marshal.h: $(srcdir)/mucharmap-marshal.list
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=_mucharmap_marshal $(srcdir)/mucharmap-marshal.list \
                            --header \
                            --internal > mucharmap-marshal.h.tmp \
	  && mv mucharmap-marshal.h.tmp mucharmap-marshal.h ) \
        || ( rm -f mucharmap-marshal.h.tmp && exit 1 )

mucharmap-marshal.c: $(srcdir)/mucharmap-marshal.list
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=_mucharmap_marshal $(srcdir)/mucharmap-marshal.list \
			    --header \
                            --body \
                            --internal > mucharmap-marshal.c.tmp \
          && mv mucharmap-marshal.c.tmp mucharmap-marshal.c ) \
        || ( rm -f mucharmap-marshal.c.tmp && exit 1 )


mucharmap-type-builtins.h: stamp-mucharmap-type-builtins.h
	@true

stamp-mucharmap-type-builtins.h: mucharmap-type-builtins.h.template $(libmucharmapinclude_HEADERS)
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-gtbh \
	&& (cmp -s xgen-gtbh mucharmap-type-builtins.h || cp xgen-gtbh mucharmap-type-builtins.h ) \
	&& rm -f xgen-gtbh \
	&& echo timestamp > $(@F)

mucharmap-type-builtins.c: mucharmap-type-builtins.c.template $(libmucharmapinclude_HEADERS)
	$(AM_V_GEN) $(GLIB_MKENUMS) --template $< $(filter-out $<,$^) > xgen-gtbc \
	&& (cmp -s xgen-gtbc mucharmap-type-builtins.c || cp xgen-gtbc mucharmap-type-builtins.c ) \
	&& rm -f xgen-gtbc

# printoptions.h: print.glade
# 	$(AM_V_GEN) $(GTK_BUILDER_CONVERT) --root custom_tab_container $< - | \
# 		$(XMLLINT) --noblanks - | \
# 		$(HEXDUMP) -v -e '"" 1/1 "0x%02x,""\n"' > xgen-$(@F) \
# 	&& (cmp -s xgen-$(@F) $@ || cp -f xgen-$(@F) $@) \
# 	&& rm -f xgen-$(@F)

if ENABLE_INTROSPECTION

girdir = $(GIRDIR)
gir_DATA = Mucharmap-$(MUCHARMAP_API_VERSION).gir

typelibdir = $(GIRTYPELIBDIR)
typelib_DATA = Mucharmap-$(MUCHARMAP_API_VERSION).typelib

Mucharmap-$(MUCHARMAP_API_VERSION).gir: $(G_IR_SCANNER) libmucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@.la $(libmucharmapinclude_HEADERS) Makefile
	$(AM_V_GEN) PKG_CONFIG_PATH=$(top_builddir):$$PKG_CONFIG_PATH \
	$(G_IR_SCANNER) \
		--verbose \
		--namespace=Mucharmap \
		--nsversion=$(MUCHARMAP_API_VERSION) \
		--include=GLib-2.0 \
		--include=GObject-2.0 \
		--include=Gtk-$(GTK_API_VERSION) \
		--library=mucharmap@MUCHARMAP_LIBRARY_SUFFIX_U@ \
		--libtool="$(LIBTOOL)" \
		--output $@ \
		--pkg mucharmap-$(MUCHARMAP_API_PC_VERSION) \
		-I$(top_srcdir) \
		-I$(top_builddir) \
		-DMUCHARMAP_COMPILATION \
		$(filter %.h,$^)

%.typelib: %.gir $(G_IR_COMPILER)
	$(AM_V_GEN) $(G_IR_COMPILER) --includedir=$(srcdir) --includedir=. $(G_IR_COMPILER_OPTS) $< -o $@

endif # ENABLE_INTROSPECTION

EXTRA_DIST = \
	mucharmap-marshal.list \
	mucharmap-type-builtins.c.template \
	mucharmap-type-builtins.h.template \
	mucharmaprc.rc \
	$(NULL)

CLEANFILES = \
	stamp-mucharmap-type-builtins.h \
	$(BUILT_SOURCES) \
	$(gir_DATA) \
	$(typelib_DATA) \
	$(NULL)

if ENABLE_CHARMAP

# don't do the symlink stuff if this is windows
if !OS_WIN32
# install a symlink to the binary called "charmap"
install-exec-hook:
	test -e "$(DESTDIR)$(bindir)/charmap" \
        || (cd "$(DESTDIR)$(bindir)" && $(LN_S) mucharmap charmap) ; \
	test -e "$(DESTDIR)$(bindir)/mate-character-map" \
        || (cd "$(DESTDIR)$(bindir)" && $(LN_S) mucharmap mate-character-map)

# remove bin/charmap if it's a symlink (it's a pain to test what it's
# a symlink to)
uninstall-local:
	test -h "$(DESTDIR)$(bindir)/charmap" \
	&& rm -f "$(DESTDIR)$(bindir)/charmap" ; \
	test -h "$(DESTDIR)$(bindir)/mate-character-map" \
	&& rm -f "$(DESTDIR)$(bindir)/mate-character-map"

endif # !OS_WIN32

endif # ENABLE_CHARMAP

-include $(top_srcdir)/git.mk
