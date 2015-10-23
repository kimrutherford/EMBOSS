## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajax.la

libajax_la_CFLAGS = $(WARN_CFLAGS) $(DEVWARN_CFLAGS) $(JAVA_CFLAGS)

libajax_la_CPPFLAGS  = -I$(top_srcdir)/ajax/pcre
if !ESYSTEMLIBS
libajax_la_CPPFLAGS += -I$(top_srcdir)/ajax/expat
libajax_la_CPPFLAGS += -I$(top_srcdir)/ajax/zlib
endif
libajax_la_CPPFLAGS += -I$(top_srcdir)/plplot
libajax_la_CPPFLAGS += $(JAVA_CPPFLAGS)
libajax_la_CPPFLAGS += $(MYSQL_CPPFLAGS)
libajax_la_CPPFLAGS += $(POSTGRESQL_CPPFLAGS)
libajax_la_CPPFLAGS += -DAJAX_FIXED_ROOT=$(AJAX_FIXED_ROOT)
libajax_la_CPPFLAGS += -DAJAX_SYSTEM="$(AJAX_SYSTEM)"
libajax_la_CPPFLAGS += -DPREFIX=\"$(prefix)\"

libajax_la_LDFLAGS  = -version-info 6:5:0
libajax_la_LDFLAGS += $(JAVA_LDFLAGS)
libajax_la_LDFLAGS += $(MYSQL_LDFLAGS)
libajax_la_LDFLAGS += $(POSTGRESQL_LDFLAGS)
if ISCYGWIN
libajax_la_LDFLAGS += -no-undefined
libajax_la_LDFLAGS += -Wl,--add-stdcall-alias
libajax_la_LDFLAGS += -Wl,--export-all-symbols
libajax_la_LDFLAGS += -Wl,--enable-auto-import
libajax_la_LDFLAGS += -L../../plplot
libajax_la_LDFLAGS += -L../expat
libajax_la_LDFLAGS += -L../pcre
libajax_la_LDFLAGS += -L../zlib
libajax_la_LDFLAGS += -leplplot
libajax_la_LDFLAGS += -leexpat
libajax_la_LDFLAGS += -lepcre
libajax_la_LDFLAGS += -lezlib
endif

libajax_la_SOURCES = \
ajalign.c \
ajarr.c \
ajassem.c \
ajassemread.c \
ajassemwrite.c \
ajassert.c \
ajbamindex.c \
ajbase.c \
ajcall.c \
ajcigar.c \
ajcod.c \
ajdan.c \
ajdmx.c \
ajdom.c \
ajdomain.c \
ajexcept.c \
ajfeat.c \
ajfeatread.c \
ajfeatwrite.c \
ajfile.c \
ajfiledata.c \
ajfileio.c \
ajfmt.c \
ajftp.c \
ajhttp.c \
ajindex.c \
ajjava.c \
ajlist.c \
ajmath.c \
ajmatrices.c \
ajmem.c \
ajmess.c \
ajnam.c \
ajnexus.c \
ajobo.c \
ajoboread.c \
ajobowrite.c \
ajpat.c \
ajpdb.c \
ajpdbio.c \
ajphylo.c \
ajquery.c \
ajrange.c \
ajreg.c \
ajreport.c \
ajrefseq.c \
ajrefseqread.c \
ajrefseqwrite.c \
ajresource.c \
ajresourceread.c \
ajresourcewrite.c \
ajseq.c \
ajseqabi.c \
ajseqbam.c \
ajseqread.c \
ajseqtype.c \
ajseqwrite.c \
ajsort.c \
ajsql.c \
ajstr.c \
ajsys.c \
ajtable.c \
ajtagval.c \
ajtax.c \
ajtaxread.c \
ajtaxwrite.c \
ajtext.c \
ajtextread.c \
ajtextwrite.c \
ajtime.c \
ajtranslate.c \
ajtree.c \
ajurl.c \
ajurlread.c \
ajurlwrite.c \
ajutil.c \
ajvar.c \
ajvarbcf.c \
ajvarread.c \
ajvarwrite.c \
ajvector.c \
ajxml.c \
ajxmlread.c \
ajxmlwrite.c

include_HEADERS = \
ajax.h \
ajarch.h \
ajdefine.h \
ajalign.h \
ajarr.h \
ajassem.h \
ajassemdata.h \
ajassemread.h \
ajassemwrite.h \
ajassert.h \
ajbamindex.h \
ajbase.h \
ajcall.h \
ajcigar.h \
ajcod.h \
ajdan.h \
ajdmx.h \
ajdom.h \
ajdomain.h \
ajexcept.h \
ajfeat.h \
ajfeatdata.h \
ajfeatread.h \
ajfeatwrite.h \
ajfile.h \
ajfiledata.h \
ajfileio.h \
ajfmt.h \
ajftp.h \
ajhttp.h \
ajindex.h \
ajjava.h \
ajlib.h \
ajlist.h \
ajmath.h \
ajmatrices.h \
ajmem.h \
ajmess.h \
ajnam.h \
ajnexus.h \
ajobo.h \
ajobodata.h \
ajoboread.h \
ajobowrite.h \
ajpat.h \
ajpdb.h \
ajpdbio.h \
ajphylo.h \
ajquery.h \
ajquerydata.h \
ajrange.h \
ajreg.h \
ajreport.h \
ajrefseq.h \
ajrefseqdata.h \
ajrefseqread.h \
ajrefseqwrite.h \
ajresource.h \
ajresourcedata.h \
ajresourceread.h \
ajresourcewrite.h \
ajseq.h \
ajseqabi.h \
ajseqbam.h \
ajseqdata.h \
ajseqread.h \
ajseqtype.h \
ajseqwrite.h \
ajsort.h \
ajsql.h \
ajstr.h \
ajsys.h \
ajtable.h \
ajtagval.h \
ajtax.h \
ajtaxdata.h \
ajtaxread.h \
ajtaxwrite.h \
ajtext.h \
ajtextdata.h \
ajtextread.h \
ajtextwrite.h \
ajtime.h \
ajtranslate.h \
ajtree.h \
ajurl.h \
ajurldata.h \
ajurlread.h \
ajurlwrite.h \
ajutil.h \
ajvar.h \
ajvarbcf.h \
ajvardata.h \
ajvarread.h \
ajvarwrite.h \
ajvector.h \
ajxml.h \
ajxmldata.h \
ajxmlread.h \
ajxmlwrite.h
