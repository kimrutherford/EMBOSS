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
 
import java.io.*;
import java.util.Vector;
import javax.swing.*;
import java.awt.event.*;
import java.awt.BorderLayout;
import java.awt.Dimension;

public class Favorites
{

  private JMenuItem favItems[] = null;
  private JMenu favJMenu;

  public Favorites(JMenu favJMenu)
  {
    this.favJMenu = favJMenu;
    checkSavedFavorites();

    if(favItems == null)
      defaultFavorites();
  }

  private void checkSavedFavorites()
  {
    File ffav = new File(System.getProperty("user.home")
                + System.getProperty("file.separator")
                + ".jembossFavorites");
    if(!ffav.exists())
      return;

    int nmenu = 0;
    Vector vfav = new Vector();
    String line = null;
    try
    {
      BufferedReader reader = new BufferedReader(new FileReader(ffav));
      while((line = reader.readLine()) != null)
      {
        line = line.trim();
        vfav.add(line);
        if(!line.equals(""))
          nmenu++;
      }

      if(nmenu < 1)
        return;
  
      favItems = new JMenuItem[nmenu];
      nmenu = 0;
      for(int i=0;i<vfav.size();i++)
      { 
        line = (String)vfav.get(i);
        if(line.equals(""))
        {
          favJMenu.add(new JSeparator());
          continue;
        }
        int index   = line.indexOf("\t");
        favItems[nmenu] = new JMenuItem(line.substring(0,index));
        favItems[nmenu].setActionCommand(line.substring(index+1));
        favJMenu.add(favItems[nmenu]);
        nmenu++;
      }
    }
    catch(Exception exp){}

  }

  public JMenuItem[] getFavorites()
  {
    return favItems;
  }

  protected JMenuItem add(String prog, String des)
  {
    JMenuItem fav = new JMenuItem(des+" ("+prog+")");
    fav.setActionCommand(prog);
    favJMenu.add(fav);
    return fav;
  }

  /**
  *
  * Implement edit dialog for favourites
  *
  */
  protected void edit(String allProgs[])
  { 
    JPanel panel = new JPanel(new BorderLayout());
    final Box bdown = Box.createVerticalBox();
    JScrollPane jsp = new JScrollPane(bdown);
    jsp.setPreferredSize(new Dimension(500,300));
    panel.add(jsp,BorderLayout.CENTER);

    Box bacross1 = Box.createHorizontalBox();
    bacross1.add(new JLabel("Application and Description"));
    bacross1.add(Box.createHorizontalGlue());
    panel.add(bacross1,BorderLayout.NORTH);

    int nmenu = favJMenu.getItemCount();
    JTextField tf[] = new JTextField[nmenu];
    JComboBox combo[] = new JComboBox[nmenu];

    for(int i=0; i<nmenu; i++)
    {
      final JMenuItem fav = favJMenu.getItem(i);
      if(fav == null)
        continue;

      String action = fav.getActionCommand();
      if(action.equals("ADD") || action.equals("EDIT"))
        continue;

      final Box bacross = Box.createHorizontalBox();
      combo[i] = new JComboBox(allProgs);
      combo[i].setSelectedItem(action);
      bacross.add(combo[i]);

      tf[i] = new JTextField(fav.getText());
      Dimension d = tf[i].getPreferredSize();
      d = new Dimension(200,(int)d.getHeight());
      tf[i].setPreferredSize(d);
      tf[i].setCaretPosition(0);
      bacross.add(tf[i]);

      JButton deleteFav = new JButton("Delete");
      deleteFav.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          bdown.remove(bacross);
          bdown.repaint();
          fav.setActionCommand("DELETE");
        }
      });
      bacross.add(deleteFav);
      bacross.add(Box.createHorizontalGlue());
      bdown.add(bacross);
    }
    bdown.add(Box.createVerticalGlue());
 
    int n = JOptionPane.showConfirmDialog(null, panel, 
                                  "Edit Favourites",
                                  JOptionPane.OK_CANCEL_OPTION);

    if(n == JOptionPane.CANCEL_OPTION)
      return;

// upate menu items
    for(int i=0; i<nmenu; i++)
    {
      JMenuItem fav = favJMenu.getItem(i);
      if(fav == null)
        continue;
                                                                                               
      String action = fav.getActionCommand();
      if(action.equals("ADD") || action.equals("EDIT") ||
         action.equals("DELETE"))
        continue;

      fav.setText(tf[i].getText());
      fav.setActionCommand((String)combo[i].getSelectedItem());
    }
   
// lastly any to delete
    for(int i=nmenu-1; i!=0; i--)
    {
      JMenuItem fav = favJMenu.getItem(i);
      if(fav == null)
        continue;

      String action = fav.getActionCommand();
      if(action.equals("DELETE"))
        favJMenu.remove(fav);
    }
  }

  private void defaultFavorites()
  {
    String favs[][] = {
                         { "Add to Favourites", "ADD" },
                         { "Edit Favourites"  , "EDIT" },
                         { "Database Sequence Retrieval", "seqret"},
                         { "Multiple Alignments", "emma" },
                         { "Dotplots (exact)", "dottup" },
                         { "Dotplots (similar)", "dotmatcher" },
                         { "Primer Design", "eprimer3" },
                         { "Pattern Search (nucleotide)", "fuzznuc" },
                         { "Pattern Search (protein)", "fuzzpro" },
                         { "Find Reading Frames", "getorf" },
                         { "Best Local Alignments", "matcher" },
                         { "Global Alignments", "needle" },
                         { "Display Restriction Sites", "remap" }
                      };
                                                                                
                                                                                
    int nfavs = favs.length;
    favItems  = new JMenuItem[nfavs];
    for(int i=0; i<nfavs; i++)
    {
      favItems[i] = new JMenuItem(favs[i][0]);
      favItems[i].setActionCommand(favs[i][1]);
      favJMenu.add(favItems[i]);
      if(favItems[i].getActionCommand().equals("EDIT"))
        favJMenu.add(new JSeparator());
    }
    favJMenu.add(new JSeparator());
  }

}

