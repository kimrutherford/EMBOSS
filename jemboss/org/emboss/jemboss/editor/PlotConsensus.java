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
*
***************************************************************/

package org.emboss.jemboss.editor;

import java.io.*;
import java.util.Vector;
import java.util.Enumeration;
import javax.swing.*;
import java.awt.*;

/**
*
* Plot the average similarity. This is calculated by:
*
* <p> Av. Sim. =    sum( Mij*wi + Mji*w2  ) / 
*              ( (Nseq*Wsize)*((Nseq-1)*Wsize) )
*
* <p>   sum   - over column*window size
* <br>  M     - matrix comparison table
* <br>  i,j   - wrt residue i or j
* <br>  Nseq  - no. of sequences in the alignment
* <br>  Wsize - window size
*
*/
public class PlotConsensus extends JPanel
{

  /** sum of scores for each column */
  private float[] sumscore;
  /** number of graph bins making up the seq length */
  private int numbins;
  /** residue width   */
  private int interval;
  /** minimum y value */
  private float ymin = 0.f;
  /** maximum y value */
  private float ymax = 0.f;
  /** associated sequence panel */
  private GraphicSequenceCollection viewPane = null;

  /**
  *
  * @param matrixFile	scoring matrix file
  * @param seqs		vector of the Sequence objects
  * @param winsize	window size
  * @param interval 	residue width
  * @param viewPane	graphical sequence view
  *
  */
  public PlotConsensus(File matrixFile, Vector seqs, int winsize,
                       int interval, GraphicSequenceCollection viewPane)
  {
    this.interval = interval;
    this.viewPane = viewPane;
    System.out.println("viewPane ");
    Matrix mat = new Matrix(matrixFile);
    createPlot(mat,seqs,winsize);
  }

  /**
  *
  * @param matrixFile   scoring matrix file
  * @param seqs         vector of the Sequence objects
  * @param winsize      window size
  * @param interval     residue width
  * 
  */
  public PlotConsensus(File matrixFile, Vector seqs, int winsize,
                       int interval)
  {   
    this.interval = interval;
    Matrix mat = new Matrix(matrixFile);
    createPlot(mat,seqs,winsize);
  }
 
  /**
  *
  * @param matrixJar		jar file containing matrix
  * @param matrixFilename     	scoring matrix file
  * @param seqs           	vector of the Sequence objects
  * @param winsize        	window size
  * @param interval     	residue width
  * 
  */
  public PlotConsensus(String matrixJar, String matrixFileName,
                       Vector seqs, int winsize,
                       int interval)
  {
    this(new Matrix(matrixJar,matrixFileName),seqs,
                                 winsize,interval);
  }

  /**
  *
  * @param mat     	scoring matrix
  * @param seqs     	vector of the Sequence objects
  * @param winsize   	window size
  * @param interval     residue width
  * @param viewPane	graphical sequence view
  * 
  */
  public PlotConsensus(Matrix mat, Vector seqs, int winsize,
                       int interval, GraphicSequenceCollection viewPane)
  {
    this.viewPane = viewPane;
    this.interval = interval;
    createPlot(mat,seqs,winsize);
  }

 
  /**
  *
  * @param mat          scoring matrix
  * @param seqs         vector of the Sequence objects
  * @param winsize      window size
  * @param interval     residue width
  * 
  */
  public PlotConsensus(Matrix mat,
                       Vector seqs, int winsize,
                       int interval)
  {
    this.interval = interval;
    createPlot(mat,seqs,winsize);
  }

  /**
  *
  * Create the consensus plot
  * @param mat 		scoring matrix
  * @param seqs         vector of the Sequence objects
  * @param winsize      window size
  *
  */
  private void createPlot(Matrix mat, Vector seqs, int winsize)
  {
    int matrix[][] = mat.getMatrix();
    int numseq = seqs.size();

    int mseq = getMaxSequenceLength(seqs);
    //int matsize = mat.getIDimension();
    numbins = mseq;
    sumscore = new float[numbins];
    //float x[] = new float[numbins];
    //float y[] = new float[numbins];
    int bin2 = (int)(winsize/2);
    float score[][] = new float[numseq][mseq];

    int m1, m2;
    String s1, s2;
    float contri,contrj;

// loop over the sequence set length
    for(int k=0; k < mseq; k++)
    {
// generate a score for each column
      for(int i=0; i < numseq; i++)
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1);
        for(int j=i+1; j < numseq; j++)
        {
          s2 = getResidue(seqs,j,k);
          m2 = mat.getMatrixIndex(s2);
          if(m1 >= 0 && m2 >= 0)
          {
            contrj = (float)matrix[m1][m2]*getSequenceWeight(seqs,j)+
                            score[i][k];
            contri = (float)matrix[m1][m2]*getSequenceWeight(seqs,i)+
                            score[j][k];
            score[i][k] = contrj;
            score[j][k] = contri;
          }     
        }
      }
      // calculate score for the entire window
      if(k >= (winsize-1))
      {
        int binup = k+1;
        int binlo = k - winsize + 1;
        int binmid = binlo+bin2;
        for(int i=0;i<numseq;i++)
          for(int bin=binlo; bin < binup; bin++)
            sumscore[binmid] += score[i][bin];
      }
    }

    for(int bin=0; bin < numbins; bin++)
    {
      sumscore[bin] = sumscore[bin]/((float)(numseq*(numseq-1)*
                                             winsize*winsize));
      if(ymax<sumscore[bin])
        ymax = sumscore[bin];
      if(ymin>sumscore[bin])
        ymin = sumscore[bin];
    }
