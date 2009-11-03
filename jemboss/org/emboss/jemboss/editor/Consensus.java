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

import java.io.*;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;


/**
*
* Calculate a consensus using the same method as 'cons'
* in the EMBOSS suite.
*
*/
public class Consensus
{
  private int matrix[][];
  private String cons = "";

  /**
  *
  * @param matrixFile	scoring matrix file
  * @param seqs		vector of Sequence objects
  * @param fplural	defines no. of +ve scoring matches below
  *               	which there is no consensus.	
  * @param setcase	upper/lower case given if score above/below
  *                     user defined +ve matching threshold.
  * @param identity	defines the number of identical symbols
  *                	requires in an alignment column for it to
  *                	included in the consensus.
  *
  */
  public Consensus(File matrixFile, Vector seqs, float fplural,
                   float setcase, int identity)
  {
    Matrix mat = new Matrix(matrixFile);
    matrix = mat.getMatrix();
    calculateCons(mat,seqs,fplural,setcase,identity);
  }

  /**
  *
  * @param matrixJar    	jar file containing scoring matrix 
  * @param matrixFileName   	scoring matrix file name
  * @param seqs         	vector of Sequence objects
  * @param fplural      	defines no. of +ve scoring matches below
  *                     	which there is no consensus.
  * @param setcase      	upper/lower case given if score above/below
  *                     	user defined +ve matching threshold.
  * @param identity     	defines the number of identical symbols
  *                     	requires in an alignment column for it to
  *                     	included in the consensus.
  *
  */
  public Consensus(String matrixJar, String matrixFileName, 
                   Vector seqs, float fplural,
                   float setcase, int identity)
  {
    this(new Matrix(matrixJar,matrixFileName),
               seqs,fplural,setcase,identity);
  }

  /**
  *
  * @param mat		scoring matrix
  * @param seqs         vector of Sequence objects
  * @param fplural      defines no. of +ve scoring matches below
  *                     which there is no consensus.
  * @param setcase      upper/lower case given if score above/below
  *                     user defined +ve matching threshold.
  * @param identity     defines the number of identical symbols
  *                     requires in an alignment column for it to
  *                     included in the consensus.
  *
  */
  public Consensus(Matrix mat,
                   Vector seqs, float fplural,
                   float setcase, int identity)
  {
    matrix = mat.getMatrix();
    calculateCons(mat,seqs,fplural,setcase,identity);
  }


  /**
  *
  * Routine to calculate the consensus of a set of sequences
  *
  * @param mat          scoring matrix
  * @param seqs         vector of Sequence objects
  * @param fplural      defines no. of +ve scoring matches below
  *                     which there is no consensus.
  * @param setcase      upper/lower case given if score above/below
  *                     user defined +ve matching threshold.
  * @param identity     defines the number of identical symbols
  *                     requires in an alignment column for it to
  *                     included in the consensus.
  *
  */
  private void calculateCons(Matrix mat, Vector seqs, float fplural,
                            float setcase, int identity)
  {
    int nseqs = seqs.size();
    int mlen = getMaxSequenceLength(seqs);

    String nocon = "-";
    if(((Sequence)seqs.get(0)).isProtein())
      nocon = "x";

    String res = "";

    int matsize = mat.getIDimension();
    float identical[] = new float[matsize];
    float matching[] = new float[matsize];
    float score[]    = new float[nseqs];

    int i;
    int j;
    int m1;
    int m2;
    float contri = 0.f;
    float contrj = 0.f;
    String s1;
    String s2;

    for(int k=0; k< mlen; k++)
    {
      res = nocon;
      for(i=0;i<matsize;i++)          /* reset id's and +ve matches */
      {
        identical[i] = 0.f;
        matching[i]  = 0.f;
      }

      for(i=0;i<nseqs;i++)
        score[i] = 0.f;
 
      for(i=0;i<nseqs;i++)            /* generate score for columns */
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1); 

        if(m1 >= 0)
          identical[m1] += getSequenceWeight(seqs,i);
        for(j=i+1;j<nseqs;j++)
        {
          s2 = getResidue(seqs,j,k);
          m2 = mat.getMatrixIndex(s2);
          if(m1 >= 0 && m2 >= 0)
          { 
            contri = matrix[m1][m2]*getSequenceWeight(seqs,j)+score[i];
            contrj = matrix[m1][m2]*getSequenceWeight(seqs,i)+score[j];
            score[i] = contri;
            score[j] = contrj;
          }
        }
      }

      int highindex = -1;
      float max = -(float)Integer.MAX_VALUE;
      for(i=0;i<nseqs;i++)
      {
        if( score[i] > max ||
           (score[i] == max &&
             (getResidue(seqs,highindex,k).equals("-") ||
              getResidue(seqs,highindex,k).equals(".")) ))
        {
          highindex = i;
          max       = score[i];
        }
      }


