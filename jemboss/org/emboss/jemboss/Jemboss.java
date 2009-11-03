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


import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.io.*;

import java.security.Security; //ssl
import java.net.*;

import org.emboss.jemboss.gui.filetree.*;   // local files
import org.emboss.jemboss.gui.*;            // Jemboss graphics
import org.emboss.jemboss.soap.*;           // results manager

/**
*
*  Java interface to EMBOSS (http://www.emboss.org/). This
*  can be run in two different ways. In a STANDALONE mode 
*  with a locally installation of EMBOSS, or
*  CLIENT-SERVER mode download the client from a site,
*  such as the HGMP, which runs the Jemboss server.
*
*/
public class Jemboss implements ActionListener
{

/** Jemboss frame      */
  private JFrame f;
/** Jemboss split pain */
  private JSplitPane pmain; 
/** Local filemanager panel */
  private JPanel p3;
/** Local filemanager */
  public static DragTree tree;
/** Button to open local filemanager */
  private JButton extend;
/** Scroll pane for local filemanager */
  private JScrollPane scrollTree;
/** true if in client-server mode (using SOAP) */
  public static boolean withSoap;
/** to manage the pending results */
  public static PendingResults resultsManager;
/** Jemboss window dimension */
  private static Dimension jdim;
/** Jemboss window dimension with local filemanager displayed */
  private static Dimension jdimExtend;
/** Image for displaying the local filemanager */
  private ImageIcon fwdArrow;
/** Image for hiding the local filemanager */
  private ImageIcon bwdArrow;
/** Jemboss main menu */
  private SetUpMenuBar mainMenu;

  /**
  *
  * Display the Jemboss GUI.
  *
  */
  public Jemboss()
  {

    // initialize settings
    JembossParams mysettings = new JembossParams();

    ClassLoader cl = this.getClass().getClassLoader();
    fwdArrow = new ImageIcon(cl.getResource("images/Forward_arrow_button.gif"));
    bwdArrow = new ImageIcon(cl.getResource("images/Backward_arrow_button.gif"));

    if(withSoap && mysettings.getPublicSoapURL().startsWith("https"))
    {
      //SSL settings

//    System.setProperty ("javax.net.debug", "all");
      com.sun.net.ssl.internal.ssl.Provider p =
                     new com.sun.net.ssl.internal.ssl.Provider();
      Security.addProvider(p);

      //have to do it this way to work with JNLP
      URL.setURLStreamHandlerFactory( new URLStreamHandlerFactory()
      {
        public URLStreamHandler createURLStreamHandler(final String protocol)
        {
          if(protocol != null && protocol.compareTo("https") == 0)
          {
            return new com.sun.net.ssl.internal.www.protocol.https.Handler();
          }
          return null;
        }
      });
//    System.setProperty("java.protocol.handler.pkgs",
//                        "com.sun.net.ssl.internal.www.protocol");

      //location of keystore
      System.setProperty("javax.net.ssl.trustStore",
                        "resources/client.keystore");

      String jembossClientKeyStore = System.getProperty("user.home") + 
                                System.getProperty("file.separator") + 
                                             ".jembossClientKeystore";

      try
      {
        new JembossJarUtil("resources/client.jar").writeByteFile(
                     "client.keystore",jembossClientKeyStore);
        System.setProperty("javax.net.ssl.trustStore",
                            jembossClientKeyStore);
      }
      catch(Exception exp){}

    }

    f = new JFrame("Jemboss");

    // set to the working dir
    if(!withSoap &&
       mysettings.getUserHome().equals(System.getProperty("user.home")))
    {
      mysettings.setUserHome(System.getProperty("user.dir"));
      if(mysettings.getDebug())
        System.out.println("Standalone mode");
    }

    // make the local file manager
    tree = new DragTree(new File(mysettings.getUserHome()),
                        f, mysettings);

    JPanel filePanel = new JPanel(new BorderLayout());
    filePanel.add(new LocalTreeToolBar(mysettings),
                  BorderLayout.NORTH);
    filePanel.add(tree,BorderLayout.CENTER);
    scrollTree = new JScrollPane(filePanel);

    JPanel p1 = new JPanel(new BorderLayout());         // menu panel
    ScrollPanel p2 = new ScrollPanel(new GridLayout()); // emboss form pain
    p3 = new JPanel(new BorderLayout());                // filemanager panel

    JScrollPane scrollProgForm = new JScrollPane(p2);
    JPanel pwork = new JPanel(new BorderLayout());
    JPanel pform = new JPanel(new BorderLayout());

    pform.add(scrollProgForm, BorderLayout.CENTER);
    pwork.add(pform, BorderLayout.WEST);
    pwork.add(p3, BorderLayout.CENTER);

    JMenuBar btmMenu = new JMenuBar();

    // button to extend window
    extend = new JButton(fwdArrow);
    extend.setBorder(BorderFactory.createMatteBorder(0,0,0,0, Color.black));
    extend.addActionListener(this);
    extend.setToolTipText("Open and close file manager.");

    Dimension d = f.getToolkit().getScreenSize();

    resultsManager = new PendingResults(mysettings, withSoap);
    btmMenu.add(resultsManager.statusPanel(f));
    btmMenu.add(extend);
    pform.add(btmMenu,BorderLayout.SOUTH);

    pmain = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
                                  p1,pwork);
    pmain.setOneTouchExpandable(true);

