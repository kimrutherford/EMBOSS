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
import java.util.prefs.Preferences;
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
/** main split panel that contains all panels generated here */
  private JSplitPane pmain; 
/** Local filemanager panel */
  private JPanel localFileManagerPanel;
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
/** Image for displaying the local filemanager */
  private ImageIcon displayFileManager;
/** Image for hiding the local filemanager */
  private ImageIcon hideFileManager;
/** Jemboss main menu */
  private SetUpMenuBar mainMenu;

  final String fileseparator = System.getProperty("file.separator");

  //Default values for this frame's preferences
  public static final int DEFAULT_WINDOW_X = 50;
  public static final int DEFAULT_WINDOW_Y = 50;
  public static final int DEFAULT_WINDOW_WIDTH = 300;
  public static final int DEFAULT_WINDOW_HEIGHT = 100;

  // Keys for this frame's preferences
  public static final String WINDOW_X_KEY = "JEMBOSS_WINDOW_X";
  public static final String WINDOW_Y_KEY = "JEMBOSS_WINDOW_Y";
  public static final String WINDOW_WIDTH_KEY = "JEMBOSS_WINDOW_WIDTH";
  public static final String WINDOW_HEIGHT_KEY = "JEMBOSS_WINDOW_HEIGHT";
  public static final String DIVIDER1_LOCATION_KEY = "JEMBOSS_DIVIDER1_LOCATION";
  public static final String DIVIDER2_LOCATION_KEY = "JEMBOSS_DIVIDER2_LOCATION";
  public static final String LOCAL_FILE_MAN_KEY = "LOCAL_FILE_MANAGER";
  
  //A reference to a Preferences object
  private Preferences myPreferences = null;
  
  /**
  *
  * Display the Jemboss GUI.
  *
  */
  public Jemboss()
  {
    // initialize settings
    JembossParams mysettings = new JembossParams();
    
    if (JembossParams.isJembossServer() == false)
    	withSoap = false;

    ClassLoader cl = this.getClass().getClassLoader();
    displayFileManager = new ImageIcon(cl.getResource("images/Forward_arrow_button.gif"));
    hideFileManager = new ImageIcon(cl.getResource("images/Backward_arrow_button.gif"));

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
                                                       fileseparator + 
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
    f.addWindowListener(new winExit());
    checkHomeDirectories(mysettings);
    initMainFrame(mysettings);
  }
  
  
  /**
  *
  * Checks and sets user-home and results-home directories.
  * When the user.home property has its default value
  * and the operating system is windows jemboss 
  * user-home directory is set to ${EMBOSS_ROOT}\${user.name},
  * similarly the jemboss results-home directory is set to
  * ${EMBOSS_ROOT}\${user.name}\results
  * 
  */  
  private void checkHomeDirectories(JembossParams mysettings){
    if(!withSoap &&
       mysettings.getUserHome().equals(System.getProperty("user.home")))
    {
        if (System.getProperty("os.name").startsWith("Windows")) {
            String emboss_root = mysettings.getEmbossBin();
            String userhome = emboss_root + fileseparator
            + System.getProperty("user.name");
            String resultshome = userhome + fileseparator + "results";
            File rh = new File(resultshome);
            if (!rh.exists() && !rh.mkdirs()){               
                JOptionPane.showMessageDialog(f,
                        ("Jemboss not able to set its results directory "+
                        "to the following location:\n"+resultshome), 
                        "Message", JOptionPane.INFORMATION_MESSAGE);
                // TODO: we should have a checkbox in the JOptionPane
                // to let user select not to see this notification later
            }
            if (rh.exists()) {
                mysettings.setUserHome(userhome);
                mysettings.setResultsHome(resultshome);
            }
        } else
            mysettings.setUserHome(System.getProperty("user.dir"));
        if (mysettings.getDebug())
            System.out.println("Standalone mode");
    }
  }

  /** pane on the left for listing of available programs*/
  JPanel menuPanel;
  
  /** pane for the applications form and for the local file tree*/
  JSplitPane pright;
  
  /** pane in the center for entering application parameters and starting jobs*/
  JPanel pform;
  /**
  * 
  * Initialises the main frame
  *  
  * @param mysettings
  */
  private void initMainFrame(JembossParams mysettings){
    // make the local file manager
    tree = new DragTree(new File(mysettings.getUserHome()),
                        f, mysettings);

    JPanel filePanel = new JPanel(new BorderLayout());
    filePanel.add(new LocalTreeToolBar(mysettings),
                  BorderLayout.NORTH);
    filePanel.add(tree,BorderLayout.CENTER);
    scrollTree = new JScrollPane(filePanel);

    menuPanel = new JPanel(new BorderLayout());
    JPanel embossFormPanel = new JPanel(new GridLayout());
    localFileManagerPanel = new JPanel(new BorderLayout());
    localFileManagerPanel.add(scrollTree);

    JScrollPane scrollProgForm = new JScrollPane(embossFormPanel);
    
    scrollProgForm.setBackground(Color.white);

    pform = new JPanel(new BorderLayout());

    pform.add(scrollProgForm, BorderLayout.CENTER);

    
    pright = new JSplitPane();
    pright.setLeftComponent(pform);
    localFileManagerPanel.add(scrollTree);
    pright.setRightComponent(null);
    pright.setDividerSize(0);
    

    JMenuBar btmMenu = new JMenuBar();

    // button to extend window
    extend = new JButton(displayFileManager);
    extend.setBorder(BorderFactory.createMatteBorder(0,0,0,0, Color.black));
    extend.addActionListener(this);
    extend.setToolTipText("Open file manager.");

    resultsManager = new PendingResults(mysettings, withSoap);
    btmMenu.add(resultsManager.statusPanel(f));
    btmMenu.add(extend);
    pform.add(btmMenu,BorderLayout.SOUTH);

    pmain = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
                                  menuPanel,pright);

    // setup the top menu bar
    mainMenu = new SetUpMenuBar(mysettings,f,
                                withSoap);

    new BuildProgramMenu(menuPanel,embossFormPanel,pform,scrollProgForm,
                         mysettings,withSoap,
                         mainMenu,f);

    setWindowSizesAndLocation();

  }


  /**
  * 
  * Sets window dimensions, dependent on screen size
  * @param screensize
  * 
  */
  private void setWindowSizesAndLocation() {
      Dimension screensize = f.getToolkit().getScreenSize();
      
   // Obtain a references to a Preferences object
      myPreferences = Preferences.userNodeForPackage(Jemboss.class);
      if (screensize.getWidth() < 1024) {
          menuPanel.setPreferredSize(new Dimension(130, 500));
          pform.setPreferredSize(new Dimension(500, 500));
          localFileManagerPanel.setPreferredSize(new Dimension(180, 500));
      } else {
          menuPanel.setPreferredSize(new Dimension(140, 500));
          pform.setPreferredSize(new Dimension(520, 540));
          localFileManagerPanel.setPreferredSize(new Dimension(200, 540));
      }
      pform.setMinimumSize(pform.getPreferredSize());
      menuPanel.validate();
      int a = menuPanel.getPreferredSize().width;
      pmain.setDividerLocation(myPreferences.getInt(DIVIDER1_LOCATION_KEY, a));
      if (myPreferences.getBoolean(LOCAL_FILE_MAN_KEY, false)){
          pright.setRightComponent(localFileManagerPanel);
          pright.setDividerLocation(myPreferences.getInt(DIVIDER2_LOCATION_KEY,300));
          extend.setIcon(hideFileManager);
          extend.setToolTipText("Close file manager.");
      }
      f.setContentPane(pmain);
      f.validate();
      f.pack();
      
      // locate main frame center left of screen
      int x = myPreferences.getInt(WINDOW_X_KEY, 0);
      int y = (screensize.height - f.getPreferredSize().height) / 2;
      y = myPreferences.getInt(WINDOW_Y_KEY, y);
      int width = myPreferences.getInt(WINDOW_WIDTH_KEY, pmain.getWidth()+10);
      int height = myPreferences.getInt(WINDOW_HEIGHT_KEY, pmain.getHeight()+10);
      f.setBounds(x, y, width, height);
      f.setVisible(true);
  }


  /**
  *
  *  Action event to open/hide the file manager
  *  @param ae		the action event generated
  *
  */
  public void actionPerformed(ActionEvent ae)
  {
    if(extend.getIcon() == hideFileManager )
    {
      extend.setIcon(displayFileManager);
      extend.setToolTipText("Open file manager.");
      pright.setRightComponent(null);      
      pform.setPreferredSize(new Dimension(
              pform.getPreferredSize().width,
              pform.getHeight()));
      f.pack();
      localFileManagerPanel.setPreferredSize(localFileManagerPanel.getSize());
    }
    else // opens local file manager
    {
      extend.setIcon(hideFileManager);
      extend.setToolTipText("Close file manager.");
      pright.setRightComponent(localFileManagerPanel);
      pright.validate();

      pform.setPreferredSize(new Dimension(
              pform.getPreferredSize().width,
              pform.getHeight()));
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
        // Save the state of the window as preferences
        myPreferences.putInt(WINDOW_WIDTH_KEY, f.getWidth());
        myPreferences.putInt(WINDOW_HEIGHT_KEY, f.getHeight());
        myPreferences.putInt(WINDOW_X_KEY, f.getX());
        myPreferences.putInt(WINDOW_Y_KEY, f.getY());
        myPreferences.putInt(DIVIDER1_LOCATION_KEY, pmain.getDividerLocation());
        myPreferences.putInt(DIVIDER2_LOCATION_KEY, pright.getDividerLocation());
        myPreferences.putBoolean(LOCAL_FILE_MAN_KEY, pright.getRightComponent()!=null);
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
    
    JembossParams.setStandaloneMode(!withSoap);

    new Jemboss();
  }

}

