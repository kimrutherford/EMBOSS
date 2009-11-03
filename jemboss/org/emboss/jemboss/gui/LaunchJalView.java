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
import org.emboss.jemboss.gui.form.TextFieldSink;
import jalview.AlignFrame;
//import jalview.MailProperties;
//import jalview.PIDColourScheme;

/**
*
* Launch Jalview (M.Clamp )
*
*/
public class LaunchJalView extends JFrame
{

  public LaunchJalView()
  {
    super("Jalview ");

    final TextFieldSink tfs = new TextFieldSink();
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

    Box bacross = Box.createHorizontalBox();
    final Box bdown = Box.createVerticalBox();
    bdown.add(Box.createVerticalStrut(5));

    JLabel lname = new JLabel("Multiple Sequence Filename");
    lname.setFont(org.emboss.jemboss.gui.form.SectionPanel.labfont);
    lname.setForeground(Color.black);
    bdown.add(Box.createVerticalStrut(1));
    bacross.add(Box.createHorizontalStrut(1));
    bacross.add(lname);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
 
    bdown.add(Box.createVerticalStrut(1));

    bacross = Box.createHorizontalBox();
    bacross.add(Box.createHorizontalStrut(1));
    bacross.add(tfs);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bdown.add(Box.createVerticalStrut(5));
    
    lname = new JLabel("File Format");
    lname.setFont(org.emboss.jemboss.gui.form.SectionPanel.labfont);
    lname.setForeground(Color.black);
    bacross = Box.createHorizontalBox();
    bacross.add(Box.createHorizontalStrut(1));
    bacross.add(lname);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bdown.add(Box.createVerticalStrut(1));
    String sformats[] = {"MSF", "FASTA", "CLUSTAL", "PIR"};
    final JComboBox format = new JComboBox(sformats);
    bdown.add(format);
    bdown.add(Box.createVerticalStrut(5));

    final JTextField mailServer = new JTextField();
    mailServer.setText("mercury.hgmp.mrc.ac.uk");
    bacross =  Box.createHorizontalBox();
    bacross.add(Box.createHorizontalStrut(1));
    bacross.add(mailServer);
    JLabel lmail = new JLabel(" Mail Server ");
    lmail.setForeground(Color.black);
    bacross.add(lmail);
    bdown.add(bacross);
    bdown.add(Box.createVerticalStrut(5));

    final JButton launch = new JButton("LAUNCH");
    bacross = Box.createHorizontalBox();
    bacross.add(Box.createHorizontalStrut(1));
    bacross.add(launch);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    final JFrame fr = this;
    launch.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
      
        SwingWorker launchworker = new SwingWorker()
        {

          public Object construct()
          {
            setCursor(cbusy);

            String name = tfs.getText();
            File fn = new File(name);
            if(!fn.exists())
            {
              JOptionPane.showMessageDialog(fr,name+" not found.\n"+
                                            "Note if this is a file on the server then\n"+
                                            "drag it to the local file manager first.",
                                            "File Not Found",
                                            JOptionPane.ERROR_MESSAGE);
              setCursor(cdone);
              setVisible(false);
              return null;
            }           


            String args[] = { 
              name,                                //alignment file
              "File",
              (String)format.getSelectedItem(),    //format 
              "-mail",
              mailServer.getText()                 //mail server
            };
            try
            {
              AlignFrame af = AlignFrame.parseArgs(args);
              af.setSize(700,500);
              setCursor(cdone);
              setVisible(false);
              af.show();
            }
            catch(Exception npe)
            {
              setCursor(cdone);
              JOptionPane.showMessageDialog(fr,
                       "Check the sequence entered \n" + 
                       "and the format chosen.",
                       "Error Message", JOptionPane.ERROR_MESSAGE);
            }
            return null;
          }
        };
        launchworker.start();
      }
    });

    JPanel jp = (JPanel)getContentPane();
    jp.setLayout(new BorderLayout());  
    jp.add(bdown, BorderLayout.CENTER);

    pack();
    setLocation( (int)((getWidth())/4),
                 (int)((getHeight())/4) );

    setVisible(true);      
  }
}

