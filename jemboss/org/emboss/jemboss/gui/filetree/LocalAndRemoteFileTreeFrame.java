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
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/

package org.emboss.jemboss.gui.filetree;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;

import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.Browser;

/**
*
* Local and remote combo file manager
*
*/
public class LocalAndRemoteFileTreeFrame extends JFrame
{

  /** local file tree */
  private static DragTree ltree;
  /** close/exit menu item */
  private JMenuItem prefClose;

  /**
  *
  * Displays the remote and local file managers in the
  * same frame.
  * @param mysettings 	jemboss properties
  *
  */
  public LocalAndRemoteFileTreeFrame(final JembossParams mysettings) 
                                throws JembossSoapException
  {
    super("File Manager");
    try
    {  
      final JPanel remotePanel = new JPanel(new BorderLayout());
      final RemoteFileTreePanel rtree =
                           new RemoteFileTreePanel(mysettings,false);
   
      ltree = new DragTree(new File(mysettings.getUserHome()),
                           this, mysettings);
//    ltree = new DragTree(new File(System.getProperty("user.home")), 
//                                                 this, mysettings);

      final JPanel localPanel = new JPanel(new BorderLayout());
      localPanel.add(new LocalTreeToolBar(mysettings), 
                     BorderLayout.NORTH);
      JScrollPane localTree = new JScrollPane(ltree);   

      final JSplitPane treePane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
                                                   localPanel,remotePanel);

      JMenuBar menuBar = new JMenuBar();
      JMenu prefMenu = new JMenu("File");
      prefMenu.setMnemonic(KeyEvent.VK_F);

      final Dimension panelSize = new Dimension(210, 270);

      JRadioButtonMenuItem prefV = new JRadioButtonMenuItem("Vertical Split");
      prefMenu.add(prefV);
      prefV.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          treePane.remove(remotePanel);
          treePane.remove(localPanel);
          treePane.setOrientation(JSplitPane.VERTICAL_SPLIT);
          treePane.setTopComponent(localPanel);
          treePane.setBottomComponent(remotePanel);
//        rtree.setPreferredSize(panelSize);
//        ltree.setPreferredSize(panelSize);
          remotePanel.setPreferredSize(panelSize);
          localPanel.setPreferredSize(panelSize);

          pack();
          treePane.setDividerLocation(0.5);
        }
      });
      prefV.setSelected(true);
      ButtonGroup group = new ButtonGroup();
      group.add(prefV);
     
      JRadioButtonMenuItem prefH = new JRadioButtonMenuItem("Horizontal Split");
      prefMenu.add(prefH);
      prefH.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          treePane.remove(remotePanel);
          treePane.remove(localPanel);
          treePane.setOrientation(JSplitPane.HORIZONTAL_SPLIT);
          treePane.setLeftComponent(localPanel);
          treePane.setRightComponent(remotePanel);
  
//        rtree.setPreferredSize(panelSize);
//        ltree.setPreferredSize(panelSize);
          remotePanel.setPreferredSize(panelSize);
          localPanel.setPreferredSize(panelSize);

          pack();
          treePane.setDividerLocation(0.5);
        }
      });
      group.add(prefH);

// close / exit
      prefClose = new JMenuItem("Close");
      prefClose.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_E, ActionEvent.CTRL_MASK));

      prefClose.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          setVisible(false);
        }
      });
      prefMenu.addSeparator();
      prefMenu.add(prefClose);
      menuBar.add(prefMenu);


      JMenu helpMenu = new JMenu("Help");
      helpMenu.setMnemonic(KeyEvent.VK_H);
      JMenuItem fmh = new JMenuItem("About File Manager");
      fmh.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          ClassLoader cl = this.getClass().getClassLoader();
          try
          {
            URL inURL = cl.getResource("resources/filemgr.html");
            new Browser(inURL,"resources/filemgr.html");
          }
          catch (MalformedURLException mex)
          {
            System.out.println("Didn't find resources/filemgr.html");
          }
          catch (IOException iex)
          {
            System.out.println("Didn't find resources/filemgr.html");
          }
        }
      });
      helpMenu.add(fmh);
      menuBar.add(helpMenu);
      setJMenuBar(menuBar);
      
      rtree.setPreferredSize(panelSize);
      localTree.setPreferredSize(panelSize);
      localPanel.add(localTree,BorderLayout.CENTER);
      remotePanel.add(rtree,BorderLayout.CENTER);

// local panel menu
      JComboBox rootSelect = rtree.getRootSelect();
      JPanel lbar = new JPanel(new BorderLayout());
      Dimension d = new Dimension((int)lbar.getPreferredSize().getWidth(),
                            (int)rootSelect.getMinimumSize().getHeight());
      
      lbar.add(new JLabel(" LOCAL"),BorderLayout.WEST);
      lbar.setPreferredSize(d);
      localPanel.add(lbar,BorderLayout.SOUTH);

// remote panel menu
      JPanel rbar = new JPanel(new BorderLayout());
      rbar.add(new JLabel(" REMOTE "),BorderLayout.WEST);
      rbar.add(rootSelect,BorderLayout.EAST);
      rbar.setPreferredSize(d);
      remotePanel.add(rbar,BorderLayout.SOUTH);

//    treePane.setOneTouchExpandable(true);
      getContentPane().add(treePane);
      pack();

    }
    catch(JembossSoapException jse)
    {
      throw new JembossSoapException();
    }
  }

  /**
  *
  * Set the closing menu item to exit (for standalone
  * file manager)
  *
  */
  public void setExit()
  {
    prefClose.setText("Exit");
    prefClose.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_E, ActionEvent.CTRL_MASK));
    prefClose.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        System.exit(0);
      }
    });

  }

  /**
  *
  * Returns the local file tree
  * @return 	local file tree
  *
  */
  public static DragTree getLocalDragTree()
  {
    return ltree;
  }

}

