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


package org.emboss.jemboss.gui.form;

import java.awt.*;
import javax.swing.*;
import java.io.FileWriter;
import java.io.IOException;

import org.emboss.jemboss.gui.ScrollPanel;
import org.emboss.jemboss.gui.sequenceChooser.*;
import org.emboss.jemboss.JembossParams;

/**
*
* List file panel, enabling the user to generate a list of
* sequence in the GUI which can be used to generate an EMBOSS
* list file to be used in the command line.
*
*/
public class ListFilePanel extends JPanel
{

  /** maximum number of sequences in list */
  private int nFiles;
  /** container for sequences */
  private FileChooser fileChooser[];

  /**
  *
  * @param nFiles	maximum number of files in a list
  * @param mysettings	jemboss properties
  *
  */  
  public ListFilePanel(int nFiles, JembossParams mysettings)
  {
    super(new BorderLayout());
    this.nFiles = nFiles;

    Box bdown = Box.createVerticalBox();
    fileChooser = new FileChooser[nFiles];

    Dimension fcdim = null;

    bdown.add(Box.createVerticalStrut(2));
    for(int i=0;i < nFiles;i++)
    {
      fileChooser[i] = new FileChooser(bdown,"",mysettings); 
      if(fcdim == null)
      {
        fcdim = fileChooser[i].getPreferredSize();
        fcdim = new Dimension(210, (int)fcdim.getHeight());
      }
      fileChooser[i].setPreferredSize(fcdim);

      bdown.add(Box.createVerticalStrut(2));  
    }

    ScrollPanel sp = new ScrollPanel(new BorderLayout());
    sp.add(bdown);
    JScrollPane scroll = new JScrollPane(sp);
    scroll.setPreferredSize(new Dimension(350,100));
    
    this.add(scroll, BorderLayout.WEST);
    setPreferredSize(new Dimension(350,100));
    setMaximumSize(new Dimension(350,100));
  }

  /**
  *
  * @return 	list of filenames
  *
  */
  public String getListFile()
  {
    String list = "";
    String ls = System.getProperty("line.separator");
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
        list = list.concat(fileChooser[i].getFileChosen() + ls);
    }

    return list;
  }

  /**
  *
  * @return list of filenames
  *
  */
  public String[] getArrayListFile()
  {

    int nseqs = 0;
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
        nseqs++;
    }

    String list[] = new String[nseqs];
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
        list[i] = fileChooser[i].getFileChosen();
    }

    return list;
  }

  /**
  *
  *  @param file name for list file
  *
  */ 
  public boolean writeListFile(String fn)
  {
    boolean writeOK = true;
                                                                                             
    try
    {
      FileWriter out = new FileWriter(fn);
      out.write(getListFile());
      out.close();
    }
    catch(IOException ioe)
    {
      ioe.printStackTrace();
      writeOK = false;
    }
                                                                                             
    return writeOK;
  }

  /**
  *
  * Get a sequence form the list using its number in
  * the list
  * @param n	sequence number
  * @return 	nth sequence file name
  *
  */
  public String getSequence(int n)
  {
    String fn="";

    int nseqs = 0;
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
      {
        nseqs++;
        if(nseqs == n)
        {
          fn = fileChooser[i].getFileChosen();
          break;
        }
      }
    }

    return fn;
  }

  /**
  *
  * Reset the values of the list files
  *
  */
  public void doReset()
  {
    for(int i=0;i < nFiles;i++)
      fileChooser[i].setText("");
  }

}


