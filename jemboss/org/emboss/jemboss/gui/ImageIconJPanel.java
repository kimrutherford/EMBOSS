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

package org.emboss.jemboss.gui;

import java.awt.Rectangle;
import java.awt.Dimension;
import java.awt.Graphics;
import javax.swing.ImageIcon;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.Scrollable;
import java.lang.ClassLoader;

/**
*
* Use to display ImageIcons on a JPanel
*
*/
public class ImageIconJPanel extends JPanel implements Scrollable
{

  /** image icon */
  private ImageIcon ii;

  /**
  *
  * @param ii 	image icon to display
  *
  */
  public ImageIconJPanel(ImageIcon ii)
  {
    super();
    this.ii = ii;
  }

  
  /**
  *
  * Override the paintComponent
  *
  */
  public void paintComponent(Graphics g)
  {
    super.paintComponent(g);
    if(ii != null)
      ii.paintIcon(this,g,0,0);
  }

  /**
  *
  * Override getPreferredSize to return the 
  * dimensions of the ImageIcon
  *
  */
  public Dimension getPreferredSize()
  {
    return new Dimension(ii.getIconWidth(),
                         ii.getIconHeight());
  }


// Scrollable interface
  public Dimension getPreferredScrollableViewportSize()
  {
    return getPreferredSize();
  }

  public boolean getScrollableTracksViewportHeight()
  {
    return false;
  }

  public boolean getScrollableTracksViewportWidth()
  {
    return false;
  }

  public int getScrollableBlockIncrement(Rectangle r,
                    int orientation, int direction)
  {
    return 60;
  }

  public int getScrollableUnitIncrement(Rectangle r,
                    int orientation, int direction)
  {
    return 10;
  }

  public static void main(String args[])
  {
    JFrame frame = new JFrame();
    ClassLoader cl = ClassLoader.getSystemClassLoader();
    ImageIcon icon = new ImageIcon(cl.getResource("images/Jemboss_logo_small.gif"));

    JScrollPane iconScroll = new JScrollPane(new ImageIconJPanel(icon));
    iconScroll.setPreferredSize(new Dimension(100,100));
    frame.getContentPane().add(iconScroll);
    frame.pack();
    frame.setVisible(true);
  }


}

