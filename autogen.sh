#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="mucharmap"

(test -f $srcdir/mucharmap/main.c) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

which mate-autogen || {
    echo "You need to install mate-common and make"
    echo "sure the mate-autogen.sh script is in your \$PATH."
    exit 1
}

which yelp-build || {
    echo "You need to install yelp-tools" 
    exit 1
}

REQUIRED_INTLTOOL_VERSION=0.40.4
REQUIRED_AUTOMAKE_VERSION=1.9

. mate-autogen
