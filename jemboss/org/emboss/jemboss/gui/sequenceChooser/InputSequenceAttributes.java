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
import java.awt.event.*;
import java.util.ArrayList;
import java.util.List;

import org.emboss.jemboss.gui.form.*;

/**
*
* Creates the input sequence attibutes window 
*
*/
public class InputSequenceAttributes 
{

  /** -sbegin flag */
  private TextFieldInt sbeg = new TextFieldInt();
  /** -send flag   */
  private TextFieldInt send = new TextFieldInt();
//  private TextFieldSink sopenFile = new TextFieldSink();
//  private JTextField sdbName = new JTextField();
//  private JTextField sID = new JTextField();
  /** uniform feature object */
  private JTextField UFO = new JTextField();
  /** available databases    */
  private JComboBox dbs;
  /** combobox for file formats */
  private JComboBox fileFormats;
  /** select to reverse sequence */
  private JRadioButton rev;
  /** select to specify a nucleotide sequence */
  private JRadioButton nucleotide;
  /** select to specify a protein sequence    */
  private JRadioButton protein;
  /** select to specify lower case */
  private JRadioButton lower;
  /** select to specify upper case */
  private JRadioButton upper;
  /** scroll panel for attibute panel */
  private JScrollPane rscroll;

  /** input format types */
  private String ff[] = {"unspecified", "text", "fasta", "msf", 
                         "gcg", "gcg8", "embl", "swiss", "ncbi",
                         "pearson", "genbank", "nbrf", "pir", 
                         "codata", "strider", "clustal", "aln",
                         "bam", "sam",
                         "phylip", "acedb", "dbid", "hennig86", 
                         "jackknifer", "jackknifernon",
                         "nexus", "nexusnon", "paup", "paupnon", 
                         "treecon", "mega", "meganon", "ig", "experiment",
                         "staden", "plain", "gff", "raw", "selex"};
  

  /**
  *
  * @param db	available databases
  * @param fc	file chooser
  *
  */
  public InputSequenceAttributes(String[] db, final FileChooser fc)
  {

    JPanel pscroll = new JPanel(new BorderLayout());
    rscroll = new JScrollPane(pscroll);

    Box b = new Box(BoxLayout.Y_AXIS);
    pscroll.add(b,BorderLayout.CENTER);
   
    JLabel lab = new JLabel("Sequence Attributes");
    lab.setForeground(Color.red);
    lab.setFont(new Font("SansSerif", Font.BOLD, 13));
    Box bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    
    sbeg.setPreferredSize(new Dimension(50, 28));
    sbeg.setMaximumSize(new Dimension(50, 28));

    send.setPreferredSize(new Dimension(50, 28));
    send.setMaximumSize(new Dimension(50, 28));

    b.add(Box.createRigidArea(new Dimension(0,3)));   //databases
    bx = new Box(BoxLayout.X_AXIS);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    b.add(bx);
    dbs = new JComboBox(db);
    dbs.insertItemAt("unspecified",0); 
    dbs.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        String sel = (String)((JComboBox)e.getSource()).getSelectedItem();
        if(!sel.equalsIgnoreCase("unspecified"))
          fc.setText(sel + ":");
      }
    });

    Dimension min = dbs.getMinimumSize();
    dbs.setPreferredSize(min);
    dbs.setMaximumSize(min);
    dbs.setSelectedIndex(0);

    bx.add(dbs);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("Databases available");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());


    b.add(Box.createRigidArea(new Dimension(0,3)));   //sformat
    bx = new Box(BoxLayout.X_AXIS);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    b.add(bx);
    fileFormats = new JComboBox(ff);
    fileFormats.setPreferredSize(min);
    fileFormats.setMaximumSize(min);
    fileFormats.setSelectedIndex(0);

    bx.add(fileFormats);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("Sequence format");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());
    

    b.add(Box.createRigidArea(new Dimension(0,3)));   //sbeg
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(sbeg);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("begin");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    b.add(Box.createRigidArea(new Dimension(0,3)));   //send
    bx = new Box(BoxLayout.X_AXIS);
    b.add(bx);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(send);
    bx.add(Box.createRigidArea(new Dimension(2,0)));
    lab = new JLabel("end");
    lab.setForeground(Color.black);
    bx.add(lab);
    bx.add(Box.createHorizontalGlue());

    b.add(Box.createRigidArea(new Dimension(0,3)));   //srev
    bx = new Box(BoxLayout.X_AXIS);
    rev = new JRadioButton("reverse complement");
