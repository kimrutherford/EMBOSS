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

import javax.swing.JPanel;
import java.awt.*;

/**
*
* Panel separator
*
*/
public class Separator extends JPanel
{
  
  /** size of separator */
  private Dimension d;

  /**
  *
  * @param d	size of separator
  *
  */
  public Separator(Dimension d)
  {
    super();
    this.d = d;
    setSize(d);
  }

  /**
  *
  * Override paint to draw separator
  *
  */
  public void paint(Graphics g) 
  {
    Dimension size = getSize();
    Color light = getBackground().brighter().brighter();
    Color dark  = getBackground().darker();
    int length = d.width;
    int xpos = (size.width)/2 - length/2;
    int ypos = (size.height)/2;
    g.setColor(dark);
    g.drawLine(xpos,ypos,xpos+length,ypos);
    g.setColor(light);
    g.drawLine(xpos,ypos+1,xpos+length,ypos+1);
  }


}

