plugin_LTLIBRARIES = libgstladspavidflanger.la 

# sources used to compile this plug-in
libgstladspavidflanger_la_SOURCES = gstladspavidflanger.c gstladspavidflanger.h ladspa_wrapper.c ladspa_wrapper.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstladspavidflanger_la_CFLAGS = $(GST_CFLAGS)
libgstladspavidflanger_la_LIBADD = $(GST_LIBS)
libgstladspavidflanger_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstladspavidflanger_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstladspavidflanger.h ladspa_wrapper.h
