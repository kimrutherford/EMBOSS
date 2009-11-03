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
import java.awt.print.*;
import java.util.*;
import java.io.File;

import org.emboss.jemboss.gui.form.MultiLineToolTipUI;

/**
*  
* This class can be used to get a grapical representation
* of a collection of sequences.
*
*/
public class GraphicSequenceCollection extends JPanel
                                       implements Printable, Scrollable
{

  /** Vector of sequences removed from panel */
  private Vector removedSeqs = new Vector();
  /** Vector containing Sequence objects     */
  protected Vector seqs;             
  /** Vector containing graphical sequences  */
  protected Vector graphicSequence;  
  /** Vector containing graphical names of sequences */
  protected Vector graphicName;      
  /** Colour scheme to use                   */
  protected Hashtable colorScheme;   
  /** Consensus plot  */
  private PlotConsensus pc = null; 
  private int hgt;
  private int len;
  /** longest sequence length */ 
  protected int MAXSEQLENGTH = 0;
  /** number of residues on each line for print */
  private int numResiduePerLine = 0;   
  /** panel for sequence numbers */
  protected SequenceJPanel numberDraw;
  /** sequence scrollpane */
  protected JScrollPane jspSequence; 
  /** sequence name panel */ 
  private JPanel seqNamePanel;
  /** container for sequences */
  protected Box seqBox;
  /** container for sequence names */
  protected Box seqNameBox;
  /** container for consensus plot */
  private Box plotconsSeqBox = null;
  /** draw the sequence */
  protected boolean drawSequence;
  /** draw black square around residues */
  protected boolean drawBlackBox;
  /** colour residues   */
  protected boolean drawColorBox;
  /** draw sequence position number */
  protected boolean drawNumber;
  /** draw as per prettplot */
  //private boolean prettPlot = false;
  /** gap between sequences and consensus plot */
  private int plotConStrut = 20;
  /** pretty plot values */
  private PrettyPlotJFrame prettyPlot;
  /** scoring matrix */
  private Matrix mat;

  /**
  *
  * @param seqs		vector of sequences
  * @param colorScheme	sequence colour scheme
  * @param jspSequence 	sequence scrollpane
  * @param drawSequence true to draw the sequence
  * @param drawBlackBox true to draw black square around residues
  * @param drawColorBox true to colour residues
  * @param drawNumber   true to draw sequence position number
  * @param statusField	status field in the editor
  *
  */
  public GraphicSequenceCollection(Vector seqs, Hashtable colorScheme,
                         JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    super(new BorderLayout());
    this.seqs = seqs;
    this.colorScheme = colorScheme;
    this.jspSequence = jspSequence;
    this.drawSequence = drawSequence;
    this.drawBlackBox = drawBlackBox;
    this.drawColorBox = drawColorBox;
    this.drawNumber = drawNumber;

    jspSequence.getViewport().setBackground(Color.white);
    setBackground(Color.white);
    MultiLineToolTipUI.initialize();
    graphicSequence = new Vector();
    graphicName = new Vector();

// find maximum seq length
    setMaxSeqLength();

    Box centerBox = new Box(BoxLayout.Y_AXIS);
    seqBox = new Box(BoxLayout.Y_AXIS);
    centerBox.add(seqBox);

    Box westBox = new Box(BoxLayout.Y_AXIS);
    seqNameBox = new Box(BoxLayout.Y_AXIS);
    westBox.add(seqNameBox);
    seqNamePanel = new JPanel(new BorderLayout());
    seqNamePanel.add(westBox,BorderLayout.CENTER);
    seqNamePanel.setBackground(Color.white);
    jspSequence.setRowHeaderView(seqNamePanel);

// draw residue/base numbering
    if(drawNumber)
      drawNumber();

// draw names and sequences 
    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
      addSequence((Sequence)enumer.nextElement(),false,0,0);

    westBox.add(Box.createVerticalGlue());
    centerBox.add(Box.createVerticalGlue());
    plotconsSeqBox = new Box(BoxLayout.Y_AXIS);
    centerBox.add(plotconsSeqBox);
    add(centerBox,BorderLayout.CENTER);
    
    int xfill = getNameWidth();
    seqNamePanel.setPreferredSize(new Dimension(xfill,2000));
  }
 

  /**
  *
  * @param seqs         vector of sequences
  * @param jspSequence  sequence scrollpane
  * @param drawSequence true to draw the sequence
  * @param drawBlackBox true to draw black square around residues
  * @param drawColorBox true to colour residues
  * @param drawNumber   true to draw sequence position number
  * @param statusField  status field in the editor
  *
  */
  public GraphicSequenceCollection(Vector seqs, JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    this(seqs,null,jspSequence,drawSequence,
         drawBlackBox,drawColorBox,drawNumber,statusField);
  }

  /**
  *
  * Get the vector of Sequences
  * @return 	vector of Sequences
  *
  */
  protected Vector getSequenceCollection()
  {
    return seqs;
  }


  /**
  *
  * Get the consensus sequence
  * @return     consensus sequence
  *
  */
  protected Sequence getConsensusSequence()
  {
    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {
      Sequence s = (Sequence)enumer.nextElement();
      String name = s.getName();
      if(name.equals(Consensus.DEFAULT_SEQUENCE_NAME))
        return s;
    }
    return null;
  }


  /**
  *
  * Get the number of sequences
  * @return     vector of Sequences
  *
  */
  protected int getNumberSequences()
  {
    return seqs.size();
  }


  /**
  *
  * Get the position of the sequence JViewPort 
  * @return 	position of the sequence JViewPort
  *
  */
  protected Point getViewPosition() 
  {
    return jspSequence.getViewport().getViewPosition();
  }

  /**
  *
  * Get the Rectangle being displayed by the sequence JViewPort
  * @return     rectangle being displayed by the sequence JViewPort
  *
  */
  protected Rectangle getViewRect()
  {
    Rectangle r = jspSequence.getViewport().getViewRect();

// adjustment for the sequence names on the west
//  r.x = r.x - westBox.getWidth();
//  if(r.x < 0)
//    r.x = 0;
    return r;
  }

  /**
  *
  * Calculate and display the consensus plot
  * @param File matrix - scoring matrix
  * @param int wsize window size to average scores over
  *
  */
  protected void showConsensusPlot(File matrix, int wsize)
  {
    deleteConsensusPlot();
    SequenceJPanel sj = (SequenceJPanel)graphicSequence.get(0);
    int interval = sj.getSequenceResidueWidth();

    pc =  new PlotConsensus(matrix,seqs,wsize,interval,this);
    pc.setBackground(Color.white);
    
    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(pc);
    XBox.add(Box.createHorizontalGlue());
    plotconsSeqBox.add(Box.createVerticalStrut(plotConStrut));
    plotconsSeqBox.add(XBox);
    plotconsSeqBox.add(Box.createVerticalGlue());
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);
    setJScrollPaneViewportView();
  }

  /**
  *
  * Calculate and display the consensus plot
  * @param File matrix - scoring matrix
  * @param int wsize window size to average scores over
  *
  */
  protected void showConsensusPlot(Matrix mat, int wsize)
  {
    deleteConsensusPlot();
    SequenceJPanel sj = (SequenceJPanel)graphicSequence.get(0);
    int interval = sj.getSequenceResidueWidth();

    Vector seqs_ = new Vector(seqs);
    Sequence seq = (Sequence)seqs.lastElement();
    if (seq.getName().equals(Consensus.DEFAULT_SEQUENCE_NAME))
       seqs_.removeElementAt(seqs.size()-1);
    pc =  new PlotConsensus(mat,seqs_,wsize,interval,this);
    pc.setBackground(Color.white);
   
    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(pc);
    XBox.add(Box.createHorizontalGlue());
    plotconsSeqBox.add(Box.createVerticalStrut(20));
    plotconsSeqBox.add(XBox);
    plotconsSeqBox.add(Box.createVerticalGlue());
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);
    setJScrollPaneViewportView();
  }

  /**
  *
  * Remove the consensus plot from the sequence editor
  *
  */
  protected void deleteConsensusPlot()
  {
    plotconsSeqBox.removeAll();
  }

  /**
  *
  * Set the longest sequence length
  * @param max 	longest sequence length
  *
  */
  protected void setMaxSequenceLength(int max)
  {
    if(max > MAXSEQLENGTH)
      MAXSEQLENGTH = max;
  }

  /**
  *
  * Calculate the longest sequence length
  *
  */
  protected void setMaxSeqLength()
  {
    MAXSEQLENGTH = 0;
    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {
      Sequence seq = (Sequence)(enumer.nextElement());
      if(seq.getSequence().length()>MAXSEQLENGTH)
        MAXSEQLENGTH = seq.getSequence().length();
    }
  }

  /**
  *
  * Get the longest sequence length
  * @return 	longest sequence length
  *
  */
  public int getMaxSeqLength()
  {
    return MAXSEQLENGTH;
  }

  /**
  *
  * Lock/group the sequences
  * @param 	true to lock sequences
  *
  */
  protected void setSequenceLock(boolean llock)
  {
    Enumeration enumer = graphicName.elements();
    if(!llock)
    {
      while(enumer.hasMoreElements())
        ((SequenceNameJButton)enumer.nextElement()).setSelected(false); 
      enumer = graphicSequence.elements();
      while(enumer.hasMoreElements())
        ((SequenceJPanel)enumer.nextElement()).detachAll();  
       
      return;
    }

    int i = 0;
    Vector selected = new Vector();
    while(enumer.hasMoreElements())
    {
      SequenceNameJButton sbutt = (SequenceNameJButton)enumer.nextElement();
      if(sbutt.isSelected())
        selected.add((SequenceJPanel)graphicSequence.get(i));
      i++;
    }

    // group sequences
    for(i=0;i<selected.size();i++)
    {
      SequenceJPanel si = (SequenceJPanel)selected.get(i);
      for(int j=0;j<selected.size();j++)
        if(i!=j)
          ((SequenceJPanel)selected.get(j)).attach(si);
    }
  }

  /**
  *
  * Set the size of the sequence number panel
  *
  */
  protected void setNumberSize()
  {
    Dimension actual = numberDraw.getMaximumSize();
    int slen = numberDraw.getResidueWidth()*(int)(MAXSEQLENGTH*1.5);
    numberDraw.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
  }


  protected void setMatrix(Matrix mat)
  {
    this.mat = mat;
  }

  /**
  *
  * Determine the colour of a residue at a given position. If
  * the residues at that position in all the sequences are identical
  * then return red otherwise return black.
  * @param s		residue at position pos
  * @param pos		residue position
  * @param seqName	sequence name
  * @return 	red if all identical otherwise return black
  *
  */
  protected Color getColor(String s, int pos, String seqName)
  {
    if(s.equals("-") || s.equals("."))
      return Color.black;

    int identical = 1;
    int nseqs = 0;
    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {
      nseqs++;
      Sequence seq = (Sequence)(enumer.nextElement());
      if(!seqName.equals(seq.getName()))
      {
        SequenceJPanel seqPanel = (SequenceJPanel)graphicSequence.get(nseqs);
        if(pos < seq.getLength() && seqPanel.isPrettyPlot())
          if(seq.getResidue(pos).equalsIgnoreCase(s))
            identical++;
      }
    }
  
    if(identical >= prettyPlot.getMinimumIdentity(nseqs))
      return prettyPlot.getIDColour();

    else if(mat != null)
    {
      double threshold = prettyPlot.getMatchThreshold();
      int m1 = mat.getMatrixIndex(s);
      int matrix[][] = mat.getMatrix();
      float matching = 0.f;
      nseqs = 0;

      enumer = seqs.elements();
      while(enumer.hasMoreElements())
      {
        nseqs++;
        Sequence seq = (Sequence)(enumer.nextElement());
        SequenceJPanel seqPanel = (SequenceJPanel)graphicSequence.get(nseqs);
//      if(!seqName.equals(seq.getName()))
//      {
          if(pos < seq.getLength() && seqPanel.isPrettyPlot())
          {
            int m2 = mat.getMatrixIndex(seq.getResidue(pos));
            if(m1 >= 0 && m2 >= 0 && matrix[m1][m2]>0)
              matching += seq.getWeight();
          }
//      }
      }
      if(matching >= threshold)
        return prettyPlot.getMatchColour(); 
    }
    return Color.black;
  }


  /**
  *
  * Determine the colour of a residue background
  *
  */
  protected Color getPrettyBackground(Color textColour)
  {
    if(textColour.equals(prettyPlot.getIDColour()))
      return prettyPlot.getIDBackgroundColour();
    else if(textColour.equals(prettyPlot.getMatchColour()))
      return prettyPlot.getMatchBackgroundColour();
    else
      return null;
  }

 
  /**
  *
  * Determine if the identities are to be boxed
  * @return 	true if to draw boxes
  *
  */ 
  protected boolean isPrettyBox()
  {
    return prettyPlot.isPrettyBox();
  }

  
  /**
  *
  * 
  *
  */
  protected int testUpAndDown(int pos, Sequence seq)
  {
    int seqIndex = seqs.indexOf(seq);

    int testUp = -1;
    if(seqIndex == 0)
      testUp = 1;
    else
    {
      Sequence seqUp = (Sequence)seqs.get(seqIndex-1);
      String res = seqUp.getSequence().substring(pos,pos+1);
      Color col = getColor(res,pos,seqUp.getName());
      if(col.equals(Color.black))
        testUp = 1;
    }

    int testDown = -1;
    if(seqIndex+1 == seqs.size())
      testDown= 1;
    else
    {
      Sequence seqDown = (Sequence)seqs.get(seqIndex+1);
      SequenceJPanel seqPanelDown = (SequenceJPanel)graphicSequence.get(seqIndex+2);
      String res = seqDown.getSequence().substring(pos,pos+1);
      Color col = getColor(res,pos,seqDown.getName());
      if(col.equals(Color.black) || !seqPanelDown.isPrettyPlot())   
        testDown = 1;
    }

    if(testUp > -1 && testDown > -1)
      return 3;
    else if(testDown > -1)
      return 2;
    else if(testUp > -1)
      return 1;
    return -1;
  }


  /**
  *
  * Remove a sequence from the editor display
  * @param name		name of sequence to remove
  *
  */
  private Sequence removeSequence(String name)
  {
    boolean removed = false;
    int index = 0;
    Enumeration enumer = seqs.elements();
    Sequence seq = null;

    while(enumer.hasMoreElements())
    {
      seq = (Sequence)enumer.nextElement();
      if(seq.getName().equals(name))
      {
        removedSeqs.add(seq);
        removed = true;
        seqs.remove(seq);
        break;
      }
      index++;
    }

    if(!removed)
      return null;
    if(drawNumber)
      index++;

    seqBox.remove(index);
    seqNameBox.remove(index);
    graphicName.removeElementAt(index);
    graphicSequence.removeElementAt(index);
    return seq;
  }

  /**
  *
  *  Delete a sequence from the sequence collection display
  *  and resize the sequence panel display
  *  @param name	name of sequence to remove
  *
  */
  protected void deleteSequence(String name)
  {
    removeSequence(name);
    setMaxSeqLength();
    numberDraw.setSequenceLength(MAXSEQLENGTH);
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);
    numberDraw.setMaximumSize(numberDraw.getPreferredSize());
    numberDraw.setMinimumSize(numberDraw.getPreferredSize());
    setJScrollPaneViewportView();
  }

  /**
  *
  * Move a sequence to a new position
  * @param name		name of sequence to remove
  * @param i		new position in the sequence editor
  *
  */
  protected void moveSequence(String name, int i)
  {
    Sequence seq = removeSequence(name);
    addSequence(seq,true,0,0,i);
  }

  /**
  *
  * Sorts the sequences and displays them by their id's
  *
  */
  protected void idSort()
  {
    int nseqs = 0;

// get no. of sequences excl. consensus
    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {  
      String name = ((Sequence)enumer.nextElement()).getName();
      if(!name.equals(Consensus.DEFAULT_SEQUENCE_NAME))
        nseqs++;
    }

    String seqName[] = new String[nseqs];
    int i = 0;
    enumer = seqs.elements(); 
    while(enumer.hasMoreElements())
    {
      String name = ((Sequence)enumer.nextElement()).getName();
      if(!name.equals(Consensus.DEFAULT_SEQUENCE_NAME))
      {
        seqName[i] = new String(name);
        i++;
      }
    }

    Arrays.sort(seqName);
    for(i=0;i<nseqs;i++)
      moveSequence(seqName[i],i);
  }

  /**
  *
  *  Add a sequence at a particular index to the sequence 
  *  collection display and to the collection of sequences 
  *  (seqs) with a specified y-padding.
  *  @param seq			sequence to add to the editor display
  *  @param addToSequences	true to add seq to the vector of sequences
  *  @param ypad		sequence panel height padding
  *  @param fontSize		font size
  *  @param index		sequence index
  *
  */
  protected void addSequence(Sequence seq, boolean addToSequences, 
                             int ypad, int fontSize, int index)
  {
    if(addToSequences)
      seqs.add(index,seq);
    if(drawNumber)
      index++;

    SequenceJPanel gs = new SequenceJPanel(seq,this,
                           drawSequence,drawBlackBox,drawColorBox,
                           colorScheme,ypad);
    graphicSequence.add(index,gs);

    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(gs);
    XBox.add(Box.createHorizontalGlue());
    seqBox.add(XBox,index);
    gs.setToolTipText("");   //enable tooltip display

    SequenceNameJButton snj = new SequenceNameJButton(seq,ypad);
    graphicName.add(index,snj);
    XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(Box.createHorizontalGlue());
    XBox.add(snj);
    seqNameBox.add(XBox,index);

    if(seq.getLength()>MAXSEQLENGTH)
      MAXSEQLENGTH = seq.getLength();

    Dimension actual = gs.getMaximumSize();
    int slen = gs.getResidueWidth()*(int)(MAXSEQLENGTH*1.2);
    gs.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
  }



  /**
  *
  *  Add a sequence to the sequence collection display and
  *  to the collection of sequences (seqs) with a specified
  *  y-padding.
  *  @param seq                 sequence to add to the editor display
  *  @param addToSequences      true to add seq to the vector of sequences
  *  @param ypad                sequence panel height padding
  *  @param fontSize            font size
  *
  */
  protected void addSequence(Sequence seq, boolean addToSequences,
                             int ypad, int fontSize)
  {
    if(addToSequences)
      seqs.add(seq);
    SequenceJPanel gs = new SequenceJPanel(seq,this,
                           drawSequence,drawBlackBox,drawColorBox,
                           colorScheme,fontSize,ypad);
    graphicSequence.add(gs);

    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(gs);
    XBox.add(Box.createHorizontalGlue());
    seqBox.add(XBox);
    gs.setToolTipText("");   //enable tooltip display

    SequenceNameJButton snj = new SequenceNameJButton(seq,ypad);
    graphicName.add(snj);
    XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(Box.createHorizontalGlue());
    XBox.add(snj);
    seqNameBox.add(XBox);

    if(seq.getLength()>MAXSEQLENGTH)
      MAXSEQLENGTH = seq.getLength();

    Dimension actual = gs.getMaximumSize();
    int slen = gs.getResidueWidth()*(int)(MAXSEQLENGTH*1.2);
    gs.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
  }


  /**
  *
  * Get the sequence view size
  * @return 	sequence view dimension
  *
  */
  public Dimension getViewSize()
  {
    hgt = 1;
    len = 0; 
    Enumeration enumer = graphicSequence.elements();
    while(enumer.hasMoreElements())
    {
      SequenceJPanel gs = (SequenceJPanel)enumer.nextElement();
      hgt = hgt+gs.getSequenceHeight();
      if(len<gs.getSequenceWidth())
        len = gs.getSequenceWidth();
    }

    if(pc !=null) 
    {
      Dimension dplot = pc.getPreferredSize();
      hgt = hgt + (int)dplot.getHeight() + plotConStrut;
    }
    return new Dimension(len,hgt);
  }


  public String getName(int i)
  {
    return ((Sequence)seqs.get(i)).getName();
  }

  /**
  *
  * Get the sequence name view size
  * @return	sequence name dimension
  *
  */
  public Dimension getNameViewSize()
  {
    int hgtName = getNameHeight();
    int lenName = getNameWidth();
    return new Dimension(lenName,hgtName);
  }

  /**
  *
  * Get the sequence view size
  * @return     sequence view dimension
  *
  */
  public Dimension getPanelSize()
  {
    getViewSize();
    return new Dimension(len,hgt);
  }

  /**
  *
  * Get the sequence name panel height 
  * @return     sequence name panel height
  *
  */
  public int getNameHeight()
  {
    int hgtName = 0;
    Enumeration enumer = graphicName.elements();
    while(enumer.hasMoreElements())
      hgtName = hgtName+
          ((SequenceNameJButton)enumer.nextElement()).getPanelHeight();
    return hgtName;
  }

  /**
  *
  * Get the sequence name panel width
  * @return     sequence name panel width
  *
  */
  public int getNameWidth()
  {
    int lenName = 0;
    Enumeration enumer = graphicName.elements();
    while(enumer.hasMoreElements())
    {
      SequenceNameJButton gs = (SequenceNameJButton)enumer.nextElement();
      if(lenName<gs.getPanelWidth())
        lenName = gs.getPanelWidth();
    }
    return lenName;
  }
 
  /**
  *
  * Set the sequence name panel width
  * @param x	sequence name panel  width
  *
  */
  public void setNamePanelWidth(int x)
  {
    seqNamePanel.setPreferredSize(new Dimension(x,1000));
  }

  /**
  *
  * Get the sequence panel height
  * @return     sequence panel height
  *
  */
  public int getPanelHeight()
  {
    getViewSize();
    return hgt;
  }

  /**
  *
  * Get the sequence panel width
  * @return     sequence panel width
  *
  */
  public int getPanelWidth()
  {
//  getNameViewSize();
    getViewSize();
    return len;
  }

  /**
  *
  * Get the vector containing the SequenceJPanel objects
  * @return	vector containing the SequenceJPanel objects
  *
  */
  public Vector getGraphicSequence()
  {
    return graphicSequence;
  }

  /**
  *
  * Draw the boxes around the residues of each sequence
  * @param	true to draw boxes, false not to display
  *		boxes
  *
  */
  public void setDrawBoxes(boolean drawBlackBox)
  {
    this.drawBlackBox = drawBlackBox;
    Enumeration enumer = graphicSequence.elements();
    while(enumer.hasMoreElements())
      ((SequenceJPanel)(enumer.nextElement())).setDrawBoxes(drawBlackBox);
    setJScrollPaneViewportView();
  }

  /**
  *
  * Draw the residue colours in each sequence
  * @param      true to display colour, false not to display
  *             colour
  *
  */
  public void setDrawColor(boolean drawColorBox)
  {
    this.drawColorBox = drawColorBox;
    Enumeration enumer = graphicSequence.elements();
    while(enumer.hasMoreElements())
      ((SequenceJPanel)(enumer.nextElement())).setDrawColor(drawColorBox);
    setJScrollPaneViewportView();
  }

  /**
  *
  * Set the font size of the sequences in the editor
  * @param	font size to use
  *
  */
  public void setFontSizeForCollection(int fs)
  {
    Enumeration enumer = graphicSequence.elements();

    while(enumer.hasMoreElements())
    {
      SequenceJPanel gs = (SequenceJPanel)enumer.nextElement();
      gs.setFontSize(fs);
      Dimension actual = gs.getMaximumSize();
      int slen = gs.getResidueWidth()*(int)(MAXSEQLENGTH*1.2);
      gs.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
    }

    Enumeration enumName = graphicName.elements();
    while(enumName.hasMoreElements())
    {
      SequenceNameJButton snjp = (SequenceNameJButton)enumName.nextElement();
      snjp.setFontSize(fs);
      snjp.setMaximumSize(snjp.getPreferredSize());
    }
   
// rescale consensus plot
    if(pc != null)
    {
      SequenceJPanel sj = (SequenceJPanel)graphicSequence.get(0);
      int interval = sj.getSequenceResidueWidth();
      pc.setInterval(interval);
      pc.setPlotSize();
    }
   
//
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);

    setNamePanelWidth(getNameWidth());
    setJScrollPaneViewportView();
  }

  /**
  *
  * Get the font size used to display the sequences in the
  * editor
  * return 	font size
  *
  */
  public int getFontSize()
  {
    return ((SequenceJPanel)graphicSequence.get(0)).getFontSize();
  }

  /**
  *
  * Search sequences for a pattern and highlight matches. Set the
  * viewport to that position.
  *
  * @param String pat pattern to match 
  * @param int oldResPosition if this is a repeat of a search
  *        this is the position the old search finished at
  * @param boolean wrapAround true if the search should wrap
  *        around the sequences
  * @return int the matching position found (or -1 if none found)
  *
  */
  public int findPattern(String pat, int oldResPosition,
                         boolean wrapAround)
  {
    int resWidth = ((SequenceJPanel)graphicSequence.get(0)).
                                           getSequenceResidueWidth();
    Rectangle r = getViewRect();
    int ypos = r.y;
    int xpos = r.x/resWidth;
    int viewWidth = r.width/resWidth;
    pat = pat.toLowerCase();
            
// highlight matching segments of seqs
    Enumeration enumer = graphicSequence.elements();
    while(enumer.hasMoreElements())
    {
      SequenceJPanel sjp = (SequenceJPanel)enumer.nextElement();
      sjp.showPattern(pat);
    }

// move view to the next occurence of that pattern
    if(oldResPosition > -1)      // possibly this as well: (&& xpos < oldResPosition)
      xpos = oldResPosition;

    int newResPos = searchSequences(xpos,pat);

    if(newResPos > -1)
    {
      int mid = findMiddle(newResPos,viewWidth,pat);
      jspSequence.getViewport().setViewPosition(
                        new Point(mid*resWidth,ypos));
//    System.out.println("xpos "+xpos+" newResPos "+newResPos+
//                       " viewWidth "+viewWidth+" mid "+mid+
//                       " oldResPosition "+oldResPosition);
    }
    else if(wrapAround)     // search from start of seqs
    {
//    JOptionPane.show
      newResPos = searchSequences(0,pat);
//    System.out.println("SEARCH FROM START OF SEQUENCE");
      if(newResPos > -1)
      {
        int mid = findMiddle(newResPos,viewWidth,pat);
        jspSequence.getViewport().setViewPosition(
                         new Point(mid*resWidth,ypos));
      }
    }
    else if(!wrapAround)
      newResPos = oldResPosition;

    return newResPos+1;
  }

  /**
  *
  * Search the sequences for the position in that matches
  * the given pattern to search for. 
  *
  * @param int startSearch position at which the search is started
  * @param String pat is the pattern to search
  * @return int position in a sequence that next matches the pattern
  *         (or -1 if none found)
  *
  */
  private int searchSequences(int startSearch, String pat)
  {
    int newResPos = 0;
    int nfound = 0;

    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {
      Sequence seq = (Sequence)enumer.nextElement();
      int index = seq.getSequence().toLowerCase().indexOf(pat,startSearch);
      if(index > -1)
      {
        if(nfound == 0 || index < newResPos)
          newResPos = index;
        nfound++;
      }
    }
    if(nfound == 0)
      return -1;
    return newResPos;
  }

  /**
  *
  * Locate the center of a vieport view that contains the
  * defined position to display
  *
  * @param int newResPos position in the sequence to display
  * @param int viewWidth width of the viewport
  * @param String pat matching pattern
  * @return int position to set the viewport to so that the
  *         pattern is displayed in the middle of it 
  *
  */
  private int findMiddle(int newResPos, int viewWidth, String pat)
  {
    int mid;
    int viewWidth2 = viewWidth/2;
    if(newResPos <= viewWidth2)
      mid = 0;
    else
      mid = newResPos-viewWidth2+(pat.length()/2);
    return mid;
  }

  /** 
  *
  * Set the colour scheme to use
  * @param colourTable	colour scheme as a hashtable
  *
  */
  public void setColorScheme(Hashtable colourTable)
  {
    this.colorScheme  = colourTable;
    this.drawColorBox = true;

    Enumeration enumer = graphicSequence.elements();
    while(enumer.hasMoreElements())
    {
      SequenceJPanel sjp = (SequenceJPanel)enumer.nextElement();
      sjp.setColorScheme(colourTable);
      sjp.setDrawColor(drawColorBox);
    }
  }
 
  /** 
  *
  * Display the sequences as per emboss prettyplot colours
  * @param bpretty	true if displaying as prettyplot
  *
  */
  public void setPrettyPlot(boolean bpretty, PrettyPlotJFrame prettyPlot)
  {
    this.prettyPlot = prettyPlot;
    Enumeration enumer = graphicSequence.elements();
    while(enumer.hasMoreElements())
      ((SequenceJPanel)(enumer.nextElement())).setPrettyPlot(bpretty);
  }
 
  /**
  *
  * Force display in the viewport
  *
  */
  public void setJScrollPaneViewportView()
  {
    jspSequence.setViewportView(this);
  }

  /**
  *
  * Get the number of pages to print the alignment in
  * a given format
  * @param format	format for printing
  * @return		number of pages
  *
  */
  public int getNumberPages(PageFormat format)
  {
    return getNumberPages(format,getResiduesPerLine(format));
  }

  /**
  *
  * Get the number of pages to print the alignment in
  * a given format and with a given number of residues per line
  * @param format         format for printing
  * @param numResPerLine  number of residues per line
  * @return               number of pages
  *
  */
  public int getNumberPages(PageFormat format, int numResPerLine)
  {
    double pageHeight = format.getImageableHeight();
    int residueWidth  = ((SequenceJPanel)graphicSequence.get(0)).getSequenceHeight();
    int nblockPerPage = (int)(pageHeight/((graphicSequence.size()+2)*residueWidth));
    int npage         = MAXSEQLENGTH/(nblockPerPage*numResPerLine)+1;
    return npage;
  }

  /**
  *
  * Get the imageable size. This is for fitting the image
  * to one page.
  * @return               image size
  *
  */
  public Dimension getImageableSize(int numResPerLine)
  {
    SequenceJPanel seq = (SequenceJPanel)graphicSequence.get(0);
    int residueHeight = seq.getSequenceHeight();
    int alignHeight   = (graphicSequence.size()+1)*residueHeight;
    int nalign = Math.round(((float)MAXSEQLENGTH/
                             (float)numResPerLine)+.5f)*alignHeight;
//  System.out.println("nalign "+
//                     Math.round(((float)MAXSEQLENGTH/(float)numResPerLine)+.5f)+
//                     "  alignHeight "+alignHeight+"  nalign "+nalign+" 
//                     residueHeight "+residueHeight+ 
//                     "  graphicSequence.size()+1 "+(graphicSequence.size()+1));
    int width  = (seq.getResidueWidth()*(numResPerLine+2))+getNameWidth();
    return new Dimension(width,nalign);
  }

  /**
  *
  * Get the number residues per line
  * @param format    	format for printing
  * @return		number residues per line	
  *
  */
  public int getResiduesPerLine(PageFormat format)
  {
    double pwidth = format.getImageableWidth()-(double)getNameWidth();
//  int resWidth = ((SequenceJPanel)graphicSequence.get(0)).getSequenceHeight();
    int resWidth = ((SequenceJPanel)graphicSequence.get(0)).getSequenceResidueWidth();
    return (int)(pwidth/(double)resWidth);
  }

  /**
  *
  * Get the number residues per page
  * @param format         format for printing
  * @param numResPerLine  number residues per line        
  * @return 		  number residues per page
  *
  */
  public int getResiduesPerPage(PageFormat format, int numResPerLine)
  {
    double pageHeight = format.getImageableHeight();
    int residueWidth  = ((SequenceJPanel)graphicSequence.get(0)).getSequenceHeight();
    int nblockPerPage = (int)(pageHeight/(residueWidth*(graphicSequence.size()+2)));
    return nblockPerPage*numResPerLine;
  }

  /**
  *
  * Used to print the sequence alignment
  * @param g		graphics
  * @param format	page format
  * @param pageIndex 	page number to print
  * @throws PrinterException
  *
  */
  public int print(Graphics g, PageFormat format, int pageIndex) 
                                             throws PrinterException
  {
    Graphics2D g2d = (Graphics2D) g.create();
    drawSequences(g2d,format,pageIndex,numResiduePerLine);
    return Printable.PAGE_EXISTS;
  }

  /**
  *
  * Set the number of residues per line to user setting
  * @param numResiduePerLine 	number of residues per line
  *
  */
  protected void setNumberOfResiduesPerLine(int numResiduePerLine)
  {
    this.numResiduePerLine = numResiduePerLine;
  }

  /**
  *
  * Draws the sequences for printing
  * @param g2d		graphics
  * @param format	page format
  * @param pageIndex	page number to print
  *
  */
  public void drawSequences(Graphics2D g2d, PageFormat format, 
                            int pageIndex)
  {
    int numResPerLine = getResiduesPerLine(format);
    drawSequences(g2d,format,pageIndex,numResPerLine);
  }

  /**
  *
  * Draws the sequences for printing
  * @param g2d          	graphics
  * @param format       	page format
  * @param pageIndex    	page number to print
  * @param numResPerLine 	number of residues per line
  */
  public void drawSequences(Graphics2D g2d, PageFormat format, 
                            int pageIndex, int numResPerLine)
  {
    // move origin from the corner of the Paper to the corner of imageable area
    g2d.translate(format.getImageableX(), format.getImageableY());

    int resPerPage  = getResiduesPerPage(format,numResPerLine);
    int istart = resPerPage*pageIndex;
    int istop  = istart+resPerPage;

    if(istop > MAXSEQLENGTH)
      istop = MAXSEQLENGTH;

//  System.out.println("pageIndex "+pageIndex+" numResPerLine "+numResPerLine);
    for(int i=istart;i<istop;i+=numResPerLine)
    {
      Enumeration enumer = graphicSequence.elements();
      SequenceJPanel gs = null;
      int iend;

      while(enumer.hasMoreElements())
      {
        iend = i+numResPerLine;
        if(iend > istop)
          iend = istop;

        gs = (SequenceJPanel)(enumer.nextElement());
        gs.getSequencePrintGraphic(g2d,getNameWidth(),i,iend);
//                                 i+numResPerLine);
        gs.getNamePrintGraphic(g2d);
        g2d.translate(0,gs.getSequenceHeight());
      }
      g2d.translate(0,gs.getSequenceHeight());
    }
  }

