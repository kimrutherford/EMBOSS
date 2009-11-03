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
import java.awt.event.AdjustmentListener;


/**
*  
* This class can be used to get a grapical representation
* of a collection of sequences.
*
*/
public class GroutGraphicSequenceCollection extends 
		GraphicSequenceCollection implements AdjustmentListener
{

		Vector targets; // = new Vector();

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
  public GroutGraphicSequenceCollection(Vector seqs, Hashtable colorScheme,
                         JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    super(seqs, colorScheme, jspSequence, drawSequence, drawBlackBox,
					 drawColorBox, drawNumber, statusField);
		jspSequence.getHorizontalScrollBar().addAdjustmentListener(this);
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
  public GroutGraphicSequenceCollection(Vector seqs, JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    super(seqs,null,jspSequence,drawSequence,
         drawBlackBox,drawColorBox,drawNumber,statusField);
		jspSequence.getHorizontalScrollBar().addAdjustmentListener(this);
  }

  /**
  *
  * @param seqs		      vector of sequences
  * @param indents      vector of indents
  * @param colorScheme	sequence colour scheme
  * @param jspSequence 	sequence scrollpane
  * @param drawSequence true to draw the sequence
  * @param drawBlackBox true to draw black square around residues
  * @param drawColorBox true to colour residues
  * @param drawNumber   true to draw sequence position number
  * @param statusField	status field in the editor
  *
  */
  public GroutGraphicSequenceCollection(Vector seqs, Vector indents,
																				Hashtable colorScheme,
                         JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    super(seqs, colorScheme, jspSequence, drawSequence, drawBlackBox,
					 drawColorBox, drawNumber, statusField);
		doIndent(indents);
		jspSequence.getHorizontalScrollBar().addAdjustmentListener(this);
  }
 
  /**
  *
  * @param seqs         vector of sequences
  * @param indents      vector of indents
  * @param jspSequence  sequence scrollpane
  * @param drawSequence true to draw the sequence
  * @param drawBlackBox true to draw black square around residues
  * @param drawColorBox true to colour residues
  * @param drawNumber   true to draw sequence position number
  * @param statusField  status field in the editor
  *
  */
  public GroutGraphicSequenceCollection(Vector seqs, Vector indents,
																				JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    super(seqs,null,jspSequence,drawSequence,
         drawBlackBox,drawColorBox,drawNumber,statusField);
		
		// System.out.println("GroutGraphicSequenceCollection Vector seqs, Vector indents");
		doIndent(indents);
		jspSequence.getHorizontalScrollBar().addAdjustmentListener(this);
  }

	public void doIndent(Vector indents)
		{
				Vector numberSequenceJpanels = new Vector();
				// System.out.println("gsc setIndents to " + indents.elementAt(0) +  " " + indents.elementAt(1));
				int limit = graphicSequence.size();
				/*
				if(seqs.size() < indents.size())
				{
						limit = seqs.size();
				}
				else
				{
						limit = indents.size();
				}
				*/
				int drawNumberJPanels = 0;
				// System.out.println("xclimit = " + limit);
				for(int i = 0; i < limit; ++i)
				{
						// this is the new way
						// System.out.println("xclindenting graphicSequence.elementAt(" + i + ") = " + graphicSequence.elementAt(i));
				// System.out.println("xcligraphicSequence = " + graphicSequence);
				// System.out.println("xclindents = " + indents);
				// System.out.println("indents.elementAt(i - drawNumberJPanels) = " + indents.elementAt(i - drawNumberJPanels));
						if(!((SequenceJPanel) graphicSequence.elementAt
								 (i)).drawNumber)
						{
								((GroutSequenceJPanel) graphicSequence.elementAt
								 (i)).setIndent
										(((Integer) indents.elementAt(i - drawNumberJPanels)).intValue());
								setMaxSequenceLength(((GroutSequence) seqs.elementAt(i - drawNumberJPanels)).getLength());
								// System.out.println("MAXSEQLENGTH = " + MAXSEQLENGTH);
						}
						else
						{
								numberSequenceJpanels.addElement(graphicSequence.elementAt(i));
								((GroutSequenceJPanel) graphicSequence.elementAt
								 (i)).setIndent
										(((Integer) indents.elementAt(drawNumberJPanels)).intValue());
								++drawNumberJPanels;
						}
						// this is the old way
						/*
						((GroutSequence) seqs.elementAt(i)).setIndent
								(((Integer) indents.elementAt(i)).intValue());
						
						setMaxSequenceLength(((GroutSequence) seqs.elementAt(i)).getLength());
						if(getGraphics() != null)
						{
								// I am sure you need something like this
								paintComponent(getGraphics());
								// viewPaneResize();
								((SequenceJPanel)graphicSequence.elementAt(0)).paintComponent(((SequenceJPanel)graphicSequence.elementAt(0)).getGraphics());
								((SequenceJPanel)graphicSequence.elementAt(0)).viewPaneResize();

						}
						*/
						//						((Sequence) seqs.elementAt(i))
						//								.indentSequence(((Integer) indents.elementAt(i)).intValue());
				}
				for(int i = 0; i < numberSequenceJpanels.size(); ++i)
				{
						((SequenceJPanel)numberSequenceJpanels.elementAt(i)).setSequenceLength(MAXSEQLENGTH);
				}
				//				addResidue();
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

    GroutSequenceJPanel gs = new GroutSequenceJPanel(seq,this,
                           drawSequence,drawBlackBox,drawColorBox,
                           colorScheme,ypad);
		// System.out.println("adding new gs = " + gs);
    graphicSequence.add(index,gs);
				for(int i = 0; i < graphicSequence.size(); ++i)
				{
						// System.out.println("as graphicSequence.elementAt(" + i + ") = " + graphicSequence.elementAt(i));
				}

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
    GroutSequenceJPanel gs = new GroutSequenceJPanel(seq,this,
                           drawSequence,drawBlackBox,drawColorBox,
                           colorScheme,fontSize,ypad);
		// System.out.println("adding new2 gs = " + gs);
    graphicSequence.add(gs);
				// for(int i = 0; i < graphicSequence.size(); ++i)
				// {
						// System.out.println("graphicSequence.elementAt(" + i + ") = " + graphicSequence.elementAt(i));
			// 	}

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
  * Moves the view position to this residue
  *
  * @param position the position to view
  *
  */
		public void setPosition(int position)
		{
				int extent = jspSequence.getHorizontalScrollBar().getVisibleAmount();
				int residueWidth = ((SequenceJPanel)graphicSequence.elementAt(0))
						.getResidueWidth();

				int bottom = (position * residueWidth) - extent / 2;
				if(bottom < 0)
				{
						bottom = 0;
				}

				// System.out.println("GroutGraphicSequenceCollection setPosition to " + bottom);
				// System.out.println("position2 = " + position + " residueWidth = " + residueWidth + " extent = " + extent + " (position * residueWidth) = " + (position * residueWidth)  + " extent / 2 = " +  extent / 2);
				jspSequence.getHorizontalScrollBar().setValue(bottom);
		}

		private void addResidue()
		{
				int resPos = 1;
				int pressedResidue = 0;
				String padChar = new String("-");
				for(int i = 0; i< graphicSequence.size(); ++i)
				{
						SequenceJPanel seqP = ((SequenceJPanel) graphicSequence.elementAt(i));
						// System.out.println("seqP = " + seqP + " seqP.seq = " + seqP.seq);
						if(seqP.seq != null)
						{
								// System.out.println("seqP.seq.getSequence() = " + seqP.seq.getSequence());
						
								//String seqS = seqP.seq.getSequence();

								// System.out.println("seqS = " + seqS);

								if(resPos == pressedResidue+1 && resPos > 0)
								{
										seqP.seq.insertResidue(padChar,pressedResidue);
										setMaxSequenceLength(seqP.seq.getLength());
										// seqP.pressedResidue = pressedResidue+1;
										seqP.paintComponent(getGraphics());
										seqP.viewPaneResize();
								}
						}
				}
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
			if(graphicSequence.elementAt(0) != null)
			{
					return ((SequenceJPanel)graphicSequence.elementAt(0))
							.getResidueWidth();
			}
			else
			{
					return 18;
			}
  }

  public synchronized void addGroutGraphicSequenceCollectionListener(GroutGraphicSequenceCollectionListener listener)
  {
		// System.out.println("adding to " + targets + " = " + listener);
		if(targets == null)
		{
				targets = new Vector();
		}
    targets.addElement(listener);
  }
  
  public synchronized void removeGroutGraphicSequenceCollectionListener(GroutGraphicSequenceCollectionListener listener)
  {
    targets.removeElement(listener);
  }
  
  protected void notifyTargets(String propertyName, Object oldValue, 
															 Object newValue)
  {
				// System.out.println("notifyTargets = " + targets.size());
    Vector l;
    GroutGraphicSequenceCollectionEvent s = 
				new GroutGraphicSequenceCollectionEvent((Object) this, 
																								propertyName, 
																								oldValue, newValue);
    synchronized(this)
    {
      l = (Vector) targets.clone();
    }
    for (int i = 0; i < l.size(); i++)
    {
      GroutGraphicSequenceCollectionListener sl = 
					(GroutGraphicSequenceCollectionListener) l.elementAt(i);
			// System.out.println(propertyName + " changed in " + sl);
      sl.groutGraphicSequenceCollectionChanged(s);
    }
  }

		public void adjustmentValueChanged(AdjustmentEvent evt)
		{
				// System.out.println("result = " + evt + " graphicSequence.size() = " + graphicSequence.size());
				if(graphicSequence.size() > 0)
				{
						int bottom = jspSequence.getHorizontalScrollBar().getValue();
						int extent = jspSequence.getHorizontalScrollBar().getVisibleAmount();
						int residueWidth = ((SequenceJPanel)graphicSequence.elementAt(0))
								.getResidueWidth();
						int middlePosition = (bottom + (extent / 2)) / residueWidth;

						notifyTargets("sequencePositionChanged", new Integer(0),
													new Integer(middlePosition));
				}
		}

    protected void drawNumber()
    {
			// System.out.println("drawNumber zdgtja");
				
      numberDraw = new GroutSequenceJPanel(10,MAXSEQLENGTH);
			// System.out.println("numberDraw = " + numberDraw);
			addGroutGraphicSequenceCollectionListener(((GroutSequenceJPanel) numberDraw));
      graphicSequence.add(numberDraw);
				for(int i = 0; i < graphicSequence.size(); ++i)
				{
						// System.out.println("dn graphicSequence.elementAt(" + i + ") = " + graphicSequence.elementAt(i));
				}
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

		public int setSize()
		{
				Dimension size = getPreferredSize();
				size.height = numberDraw.seqHeight * graphicSequence.size();
				setPreferredSize(size);
				return size.height;
		}

}

