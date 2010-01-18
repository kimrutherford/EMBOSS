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

import jalview.appletgui.AlignFrame;
import jalview.appletgui.PaintRefresher;
import jalview.datamodel.Alignment;
import jalview.io.AppletFormatAdapter;
import jalview.io.IdentifyFile;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;

import javax.swing.*;

import org.emboss.jemboss.gui.form.TextFieldSink;

/**
*
* Launches JalviewLite (see jalview.org)
*
*/
public class LaunchJalView extends JFrame
{

  public LaunchJalView()
  {
    super("Launch JalviewLite ");

    final TextFieldSink tfs = new TextFieldSink();
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

    Box bacross = Box.createHorizontalBox();
    final Box bdown = Box.createVerticalBox();
    bdown.add(Box.createVerticalStrut(5));

    JLabel lname = new JLabel("Multiple Sequence Filename:");
    lname.setFont(org.emboss.jemboss.gui.form.SectionPanel.labfont);
    lname.setForeground(Color.black);
    bdown.add(Box.createVerticalStrut(5));
    bacross.add(Box.createHorizontalStrut(5));
    bacross.add(lname);
    bacross.add(Box.createHorizontalStrut(100));
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
 
    bdown.add(Box.createVerticalStrut(1));

    bacross = Box.createHorizontalBox();
    bacross.add(Box.createHorizontalStrut(5));
    bacross.add(tfs);
    bdown.add(bacross);

    bdown.add(Box.createVerticalStrut(5));

    final JButton launch = new JButton("LAUNCH");
    bacross = Box.createHorizontalBox();
    bacross.add(Box.createHorizontalStrut(100));
    bacross.add(launch);
    bdown.add(bacross);

    tfs.setToolTipText("Enter full path of your input alignment" +
    		" or multiple sequence file," +
    		" or drag the file from local file manager to here.");
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

            String filename = tfs.getText();
            File fn = new File(filename);
            if(!fn.exists())
            {
              JOptionPane.showMessageDialog(fr,filename+" not found.\n"+
                                            "Note if this is a file on the server then\n"+
                                            "drag it to the local file manager first.",
                                            "File Not Found",
                                            JOptionPane.ERROR_MESSAGE);
              setCursor(cdone);
              dispose();
              return null;
            }           

            try
            {
              callJalview(filename, filename);
              setCursor(cdone);
              dispose();
            }
            catch(Exception npe)
            {
              setCursor(cdone);
              JOptionPane.showMessageDialog(fr,
                       "Check the input file entered.",
                       "Error", JOptionPane.ERROR_MESSAGE);
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
  
  
  public static void callJalview(String filename, String title) throws IOException
  {
      { // this block is a workaround to stop jalview calling System.exit()
          final Label dummylabel = new Label("dummy");
          final String dummyname = "dummy";
          PaintRefresher.Register(dummylabel, dummyname);
      }

      Alignment al = null;
      String format = new IdentifyFile().Identify(filename,
              AppletFormatAdapter.FILE);
      al = new AppletFormatAdapter().readFile(filename,
              AppletFormatAdapter.FILE, format);
      if (al.getHeight() > 0)
      {
          AlignFrame af = new AlignFrame(al, null, title, false);
          af.setVisible(true);
      }
  }

}

