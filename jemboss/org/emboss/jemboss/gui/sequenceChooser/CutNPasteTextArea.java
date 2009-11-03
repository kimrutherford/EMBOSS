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

import java.awt.datatransfer.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.awt.Color;

import java.awt.Dimension;
import java.io.*;


/**
*
* Makes a cut-and-paste text area.
*
*/
public class CutNPasteTextArea 
{

  /** sequence paste field     */
  TextAreaSink seqPaste;
  /** default border for field */
  protected static Border defaultBorder = new BevelBorder(BevelBorder.LOWERED);

  /**
  *
  * @param Box to add the cut and paste text area into
  * @param String label for the cut and paste text area
  *
  */
  public CutNPasteTextArea(Box pasteBox, String name) 
  {

    Box bdown;
    bdown = Box.createVerticalBox();
    pasteBox.add(bdown);
    Box pname;
    pname = Box.createHorizontalBox();
    JLabel lname = new JLabel(name);
    lname.setForeground(Color.black);

    bdown.add(pname);
    pname.add(lname);
    pname.add(Box.createHorizontalGlue());
    Box seq;
    seq = Box.createHorizontalBox();
    bdown.add(seq);

    seqPaste = new TextAreaSink();
    JScrollPane seqScroll = new JScrollPane(seqPaste);
    seqScroll.setPreferredSize(new Dimension(470, 100));
    seqPaste.setBorder(defaultBorder);

    seq.add(seqScroll);
    seq.add(Box.createHorizontalGlue());
    
  }

  /**
  *
  * @return 	String contents of the text area
  *
  */
  public String getText() 
  {
    return seqPaste.getText();
  }

  /**
  *
  * Set contents of the text area
  * @param s	contents to apply to the text area
  *
  */
  public void setText(String s)
  {
    seqPaste.setText(s);
  }

}

