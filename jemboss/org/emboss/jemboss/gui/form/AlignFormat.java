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
import javax.swing.JCheckBox;

/**
*
* Handles emboss alignment formats
* http://www.hgmp.mrc.ac.uk/Software/EMBOSS/Themes/AlignFormats.html
*
*/
public class AlignFormat
{

  /** EMBOSS alignment formats */
  private static String align_msf[] = 
            {"unknown", "multiple", "simple", 
             "fasta", "msf", "srs"};

  /** EMBOSS pairwise alignment formats */
  private static String align_pairwise[] =
            {"unknown", "multiple", "simple",
             "fasta", "msf", "srs",
             "pair", "markx0", "markx1",
             "markx2", "markx3", "markx10",
             "srspair", "score"};

  /** component to display formats */
  private JembossComboPopup cp;
  /** default format */
  private String def;
  /** true if pairwise alignment format */
  private boolean lpair;

  /**
  *
  * @param parseAcd	ACD parser for the application
  * @param nf		field number in ACD
  *
  */
  public AlignFormat(ParseAcd parseAcd, int nf) 
  {
    lpair = isPairWise(parseAcd,nf);
    cp = new JembossComboPopup(getAlignFormats(lpair));

    int np = parseAcd.getNumofParams(nf);

    for(int i=0;i<np;i++)
      if(parseAcd.getParameterAttribute(nf,i).equals("aformat"))
      {
        def = parseAcd.getParamValueStr(nf,i).toLowerCase();
        cp.setSelectedItem(def);
      }
 
    Dimension d = cp.getPreferredSize();
    d = new Dimension(150,(int)d.getHeight());
    
    cp.setMaximumSize(d);
    cp.setPreferredSize(d);
  }


  /**
  *
  * @param def		default format
  * @param lpair	true if pairwise alignment format
  *
  */
  public AlignFormat(String def, boolean lpair)
  {
    this.def   = def;
    this.lpair = lpair;
    cp = new JembossComboPopup(getAlignFormats(lpair));
    cp.setSelectedItem(def);
                                                                                
    Dimension d = cp.getPreferredSize();
    d = new Dimension(150,(int)d.getHeight());
                                                                                
    cp.setMaximumSize(d);
    cp.setPreferredSize(d);
  }


  /**
  *
  * Get the combo-popup component for alignment formats
  * @return 	combo-popup component for alignment formats
  *
  */
  public JembossComboPopup getComboPopup()
  {
    return cp;
  }

  /**
  *
  * @return def		default alignment format
  *
  */
  public String getDefaultFormat()
  {
    return def;
  }

  /**
  *
  * Get the alignment formats as a string array
  * @param lpair	true if pairwise alignment
  * @return 		alignment formats as a string array
  *
  */
  private String[] getAlignFormats(boolean lpair)
  {
    if(lpair)
      return align_pairwise;
    return align_msf;
  }

  /**
  *
  * Get the alignment format to add to the EMBOSS
  * command line
  * @return 	alignment format for EMBOSS command line
  * 
  */
  public String getAlignFormat()
  {
    String report = " -aformat " + cp.getSelectedItem();
    return report;
  }

  /**
  *
  * Method to determine whether this is a pairwise (returning
  * true) or multiple sequence alignment format.
  * @param parseAcd     ACD parser for the application
  * @param nf           field number in ACD
  * @return true if a pairwise alignment
  *
  */
  private boolean isPairWise(ParseAcd parseAcd, int nf)
  {
    for(int i=0; i<nf;i++)
      if(parseAcd.getParameterAttribute(i,0).equalsIgnoreCase("seqset"))
        return false;

    return true;
  }

  /**
  *
  * @return String report of the available report formats
  *
  */
  public String getToolTip()
  {
    String ls = PlafMacros.getLineSeparator();
    String commonFormat =
        "unknown - synonym for simple format."+ls+
        "multiple - synonym for simple format."+ls+
        "simple - displays the sequence names and positions and"+ls+
        "puts the markup line underneath the sequences."+ls+
        "fasta - Fasta sequence format with gaps"+ls+
        "msf - MSF sequence format."+ls+
        "srs - shows the sequence ID name and position."+ls;

    if(lpair)
      return commonFormat + ls +
        "pair - default format used with only 2 sequences."+ls+
        "markx0 - default output format for FASTA programs."+ls+
        "markx1 - alternative format for FASTA programs in which"+ls+
        "identities are not marked. Conservative replacements are"+ls+
        "denoted by \'x\' and non-conservative substitutions by \'X\'."+ls+
        "markx2 - alternative format for FASTA programs in which"+ls+
        "residues in the second sequence are only shown if they"+ls+
        "are different from the first."+ls+
        "markx3 - alternative format for FASTA programs in which"+ls+
        "the sequences are displayed in FASTA format. These can"+ls+
        "be used to build a primitive multiple alignment."+ls+
        "markx10 - alternative format for FASTA programs in which"+ls+
        "the sequences are displayed in FASTA format and the"+ls+
        "sequence length, alignment start and stop is given in"+ls+
        "in lines starting with a semi colon after the title line"+ls+
        "srspair - similar in style to pair format."+ls+
        "score - does not display the sequence alignment. Only"+ls+
        "shows names of the sequences, length of the alignment and"+ls+ 
        "the score in brackets.";

    return commonFormat;
  }

}

