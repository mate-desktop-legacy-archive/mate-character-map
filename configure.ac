# Copyright © 2003  Noah Levitt
# Copyright © 2007, 2008, 2009 Christian Persch
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

m4_define([mucharmap_version_major],[1])
m4_define([mucharmap_version_minor],[6])
m4_define([mucharmap_version_micro],[0])
m4_define([mucharmap_version_extra],[])
m4_define([mucharmap_version],[mucharmap_version_major.mucharmap_version_minor.mucharmap_version_micro()mucharmap_version_extra])

# Before making a release, the libtool version should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
m4_define([mucharmap_lt_current],[7])
m4_define([mucharmap_lt_revision],[0])
m4_define([mucharmap_lt_age],[0])
m4_define([mucharmap_lt_version_info],[mucharmap_lt_current:mucharmap_lt_revision:mucharmap_lt_age])
m4_define([mucharmap_lt_current_minus_age],[m4_eval(mucharmap_lt_current - mucharmap_lt_age)])

AC_PREREQ([2.56])

AC_INIT([MATE Character Map],[mucharmap_version],[https://github.com/mate-desktop/mate-character-map/issues/],[mucharmap])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([mucharmap/mucharmap.h.in])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign dist-xz no-dist-gzip])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

if test -z "$enable_maintainer_mode"; then
  enable_maintainer_mode=yes
fi
AM_MAINTAINER_MODE([enable])

# checks for progs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.40.0])
GLIB_GSETTINGS

AM_PROG_CC_C_O

# checks for stuff
MATE_COMMON_INIT
MATE_DEBUG_CHECK
MATE_COMPILE_WARNINGS([maximum])
#MATE_MAINTAINER_MODE_DEFINES

GLIB_REQUIRED=2.16.3
GIO_REQUIRED=2.25.5


# check for win32
case "$host" in
  *-*-mingw*) os_win32="yes" ;;
  *) os_win32="no" ;;
esac

if test "x$os_win32" = "xyes"
then
    CFLAGS="$CFLAGS -mms-bitfields -mwindows"
fi
AM_CONDITIONAL(OS_WIN32, test "x$os_win32" = "xyes")

# Which gtk+ API version to compile against

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=2.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0) GTK_API_VERSION=2.0
       GTK_REQUIRED=2.24.0
       MUCHARMAP_API_VERSION=2.0
       MUCHARMAP_API_MAJOR_VERSION=2
       MUCHARMAP_API_MINOR_VERSION=0
       MUCHARMAP_API_PC_VERSION=2
       MUCHARMAP_LIBRARY_SUFFIX=
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED=2.90.0
       MUCHARMAP_API_VERSION=2.90
       MUCHARMAP_API_MAJOR_VERSION=2
       MUCHARMAP_API_MINOR_VERSION=90
       MUCHARMAP_API_PC_VERSION=2.90
       MUCHARMAP_LIBRARY_SUFFIX="-$MUCHARMAP_API_VERSION"
       ;;
esac

AC_SUBST([GTK_API_VERSION])
AC_SUBST([MUCHARMAP_API_VERSION])
AC_SUBST([MUCHARMAP_API_MAJOR_VERSION])
AC_SUBST([MUCHARMAP_API_MINOR_VERSION])
AC_SUBST([MUCHARMAP_API_PC_VERSION])
AC_SUBST([MUCHARMAP_LIBRARY_SUFFIX_U],[AS_TR_SH([$MUCHARMAP_LIBRARY_SUFFIX])])

AM_CONDITIONAL([HAVE_GTK_2],[test "$with_gtk" = "2.0"])
AM_CONDITIONAL([HAVE_GTK_3],[test "$with_gtk" = "3.0"])


PKG_CHECK_MODULES([GTK],[
  glib-2.0 >= $GLIB_REQUIRED
  gio-2.0 >= $GIO_REQUIRED
  gtk+-$GTK_API_VERSION >= $GTK_REQUIRED
])

