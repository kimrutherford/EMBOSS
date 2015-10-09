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
//import java.awt.event.KeyEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.swing.*;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

import org.apache.regexp.*;
import java.util.Hashtable;
import java.util.Enumeration;

//import org.emboss.grout.*;
import org.emboss.jemboss.graphics.*;
import org.emboss.jemboss.gui.filetree.FileEditorDisplay;
import org.emboss.jemboss.JembossParams;

/**
*
* Displays JTabbedPane of the contents of the Hashtable
*
*/
public class ShowResultSet extends JFrame
{

  /** menu bar */
  private ResultsMenuBar menuBar;
  /** tabbed pane */
  private JTabbedPane rtp;
  /** grout panel */
//  private GroutPanel grout = null;
  /** toolbar */
  private JToolBar toolbar = null;

  /**
  * 
  * @param reslist 	result list
  * @param project	the result data to display
  * @param mysettings	jemboss properties
  *
  */
  public ShowResultSet(Hashtable reslist, String project, 
                                JembossParams mysettings)
  {
    this(reslist,null,project,mysettings);
  }

  /**
  *
  * @param reslist      result list
  * @param inputFiles 	input files
  *
  */
  public ShowResultSet(Hashtable reslist, Hashtable inputFiles)
  {
    this(reslist,inputFiles,null,null);
  }

  /**
  *
  * @param reslist      result list
  * @param mysettings   jemboss properties
  *
  */
  public ShowResultSet(Hashtable reslist, JembossParams mysettings)
  {
    this(reslist,null,null,mysettings); 
  }

  /**
  *
  * @param reslist      result list
  * @param inputFiles   input files
  * @param mysettings   jemboss properties
  *
  */
  public ShowResultSet(Hashtable reslist, Hashtable inputFiles,
                       JembossParams mysettings)
  {
    this(reslist,inputFiles,null,mysettings);
  }

  /**
  *  
  * @param reslist	result list 
  * @param inputFiles	the input data to display
  * @param project      the result data to display
  * @param mysettings   jemboss properties
  *
  */
  public ShowResultSet(Hashtable reslist, Hashtable inputFiles, 
                       String project, JembossParams mysettings)
  {
    super("Saved Results - "+(project==null?"":project));
    rtp = new JTabbedPane();

    setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

    JScrollPane r1;
    
    String prjdir = null;
    
    if(project!=null)
        prjdir = mysettings.getResultsHome()+project;

    String stabs[] = addHashContentsToTab(reslist,rtp,prjdir,mysettings);
    if(inputFiles != null)
      addHashContentsToTab(inputFiles,rtp, prjdir,mysettings);

// now load png files into pane
    for(int i=0; i<stabs.length;i++)
    {
      if(stabs[i].endsWith(".dat"))
      {
        Graph2DPlot gp = new Graph2DPlot();
        r1 = new JScrollPane(gp);
        gp.setFileData(new String((byte [])reslist.get(stabs[i])),
                       stabs[i]);
        rtp.add(r1, i);
        rtp.setTitleAt(i,stabs[i]);
        setJMenuBar(gp.getMenuBar(false, this));
      }
      else
      {
        ImageIcon i1 = new ImageIcon((byte [])reslist.get(stabs[i]));
        ImageIconJPanel iiPanel = new ImageIconJPanel(i1);
        r1 = new JScrollPane(iiPanel);
        r1.getViewport().setBackground(Color.white);
        if(stabs[i] != null)
        {
          rtp.add(r1,i);
          rtp.setTitleAt(i,stabs[i]);
        }
      }
    }

    String cmd = "cmd";
    if(reslist.containsKey(cmd))
    {
      FileEditorDisplay fed = new FileEditorDisplay(cmd,
                                         reslist.get(cmd));
      fed.setCaretPosition(0);
      r1 = new JScrollPane(fed);
      rtp.add(cmd,r1);
    }

    menuBar = new ResultsMenuBar(this,rtp,reslist,inputFiles,
                                 project,mysettings);

    rtp.addChangeListener(new ChangeListener() 
    {
       public void stateChanged(ChangeEvent e) 
       {
         setJMenuBar();
       }
    });
                      
    setJMenuBar();        
    setSize(640,580);
    getContentPane().add(rtp,BorderLayout.CENTER);
    setVisible(true);
  }

  /**
  *
  * Set the menu bar based on the title of the
  * tabbed pane
  *
  */
  public void setJMenuBar()
  {
    int index = rtp.getSelectedIndex();
    if (index == -1)
        return;
    String title = rtp.getTitleAt(index);
    if (toolbar != null)
        remove(toolbar);
    
//    if(title.endsWith("x3d"))
//    {
//      grout = (GroutPanel)rtp.getSelectedComponent();
//      JMenuBar groutMenuBar = grout.getMenuBar();
//      setJMenuBar(groutMenuBar);
//      toolbar = grout.getToolBar();
//    }
//    else
    
    if(title.endsWith(".dat"))
    {
      Graph2DPlot graph = getGraph2DPlot((JScrollPane)rtp.getSelectedComponent());
      if(graph == null)
        return;

      JMenuBar graphMenuBar = graph.getMenuBar(false, this);
      toolbar = graph.getToolBar();
      setJMenuBar(graphMenuBar);
    }
    else if(title.endsWith(".png"))
    {
      menuBar.getComponent(2).setEnabled(false);
      setJMenuBar(menuBar);
      toolbar = menuBar.getToolBar();
      toolbar.getComponent(0).setEnabled(false);
      toolbar.getComponent(2).setEnabled(false);
    }    
    else 
    {
      menuBar.getComponent(2).setEnabled(true);
      setJMenuBar(menuBar);
      toolbar = menuBar.getToolBar();
      toolbar.getComponent(0).setEnabled(true);
      toolbar.getComponent(2).setEnabled(true);
    }
    if (toolbar !=null)
        getContentPane().add(toolbar,BorderLayout.NORTH);
  }


