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


package org.emboss.jemboss.gui.sequenceChooser;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.filechooser.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.io.*;

import org.emboss.jemboss.gui.form.*;
import org.emboss.jemboss.gui.AdvancedOptions;
import org.emboss.jemboss.JembossParams;

/**
*
* Creates a JFileChooser and add it to the Box.
*
*/
public class FileChooser 
{
  /** file chooser */
  private final JFileChooser fc;
  /** file name text field */
  private TextFieldSink fileName;
  /** label for text field */
  private JLabel lname;

  /**
  *
  * @param bdown	container in form to add to
  * @param name		label name for field
  * @param mysettings	jemboss properties 	
  *
  */
  public FileChooser(Box bdown, String name,
             final JembossParams mysettings)
  {

    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);

    fc = new JFileChooser();
    System.setSecurityManager(sm);

    JButton openButton = new JButton("Browse files...");
    fileName = new TextFieldSink();

    Box pname = Box.createHorizontalBox();
    lname = new JLabel(name);
    lname.setFont(org.emboss.jemboss.gui.form.SectionPanel.labfont);

    bdown.add(pname);
    pname.add(lname);
    pname.add(Box.createHorizontalGlue());

    Box file = Box.createHorizontalBox();
    bdown.add(file);

    file.add(fileName);
    file.add(Box.createRigidArea(new Dimension(2,2)));
    file.add(openButton);

    fc.addChoosableFileFilter(new SequenceFilter());
    openButton.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e)       
      {
        String dir = mysettings.getUserHome();
        File cwd = new File(dir);
        if(cwd.isDirectory() && cwd.canRead())
          fc.setCurrentDirectory(cwd);

        int returnVal = fc.showOpenDialog(fc);

        if (returnVal == JFileChooser.APPROVE_OPTION) 
        {
          String fileSelected = fc.getSelectedFile().getAbsolutePath();
          String currentDirectory = fc.getCurrentDirectory().getPath();
          fileName.setText(fileSelected);
        }
      }
    });

    file.add(Box.createHorizontalGlue());
  }

  /**
  *
  * Get the selected file
  * @return 	file name
  *
  */ 
  public String getFileChosen() 
  {
    return fileName.getText();
  }

  /**
  *
  * Get the JFileChooser 
  * @return	file chooser
  *
  */
  public JFileChooser getJFileChooser() 
  {
    return fc;
  }

  /**
  *
  * Get the text field
  * @return	text field
  *
  */
  public TextFieldSink getTextFieldSink() 
  {
    return fileName;
  }

  /**
  *
  * Set the text field
  * @param txt	text to set in the text field
  *
  */
  public void setText(String txt)
  {
    fileName.setText(txt);
  }

  /**
  *
  * Set the size of the text field
  * @param d	size of text field
  *
  */
  public void setSize(Dimension d)
  {
    fileName.setPreferredSize(d);
    fileName.setMinimumSize(d);
    fileName.setMaximumSize(d);
  }

  /**
  *
  * Set the preferred size of the text field
  * @param d    size of text field
  *
  */
  public void setPreferredSize(Dimension d)
  {
    fileName.setPreferredSize(d);
  }

  /**
  *
  * Get the preferred size of the text field
  * @return    	size of text field
  *
  */
  public Dimension getPreferredSize()
  {
    return fileName.getPreferredSize();
  }

  /**
  * 
  * Set the label colour
  * @param c	colour
  *
  */
  public void setForeground(Color c)
  {
    lname.setForeground(c);
  }

}

