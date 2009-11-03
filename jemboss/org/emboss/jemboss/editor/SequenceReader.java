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
import java.io.*;
import java.util.Vector;
import java.util.Hashtable;
import java.util.StringTokenizer;

import org.emboss.jemboss.gui.sequenceChooser.SequenceFilter;

/**
*
* This class reads sequences in FASTA and MSF format.
* The sequences are read and stored as a Vector.
*
*/
public class SequenceReader
{

  /** sequence file */
  private File seqFile;
  /** sequence store */
  private Vector seqs;
  /** true if read ok */
  private boolean reading = false;

  public SequenceReader()
  {
    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);
    JFileChooser fc = new JFileChooser(System.getProperty("user.home"));
    System.setSecurityManager(sm);

    fc.addChoosableFileFilter(new SequenceFilter());
    int returnVal = fc.showOpenDialog(fc);
  
    if(returnVal == JFileChooser.APPROVE_OPTION)
    {
      seqFile = fc.getSelectedFile();
      readSequenceFile();
      reading = true;
    }
  } 

  /**
  *
  * @param seqFile	sequence file
  *
  */
  public SequenceReader(File seqFile)
  {
    this.seqFile = seqFile;
    readSequenceFile();
    reading = true;
  }

  /**
  *
  * @param seqString	sequence
  *
  */
  public SequenceReader(String seqString)
  {
    readSequenceString(seqString);
    reading = true;
  }

  /**
  *
  *
  */
  public boolean isReading()
  {
    return reading;
  }

  /**
  *
  * Read a sequence file and create a vector of sequences
  * @return 	vector of Sequence objects
  *
  */
  public Vector readSequenceFile()
  {
    BufferedReader in = null;
    try
    {
      in = new BufferedReader(new FileReader(seqFile));
      String line = in.readLine();

// fasta
      if(line.startsWith(">"))
        return readFastaFile(new BufferedReader(new FileReader(seqFile)));

// msf
      int index = line.indexOf("PileUp");
      if(index > -1)
        return readMSFFile(new BufferedReader(new FileReader(seqFile)));
      index = line.indexOf("!!AA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new FileReader(seqFile)));
      index = line.indexOf("!!NA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new FileReader(seqFile)));

// clustal 
      index = line.indexOf("CLUSTAL");
      if(index > -1)
        return readClustalFile(new BufferedReader(new FileReader(seqFile)));

//jpred
      index = line.indexOf(":");
      if(index > -1)
      {
        if((index = line.indexOf(",",index)) > -1)
        {
          int index1 = line.indexOf(",",index+1);
          if(index1 > -1 && (index1-index) == 2)
          {
            index = line.indexOf(",",index1+1);
            if(index > -1 && (index-index1) == 2)
              readJPredFile(new BufferedReader(new FileReader(seqFile)));
          }
        }
      }

    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Error");
    }
    return null;
  }

  /**
  *
  * Read a formatted sequence string (e.g msf) and create a vector
  * of sequences
  * @return     vector of Sequence objects
  *
  */
  public Vector readSequenceString(String seqString)
  {
    BufferedReader in = null;
    try
    {
      in = new BufferedReader(new StringReader(seqString));
      String line = in.readLine();

// fasta
      if(line.startsWith(">"))
        return readFastaFile(new BufferedReader(new StringReader(seqString)));

// msf
      int index = line.indexOf("PileUp");
      if(index > -1)
        return readMSFFile(new BufferedReader(new StringReader(seqString)));
      index = line.indexOf("!!AA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new StringReader(seqString)));
      index = line.indexOf("!!NA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new StringReader(seqString)));

// clustal
      index = line.indexOf("CLUSTAL");
      if(index > -1)
        return readClustalFile(new BufferedReader(new StringReader(seqString)));

//jpred 
      index = line.indexOf(":");
      if(index > -1)
      {
        if((index = line.indexOf(",",index)) > -1)
        {
          int index1 = line.indexOf(",",index+1);
          if(index1 > -1 && (index1-index) == 2)
          {
            index = line.indexOf(",",index1+1);
            if(index > -1 && (index-index1) == 2)
              readJPredFile(new BufferedReader(new StringReader(seqString)));
          }
        }
      }

    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Error");
    }
    return null;
  }

  /**
  *
  * Reads in the FASTA sequence file and creates a Vector 
  * containing the sequence(s).
  * @param in	buffered reader 
  *
  */
  public Vector readFastaFile(BufferedReader in)
  {
    seqs = new Vector();
//  BufferedReader in = null;
    String seqString = "";
    
    try
    {
//    in = new BufferedReader(new FileReader(seqFile));
      String line;
      String name = null;
      Sequence seq;

      while((line = in.readLine()) != null )
      {
        if(line.startsWith(">"))
        {
          if(!seqString.equals(""))
          {
            seqs.add(new Sequence(name,seqString));
            seqString = "";
            name = line.substring(1);
          }
          else if(line.startsWith(">") && seqString.equals(""))
            name = line.substring(1);
        }
        else
          seqString = seqString.concat(line);
      }
      in.close();

      if(!seqString.equals(""))
        seqs.add(new Sequence(name,seqString));
    }
    catch (IOException e)
    {
      System.out.println("SequenceReader FASTA Error");
    }
    return seqs;
  }

  /**
  *
  * Reads in the MSF sequence file and creates a Vector
  * containing the sequence(s).
  * @param in	buffered reader
  */
  public Vector readMSFFile(BufferedReader in)
  {
    seqs = new Vector();
//  BufferedReader in = null;
    String seqString = "";

    try
    {
//    in = new BufferedReader(new FileReader(seqFile));
      String line;
      Sequence seq = null;
      String type = null;
      String bit;
      StringTokenizer st;
      boolean header = true;
      Hashtable seqIndex = new Hashtable();
      int num = 0;

      while((line = in.readLine()) != null )
      {
        if(line.startsWith("//"))  //end of header
          header = false;
        if(line.equals(""))
          continue;

        if(header)      // read the MSF header
        {
          st = new StringTokenizer(line," ");
          while (st.hasMoreTokens())
          {
            bit = st.nextToken(" ").trim();
            if(bit.startsWith("Name:"))
            {
              String name = st.nextToken(" ").trim();
              seq = new Sequence(name,"");
              seqs.add(num,seq);

              if(type != null)
              {
                if(type.equalsIgnoreCase("P"))
                  seq.setType(true);
                else if(type.equalsIgnoreCase("N"))
                  seq.setType(false);
              }   
              seqIndex.put(name,new Integer(num));
              num++;
            }
            else if(bit.startsWith("Weight:"))
              seq.setWeight(Float.parseFloat(st.nextToken().trim()));
            else if(bit.startsWith("Type:"))
              type = st.nextToken(" ").trim();
          }
        }
        else      // read the MSF sequences
        {
          int index = line.indexOf(" ");
          if(index > -1)
          {
            String name = line.substring(0,index);
            if(!seqIndex.containsKey(name))
              System.out.println("Error reading sequence ");
            else
            {
              st = new StringTokenizer(line.substring(index)," ");
              seqString = new String();
              while (st.hasMoreTokens())
                seqString = seqString.concat(st.nextToken(" ").trim());

              int seqInd = ((Integer)seqIndex.get(name)).intValue();
              seq = (Sequence)seqs.elementAt(seqInd);
              seq.appendToSequence(seqString);
            }
          }
        }
      }
      in.close();

    }
    catch (IOException e)
    {
      System.out.println("SequenceReader MSF Error");
    }
    return seqs;
  }


  /**
  *
  * Reads in the CLUSTAL sequence file and creates a Vector
  * containing the sequence(s).
  * @param in   buffered reader
  */
  public Vector readClustalFile(BufferedReader in)
  {
    seqs = new Vector();
    String seqString = "";

    try
    {
      String line;
      Sequence seq;
      String type = null;
      StringTokenizer st;
      Hashtable seqIndex = new Hashtable();
      int num = 0;

      while((line = in.readLine()) != null )
      {
        line = line.trim();
        if(line.equals(""))
          continue;
        if(line.startsWith("CLUSTAL "))
          continue;

        int index = line.indexOf(" ");
        if(index > -1)
        {
          String name = line.substring(0,index);
          if(!seqIndex.containsKey(name))
          {
            seqIndex.put(name,new Integer(num));
            seq = new Sequence(name,"");
            seqs.add(num++,seq);
          }
  
          st = new StringTokenizer(line.substring(index)," ");
          seqString = new String();
          while (st.hasMoreTokens())
            seqString = seqString.concat(st.nextToken(" ").trim());

          int seqInd = ((Integer)seqIndex.get(name)).intValue();
          seq = (Sequence)seqs.elementAt(seqInd);
          seq.appendToSequence(seqString);
        }

      }
    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Clutal Error");
    }
    return seqs;

  }


  /**
  *
  * Reads in the JPred sequence file and creates a Vector
  * containing the sequence(s).
  * @param in   buffered reader
  */
  public Vector readJPredFile(BufferedReader in)
  {
    seqs = new Vector();

    System.out.println("readJPredFile");
    try
    {
      String line;
      String name;
      StringBuffer seqString;
      Sequence seq;
      StringTokenizer st;
      while((line = in.readLine()) != null )
      {
        line = line.trim();
        if(line.equals(""))
          continue;
        int index = line.indexOf(":");
        if(index > -1)
        {
          int index1 = line.indexOf(";")+1;
          if(index1 < 0)
            index1 = 0;
          name = line.substring(index1,index);
          line = line.substring(index+1);
        }
        else
          continue;
        st = new StringTokenizer(line,",");
        seqString = new StringBuffer();
        while(st.hasMoreTokens())
          seqString = seqString.append(st.nextToken(",").trim());
        seq = new Sequence(name,seqString.toString());
        seqs.add(seq);
      }
    }
    catch (IOException e)
    {
      System.out.println("SequenceReader JPred Error");
    }
    return seqs;
  }


  /**
  *
  * Returns the number of sequences.
  * @return number of sequences
  *
  */
  public int getNumberOfSequences()
  {
    return seqs.size();
  }

  /**
  *
  * Returns the sequence at a given position in
  * the Sequence Vector store.
  * @return	sequence object
  *
  */
  public Sequence getSequence(int index)
  {
    return (Sequence)seqs.get(index);
  }

  /**
  *
  * Returns the Sequence Vector store
  * @return	collection of Sequence objects
  *
  */
  public Vector getSequenceVector()
  {
    return seqs;
  }

  /**
  *
  * Get the sequence file
  * @return	sequence file
  *
  */
  public File getSequenceFile()
  {
    return seqFile;
  }
  
}

