## Makefile.am for Jemboss
## Process this file with automake to produce Makefile.in in all directories.
AUTOMAKE_OPTIONS = gnits

SUBDIRS = images lib resources utils

# files with nonstandard names in this directory
EXTRA_DIST = LICENSE

clean-local:
if EHAVEANT
	ant clean;
endif

all-local:

install-exec-hook:
	echo "Preparing $(DESTDIR)$(bindir)/runJemboss.csh"; \
	echo "#!/bin/csh" > $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "setenv CLASSPATH $(prefix)/share/$(PACKAGE)/jemboss/lib/mail.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/activation.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jakarta-regexp-1.2.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jalviewApplet.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jcert.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jnet.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jsse.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/axis.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/commons-discovery.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/commons-logging.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/jaxrpc.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/log4j-1.2.4.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/saaj.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/wsdl4j.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jemboss.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/client.jar:$(prefix)/share/$(PACKAGE)/jemboss" >> $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "(cd $(prefix)/share/$(PACKAGE)/jemboss; java org/emboss/jemboss/Jemboss &)" >> $(DESTDIR)$(bindir)/runJemboss.csh; \
	chmod 755 $(DESTDIR)$(bindir)/runJemboss.csh; \
	echo "Updating $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/runJemboss.sh"; \
	sed "s|JEMBOSS_HOME=.|JEMBOSS_HOME=$(prefix)/share/$(PACKAGE)/jemboss/|" runJemboss.sh > $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/runJemboss.sh; \
	chmod 755 $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/runJemboss.sh;

pkgdata_DATA = LICENSE readme.html README

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss



# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
if EHAVEANT
	ant package -Demboss.data=../emboss/data -Dname=jemboss -Djar.dir=lib -Dnotformemboss=yes;
	ant clean;
endif
	tar cBf - api | ( cd $(distdir); tar xBf - ; rm -rf api/CVS );