//  rev.setBackground(Color.white);
    rev.setSelected(false);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(rev);
    bx.add(Box.createHorizontalGlue());
    b.add(bx);

    b.add(Box.createRigidArea(new Dimension(0,3)));   //nucleotide
    bx = new Box(BoxLayout.X_AXIS);
    nucleotide = new JRadioButton("nucleotide");
//  nucleotide.setBackground(Color.white);
    nucleotide.setSelected(false);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(nucleotide);
    bx.add(Box.createHorizontalGlue());
    b.add(bx);

    b.add(Box.createRigidArea(new Dimension(0,3)));   //protein
    bx = new Box(BoxLayout.X_AXIS);
    protein = new JRadioButton("protein");
//  protein.setBackground(Color.white);
    protein.setSelected(false);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(protein);
    bx.add(Box.createHorizontalGlue());
    b.add(bx);

    ButtonGroup group = new ButtonGroup();
    group.add(nucleotide);
    group.add(protein);

    b.add(Box.createRigidArea(new Dimension(0,3)));   //lower
    bx = new Box(BoxLayout.X_AXIS);
    lower = new JRadioButton("Make lower case");
//  lower.setBackground(Color.white);
    lower.setSelected(false);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(lower);
    bx.add(Box.createHorizontalGlue());
    b.add(bx);

    b.add(Box.createRigidArea(new Dimension(0,3)));   //upper
    bx = new Box(BoxLayout.X_AXIS);
    upper = new JRadioButton("Make upper case");
//  upper.setBackground(Color.white);
    lower.setSelected(false);
    bx.add(Box.createRigidArea(new Dimension(3,0)));
    bx.add(upper);
    bx.add(Box.createHorizontalGlue());
    b.add(bx);

    group = new ButtonGroup();
    group.add(upper);
    group.add(lower);


//  sopenFile = new TextFieldSink();                 //sopenfile
//  sopenFile.setPreferredSize(new Dimension(100, 30));
//  sopenFile.setMaximumSize(new Dimension(100, 30));


//  b.add(Box.createRigidArea(new Dimension(0,3)));  
//  bx = new Box(BoxLayout.X_AXIS);
//  b.add(bx);
//  bx.add(Box.createRigidArea(new Dimension(3,0)));
//  bx.add(sopenFile);
//  bx.add(Box.createRigidArea(new Dimension(2,0)));
//  lab = new JLabel("Input filename");
//  lab.setForeground(Color.black);
//  bx.add(lab);
//  bx.add(Box.createHorizontalGlue());
    
//  sdbName = new JTextField();                     //sdbName
//  sdbName.setPreferredSize(new Dimension(100, 30));
//  sdbName.setMaximumSize(new Dimension(100, 30));

//  b.add(Box.createRigidArea(new Dimension(0,3)));
//  bx = new Box(BoxLayout.X_AXIS);
//  b.add(bx);
//  bx.add(Box.createRigidArea(new Dimension(3,0)));
//  bx.add(sdbName);
//  bx.add(Box.createRigidArea(new Dimension(2,0)));
//  lab = new JLabel("Database name");
//  lab.setForeground(Color.black);
//  bx.add(lab);
//  bx.add(Box.createHorizontalGlue());


//  sID = new JTextField();                        //sid
//  sID.setPreferredSize(new Dimension(100, 30));
//  sID.setMaximumSize(new Dimension(100, 30));


//  b.add(Box.createRigidArea(new Dimension(0,3)));
//  bx = new Box(BoxLayout.X_AXIS);
//  b.add(bx);
//  bx.add(Box.createRigidArea(new Dimension(3,0)));
//  bx.add(sID);
//  bx.add(Box.createRigidArea(new Dimension(2,0)));
//  lab = new JLabel("Database entry name");
//  lab.setForeground(Color.black);
//  bx.add(lab);
//  bx.add(Box.createHorizontalGlue());


    UFO = new JTextField();                     //ufo
    UFO.setPreferredSize(new Dimension(100, 30));
    UFO.setMaximumSize(new Dimension(100, 30));


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

  }

  /**
  *
  * Get the scroll panel
  * @return	scroll panel
  *
  */
  public JScrollPane getJScrollPane()
  {
    return rscroll;
  }

  /**
  *
  * Set the -sbegin flag
  * @param iseq		value for -sbeg
  *
  */
  public void setBegSeq(int iseq)
  {
    sbeg.setValue(iseq);
  }

  /**
  *
  * Set the -send flag
  * @param iseq         value for -send
  *
  */
  public void setEndSeq(int iseq)
  {
    send.setValue(iseq);
  }

  /**
  *
  * Set the -sbegin flag
  * @param s         	value for -sbeg
  *
  */
  public void setBegSeq(String s)
  {
    sbeg.setText(s);
  }

  /**
  *
  * Set the -send flag
  * @param s         	value for -send
  *
  */
  public void setEndSeq(String s)
  {
    send.setText(s);
  }

  /**
  * 
  * Get the -sbegin flag
  * @return 	value for -sbeg
  *
  */
  public String getBegSeq()
  {
    return sbeg.getText();
  }

  /**
  * 
  * Get the -send flag
  * @return     value for -send
  *
  */
  public String getEndSeq()
  {
    return send.getText();
  }

  /**
  *
  * Determine if there is a default -sbeg value
  * @return	true if -sbeg is set
  *
  */
  public boolean isBeginSeqDefault()
  {
    if(sbeg.getText() == null ||
       sbeg.getText().equals("") )
     return true;
    else
     return false; 
  }

  /**
  *
  * Determine if there is a default -send value
  * @return     true if -send is set
  *
  */
  public boolean isEndSeqDefault()
  {
    if(send.getText() == null ||
       sbeg.getText().equals("") )
     return true;
    else
     return false;
  }

