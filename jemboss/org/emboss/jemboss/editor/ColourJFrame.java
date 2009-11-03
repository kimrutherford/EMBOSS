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
import java.util.*;
import org.emboss.jemboss.gui.ScrollPanel;

/**
*
* Colour pallette display and editor.
*
*/
public class ColourJFrame extends JFrame
{
  /** Popup menu system                        */
  private JPopupMenu popup;
  /** Container for the all the residue colour */
  private Box YBox = new Box(BoxLayout.Y_AXIS);
  /** Colour scheme                            */
  private Hashtable colourTable;
  /** Scroll pane for the colour pallette      */
  private JScrollPane jspColour;
  /** Associated alignment viewer              */
  private AlignJFrame align;

  /**
  *
  * @param align 	alignment viewer
  *
  */
  public ColourJFrame(AlignJFrame align)
  {
    super("Colour");

    this.align = align;
    JPanel mainPane = (JPanel)getContentPane();
    mainPane.setLayout(new BorderLayout());
    
    ScrollPanel mainColourPanel = new ScrollPanel(new BorderLayout());
    mainColourPanel.add(YBox);
    jspColour = new JScrollPane(mainColourPanel);
    mainPane.add(jspColour);
    mainColourPanel.setBackground(Color.white);

// set up a menu bar
    JMenuBar menuBar = new JMenuBar();

// File menu
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

// exit
    fileMenu.add(new JSeparator());
    JMenuItem fileMenuExit = new JMenuItem("Close");
    fileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(fileMenuExit);
    menuBar.add(fileMenu);
    setJMenuBar(menuBar);
    setSize(70,150);
  }


  /**
  *
  * Set the colour scheme to display
  * @param colourTable	hashtable containing a colour scheme
  *
  */
  public void setCurrentColour(Hashtable colourTable)
  {
    this.colourTable = colourTable;
    Box XBox;
    ColourMenu cm;
    JLabel residueField;
    YBox.removeAll();
    
    Enumeration enumer = colourTable.keys();
    while(enumer.hasMoreElements())
    {
      String res = (String)enumer.nextElement();
      XBox = new Box(BoxLayout.X_AXIS);
      residueField = new JLabel(res);
      residueField.setPreferredSize(new Dimension(20,20));
      residueField.setMaximumSize(new Dimension(20,20));
      XBox.add(residueField);
      ResidueColourPanel colPane = new ResidueColourPanel(
                             res,
                             (Color)colourTable.get(res));
      XBox.add(colPane);
      YBox.add(XBox);
    }
    YBox.add(Box.createVerticalGlue());

    jspColour.getViewport().setViewPosition(new Point(0,0));
  }


  /**
  *
  * Get the colour scheme as a hashtable
  * @return 	the colour scheme as a hashtable
  *
  */
  public Hashtable getCurrentColourScheme()
  {
    return colourTable;
  }



  /**
  *
  * Colour panel for each individual residue in the pallette
  *
  */
  class ResidueColourPanel extends ColourPanel
  {
    private String res;

    /**
    *
    * @param res	residue symbol
    * @param col 	colour of residue
    *
    */
    public ResidueColourPanel(String res,Color col)
    {
      super(res+" Colour",col);
      this.res = res;
    }


    /**
    *
    * Popup menu actions
    * @param e	action event
    *
    */
    public void actionPerformed(ActionEvent e)
    {
      ColourMenu m = (ColourMenu)e.getSource();
      col = m.getColor();
      repaint();
      colourTable.remove(res);
      colourTable.put(res,col);
      align.repaintSequences(colourTable);
    }
  }

}

