#!/bin/sh
# you can either set the environment variables AUTOCONF, AUTOHEADER, AUTOMAKE,
# ACLOCAL, AUTOPOINT and/or LIBTOOLIZE to the right versions, or leave them
# unset and get the defaults

cd src
autogen Makefile.def || {
 echo 'autogen Makefile.def failed';
 exit 1;
}
cd ..

autoreconf --verbose --force --install --make || {
 echo 'autogen.sh failed';
 exit 1;
}

./configure || {
 echo 'configure failed';
 exit 1;
}

echo
echo "Now type 'make' to compile this module."
echo
