/*******************************************************************
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

import org.emboss.jemboss.*;
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.gui.filetree.*;
import org.emboss.jemboss.draw.DNADraw;
import org.emboss.jemboss.draw.Wizard;

import java.net.URL;

import java.awt.*;
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.util.Vector;

/**
*
* Sets the top menu bar for Jemboss main window
*
*/
public class SetUpMenuBar
{

  /** sequence store list */
  public static SequenceList seqList;
  /** local and remote file manager window */
  public static LocalAndRemoteFileTreeFrame localAndRemoteTree = null;
  /** local and remote file manager menu item */
  private JMenuItem showLocalRemoteFile;
  /** show saved results menu item */
  private JMenuItem fileMenuShowres;
  /** server settings */
  private ServerSetup ss = null;
  /** advanced options */
  private AdvancedOptions ao;
  /** favorites menu */
  private JMenu favMenu;

  /**
  *
  * @param mysettings	jemboss properties
  * @param f		frame
  * @param withSoap	true if in client-server mode
  *
  */
  public SetUpMenuBar(final JembossParams mysettings, final JFrame f,
                      final boolean withSoap)
  {

    // cursors to show when busy
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

    JMenuBar menuPanel = new JMenuBar();
    new BoxLayout(menuPanel,BoxLayout.X_AXIS);
    menuPanel.add(Box.createRigidArea(new Dimension(5,24))); 

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

    fileMenuShowres = new JMenuItem("Saved Results");
    fileMenuShowres.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        f.setCursor(cbusy);
        new ShowSavedResults(mysettings,f);
        f.setCursor(cdone);
      }
    });
    fileMenu.add(fileMenuShowres);
 
    if(withSoap)
    {
      showLocalRemoteFile = new JMenuItem("Local and Remote Files");
      showLocalRemoteFile.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          f.setCursor(cbusy);
     
          if(localAndRemoteTree == null)
          {
            try
            {
              localAndRemoteTree = new LocalAndRemoteFileTreeFrame(mysettings);
              Dimension d = f.getToolkit().getScreenSize();
              int locY = (int)(d.getHeight()-localAndRemoteTree.getHeight())/2;
              int wid1 = (int)localAndRemoteTree.getPreferredSize().getWidth();
              int wid2 = f.getWidth();
              wid1 = (int)d.getWidth()-wid1;
              if(wid2 < wid1)
                wid1 = wid2;     
              localAndRemoteTree.setLocation(wid1,locY);
              localAndRemoteTree.setVisible(true);
            }
            catch(JembossSoapException jse)
            {
              localAndRemoteTree = null;
              AuthPopup ap = new AuthPopup(mysettings,f); 
              ap.setBottomPanel();
              ap.setSize(380,170);
              ap.pack();
              ap.setVisible(true);
            }
          }
          else
            localAndRemoteTree.setVisible(true);
          f.setCursor(cdone);
        }
      });
      fileMenu.add(showLocalRemoteFile);
      fileMenu.addSeparator();
    }

//  final AdvancedOptions ao = new AdvancedOptions(mysettings);
    ao = new AdvancedOptions(mysettings);
    JMenuItem fileMenuExit = new JMenuItem("Exit");
    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        exitJemboss();
//      if(ao.isSaveUserHomeSelected())
//        ao.userHomeSave();

//      if(seqList.isStoreSequenceList())  //create a SequenceList file
//        saveSequenceList();

//      deleteTmp(new File(cwd), ".jembosstmp");
//      System.exit(0);
      }
    });
    fileMenu.add(fileMenuExit);
    menuPanel.add(fileMenu);

    JMenu prefsMenu = new JMenu("Preferences");
    prefsMenu.setMnemonic(KeyEvent.VK_P);

    JMenuItem showAdvOpt = new JMenuItem("Advanced Options");
    showAdvOpt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        Object[] options = { "APPLY", "CANCEL" };

        int select = JOptionPane.showOptionDialog(null, 
                    ao,"Advanced Options",
                    JOptionPane.YES_NO_CANCEL_OPTION,
                    JOptionPane.PLAIN_MESSAGE,
                    null,
                    options,
                    options[0]);

        if(select == 0)
        {
          if(!ao.getHomeDirectory().equals(mysettings.getUserHome()))
            mysettings.setUserHome(ao.getHomeDirectory());

          if(!withSoap && !ao.getResultsDirectory().equals(mysettings.getResultsHome()))
            mysettings.setResultsHome(ao.getResultsDirectory()); 
        }
