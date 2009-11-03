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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Properties;
import java.util.prefs.Preferences;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import org.emboss.jemboss.Jemboss;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.filetree.LocalAndRemoteFileTreeFrame;
import org.emboss.jemboss.gui.form.Separator;

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
  public static JCheckBox prefJNI;
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
  private String time[] = {"5 s", "10 s", "15 s", "20 s", "30 s", "60 s"};
  
  Preferences preferences;
  private static final String SHADE_GUI = "SHADE_GUI";
  private static final String CALCULATE_JNI_DEPENDENCIES = "CALCULATE_JNI_DEPENDENCIES";
  private static final String SAVE_DIRECTORY_MODIFICATIONS = "SAVE_DIRECTORY_MODIFICATIONS";
  private static final String JOBMANAGER_FREQUENCY = "JOBMANAGER_FREQUENCY";
  JButton setUserHome;
  
  /** flag to remember if any of the User Home or Results Home directories has changed */
  boolean usersChangedDirectory = false;
  
  /**
  *
  * @param mysettings	Jemboss settings
  *
  */
  public AdvancedOptions(final JembossParams mysettings)
  {
    super();
    preferences = Preferences.userNodeForPackage(AdvancedOptions.class);

    cwd = mysettings.getUserHome();

    Box bdown =  Box.createVerticalBox();
    Box bleft =  Box.createHorizontalBox();

//shade or remove unused parameters
    prefShadeGUI = new JCheckBox("Shade unused parameters");
    prefShadeGUI.setSelected(preferences.getBoolean(SHADE_GUI, true));
    prefShadeGUI.addItemListener(new ItemListener(){
        public void itemStateChanged(ItemEvent e) {
            preferences.putBoolean(SHADE_GUI,
                    prefShadeGUI.isSelected());
        }
    });
    bleft.add(prefShadeGUI);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    bdown.add(Box.createVerticalStrut(4));

//use JNI to calculate parameter dependencies
    prefJNI = new JCheckBox("Calculate dependencies (JNI)");
    prefJNI.setSelected(preferences.getBoolean(CALCULATE_JNI_DEPENDENCIES, true));
    prefJNI.addItemListener(new ItemListener(){
        public void itemStateChanged(ItemEvent e) {
            preferences.putBoolean(CALCULATE_JNI_DEPENDENCIES,
                    prefJNI.isSelected());
        }
    });    
    bleft =  Box.createHorizontalBox();
    bleft.add(prefJNI);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    bdown.add(Box.createVerticalStrut(5));

//frequency of job manager updates
//  if(Jemboss.withSoap)
//  {
      jobMgr = new JComboBox(time);
      
      int hgt = (new Double(jobMgr.getPreferredSize().getHeight())).intValue();
      jobMgr.setPreferredSize(new Dimension(70,hgt));
      jobMgr.setMaximumSize(new Dimension(70,hgt));
      jobMgr.setSelectedIndex(preferences.getInt(JOBMANAGER_FREQUENCY, 2));
      jobMgr.addItemListener(new ItemListener(){
          public void itemStateChanged(ItemEvent e) {
              preferences.putInt(JOBMANAGER_FREQUENCY,
                      jobMgr.getSelectedIndex());
          }
      });    

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
      JLabel lresults = new JLabel("Local Results Directory:");
      lresults.setToolTipText("Root directory where EMBOSS job results are stored" +
      		"\nKnown as Working Directory in the local file manager");
      lresults.setForeground(Color.black);
      bleft.add(lresults);
      bleft.add(Box.createHorizontalGlue());
      bdown.add(bleft);

      resultsHome.setText(mysettings.getResultsHome());
      bleft =  Box.createHorizontalBox();
      bleft.add(resultsHome);
      bdown.add(bleft);
      JButton setResultsHome = new JButton("Set");
      setResultsHome.setToolTipText("Updates Results Home directory " +
              "in local file manager\n" +
              "Results of new EMBOSS jobs are stored in the updated Results Home directory\n" +
              "To have your settings persist across Jemboss sessions don't forget to check\n" +
              "'Remember between Jemboss sessions' option"
              );
      setResultsHome.setEnabled(false);
      bleft =  Box.createHorizontalBox();
      bleft.add(setResultsHome);
      bleft.add(Box.createHorizontalGlue());
      bdown.add(bleft);
      resultsHome.getDocument().addDocumentListener(new MyDocumentListener(setResultsHome));
      setResultsHome.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          updateResultsHome(mysettings);
        }
      });
    }

//set users home root directory
    bleft =  Box.createHorizontalBox();         
    JLabel lhome = new JLabel("Local Home Directory:");
    lhome.setToolTipText("User Home directory in local file manager");
    lhome.setForeground(Color.black);
    bleft.add(lhome);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);

    userHome.setText(cwd);
    bleft =  Box.createHorizontalBox();
    bleft.add(userHome);
    bdown.add(bleft);
    setUserHome = new JButton("Set");
    setUserHome.setToolTipText("Updates User Home directory " +
    		"in local file manager\n" +
    		"To have your settings persist across Jemboss sessions don't forget to check\n" +
    		"'Remember between Jemboss sessions' option"
            );
    setUserHome.setEnabled(false);
    userHome.getDocument().addDocumentListener(new MyDocumentListener(setUserHome));
    bleft =  Box.createHorizontalBox();
    bleft.add(setUserHome);
    JButton jreset = new JButton("Reset");
    jreset.setToolTipText("Resets User Home directory to its system defaults");
    bleft.add(jreset);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    
    setUserHome.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        updateUserHome(mysettings);
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
          LocalAndRemoteFileTreeFrame.getLocalDragTree().newRoot(cwd);
        userHome.setText(cwd);
      }
    });

