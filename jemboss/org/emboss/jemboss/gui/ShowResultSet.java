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
import java.awt.event.KeyEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.*;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

import org.apache.regexp.*;
import java.io.*;
import java.util.Hashtable;
import java.util.Enumeration;

import org.emboss.grout.*;
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
  private GroutPanel grout = null;

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
    super("Saved Results");
    rtp = new JTabbedPane();

    setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);

    ScrollPanel s1;
    JScrollPane r1;

    String stabs[] = addHashContentsToTab(reslist,rtp);
    if(inputFiles != null)
      addHashContentsToTab(inputFiles,rtp);

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
    String title = rtp.getTitleAt(index);
    if(title.endsWith("x3d"))
    {
      grout = (GroutPanel)rtp.getSelectedComponent();
      JMenuBar groutMenuBar = grout.getMenuBar();
      setJMenuBar(groutMenuBar);
      remove(menuBar.getToolBar());
      getContentPane().add(grout.getToolBar(),BorderLayout.NORTH);
    }
    else if(title.endsWith(".dat"))
    {
      Graph2DPlot graph = getGraph2DPlot((JScrollPane)rtp.getSelectedComponent());
      if(graph == null)
        return;

      JMenuBar graphMenuBar = graph.getMenuBar(false, this);
      remove(menuBar.getToolBar());
      setJMenuBar(graphMenuBar);
    }
    else 
    {
      setJMenuBar(menuBar);
      if(grout != null)
        remove(grout.getToolBar());

      getContentPane().add(menuBar.getToolBar(),BorderLayout.NORTH);
    }
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
  private String[] addHashContentsToTab(Hashtable h,JTabbedPane rtp)
  {
    ScrollPanel s1;
    JScrollPane r1;

    String cmd = "cmd";
    Enumeration enumer = h.keys();
    String stabs[] = new String[h.size()];
    int ntabs = 0;

    while (enumer.hasMoreElements())
    {
      String thiskey = (String)enumer.nextElement().toString();
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
        else if (thiskey.endsWith("x3d")) // grout
        {
          GroutPanel panel = new GroutPanel()
          {
            protected void addDisposeOfGroutPanelMenuItem(JMenu menu)
            {
              JMenuItem menuItem = new JMenuItem("Close");
              menuItem.setAccelerator(KeyStroke.getKeyStroke(
                        KeyEvent.VK_E, ActionEvent.CTRL_MASK));

              menuItem.addActionListener(new ActionListener()
              {
                public void actionPerformed(ActionEvent e)
                {
                  dispose();
                }
              });
              menu.add(menuItem);                         
            }
          };

          if(h.get(thiskey) instanceof String)
            panel.setX3DFile((String)h.get(thiskey));
          else
            panel.setX3DFile(new String((byte[])h.get(thiskey)));
          rtp.add(thiskey,panel);
          setJMenuBar(panel.getMenuBar());
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
