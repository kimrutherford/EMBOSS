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
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import org.emboss.jemboss.gui.form.Separator;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.Jemboss;
import org.emboss.jemboss.gui.filetree.FileSave;

/**
*
* Jemboss advanced options and settings. 
* <br>Job manager update times.
* <br>Calculate parameer dependencies.
* <br>User home root directory.
* <br>Save user working directory.
*
*/
public class AdvancedOptions extends JPanel
{

  /** use JNI to calculate parameter dependencies */
  public static JCheckBox prefjni;
  /** shade or remove unused parameters */
  public static JCheckBox prefShadeGUI;
  /** job manager update times */
  public static JComboBox jobMgr;
  /** save user home directory */
  private JCheckBox saveUserHome;
  /** user home directory */
  private JTextField userHome = new JTextField();
  /** user home directory */
  private JTextField resultsHome = new JTextField();

  /** current working directory */
  private String cwd;
  /** times for job manager updates */
  private String time[] = new String[6];

  /**
  *
  * @param mysettings	Jemboss settings
  *
  */
  public AdvancedOptions(final JembossParams mysettings)
  {
    super();

    cwd = mysettings.getUserHome();
    time[0] = "5 s";
    time[1] = "10 s";
    time[2] = "15 s";
    time[3] = "20 s";
    time[4] = "30 s";
    time[5] = "60 s";

    Box bdown =  Box.createVerticalBox();
    Box bleft =  Box.createHorizontalBox();

//shade or remove unused parameters
    prefShadeGUI = new JCheckBox("Shade unused parameters");
    prefShadeGUI.setSelected(true);
    bleft.add(prefShadeGUI);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    bdown.add(Box.createVerticalStrut(4));

//use JNI to calculate parameter dependencies
    prefjni = new JCheckBox("Calculate dependencies (JNI)");
    prefjni.setSelected(true);
    bleft =  Box.createHorizontalBox();
    bleft.add(prefjni);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    bdown.add(Box.createVerticalStrut(5));

//frequency of job manager updates
//  if(Jemboss.withSoap)
//  {
      jobMgr = new JComboBox(time);
      jobMgr.setSelectedIndex(2);
      int hgt = (new Double(jobMgr.getPreferredSize().getHeight())).intValue();
      jobMgr.setPreferredSize(new Dimension(70,hgt));
      jobMgr.setMaximumSize(new Dimension(70,hgt));
      bleft =  Box.createHorizontalBox();
      bleft.add(jobMgr);
      JLabel ljobMgr = new JLabel(" Job Manager update frequency");
      ljobMgr.setForeground(Color.black);
      bleft.add(ljobMgr);
      bleft.add(Box.createHorizontalGlue());
      bdown.add(bleft);
//  }

    bdown.add(Box.createVerticalStrut(5));
    bdown.add(new Separator(new Dimension(400,10)));
    bdown.add(Box.createVerticalStrut(5));

    if(!Jemboss.withSoap)
    {
      bleft =  Box.createHorizontalBox();
      JLabel lresults = new JLabel("Results Directory:");
      lresults.setForeground(Color.black);
      bleft.add(lresults);
      bleft.add(Box.createHorizontalGlue());
      bdown.add(bleft);

      resultsHome.setText(mysettings.getResultsHome());
      bleft =  Box.createHorizontalBox();
      bleft.add(resultsHome);
      bdown.add(bleft);
    }

//set users home root directory
    bleft =  Box.createHorizontalBox();         
    JLabel lhome = new JLabel("Local Home Directory:");
    lhome.setForeground(Color.black);
    bleft.add(lhome);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);

    userHome.setText(cwd);
    bleft =  Box.createHorizontalBox();
    bleft.add(userHome);
    bdown.add(bleft);
    JButton jroot = new JButton("Set");
    bleft =  Box.createHorizontalBox();
    bleft.add(jroot);
    JButton jreset = new JButton("Reset");
    bleft.add(jreset);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    
    jroot.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        String cwd = userHome.getText();
        File f = new File(cwd);
        if(f.exists() && f.canRead())
        {
          mysettings.setUserHome(cwd);
          org.emboss.jemboss.Jemboss.tree.newRoot(cwd);
          if(SetUpMenuBar.localAndRemoteTree != null)
            SetUpMenuBar.localAndRemoteTree.getLocalDragTree().newRoot(cwd);

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
    });

