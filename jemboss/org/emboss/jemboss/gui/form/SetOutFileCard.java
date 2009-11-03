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
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

import org.emboss.jemboss.gui.sequenceChooser.*;

/**
*
* Creates an output file Box
*
*/
public class SetOutFileCard
{

  /**
  *
  * @param pan		container for this parameter
  * @param tfs		field sink for output file name
  * @param labelColor	label coulour
  * @param sectionPane	section panel
  *
  */
  public SetOutFileCard(Box pan, TextFieldSink tfs, Color labelColor,
                        final JPanel sectionPane)
  {
    Box bdown = new Box(BoxLayout.Y_AXIS);
    Box bacross = new Box(BoxLayout.X_AXIS);

    JLabel lab = new JLabel(" Output Sequence Name");
    JButton boption = new JButton("Output Sequence Options");

    lab.setForeground(labelColor);
    tfs.setPreferredSize(boption.getPreferredSize());
    bacross.add(tfs);
    bacross.add(lab);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    BuildJembossForm.outSeqAttr = new OutputSequenceAttributes();
    final JScrollPane rscroll = BuildJembossForm.outSeqAttr.getJScrollPane();
    boption.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JOptionPane jop = new JOptionPane();
        jop.showMessageDialog(sectionPane,rscroll,
                "Output Sequence",
                JOptionPane.PLAIN_MESSAGE);
      }
    });
    bacross = new Box(BoxLayout.X_AXIS);
    bacross.add(boption);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    pan.add(bdown);
  }

}
