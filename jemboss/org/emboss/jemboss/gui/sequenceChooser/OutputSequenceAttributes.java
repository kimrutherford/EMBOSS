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
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.ArrayList;
import java.util.List;

import javax.swing.*;

import org.emboss.jemboss.gui.form.TextFieldSink;

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
 
  /** feature format */
  private JTextField offormat = new JTextField();
  /** features filename */
  private JTextField ofname = new JTextField();
  /** uniform feature object */
  //private JTextField UFO = new JTextField();
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

  

  public OutputSequenceAttributes(final TextFieldSink tfs) 
  {
    JPanel pscroll = new JPanel(new BorderLayout());
    rscroll = new JScrollPane(pscroll);

    Box b = new Box(BoxLayout.Y_AXIS);
    pscroll.add(b,BorderLayout.CENTER);

    Box bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    JLabel lab = new JLabel("Sequence Attributes");
    lab.setForeground(Color.red);
    lab.setFont(new Font("SansSerif", Font.BOLD, 13));
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    b.add(Box.createRigidArea(new Dimension(0,3)));   //oformat
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    fileFormats = new JComboBox(ff);
    setRigidSize(fileFormats, 100, 25);
    fileFormats.setSelectedIndex(0);

    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(fileFormats);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    lab = new JLabel("Sequence format");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());
    
    osextension = new JTextField();
    setRigidSize(osextension, 100, 30);

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(osextension);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    final JLabel lab_fne = new JLabel("File name extension");
    lab.setForeground(Color.black);
    bx.add(lab_fne);
    bx.add(Box.createHorizontalGlue());


    osname = new JTextField();
    setRigidSize(osname, 100, 30);

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(osname);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    final JLabel lab_bfn = new JLabel("Base file name");
    lab.setForeground(Color.black);
    bx.add(lab_bfn);
    bx.add(Box.createHorizontalGlue());

    //ossingle
    ossingle = new JRadioButton("Separate file for each entry");
    ossingle.addItemListener(new ItemListener(){
        public void itemStateChanged(ItemEvent e) {
            final String why = "'Separate file for each entry' option " +
            "has been selected";
            if (ossingle.isSelected()){
                osname.setEnabled(false);
                osextension.setEnabled(false);
                osname.setToolTipText(why);
                osextension.setToolTipText(why);
                lab_fne.setEnabled(false);
                lab_bfn.setEnabled(false);
                tfs.setEnabled(false);
            } else {
                osname.setEnabled(true);
                osextension.setEnabled(true);
                osname.setToolTipText("");
                osextension.setToolTipText("");
                tfs.setEnabled(true);
                lab_fne.setEnabled(true);
                lab_bfn.setEnabled(true);
            }
        }
    });

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(ossingle);
    bx.add(Box.createHorizontalGlue());

    //UFO = new JTextField();                      //oufo
    //setRigidSize(UFO, 100, 30);

    //b.add(Box.createRigidArea(new Dimension(0,3)));
    //bx = new Box(BoxLayout.X_AXIS);
    //b.add(bx);
    //bx.add(Box.createRigidArea(new Dimension(3,0)));
    //bx.add(UFO);
    //bx.add(Box.createRigidArea(new Dimension(2,0)));
    //lab = new JLabel("UFO features");
    //lab.setForeground(Color.black);
    //bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    offormat = new JTextField();
    setRigidSize(offormat, 100, 30);

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(offormat);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    final JLabel lab_ff = new JLabel("Features format");
    lab.setForeground(Color.black);
    bx.add(lab_ff);
    bx.add(Box.createHorizontalGlue());

    ofname = new JTextField();              //ofname
    setRigidSize(ofname, 100, 30);

    b.add(Box.createRigidArea(new Dimension(0,3)));
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(ofname);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    final JLabel lab_ffn = new JLabel("Features file name");
    lab.setForeground(Color.black);
    bx.add(lab_ffn);
    bx.add(Box.createHorizontalGlue());

    fileFormats.addItemListener(new ItemListener(){
        public void itemStateChanged(ItemEvent e) {
            final String why = "A sequence format that is " +
            		"capable of holding a feature table has been specified";
            int i = fileFormats.getSelectedIndex();
            String format = ff[i];
            if (format.equals("swiss")||format.equals("nbrf")||
                    format.equals("pir") || format.equals("genbank")||
                    format.equals("embl") || format.equals("gff")){
                offormat.setEnabled(false);
                ofname.setEnabled(false);
                lab_ff.setEnabled(false);
                lab_ffn.setEnabled(false);
                offormat.setToolTipText(why);
                ofname.setToolTipText(why);
                //UFO.setEnabled(false);
            } else {
                offormat.setEnabled(true);
                ofname.setEnabled(true);
                offormat.setToolTipText("");
                ofname.setToolTipText("");
                lab_ff.setEnabled(true);
                lab_ffn.setEnabled(true);
                //UFO.setEnabled(true);
            }
        }
    });

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
//  public boolean isUFODefault()
//  {
//    if(UFO.getText() == null || UFO.getText().equals(""))
//     return true;
//    else
//     return false;
//  }

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

    if(!getFormatChoosen().equals("unspecified"))
       options = options.concat(" -osformat " + getFormatChoosen());
     
    if(osextension.isEnabled() && !isExtensionDefault())
       options = options.concat(" -osextension " + osextension.getText());

    if(osname.isEnabled() && !isNameDefault())
       options = options.concat(" -osname " + osname.getText());

    if(ossingle.isSelected())
       options = options.concat(" -ossingle ");

    if(ofname.isEnabled() && !isFNameDefault())
       options = options.concat(" -ofname " + ofname.getText());   

    if(offormat.isEnabled() && offormat.getText() != null && !offormat.getText().equals(""))
        options = options.concat(" -offormat " + offormat.getText());   
    
    return options;
  }

  
  public List getOuputSeqAttrA() 
  {
    List options= new ArrayList();

    if(!getFormatChoosen().equals("unspecified")){
    	options.add("-osformat");
    	options.add(getFormatChoosen());
    }
     
    if(osextension.isEnabled() && !isExtensionDefault()){
    	options.add("-osextension");
    	options.add(osextension.getText());
    }

    if(osname.isEnabled() && !isNameDefault()){
    	options.add("-osname");
    	options.add(osname.getText());
    }

    if(ossingle.isSelected())
       options.add("-ossingle");

    if(ofname.isEnabled() && !isFNameDefault()){
    	options.add("-ofname");
    	options.add(ofname.getText());
    }

    if(offormat.isEnabled() && offormat.getText() != null && !offormat.getText().equals("")){
    	options.add("-offormat");
    	options.add(offormat.getText());
    }
    
    return options;
  }

  
  private void setRigidSize(JComponent c, int width, int height){
      Dimension d = new Dimension(width, height);
      c.setPreferredSize(d);
      c.setMinimumSize(d);
      c.setMaximumSize(d);
  }

}

