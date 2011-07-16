## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajax.la

AM_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS)

if ISCYGWIN
CYGWIN_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias \
-Wl,--export-all-symbols -Wl,--enable-auto-import
CYGWIN_LDAJAX = -L../../plplot -L../expat -L../pcre -L../zlib \
-leplplot -leexpat -lepcre -lezlib
endif

if !ESYSTEMLIBS
EXTRA_INCLUDES = -I$(top_srcdir)/ajax/expat -I$(top_srcdir)/ajax/zlib
endif


AM_CPPFLAGS = -DAJAX_FIXED_ROOT=$(AJAX_FIXED_ROOT) \
-DAJAX_SYSTEM="$(AJAX_SYSTEM)" -DPREFIX=\"$(prefix)\" \
$(MYSQL_CPPFLAGS) $(POSTGRESQL_CPPFLAGS) \
-I. -I$(top_srcdir)/plplot -I$(top_srcdir)/ajax/pcre $(EXTRA_INCLUDES)

AJAXSRC = ajalign.c ajarr.c \
ajassem.c ajassemread.c ajassemwrite.c \
ajassert.c \
ajbase.c ajcall.c ajcod.c \
ajdan.c ajdmx.c ajdom.c ajdomain.c \
ajexcept.c \
ajfeat.c ajfeatread.c ajfeatwrite.c \
ajfile.c ajfiledata.c ajfileio.c ajfmt.c \
ajhttp.c ajindex.c ajjava.c ajlist.c \
ajmath.c ajmatrices.c ajmem.c ajmess.c \
ajnam.c ajnexus.c \
ajobo.c ajoboread.c ajobowrite.c \
ajpat.c ajpdb.c ajpdbio.c ajphylo.c \
ajquery.c \
ajrange.c ajreg.c ajreport.c \
ajresource.c ajresourceread.c ajresourcewrite.c \
ajseq.c ajseqabi.c ajseqbam.c ajseqread.c ajseqtype.c ajseqwrite.c \
ajsort.c ajsql.c ajstr.c ajsys.c \
ajtable.c \
ajtax.c ajtaxread.c ajtaxwrite.c \
ajtext.c ajtextread.c ajtextwrite.c \
ajtime.c ajtranslate.c ajtree.c \
ajurl.c ajurlread.c ajurlwrite.c \
ajutil.c \
ajvar.c ajvarread.c ajvarwrite.c \
ajvector.c

include_HEADERS = ajax.h ajarch.h ajdefine.h \
ajalign.h \
ajarr.h \
ajassem.h ajassemdata.h ajassemread.h ajassemwrite.h \
ajassert.h \
ajbase.h ajcall.h ajcod.h \
ajdan.h ajdmx.h ajdom.h ajdomain.h ajexcept.h \
ajfeat.h ajfeatdata.h ajfeatread.h ajfeatwrite.h \
ajfile.h ajfiledata.h ajfileio.h ajfmt.h \
ajhttp.h ajindex.h ajjava.h ajlist.h \
ajmath.h ajmatrices.h ajmem.h ajmess.h \
ajnam.h ajnexus.h \
ajobo.h ajobodata.h ajoboread.h ajobowrite.h \
ajpat.h ajpdb.h ajpdbio.h ajphylo.h \
ajquery.h ajquerydata.h \
ajrange.h ajreg.h ajreport.h \
ajresource.h ajresourcedata.h ajresourceread.h ajresourcewrite.h \
ajseq.h ajseqabi.h ajseqbam.h ajseqdata.h ajseqread.h ajseqtype.h ajseqwrite.h \
ajsort.h ajsql.h ajstr.h ajsys.h \
ajtable.h \
ajtax.h ajtaxdata.h ajtaxread.h ajtaxwrite.h \
ajtext.h ajtextdata.h ajtextread.h ajtextwrite.h \
ajtime.h ajtranslate.h ajtree.h \
ajurl.h ajurldata.h ajurlread.h ajurlwrite.h \
ajutil.h \
ajvar.h ajvardata.h ajvarread.h ajvarwrite.h \
ajvector.h

libajax_la_SOURCES = $(AJAXSRC)

VERS_INF = -version-info 6:4:0

if ISCYGWIN
LINKFLAGS = $(VERS_INF) $(CYGWIN_LDFLAGS) $(CYGWIN_LDAJAX)
else
LINKFLAGS = $(VERS_INF)
endif

libajax_la_LDFLAGS = $(LINKFLAGS) $(MYSQL_LDFLAGS) $(POSTGRESQL_LDFLAGS)
