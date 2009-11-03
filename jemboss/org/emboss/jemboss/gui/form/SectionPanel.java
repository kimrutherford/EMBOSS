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

package org.emboss.jemboss.gui.form;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.TitledBorder;
import javax.swing.border.Border;
import java.util.Vector;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.awt.event.*;
import java.io.*;

import org.emboss.jemboss.programs.RunEmbossApplication2;
import org.emboss.jemboss.gui.AdvancedOptions;
import org.emboss.jemboss.gui.BuildProgramMenu;
import org.emboss.jemboss.parser.*;
import org.emboss.jemboss.programs.ListFile;
import org.emboss.jemboss.gui.sequenceChooser.*;
import org.emboss.jemboss.soap.CallAjax;
import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.JembossParams;

/**
*
* Responsible for displaying the Jemboss application form
* generated from the ACD file. This also handles events related to
* dependent parameters.
*
*/
public class SectionPanel
{

  private MultiTextField multiTextField[];
  /** text field sink    */
  private TextFieldSink textf[];
  /** integer field sink */
  private TextFieldInt textInt[];
  /** float field sink   */
  private TextFieldFloat textFloat[];
  /** range field        */
  private JTextField rangeField[];
  /** boolean fields     */
  private JCheckBox  checkBox[];
  /** input sequence attributes */
  private InputSequenceAttributes inSeqAttr[];
  /** list file panel    */
  private ListFilePanel filelist[];
  /** checkbox popup field for list and select */
  private JembossComboPopup fieldOption[];
  /** multiple option field for list and select */
  private JList multiOption[];
  /** input sequence field */
  private SetInFileCard inSeq[];
  /** field label container */
  private Box lab[];
  /** available databases */
  private String db[];

//private JComboBox graphics;

  /** ACD parser for the application selected */
  private ParseAcd parseAcd;
  /** number of fields in the ACD */
  private int numofFields;
  /** field number to start from */
  private int nf;
  /** sequence length */
  public static int ajaxLength;
  /** sequence weight */
  public static float ajaxWeight;
  /** sequence type   */
  public static boolean ajaxProtein;
  /** true if in client-server mode */
  private boolean withSoap;
  /** application name   */
  private String appName = "";
  /** Jemboss form panel */
  private JPanel p3;
  /** container for this ACD section */
  private JPanel sectionPane;
  /** container for this ACD section */
  private Box sectionBox;
  /** jemboss properties */
  private JembossParams mysettings;

// input, required, advanced & output sections
  /** true if the input section    */
  private boolean isInp = false;
  /** true if the required section */
  private boolean isReq = false;
  /** true if the advanced section */
  private boolean isAdv = false;
  /** true if the additional section */
  private boolean isAdd = false;
  /** true if the output section   */
  private boolean isOut = false;

  /** true if shading disabled fields, otherwise they 
      are removed from the form */
  private boolean isShadedGUI;

  /** label colour */
  public static Color labelColor = new Color(0, 0, 0);
  /** label font   */
  public static Font labfont = new Font("SansSerif", Font.BOLD, 12);  
  /** small label font */
  public static Font labfont2 = new Font("SansSerif", Font.BOLD, 11);

  /** maximum width of the form */
  private final int maxSectionWidth = 498;
  /** Jemboss frame    */
  private JFrame f;
  /** report format    */
  private ReportFormat rf=null;
  /** alignment format */
  private AlignFormat af=null;

