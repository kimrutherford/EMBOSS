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

import java.awt.*;
import javax.swing.*;

/**
*
* Draw sequence name toggle button
*
*/
public class SequenceNameJButton extends JToggleButton
{
  /** sequence */
  private Sequence seq;
  /** boundary width */
  private int boundWidth = 6;
  /** font size */
  private int fontSize = 12;
  /** name label */
  private String nameLabel;
  /** height pad */
  private int ypad=0;
  /** font */
  private Font font = new Font("Monospaced",
                      Font.PLAIN, fontSize);

  /**
  *
  * @param seq 	sequence object
  * @param ypad	height padding
  *
  */
  public SequenceNameJButton(Sequence seq, int ypad)
  {
    super();
    this.seq = seq;
    this.ypad = ypad;
    if(seq.getID().equals(""))
      nameLabel = new String("");
    else
      nameLabel = new String(seq.getID()+"/"+
                             Integer.toString(seq.getLength()));

    setText(nameLabel);
    setBackground(Color.white);
//  setMaximumSize(getPreferredSize());
//  setMinimumSize(getPreferredSize());
    setHorizontalTextPosition(SwingConstants.RIGHT);
    setMargin(new Insets(0,0,0,0));
    setFont(font);
    setBorderPainted(false);
  }

  /**
  *
  * Set the font size
  * @param size	font size
  *
  */
  public void setFontSize(int size)
  {
    fontSize = size;
    this.font = new Font("Monospaced",
                          Font.PLAIN, fontSize);
    setMaximumSize(getPreferredSize());
    setMinimumSize(getPreferredSize());
    setFont(font);
  }

  /**
  *
  * Get the font
  * @return 	font
  *
  */
  public Font getFont()
  {
    return font;
  }

  /**
  *
  * Get the button width
  * @return	 button width
  *
  */
  public int getPanelWidth()
  {
    FontMetrics metrics = getFontMetrics(font);
    return metrics.stringWidth(nameLabel)+boundWidth;
  }

  /**
  *
  * Get the button height
  * @return       button height
  *
  */
  public int getPanelHeight()
  {
    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    return metrics.stringWidth("A")+boundWidth;
  }

  /**
  *
  * Get the preferred size of this component
  * @param	preferred dimension
  *
  */
  public Dimension getPreferredSize()
  {
    return new Dimension(getPanelWidth(),
                         getPanelHeight()+ypad);
  }


}

