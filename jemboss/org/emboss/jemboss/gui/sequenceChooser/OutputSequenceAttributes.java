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

package org.emboss.jemboss.gui.sequenceChooser;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.io.*;

/**
*
* Creates the output sequence attibutes window
*
*/
public class OutputSequenceAttributes
{

  /** ouput file extension to use */
  private JTextField osextension = new JTextField();
  /** output base name */
  private JTextField osname = new JTextField();
 
//private JTextField osdbname = new JTextField();
  /** feature format */
  private JTextField offormat = new JTextField();
  /** features filename */
  private JTextField ofname = new JTextField();
  /** uniform feature object */
  private JTextField UFO = new JTextField();
  /** separate file for each entry */
  private JRadioButton ossingle;
  /** combobox for file formats */
  private JComboBox fileFormats;
  /** scroll pane for output attributes */
  private JScrollPane rscroll;

  /** output format types */
  private String ff[] = {"unspecified", "text", "fasta", "msf", "fitch",
                         "gcg", "gcg8", "embl", "swiss", "ncbi",
                         "pearson", "genbank", "nbrf", "pir", 
                         "codata", "strider", "clustal", "aln",
                         "phylip", "phylip3", "asn1", "acedb", "dbid", "hennig86", 
                         "jackknifer", "jackknifernon",
                         "nexus", "nexusnon", "paup", "paupnon", 
                         "treecon", "mega", "meganon", "ig", "experiment",
                         "staden", "plain", "gff", "raw", "selex"};

  

  public OutputSequenceAttributes() 
  {
    JPanel pscroll = new JPanel(new BorderLayout());
    rscroll = new JScrollPane(pscroll);

    Box b = new Box(BoxLayout.Y_AXIS);
    pscroll.add(b,BorderLayout.CENTER);

    Box bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    JLabel lab = new JLabel("Sequence Attributes");
    lab.setForeground(Color.red);
    lab.setFont(new Font("SansSerif", Font.BOLD, 13));
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    b.add(Box.createRigidArea(new Dimension(0,3)));   //oformat
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    fileFormats = new JComboBox(ff){
      public Dimension getMinimumSize() 
      {
        return getPreferredSize();
      }
      public Dimension getPreferredSize() 
      {
        return new Dimension(100, 25);
      }
      public Dimension getMaximumSize() 
      {
        return getPreferredSize();
      }
    };
    fileFormats.setSelectedIndex(0);

    bx.add(fileFormats);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("Sequence format");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());
    
    osextension = new JTextField()
    {   
      public Dimension getMinimumSize() 
      {
        return getPreferredSize();
      }
      public Dimension getPreferredSize() 
      {
        return new Dimension(100, 30);
      }
      public Dimension getMaximumSize() 
      {
        return getPreferredSize();
      }
    };

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(osextension);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("File name extension");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());


    osname = new JTextField()
    {       
      public Dimension getMinimumSize() 
      {
        return getPreferredSize();
      }
      public Dimension getPreferredSize() 
      {
        return new Dimension(100, 30);
      }
      public Dimension getMaximumSize() 
      {
        return getPreferredSize();
      }
    };

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(osname);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("Base file name");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());


                                                 //ossingle
    ossingle = new JRadioButton("Separate file for each entry");
//  ossingle.setBackground(Color.white);
    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(ossingle);
    bx.add(Box.createHorizontalGlue());

    UFO = new JTextField()                      //oufo
    {
      public Dimension getMinimumSize() 
      {
        return getPreferredSize();
      }
      public Dimension getPreferredSize() 
      {
        return new Dimension(100, 30);
      }
      public Dimension getMaximumSize() 
      {
        return getPreferredSize();
      }
    };

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(UFO);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("UFO features");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    offormat = new JTextField()
    {
      public Dimension getMinimumSize() 
      {
        return getPreferredSize();
      }
      public Dimension getPreferredSize() 
      {
        return new Dimension(100, 30);
      }
      public Dimension getMaximumSize() 
      {
        return getPreferredSize();
      }
    };

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(offormat);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("Features format");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    ofname = new JTextField()              //ofname
    {
      public Dimension getMinimumSize() 
      {
        return getPreferredSize();
      }
      public Dimension getPreferredSize() 
      {
        return new Dimension(100, 30);
      }
      public Dimension getMaximumSize() 
      {
        return getPreferredSize();
      }
    };

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(ofname);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("Features file name");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

  }

  /**
  *
  * Get the scoll panel 
  * @return	scroll panel
  *
  */
  public JScrollPane getJScrollPane()
  {
    return rscroll;
  }

  /**
  *
  * Get the format selected
  * @return 	selected format
  *
  */
  public String getFormatChoosen() 
  {
    return (String)fileFormats.getSelectedItem();
  }

  /**
  *
  * Determine if UFO has a value
  * @return 	true if a value has been set
  *
  */
  public boolean isUFODefault()
  {
    if(UFO.getText() == null || UFO.getText().equals(""))
     return true;
    else
     return false;
  }

  /**
  *
  * Determine if an extension has been selected
  * @return     true if a value has been set
  *
  */
  public boolean isExtensionDefault()
  {
    if(osextension.getText() == null || osextension.getText().equals(""))
     return true;
    else
     return false;
  }

  /**
  *
  * Determine if a output file name is set
  * @return     true if a value has been set
  *
  */
  public boolean isNameDefault()
  {
    if(osname.getText() == null || osname.getText().equals(""))
     return true;
    else
     return false;
  }

//public boolean isDBNameDefault()
//{
//  if(osdbname.getText() == null || osdbname.getText().equals(""))
//   return true;
//  else
//   return false;
//}

  /**
  *
  * Determine if a feature name is set
  * @return     true if a value has been set
  *
  */
  public boolean isFNameDefault()
  {
    if(ofname.getText() == null || ofname.getText().equals(""))
     return true;
    else
     return false;
  }

  /**
  *
  * Get the EMBOSS command line for the output sequence 
  * attributes
  * @return	output attibutes command line options
  *
  */
  public String getOuputSeqAttr() 
  {
    String options="";

    if(!isUFODefault())
      options = options.concat(" -oufo " + UFO.getText());

    if(!getFormatChoosen().equals("unspecified"))
       options = options.concat(" -osformat " + getFormatChoosen());
     
    if(!isExtensionDefault())
       options = options.concat(" -osextension " + osextension.getText());

    if(!isNameDefault())
       options = options.concat(" -osname " + osname.getText());

//  if(!isDBNameDefault())
//     options = options.concat(" -osdbname " + osdbname.getText());

    if(ossingle.isSelected())
       options = options.concat(" -ossingle ");

    if(!isFNameDefault())
       options = options.concat(" -ofname " + ofname.getText());   

    return options;
  }

}

