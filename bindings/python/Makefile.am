NULL =

# FIXMEchpe: keep this in sync with mucharmap/Makefile.am !
srcdirheaders = \
	mucharmap-block-chapters-model.h \
	mucharmap-block-codepoint-list.h \
	mucharmap-chapters-model.h \
	mucharmap-chapters-view.h \
	mucharmap-charmap.h \
	mucharmap-chartable.h \
	mucharmap-codepoint-list.h \
	mucharmap-script-chapters-model.h \
	mucharmap-script-codepoint-list.h \
	mucharmap-unicode-info.h \
	$(NULL)
builddirheaders = \
	mucharmap.h \
	mucharmap-type-builtins.h \
	$(NULL)

defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = mucharmap.defs

pkgpyexecdir = $(pyexecdir)/gtk-2.0
pkgpyexec_LTLIBRARIES = mucharmap.la

BUILT_SOURCES = \
	mucharmap.c \
	$(NULL)

mucharmap_la_SOURCES = \
	mucharmap.c \
	mucharmapmodule.c \
	$(NULL)

mucharmap_la_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(PYTHON_INCLUDES) \
	$(DISABLE_DEPRECATED) \
	$(AM_CPPFLAGS)

mucharmap_la_CFLAGS = \
	$(NO_STRICT_ALIASING_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(GTK_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS)

mucharmap_la_LIBADD = \
	$(top_builddir)/mucharmap/libmucharmap.la \
	$(PYGTK_LIBS) \
	$(PYTHON_LIB_LOC) \
	$(PYTHON_LIBS) \
	$(PYTHON_EXTRA_LIBS)

mucharmap_la_LDFLAGS = \
	-module \
	-avoid-version	\
	-export-symbols-regex initmucharmap \
	$(AM_LDFLAGS)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(NULL)

EXTRA_DIST = \
	mucharmap.defs \
	mucharmap.override \
	$(NULL)

mucharmap.c: mucharmap.defs mucharmap.override

.defs.c:
	$(AM_V_GEN) (cd $(srcdir)\
	 && $(PYGTK_CODEGEN) \
	    --override $*.override \
	    --prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c

regenerate-python-binding:
	$(AM_V_GEN) $(PYGTK_H2DEF) \
		$(sort $(addprefix $(top_srcdir)/mucharmap/,$(srcdirheaders)) \
		       $(addprefix $(top_builddir)/mucharmap/,$(builddirheaders))) \
		> mucharmap.defs.new

-include $(top_srcdir)/git.mk
