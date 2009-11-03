## Makefile.am for Jemboss
## Process this file with automake to produce Makefile.in in all directories.
AUTOMAKE_OPTIONS = gnits

SUBDIRS = api images lib org resources utils

# files with nonstandard names in this directory
EXTRA_DIST = LICENSE

all-local:
if ISCYGWIN
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "Not compiling Jemboss"; \
	else \
	  (if test "${SHELL}" = "/bin/bash" ; then \
	  declare -x CLASSPATH=`cygpath -wp lib/JembossPrintAlignment.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:lib/jcert.jar:lib/jnet.jar:lib/jsse.jar:.:lib/axis/axis-ant.jar:lib/axis/axis.jar:lib/axis/commons-discovery.jar:lib/axis/commons-logging.jar:lib/axis/jaxrpc.jar:lib/axis/log4j-1.2.4.jar:lib/axis/saaj.jar:lib/axis/wsdl4j.jar:lib/grout.jar:$(srcdir)`; \
	  export CLASSPATH; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java org/emboss/jemboss/draw/PrintDNAImage.java_o; \
          javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/draw/DNADraw.java; \
	  javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/Jemboss.java; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o ./org/emboss/jemboss/editor/PrintAlignmentImage.java; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java_o org/emboss/jemboss/draw/PrintDNAImage.java; \
	  else \
	  CLASSPATH="lib/JembossPrintAlignment.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:lib/jcert.jar:lib/jnet.jar:lib/jsse.jar:.:lib/axis/axis-ant.jar:lib/axis/axis.jar:lib/axis/commons-discovery.jar:lib/axis/commons-logging.jar:lib/axis/jaxrpc.jar:lib/axis/log4j-1.2.4.jar:lib/axis/saaj.jar:lib/axis/wsdl4j.jar:lib/grout.jar:$(srcdir)"; \
	  export CLASSPATH; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java org/emboss/jemboss/draw/PrintDNAImage.java_o; \
          javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/draw/DNADraw.java; \
	  javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/Jemboss.java; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o ./org/emboss/jemboss/editor/PrintAlignmentImage.java; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java_o org/emboss/jemboss/draw/PrintDNAImage.java; \
	  fi); \
	fi
else
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "Not compiling Jemboss"; \
	else \
	  (if test "${SHELL}" = "/bin/bash" ; then \
	  declare -x CLASSPATH=lib/JembossPrintAlignment.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:lib/jcert.jar:lib/jnet.jar:lib/jsse.jar:.:lib/axis/axis-ant.jar:lib/axis/axis.jar:lib/axis/commons-discovery.jar:lib/axis/commons-logging.jar:lib/axis/jaxrpc.jar:lib/axis/log4j-1.2.4.jar:lib/axis/saaj.jar:lib/axis/wsdl4j.jar:lib/grout.jar:$(srcdir); \
	  export CLASSPATH; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java org/emboss/jemboss/draw/PrintDNAImage.java_o; \
          javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/draw/DNADraw.java; \
	  javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/Jemboss.java; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o ./org/emboss/jemboss/editor/PrintAlignmentImage.java; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java_o org/emboss/jemboss/draw/PrintDNAImage.java; \
	  else \
	  CLASSPATH="lib/JembossPrintAlignment.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:lib/jcert.jar:lib/jnet.jar:lib/jsse.jar:.:lib/axis/axis-ant.jar:lib/axis/axis.jar:lib/axis/commons-discovery.jar:lib/axis/commons-logging.jar:lib/axis/jaxrpc.jar:lib/axis/log4j-1.2.4.jar:lib/axis/saaj.jar:lib/axis/wsdl4j.jar:lib/grout.jar:$(srcdir)"; \
	  export CLASSPATH; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java org/emboss/jemboss/draw/PrintDNAImage.java_o; \
          javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/draw/DNADraw.java; \
	  javac -target 1.3 -source 1.3 -d . org/emboss/jemboss/Jemboss.java; \
          mv -f ./org/emboss/jemboss/editor/PrintAlignmentImage.java_o ./org/emboss/jemboss/editor/PrintAlignmentImage.java; \
          mv -f org/emboss/jemboss/draw/PrintDNAImage.java_o org/emboss/jemboss/draw/PrintDNAImage.java; \
	  fi); \
	fi
endif

install-exec-hook:
	rm -f $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "#!/bin/csh" >> $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "setenv LD_LIBRARY_PATH $(libdir)" >> $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "setenv CLASSPATH $(prefix)/share/$(PACKAGE)/jemboss/lib/JembossPrintAlignment.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/xerces.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/mail.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/activation.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jakarta-regexp-1.2.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jalview.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jcert.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jnet.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jsse.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/axis-ant.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/axis.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/commons-discovery.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/commons-logging.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/jaxrpc.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/log4j-1.2.4.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/saaj.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/wsdl4j.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/grout.jar:$(prefix)/share/$(PACKAGE)/jemboss" >> $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "(cd $(prefix)/share/$(PACKAGE)/jemboss; java org/emboss/jemboss/Jemboss &)" >> $(DESTDIR)$(bindir)/runJemboss.csh; \
	chmod 755 $(DESTDIR)$(bindir)/runJemboss.csh;

pkgdata_DATA = LICENSE readme.html runJemboss.sh README

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss



# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
	tar cBf - api | ( cd $(distdir); tar xBf - ; rm -rf api/CVS ) 