  /**
  *
  * @param f 		Jemboss frame
  * @param p3		JPanel ACD form panel
  * @param fieldPane	containing all the fields 
  * @param parseAcd	representing the ACD file to display as a form
  * @param nff		field number
  * @param textf		text fields in the form
  * @param textInt	integer fields in the form
  * @param textFloat	float fields in the form
  * @param rangeField	ranges in the form
  * @param checkBox	for the boolean switches
  * @param inSeqAttr	the input sequence(s) attributes
  * @param fieldOption	for the list/selection data types
  * @param multiOption	for multiple selection lists
  * @param inSeq 		the input sequence(s)
  * @param filelist	list file panel
  * @param db		String array containing the databases
  * @param des		String containing the one line description for the application
  * @param lab		Box for all the component labels
  * @param numofFields	total number of fields
  * @param mysettings	jemboss properties
  * @param withSoap 	true if using a soap server
  *
  */
  protected SectionPanel(JFrame f, JPanel p3, Box fieldPane, 
            ParseAcd parseAcd, int nff, final MultiTextField multiTextField[],
            final TextFieldSink textf[], 
            TextFieldInt textInt[], TextFieldFloat textFloat[],
            JTextField rangeField[], JCheckBox  checkBox[],
            InputSequenceAttributes inSeqAttr[],
            JembossComboPopup fieldOption[], JList multiOption[], 
            SetInFileCard inSeq[], ListFilePanel filelist[], JComboBox graphics,
            String db[], String des, Box lab[], int numofFields,
            JembossParams mysettings, boolean withSoap, String[] envp)
  {

    Border etched = BorderFactory.createEtchedBorder();
    isShadedGUI = AdvancedOptions.prefShadeGUI.isSelected();

    this.p3 = p3;
    this.textf = textf;
    this.textInt = textInt;
    this.textFloat = textFloat;
    this.rangeField = rangeField;
    this.checkBox = checkBox;
    this.inSeqAttr = inSeqAttr;
    this.fieldOption = fieldOption;
    this.multiOption = multiOption;
    this.inSeq = inSeq;
    this.numofFields = numofFields;
    this.db = db;
    this.lab = lab;
    this.parseAcd = parseAcd;
    this.mysettings = mysettings;
    this.withSoap = withSoap; 
    this.f = f;
//  this.graphics = graphics;

//using JNI?
    nf = nff;

    String att = parseAcd.getParameterAttribute(nf,0).toLowerCase();

//set the ACD title
    if(att.startsWith("appl"))
    {
      setAppTitle(des,p3);
      nf++;
      att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
    }
  
    Box section = new Box(BoxLayout.Y_AXIS);
    fieldPane.add(Box.createRigidArea(new Dimension(0,10)));

    JLabel bxlab = new JLabel();
    bxlab.setFont(labfont);

    sectionPane = new JPanel(new GridLayout(1,1));
    sectionPane.setBorder(BorderFactory.createEtchedBorder());
    String nameSect = parseAcd.getParamValueStr(nf, 0);

    if(att.equals("section"))
    {
      TitledBorder title;
      title = BorderFactory.createTitledBorder(etched, 
                parseAcd.getInfoParamValue(nf).toLowerCase(),
                TitledBorder.LEFT,TitledBorder.TOP,
                new Font("SansSerif", Font.BOLD, 13),
                Color.blue);
      sectionPane.setBorder(title);

      String secType = parseAcd.getInfoParamValue(nf).toLowerCase();
      if(secType.startsWith("advanced "))
        isAdv = true;
      else if(secType.startsWith("additional "))
        isAdd = true;
      else if(secType.startsWith("input "))
        isInp = true;
      else if(secType.startsWith("output "))
        isOut = true;
      else if(secType.startsWith("required "))
        isOut = true;
      else
        System.out.println("Unknown section type " + secType);

      nf++;
      att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
    }
    String varName = parseAcd.getParamValueStr(nf, 0);

    int numberOfParameters = 0;

// loop over all the fields in the section or the all fields in the ACD
    while( !( att.equals("endsection")  && varName.equals(nameSect)) &&
            nf < numofFields )
    {
     
      if(!(att.equals("var") || att.equals("variable"))) 
      {
        numberOfParameters++;
        final int h = parseAcd.getGuiHandleNumber(nf);
        Box pan = new Box(BoxLayout.X_AXIS);
        section.add(pan);

        String l = getMinMaxDefault(null,null,null,nf);

      	lab[nf] = new LabelTextBox(parseAcd.getInfoParamValue(nf),
                                  parseAcd.getHelpParamValue(nf));

        if(l != null && !att.startsWith("bool"))
        {
          bxlab = new JLabel(" " + l);
          bxlab.setFont(labfont2);
          bxlab.setForeground(Color.blue);
          lab[nf].add(bxlab);
        }

        if(att.startsWith("appl"))
        {
          setAppTitle(des,p3);
        }
        else if(att.endsWith("graph"))
        {
          Dimension d  = graphics.getPreferredSize();
          d = new Dimension(150,(int)d.getHeight());
          graphics.setMaximumSize(d);
          graphics.setPreferredSize(d);

          pan.add(graphics);
          pan.add(new JLabel(" Graph Format"));
        }
        else if(att.startsWith("int"))
        {
          if(parseAcd.isDefaultParamValueStr(nf)) 
          {
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textInt[h].setValue(Integer.parseInt(
                           parseAcd.getDefaultParamValueStr(nf)));
          }
          else
          {
            Double val = new Double(parseAcd.getDefaultParamValueDbl(nf));
            textInt[h].setValue(val.intValue());
          }
          pan.add(textInt[h]);
          pan.add(lab[nf]);
        } else if(att.startsWith("float")) {
          if(parseAcd.isDefaultParamValueStr(nf))
          {
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textFloat[h].setValue(Double.parseDouble(
                           parseAcd.getDefaultParamValueStr(nf)));
          }
          else
          {
            Double val = new Double(parseAcd.getDefaultParamValueDbl(nf) );
            textFloat[h].setValue(val.doubleValue());
          }
          pan.add(textFloat[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("matrix"))
        {
          if(parseAcd.isDefaultParamValueStr(nf))
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textf[h].setText( parseAcd.getDefaultParamValueStr(nf));
         
          pan.add(textf[h]);
          pan.add(lab[nf]);
  
          Box pan2 = new Box(BoxLayout.X_AXIS);
          section.add(pan2);
          
          JembossComboPopup selectMatrix = new JembossComboPopup(
                                        BuildProgramMenu.getMatrices());
          selectMatrix.addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              JembossComboPopup cb = (JembossComboPopup)e.getSource();
              String matrix = (String)cb.getSelectedItem();
              textf[h].setText(matrix);
            }
          });

          Dimension d = selectMatrix.getPreferredSize();
          d = new Dimension(150,(int)d.getHeight());

          selectMatrix.setMaximumSize(d);
          selectMatrix.setPreferredSize(d);

          pan2.add(selectMatrix);
          pan2.add(Box.createHorizontalGlue());
          
        }
        else if(att.startsWith("codon"))
        {   
          if(parseAcd.isDefaultParamValueStr(nf))
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textf[h].setText( parseAcd.getDefaultParamValueStr(nf));
          pan.add(textf[h]);
          pan.add(lab[nf]);
   
          Box pan2 = new Box(BoxLayout.X_AXIS);
          section.add(pan2);
          JembossComboPopup selectMatrix = new JembossComboPopup(
                                        BuildProgramMenu.getCodonUsage());
          selectMatrix.addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              JembossComboPopup cb = (JembossComboPopup)e.getSource();
              String matrix = (String)cb.getSelectedItem();
              textf[h].setText(matrix);
            }
          });