GLIB_GENMARSHAL="$($PKG_CONFIG --variable=glib_genmarshal glib-2.0)"
AC_SUBST([GLIB_GENMARSHAL])
GLIB_MKENUMS="$($PKG_CONFIG --variable=glib_mkenums glib-2.0)"
AC_SUBST([GLIB_MKENUMS])

GLIB_GSETTINGS

AC_MSG_CHECKING([whether to build the charmap programme])
AC_ARG_ENABLE([charmap],
  [AS_HELP_STRING([--disable-charmap],[disable building the charmap programme])],
  [],[enable_charmap=yes])
AC_MSG_RESULT([$enable_charmap])

AM_CONDITIONAL([ENABLE_CHARMAP],[test "$enable_charmap" = "yes"])

# checks for funcs
AC_CHECK_FUNCS([bind_textdomain_codeset])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AH_TEMPLATE([ENABLE_UNIHAN], [Define if you want CJK ideograph information])
AC_ARG_ENABLE(unihan,
              AC_HELP_STRING([--disable-unihan],
                             [don't build in CJK ideograph information]))
if test "x$enable_unihan" != "xno" ; then
    AC_DEFINE([ENABLE_UNIHAN],[1],[Define if you want CJK ideograph information])
fi

# Some utilities
AC_PROG_LN_S

# AC_PATH_PROG([GTK_BUILDER_CONVERT],[gtk-builder-convert],[false])
# if test "$GTK_BUILDER_CONVERT" = "false"; then
#   AC_MSG_ERROR([gtk-builder-convert not found])
# fi
#
# AC_PATH_PROG([XMLLINT],[xmllint],[false])
# if test "$XMLLINT" = "false"; then
#   AC_MSG_ERROR([xmllint not found])
# fi
#
# AC_PATH_PROG([HEXDUMP],[hexdump],[false])
# if test "$HEXDUMP" = "false"; then
#   AC_MSG_ERROR([hexdump not found])
# fi

# ****
# i18n
# ****

GETTEXT_PACKAGE=mucharmap
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[The mucharmap gettext domain])
AM_GLIB_GNU_GETTEXT

# ***************
# Python Bindings
# ***************

# Only support this on gtk+-2.0; on 3.0 you should just use gobject-introspection

if test "$with_gtk" = "2.0"; then

AC_MSG_CHECKING([whether python bindings are requested])
AC_ARG_ENABLE([python-bindings],
        AS_HELP_STRING([--enable-python-bindings],[Enable python bindings]),
        [],[enable_python_bindings=no])
AC_MSG_RESULT([$enable_python_bindings])

if test "$enable_python_bindings" = "yes"; then
  AM_PATH_PYTHON([2.4])
  AM_CHECK_PYTHON_HEADERS([],[AC_MSG_ERROR([Python headers not found])])

  PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
  PY_EXEC_PREFIX=`$PYTHON -c 'import sys ; print sys.exec_prefix'`
  PYTHON_LIBS="-lpython$PYTHON_VERSION"
  PYTHON_LIB_LOC="-L$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config"
  PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
  PYTHON_MAKEFILE="$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config/Makefile"
  PYTHON_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
  PYTHON_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
  PYTHON_EXTRA_LIBS="$PYTHON_BASEMODLIBS $PYTHON_OTHER_LIBS"
  AC_SUBST([PYTHON_LIBS])
  AC_SUBST([PYTHON_LIB_LOC])
  AC_SUBST([PYTHON_CFLAGS])
  AC_SUBST([PYTHON_EXTRA_LIBS])

  PYGTK_REQUIRED=2.7.1
  PKG_CHECK_MODULES([PYGTK],[pygtk-2.0 >= $PYGTK_REQUIRED])

  AC_SUBST([PYGTK_CFLAGS])
  AC_SUBST([PYGTK_LIBS])

  AC_MSG_CHECKING([for pygtk defs])
  PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
  AC_MSG_RESULT([$PYGTK_DEFSDIR])

  AC_MSG_CHECKING([for pygtk codegen])
  PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
  AC_MSG_RESULT([$PYGTK_CODEGEN])

  AC_MSG_CHECKING([for pygtk h2def])
  PYGTK_H2DEF="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/h2def.py"
  AC_MSG_RESULT([$PYGTK_H2DEF])

  AC_SUBST([PYGTK_DEFSDIR])
  AC_SUBST([PYGTK_CODEGEN])
  AC_SUBST([PYGTK_H2DEF])

  dnl Check for -fno-strict-aliasing
  FLAGS="-fno-strict-aliasing"
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $FLAGS"
  AC_MSG_CHECKING([whether [$]CC understands $FLAGS])
  AC_TRY_COMPILE([], [], [compiler_has_option=yes], [compiler_has_option=no])
  CFLAGS="$save_CFLAGS"
  AC_MSG_RESULT($compiler_has_option)
  if test $compiler_has_option = yes; then
          NO_STRICT_ALIASING_CFLAGS="$FLAGS"
  fi
  AC_SUBST([NO_STRICT_ALIASING_CFLAGS])
fi

else # with_gtk = 3.0
  enable_python_bindings=no
fi # with_gtk = 2.0

AM_CONDITIONAL([ENABLE_PYTHON_BINDINGS],[test "$enable_python_bindings" = "yes"])

# ***
# GIR
# ***

AC_MSG_CHECKING([whether GObject introspection is requested])
AC_ARG_ENABLE([introspection],
        AS_HELP_STRING([--enable-introspection],[Enable GObject introspection]),
        [],[enable_introspection=no])
AC_MSG_RESULT([$enable_introspection])

if test "$enable_introspection" = "yes"; then
  GOBJECT_INTROSPECTION_REQUIRED=0.6
  PKG_CHECK_MODULES([GOBJECT_INTROSPECTION],[gobject-introspection-1.0 >= $GOBJECT_INTROSPECTION_REQUIRED])

  G_IR_SCANNER="$($PKG_CONFIG --variable=g_ir_scanner gobject-introspection-1.0)"
  G_IR_COMPILER="$($PKG_CONFIG --variable=g_ir_compiler gobject-introspection-1.0)"
  G_IR_GENERATE="$($PKG_CONFIG --variable=g_ir_generate gobject-introspection-1.0)"
  GIRDIR="$($PKG_CONFIG --variable=girdir gobject-introspection-1.0)"
  GIRTYPELIBDIR="$($PKG_CONFIG --variable=typelibdir gobject-introspection-1.0)"
  AC_SUBST([G_IR_SCANNER])
  AC_SUBST([G_IR_COMPILER])
  AC_SUBST([G_IR_GENERATE])
  AC_SUBST([GIRDIR])
  AC_SUBST([GIRTYPELIBDIR])
fi

AM_CONDITIONAL([ENABLE_INTROSPECTION],[test "$enable_introspection" = "yes"])

# ***************
# API & User Docs
# ***************

YELP_HELP_INIT

GTK_DOC_CHECK([1.0])

# *****************************************************************************

AC_SUBST([MUCHARMAP_VERSION_MAJOR],[mucharmap_version_major])
AC_SUBST([MUCHARMAP_VERSION_MINOR],[mucharmap_version_minor])
AC_SUBST([MUCHARMAP_VERSION_MICRO],[mucharmap_version_micro])
AC_SUBST([LIBMUCHARMAP_LT_VERSION],[mucharmap_lt_version_info])
AC_SUBST([LIBMUCHARMAP_LT_CURRENT_MINUS_AGE],[mucharmap_lt_current_minus_age])

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
mucharmap.desktop.in
mucharmap.nsi
bindings/Makefile
bindings/python/Makefile
docs/Makefile
docs/reference/Makefile
mucharmap/Makefile
mucharmap/mucharmap.h
help/Makefile
pixmaps/Makefile
po/Makefile.in
])

AC_CONFIG_FILES([mucharmap-${MUCHARMAP_API_PC_VERSION}.pc:mucharmap.pc.in],[MUCHARMAP_API_PC_VERSION=$MUCHARMAP_API_PC_VERSION])

AC_OUTPUT
