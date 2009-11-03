/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.editor;

import javax.swing.*;
import javax.swing.table.TableColumn;
import java.util.Vector;
import java.awt.event.*;
import java.awt.*;


/**
*
* Use to display the identical matches table for each 
* pair of sequences
*
*/
public class IDTableJFrame extends JFrame
{

  private PrintableJTable idTable = new PrintableJTable();

  /**
  *
  * @param vseq	collection of sequences to get the percentage 
  *             of identical matches between each pair of sequences
  *
  */
  public IDTableJFrame(Vector vseq)
  {
    super("Percentatge ID");

    int nseq = vseq.size();

    Font font = idTable.getFont();
    FontMetrics fm = idTable.getFontMetrics(font);
    int max_width = 0;

    String[] columnNames = new String[nseq+1];
    Object[][] rowData = new Object[nseq][nseq+1];

    columnNames[0] = new String("");

    for(int i=0;i<nseq;i++)
    {
      Sequence s1 = (Sequence)vseq.get(i);
      int len1 = s1.getLength();
      String s1Name = s1.getName();

      int w = fm.stringWidth(s1Name+"  ");
      if(w > max_width)
        max_width = w;
 
      columnNames[i+1] = s1Name;
      rowData[i][0] = s1Name;
  
      if(!s1Name.equals(Consensus.DEFAULT_SEQUENCE_NAME))
      {
        for(int j=i;j<nseq;j++)
        {
          Sequence s2 = (Sequence)vseq.get(j);
          if(!s2.getName().equals(Consensus.DEFAULT_SEQUENCE_NAME))
          {
            int id = 0;
            int len2 = s2.getLength();
            if(len2 > len1)
              len1 = len2;

            for(int p=0;p<len1;p++)
            {
              try
              {
                if(s1.getResidueAt(p).equals(
                   s2.getResidueAt(p)))
                  id++;
              }
              catch(StringIndexOutOfBoundsException outBounds)
              {
                System.out.println("Sequences have different length!");
              }
            }

            float percent = Math.round((float)id/(float)len1*10000.f);
            rowData[i][j+1] = new Float(percent/100.f);
          }
        }
      }
    }

    idTable = new PrintableJTable(rowData,columnNames);
    TableColumn column = null;

    column = idTable.getColumnModel().getColumn(0);
    column.setPreferredWidth(max_width);
    for(int i=0;i<nseq;i++)
    {
      column = idTable.getColumnModel().getColumn(i+1);
      String name = idTable.getColumnName(i+1);
      int wid = fm.stringWidth(name+"  ");
      column.setPreferredWidth(wid);
        
    }
    idTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

// set up a menu bar
    JMenuBar menuBar = new JMenuBar();

// File menu
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

// print
    JMenuItem printMenu = new JMenuItem("Print");
    printMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_P, ActionEvent.CTRL_MASK));

    printMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        idTable.doPrintActions();
      }
    });
    fileMenu.add(printMenu);

// exit
    fileMenu.add(new JSeparator());
    JMenuItem fileMenuExit = new JMenuItem("Close");
    fileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(fileMenuExit);
    menuBar.add(fileMenu);
    setJMenuBar(menuBar);

//  int height = (int)idTable.getPreferredSize().getHeight();
//  idTable.setPreferredSize(new Dimension(width+max_width,height));
    JScrollPane scrollTable = new JScrollPane(idTable);
    scrollTable.setPreferredSize(idTable.getPreferredSize());
    getContentPane().add(scrollTable);
    pack();

  }

 
  protected void printTable()
  {
    int ncol = idTable.getColumnCount();
    int nrow = idTable.getRowCount();
    for(int i=0;i<nrow;i++)
    {
      for(int j=0;j<ncol;j++)
      {
        Object val = idTable.getValueAt(i,j);
        if(val==null)
          val="-";
        System.out.print(val+"\t");
      }
      System.out.print("\n");
    }   
  }

}

