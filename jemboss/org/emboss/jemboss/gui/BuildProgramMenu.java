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

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

import org.emboss.jemboss.JembossJarUtil;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.programs.*;      // running EMBOSS programs
import org.emboss.jemboss.gui.startup.*;   // finds progs, groups, docs & db's
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.gui.form.*;      // prog forms constructed from ACD

/**
*
* BuildProgramMenu class construct the program menus.
*
*/
public class BuildProgramMenu
{
  /** database names */
  private static String db[] = {"",""};
  /** matrices */
  private static Vector matrices = new Vector();
  /** codons usage tables  */
  private static Vector codons = new Vector();
  /** acd files cache */
  private Hashtable acdStore = new Hashtable();   
  /** login window */
  private AuthPopup splashing;
  /** thread for progress monitor on the login window */
  private SplashThread splashThread;
  /** environment vars */
  private String[] envp;
  /** current appliction loaded */
  private int currentApp = -1;
  /** favorite menu */
  private Favorites favorites;

  /**
  *
  *  @param  p1 		menu pane
  *  @param  p2 		form pane
  *  @param  pform 		pane containing emboss form and job manager
  *  @param  scrollProgForm 	EMBOSS form scroll pane 
  *  @param  mysettings		Jemboss settings
  *  @param  withSoap 		true if using SOAP server
  *  @param  mainMenu		Jemboss main menu bar
  *  @param  f			Jemboss frame
  *  @param  jform 		form pane dimension
  *
  */
  public BuildProgramMenu(final JPanel p1, final ScrollPanel p2, 
           final JPanel pform, final JScrollPane scrollProgForm,
           final JembossParams mysettings,
           final boolean withSoap, final SetUpMenuBar mainMenu,
           final JFrame f, final Dimension jform)
  {
  
    final String fs = new String(System.getProperty("file.separator"));
    final String cwd = new String(System.getProperty("user.dir") + fs);
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  
    if(withSoap)
    {  
      splashing = new AuthPopup(mysettings,400);
      if(mysettings.getUseAuth())
      splashing.setBottomPanel();
      splashing.setSize(380,200);
      splashing.pack();
      splashing.setVisible(true);

      splashThread = new SplashThread(splashing,400-4);
      splashThread.start();
    }
    else 
    {
      String[] env = null;
      
      if(mysettings.isCygwin())
        env = new String[4];  /* environment vars */
      else
        env = new String[3];
      String ps = new String(System.getProperty("path.separator"));
      String embossBin  = mysettings.getEmbossBin();
      String embossPath = mysettings.getEmbossPath();
      embossPath = new String("PATH" + ps +
                      embossPath + ps + embossBin + ps);
      env[0] = "PATH=" + embossPath;
      env[1] = "PLPLOT_LIB=" + mysettings.getPlplot();
//    env[2] = "EMBOSS_DATA=" + mysettings.getEmbossData();
      env[2] = "HOME=" + System.getProperty("user.home") + fs;
      if(mysettings.isCygwin())
        env[3] = "EMBOSSCYGROOT=" + mysettings.getCygwinRoot();

      envp = mysettings.getEmbossEnvironmentArray(env);
    }

    
    SwingWorker groupworker = new SwingWorker() 
    {
      String woss = "";

      public Object construct() 
      {
        if(withSoap) 
        {
          mainMenu.setEnableFileManagers(false);
          mainMenu.setEnableShowResults(false);

          if(mysettings.getPublicSoapURL().startsWith("https") &&
             !mysettings.getUseHTTPSProxy())
          {
            System.setProperty("https.proxyHost", "");
            System.setProperty("http.proxyHost", "");
            System.setProperty("proxyHost", "");  
            String settings[] = new String[1];
            settings[0] = new String("proxy.override=true");
            mysettings.updateJembossPropStrings(settings);
          }

          SwingWorker databaseworker = new SwingWorker()
          {
            public Object construct()
            {
              ShowDB showdb = null;
              try
              {
                showdb  = new ShowDB(mysettings);
              }
              catch (Exception ex)
              {
                splashing.doneSomething("Cannot connect!");
                ServerSetup ss = new ServerSetup(mysettings);
                int sso = JOptionPane.showConfirmDialog(f,ss,
                           "Check Settings",
                           JOptionPane.OK_CANCEL_OPTION,
                           JOptionPane.ERROR_MESSAGE,null);
                if(sso == JOptionPane.OK_OPTION)
                  ss.setNewSettings();
                else
                  System.exit(0);

                try
                {
                  showdb  = new ShowDB(mysettings);
                }
                catch (Exception exp)
                {
                  exp.printStackTrace();
                }
              }
              String showdbOut = showdb.getDBText();

              Database d = new Database(showdbOut);
              db = d.getDB();
              mainMenu.setEnableFileManagers(true);
              mainMenu.setEnableShowResults(true);
              splashing.doneSomething("");
              splashThread.setInterval(0);

              matrices = showdb.getMatrices();  // get the available matrices
              codons   = showdb.getCodonUsage();

              JLabel jl = new JLabel("<html>"); // not used but speeds first
                                                // ACD form loading which
                                                // uses html
              return null;
            }
          };
          databaseworker.start();
          
          splashing.doneSomething("");

          int iloop = 0;

          try
          {
            try
            {
              Hashtable hwoss = (new JembossJarUtil("resources/wossname.jar")).getHash();
              if(hwoss.containsKey("wossname.out"))
                woss = new String((byte[])hwoss.get("wossname.out"));

//            mainMenu.setEnableFileManagers(false);
//            mainMenu.setEnableShowResults(false);

//            Hashtable hshowdb = (new JembossJarUtil("resources/showdb.jar")).getHash();
//            mainMenu.setEnableFileManagers(false);
//            mainMenu.setEnableShowResults(false);   

//            if(hshowdb.containsKey("showdb.out"))
//            {
//              String showdbOut = new String((byte[])hshowdb.get("showdb.out"));
//              Database d = new Database(showdbOut);
//              db = d.getDB();
//            }
            }
            catch (Exception ex){ System.out.println("calling the server"); }

            if(woss.equals(""))
            {
              GetWossname ewoss = new GetWossname(mysettings);
              woss = ewoss.getWossnameText(); 
              mainMenu.setEnableFileManagers(true);
              mainMenu.setEnableShowResults(true);
            }
            
            splashing.doneSomething("");
          } 
          catch(Exception e)
          {
            splashing.doneSomething("Cannot connect!");
            ServerSetup ss = new ServerSetup(mysettings);
            int sso = JOptionPane.showConfirmDialog(f,ss,
                           "Check Settings",
                           JOptionPane.OK_CANCEL_OPTION,
                           JOptionPane.ERROR_MESSAGE,null);
            if(sso == JOptionPane.OK_OPTION)
              ss.setNewSettings();
            else
              System.exit(0);
          }
        } 
        else 
        {
          String embossBin = mysettings.getEmbossBin();
          String embossCommand = new String(embossBin + "wossname -colon -gui -auto");
          RunEmbossApplication2 rea = new RunEmbossApplication2(
                                      embossCommand,envp,null);
          rea.waitFor();
          woss = rea.getProcessStdout();

          embossCommand = new String(embossBin + "showdb -auto");
          rea = new RunEmbossApplication2(embossCommand,envp,null);
          rea.waitFor();
          String showdbOut = rea.getProcessStdout();

          Database d = new Database(showdbOut);
          db = d.getDB();

          // get the available matrices
          String dataFile[] = (new File(mysettings.getEmbossData())).list(new FilenameFilter()
          {
            public boolean accept(File dir, String name)
            {
              File fileName = new File(dir, name);
              return !fileName.isDirectory();
            };
          });

          matrices = new Vector();
          for(int i=0;i<dataFile.length;i++)
            matrices.add(dataFile[i]);
          
          // get the available codon usage tables
          dataFile = (new File(mysettings.getEmbossData()+
                                  "/CODONS")).list(new FilenameFilter()
          {
            public boolean accept(File dir, String name)
            {
              File fileName = new File(dir, name);
              return !fileName.isDirectory();
            };
          });

          codons = new Vector();
          for(int i=0;i<dataFile.length;i++)
            codons.add(dataFile[i]);
        }

        return woss;
      }


      public void finished() 
      {
// sets the delay for dismissing tooltips
        MultiLineToolTipUI.initialize();
        ToolTipManager toolTipManager = ToolTipManager.sharedInstance();
        toolTipManager.setDismissDelay(80000);

        try
        {
          acdStore = (new JembossJarUtil("resources/acdstore.jar")).getHash();
        }
        catch (Exception ex){}

// program menu
        JMenuBar menuBar = new JMenuBar();
        ProgList progs = new ProgList(woss,menuBar);

        if(withSoap)
          splashing.doneSomething("");

        int npG = progs.getNumPrimaryGroups();
        menuBar.setLayout(new GridLayout(npG,1));
   
        final int numProgs = progs.getNumProgs();
        final String allAcd[] = progs.getProgsList();
        final String allDes[] = progs.getProgDescription();

        p1.add(menuBar, BorderLayout.NORTH);
        f.setVisible(true);

// favorites
        favorites = new Favorites(mainMenu.getFavoriteJMenu());
        JMenuItem favItems[] = favorites.getFavorites();
        for(int i=0; i<favItems.length;i++)
        {
          favItems[i].addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              f.setCursor(cbusy);
              if(e.getActionCommand().equals("ADD") &&
                 currentApp > -1)
              {
                JMenuItem favItem = favorites.add(allAcd[currentApp],
                                              allDes[currentApp]);
                favItem.addActionListener(new ActionListener()
                {
                  public void actionPerformed(ActionEvent e)
                  {
                    f.setCursor(cbusy);
                    currentApp = setForm(e,f,scrollProgForm,numProgs,p2,
                             mysettings,cwd,allAcd,allDes,withSoap);
                    f.setCursor(cdone);
                  }
                });
              }
              else if(e.getActionCommand().equals("EDIT"))
                favorites.edit(allAcd);
              else
                currentApp = setForm(e,f,scrollProgForm,numProgs,p2,
                             mysettings,cwd,allAcd,allDes,withSoap);
              f.setCursor(cdone);
            }
          });
        }

// main menu
        JMenuItem mi[] = new JMenuItem[numProgs];
        mi = progs.getMenuItems();
        int nm = progs.getNumberMenuItems();

// create action listeners into menu to build Jemboss program forms
        for(int i=0; i<nm;i++)
        {
          mi[i].addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              f.setCursor(cbusy);
              currentApp= setForm(e,f,scrollProgForm,numProgs,p2,
                          mysettings,cwd,allAcd,allDes,withSoap);
              f.setCursor(cdone);
            }
          });

          if(withSoap)
          {
            JFrame splashf = splashing.getSplashFrame();
            if(splashf.isVisible())
              splashf.toFront();
          }
        }