  private Graph2DPlot getGraph2DPlot(JScrollPane jsp)
  {
    Component comp = jsp.getViewport().getView();
    if(comp instanceof Graph2DPlot)
        return (Graph2DPlot)comp;

    return null;
  }

  /**
  *
  * Add the contents of a hash table to the tabbed pane
  * @param h	hash table
  * @param rtp	tabbed pane
  * @return	array of names of PNG tabs
  *
  */
  private String[] addHashContentsToTab(final Hashtable h,
          final JTabbedPane rtp, final String prjdir, JembossParams settings)
  {
    JScrollPane r1;

    String cmd = "cmd";
    Enumeration enumer = h.keys();
    String stabs[] = new String[h.size()];
    int ntabs = 0;

    while (enumer.hasMoreElements())
    {
      final String thiskey = (String)enumer.nextElement().toString();
      if(!thiskey.equals(cmd))
      {
        if( thiskey.endsWith("png") || thiskey.endsWith("html") ||
            thiskey.endsWith(".dat") )
        {
          int index = findInt(thiskey);
          if(index>0 && index < stabs.length)
          {
            stabs[index-1] = new String(thiskey);
            ntabs++;
          }
          else
          {
            ImageIcon i1 = new ImageIcon((byte [])h.get(thiskey));
            ImageIconJPanel iiPanel = new ImageIconJPanel(i1);
            r1 = new JScrollPane(iiPanel);
            r1.getViewport().setBackground(Color.white);
            rtp.add(thiskey,r1);
          }
        }
        else if( thiskey.endsWith(".pdf") || thiskey.endsWith(".svg") )
        {
            Box box = new Box(BoxLayout.Y_AXIS);
            Object obj = h.get(thiskey);
            if (settings.getDesktopSupportsOPENAction())
            {
                String fname;
                JButton bt = new JButton("Open using default desktop viewer");               

                if (prjdir!=null)
                    fname =  prjdir+File.separator+thiskey;
                else
                {
                    fname =  thiskey;
                }

                if (JembossParams.isJembossServer()|| !new File(fname).exists())
                {
                    File tmpf;
                    FileOutputStream out;

                    try {
                        tmpf = File.createTempFile("jemboss_",
                                thiskey.substring(thiskey.lastIndexOf('.')));
                        out = new FileOutputStream(tmpf);

                        if(obj instanceof byte[])
                            out.write((byte[])obj);
                        else
                        {
                            String s = (String)obj;

                            for(int i=0;i<s.length();i++){
                                out.write(s.charAt(i));
                            }
                        }

                        out.close();
                        tmpf.deleteOnExit();
                        fname = tmpf.getAbsolutePath();
                    } catch (IOException e) {
                        e.printStackTrace();
                        fname=null;
                    }
                }

                if(fname!=null)
                {
                    final String fname_ = fname;

                    bt.addActionListener(new ActionListener()
                    {
                        public void actionPerformed(ActionEvent e)
                        {

                            try
                            {
                                Desktop.getDesktop().open(new File(fname_));
                            }
                            catch (IOException ex)
                            {
                                ex.printStackTrace();
                                JOptionPane.showMessageDialog(rtp,
                                        ex.getMessage(),"Error",
                                        JOptionPane.ERROR_MESSAGE);
                            }
                        }
                    }
                    );
                    box.add(bt);
                }
            }

            JTextArea ta;

            if(obj instanceof byte[])
                ta = new JTextArea(new String((byte[])obj));
            else
                ta = new JTextArea((String)obj);

            ta.setEditable(false);
            r1 = new JScrollPane(ta);

            box.add(r1);
            rtp.add(thiskey,box);
        }
        else
        {
          FileEditorDisplay fed = new FileEditorDisplay(thiskey,
                                                     h.get(thiskey));
          fed.setCaretPosition(0);
          r1 = new JScrollPane(fed);
          rtp.add(thiskey,r1);
        }
      }
    }

    String pngtabs[] = new String[ntabs];
    for(int i=0;i<ntabs;i++)
      pngtabs[i] = new String(stabs[i]);
    
    return pngtabs;
  }

  /**
  *
  * Find the number in a string expression
  * @param exp	string expression
  * @return 	number in a string expression or -1 
  *		if none found
  *
  */
  private int findInt(String exp)
  {

    RECompiler rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^(.*?)([:digit:]+)");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(2))).intValue();
        return ia;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return -1;
  }


  public class GroutPanelListener implements ActionListener
  {
    public void actionPerformed(ActionEvent e)
    {
      dispose();
    }
  }
}