      for(i=0;i<nseqs;i++)        /* find +ve matches in the column */
      {
        s1 = getResidue(seqs,i,k); 
        m1 = mat.getMatrixIndex(s1);
        if(matching[m1] == 0.f)
        {
          for(j=0;j<nseqs;j++)
          {
            if(i != j)
            {
              s2 = getResidue(seqs,j,k);
              m2 = mat.getMatrixIndex(s2);
              if(m1 >= 0 && m2 >= 0 && matrix[m1][m2] > 0)
                matching[m1] += getSequenceWeight(seqs,j);
            }
          }
        }
      }


      int matchingmaxindex  = 0;  /* get max matching and identical */
      int identicalmaxindex = 0;
      for(i=0;i<nseqs;i++)
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1);

        if(m1 >= 0)
        {
          if(identical[m1] > identical[identicalmaxindex])
            identicalmaxindex = m1;
        }
      }

      for(i=0;i<nseqs;i++)
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1);
        if(m1 >= 0)
        {
          if(matching[m1] > matching[matchingmaxindex])
            matchingmaxindex = m1;
          else if(matching[m1] ==  matching[matchingmaxindex])
          {
            if(identical[m1] > identical[matchingmaxindex])
              matchingmaxindex= m1;
          }
        }
      }


      /* plurality check */
      s1 = getResidue(seqs,highindex,k); 
      m1 = mat.getMatrixIndex(s1);

      if(m1 >= 0 && matching[m1] >= fplural
         && !s1.equals("-") && !s1.equals("."))
         res = s1;

      if(matching[m1] <= setcase)
        res = res.toLowerCase();

      if(identity>0)                      /* if just looking for id's */
      {
        j=0;
        for(i=0;i<nseqs;i++)
        {
          s1 = getResidue(seqs,i,k); 
          m1 = mat.getMatrixIndex(s1);

          if(matchingmaxindex == m1 
             && !s1.equals("-") && !s1.equals("."))
            j++;
        }
        if(j<identity)
          res = nocon;
      }

      cons = cons.concat(res);
    }
  }

/**
*
* Check all sequences are the same length
* @param seqs	collection of sequences
* @return	true if all sequences are the same length
*
*/
  public boolean isEqualSequenceLength(Vector seqs)
  {
    int len = 0;
    int numseq=0;
    Enumeration enumer = seqs.elements();
    while(enumer.hasMoreElements())
    {
      Sequence seq = (Sequence)enumer.nextElement();
      if(numseq > 0)
        if(len != seq.getLength())
          return false;
     
      len = seq.getLength();
      numseq++;
    }
    return true;
  }

/**
*
* Check all sequences lengths and return length of
* the longest sequence
* @param seqs   collection of sequences
* @return       length of longest sequence
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
* Get the consensus sequence
* @return 	the consensus sequence
*
*/
  public Sequence getConsensusSequence()
  {
    return new Sequence("Consensus",cons);
  }

/**
*
* Get the sequence weight
* @param seqs	set of sequences
* @param i 	index of a sequence in the set
* @return	sequence weight
*
*/
  public float getSequenceWeight(Vector seqs, int i)
  {
    return ((Sequence)seqs.get(i)).getWeight();
  }

/**
*
* Get the residue at a given position from a given sequence
* @param seqs   set of sequences
* @param i      index of a sequence in the set
* @param j	residue position
* @return       residue
*
*/
  public String getResidue(Vector seqs, int i, int k)
  {
    String res = "-";
    try
    {
      res = ((Sequence)seqs.get(i)).getSequence().substring(k,k+1);
    }
    catch(StringIndexOutOfBoundsException sexp)
    {
    }
    return res;
  }

  public static void main(String args[])
  {
    Vector seqs = new Vector();

    seqs.add(new Sequence("MHQDGISSMNQLGGLFVNGRPQ"));
    seqs.add(new Sequence("-MQNSHSGVNQLGGVFVNGRPQ"));
    seqs.add(new Sequence("STPLGQGRVNQLGGVFINGRPP"));
    seqs.add(new Sequence("STPLGQGRVNQLGGVFINGRPP"));
    seqs.add(new Sequence("-MEQTYGEVNQLGGVFVNGRPE"));
    seqs.add(new Sequence("-MEQTYGEVNQLGGVFVNGRPE"));
    seqs.add(new Sequence("MHQDGISSMNQLGGLFVNGRPH"));
    seqs.add(new Sequence("MHQDGISSMNQLGGLFVNGRPR"));
    seqs.add(new Sequence("MHQDGISSMNQLLGLFVNGRPR"));
    seqs.add(new Sequence("MHQDGISSMNQLLGGGGGGGGR"));

    new Consensus(new File("/packages/emboss_dev/tcarver/emboss/emboss/emboss/data/EBLOSUM62"),
                  seqs,49.75f,0.f,0);
  }
}