//  public boolean isOpenFileDefault()
//  {
//    if(sopenFile.getText() == null || sopenFile.getText().equals(""))
//     return true;
//    else
//     return false;
//  }


//  public boolean isDBNameDefault()
//  {
//    if(sdbName.getText() == null || sdbName.getText().equals(""))
//     return true;
//    else
//     return false;
//  }

//  public boolean isSIDDefault()
//  {
//    if(sID.getText() == null || sID.getText().equals(""))
//     return true;
//    else
//     return false;
//  }

  /**
  *
  * Determine if there is a default -ufo value
  * @return     true if -ufo is set
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
  * Get the file format selected 
  * @return	file format
  *
  */
  public String getFormatChoosen() 
  {
    return (String)fileFormats.getSelectedItem();
  }

  /**
  *
  * Get the EMBOSS command line for the values selected
  * for the input sequence attributes 
  * @param seq	sequence number
  * @return	command line input sequence options
  *
  */
  public String getInputSeqAttr(int seq) 
  {
    String options = " ";

    if(!isBeginSeqDefault())
      options = options.concat(" -sbegin" + seq + " " +
                                       sbeg.getValue());

    if(!isEndSeqDefault())
      options = options.concat(" -send" + seq + " " +
                                     send.getValue());

    if(!getFormatChoosen().equals("unspecified"))
      options = options.concat(" -sformat" + seq + " " +
                                    getFormatChoosen());

    if(rev.isSelected())
      options = options.concat(" -srev" + seq);

    if(nucleotide.isSelected())
      options = options.concat(" -snucleotide" + seq);

    if(protein.isSelected())
      options = options.concat(" -sprotein" + seq);

    if(lower.isSelected())
      options = options.concat(" -slower" + seq);

    if(upper.isSelected())
      options = options.concat(" -supper" + seq);

//    if(!isOpenFileDefault())
//      options = options.concat(" -sopenfile" + seq + " " +
//                                     sopenFile.getText());

//    if(!isDBNameDefault())
//      options = options.concat(" -sdbname" + seq + " " +
//                                     sdbName.getText());    

//    if(!isSIDDefault())
//      options = options.concat(" -sid" + seq + " " +
//                                     sID.getText());

    if(!isUFODefault())
      options = options.concat(" -ufo" + seq + " " +
                                       UFO.getText());

    return options;
  }

  /**
  *
  * Get the EMBOSS command line input sequence options for the values selected
  * @param seq	sequence number
  * @return	command line input sequence options as string array
  *
  */  
  public List getInputSeqAttrA(int seq) 
  {
    List optionsA= new ArrayList();
    if(!isBeginSeqDefault()){
      optionsA.add("-sbegin"+ seq);
      optionsA.add(String.valueOf(sbeg.getValue()));
    }

    if(!isEndSeqDefault()){
      optionsA.add("-send"+ seq);
      optionsA.add(String.valueOf(send.getValue()));
    }

    if(!getFormatChoosen().equals("unspecified")){
      optionsA.add("-sformat"+ seq);
      optionsA.add(getFormatChoosen());
    }
    if(rev.isSelected()){
      optionsA.add("-srev"+ seq);
    }

    if(nucleotide.isSelected()){
      optionsA.add("-snucleotide"+ seq);
    }

    if(protein.isSelected()){
      optionsA.add("-sprotein"+ seq);
    }

    if(lower.isSelected()){
      optionsA.add("-slower"+ seq);
    }

    if(upper.isSelected()){
      optionsA.add("-supper"+ seq);
    }

    if(!isUFODefault()){
      optionsA.add("-ufo"+ seq);
      optionsA.add(UFO.getText());
    }

    return optionsA;
  }

}

