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

/**
*
* Sequence object
*
*/
public class Sequence 
{

  /** sequence name        */
  private String name; 
  /** sequence as a string */  
  protected String seq;
  /** sequence length      */
  protected int length;
  /** sequence weight      */
  private float wt;
  /** sequence id 	   */
  private String id;
  /** sequence type	   */
  private boolean protein;

  /**
  *
  * @param name		sequence name
  * @param seq		sequence string
  * @param length	sequence length
  * @param weight	sequence weight
  *
  */
  public Sequence(String name, String seq, int length,
                  float wt)
  {
    this.name = name;
    this.seq = seq;
    this.length = length;
    this.wt = wt;
    int index = name.indexOf(" ");
    if(index>0)
      id = name.substring(0,index);
    else
      id = name;
  }

  /**
  *
  * @param name         sequence name
  * @param seq          sequence string
  * @param length       sequence length
  *
  */
  public Sequence(String name, String seq, int length)
  {
    this(name,seq,length,1.0f);
  }

  /**
  *
  * @param name         sequence name
  * @param seq          sequence string
  *
  */
  public Sequence(String name, String seq)
  {
    this(name,seq,seq.length());
  }

  /**
  *
  * @param seq          sequence string
  *
  */
  public Sequence(String seq)
  {
    this(new String(""),seq);
  }

  /**
  *
  * @param seq          sequence string
  * @param weight       sequence weight
  *
  */
  public Sequence(String seq, float wt)
  {
    this(new String(""),seq,seq.length(),wt);
  }

  /**
  *
  * Get the sequence name
  * @return 	sequence name
  *
  */
  public String getName()
  {
    return name;
  }

  /**
  *
  * Get the sequence id
  * @return     sequence id
  *
  */
  public String getID()
  {
    return id;
  }

  /**
  *
  * Get the sequence 
  * @return     sequence 
  *
  */
  public String getSequence()
  {
    return seq;
  }


  /**
  *
  * Get the residue at a position
  * @return     sequence
  *
  */
  public String getResidueAt(int p)
  {
    return seq.substring(p,p+1);
  }


  /**
  *
  * Get the sequence length
  * @return     sequence length
  *
  */
  public int getLength()
  {
    return length;
  }

  /**
  *
  * Get the sequence weight
  * @return     sequence weight
  *
  */
  public float getWeight()
  {
    return wt;
  }

  /**
  *
  * Get a residue in the sequence at a given position
  * @param pos	sequence position
  * @return     residue
  *
  */
  public String getResidue(int pos)
  {
    return seq.substring(pos,pos+1);
  }

  /**
  *
  * Insert a residue into the sequence
  * @param s	residue to insert
  * @param pos	position to insert into sequence
  *
  */
  public void insertResidue(String s, int pos)
  {
    seq = seq.substring(0,pos)+s+
          seq.substring(pos);
    length++;
  }


  /**
  *
  * Delete a residue from the sequence
  * @param pos  position to delete the residue
  *
  */
  public void deleteResidue(int pos)
  {
    seq = seq.substring(0,pos)+
          seq.substring(pos+1);
    length--;
  }


  /**
  *
  * Trim a sequence 
  * @param start	new start position
  * @param end		new end position
  *
  */
  public void trim(int start, int end)
  {
    seq    = seq.substring(start-1,end);
    length = end-start+1;
  }


  /**
  *
  *  Append to a sequence
  *  @param s	sequence to append 
  *
  */
  public void appendToSequence(String s)
  {
    seq = seq+s;
    length+=s.length();
  }

  /**
  *
  *  Reverse the sequence order
  *
  */
  public void reverseSequence()
  {
    char tmpChar[] = new char[length];
    for(int i = 0; i < length; i++)
      tmpChar[i] = seq.charAt(length-i-1);

    seq = new String(tmpChar);
  }


  /**
  *
  *  Reverse and complement the sequence 
  *
  */
  public void reverseComplementSequence()
  {
    char tmpChar[] = new char[length];

    for(int i = 0; i < length; i++)
      tmpChar[i] = complement(seq.charAt(length-i-1));
    
    seq = new String(tmpChar);
  }
 
 
  /**
  *
  * Set the sequence type
  *
  */
  public void setType(boolean protein)
  {
    this.protein = protein;
  }


  /**
  *
  * Set the sequence weight
  * @param 	sequence weight
  *
  */
  public void setWeight(float wt)
  {
    this.wt = wt;
  }


  /**
  *
  * Get the sequence type
  * @param 	true if protein
  *
  */
  public boolean isProtein()
  {
    return protein;
  }


  /**
  *
  * Complement the sequence
  *
  */
  public void complementSequence()
  {
    char tmpChar[] = new char[length];

    for(int i = 0; i < length; i++)
      tmpChar[i] = complement(seq.charAt(i));
   
    seq = new String(tmpChar);
  }


  /**
  *
  * Complement a residue 
  * @param c	residue to complement
  * @return 	complement residue (or no change if not cagt)
  *
  */
  private char complement(char c)
  {
    if(c == 't')
      return 'a';
    else if(c == 'T')
      return 'A';
    else if(c == 'a')
      return 't';
    else if(c == 'A')
      return 'T';
    else if(c == 'g')
      return 'c';
    else if(c == 'G')
      return 'C';
    else if(c == 'c')
      return 'g';
    else if(c == 'C')
      return 'G';
    return c;
  }

  public static void main(String args[])
  {
    new Sequence("Seq","ACTATACAG",9);
  }

}

