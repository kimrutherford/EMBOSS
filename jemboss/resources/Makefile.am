# The intention here is to check whether JAVA_OK exists and, if
# so, do a server installation. Otherwise check to see whether
# there is already a resources.jar in the directory (from
# an 'ant' build of a standalone setup) and, if so, copy it.
#
EMLOCDATA = $(CURDIR)/../../emboss/data
install-exec-hook:
	(if test -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources ; then \
		echo "resources folder exists"; \
	else \
		mkdir -p $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources; \
	fi);
	echo "Preparing $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/jemboss.properties"; \
	sed "s|user.auth=true|user.auth=false|" jemboss.properties | sed "s|jemboss.server=true|jemboss.server=false|" | sed "s|/packages/emboss/STABLE|$(prefix)|" > jemboss.properties.new;
if !AMPDF
	echo "embossHavePDF=false" >> jemboss.properties.new;
endif
	mv -f -v jemboss.properties.new $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/jemboss.properties;
	@if test "${JAVA_OK}" = "yes" ; then \
		rm -f   $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/acdstore.jar; \
		cd $(EMBOSS_TOP)/emboss/acd; \
		jar cvf $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/acdstore.jar *.acd; \
		rm -f   $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/resources.jar; \
		[ -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/data ] || mkdir -p $(DESTDIR)$(prefix)/share/$(PACKAGE)/data; \
		jar cvf $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/resources.jar $(EMLOCDATA)/EPAM* $(EMLOCDATA)/EBLOSUM* $(EMLOCDATA)/EDNA*; \
	else \
		(if test -f resources.jar ; then \
		  cp resources.jar $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources; \
		fi); \
	fi




pkgdata_DATA = readme.html readmeAlign.html readmeDNADraw.html results.html version filemgr.html seqList.html

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/resources
