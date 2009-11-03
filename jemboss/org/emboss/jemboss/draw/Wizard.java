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
                                                                                
                                                                                
package org.emboss.jemboss.draw;

import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.util.Vector;
import java.util.Hashtable;
import org.emboss.jemboss.gui.ScrollPanel;


/**
*
* DNA draw wizard 
*
*/
public class Wizard
{

  private DNADraw dna = null;

  public Wizard(DNADraw dna_current)
  {
    int n = getOption(dna_current);  // option 0 - read data file
                                     // option 1 - create dna display
                                     // option 2 - edit existing dna
    if(n == 0)
    {
      Vector restrictionEnzyme = new Vector();
      EmbossCirdnaReader dnaRead = new EmbossCirdnaReader();

      Hashtable lineAttr = new Hashtable();
      lineAttr.put("lsize",new Integer(5));
      lineAttr.put("circular",new Boolean(true));
      lineAttr.put("start",new Integer(dnaRead.getStart()));
      lineAttr.put("end",new Integer(dnaRead.getEnd()));

      dna = new DNADraw(dnaRead.getBlock(),
                        dnaRead.getRestrictionEnzyme(),
                        lineAttr,0,100,100);
    }
    else if(n == 1 || n == 2)
    {
      JFrame f = new JFrame("Genetic Feature");
   
      Vector block = new Vector();
      Vector restrictionEnzyme = new Vector();
      if(dna_current == null)
        dna = new DNADraw();
      else
      {
        dna = dna_current;
        block = dna_current.getGeneticMarker();
        restrictionEnzyme = dna_current.getRestrictionEnzyme();
      }

      LineAttribute la = new LineAttribute(dna);

      GeneticMarker gm;
      if(dna_current != null)
        gm = new GeneticMarker(dna_current,block);
      else
        gm = new GeneticMarker(dna,block);

      RestrictionEnzyme re;
      if(dna_current != null)
        re = new RestrictionEnzyme(dna_current,restrictionEnzyme);
      else
        re = new RestrictionEnzyme(dna,restrictionEnzyme);

      Ticks tk = new Ticks(dna_current,false);

      la.setMinimumSize(la.getPreferredSize());
      la.setMaximumSize(la.getPreferredSize());

      re.setMinimumSize(re.getPreferredSize());
      re.setMaximumSize(re.getPreferredSize());

      ScrollPanel pane = new ScrollPanel(new BorderLayout());
      Box bdown = Box.createVerticalBox();
      bdown.add(new JLabel("DNA Attributes"));
      Box bacross = Box.createHorizontalBox();
      bacross.add(la);
      bacross.add(tk);
      bacross.add(Box.createHorizontalGlue());
      bdown.add(bacross);

      bdown.add(Box.createHorizontalStrut(10));
      bdown.add(new JLabel("Genetic Feature"));
      bacross = Box.createHorizontalBox();
      bacross.add(gm);
      bacross.add(Box.createHorizontalGlue());
      bdown.add(bacross);

      bdown.add(Box.createHorizontalStrut(10));
      bdown.add(new JLabel("Restriction Enzymes"));
      bacross = Box.createHorizontalBox();
      bacross.add(re);
      bacross.add(Box.createHorizontalGlue());
      bdown.add(bacross);
      pane.add(bdown,BorderLayout.CENTER);
    
      JScrollPane createWizScroll = new JScrollPane(pane);
    
      Dimension dscreen = f.getToolkit().getScreenSize();
      int wid = (int)dscreen.getWidth();
      if(wid > 700)
        wid = 700;

      int hgt = (int)dscreen.getHeight();
      if(hgt > 750)
        hgt = 700;
      hgt-=50;

      Dimension d = new Dimension(wid,hgt); 
      createWizScroll.setPreferredSize(d);

      JOptionPane.showMessageDialog(null,
                      createWizScroll, "DNA Wizard",
                      JOptionPane.PLAIN_MESSAGE);

      dna.setGeneticMarker(block);
      dna.setRestrictionEnzyme(restrictionEnzyme);
      dna.setLineAttributes(la.getLineAttr());
      dna.setStartTick(tk.getStartTick());
      dna.setMinorTickInterval(tk.getMinorTickInterval());
      dna.setTickInterval(tk.getTickInterval());

      int s = la.getStart();
      dna.setStart(s);
  
      s = la.getEnd();
      dna.setEnd(s);

    }
  }


  public DNADraw getDNADraw()
  {
    return dna;
  }


  private int getOption(DNADraw dna_current)
  {
    Box bdown = Box.createVerticalBox();

    JRadioButton[] radioButtons;

    if(dna_current !=  null)
      radioButtons = new JRadioButton[3];
    else
      radioButtons = new JRadioButton[2];
    final ButtonGroup group = new ButtonGroup();
    radioButtons[0] = new JRadioButton("Read in data file");
    group.add(radioButtons[0]);
    radioButtons[1] = new JRadioButton("Create new dna display");
    group.add(radioButtons[1]);
    radioButtons[1].setSelected(true);
    bdown.add(radioButtons[0]);
    bdown.add(radioButtons[1]);

    if(dna_current !=  null)
    {
      radioButtons[2] = new JRadioButton("Edit current dna display");
      group.add(radioButtons[2]);
      radioButtons[2].setSelected(true);
      bdown.add(radioButtons[2]);
    }

    JPanel pane = new JPanel(new BorderLayout());
    pane.add(bdown);
    JOptionPane.showMessageDialog(null, 
                      pane, "Jemboss DNA Viewer Wizard", 
                      JOptionPane.QUESTION_MESSAGE);

    if(radioButtons[0].isSelected())
      return 0;
    else if(radioButtons[1].isSelected())
      return 1;
    else if(radioButtons[2].isSelected())
      return 2;
     
    return 1;
  }

}

