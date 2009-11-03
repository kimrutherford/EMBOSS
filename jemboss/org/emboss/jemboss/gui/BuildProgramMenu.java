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
  static private String[] envp;
  /** current appliction loaded */
  private int currentApp = -1;
  /** favorite menu */
  private Favorites favorites;

  /**
  *
  *  @param  menuPane 		menu pane
  *  @param  formPane 		form pane
  *  @param  pform 		pane containing emboss form and job manager
  *  @param  scrollProgForm 	EMBOSS form scroll pane 
  *  @param  mysettings		Jemboss settings
  *  @param  withSoap 		true if using SOAP server
  *  @param  mainMenu		Jemboss main menu bar
  *  @param  f			Jemboss frame
  *
  */
  public BuildProgramMenu(final JPanel menuPane, final JPanel formPane, 
           final JPanel pform, final JScrollPane scrollProgForm,
           final JembossParams mysettings,
           final boolean withSoap, final SetUpMenuBar mainMenu,
           final JFrame f)
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
      
      if(JembossParams.isCygwin())
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
      if(JembossParams.isCygwin())
        env[3] = "EMBOSSCYGROOT=" + JembossParams.getCygwinRoot();

      envp = mysettings.getEmbossEnvironmentArray(env);
    }

    SwingWorker groupworker = new SwingWorker()
    {
      String woss = "";
      ProgList progs;
      
      public Object construct() 
      {
          putJembossLogo();
          f.validate();
          try {
              if (withSoap) {
                  constructWithSoap();
              } else {
                  constructWithoutSoap();
              }
          } catch (Exception e) {
          } finally {
          }
          return woss;
      }

      private void constructWithSoap(){

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

              /*JLabel jl = */new JLabel("<html>"); // not used but speeds first
                                                // ACD form loading which
                                                // uses html
              return null;
            }
          };
          databaseworker.start();
          
          splashing.doneSomething("");

          try
          {
            try
            {
              Hashtable hwoss = (new JembossJarUtil("resources/wossname.jar")).getHash();
              if(hwoss.containsKey("wossname.out"))
                woss = new String((byte[])hwoss.get("wossname.out"));
            }
            catch (Exception ex){}

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
            
      private void constructWithoutSoap(){

          String embossBin = mysettings.getEmbossBin();
          String embossCommand;
        RunEmbossApplication2 rea=null;
        try {
            embossCommand = new String(embossBin + "wossname -colon -gui -auto");
              rea = new RunEmbossApplication2(
                                          embossCommand,envp,null);
              if (rea.getStatus().equals("1")){
                  String error = rea.getInitialIOError();
                  JOptionPane.showMessageDialog(null,
                          "Problem while getting emboss application list:\n"+
                          error,
                          "alert", JOptionPane.ERROR_MESSAGE);
                  return;                  
              }
              rea.waitFor();
              woss = rea.getProcessStdout();
        } catch (RuntimeException e) {
            try {
                System.err.println(rea.getProcessStderr());
            } catch (RuntimeException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }
            JOptionPane.showMessageDialog(null,
                    "Problem while getting emboss application list:"+
                    e.getMessage(),
                    "alert", JOptionPane.ERROR_MESSAGE);
            return;
        }

          embossCommand = new String(embossBin + "showdb -auto");
          rea = new RunEmbossApplication2(embossCommand,envp,null);
          rea.waitFor();
          String showdbOut = rea.getProcessStdout();

          Database d = new Database(showdbOut);
          db = d.getDB();
          
          setMatrices(mysettings);
          setCoddonUsage(mysettings);     
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
        progs = new ProgList(woss,menuBar);

        if(withSoap)
          splashing.doneSomething("");

        int npG = progs.getNumPrimaryGroups();
        menuBar.setLayout(new GridLayout(npG,1));
   
        final int numProgs = progs.getNumProgs();
        final String allAcd[] = progs.getProgsList();
        final String allDes[] = progs.getProgDescription();

        menuPane.add(menuBar, BorderLayout.NORTH);

        favourites(allAcd, allDes, numProgs);
        constructProgramMenu(allAcd, numProgs);        
        JList progList = constructScrollList(allAcd, numProgs);
        constructSearchToolbar(progList);
        f.validate();
      }

// favorites
      private void favourites(final String[] allAcd, final String[] allDes, 
              final int numProgs){
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
                    currentApp = setForm(e, numProgs, allAcd);
                    f.setCursor(cdone);
                  }
                });
              }
              else if(e.getActionCommand().equals("EDIT"))
                favorites.edit(allAcd);
              else
                currentApp = setForm(e, numProgs, allAcd);
              f.setCursor(cdone);
            }
          });
        }
      }

// main menu
      private void constructProgramMenu(final String[] allAcd, final int numProgs){
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
              currentApp= setForm(e, numProgs, allAcd);
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
      }