          Dimension d = selectMatrix.getPreferredSize();
          d = new Dimension(150,(int)d.getHeight());

          selectMatrix.setMaximumSize(d);
          selectMatrix.setPreferredSize(d);

          pan2.add(selectMatrix);
          pan2.add(Box.createHorizontalGlue());
        }
        else if(att.startsWith("dirlist") || att.startsWith("string") ||
                att.startsWith("infile")  || att.startsWith("regexp") ||
                att.startsWith("featout") )
        {
          if(parseAcd.isDefaultParamValueStr(nf)) 
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textf[h].setText( parseAcd.getDefaultParamValueStr(nf));

          pan.add(textf[h]);
          pan.add(lab[nf]);
        }
        else if( att.startsWith("pattern") )
        {
          LabelTextBox labs[] = new LabelTextBox[2];
          
          labs[0] = (LabelTextBox)lab[nf];
          labs[1] = new LabelTextBox("Pattern mismatch",
                                     "Number of pattern mismatch");
          
          multiTextField[h] = new MultiTextField(2, labs);
          pan.add(multiTextField[h].getBoxOfTextFields());
        }
        else if(att.startsWith("seqout"))
        {
          new SetOutFileCard(pan,textf[h],labelColor,sectionPane);
        }
        else if(att.startsWith("outfile") || att.startsWith("datafile"))
        {
          if(parseAcd.isDefaultParamValueStr(nf))
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textf[h].setText( parseAcd.getDefaultParamValueStr(nf));

          if(parseAcd.getInfoParamValue(nf).equals(""))
          {
            bxlab = new JLabel(" " + att + " file name");
            bxlab.setForeground(labelColor);
          }
          lab[nf].add(bxlab);
          pan.add(textf[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("seqset")) 
        {
          inSeq[h] = new SetInFileCard(sectionPane,h,db,
                              "Multiple Sequence Filename",
                              appName,inSeqAttr,true,mysettings);
          pan.add(inSeq[h]);
        }
        else if(att.startsWith("sequence") || att.startsWith("seqall"))
        {
          String tit = parseAcd.getInfoParamValue(nf);
          if(tit.equals(""))
            tit = "Sequence Filename";

          inSeq[h] = new SetInFileCard(sectionPane,h,db,tit,
                                appName,inSeqAttr,true,mysettings);
          pan.add(inSeq[h]);
        }
        else if(att.startsWith("filelist"))
        {
          filelist[h] = new ListFilePanel(15,mysettings);
          pan.add(filelist[h]);
          pan.add(Box.createVerticalStrut(100));
        }
        else if(att.startsWith("range"))
        {
          pan.add(rangeField[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("bool"))
        {
//        checkBox[h] = new JCheckBox();
          if(parseAcd.isDefaultParamValueStr(nf))
            if(parseAcd.getDefaultParamValueStr(nf).equalsIgnoreCase("Y") ||
               parseAcd.getDefaultParamValueStr(nf).equalsIgnoreCase("Yes") )
              checkBox[h].setSelected(true);

          pan.add(checkBox[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("report"))
        {
          // possible report formats
          rf = new ReportFormat(parseAcd,nf);

          pan.add(rf.getComboPopup());
          pan.add(new LabelTextBox(" Report format ("+
                             rf.getDefaultFormat()+")",
                             ReportFormat.getToolTip()));

          section.add(rf.getReportCheckBox());
        }
        else if(att.startsWith("align"))
        {
          af = new AlignFormat(parseAcd,nf);
          pan.add(af.getComboPopup());
          pan.add(new LabelTextBox(" Align format ("+
                           af.getDefaultFormat()+")",
                           af.getToolTip()));
        }
        else if(att.startsWith("list") || att.startsWith("select"))
        {
          String list[];
          if(att.startsWith("list"))
            list = parseAcd.getList(nf);
          else
            list = parseAcd.getSelect(nf);

          double max = 1.;
          if(parseAcd.isMaxParamValue(nf))
            max = Double.parseDouble(parseAcd.getMaxParam(nf));

          if(max > 1.0)
          {
            multiOption[h] = new JList(list);
            multiOption[h].setSelectionMode
               (ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
    
            Vector def = parseAcd.getListOrSelectDefault();

            int selectedIndeces[] = new int[def.size()];
            for(int i=0;i<def.size();i++)
              selectedIndeces[i] = ((Integer)def.get(i)).intValue();

            multiOption[h].setSelectedIndices(selectedIndeces);

            JScrollPane scrollPane = new JScrollPane(multiOption[h]);
            Dimension d = new Dimension(150,100);
            scrollPane.setMinimumSize(d);
            scrollPane.setMaximumSize(d);
            scrollPane.setPreferredSize(d);
            pan.add(scrollPane);
          }
          else
          {
            fieldOption[h] = new JembossComboPopup(list);

            Vector def = parseAcd.getListOrSelectDefault();

            for(int i=0;i<def.size();i++)
              fieldOption[h].setSelectedIndex( ((Integer)def.get(i)).intValue() );

            Dimension d = fieldOption[h].getPreferredSize();
            d = new Dimension(150,(int)d.getHeight());
            
            fieldOption[h].setMaximumSize(d);
            fieldOption[h].setPreferredSize(d);
            pan.add(fieldOption[h]);
          }
          pan.add(lab[nf]);
        }

        pan.add(Box.createHorizontalGlue());
        section.add(Box.createVerticalStrut(10));
      }

//using jni?
      if(AdvancedOptions.prefjni.isSelected())
        checkDependents(section,envp);

      if((att.startsWith("seqset") || att.startsWith("seqall")||
          att.startsWith("sequence")) && !isInp )
      {
        section.add(new Separator(new Dimension(350,10)));
        section.add(Box.createVerticalStrut(10));
      }

      nf++;
      if(nf<numofFields)
      {
        att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
        varName = parseAcd.getParamValueStr(nf, 0);
      }
    }

    nf++;

    if(numberOfParameters >0)  // ensure there are parameters 
    {                          // in this section
      sectionPane.add(section);
      sectionBox = new Box(BoxLayout.X_AXIS);

      sectionResize(sectionPane);
      sectionBox.add(Box.createRigidArea(new Dimension(2,0)));
      sectionBox.add(sectionPane);
      sectionBox.add(Box.createHorizontalGlue());
    }
    
  }

  /** Get the report format */
  protected ReportFormat getReportFormat() { return rf; }

  /**
  *
  * Determine if report formats are used in the EMBOSS program
  * @return	true if a report format is used
  *
  */
  protected boolean isReportFormat()
  {
    if(rf==null)
      return false;
    return true;
  }

  /** Get the alignment format */
  protected AlignFormat getAlignFormat() { return af; }
  
  /**
  *
  * Determine if alignment formats are used in the EMBOSS program
  * @return     true if a alignment format is used
  *
  */
  protected boolean isAlignFormat()
  {
    if(af==null)
      return false;
    return true;
  }

  /** get the panel for this section */
  protected JPanel getSectionPanel() { return sectionPane; }
  /** get the box or this section    */
  protected Box getSectionBox() { return sectionBox; }
  /** @return     true if the input section  */
  protected boolean isInputSection() { return isInp; }
  /** @return     true if the output section */
  protected boolean isOutputSection() { return isOut; }
  /** @return     true if the required section */
  protected boolean isRequiredSection() { return isReq; }
  /** @return     true if the advanced section */
  protected boolean isAdvancedSection() { return isAdv; }
  /** @return     true if the additional section */
  protected boolean isAdditionalSection() { return isAdd; }

  /** 
  *
  * Get the current field 
  * @return 	current field number
  *
  */
  protected int getFieldNum() { return nf; }

  /**
  *
  * Set the application title panel
  * @param des	short description of the program
  * @param p3	JPanel for the ACD form
  *
  */
  private void setAppTitle(String des, JPanel p3)
  {
    appName = parseAcd.getParamValueStr(nf,0).toUpperCase();
    Box bylabP = new Box(BoxLayout.Y_AXIS);
    Box bxlabP = new Box(BoxLayout.X_AXIS);
  
    JScrollPane mainScroller = (JScrollPane)(p3.getParent().getParent().getParent());

    JPanel jTitle = new JPanel(new BorderLayout());
    jTitle.setBackground(Color.white);

    final ApplicationNamePanel namePanel = new ApplicationNamePanel(
                                              appName,10,18,18);
    bxlabP.add(namePanel);
    bxlabP.add(Box.createHorizontalGlue());
    bylabP.add(bxlabP);

    JLabel labP = new JLabel(des);
    labP.setFont(labfont);
    bxlabP = new Box(BoxLayout.X_AXIS);
    bxlabP.add(Box.createHorizontalStrut(10));
    bxlabP.add(labP);
    bxlabP.add(Box.createHorizontalGlue());
    bylabP.add(bxlabP);

    jTitle.add(bylabP, BorderLayout.WEST);
    mainScroller.setColumnHeaderView(jTitle);
  }


  /**
  *
  * Get the min, max and default values for a field in
  * the ACD as a string representation in the form 
  * (min: max: default:)
  * @param min		minimum value for parameter 
  * @param max		maximum value for parameter
  * @param def		default value for parameter
  * @param nfield	field number in ACD
  * @return		string of (min: max: default:) if specified
  *
  */
  private String getMinMaxDefault(String min,String max,
                                 String def, int nfield)
  {
    String l = new String("");

    if(parseAcd.isMinParamValue(nfield) && min == null)
      min = parseAcd.getMinParam(nfield);
    if(parseAcd.isMaxParamValue(nfield) && max == null)
      max = parseAcd.getMaxParam(nfield);
    if(parseAcd.isDefaultParamValueStr(nfield) && def == null)
      def = parseAcd.getDefaultParamValueStr(nfield);

    if(min != null && !min.startsWith("$") 
                   && !min.startsWith("@")) 
    {
      l = l.concat("(min:" + min);
      if(parseAcd.isMaxParamValue(nfield) 
                  && !max.startsWith("$") 
                  && !max.startsWith("@"))
        l = l.concat(" max:" + max);

      if(parseAcd.isDefaultParamValueStr(nfield) 
                         && !def.startsWith("$") 
                         && !def.startsWith("@") 
                         && !def.equals(""))
        l = l.concat(" default:" + def + ") ");
      else
        l = l.concat(") ");
    }
    else if(parseAcd.isMaxParamValue(nfield) 
                     && !max.startsWith("$")
                     && !max.startsWith("@"))
    {
      l = l.concat("(max:" + max);
      if(parseAcd.isDefaultParamValueStr(nfield)
                         && !def.startsWith("$")
                         && !def.startsWith("@") 
                         && !def.equals(""))
        l = l.concat(" default:" + def + ") ");
      else
        l = l.concat(") ");
    }
    else if(parseAcd.isDefaultParamValueStr(nfield) 
                            && !def.startsWith("$")
                            && !def.startsWith("@")
                            && !def.equals(""))
    {
      l = l.concat("(default:" + def + ") ");
    }
    else
    {
      if(parseAcd.isMinParamValue(nfield) || 
         parseAcd.isMaxParamValue(nfield) ||
         parseAcd.isDefaultParamValueStr(nfield) )
        l = "";
      else
        l = null;
    }

    return l;
  }


  /**
  *
  * Checks for dependent variables and adds in action listeners
  * @param section	form container for parameters
  *
  */
  private void checkDependents(Box section, final String[] envp)
  {
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
    final String att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
    final String varName = parseAcd.getParamValueStr(nf,0);
    final String valS = parseAcd.getParamValueStr(nf,0).toLowerCase();
    final int nff = nf;

    if(parseAcd.isDependents(valS,nf,numofFields)) 
    {
      final int h = parseAcd.getGuiHandleNumber(nf);
      final int nod = parseAcd.getNumOfDependents();
      final Dependent dep[] = parseAcd.getDependents();

      if (att.startsWith("seqset") || att.startsWith("seqall")||
          att.startsWith("sequence") ) 
      {
        Box left = new Box(BoxLayout.X_AXIS);
        JButton upload = new JButton("LOAD SEQUENCE ATTRIBUTES");

        upload.setToolTipText(
                "After entering your sequence above, click here. This\n" +
                "will display the input parameters for " + appName + "\n" +
                "that are dependent on the sequence attributes.");

        upload.setForeground(Color.red);
        Dimension d = upload.getPreferredSize();
        upload.setPreferredSize(new Dimension(maxSectionWidth, 
                                         (int)d.getHeight()));
        left.add(upload);
        left.add(Box.createHorizontalGlue());
        section.add(left);
        section.add(Box.createVerticalStrut(10));

        final SetInFileCard sifc = inSeq[h];
        upload.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            f.setCursor(cbusy);
            String fc = null;
            String fname;
            if(sifc.isFileName() || sifc.isListFile())
            {
              if(sifc.isListFile())
                fname = sifc.getSequence(1);
              else
                fname = sifc.getFileChosen();
              fc = AjaxUtil.getFileOrDatabaseForAjax(fname,db,f,withSoap);
            }
            else                                     // Cut-n-Paste
            {
              fc = sifc.getCutNPasteText();

              if(!withSoap)
              {
                String cwd = System.getProperty("user.dir");
                String tmp = null;
                try
                {
                  File tf;
                  try
                  {
                    if(mysettings.isCygwin())
                      tmp = mysettings.getCygwinRoot()+System.getProperty("file.separator")+"tmp";
                    else
                      tmp = System.getProperty("java.io.tmpdir");

                    tf = File.createTempFile("attr", ".jembosstmp", new File(tmp));
                  }
                  catch(IOException ioe)
                  {
                    tf = File.createTempFile("attr", ".jembosstmp",
                                                new File(cwd));
                  }

                  PrintWriter out = new PrintWriter(new FileWriter(tf));
                  out.println(fc);
                  out.close();
                  fc = tf.getCanonicalPath();
                }
                catch (IOException ioe)
                {
                  JOptionPane.showMessageDialog(null,
                       "Cannot write to\n"+
                       tmp+"\n"+
                       "or\n"+
                       cwd,
                       "Problem creating a temporary file!", JOptionPane.ERROR_MESSAGE);
                }
              }
            }

            if(!withSoap && fc!=null)    //Ajax without SOAP
            {
              boolean ok = true;
              Ajax aj = null;
              if(mysettings.isCygwin())
                ok = cygwinSeqAttr(fc,envp,att);
              else
              {
                aj = new Ajax();
                if(att.startsWith("seqset"))
                  ok = aj.seqsetType(fc);
                else
                  ok = aj.seqType(fc);
              }

              if(ok)
              {
                if(!mysettings.isCygwin())
                {
                  ajaxLength  = aj.length;
                  ajaxWeight  = aj.weight;
                  ajaxProtein = aj.protein;
                }
 
                if( (updateBeginEnd(inSeqAttr[h].getBegSeq(),
                                    inSeqAttr[h].getEndSeq())) &&
                    (!att.startsWith("seqset")) &&
                    (!att.startsWith("seqall"))  )
                {
                  inSeqAttr[h].setBegSeq(1);
                  inSeqAttr[h].setEndSeq(ajaxLength);
                }
                resolveDependents(nod,dep,sifc.getFileChosen(),varName);
              }
              else
                JOptionPane.showMessageDialog(sectionPane,
                          "Sequence not found.\n" +
                          "Check the sequence entered.",
                          "Error Message", JOptionPane.ERROR_MESSAGE);

            }
            else if(fc!=null)    //Ajax with SOAP
            {

              try
              {
                CallAjax ca = new CallAjax(fc,att,mysettings);  
                if(ca.getStatus().equals("0") && ca.getLength() > 0)
                {
                  ajaxLength  = ca.getLength();
                  ajaxWeight  = ca.getWeight();
                  ajaxProtein = ca.isProtein();
                  int seqLen  = ca.getLength();
                  if( (updateBeginEnd(inSeqAttr[h].getBegSeq(),
                                    inSeqAttr[h].getEndSeq())) &&
                    (!att.startsWith("seqset")) &&
                    (!att.startsWith("seqall"))  )
                  {
                    inSeqAttr[h].setBegSeq(1);     
                    inSeqAttr[h].setEndSeq(seqLen);
                  }
                  resolveDependents(nod,dep,sifc.getFileChosen(),varName);
                }
                else
                {
                  JOptionPane.showMessageDialog(sectionPane,
                          "Sequence not found." +
                          "\nCheck the sequence entered.",
                          "Error Message", JOptionPane.ERROR_MESSAGE);
                }
//              System.out.println("PROPERTIES::: "+ca.getStatus()+"  "+ajaxLength+" "+ajaxWeight);
              }
              catch (JembossSoapException eae)
              {
                System.out.println("Call to Ajax library failed");
              }
            }
            f.setCursor(cdone);
//          resolveDependents(nod,dep,sifc.getFileChosen(),varName);
          }
        });
      }
      else if(att.startsWith("list") || att.startsWith("select"))
      {
        double max = 1.;
        if(parseAcd.isMaxParamValue(nff))
          max = Double.parseDouble(parseAcd.getMaxParam(nff));

        if(max <= 1.0)
        {
          fieldOption[h].addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              String sel = "";
              int index = fieldOption[h].getSelectedIndex();
              if(att.startsWith("select"))
                sel = new String((new Integer(index+1)).toString());
              else if(att.startsWith("list"))
                sel = new String(parseAcd.getListLabel(nff,index));
              resolveDependents(nod,dep,sel,varName);
            }
          });
        }
      }
      else if(att.startsWith("bool"))
      {
        checkBox[h].addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            String sel = "";
            if(checkBox[h].isSelected())
              sel = new String("true");
            else
              sel = new String("false");
            resolveDependents(nod,dep,sel,varName);
          }
        });
        // resolve dependents as for is loaded in
        String sel = "";
        if(checkBox[h].isSelected())
          sel = new String("true");
        else
          sel = new String("false");
        resolveDependents(nod,dep,sel,varName);
      }
    }

  }


  /**
  *
  * Cygwin uses infoseq to get sequence length and type
  * and uses infoalign to get the sequence weight.  
  *
  */
  private boolean cygwinSeqAttr(String fc, String[] envp, String att)
  {
    String command = mysettings.getEmbossBin().concat(
     "infoseq -only -type -length -nohead -auto "+fc);
    RunEmbossApplication2 rea = new RunEmbossApplication2(command,envp,null);
    rea.waitFor();

    String stdout = rea.getProcessStdout();
    if(stdout.trim().equals(""))
      return false;

    StringTokenizer stok = new StringTokenizer(stdout,"\n ");

    if(stok.nextToken().trim().equalsIgnoreCase("P"))
      ajaxProtein = true;
    else
      ajaxProtein = false;

    ajaxLength = Integer.parseInt(stok.nextToken().trim());

    if(att.startsWith("seqset"))
    {
      command = mysettings.getEmbossBin().concat(
                 "infoalign -only -weight  -nohead -out stdout -auto "+fc);
      rea = new RunEmbossApplication2(command,envp,null);
      rea.waitFor();
      stok = new StringTokenizer(rea.getProcessStdout(),"\n");
      ajaxWeight = 0.f;
      while(stok.hasMoreTokens())
        ajaxWeight+= Float.parseFloat(stok.nextToken());
    }
//  System.out.println(command+"\n"+rea.getProcessStdout());
    return true;
  }

  /**
  *
  * Check sbeg and send values before updating in the
  * panel to avoid overriding user selection
  * @param s	start
  * @param e	end
  * @return 	true to automatically update
  *
  */
  private boolean updateBeginEnd(String s, String e)
  {
    if( ((s!=null) && (!s.equals(""))) ||
        ((e!=null) && (!e.equals(""))) )
    {
      int n = JOptionPane.showConfirmDialog(f,
            "Overwrite the input sequence \n" + 
            "start :" + s + "\n" + "end :" + e + 
            "\n" + "values already set?",
            "Confirm",
            JOptionPane.YES_NO_OPTION);

      if(n == JOptionPane.NO_OPTION)
        return false;
    }

    return true;
  }


  /**
  *
  * Given an ACD variable and the value it takes, resolve any dependent 
  * attribute values.
  * @param nod 		number of dependent values
  * @param dep[] 	dependents
  * @param textVal 	value of variable
  * @param varName 	variable name
  *
  */
  private void resolveDependents(int nod, Dependent dep[], String textVal, 
                                 String varName)
  {
    for(int i=0;i<nod;i++)
    {
      String exp = dep[i].getDependentExp();
      int field = dep[i].getDependentField();
      int param = dep[i].getDependentParam();

      try
      {
        AcdVarResolve avr = new AcdVarResolve(exp,textVal,varName,parseAcd,
                            numofFields,textf,textInt,textFloat,fieldOption,
                            checkBox);

//      System.out.println(exp + "EXP ==> " + avr.getResult() + " " + textVal);
        exp = avr.getResult();
      }
      catch(NullPointerException npe)
      {
        continue;
      }

      AcdFunResolve afr = new AcdFunResolve(exp);
      String result = afr.getResult();
      String att = parseAcd.getParameterAttribute(
                    dep[i].getDependentField(),0).toLowerCase();
      String type = dep[i].getDependentType();
//    System.out.println(varName + " RES => " + result +" "+ type +" att "+ att
//            +" : "+ parseAcd.getParamValueStr(dep[i].getDependentField(),0));
      int h = parseAcd.getGuiHandleNumber(field);


      // ACD changes:
      // required is now standard
      // optional is now additional
      if( (att.equals("list")     || att.equals("select")) && 
          (type.startsWith("stand") || type.startsWith("add")) ) 
//        (type.startsWith("req") || type.startsWith("opt")) ) 
      {
        double max = 1.;
        if(parseAcd.isMaxParamValue(field))
          max = Double.parseDouble(parseAcd.getMaxParam(field));

        if(max <= 1.0)
        {
          if(result.equals("false"))
            setShadingAndVisibility(fieldOption[h], false, field);
          else
            setShadingAndVisibility(fieldOption[h], true , field);
        }
        else
        {
          if(result.equals("false"))
            setShadingAndVisibility(multiOption[h], false, field);
          else
            setShadingAndVisibility(multiOption[h], true, field);
        }

      }
      else if(att.startsWith("datafile")|| att.startsWith("featout")  ||
              att.startsWith("string")  || att.startsWith("seqout") ||
              att.startsWith("outfile") || att.startsWith("matrix") ||
              att.startsWith("infile")  || att.startsWith("regexp") ||
              att.startsWith("codon")   || att.startsWith("dirlist") )
      {

        if( (type.startsWith("add") || type.startsWith("stand")) 
                                    && result.equals("false"))
          setShadingAndVisibility(textf[h], false, field);
        else if ( (type.startsWith("add") || type.startsWith("stand")) 
                                           && result.equals("true"))
          setShadingAndVisibility(textf[h], true, field);

        if(type.startsWith("def"))
        {
          if(att.startsWith("matrix"))
          {
            Vector mat = BuildProgramMenu.getMatrices();
            Enumeration en = mat.elements();
            while(en.hasMoreElements())
            {
              String m = (String)en.nextElement();
              if(result.equalsIgnoreCase(m))
              {
                textf[h].setText(m);
                break;
              }
            }
          }
          else
            textf[h].setText(result);
        }

      }
      else if(att.startsWith("int"))
      {
        if( (type.startsWith("add") || type.startsWith("stand"))
                                    && result.equals("false"))
          setShadingAndVisibility(textInt[h], false, field);
        else if ( (type.startsWith("add") || type.startsWith("stand"))
                                           && result.equals("true"))
          setShadingAndVisibility(textInt[h], true, field);
        
        if(type.startsWith("def"))
          textInt[h].setValue(Integer.parseInt(result));
      }
      else if(att.startsWith("float"))
      {
        if( (type.startsWith("add") || type.startsWith("stand"))
                                    && result.equals("false"))
          setShadingAndVisibility(textFloat[h], false, field);
        else if ( (type.startsWith("add") || type.startsWith("stand"))
                                           && result.equals("true"))
          setShadingAndVisibility(textFloat[h], true, field);
        
        if(type.startsWith("def"))
          textFloat[h].setValue(Double.parseDouble(result));
      }
      else if(att.startsWith("bool"))
      {
        if(type.startsWith("add") || type.startsWith("stand")) 
        {
          if(result.equals("false"))
            setShadingAndVisibility(checkBox[h], false, field);
          else 
            setShadingAndVisibility(checkBox[h], true, field);
        }
        else if(type.startsWith("def"))
        {
          if(result.equals("false"))
            checkBox[h].setSelected(false);
          else
            checkBox[h].setSelected(true);
        }
      }
                                              //n.b. no default labels on bools
      if(type.startsWith("def") && (!att.startsWith("bool"))) 
      {
        String l = getMinMaxDefault(null,null,result,field);
        if(lab[field] != null)
          ((JLabel)lab[field].getComponent(1)).setText(" " + l);
      }
      else if(type.startsWith("min"))
      {
        String l = getMinMaxDefault(result,null,null,field);
        if(lab[field] != null)
          ((JLabel)lab[field].getComponent(1)).setText(" " + l);
      }
      else if(type.startsWith("max"))
      {
        String l = getMinMaxDefault(null,result,null,field);  
        if(lab[field] != null)
          ((JLabel)lab[field].getComponent(1)).setText(" " + l);
      }

    }

// use to resize sections
//
    if(!isShadedGUI)
    {
      sectionResize(BuildJembossForm.addSection);
      sectionResize(BuildJembossForm.advSection);
      sectionResize(BuildJembossForm.reqdSection);
      sectionResize(BuildJembossForm.outSection);
    }

    p3.setVisible(false);  //this seems to be necessary to force 
    p3.setVisible(true);   //it to re-display sections properly!!

  }

  /**
  *
  * Set the shading or visibility of a component
  * @param c		component 
  * @param useThis	true if enabled/visible
  * @param field	field number in the ACD
  *
  */
  private void setShadingAndVisibility(Component c, 
                        boolean useThis, int field)
  {

    if( c != null)
    {
      if(isShadedGUI)
      {
        c.setEnabled(useThis);
      }
      else
      {
        c.setVisible(useThis);
        if(lab[field] != null)
          lab[field].setVisible(useThis);
      }
    }
  }

  /**
  *
  * Resize the section panel
  * @param p 	section panel
  *
  */
  private void sectionResize(JPanel p)
  {
    if(p != null)
    {
      Dimension min = p.getMinimumSize();
      int w = maxSectionWidth;
      int h = (int)min.getHeight();
      Dimension d = new Dimension (w,h);
      p.setMaximumSize(d);
      p.setPreferredSize(d);
      p.setVisible(false);
      p.setVisible(true);
    }
  }

}

