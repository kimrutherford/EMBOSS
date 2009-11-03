## Process this file with automake to produce Makefile.in

if ISCYGWIN
lib_LTLIBRARIES = libajax.la
else
lib_LTLIBRARIES = libajax.la libajaxg.la
endif

if ISSHARED
if ISAIXIA64
if NEEDAJAX
AM_CFLAGS = -Wl,-G -L../plplot/.libs -L.libs -lajax -leplplot -lm
else
AM_CFLAGS = -Wl,-G -L../plplot/.libs -L.libs -leplplot -lm
endif
else
AM_CFLAGS =
endif
endif

if ISCYGWIN
AM_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import -L../plplot -leplplot
endif

## PCRE stuff (version 4.3)

POSIX_MALLOC_THRESHOLD = @POSIX_MALLOC_THRESHOLD@
PCRELIBVERSION = @PCRE_LIB_VERSION@
PCREPOSIXLIBVERSION = @PCRE_POSIXLIB_VERSION@

INCLUDES = -DAJAX_FIXED_ROOT=$(AJAX_FIXED_ROOT) -DPREFIX=\"$(prefix)\" -I$(top_srcdir)/plplot $(POSIX_MALLOC_THRESHOLD) 
#LDFLAGS = -no-undefined
LIBS =

AJAXSRC = ajacd.c ajalign.c ajarr.c ajassert.c \
ajbase.c ajcall.c ajcod.c \
ajdan.c ajdmx.c ajdom.c ajdomain.c \
ajexcept.c \
ajfeat.c ajfile.c ajfiledata.c ajfileio.c ajfmt.c \
ajgraphxml.c ajindex.c ajjava.c ajlist.c \
ajmath.c ajmatrices.c ajmem.c ajmess.c ajnam.c \
ajnexus.c \
ajpat.c ajpdb.c ajpdbio.c ajphylo.c \
ajrange.c ajreg.c ajreport.c \
ajseq.c ajseqabi.c ajseqdb.c ajseqread.c ajseqtype.c ajseqwrite.c \
ajsort.c ajstr.c ajsys.c \
ajtable.c ajtime.c ajtranslate.c ajtree.c \
ajutil.c ajvector.c \
pcre.c pcre_study.c pcreposix.c

AJAXGSRC = ajgraph.c ajhist.c

include_HEADERS = ajax.h ajarch.h ajdefine.h \
ajacd.h ajalign.h ajarr.h ajassert.h \
ajbase.h ajcall.h ajcod.h \
ajdan.h ajdmx.h ajdom.h ajdomain.h ajexcept.h \
ajfeat.h ajfeatdata.h ajfile.h ajfiledata.h ajfileio.h ajfmt.h \
ajgraph.h ajgraphxml.h ajgraphstruct.h ajhist.h ajindex.h ajjava.h ajlist.h \
ajmath.h ajmatrices.h ajmem.h ajmess.h ajnam.h ajnexus.h \
ajpat.h ajpdb.h ajpdbio.h ajphylo.h \
ajrange.h ajreg.h ajreport.h \
ajseq.h ajseqabi.h ajseqdata.h ajseqdb.h ajseqread.h ajseqtype.h ajseqwrite.h \
ajsort.h ajstr.h ajsys.h \
ajtable.h ajtime.h ajtranslate.h ajtree.h ajutil.h ajvector.h \
pcre.h pcre_chartables.c pcre_config.h pcre_get.c pcre_printint.c \
pcre_internal.h pcreposix.h


if ISCYGWIN
libajax_la_SOURCES = $(AJAXSRC) $(AJAXGSRC)
libajax_la_LDFLAGS = -version-info 6:0:0
else
libajax_la_SOURCES = $(AJAXSRC)
libajax_la_LDFLAGS = -version-info 6:0:0
libajaxg_la_SOURCES = $(AJAXGSRC)
libajaxg_la_LDFLAGS = -version-info 6:0:0
endif
