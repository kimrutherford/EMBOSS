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
* Colour panel for each individual residue in the pallette
*
*/
public class ColourPanel extends JPanel
                      implements ActionListener
{
  protected Color col;
  private int xsize = 20;
  private int ysize = 20;
  private JPopupMenu popup;

  /**
  *
  * @param menu       menu text
  * @param col        colour of residue
  *
  */
  public ColourPanel(String menu,Color col)
  {
    super();
    this.col = col;
    setPreferredSize(new Dimension(xsize,ysize));
    setMaximumSize(new Dimension(xsize,ysize));

    ColourMenu cm = new ColourMenu(menu);
    popup = new JPopupMenu();
    addMouseListener(new PopupListener());
    cm.addActionListener(this);
    popup.add(cm);
  }


  /**
  *
  * Popup menu actions
  * @param e  action event
  *
  */
  public void actionPerformed(ActionEvent e)
  {
    ColourMenu m = (ColourMenu)e.getSource();
    col = m.getColor();
    repaint();
  }


  /**
  *
  * Return current colour selection
  * @return 	colour
  *
  */
  public Color getColour()
  {
    return col;
  }


  /**
  *
  * Override paintComponent
  * @param g  graphics
  *
  */
  public void paintComponent(Graphics g)
  {
// let UI delegate paint first (incl. background filling)
    super.paintComponent(g);
    g.setColor(col);
    g.fillRect(0,0,xsize,ysize);
    g.setColor(Color.black);
    g.drawRect(0,0,xsize,ysize);
  }


  /**
  *
  * Popup listener
  *
  */
  class PopupListener extends MouseAdapter
  {
    public void mousePressed(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    public void mouseReleased(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    private void maybeShowPopup(MouseEvent e)
    {
      if(e.isPopupTrigger())
        popup.show(e.getComponent(),
                e.getX(), e.getY());
    }
  }

}

