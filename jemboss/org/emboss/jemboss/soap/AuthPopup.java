/****************************************************************
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  Based on EmbreoAuthPopup
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.filetree.LocalAndRemoteFileTreeFrame;

/**
*
* Display a popup asking the user to enter a username 
* and password
*
*/
public class AuthPopup extends JFrame
{

  /** splash panel */
  private JPanel splashp;
  /** progress monitor */
  private int iprogress = 0;
  /** max value for the progress monitor */
  private int iprogressmax = 0;
  /** jemboss properties */
  private JembossParams mysettings;
  /** progress bar */
  private JProgressBar progressBar;
  /** progress label */
  private JLabel progressLabel;
  /** OK button */
  private JButton okButton;
  /** username field */
  private JTextField ufield;
  /** passwd field */
  private JPasswordField pfield;
  /** true if window is to be hidden */
  private boolean exitOnDone = false;
  /** prompt for login panel */
  private JPanel promptPanel;

  /**
  *
  * @param mysettings jemboss properties
  * @param f          Parent frame the popup is associated with
  *
  */
  public AuthPopup(final JembossParams mysettings, JFrame f) 
  {

    if(f != null)
    {
      String text = "";
      if (mysettings.getUseAuth() == true) 
      {
        if (mysettings.getServiceUserName() == null) 
	  text = "You need to supply a username and password\n"
	       + "before running an application.";
        else 
          text = "Login to server failed\n"
	       + "Please check your login details.";
      } 
      else
      {
        text = "The server wants a username and password,\n"
	     + "but we weren't expecting to need to.\n"
             + "Please supply the correct login details.";
      }
      JOptionPane.showMessageDialog(f,text,"Authentication failed",
                                    JOptionPane.ERROR_MESSAGE);
    }

// setup a Jemboss login box
    JPanel logoPanel = new JPanel(new BorderLayout());
    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon ii = new ImageIcon(
               cl.getResource("images/Jemboss_logo_greyback.gif"));
    logoPanel.add(new JLabel(ii),BorderLayout.WEST);

    splashp = (JPanel)getContentPane();
    splashp.setLayout(new BorderLayout());
    splashp.add(logoPanel, BorderLayout.NORTH);

    //if required, a login prompt
    if (mysettings.getUseAuth()) 
    {
      setTitle("Login");
      promptPanel = new JPanel(new BorderLayout());
      JPanel loginPanel = new JPanel();
      loginPanel.setLayout(new GridLayout(2,2));
      
      ufield = new JTextField(16);
      if (mysettings.getServiceUserName() != null) 
	ufield.setText(mysettings.getServiceUserName());
      
      pfield = new JPasswordField(16);
      //final JTextField xfield = new JTextField(16);

      JLabel ulab = new JLabel(" Username:", SwingConstants.LEFT);
      JLabel plab = new JLabel(" Password:", SwingConstants.LEFT);
      //add labels etc
      loginPanel.add(ulab);
      loginPanel.add(ufield);
      loginPanel.add(plab);
      loginPanel.add(pfield);

      promptPanel.add(loginPanel, BorderLayout.CENTER);
      setSize(380,170);

      // all added, display the frame
      setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
    }
    else
    {
      setTitle("Jemboss Launch");
      exitOnDone = true;
    }
    setLocation(1,5);
    this.mysettings = mysettings;
  }

  /**
  *
  * Called on startup to display the login and progress bar.
  * @param mysettings	jemboss properties
  * @param iprogressmax	max value for progress monitor
  *
  */
  public AuthPopup(final JembossParams mysettings, int iprogressmax)
  {
     this(mysettings,null);
     this.mysettings = mysettings;
     this.iprogressmax = iprogressmax;

     //progress meter at startup
     if (iprogressmax > 0)
     {
       JPanel progressPanel = new JPanel();
       progressPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
       progressBar = new JProgressBar(0,iprogressmax);
       progressBar.setValue(0);
       progressLabel = new JLabel("Starting up.");
       progressPanel.add(progressBar);
       progressPanel.add(progressLabel);
       splashp.add(progressPanel,BorderLayout.SOUTH);
     }

    // add a border to the main pane to make is stand out
    splashp.setBorder(BorderFactory.createMatteBorder(2,2,2,2,Color.black));

    setSize(380,200);

    // all added, display the frame
    setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
  }

