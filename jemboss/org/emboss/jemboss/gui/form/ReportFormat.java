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

import org.emboss.jemboss.parser.ParseAcd;
import java.awt.Dimension;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JCheckBox;
import javax.swing.Box;
import javax.swing.BoxLayout;

/**
*
* EMBOSS Report formats
* http://www.hgmp.mrc.ac.uk/Software/EMBOSS/Themes/ReportFormats.html
*
*/
public class ReportFormat
{

  /** EMBOSS Report formats */
  private static String rpt[] = 
            {"embl", "genbank", "gff", "pir",
             "swiss", "listfile", "dbmotif",
             "diffseq", "excel", "feattable",
             "motif", "nametable", "regions",
             "seqtable", "simple", "srs",
             "table", "tagseq", "dasgff"};

  /** container for report formats */
  private JembossComboPopup cp;
  /** select to show the accession number */
  private JCheckBox raccshow = new JCheckBox();
  /** select to show the description */
  private JCheckBox rdesshow = new JCheckBox();
  /** select to show the USA */
  private JCheckBox rusashow = new JCheckBox();
  /** default report format */
  private String def;

  /**
  *
  * @param parseAcd	ACD parser for an emboss application
  * @param nf		report format field number in the ACD
  * 
  */
  public ReportFormat(ParseAcd parseAcd, int nf)
  {
    cp = new JembossComboPopup(getReportFormats());

    int np = parseAcd.getNumofParams(nf);

    for(int i=0;i<np;i++)
      if(parseAcd.getParameterAttribute(nf,i).equals("rformat"))
      {
        def = parseAcd.getParamValueStr(nf,i);
        cp.setSelectedItem(parseAcd.getParamValueStr(nf,i));
      }
 
    Dimension d = cp.getPreferredSize();
    d = new Dimension(150,(int)d.getHeight());
    
    cp.setMaximumSize(d);
    cp.setPreferredSize(d);

  }

  /**
  *
  * Get the combo popup box containing the report formats
  * @return 	combo popup box 
  *
  */
  public JembossComboPopup getComboPopup()
  {
    return cp;
  }

  /**
  *
  * Get the default report format
  * @return 	default report format
  *
  */
  public String getDefaultFormat()
  {
    return def;
  }

  /**
  *
  * Get the check box for accession number display 
  * @return	check box
  *
  */
  public JCheckBox getAccCheckBox()
  {
    return raccshow;
  }

  /**
  *
  * Get the check box for description display 
  * @return     check box
  *
  */
  public JCheckBox getDesCheckBox()
  {
    return rdesshow;
  }

  /**
  *
  * Get the check box for USA display 
  * @return     check box
  *
  */
  public JCheckBox getUsaDesCheckBox()
  {
    return rusashow;
  }

  
  /**
  *
  * Get the available report formats
  * @return	string array of report formats
  *
  */
  private static String[] getReportFormats()
  {
    return rpt;
  }

  /**
  *
  * Returns EMBOSS command line options for report formats
  * based on what has been selected
  * @return	EMBOSS command line options as string
  *
  */
  public String getReportFormat()
  {
    String report = " -rformat " + cp.getSelectedItem();
    if(raccshow.isSelected())
      report = report.concat(" -raccshow ");
    if(rdesshow.isSelected())
      report = report.concat(" -rdesshow ");
    if(rusashow.isSelected())
      report = report.concat(" -rusashow ");
    return report;
  }

  /**
   * Returns EMBOSS command line options for report formats
   * based on what has been selected
   * @return	EMBOSS command line options as string array
   */
  public List getReportFormatA()
  {
	  List ret = new ArrayList();
	  ret.add("-rformat");
	  ret.add(cp.getSelectedItem());
	  if(raccshow.isSelected())
		  ret.add("-raccshow");
	  if(rdesshow.isSelected())
		  ret.add("-rdesshow");
	  if(rusashow.isSelected())
		  ret.add("-rusashow");
	  return ret;
  }

  /**
  *
  * Get the container box for the report format
  * display on the Jemboss application form
  * @return	container box for the report format
  *
  */
  protected Box getReportCheckBox()
  {
    Box pan = new Box(BoxLayout.X_AXIS);
    // -raccshow  show accession
    pan.add(getAccCheckBox());
    pan.add(new LabelTextBox("Accession number",
        "Displays the accession number in the report"));
    pan.add(Box.createHorizontalStrut(20));

    // -rdesshow  show description
    pan.add(getDesCheckBox());
    pan.add(new LabelTextBox("Description",
        "Displays the sequence description in the report"));
    pan.add(Box.createHorizontalStrut(20));

    // -rusashow  show the full USA
    pan.add(getUsaDesCheckBox());
    pan.add(new LabelTextBox("Full USA",
        "Displays the universal sequence address in the report"));

    pan.add(Box.createHorizontalGlue());
    return pan;

  }

  /**
  *
  * @return String report of the available report formats
  *
  */
  public static String getToolTip()
  {
    String ls = PlafMacros.getLineSeparator();
    return "embl \t\t\t\t\t\t\t\t- EMBL feature table format."+ls+
           "genbank \t- Genbank feature table format."+ls+
           "gff \t\t\t\t\t\t\t\t\t\t\t\t- GFF feature table format."+ls+
           "pir \t\t\t\t\t\t\t\t\t\t\t\t- PIR feature table format."+ls+
           "swiss \t\t\t\t\t\t\t- SwissProt feature table format."+ls+
           "listfile \t\t\t\t\t\t- writes out a list file with start and end"+
           "points of the motifs given"+ls+"by '[start:end]' after the"+
           "sequence's full USA. This can be read by other EMBOSS"+ls+
           "programs using '@' or 'list::' before the filename."+ls+
           "dbmotif \t\t\t\t- DbMotif format."+ls+
           "diffseq \t\t\t\t\t- Useful when reporting the results of two sequences"+ls+
           "aligned, as in the program diffseq."+ls+
           "excel \t\t\t\t\t\t\t\t- TAB-delimited table format for spread-sheets"+ls+
           "feattable \t\t- FeatTable format. It is an EMBL feature table"+ls+
           "table using only the tags in the report definition."+ls+
           "motif \t\t\t\t\t\t\t\t- Motif format. Based on the original"+ls+
           "output format of antigenic, helixturnhelix and sigcleave."+ ls+
           "regions \t\t\t- Regions format."+ls+
           "seqtable \t\t- Simple table format that includes the feature sequence."+ls+
           "simple \t\t\t\t\t- SRS simple format."+ls+
           "srs \t\t\t\t\t\t\t\t\t\t\t- SRS format."+ls+
           "table \t\t\t\t\t\t\t- Table format."+ls+
           "tagseq \t\t\t\t-  Tagseq format. Features are marked up below the sequence.";
  }

}

