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

package org.emboss.jemboss.editor;

import java.util.Hashtable;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

/**
*
* Colour menu displayed in a popup menu from the colour
* display pallette (ColourJFrame).
*
*/
class ColourMenu extends JMenu
{
  /** unselected border */
  protected Border unselectedBorder;
  /** selected border   */
  protected Border selectedBorder;
  /** active border     */
  protected Border activeBorder;
  /** hashtable of the colour panes */
  protected Hashtable panes;
  /** selected colour pane */
  protected ColorPane selected;

  public ColourMenu(String name)
  {
    super(name);
    unselectedBorder = new CompoundBorder(
      new MatteBorder(1, 1, 1, 1, getBackground()),
      new BevelBorder(BevelBorder.LOWERED,
      Color.white, Color.gray));
    selectedBorder = new CompoundBorder(
      new MatteBorder(2, 2, 2, 2, Color.red),
      new MatteBorder(1, 1, 1, 1, getBackground()));
    activeBorder = new CompoundBorder(
      new MatteBorder(2, 2, 2, 2, Color.blue),
      new MatteBorder(1, 1, 1, 1, getBackground()));

    JPanel p = new JPanel();
    p.setBorder(new EmptyBorder(5, 5, 5, 5));
    p.setLayout(new GridLayout(8, 8));
    panes = new Hashtable();

    int[] values = new int[] { 0, 128, 192, 255 };
    for (int r=0; r<values.length; r++)
    {
      for (int g=0; g<values.length; g++)
      {
        for (int b=0; b<values.length; b++)
        {
          Color c = new Color(values[r], values[g], values[b]);
          ColorPane pn = new ColorPane(c);
          p.add(pn);
          panes.put(c, pn);
        }
      }
    }
    add(p);
  }

  public void setColor(Color c)
  {
    Object obj = panes.get(c);
    if (obj == null)
      return;
    if (selected != null)
      selected.setSelected(false);
    selected = (ColorPane)obj;
    selected.setSelected(true);
  }

  public Color getColor()
  {
    if (selected == null)
      return null;
    return selected.getColor();
  }

  public void doSelection()
  {
    fireActionPerformed(new ActionEvent(this,
      ActionEvent.ACTION_PERFORMED, getActionCommand()));
  }

  class ColorPane extends JPanel implements MouseListener
  {
    protected Color col;
    protected boolean selected;

    public ColorPane(Color c)
    {
      col = c;
      setBackground(c);
      setBorder(unselectedBorder);
      String msg = "R "+c.getRed()+", G "+c.getGreen()+
        ", B "+c.getBlue();
      setToolTipText(msg);
      addMouseListener(this);
    }

    public Color getColor() { return col; }

    public Dimension getPreferredSize()
    {
      return new Dimension(15, 15);
    }
    public Dimension getMaximumSize() { return getPreferredSize(); }
    public Dimension getMinimumSize() { return getPreferredSize(); }

    public void setSelected(boolean select)
    {
      selected = select;
      if (selected)
        setBorder(selectedBorder);
      else
        setBorder(unselectedBorder);
    }

    public boolean isSelected() { return selected; }
    public void mousePressed(MouseEvent e) {}
    public void mouseClicked(MouseEvent e) {}
    public void mouseReleased(MouseEvent e)
    {
      setColor(col);
      MenuSelectionManager.defaultManager().clearSelectedPath();
      doSelection();
    }

    public void mouseEntered(MouseEvent e)
    {
      setBorder(activeBorder);
    }

    public void mouseExited(MouseEvent e)
    {
      setBorder(selected ? selectedBorder :
        unselectedBorder);
    }
  }
}

