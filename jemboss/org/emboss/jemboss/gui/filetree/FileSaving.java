/********************************************************************
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
* 
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
* 
*  You should have received a copy of the GNU Library General Public
*  License along with this library; if not, write to the
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/


package org.emboss.jemboss.gui.filetree;

import java.io.*;
import java.awt.*;
import javax.swing.*;
import org.emboss.jemboss.gui.sequenceChooser.*;
import org.emboss.jemboss.JembossParams;

/**
*
* Saves to local files from a text pane
*
*/
public class FileSaving
{

  /** true if the file is saved ok */
  private boolean lsaved = false;
  /** file to save to */
  private String fileSelected;
  /** path to saved file */
  private String cwd;

  /**
  *
  * @param seqText	text pane to save content from
  * @param pngContent	png content
  * @param mysettings	jemboss properties
  *
  */
  public FileSaving(JTextPane seqText, byte[] pngContent, JembossParams mysettings)
  {

    Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
    String fs = new String(System.getProperty("file.separator"));

    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);
    JFileChooser fc = new JFileChooser(mysettings.getUserHome());
    System.setSecurityManager(sm);

    fc.addChoosableFileFilter(new SequenceFilter());
    int returnVal = fc.showSaveDialog(fc);

    if (returnVal == JFileChooser.APPROVE_OPTION)
    {
      File files = fc.getSelectedFile();
      cwd = (fc.getCurrentDirectory()).getAbsolutePath();
      fileSelected = files.getName();

      seqText.setCursor(cbusy);
      FileSave fsave = new FileSave(new File(cwd + fs + fileSelected));
      if(fsave.doWrite())
      {
        if(pngContent != null)
          fsave.fileSaving(pngContent);
        else
          fsave.fileSaving(seqText.getText());
  

        if(fsave.writeOK())
          lsaved = true;    
//      if(!fsave.fileExists())
//      {
//        org.emboss.jemboss.Jemboss.tree.addObject(fileSelected,cwd,null);
//        DragTree ltree = org.emboss.jemboss.gui.SetUpMenuBar.getLocalDragTree();
//        if(ltree!=null)
//          ltree.addObject(fileSelected,cwd,null);
//      }
      }
      seqText.setCursor(cdone);
    }
  }

  /**
  *
  * @return 	true if the file is saved ok
  *
  */
  public boolean writeOK()
  {
    return lsaved;
  }

  /**
  *
  * @return	saved file name
  *
  */
  public String getFileName()
  {
    return fileSelected;
  }

  /**
  *
  * @return 	path to saved file
  *
  */
  public String getPath()
  {
    return cwd;
  }

}

