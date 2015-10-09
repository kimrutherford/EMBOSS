## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libacd.la

libacd_la_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS) $(JAVA_CFLAGS)

libacd_la_CPPFLAGS  = -I$(top_srcdir)/ajax/ajaxdb
libacd_la_CPPFLAGS += -I$(top_srcdir)/ajax/ensembl
libacd_la_CPPFLAGS += -I$(top_srcdir)/ajax/graphics
libacd_la_CPPFLAGS += -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
libacd_la_CPPFLAGS += -I$(top_srcdir)/ajax/expat
libacd_la_CPPFLAGS += -I$(top_srcdir)/ajax/zlib
endif
libacd_la_CPPFLAGS += -I$(top_srcdir)/plplot
libacd_la_CPPFLAGS += $(JAVA_CPPFLAGS)

libacd_la_LDFLAGS  = -version-info 6:4:0
if ISCYGWIN
libacd_la_LDFLAGS += -no-undefined
libacd_la_LDFLAGS += -Wl,--add-stdcall-alias
libacd_la_LDFLAGS += -Wl,--export-all-symbols
libacd_la_LDFLAGS += -Wl,--enable-auto-import
libacd_la_LDFLAGS += -L../../plplot
libacd_la_LDFLAGS += -L../pcre
libacd_la_LDFLAGS += -L../expat
libacd_la_LDFLAGS += -L../zlib
libacd_la_LDFLAGS += -L../core
libacd_la_LDFLAGS += -L../graphics
libacd_la_LDFLAGS += -L../ensembl
libacd_la_LDFLAGS += -L../ajaxdb
libacd_la_LDFLAGS += -lajaxdb
libacd_la_LDFLAGS += -lensembl
libacd_la_LDFLAGS += -lajaxg
libacd_la_LDFLAGS += -lajax
libacd_la_LDFLAGS += -leplplot
libacd_la_LDFLAGS += -leexpat
libacd_la_LDFLAGS += -lepcre
libacd_la_LDFLAGS += -lezlib
endif

libacd_la_SOURCES = ajacd.c

include_HEADERS = ajacd.h
