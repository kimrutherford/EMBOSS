pkgdata_DATA = CutNPasteTextArea.java FileChooser.java \
InputSequenceAttributes.java OutputSequenceAttributes.java \
SequenceFilter.java TextAreaSink.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/sequenceChooser

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/sequenceChooser; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/sequenceChooser; \
	done; \
	fi