//reset button
    jreset.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        cwd = System.getProperty("user.home");
        mysettings.setUserHome(cwd);
        org.emboss.jemboss.Jemboss.tree.newRoot(cwd);
        if(SetUpMenuBar.localAndRemoteTree != null)
          SetUpMenuBar.localAndRemoteTree.getLocalDragTree().newRoot(cwd);
        userHome.setText(cwd);
      }
    });

//save user work dir checkbox
    saveUserHome = new JCheckBox("Save between Jemboss sessions");
    saveUserHome.setSelected(false);
    bleft =  Box.createHorizontalBox();
    bleft.add(saveUserHome);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);

    bdown.add(Box.createVerticalStrut(5));
    bdown.add(new Separator(new Dimension(400,10)));
    bdown.add(Box.createVerticalStrut(5));

    this.add(bdown);
  }


  /**
  *
  * Get the user home/working directory
  * @return     user home directory
  *
  */
  public String getResultsDirectory()
  {
    return resultsHome.getText();
  }

  /**
  *
  * Get the user home/working directory
  * @return 	user home directory
  *
  */
  public String getHomeDirectory()
  {
    return userHome.getText();
  }

  /**
  *
  * @return	if true save the user home/working directory
  *
  */
  public boolean isSaveUserHomeSelected()
  {
    return saveUserHome.isSelected();
  }

  /**
  *
  * Save the user home/working directory to jemboss.properties
  *
  */
  public void userHomeSave()
  {
    String uhome = System.getProperty("user.home");
    String fs = System.getProperty("file.separator");
    String jemProp = uhome+fs+"jemboss.properties";
    File fjemProp = new File(jemProp);
 
    String uHome = "user.home="+getHomeDirectory();
    if(!Jemboss.withSoap)
      uHome = uHome + "\nresults.home="+getResultsDirectory();

    uHome = addEscapeChars(uHome);

    if(fjemProp.exists())       // re-write jemboss.properties
      rewriteProperties(jemProp,uHome);
    else                        // write new jemboss.properties
    {
      FileSave fsave = new FileSave(fjemProp);
      fsave.fileSaving(uHome);
    }
  }

  /**
  *
  * Re-write jemboss.properties when there is an existing jemboss,properties
  * and insert/update the user.home property
  * @param jemProp	jemboss.properties file	
  * @param uHome	user home directory
  *
  */
  public void rewriteProperties(String jemProp, String uHome)
  {
     File file_txt = new File(jemProp);
     File file_tmp = new File(jemProp + ".tmp");
     try 
     {
       BufferedReader bufferedreader = new BufferedReader(new FileReader(file_txt));
       BufferedWriter bufferedwriter = new BufferedWriter(new FileWriter(file_tmp));
       String line;
       while ((line = bufferedreader.readLine()) != null) 
       {
         if(line.startsWith("user.home"))
           line = uHome;

         bufferedwriter.write(line);
         bufferedwriter.newLine();
       }
       bufferedreader.close();
       bufferedwriter.close();
       file_txt.delete();
       file_tmp.renameTo(file_txt);
     } 
     catch (FileNotFoundException filenotfoundexception)
     {
       System.err.println("jemboss.properties read error");
     } 
     catch (IOException e) 
     {
       System.err.println("jemboss.properties i/o error");
     }

  }

  /**
  *
  * Add in escape chars (for windows) to the backslash chars
  * @param l	string to insert escape characters to
  *
  */
  private String addEscapeChars(String l)
  {
    int n = l.indexOf("\\");

    while( n > -1) 
    {
      l = l.substring(0,n)+"\\"+l.substring(n,l.length());
      n = l.indexOf("\\",n+2);
    }
    return l;
  }

}