//scrollable interface methods

  /**
  *
  * Override for scrollable interface 
  *
  */
  public Dimension getPreferredScrollableViewportSize()
  {
    return getPreferredSize();
  }

  /**
  *
  * Override for scrollable interface 
  *
  */
  public boolean getScrollableTracksViewportHeight()
  {
    return false;
  }

  /**
  *
  * Override for scrollable interface 
  *
  */
  public boolean getScrollableTracksViewportWidth()
  {
    return false;
  }

  /**
  *
  * Override for scrollable interface 
  *
  */
  public int getScrollableBlockIncrement(Rectangle r,
                    int orientation, int direction)
  {
    return 60;
  }

  /**
  *
  * Override for scrollable interface 
  *
  */
  public int getScrollableUnitIncrement(Rectangle r,
                    int orientation, int direction)
  {
    return 60;
  }


  public static void main(String args[])
  {
    Vector seqs = new Vector();
    seqs.add(new Sequence("Seq1","ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"+
             "ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"+
             "ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"));
    seqs.add(new Sequence("Seq2","ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"+
             "ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"+
             "ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"));

    JScrollPane jspSequence = new JScrollPane();
    GraphicSequenceCollection gsc = new GraphicSequenceCollection(
                                          seqs,null,jspSequence,
                                          true,true,true,false,null);
    jspSequence.setViewportView(gsc); 
    JFrame f = new JFrame("Sequence Panel");
    JPanel pane = (JPanel)f.getContentPane();

    pane.add(jspSequence);
    f.pack();
    f.setVisible(true);

  }

  protected void drawNumber()
  {
    numberDraw = new SequenceJPanel(10,MAXSEQLENGTH);
    graphicSequence.add(numberDraw);
    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(numberDraw);
    XBox.add(Box.createHorizontalGlue());
    seqBox.add(XBox);

    setNumberSize();
    SequenceNameJButton snjBlank =
               new SequenceNameJButton(new Sequence(" "),0);
    graphicName.add(snjBlank);
    seqNameBox.add(snjBlank);
  }

  protected void addAnnotationSequence(Sequence s)
  {
    addSequence(s,true,0,getFontSize());
  }

}