// program scroll list
        final JList progList = new JList(allAcd);
        JScrollPane scrollPane = new JScrollPane(progList);

        Box alphaPane = new Box(BoxLayout.Y_AXIS);
        Box alphaTextPane = new Box(BoxLayout.X_AXIS);
        alphaPane.add(Box.createRigidArea(new Dimension(0,10)));
        alphaTextPane.add(new JLabel("GoTo:"));
        alphaTextPane.add(Box.createRigidArea(new Dimension(5,0)));

        final JTextField alphaTextPaneEntry = new JTextField(12);
        alphaTextPaneEntry.setMaximumSize(new Dimension(100,20));
        //scroll program list on typing 
        alphaTextPaneEntry.getDocument().addDocumentListener(new DocumentListener()
        {
          public void insertUpdate(DocumentEvent e)
          {
            updateScroll();
          }
          public void removeUpdate(DocumentEvent e) 
          {
            updateScroll();
          }
          public void changedUpdate(DocumentEvent e) {}
          public void updateScroll()
          {
            for(int k=0;k<numProgs;k++)
              if(allAcd[k].startsWith(alphaTextPaneEntry.getText()))
              {
                progList.ensureIndexIsVisible(k);
                progList.setSelectionBackground(Color.cyan);
                progList.setSelectedIndex(k);
                break;
              }
          }
        });
        //load program form on carriage return
        alphaTextPaneEntry.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            f.setCursor(cbusy);
            int index = progList.getSelectedIndex();
            p2.removeAll();
            currentApp = index;
            String acdText = getAcdText(allAcd[index],mysettings,
                                                       withSoap);
            BuildJembossForm bjf = new BuildJembossForm(allDes[index],
                                  db,allAcd[index],envp,cwd,
                                  acdText,withSoap,p2,mysettings,f);
            scrollProgForm.setViewportView(p2);   
            JViewport vp = scrollProgForm.getViewport();
            vp.setViewPosition(new Point(0,0));
            f.setCursor(cdone);
          }
        });

        alphaTextPane.add(alphaTextPaneEntry);
        alphaPane.add(alphaTextPane);
        alphaPane.add(scrollPane);

        p1.add(alphaPane, BorderLayout.CENTER);

        Dimension dp1 = p1.getMinimumSize();
        dp1 = new Dimension((int)dp1.getWidth()-10,(int)dp1.getHeight());
        p1.setPreferredSize(dp1);
        p1.setMaximumSize(dp1);
        p1.setMinimumSize(dp1);


