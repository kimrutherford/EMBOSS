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
import javax.swing.border.*;
import java.net.URL;

import org.emboss.jemboss.gui.sequenceChooser.SequenceFilter;
import org.emboss.jemboss.gui.filetree.FileEditorDisplay;
import org.emboss.jemboss.gui.ScrollPanel;

/**
*  
* Displays a grapical representation of a collection of
* sequences.
*
*/
public class GroutAlignJFrame extends AlignJFrame implements
		GroutGraphicSequenceCollectionListener
{
		private Vector targets = new Vector();
		private int position = 0;
 
  /**
  * This constructor is for Grout.
  * It includes the indent for each sequence
  *
  * @param vseqs	vector containing Sequence objects
  *
  */ 
  public GroutAlignJFrame(Vector vseqs, Vector indents)
  {
			super();
			if(vseqs != null && vseqs.size() > 0)
          openMethod(vseqs, indents);
			addMenuItems();
  }

  /**
  *
  * @param vseqs	vector containing Sequence objects
  *
  */ 
  public GroutAlignJFrame(Vector vseqs)
  {
			super(vseqs);
			addMenuItems();
  }

  /**
  *
  * @param seqFile	sequence file
  *
  */
  public GroutAlignJFrame(File seqFile)
  {
			super(seqFile);
			addMenuItems();
  }

  /**
  *
  * @param seqString	formatted sequence string
  * @param name 	name of sequence set
  *
  */
  public GroutAlignJFrame(String seqString, String name)
  {
			super(seqString, name);
			addMenuItems();
  }

  public GroutAlignJFrame()
  {
			super(false);
			addMenuItems();
  }

  /**
  *
  * @param useExitMenu	true if an exit menu is to be displayed
  *			otherwise a close menu is used
  *
  */
  public GroutAlignJFrame(boolean useExitMenu)
  {
			super(useExitMenu);
  }

  /**
  *
  * Extends WindowAdapter to handle disposal of the occurance
  *
  */
  class winExit extends WindowAdapter
  {
     public void windowClosing(WindowEvent we)
     {
					// System.out.println("windowClosing HERE");
        dispose();
     }

			public void windowDeiconified(WindowEvent e)
			{
					// System.out.println("windowDeiconified HERE");
			}

			public void windowOpened(WindowEvent e)
			{
					// System.out.println("windowOpened HERE");
			}
  }

  /**
  *
  * Given a Vector of Sequence display them in the editor
  * @param seqVector	vector containing Sequence objects
  *
  */
  protected void openMethod(Vector seqVector)
  {
			//			((AlignJFrame) this).openMethod(seqVector);
		// System.out.println("GroutAlignJFrame openMethod");

    gsc = new GroutGraphicSequenceCollection(seqVector,
                                   jspSequence,true,false,true,true,
                                   statusField);
			((GroutGraphicSequenceCollection )gsc).
					addGroutGraphicSequenceCollectionListener(this);
// set colour scheme
    gsc.setColorScheme(SequenceProperties.residueColor);
    currentColour = (Hashtable)SequenceProperties.residueColor.clone();
    residueColor.setSelected(true);
    jspSequence.setViewportView(gsc);

    colourScheme("Residue colour");
  }

  /**
  *
  * Given a Vector of Sequence and the indent for each
	* display them in the editor
  *
  * @param seqVector	vector containing Sequence objects
  * @param indents    vector containing ints giving indents
  *
  */
  protected void openMethod(Vector seqVector, Vector indents)
  {
			//			((AlignJFrame) this).openMethod(seqVector);
		// System.out.println("GroutAlignJFrame openMethod indents");

    gsc = new GroutGraphicSequenceCollection(seqVector, indents,
                                   jspSequence,true,false,true,true,
                                   statusField);
		if(seqVector.size() > 1)
		{
				gsc.drawNumber();
		}
			((GroutGraphicSequenceCollection )gsc).
					addGroutGraphicSequenceCollectionListener(this);
// set colour scheme
    gsc.setColorScheme(SequenceProperties.residueColor);
    currentColour = (Hashtable)SequenceProperties.residueColor.clone();
    residueColor.setSelected(true);
    jspSequence.setViewportView(gsc);

    colourScheme("Residue colour");
  }

  public synchronized void addGroutAlignJFrameListener(GroutAlignJFrameListener listener)
  {
				// System.out.println("adding to GroutAlignJFrame = " + listener);
    targets.addElement(listener);
  }
  
  public synchronized void removeGroutAlignJFrameListener(GroutAlignJFrameListener listener)
  {
    targets.removeElement(listener);
  }
  
  protected void notifyTargets(String propertyName, Object oldValue, Object newValue)
  {
    Vector l;
    GroutAlignJFrameEvent s = new GroutAlignJFrameEvent((Object) this, propertyName, oldValue, newValue);
    synchronized(this)
    {
      l = (Vector) targets.clone();
    }
    for (int i = 0; i < l.size(); i++)
    {
      GroutAlignJFrameListener sl = (GroutAlignJFrameListener) l.elementAt(i);
      sl.groutAlignJFrameChanged(s);
    }
  }

  /**
  *
  * Moves the view position to this point
  *
  * @param position the position to view
  *
  */
		public void setPosition(int position)
		{
				// System.out.println("alignEditor setPosition");
				// System.out.println("setPosition xcv to " + position);
				((GroutGraphicSequenceCollection)gsc).setPosition(position);
				this.position = position;
		}

  /**
  *
  * Sets the indents to these values
  *
  * @param position the position to view
  *
  */
		public void setIndents(Vector indents)
		{
				// System.out.println("alignEditor setIndents to " + indents.elementAt(0) +  " " + indents.elementAt(1));
				((GroutGraphicSequenceCollection)gsc).doIndent(indents);
		}

  /**
  *
  * when you close the window, should remove lines from Grout
  *
  */
	protected void addWindowCloser()
  {
      addWindowListener(new java.awt.event.WindowAdapter()
      {
        public void windowClosing(java.awt.event.WindowEvent evt)
        {
					notifyTargets("windowClosed", new Boolean(false), new Boolean(true));
          dispose();
        }
        public void windowOpened(java.awt.event.WindowEvent evt)
        {
						setPosition(position);
						GroutSequenceJPanel sequenceJPanel = (GroutSequenceJPanel) ((GraphicSequenceCollection)gsc).getGraphicSequence().elementAt(0);
						if(sequenceJPanel != null)
						{
								sequenceJPanel.drawCentreMarker(sequenceJPanel.getGraphics());
						}
						int height = ((GroutGraphicSequenceCollection) gsc).setSize();
						Dimension size = jspSequence.getPreferredSize();
						Dimension size2 = jspSequence.getHorizontalScrollBar().getPreferredSize();
						size.height = height + size2.height + 4;
						jspSequence.setPreferredSize(size);
						pack();
        }
      });
  }

		private void addMenuItems()
		{
				JMenu groutMenu = new JMenu("Grout Menu");
				JMenuItem newAlignFrame = new JMenuItem("new Align Frame");
				newAlignFrame.addActionListener(new ActionListener()
						{
								public void actionPerformed(ActionEvent e)
								{
										notifyTargets("newAlignJFrame", new Boolean(false), new Boolean(true));
								}
						}
																				);
				groutMenu.add(newAlignFrame);
				final JCheckBoxMenuItem jumpTo = new JCheckBoxMenuItem("Jump to nearest alignment");
				jumpTo.addActionListener(new ActionListener()
						{
								public void actionPerformed(ActionEvent e)
								{
										notifyTargets("JumpToNearestAlignment",
																	new Boolean(!jumpTo.getState()), 
																	new Boolean(jumpTo.getState()));
								}
						}
																				);
				groutMenu.add(jumpTo);

				menuBar.add(groutMenu);
						
		}

  /**
  *
  * Responds to the groutGraphicSequenceCollectionChanged event
  *
  * @param e the event
  *
  */
		public void groutGraphicSequenceCollectionChanged
				(GroutGraphicSequenceCollectionEvent e)
		{
				// System.out.println("groutGraphicSequenceCollectionChanged to " + e.getNewValue());
				notifyTargets(e.getPropertyName(), e.getOldValue(), e.getNewValue());
		}

  /**
  *
  * Test main really
  *
  */
  public static void main(String args[])
  {
    Vector indents = new Vector();
    Vector seqs = new Vector();
    Sequence s = new Sequence("Seq2","ggcagcttaagccaaacattcccaaatctatgaagcagggcccattgttggtcagttgtt"+
"atttgcaatgaagcacagttctgatcatgtttaaagtggaggcacgcagggcaggagtgc"+
"ttgagcccaagcaaaggatggaaaaaaataagcctttgttgggtaaaaaaggactgtctg"+
"agactttcatttgttctgtgcaacatataagtcaatacagataagtcttcctctgcaaac"+
"ttcactaaaaagcctgggggttctggcagtctagattaaaatgcttgcacatgcagaaac"+
"ctctggggacaaagacacacttccactgaattatactctgctttaaaaaaatccccaaaa"+
"gcaaatgatcagaaatgtagaaattaatggaaggatttaaacatgaccttctcgttcaat"+
"atctactgttttttagttaaggaattacttgtgaacagataattgagattcattgctccg"+
"gcatgaaatatactaataattttattccaccagagttgctgcacatttggagacaccttc"+
"ctaagttgcagtttttgtatgtgtgcatgtagttttgttcagtgtcagcctgcactgcac"+
"agcagcacatttctgcaggggagtgagcacacatacgcactgttggtacaattgccggtg"+
"cagacatttctacctcctgacattttgcagcctacattccctgagggctgtgtgctgagg"+
"gaactgtcagagaagggctatgtgggagtgcatgccacagctgctggctggcttacttct"+
"tccttctcgctggctgtaatttccaccacggtcaggcagccagttccggcccacggttct"+
"gttgtgtagacagcagagactttggagacccggatgtcgcacgccaggtgcaagaggtgg"+
"gaatgggagaaaaggagtgacgtgggagcggagggtctgtatgtgtgcacttgggcacgt"+
"atatgtgtgctctgaaggtcaggattgccagggcaaagtagcacagtctggtatagtctg"+
"aagaagcggctgctcagctgcagaagccctctggtccggcaggatgggaacggctgcctt"+
"gccttctgcccacaccctagggacatgagctgtccttccaaacagagctccaggcactct"+
"cttggggacagcatggcaggctctgtgtggtagcagtgcctgggagttggccttttactc"+
"attgttgaaataatttttgtttattatttatttaacgatacatatatttatatatttatc"+
"aatggggtatctgcagggatgttttgacaccatcttccaggatggagattatttgtgaag"+
"acttcagtagaatcccaggactaaacgtctaaattttttctccaaacttgactgacttgg"+
"gaaaaccaggtgaatagaataagagctgaatgttttaagtaataaacgttcaaactgctc"+
"taagtaaaaaaatgcattttactgcaatgaatttctagaatatttttcccccaaagctat"+
"gcctcctaacccttaaatggtgaacaactggtttcttgctacagctcactgccatttctt"+
"cttactatcatcactaggtttcctaagattcactcatacagtattatttgaagattcagc"+
"tttgttctgtgaatgtcatcttaggattgtgtctatattcttttgcttatttctttttac"+
"tctgggcctctcatactagtaagattttaaaaagccttttcttctctgtatgtttggctc"+
"accaaggcgaaatatatattcttctctttttcatttctcaagaataaacctcatctgctt"+
"ttttgtttttctgtgttttggcttggtactgaatgactcaactgctcggttttaaagttc"+
"aaagtgtaagtacttagggttagtactgcttatttcaataatgttgacggtgactatctt"+
"tggaaagcagtaacatgctgtcttagaaatgacattaataatgggcttaaacaaatgaat"+
"aggggggtccccccactctccttttgtatgcctatgtgtgtctgatttgttaaaagatgg"+
"acagggaattgattgcagagtgtcgcttccttctaaagtagttttattttgtctactgtt"+
"agtatttaaagatcctggaggtggacataaggaataaatggaagagaaaagtagatattg"+
"tatggtggctactaaaaggaaattcaaaaagtcttagaacccgagcacctgagcaaactg"+
"cagtagtcaaaatatttatctcatgttaaagaaaggcaaatctagtgtaagaaatgagta"+
"ccatatagggttttgaagttcatatactagaaacacttaaaagatatcatttcagatatt"+
"acgtttggcattgttcttaagtatttatatctttgagtcaagctgataattaaaaaaaat"+
"ctgttaatggagtgtatatttcataatgtatcaaaatggtgtctatacctaaggtagcat"+
"tattgaagagagatatgtttatgtagtaagttattaacataatgagtaacaaataatgtt"+
"tccagaagaaaggaaaacacattttcagagtgcgtttttatcagaggaagacaaaaatac"+
"acacccctctccagtagcttatttttacaaagccggcccagtgaattagaaaaacaaagc"+
"acttggatatgatttttggaaagcccaggtacacttattattcaaaatgcacttttactg");

    Sequence s1 = new Sequence("Seq1a","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC");
    s1.reverseSequence();
		seqs.add(s1);
		seqs.add(s);
//  seqs.add(new Sequence("Seq1b","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
//  seqs.add(new Sequence("Seq2a","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
//  seqs.add(new Sequence("Seq3","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
//  seqs.add(new Sequence("Seq4","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
//  seqs.add(new Sequence("Seq5","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
//  seqs.add(new Sequence("Seq6","AAAAAACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));
//  seqs.add(new Sequence("Seq111","ACTATACAGAGTAGACTgTATAGAtTATAAGCGACATACGAGAGACGAC"));

		// System.out.println("GroutAlignJFrame main");

		indents.add(new Integer(10));
		indents.add(new Integer(0));

    new GroutAlignJFrame(seqs, indents);
  }

}