//      JOptionPane jao = new JOptionPane();
//      jao.showMessageDialog(f,ao,"Advanced Options",
//                            JOptionPane.PLAIN_MESSAGE);
      }
    });
    prefsMenu.add(showAdvOpt);
    prefsMenu.addSeparator();

    JMenuItem serverSettings = new JMenuItem("Settings");
    serverSettings.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(ss == null)
          ss = new ServerSetup(mysettings,withSoap);
        int sso = JOptionPane.showConfirmDialog(f,ss,"Jemboss Settings",
                             JOptionPane.OK_CANCEL_OPTION,
                             JOptionPane.PLAIN_MESSAGE,null);
        if(sso == JOptionPane.OK_OPTION && withSoap)
          ss.setNewSettings();
      }
    });
    prefsMenu.add(serverSettings);
    menuPanel.add(prefsMenu);
    
    JMenu toolMenu = new JMenu("Tools");
    toolMenu.setMnemonic(KeyEvent.VK_T);

    JMenuItem toolAlignJFrame = new JMenuItem("Multiple Sequence Editor - Jemboss");
    toolAlignJFrame.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        org.emboss.jemboss.editor.AlignJFrame ajFrame =
                   new org.emboss.jemboss.editor.AlignJFrame();
        ajFrame.setVisible(true);
      }
    });
    toolMenu.add(toolAlignJFrame);

    JMenuItem toolJalview  = new JMenuItem("Multiple Sequence Editor - Jalview");
    toolJalview.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new LaunchJalView();
      }
    });
    toolMenu.add(toolJalview);

    JMenuItem toolDNAEd = new JMenuItem("Jemboss DNA Editor");
    toolDNAEd.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        Wizard wiz = new Wizard(null);
        DNADraw dna = wiz.getDNADraw();

        JFrame f = new JFrame("DNA Viewer");
        Dimension d = f.getToolkit().getScreenSize();

        Vector minTicks = new Vector();
        Vector majTicks = new Vector();
        Vector block = new Vector();
        Vector restrictionEnzyme = new Vector();

        if(dna == null)
          dna = new DNADraw(minTicks,majTicks,block,
                            restrictionEnzyme);
        DNADraw.jsp = new JScrollPane(dna);
        DNADraw.jsp.getViewport().setBackground(Color.white);
        dna.setCloseAndDispose(true,f);

        f.getContentPane().add(DNADraw.jsp);
        f.setJMenuBar(dna.createMenuBar());

        f.pack();
        f.setLocation(((int)d.getWidth()-f.getWidth())/4,
                  ((int)d.getHeight()-f.getHeight())/2);

        f.setVisible(true);
      }
    });
    toolMenu.add(toolDNAEd);

    toolMenu.addSeparator();

    JMenuItem toolWorkList = new JMenuItem("Sequence List");
    seqList = new SequenceList(withSoap,mysettings);
    toolWorkList.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        seqList.setVisible(true); 
      }
    });
    toolMenu.add(toolWorkList);
    menuPanel.add(toolMenu);

    favMenu = new JMenu("Favourites");
    menuPanel.add(favMenu);

    JMenu helpMenu = new JMenu("Help");
    helpMenu.setMnemonic(KeyEvent.VK_H);

    JMenuItem helpMenuAboutJemboss = new JMenuItem("About Jemboss");
    helpMenuAboutJemboss.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        ClassLoader cl = this.getClass().getClassLoader();
        try
        {
          URL inURL = cl.getResource("resources/readme.html");
          new Browser(inURL,"resources/readme.html");
        } 
        catch (Exception ex)
        {
          JOptionPane.showMessageDialog(null,
                              "About Jemboss Guide not found!",
                              "Error", JOptionPane.ERROR_MESSAGE);
        }
      }
    });
    helpMenu.add(helpMenuAboutJemboss);

    JMenuItem helpMenuAbout = new JMenuItem("Version");
    helpMenuAbout.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      { 
        ClassLoader cl = this.getClass().getClassLoader();
        String fc = "";
        try
        {
          String line;

          InputStream in = cl.getResourceAsStream("resources/version");
          BufferedReader reader = new BufferedReader(new InputStreamReader(in));
          while((line = reader.readLine()) != null)
            fc = fc.concat(line);
        }
        catch (Exception ex)
        {
          System.out.println("Didn't find resources/" +
                             "version");
        }
        JOptionPane.showMessageDialog(f, fc +
                  " by the EMBOSS team");
      }
    });
    helpMenu.add(helpMenuAbout);

    menuPanel.add(helpMenu);
    menuPanel.add(Box.createHorizontalGlue());

    f.setJMenuBar(menuPanel);

   }

   /**
   *
   * Get the favorites JMenu
   * @return favorites JMenu
   *
   */
   public JMenu getFavoriteJMenu()
   {
     return favMenu;
   }

   /**
   *
   * Enable/disable file manager menu item
   * @param b	true to enable file manager menu item
   *
   */
   public void setEnableFileManagers(boolean b)
   {
     showLocalRemoteFile.setEnabled(b);
   }

   /**
   *
   * Enable/disable show results menu item
   * @param b   true to enable show results menu item
   *
   */
   public void setEnableShowResults(boolean b)
   {
     fileMenuShowres.setEnabled(b);
   }

   /**
   *
   *  Get local file manager 
   *  @return 	local file manager
   *
   */
   public static DragTree getLocalDragTree()
   {
     if(localAndRemoteTree == null)
       return null;

     return localAndRemoteTree.getLocalDragTree();
   }

  /**
  *
  *  Delete temporary files
  *  @param suffix	suffix of files to delete
  *
  */
  public void deleteTmp(File cwd, final String suffix) 
  {
    String tmpFiles[] = cwd.list(new FilenameFilter()
    {
      public boolean accept(File cwd, String name)
      {
        return name.endsWith(suffix);
      };
    });

    for(int h =0;h<tmpFiles.length;h++)
    {
      File tf = new File(tmpFiles[h]);
      tf.delete();
    }
  }

  /**
  *
  * Write out a tab delimeted file containing the
  * favorite menu items.
  *
  */
  private void writeFavorites()
  {
    File fseq = new File(System.getProperty("user.home")
                + System.getProperty("file.separator")
                + ".jembossFavorites");
    try
    {
      PrintWriter fout = new PrintWriter(new FileWriter(fseq));
      int nmenu = favMenu.getItemCount();
      for(int i=0;i<nmenu;i++)
      {
        JMenuItem favItem = favMenu.getItem(i);
             
        if(favItem == null)  // JSepartor
        {
          fout.println();
          continue;
        }

        fout.println(favItem.getText()+"\t"+
                     favItem.getActionCommand());
      }
      fout.close();
    }
    catch (IOException ioe){ }
    catch (NullPointerException npe) {}
                                                                                
  }

  public void exitJemboss()
  {
    writeFavorites();
    String fs = new String(System.getProperty("file.separator"));
    String cwd = new String(System.getProperty("user.dir") + fs);
    if(ao.isSaveUserHomeSelected())
      ao.userHomeSave();

    if(seqList.isStoreSequenceList())  //create a SequenceList file
      saveSequenceList();

    deleteTmp(new File(cwd),".jembosstmp");
    System.exit(0);
  }

  /**
  *
  * Save the sequence list for a future session.
  *
  */
  public static void saveSequenceList()
  {
    File fseq = new File(System.getProperty("user.home")
                + System.getProperty("file.separator")
                + ".jembossSeqList");
    try
    {
      PrintWriter fout = new PrintWriter(new FileWriter(fseq));

      for(int i=0;i<seqList.getRowCount();i++)
      {
        SequenceData seqData = seqList.getSequenceData(i);
        String sbeg = seqData.s_beg;
        if(sbeg.equals(""))
          sbeg = "-";
        String send = seqData.s_end;
        if(send.equals(""))
          send = "-";

        if(!seqData.s_name.equals(""))
          fout.println(seqData.s_name + " " +
                     sbeg + " " + send + " " +
                     seqData.s_listFile.toString()+ " " +
                     seqData.s_default.toString() + " " +
                     seqData.s_remote.toString() );
      }
      fout.close();
    }
    catch (IOException ioe){}

  }

}

