pkgdata_DATA = Database.java ProgList.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/startup

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/startup; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/startup; \
	done; \
	fi
