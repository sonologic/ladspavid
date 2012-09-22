plugin_LTLIBRARIES = libgstladspavid.la

# sources used to compile this plug-in
libgstladspavid_la_SOURCES = gstladspavid.c gstladspavid.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstladspavid_la_CFLAGS = $(GST_CFLAGS)
libgstladspavid_la_LIBADD = $(GST_LIBS)
libgstladspavid_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstladspavid_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstladspavid.h
