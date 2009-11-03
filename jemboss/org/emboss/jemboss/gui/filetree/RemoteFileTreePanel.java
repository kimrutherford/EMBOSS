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
*  based on EmbreoViewPanel
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/

package org.emboss.jemboss.gui.filetree;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Hashtable;

import org.emboss.jemboss.soap.FileRoots;
import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.JembossParams;

/**
*
* Panel for the remote file tree manager
*
*/
public class RemoteFileTreePanel extends JPanel
{

  /** busy cursor */
  final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  /** done cursor */
  final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  /** combo to select the root directory from */
  private JComboBox rootSelect;

  /**
  *
  * @param mysettings	jemboss properties
  * @throws JembossSoapExceptio	when connection fails
  *
  */
  public RemoteFileTreePanel(final JembossParams mysettings)
                                                    throws JembossSoapException
  {
    this(mysettings,true);
  }

  /**
  *
  * Creates a panel to contain the directory trees on an embreo server.
  * A combobox allows the user to select which filesystem to view.
  * @param mysettings   jemboss properties
  * @param laddCombo	
  * @throws JembossSoapExceptio when connection fails
  *
  */
  public RemoteFileTreePanel(final JembossParams mysettings, boolean laddCombo) 
                                                      throws JembossSoapException
  {
   
    setLayout(new BorderLayout());

    final Hashtable rootsdone = new Hashtable();                 //record where we are
    final FileRoots efr = new FileRoots(mysettings); //roots list
    final JPanel filep = new JPanel();
    final CardLayout fileLayout = new CardLayout();
    filep.setLayout(fileLayout);

    rootSelect = new JComboBox(efr.getRootVector());
    int cstart = efr.getDefaultRootIndex();
    if (cstart != -1) 
      rootSelect.setSelectedIndex(cstart);
    Dimension d = rootSelect.getPreferredSize();
    rootSelect.setPreferredSize(new Dimension((int)d.getWidth(),(int)d.getHeight()-5));
   
    if(laddCombo)
      add(rootSelect, BorderLayout.NORTH);
    rootSelect.addActionListener(new ActionListener() 
    {
      public void actionPerformed(ActionEvent e) 
      {
	JComboBox cb = (JComboBox)e.getSource();
	String rootName = (String)cb.getSelectedItem();
	efr.setCurrentRoot(rootName);
	efr.setCurrentDir(".");
	if (rootsdone.containsKey(rootName))   
        {
	  fileLayout.show(filep,rootName);
	} 
        else
        {
	  setCursor(cbusy);
  	  RemoteDragTree eft2 = new RemoteDragTree(mysettings, efr);
	  setCursor(cdone);
	  JScrollPane ss = new JScrollPane(eft2);
	  filep.add(ss,efr.getCurrentRoot());
	  rootsdone.put(efr.getCurrentRoot(),"yes");
	  fileLayout.show(filep,efr.getCurrentRoot());
	}
      }
    });

    setCursor(cbusy);
    RemoteDragTree eft = new RemoteDragTree(mysettings, efr);
    setCursor(cdone);
    JScrollPane s = new JScrollPane(eft);
    filep.add(s, efr.getCurrentRoot());
    rootsdone.put(efr.getCurrentRoot(),"yes");

    setPreferredSize(new Dimension(180,500));
    add(filep, BorderLayout.CENTER);

  }

  /**
  *
  * Get the root selection JComboBox
  * @return 	combobox
  *
  */
  public JComboBox getRootSelect()
  {
    return rootSelect;
  }

}

