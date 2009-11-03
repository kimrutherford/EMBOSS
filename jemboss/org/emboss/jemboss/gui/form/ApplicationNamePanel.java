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

package org.emboss.jemboss.gui.form;

import javax.swing.*;
import java.awt.*;

/**
*
* Panel to display the EMBOSS application name on
*
*/
public class ApplicationNamePanel extends JPanel
{
  /** x position for displaying name */
  private int x;
  /** y position for displaying name */
  private int y;
  /** font size */
  private int fontSize;
  /** application name */
  private String name;

  /**
  *
  * @param name 	application name
  * @param x		x position for displaying name 
  * @param y		y position for displaying name
  * @param fontSize	font size 
  *
  */
  public ApplicationNamePanel(String name, int x, int y, 
                                  int fontSize)
  {
    this.name = name;
    this.x = x;
    this.y = y;
    this.fontSize = fontSize;
    
    Font font = new Font("Dialog", Font.BOLD, fontSize);
    setFont(font);
    FontMetrics metrics = getFontMetrics(font);
    int width   = metrics.stringWidth(name+"   ");
    setPreferredSize(new Dimension(width, fontSize+1)); 
    setBackground(Color.white);
  } 

  /**
  *
  * Override paintComponent
  * @param g	graphics
  *
  */
  public void paintComponent(Graphics g)
  {
    super.paintComponent(g);
    int line_spacing = 4;

    Color top_color = new Color(200,10,0);
    Color side_color = new Color(200,10,0);
    g.setColor(Color.red);
    g.drawString(name,x,y);
  }

}

