plugin_LTLIBRARIES = libgstladspavidflanger.la libgstladspavidtest.la

# sources used to compile this plug-in
libgstladspavidflanger_la_SOURCES = gstladspavidflanger.c gstladspavidflanger.h ladspa_wrapper.c ladspa_wrapper.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstladspavidflanger_la_CFLAGS = $(GST_CFLAGS)
libgstladspavidflanger_la_LIBADD = $(GST_LIBS)
libgstladspavidflanger_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstladspavidflanger_la_LIBTOOLFLAGS = --tag=disable-static

# sources used to compile this plug-in
libgstladspavidtest_la_SOURCES = gstladspavidtest.c gstladspavidtest.h ladspa_wrapper.c ladspa_wrapper.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstladspavidtest_la_CFLAGS = $(GST_CFLAGS)
libgstladspavidtest_la_LIBADD = $(GST_LIBS)
libgstladspavidtest_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstladspavidtest_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstladspavidflanger.h ladspa_wrapper.h gstladspavidtest.h

# autogen
%.c:	%.def
	autogen $<

bin_PROGRAMS = video
video_SOURCES = video.c

video_CFLAGS = $(GST_CFLAGS)
video_LDADD = $(GST_LIBS)
video_LDFLAGS = $(GST_LDFLAGS)
# video_LIBTOOLFLAGS = 