// search tool bar
        final JRadioButton radioAND = new JRadioButton("AND");
        JRadioButton radioOR  = new JRadioButton("OR");
        Font fnt = new Font("SansSerif", Font.BOLD, 10);
        radioAND.setFont(fnt);
        radioOR.setFont(fnt);
        radioAND.setSelected(true);
                                                                                            
        ButtonGroup group = new ButtonGroup();
        group.add(radioAND);
        group.add(radioOR);

        JToolBar toolBar = new JToolBar(JToolBar.VERTICAL);
        final JTextField search = new JTextField(12);
        search.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            new KeywordSearch(search,woss,mysettings,withSoap,
                              radioAND.isSelected());
          }
        });

        Box bacross = Box.createHorizontalBox();
        JLabel keyw = new JLabel("Keyword Search");
        keyw.setFont(fnt);
        bacross.add(keyw);
        JButton go = new JButton("GO");
        go.setFont(new Font("monospaced", Font.BOLD, 10));
        go.setMargin(new Insets(0,0,0,0));
        go.setForeground(new Color(0,51,102));
        go.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            new KeywordSearch(search,woss,mysettings,withSoap,
                              radioAND.isSelected());
          }
        });
        bacross.add(Box.createHorizontalStrut(2));
        bacross.add(go);
        bacross.add(Box.createHorizontalGlue());
        toolBar.add(bacross);
        toolBar.add(search);

        bacross = Box.createHorizontalBox();
        bacross.add(radioAND);
        bacross.add(radioOR);
        bacross.add(Box.createHorizontalGlue());
        toolBar.add(bacross);

        p1.add(toolBar, BorderLayout.SOUTH);

