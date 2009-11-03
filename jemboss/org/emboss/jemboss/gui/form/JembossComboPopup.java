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

import java.util.Vector;
import javax.swing.*;
import javax.swing.plaf.basic.*;

/**
*
* Creates a scrolling JComboBox
*
*/
public class JembossComboPopup extends JComboBox
{
  
  /**
  *
  *  Creates a JComboBox that contains the elements in 
  *  the specified array.
  *  @param items 	array of objects to insert into the combo box
  *
  */
  public JembossComboPopup(Object[] items)
  {
    super(items);
    setUI(new myComboUI());
    setBorder(BorderFactory.createEtchedBorder());
  }

  /**
  *
  *  Creates a JComboBox that contains the elements in
  *  the vextor
  *  @param items       vector of objects to insert into the combo box
  *
  */
  public JembossComboPopup(Vector items)
  {
    super(items);
    setUI(new myComboUI());
    setBorder(BorderFactory.createEtchedBorder());
  }
          
  /**
  *
  * Create scroller
  *
  */                             
  public class myComboUI extends BasicComboBoxUI
  {
    protected ComboPopup createPopup()
    {
      BasicComboPopup popup = new BasicComboPopup(comboBox)
      {
        protected JScrollPane createScroller() 
        {
          return new JScrollPane( list, ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                               ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED );
        }
      };
      return popup;
    }
  }
}

