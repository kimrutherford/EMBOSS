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

package org.emboss.jemboss.gui.filetree;


import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.io.*;

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.*;            // Jemboss graphics

/**
*
*
*/
public class LocalTreeToolBar extends JToolBar
{

  /**
  *
  * Set up a menu and tool bar
  *
  */
  public LocalTreeToolBar(final JembossParams mysettings)
  {
    super();
   
    final Dimension buttonSize = new Dimension(22,24);

    JButton upBt = new JButton()
    {
      public void paintComponent(Graphics g)
      {
        super.paintComponent(g);

        Graphics2D g2 = (Graphics2D)g;

        g2.setColor(new Color(0,128,0));
        float loc1[][] = { {11,18}, {7,18}, {7,14},
                           {3,14},  {11,4} };

        g2.fill(Browser.makeShape(loc1));
        g2.setColor(Color.green);

        float loc2[][] = { {11,18}, {15,18}, {15,14},
                           {19,14},  {11,4} };
        g2.fill(Browser.makeShape(loc2));
        setSize(22,24);
      }
    };
    upBt.setPreferredSize(buttonSize);
    upBt.setMaximumSize(buttonSize);
    upBt.setToolTipText("Move Up a Directory");

    upBt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        File child = new File(mysettings.getUserHome());
        setDirectory(child.getParentFile(), mysettings);
      }
    });
    add(upBt);

// working dir
   JButton shortCut2 = new JButton()
   {
      public void paintComponent(Graphics g)
      {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D)g;
        Font font = new Font("Monospaced", Font.BOLD, 14);
        g2.setFont(font);

        g2.setColor(Color.black);
        g2.drawString("W",4,18);
        g2.setColor(Color.red);
        g2.drawString("D",10,15);
        setSize(22,24);
      }
    };
    shortCut2.setPreferredSize(buttonSize);
    shortCut2.setMaximumSize(buttonSize);
    shortCut2.setToolTipText("Move To Working Directory");

    shortCut2.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        File cwd = new File(mysettings.getResultsHome());
        setDirectory(cwd, mysettings);
      }
    });
    add(shortCut2);

// home button
    JButton homeBt = new JButton()
    {
      public void paintComponent(Graphics g)
      {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D)g;

        g2.setColor(Color.blue);
        float loc1[][] = { {3,14}, {11,3}, {19,14},
                           {17,14}, {17,18}, {5,18}, {5,14} };
        g2.fill(Browser.makeShape(loc1));

        setSize(22,24);
      }
    };
    homeBt.setPreferredSize(buttonSize);
    homeBt.setMaximumSize(buttonSize);
    homeBt.setToolTipText("Move To Home Directory");

    homeBt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        File cwd = new File(mysettings.getUserHome());
//      File cwd = new File(System.getProperty("user.home"));
        setDirectory(cwd, mysettings);
      }
    });
    add(homeBt);
    setPreferredSize(new Dimension((int)getPreferredSize().getWidth(),34));
                             
  }

  private void setDirectory(File f, JembossParams mysettings)
  {
    if(f.exists() && f.canRead())
    {
      String cwd = f.getAbsolutePath();
//    mysettings.setUserHome(cwd);
      org.emboss.jemboss.Jemboss.tree.newRoot(cwd);
      if(SetUpMenuBar.localAndRemoteTree != null)
      LocalAndRemoteFileTreeFrame.getLocalDragTree().newRoot(cwd);

      if(!f.canWrite())
        JOptionPane.showMessageDialog(null,
                      "You cannot write to this directory.",
                      "Warning: Write",
                       JOptionPane.WARNING_MESSAGE);
    }
    else
      JOptionPane.showMessageDialog(null,
                      "No access to this directory.",
                      "Error: Access",
                      JOptionPane.ERROR_MESSAGE);
  }

}

