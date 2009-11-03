/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.editor;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
*
* Frame for searching for patterns in the sequences
*
*/
public class PatternJFrame extends JFrame
{
 
  /** graphic sequence panel       */
  private GraphicSequenceCollection gsc= null;
  /** residue position to display  */
  private int viewResiduePosition = -1;
  /** wrap around search check box */
  private JCheckBox wrapAround;

  public PatternJFrame()
  {
    super("Find Pattern");

    JPanel mainPane = (JPanel)getContentPane();
    mainPane.setLayout(new BorderLayout());
    Box YBox = Box.createVerticalBox();
    YBox.add(Box.createVerticalStrut(5));
    mainPane.add(YBox, BorderLayout.NORTH);
   
// find patterns
    Box XBox = Box.createHorizontalBox();
    JLabel label = new JLabel("Pattern: ");
    XBox.add(label);

    final JTextField patternField = new JTextField();
    patternField.setPreferredSize(new Dimension(150,
            (int)label.getPreferredSize().getHeight()));

    patternField.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(gsc != null)
        {
          String pattern = patternField.getText();
          if(!pattern.equals(""))
            viewResiduePosition = gsc.findPattern(pattern,
                   viewResiduePosition,wrapAround.isSelected());
          gsc.repaint();
        }
        else
          JOptionPane.showMessageDialog(null,"No sequences loaded to search",
                                        "No Sequences",
                                        JOptionPane.WARNING_MESSAGE);
      }
    });
    XBox.add(patternField);
    mainPane.add(XBox,BorderLayout.CENTER);

// south box
    Box southBox = Box.createVerticalBox();
    mainPane.add(southBox,BorderLayout.SOUTH);
    XBox = Box.createHorizontalBox();

    wrapAround = new JCheckBox("Wrap around",true);
    XBox.add(wrapAround);
    XBox.add(Box.createHorizontalGlue());
    southBox.add(XBox);
//  JCheckBox backwardSearch = new JCheckBox("Search backwards",false);
//  XBox = Box.createHorizontalBox();
//  XBox.add(backwardSearch);
//  XBox.add(Box.createHorizontalGlue());
//  southBox.add(XBox);
 
//
    XBox = Box.createHorizontalBox();
    JButton findButton = new JButton("Find");
    XBox.add(findButton);

    findButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      { 
        if(gsc != null)
        {
          String pattern = patternField.getText();
          if(!pattern.equals(""))
            viewResiduePosition = gsc.findPattern(pattern,
                   viewResiduePosition,wrapAround.isSelected());
          gsc.repaint();
        }
        else
          JOptionPane.showMessageDialog(null,"No sequences loaded to search",
                                        "No Sequences",
                                        JOptionPane.WARNING_MESSAGE);
      }
    });
 
    JButton cancelButt = new JButton("Cancel");
    cancelButt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
        viewResiduePosition = -1;
      }
    });
    XBox.add(cancelButt);
//  XBox.add(Box.createHorizontalGlue());
    southBox.add(XBox);

    pack();
  }


  /**
  *
  * Set the graphic sequence panel 
  *
  */
  protected void setGraphic(GraphicSequenceCollection gsc)
  {
    this.gsc = gsc;
  }

}

