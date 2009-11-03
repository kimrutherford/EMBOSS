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
import org.emboss.jemboss.gui.sequenceChooser.SequenceFilter;

/**
*
* This class saves sequences in FASTA format.
*
*/
public class SequenceSaver
{

  /**
  *
  * @param seqs		sequences to save out
  * @param fsave	selected file to save to
  * 
  */
  public SequenceSaver(Vector seqs, File fsave)
  {
    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);
    String home = System.getProperty("user.home");
    JFileChooser fc = new JFileChooser(home);
    System.setSecurityManager(sm);

    if(fsave == null)
    {
        fsave = new File(home+System.getProperty("file.separator")+
                            ((Sequence)seqs.get(0)).getName()+".fasta");
    }
    else if(!fsave.getName().endsWith("fasta"))
    {
      String name = fsave.getName();
      int index = name.lastIndexOf(".");
      if(index > -1)
        name = name.substring(0,index);
      
      fsave = new File(home+System.getProperty("file.separator")+
                       name+".fasta");
    }

    fc.setSelectedFile(fsave);
    fc.addChoosableFileFilter(new SequenceFilter());
    int returnVal = fc.showSaveDialog(fc);

    if(returnVal == JFileChooser.APPROVE_OPTION)
    {
      File files = fc.getSelectedFile();
      saveFile(files,seqs);
    }
  }

  /**
  *
  * @param seqs         sequences to save out
  *
  */
  public SequenceSaver(Vector seqs)
  {
    this(seqs,null);
  }

  /**
  *
  * Save the sequences out
  * @param fsave	file to write to
  * @param seqs		sequences to save
  *
  */
  private void saveFile(File fsave,Vector seqs)
  {
    try
    {
      PrintWriter pw = new PrintWriter(new FileWriter(fsave));
      int jstop;
      
      for(int i=0; i<seqs.size();i++)
      {
        Sequence seq = (Sequence)seqs.get(i);
        int len = seq.getLength();
        pw.println(">"+seq.getName());
        String seqString = seq.getSequence();
// write lines of sequences of length 60
        for(int j=0;j<len;j+=60)  
        {
          jstop = j+60;
          if(jstop>len)
            jstop = len;
          pw.println(seqString.substring(j,jstop));
        }
      }
      pw.close();
    }
    catch(IOException ioe)
    {
      System.err.println("IOException saving "+fsave.getName());
    }
  } 

}