//  if(ymin<0)
//    ymin = ymin*-1.f;
   
    setPlotSize();
 }

  /**
  * 
  * Set the consensus plot preferred size using the
  * graph dimensions
  *
  */
  public void setPlotSize()
  {
    setPreferredSize(new Dimension(numbins*interval,
                      (int)(ymax-ymin+1)*interval));
  }

  /**
  *
  * Returns the residue of the sequence i at position k
  * @param seqs		vector of Sequence objects
  * @param i		sequence index
  * @param k 		sequence position
  * @return		residue k from sequence i
  *
  */
  public String getResidue(Vector seqs, int i, int k)
  {
    String res = "-";
    try
    {
      res = ((Sequence)seqs.get(i)).getSequence().substring(k,k+1);
    }
    catch(StringIndexOutOfBoundsException sexp){}
    return res;
  }

  /**
  *
  * Returns the weight of sequence i
  * @param seqs         vector of Sequence objects
  * @param i            sequence index
  * @return 		sequence weight
  *
  */
  public float getSequenceWeight(Vector seqs, int i)
  {
    return ((Sequence)seqs.get(i)).getWeight();
  }

  /**
  *
  * Check all sequences lengths and return length of
  * the longest sequence
  * @param seqs		vector of Sequence objects
  * @return		length of the longest sequence
  *
  */
  public int getMaxSequenceLength(Vector seqs)
  {
    int len = 0;

    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {
      Sequence seq = (Sequence)enumer.nextElement();
      if(len < seq.getLength())
        len = seq.getLength();
    }
    return len;
  }

  /**
  *
  * Override paintComponent to draw consensus plot
  * @param g	graphics
  *
  */
  public void paintComponent(Graphics g)
  {
    super.paintComponent(g);
    int y1,y2;

    float scale = 2/(ymax-ymin);
    float ytop  = scale*ymax*interval;
//  System.out.println("MAX "+ymax+" MIN "+ymin+" SCALE "+scale);
    
    int istart = 0;
    int istop  = numbins;
    if(viewPane != null)
    {
      Rectangle viewRect = viewPane.getViewRect();
      istart = viewRect.x/interval - 2;
      if(istart < 0)
        istart = 0;
      istop  = istart + viewRect.width/interval+4;
      if(istop > numbins)
        istop = numbins;
    }
//  System.out.println("istart "+istart+" istop "+istop+
//                     " interval "+interval);
  
    g.setColor(Color.lightGray);
    g.drawLine(0,(int)(scale*ymax*interval),
               numbins*interval,(int)(scale*ymax*interval));
    g.setColor(Color.red);

    for(int bin=istart; bin+1 < istop; bin++)
    {
      y1 = (int)(ytop-(scale*sumscore[bin]*interval));
      y2 = (int)(ytop-(scale*sumscore[bin+1]*interval));
      g.drawLine((int)((bin+0.5)*interval),y1,
                 (int)((bin+1.5)*interval),y2);
    }
  }

  /**
  *
  * Set the interval
  * @param interval	interval
  *
  */
  public void setInterval(int interval)
  {
    this.interval = interval;
  }

  public static void main(String args[])
  {
    Vector seqs = new Vector();
    seqs.add(new Sequence("MHQDGISSMNQLGGLFVNGRP"));
    seqs.add(new Sequence("-MQNSHSGVNQLGGVFVNGRP"));
    seqs.add(new Sequence("STPLGQGRVNQLGGVFINGRP"));
    seqs.add(new Sequence("STPLGQGRVNQLGGVFINGRP"));
    seqs.add(new Sequence("-MEQTYGEVNQLGGVFVNGRP"));

    int wsize = 2;
    int interval = 10;
    Matrix mat = new Matrix("resources/resources.jar",
                            "EDNAFULL");

    PlotConsensus pc =  new PlotConsensus(mat,seqs,wsize,interval);
    JFrame f = new JFrame("Plot Consensus");
    JPanel pane = (JPanel)f.getContentPane();

    pane.add(pc);
    pane.setPreferredSize(pc.getPreferredSize());
    f.setSize(pc.getPreferredSize());
    f.pack();
    f.setVisible(true);
  }

}

