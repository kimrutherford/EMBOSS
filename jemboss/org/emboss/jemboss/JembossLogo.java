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

package org.emboss.jemboss;

import javax.swing.*;
import java.awt.*;

public class JembossLogo extends JPanel
{

  private int x;
  private int y;
  private int fontSize;

  public JembossLogo(int x, int y, int fontSize)
  {
    this.x = x;
    this.y = y;
    this.fontSize = fontSize;
    setBackground(Color.white);
  } 

  public void paintComponent(Graphics g)
  {

    int line_spacing = 25;

    g.setFont(new Font("Dialog", Font.BOLD, fontSize));
    Color top_color = new Color(150,10,0);
    Color side_color = new Color(150,10,0);

//  Color top_color = new Color(200,200,0);
//  Color side_color = new Color(100,0,0);
    for (int i = 0; i < 5; i++)
    {
      g.setColor(top_color);
      g.drawString("Jemboss", ShiftEast(x, i), ShiftNorth(ShiftSouth(y, i), 1));
      g.setColor(side_color);
      g.drawString("Jemboss", ShiftWest(ShiftEast(x, i), 1), ShiftSouth(y, i));
    }
    g.setColor(Color.red);
    g.drawString("Jemboss", ShiftEast(x, 5), ShiftSouth(y, 5));
    int ynew = y+(line_spacing*3);

    g.setFont(new Font("Dialog", Font.BOLD, 25));
    g.setColor(Color.black);
    g.drawString("HGMP-RC", x+20, ynew);
    ynew = ynew+(line_spacing);
    g.drawString("Cambridge", x+20, ynew);
    ynew = ynew+(line_spacing);
    g.drawString("UK", x+20, ynew);

  }

  int ShiftNorth(int p, int distance) 
  {
    return (p - distance);
  }

  int ShiftSouth(int p, int distance) 
  {
    return (p + distance);
  }

  int ShiftEast(int p, int distance) 
  {
    return (p + distance);
  }

  int ShiftWest(int p, int distance) 
  {
    return (p - distance);
  }

}