// program scroll list
      private JList constructScrollList(final String[] allAcd, final int numProgs){
        final JList progList = new JList(allAcd);
        JScrollPane scrollPane = new JScrollPane(progList);

        Box alphaPane = new Box(BoxLayout.Y_AXIS);
        alphaPane.setBorder(BorderFactory.createEmptyBorder(0, 4, 0, 0));
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
            currentApp = index;
            setForm(index);
            f.setCursor(cdone);
          }
        });

        alphaTextPane.add(alphaTextPaneEntry);
        alphaPane.add(alphaTextPane);
        alphaPane.add(scrollPane);

        menuPane.add(alphaPane, BorderLayout.CENTER);

        Dimension dp1 = menuPane.getMinimumSize();
        menuPane.setPreferredSize(dp1);
        menuPane.setMaximumSize(new Dimension(dp1.width,Short.MAX_VALUE));
        menuPane.setMinimumSize(dp1);
        
        scrollProgForm.setBackground(Color.white);

        progList.setSelectionBackground(Color.cyan);

// create listener to build Jemboss program forms
        MouseListener mouseListener = new MouseAdapter()
        {
          public void mouseClicked(MouseEvent e)
          {
            f.setCursor(cbusy);
            JList source = (JList)e.getSource();
            source.setSelectionBackground(Color.cyan);
            int index = source.getSelectedIndex();
            currentApp = index;
            setForm(index);
            f.setCursor(cdone);
          }
        };
        progList.addMouseListener(mouseListener);
        progList.addKeyListener(new ProgListSelectionListener());        
        return progList;
      }


// search tool bar
      private void constructSearchToolbar(JList progList){
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

        menuPane.add(toolBar, BorderLayout.SOUTH);
      }

// put on the logo
      private void putJembossLogo(){
        ClassLoader cl = this.getClass().getClassLoader();
        ImageIcon jlo = new ImageIcon(
                  cl.getResource("images/Jemboss_logo_large.gif"));
        JLabel jlablogo = new JLabel(jlo); 
        
        formPane.add(jlablogo, BorderLayout.CENTER);
        formPane.setBackground(Color.white);
      }
      
      
      private void setForm(int i){
          formPane.removeAll();
          String acdText = getAcdText(progs.getProgsList()[i],
                  mysettings, withSoap);
          new BuildJembossForm(progs.getProgDescription()[i],
                  db,progs.getProgsList()[i],envp,cwd,
                  acdText,withSoap,formPane,mysettings,f);
          JScrollBar verticalScrollBar = scrollProgForm.getVerticalScrollBar();
          JScrollBar horizontalScrollBar = scrollProgForm.getHorizontalScrollBar();
          verticalScrollBar.setValue(verticalScrollBar.getMinimum());
          horizontalScrollBar.setValue(horizontalScrollBar.getMinimum());
          f.repaint();
      }
      
      private int setForm(ActionEvent e, int numProgs, String allAcd[])
      {
          String p = e.getActionCommand();
          int ind = p.indexOf(" ");

          if(ind > -1)
              p = p.substring(0,ind).trim();

          for(int k=0;k<numProgs;k++)
          {
              if(p.equalsIgnoreCase(allAcd[k]))
              {
                  currentApp = k;
                  setForm(k);
                  break;
              }
          }
          return currentApp;
      }

      
      final class ProgListSelectionListener implements KeyListener{

          public void keyPressed(KeyEvent e) {
              if (e.getKeyCode()==KeyEvent.VK_ENTER){
                  JList l = (JList)e.getSource();
                  setForm(l.getSelectedIndex());
              }
          }

          public void keyReleased(KeyEvent e) {
          }

          public void keyTyped(KeyEvent e) {
          }
            
        }
      
    };
    groupworker.start();
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

  private static Set matrixIndicies(String filename){
	  Set s = new HashSet();
	  try {
		  BufferedReader in = new BufferedReader(new FileReader(filename));
		  String line = in.readLine();
		  while(line != null){
			  if (!line.startsWith("#") && line.length()>0){
				  String m = line.split(" ")[0];
				  s.add(m);
			  }
			  line = in.readLine();
		  }
	  } catch (FileNotFoundException e) {
		  e.printStackTrace();
	  } catch (IOException e) {
		  e.printStackTrace();
	  }
	  return s;
  }

  public static void setMatrices(JembossParams mysettings){
	  final Set s = matrixIndicies(mysettings.getEmbossData()+File.separator+"Matrices.protein");
	  s.addAll(matrixIndicies(mysettings.getEmbossData()+File.separator+"Matrices.nucleotide"));
	  s.addAll(matrixIndicies(mysettings.getEmbossData()+File.separator+"Matrices.proteinstructure"));
	  String[] dataFile = (new File(mysettings.getEmbossData())).list(new FilenameFilter()
	  {
		  public boolean accept(File dir, String name)
		  {        	
			  if (s.contains(name)){
				  s.remove(name);
				  return true;
			  }        	
			  return false;
		  };
	  });
	  if (s.size()>0)
		  System.err.println("matrices not resolved to any file: "+s.size());
	  matrices = new Vector();
	  Arrays.sort(dataFile);
	  for(int i=0;i<dataFile.length;i++)
		  matrices.add(dataFile[i]);
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

  public static void setCoddonUsage(JembossParams mysettings){
	  // get the available codon usage tables
	  String[] dataFile = (new File(mysettings.getEmbossData()+
			  File.separator + "CODONS")).list(new FilenameFilter()
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
        System.err.println("BuildProgramMenu: Cannot read acd file " + acdToParse);
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

    public static String[] getEnvp() {
        return envp;
    }

}