    int arrowSize = fwdArrow.getIconWidth();
    Dimension jform;
    // set window dimensions, dependent on screen size
    if(d.getWidth()<1024)
    {
      jdim = new Dimension(615,500);
      jdimExtend = new Dimension(795,500);
      pmain.setPreferredSize(jdim);
      scrollTree.setPreferredSize(new Dimension(180,500));
      jform = new Dimension(615-180+arrowSize,500);
    }
    else
    {
      jdim = new Dimension(660,540);
      jdimExtend = new Dimension(840,540);
      pmain.setPreferredSize(jdim);
      scrollTree.setPreferredSize(new Dimension(180,540));
      jform = new Dimension(660-180+arrowSize,500);
    }

    // setup the top menu bar
    mainMenu = new SetUpMenuBar(mysettings,f,
                                withSoap);

    // add to Jemboss main frame and locate it center left of screen
    f.getContentPane().add(pmain);
    f.pack();
    f.setLocation(0,((int)d.getHeight()-f.getHeight())/2);

    new BuildProgramMenu(p1,p2,pform,scrollProgForm,
                         mysettings,withSoap,
                         mainMenu,f,jform);

    f.addWindowListener(new winExit());
  }


  /**
  *
  *  Action event to open the file manager
  *  @param ae		the action event generated
  *
  */
  public void actionPerformed(ActionEvent ae)
  {
    if( p3.getComponentCount() > 0 )
    {
      p3.remove(0);
      extend.setIcon(fwdArrow);
      pmain.setPreferredSize(jdim);
      f.pack();
    }
    else
    {
      p3.add(scrollTree, BorderLayout.CENTER);
      extend.setIcon(bwdArrow);
      pmain.setPreferredSize(jdimExtend);
      f.pack();
    }

  }


  /**
  *
  * Extends WindowAdapter to close window and
  * save any session properties and clean up
  *
  */
  class winExit extends WindowAdapter
  {
     public void windowClosing(WindowEvent we)
     {
        mainMenu.exitJemboss();
     }
  }


  /**
  *
  * Launches Jemboss in standalone or client-server mode.
  *
  */
  public static void main (String args[])
  {
    
    if(args.length > 0)
    {
      if(args[0].equalsIgnoreCase("local"))
        withSoap = false; 
      else 
        withSoap = true; 
    }
    else
      withSoap = true;

    new Jemboss();
  }

}