//save user/work(results) directory checkbox
    saveUserHome = new JCheckBox("Remember between Jemboss sessions");
    saveUserHome.setToolTipText("If this option is selected" +
    		"\nJemboss records above directory changes" +
    		"\nin the jemboss.properties file under your home directory");
    saveUserHome.setSelected(preferences.getBoolean(SAVE_DIRECTORY_MODIFICATIONS, true));
    saveUserHome.addItemListener(new ItemListener(){
        public void itemStateChanged(ItemEvent e) {
            preferences.putBoolean(SAVE_DIRECTORY_MODIFICATIONS,
                    saveUserHome.isSelected());
        }
    });
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
  * Save the user home/working directory to jemboss.properties
  *
  */
  public void userHomeSave()
  {
      
    if (!usersChangedDirectory)
        return;
      
    String uhome = System.getProperty("user.home");
    String fs = System.getProperty("file.separator");
    String jemProp = uhome+fs+"jemboss.properties";
    Properties p = new Properties();
    try {
        p.load(new FileInputStream(jemProp));
    } catch (FileNotFoundException e) {
        // we should ignore this error
    } catch (IOException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
    }
    
    try{
        p.put("user.home",getHomeDirectory());
        if(!Jemboss.withSoap)
            p.put("results.home",getResultsDirectory());
        p.store(new FileOutputStream(jemProp), "jemboss properties");
    } catch (IOException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
    }
  }
  
  
  protected class MyDocumentListener implements DocumentListener {
      private JButton b;
      public MyDocumentListener(JButton b){
          this.b = b;
      }
      public void insertUpdate(DocumentEvent e) {
          enableJButton(e);
      }
      public void removeUpdate(DocumentEvent e) {
          enableJButton(e);
      }
      public void changedUpdate(DocumentEvent e) {
          enableJButton(e);
      }
      private void enableJButton(DocumentEvent e) {
          b.setEnabled(true);
      }
  } 

  static JDialog dialog;
  
  public void showDiaolog(final JembossParams mysettings, boolean withSoap, JFrame f,
          int x, int y)
  {
      if (dialog == null) {
          dialog = new JDialog(f, "Advanced Options");

          JButton ok = new JButton("Apply");
          ok
          .setToolTipText("Apply your directory change(s) (if any) and closes this dialog");
          JButton cancel = new JButton("Cancel");
          cancel
          .setToolTipText("Closes this dialog without applying your directory change(s)");
          Object[] options = { ok, cancel};

          final JOptionPane optionPane = new JOptionPane(this,
                  JOptionPane.PLAIN_MESSAGE, JOptionPane.OK_CANCEL_OPTION,
                  null, options, options[0]);

          cancel.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent e) {
                  dialog.setVisible(false);
              }
          });

          ok.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent e) {
                  dialog.setVisible(false);
                  updateUserHome(mysettings);
                  updateResultsHome(mysettings);
              }
          });
          dialog.setLocation(x, y);
          dialog.setContentPane(optionPane);
          dialog.pack();
      }
      dialog.setVisible(true);
  }

  
  private void updateUserHome(JembossParams mysettings)
  {
      if (getHomeDirectory().equals(mysettings.getUserHome()))
          return;
      String userHome = getHomeDirectory();
      File f = new File(userHome);
      if (f.exists() && f.canRead()) {
          mysettings.setUserHome(userHome);
          usersChangedDirectory = true;
          Jemboss.tree.newRoot(userHome);
          if (SetUpMenuBar.localAndRemoteTree != null)
              LocalAndRemoteFileTreeFrame.getLocalDragTree().newRoot(userHome);

          if (!f.canWrite())
              JOptionPane.showMessageDialog(null,
                      "You cannot write to directory: "+userHome,
                      "Warning: Write", JOptionPane.WARNING_MESSAGE);
      } else
          JOptionPane.showMessageDialog(null, "No access to directory: "+userHome,
                  "Error: accessing User Home directory", JOptionPane.ERROR_MESSAGE);

  }

  private void updateResultsHome(JembossParams mysettings)
  {
      if (Jemboss.withSoap
              || getResultsDirectory().equals(mysettings.getResultsHome()))
          return;
      String resultsHome = getResultsDirectory();
      File f = new File(resultsHome);
      if (f.exists() && f.canRead()) {
          mysettings.setUserHome(resultsHome);
          usersChangedDirectory = true;
          Jemboss.tree.newRoot(resultsHome);
          if (SetUpMenuBar.localAndRemoteTree != null)
              LocalAndRemoteFileTreeFrame.getLocalDragTree().newRoot(resultsHome);

          if (!f.canWrite())
              JOptionPane.showMessageDialog(null,
                      "You cannot write to directory: "+resultsHome,
                      "Warning: Write", JOptionPane.WARNING_MESSAGE);
      } else
          JOptionPane.showMessageDialog(null, "No access to directory: "+resultsHome,
                  "Error: accessing Results Home directory", JOptionPane.ERROR_MESSAGE);

  }
}



