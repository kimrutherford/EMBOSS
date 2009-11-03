pkgdata_DATA = DragTree.java FileEditorDisplay.java FileNode.java \
FileSave.java FileSaving.java RemoteDragTree.java \
RemoteFileNode.java RemoteFileTreePanel.java LocalAndRemoteFileTreeFrame.java \
LocalTreeToolBar.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/filetree

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	$(top_srcdir)/mkinstalldirs $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/filetree; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(DESTDIR)$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/filetree; \
	done; \
	fi
