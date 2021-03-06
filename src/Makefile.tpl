[+ AutoGen5 template am +]
[+ CASE (suffix) +][+
   ==  am  +]# autogenerated from [+ (def-file) +] and [+ (tpl-file) +]
plugin_LTLIBRARIES = [+ FOR plugin " " +]libgstladspavid[+plugin_name+].la[+ ENDFOR plugin +]

[+ FOR plugin " " +]
libgstladspavid[+plugin_name+]_la_SOURCES = gstladspavid[+plugin_name+].c gstladspavid[+plugin_name+].h ladspa_wrapper.c ladspa_wrapper.h

libgstladspavid[+plugin_name+]_la_CFLAGS = $(GST_CFLAGS)
libgstladspavid[+plugin_name+]_la_LIBADD = $(GST_LIBS)
libgstladspavid[+plugin_name+]_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstladspavid[+plugin_name+]_la_LIBTOOLFLAGS = --tag=disable-static

gstladspavid[+plugin_name+].def:
	:> gstladspavid[+plugin_name+].def
	echo "AutoGen Definitions gstladspavid;" >> gstladspavid[+plugin_name+].def
	echo "name_lower = [+plugin_name+];" >> gstladspavid[+plugin_name+].def
	echo "name_upper = [+ (string-upcase (get "plugin_name")) +];" >> gstladspavid[+plugin_name+].def
	echo "name_class = [+ (string-capitalize (get "plugin_name")) +];" >> gstladspavid[+plugin_name+].def
	echo "name_so = [+plugin_so+];" >> gstladspavid[+plugin_name+].def

[+ ENDFOR plugin +]

# headers we need but don't want installed
noinst_HEADERS = gstladspavidflanger.h ladspa_wrapper.h gstladspavidcrossoverdist.h

# autogen
%.c:	%.def
	autogen $<

bin_PROGRAMS = video
video_SOURCES = video.c

video_CFLAGS = $(GST_CFLAGS)
video_LDADD = $(GST_LIBS)
video_LDFLAGS = $(GST_LDFLAGS)
# video_LIBTOOLFLAGS = [+ ESAC +]
