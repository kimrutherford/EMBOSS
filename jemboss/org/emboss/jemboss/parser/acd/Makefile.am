pkgdata_DATA = ApplicationFields.java ApplicationParam.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser/acd

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser/acd; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser/acd; \
	done; \
	fi
