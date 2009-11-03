install-exec-hook:
	  @if test "${JAVA_OK}" = "yes" ; then \
	          (if test -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources; then \
			echo "resources exists"; \
		   else \
			mkdir $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources; \
		   fi); \
	          rm -f   $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/acdstore.jar; \
		  cd $(EMBOSS_TOP)/emboss/acd; \
		  jar cvf $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/acdstore.jar *.acd; \
                  rm -f   $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/resources.jar; \
                  cd $(DESTDIR)$(prefix)/share/$(PACKAGE)/data; \
                  jar cvf $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/resources.jar EPAM* EBLOSUM* ENUC*; \
                  rm -f   $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/Ajax.jar; \
                  cd $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss; \
                  jar cvf $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/resources/Ajax.jar org/emboss/jemboss/parser/Ajax.class; \
	  fi


pkgdata_DATA = jemboss.properties readme.html readmeAlign.html readmeDNADraw.html results.html version filemgr.html seqList.html

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/resources