// put on the logo
        ClassLoader cl = this.getClass().getClassLoader();
        ImageIcon jlo = new ImageIcon(
                  cl.getResource("images/Jemboss_logo_large.gif"));
        JLabel jlablogo = new JLabel(jlo); 
        JPanel pFront = new JPanel();
        pFront.setBackground(Color.white);
        pFront.add(jlablogo);

// ensure fill the screen here as pform is BorderLayout.WEST
        int pwidth = (int)(f.getSize().getWidth()-p1.getSize().getWidth())-14;
        Dimension d = new Dimension(pwidth,100);
        pform.setPreferredSize(d);
        pform.setMinimumSize(d);
        jlablogo.setPreferredSize(jform);

        p2.add(pFront);

        progList.setSelectionBackground(Color.cyan);

// create listener to build Jemboss program forms
        MouseListener mouseListener = new MouseAdapter()
        {
          public void mouseClicked(MouseEvent e)
          {
//          System.gc();
            f.setCursor(cbusy);
            JList source = (JList)e.getSource();
            source.setSelectionBackground(Color.cyan);
            int index = source.getSelectedIndex();
            p2.removeAll();
            currentApp = index;
            String acdText = getAcdText(allAcd[index],mysettings,
                                                       withSoap);
            BuildJembossForm bjf = new BuildJembossForm(allDes[index],
                                  db,allAcd[index],envp,cwd,
                                  acdText,withSoap,p2,mysettings,f);
            scrollProgForm.setViewportView(p2);
            JViewport vp = scrollProgForm.getViewport();
            vp.setViewPosition(new Point(0,0));
            f.setCursor(cdone);
          }
        };
        progList.addMouseListener(mouseListener);

        p1.setVisible(false);
        p1.setVisible(true);

      }
    };
    groupworker.start();

  }

  private int setForm(ActionEvent e, JFrame f, JScrollPane scrollProgForm,
                      int numProgs, ScrollPanel p2, JembossParams mysettings, 
                      String cwd, String allAcd[], String allDes[],
                      boolean withSoap)
  {
//  JMenuItem source = (JMenuItem)(e.getSource());
                  
    String p = e.getActionCommand();
    int ind = p.indexOf(" ");

    if(ind > -1)
      p = p.substring(0,ind).trim();
                                                                                                     
    for(int k=0;k<numProgs;k++)
    {
      if(p.equalsIgnoreCase(allAcd[k]))
      {
        p2.removeAll();
        currentApp = k;

        String acdText = getAcdText(allAcd[k],mysettings,
                                               withSoap);
        BuildJembossForm bjf = new BuildJembossForm(allDes[k],
                      db,allAcd[k],envp,cwd,acdText,
                      withSoap,p2,mysettings,f);
        scrollProgForm.setViewportView(p2);
        JViewport vp = scrollProgForm.getViewport();
        vp.setViewPosition(new Point(0,0));
        break;
      }
    }
    return currentApp;
  }

  /**
  *
  * List of available EMBOSS databases
  * @return 	list of databases
  *
  */
  protected static String[] getDatabaseList()
  {
    return db;
  }

  /**
  *
  * Contains all scoring matrix files
  *
  */
  public static Vector getMatrices()
  {
    return matrices;
  }

  /**
  *
  * Contains all codon usage tables
  *
  */
  public static Vector getCodonUsage()
  {
    return codons;
  }

  /**
  *
  * Get the contents of an ACD file in the form of a String
  * @param applName		application name
  * @param mysettings		jemboss properties
  * @param withSoap		true if in client-server mode
  * @return	 		String representation of the ACD
  *
  */
  private String getAcdText(String applName, JembossParams mysettings, 
                            boolean withSoap)
  {

    String acdText = new String("");
    String line;

    if(!withSoap)
    {
      String acdDirToParse = mysettings.getAcdDirToParse();
      String acdToParse = acdDirToParse.concat(applName).concat(".acd");
      try
      {
        BufferedReader in = new BufferedReader(new FileReader(acdToParse));
        StringBuffer buff = new StringBuffer();
        
        while((line = in.readLine()) != null)
          buff = buff.append(line + "\n");

        in.close();
        acdText = buff.toString();
      }
      catch (IOException e)
      {
        System.out.println("BuildProgramMenu: Cannot read acd file " + acdText);
      }
    }
    else 
    {
      if(acdStore.containsKey(applName+".acd"))
      {
        Object obj = acdStore.get(applName+".acd");

        if(obj.getClass().getName().equals("java.lang.String"))
          acdText = (String)obj;
        else
          acdText = new String((byte[])obj);
 
//      System.out.println("Retrieved "+applName+" acd file from cache");
      }
      else
      {
        GetACD progacd = new GetACD(applName,mysettings);
        acdText = progacd.getAcd();
//      System.out.println("Retrieved "+applName+" acd file via soap");
        acdStore.put(applName+".acd",acdText);
      }
    }
    return acdText;
  }

}


