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
public class GroutSequence extends Sequence
{

		int indent = 0;

  /**
  *
  * @param name		sequence name
  * @param seq		sequence string
  * @param length	sequence length
  * @param weight	sequence weight
  *
  */
  public GroutSequence(String name, String seq, int length,
                  float wt)
  {
			super(name, seq, length, wt);
  }

  /**
  *
  * @param name         sequence name
  * @param seq          sequence string
  * @param length       sequence length
  *
  */
  public GroutSequence(String name, String seq, int length)
  {
			super(name,seq,length,1.0f);
  }

  /**
  *
  * @param name         sequence name
  * @param seq          sequence string
  *
  */
  public GroutSequence(String name, String seq)
  {
			super(name,seq,seq.length());
  }

  /**
  *
  * @param seq          sequence string
  *
  */
  public GroutSequence(String seq)
  {
			super(new String(""),seq);
  }

  /**
  *
  * @param seq          sequence string
  * @param weight       sequence weight
  *
  */
  public GroutSequence(String seq, float wt)
  {
			super(new String(""),seq,seq.length(),wt);
  }

  /**
  *
  * Insert a residue into the sequence
  * @param s	residue to insert
  * @param pos	position to insert into sequence
  *
  */
  public void setIndent(int newIndent)
		{
				// System.out.println("alignEditor setIndents to " + newIndent +  " from " + indent);
				/* this is the old way, got to be better */
				char[] array = new char[newIndent];
				String stringEg = new String("-");
				char charEg = stringEg.charAt(0);
				for(int i = 0; i < newIndent; ++i)
				{
						array[i] = charEg;
				}
				String toAdd = new String(array);
				seq = toAdd + seq.substring(indent);
				indent = newIndent;
				length = seq.length();
				// System.out.println("length = " + length);
				/* this is another way, but it has the same bug */
				/*
					for(int i = 0; i < indent; ++i)
					{
					deleteResidue(0);
					}
					for(int i = 0; i < newIndent; ++i)
					{
					insertResidue("-", 0);
					}
					indent = newIndent;
				*/
				
				/*
					seq = seq.substring(0,pos)+s+
          seq.substring(pos);
					length++;
				*/
		}

}