  /**
  *
  * Buttons across the bottom
  *
  */
  public void addBottomPanel() 
  {
    setTitle("File Manager Login");
    final JPanel buttonPanel = 
         new JPanel(new FlowLayout(FlowLayout.RIGHT));
    JButton exitButton = new JButton("Exit");
    okButton = new JButton("OK");

    exitButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        System.exit(0);
      }
    });

    final JFrame fthis = this;
    okButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        mysettings.setServiceUserName(ufield.getText());
        mysettings.setServicePasswd(pfield.getPassword());
        exitOnDone = true;

        if (iprogressmax > 0)
        {
          remove(promptPanel);
          pack();
        }
        else
          setVisible(false);

        try
        {
          LocalAndRemoteFileTreeFrame treeFrame =
                  new LocalAndRemoteFileTreeFrame(mysettings);
          treeFrame.setExit();

          Dimension d = treeFrame.getToolkit().getScreenSize();
          treeFrame.setLocation(0,((int)d.getHeight()-treeFrame.getHeight())/2);
          treeFrame.setVisible(true);
        }
        catch(JembossSoapException jse)
        {
          if (iprogressmax > 0)
          {
            promptPanel.add(buttonPanel, BorderLayout.SOUTH);
            pack();
          }
          setVisible(true);
        }
      }
    });

    buttonPanel.add(exitButton);
    buttonPanel.add(okButton);
    promptPanel.add(buttonPanel, BorderLayout.SOUTH);
    splashp.add(promptPanel);

    //close login box on carriage return in passwd field
    pfield.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        mysettings.setServiceUserName(ufield.getText());
        mysettings.setServicePasswd(pfield.getPassword());
        exitOnDone = true;
        fthis.setVisible(false);
        try
        {
          LocalAndRemoteFileTreeFrame treeFrame =
                  new LocalAndRemoteFileTreeFrame(mysettings);
          treeFrame.setExit();
          Dimension d = treeFrame.getToolkit().getScreenSize();
          treeFrame.setLocation(0,((int)d.getHeight()-treeFrame.getHeight())/2);
          treeFrame.setVisible(true);
        }
        catch(JembossSoapException jse)
        {
          fthis.setVisible(true);
        }
      }
    });

  }

  
  /**
  *
  * Buttons across the bottom
  *
  */
  public void setBottomPanel()
  {
    JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
    JButton exitButton = new JButton("Exit");
    JButton okButton = new JButton("OK");

    exitButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        System.exit(0);
      }
    });

    okButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        mysettings.setServiceUserName(ufield.getText());
        mysettings.setServicePasswd(pfield.getPassword());
        exitOnDone = true;

        if(iprogressmax > 0 &&
           iprogress != iprogressmax)
        {
          remove(promptPanel);
          pack();
        }
        else
          dispose();
      }
    });
    buttonPanel.add(exitButton);
    buttonPanel.add(okButton);
    if (promptPanel!=null){
      promptPanel.add(buttonPanel, BorderLayout.SOUTH);
      splashp.add(promptPanel);
    }
//close login box on carriage return in passwd field
    pfield.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        mysettings.setServiceUserName(ufield.getText());
        mysettings.setServicePasswd(pfield.getPassword());
        exitOnDone = true;
        if(iprogressmax > 0 &&
           iprogress != iprogressmax)
        {
          remove(promptPanel);
          pack();
        }
        else
          dispose();
      }
    });

  }


//public JButton getOKButton()
//{
//  return okButton;
//}


  /**
  *
  * Update the progress bar and label
  * @param s	string for progress label
  *
  */
  public void doneSomething(String s)
  {
    if (iprogressmax > 0)
    {
      if (iprogress < iprogressmax)
      {
        iprogress++;
        progressBar.setValue(iprogress);
      }
      progressLabel.setText(s);
      if (iprogress == iprogressmax)
      {
        progressLabel.setText("Startup complete.");
        if (exitOnDone)
          setVisible(false);
      }
    }
  }

  /**
  *
  * Finish the progress bar and label
  * @param s    string for progress label
  *
  */
  public void doneEverything(String s)
  {
    if (iprogressmax > 0)
    {
      progressBar.setValue(iprogressmax);
      progressLabel.setText(s);
      if (exitOnDone)
        setVisible(false);
    }
  }


  public JFrame getSplashFrame()
  {
    return this;
  }

}
