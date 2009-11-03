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

import java.awt.*;
import java.awt.event.*;
import java.awt.print.*;
import javax.swing.*;
import java.util.Hashtable;
import java.util.Vector;

/**
*
* Sequence panel for drawing a sequence or the 
* sequence numbers
*
*/
public class GroutSequenceJPanel extends SequenceJPanel 
		implements GroutGraphicSequenceCollectionListener
{

		int centreMarkerX = -1;
		int indent = 0;	// this is only used for drawNumber 

  /**
	 * Constructor for indenting the sequence
  *
  * @param indent number of residues to indent this sequence
  * @param seq		sequence to display
  * @param viewPane	sequence alignment panel
  * @param drawSequence	draw the sequence if true
  * @param drawBlackBox draw a black box around residues
  * @param drawColorBox	colour the residues
  * @param colorTable	colour scheme
  * @param fontSize	font size
  * @param ypad		sequence height pad
  *
  */
  public GroutSequenceJPanel(int indent, Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int fontSize, int ypad)
  {
			super(seq, viewPane, drawSequence, drawBlackBox, drawColorBox,
						colorTable, fontSize, ypad);

			setIndent(indent);
  }

  /**
  *
  * Constructor with default font size and indenting the sequence
  *
  * @param indent       number of residues to indent this sequence
  * @param seq          sequence to display
  * @param viewPane     sequence alignment panel
  * @param drawSequence draw the sequence if true
  * @param drawBlackBox draw a black box around residues
  * @param drawColorBox colour the residues
  * @param colorTable   colour scheme
  * @param ypad         sequence height pad
  *
  */
  public GroutSequenceJPanel(int indent, Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int ypad)
  {
    super(seq,viewPane,drawSequence,drawBlackBox,drawColorBox,
         colorTable,0,ypad);

		setIndent(indent);
  }

  /**
  *
  * Constructor for sequence numbering and indenting the sequence
  *
  * @param indent number of residues to indent this sequence
  * @param interval	numbering interval
  * @param seqLength	length of the sequence
  *
  */
  public GroutSequenceJPanel(int indent, int interval, int seqLength)
  {
			super(interval, seqLength);

		setIndent(indent);
  }

  /**
  *
  * @param indent number of residues to indent this sequence
  * @param seq		sequence to display
  * @param viewPane	sequence alignment panel
  * @param drawSequence	draw the sequence if true
  * @param drawBlackBox draw a black box around residues
  * @param drawColorBox	colour the residues
  * @param colorTable	colour scheme
  * @param fontSize	font size
  * @param ypad		sequence height pad
  *
  */
  public GroutSequenceJPanel(Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int fontSize, int ypad)
  {
			super(seq, viewPane, drawSequence, drawBlackBox, drawColorBox,
						colorTable, fontSize, ypad);
  }

  /**
  *
  * Constructor with default font size
  *
  * @param indent       number of residues to indent this sequence
  * @param seq          sequence to display
  * @param viewPane     sequence alignment panel
  * @param drawSequence draw the sequence if true
  * @param drawBlackBox draw a black box around residues
  * @param drawColorBox colour the residues
  * @param colorTable   colour scheme
  * @param ypad         sequence height pad
  *
  */
  public GroutSequenceJPanel(Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int ypad)
  {
    super(seq,viewPane,drawSequence,drawBlackBox,drawColorBox,
         colorTable,0,ypad);
  }

  /**
  *
  * Constructor for sequence numbering
  *
  * @param indent number of residues to indent this sequence
  * @param interval	numbering interval
  * @param seqLength	length of the sequence
  *
  */
  public GroutSequenceJPanel(int interval, int seqLength)
  {
			super(interval, seqLength);
  }

  /**
  *
  * indents the sequence by specifed amount
  *
  * @param indent number of residues to indent this sequence
  *
  */
		public void setIndent(int indent)
		{
				if(!drawNumber)
				{
						// System.out.println("adding -");
						((GroutSequence) seq).setIndent(indent);
						// System.out.println("setMaxSequenceLength");
            ((GraphicSequenceCollection)viewPane).setMaxSequenceLength(seq.getLength());
						// System.out.println("paintComponent getGraphics = " + getGraphics());
						// when this is first done, the first GroutSequenceJPanel in
						// not drawn, getGraphics() == null so the display is cliped
						if(getGraphics() != null)
						{
								paintComponent(getGraphics());
						}
						// System.out.println("viewPaneResize");
            viewPaneResize();
				}
				else
				{
						// System.out.println("drawNumber indent b4 = " + this.indent);
						this.indent = indent;
						// System.out.println("drawNumber indent af = " + this.indent);
				}
		}

  /**
  *
  * Add the mouse listener's
  *
  */
  public void init()
  {
    if(!drawNumber)
    {
      addMouseListener(new MouseAdapter()
      {
        public void mousePressed(MouseEvent e)
        {
						// System.out.println("mousePressed " + e);
        }
        public void mouseReleased(MouseEvent e)
        {
						// System.out.println("mouseReleased " + e);
        }
      });

      addMouseMotionListener(new MouseMotionAdapter()
      {
        public void mouseDragged(MouseEvent e)
        {
						// System.out.println("mouseDragged " + e);
        }
      });
    }

  }

		public void groutGraphicSequenceCollectionChanged(GroutGraphicSequenceCollectionEvent e)
		{
				repaint();
				/*
				if(getGraphics() != null)
				{
						//						drawCentreMarker(getGraphics());
				}
				*/
		}

  protected void doDrawNumber(int i, FontMetrics metrics, Graphics g)
  {
			// I do not know why this causes loads of lines
			 drawCentreMarker(g);
			if(((int)((i+1-indent)%interval) == 0) && ((i+1-indent) > 0))
					{
							String snum = Integer.toString(i+1-indent);
							int numWidth = metrics.stringWidth(snum);
							g.drawString(snum,
													 ((int)((i+0.5)*resWidth)-(numWidth/2)),
													 seqHeight-boundWidth2);
					}
	} 

  protected void drawCentreMarker(Graphics g)
	{
			Component parent = getParent();
			Component parent2 = parent.getParent();
			Component parent3 = parent2.getParent();
			Component parent4 = parent3.getParent();
			// System.out.println("parent4 = " + parent4);
			JScrollPane scrollPane = ((GraphicSequenceCollection) parent4).jspSequence;
			JScrollBar slider = scrollPane.getHorizontalScrollBar();

			int pos = slider.getValue() + (slider.getVisibleAmount() / 2);
			// int pos = slider.getValue() + (slider.getVisibleAmount() / 2) - (resWidth / 2);
			// System.out.println("pos = " + pos + " slider.getValue() = " + slider.getValue() + "slider.getVisibleAmount()  = " + slider.getVisibleAmount());

			// paintComponent(g);
			g.setColor(Color.white);
			g.drawLine(centreMarkerX, 0, centreMarkerX, seqHeight);
			// g.fillRect(pos, 0, resWidth, seqHeight);
			g.setColor(Color.black);
			g.drawLine(pos, 0, pos, seqHeight);
			centreMarkerX = pos;
	}
						
}

