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
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.File;

public class AlignApplet extends JApplet 
{
  private GraphicSequenceCollection gsc;
  private JScrollPane jspSequence;
  private Vector seqs;
  private String matrix;
  //private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  //private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

  public void init()
  {

    matrix = "/packages/emboss_dev/tcarver/emboss/emboss/emboss/data/EBLOSUM80";
    matrix = "/packages/emboss_dev/tcarver/emboss/emboss/emboss/data/EDNAFULL";


    seqs = new Vector();
    seqs.add(new Sequence("Seq1","aaaaaaaaaaaaaaaaaaaaTAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Seq2","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Seq3","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Sequence4","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Seq1","aaaaaaaaaaaaaaaaaaaaTAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Seq2","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Seq3","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
    seqs.add(new Sequence("Sequence4","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));

    jspSequence = new JScrollPane();

    gsc = new GraphicSequenceCollection(seqs,jspSequence,
                                        true,true,true,true, null);
    jspSequence.setViewportView(gsc);

// set up a menu bar
    setJMenuBar(createMenu());

// set size of sequence panel
    Dimension d = new Dimension(600, gsc.getPanelHeight()+80);
    Dimension dsequence = gsc.getViewSize();

    gsc.setMinimumSize(dsequence);
    gsc.setPreferredSize(dsequence);
    jspSequence.setPreferredSize(d);

    JPanel seqNamePanel = new JPanel(new BorderLayout());
    seqNamePanel.add(jspSequence,BorderLayout.CENTER);
    JScrollPane jspMain = new JScrollPane(seqNamePanel,
                                   JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                   JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

    getContentPane().add(jspMain,BorderLayout.CENTER);
  }

  public JMenuBar createMenu()
  {
    JMenuBar menuBar = new JMenuBar();

    JMenu fileMenu = new JMenu("File");
    menuBar.add(fileMenu);

// open sequence file
    final JMenuItem calculateCons = new JMenuItem("Consensus");
    final JMenuItem calculatePlotCon = new JMenuItem("Consensus plot");
    JMenuItem openSequence = new JMenuItem("Open...");
    openSequence.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        SequenceReader sr = new SequenceReader();

        if(sr.isReading())
        {
          gsc = new GraphicSequenceCollection(sr.getSequenceVector(),
                                              jspSequence,
                                              true,true,true,true, null);
          jspSequence.setViewportView(gsc);
          calculateCons.setText("Calculate consensus");
          calculatePlotCon.setText("Calculate Consensus plot");
        }
      }
    });
    fileMenu.add(openSequence);


    //print
    JMenuItem print = new JMenuItem("Print");
    print.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new PrintAlignment(gsc);
      }
    });
    fileMenu.add(print);


    JMenu viewMenu = new JMenu("View");
    viewMenu.setMnemonic(KeyEvent.VK_V);
    menuBar.add(viewMenu);
    JMenuItem taylor = new JMenuItem("Taylor Colour");
    viewMenu.add(taylor);
    taylor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.taylorColor);
        jspSequence.setViewportView(gsc);
      }
    });

    JMenuItem nuc = new JMenuItem("Nucleotide Colour");
    viewMenu.add(nuc);
    nuc.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.baseColor);
        jspSequence.setViewportView(gsc);
      }
    });

    viewMenu.add(new JSeparator());

    final JCheckBoxMenuItem drawBoxes = new JCheckBoxMenuItem("Draw boxes",true);
    drawBoxes.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setDrawBoxes(drawBoxes.isSelected());
      }
    });
    viewMenu.add(drawBoxes);

    final JCheckBoxMenuItem drawColorBox = 
            new JCheckBoxMenuItem("Colour boxes",true);
    drawColorBox.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setDrawColor(drawColorBox.isSelected());
      }
    });
    viewMenu.add(drawColorBox);

// options
    JMenu optionsMenu = new JMenu("Options");
    optionsMenu.setMnemonic(KeyEvent.VK_O);
    menuBar.add(optionsMenu);

// calculate
    JMenu calculateMenu = new JMenu("Calculate");
    menuBar.add(calculateMenu);
    calculateCons.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.deleteSequence(Consensus.DEFAULT_SEQUENCE_NAME);
        Consensus conseq = new Consensus(new File(matrix),
              gsc.getSequenceCollection(),1.f,1.f,1);
        gsc.addSequence(conseq.getConsensusSequence(),true,5, 1);
        calculateCons.setText("Recalculate consensus");
      }
    });
    calculateMenu.add(calculateCons);

    calculatePlotCon.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.showConsensusPlot(new File(matrix),1);
      }
    });
    calculateMenu.add(calculatePlotCon);


    String sizes[] = {"10", "12", "14", "16", "18"};
    final JComboBox fntSize = new JComboBox(sizes);
    fntSize.setPreferredSize(fntSize.getMinimumSize());
    fntSize.setMaximumSize(fntSize.getMinimumSize());
    fntSize.setSelectedItem(Integer.toString(gsc.getFontSize()));
    menuBar.add(fntSize);
    fntSize.setEditable(true);
    fntSize.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        String fsize = (String)fntSize.getSelectedItem();
        gsc.setFontSizeForCollection(Integer.parseInt(fsize));
        jspSequence.setViewportView(gsc);
      }
    });
    return menuBar;
  }
  public void start() 
  {
  }
}

