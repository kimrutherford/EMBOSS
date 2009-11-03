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

import org.emboss.jemboss.gui.ScrollPanel;
import org.emboss.jemboss.gui.sequenceChooser.CutNPasteTextArea;

import javax.swing.Box;

public class Paste extends ScrollPanel
{
  CutNPasteTextArea cnp;
 
  public Paste()
  {
    Box bdown = Box.createVerticalBox();
    cnp = new CutNPasteTextArea(bdown,"Cut and Paste:");
    add(bdown);
  }

  protected String getSequence()
  {
    return cnp.getText();
  }

}

