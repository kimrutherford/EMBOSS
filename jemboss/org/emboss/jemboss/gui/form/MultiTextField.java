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

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JTextField;

class MultiTextField
{
  /** text fields */
  private JTextField textField[];
  private LabelTextBox labs[];
  private int num;
  
  public MultiTextField(final int num, final LabelTextBox labs[])
  {
    this.num = num;
    this.textField = new JTextField[num];
    this.labs      = labs;
  }
  
  public Box getBoxOfTextFields()
  {
    Box ybox = new Box(BoxLayout.Y_AXIS);
    Dimension d = new Dimension(150, 30);
    for(int i=0; i<num; i++)
    {
      Box xbox = new Box(BoxLayout.X_AXIS);
      textField[i] = new JTextField();

      textField[i].setPreferredSize(d);
      textField[i].setMinimumSize(d);
      textField[i].setMaximumSize(d);
      
      xbox.add(textField[i]);
      xbox.add(labs[i]);
      xbox.add(Box.createHorizontalGlue());
      ybox.add(xbox);
      ybox.add(Box.createVerticalStrut(10));
    }
    
    return ybox;
  }
  
  public JTextField[] getJTextField()
  {
    return textField;
  }
}