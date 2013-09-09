## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajaxdb.la

libajaxdb_la_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS) $(JAVA_CFLAGS)

libajaxdb_la_CPPFLAGS  = -I$(top_srcdir)/ajax/ensembl
libajaxdb_la_CPPFLAGS += -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
libajaxdb_la_CPPFLAGS += -I$(top_srcdir)/ajax/expat
libajaxdb_la_CPPFLAGS += -I$(top_srcdir)/ajax/zlib
endif
libajaxdb_la_CPPFLAGS += -I$(top_srcdir)/plplot
libajaxdb_la_CPPFLAGS += $(AXIS2C_CPPFLAGS)
libajaxdb_la_CPPFLAGS += $(JAVA_CPPFLAGS)

libajaxdb_la_LDFLAGS  = -version-info 6:5:0
if ISCYGWIN
libajaxdb_la_LDFLAGS += -no-undefined
libajaxdb_la_LDFLAGS += -Wl,--add-stdcall-alias
libajaxdb_la_LDFLAGS += -Wl,--export-all-symbols
libajaxdb_la_LDFLAGS += -Wl,--enable-auto-import
libajaxdb_la_LDFLAGS += -L../../plplot
libajaxdb_la_LDFLAGS += -L../pcre
libajaxdb_la_LDFLAGS += -L../expat
libajaxdb_la_LDFLAGS += -L../zlib
libajaxdb_la_LDFLAGS += -L../core
libajaxdb_la_LDFLAGS += -L../ensembl
libajaxdb_la_LDFLAGS += -lensembl
libajaxdb_la_LDFLAGS += -lajax
libajaxdb_la_LDFLAGS += -leplplot
libajaxdb_la_LDFLAGS += -leexpat
libajaxdb_la_LDFLAGS += -lepcre
libajaxdb_la_LDFLAGS += -lezlib
endif
libajaxdb_la_LDFLAGS += $(AXIS2C_LDFLAGS)

libajaxdb_la_SOURCES = \
ajassemdb.c \
ajdas.c \
ajfeatdb.c \
ajmart.c \
ajobodb.c \
ajrefseqdb.c \
ajresourcedb.c \
ajseqdb.c \
ajsoap.c \
ajtaxdb.c \
ajtextdb.c \
ajurldb.c \
ajvardb.c \
ajxmldb.c

include_HEADERS = \
ajassemdb.h \
ajdas.h \
ajfeatdb.h \
ajmart.h \
ajobodb.h \
ajrefseqdb.h \
ajresourcedb.h \
ajseqdb.h \
ajsoap.h \
ajtaxdb.h \
ajtextdb.h \
ajurldb.h \
ajvardb.h \
ajxmldb.h
