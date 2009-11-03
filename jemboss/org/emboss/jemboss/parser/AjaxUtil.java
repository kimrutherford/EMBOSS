/********************************************************************
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public
*  License along with this library; if not, write to the
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/

package org.emboss.jemboss.parser;

import java.io.*;
import java.util.*;
import javax.swing.*;

import org.emboss.jemboss.programs.ListFile;

/**
*
* Utility to get the sequence file contents or the database
* entry ready to use in Ajax
*
*/
public class AjaxUtil
{

  /**
  *
  * Returns the sequence file contents or the database entry
  * ready to use in Ajax.
  * @param fname file or database entry
  * @param db[] array of databases available
  * @param f the JFrame or null
  * @param withSoap true if being run in client-server mode
  * @return String to pass to seqType or seqsetType (if not null)
  *
  */
  public static String getFileOrDatabaseForAjax(String fname,
                     String db[], JFrame f, boolean withSoap)
  {

    String fc = new String("");
    try
    {
      String line = new String("");
      //use the first seqs we meet in a list file
      if( (fname.startsWith("@") || fname.startsWith("list::"))
          && withSoap)
      {
        Hashtable filesInList = new Hashtable();
        ListFile.parse(fname,filesInList);
        Enumeration en = filesInList.keys();
        if(en.hasMoreElements())
        {
          Object obj = en.nextElement();
          fname = new String((byte[])filesInList.get(obj));
          int col = fname.indexOf(":");
          if(col>-1)
          {
            String possDB = fname.substring(0,col);
            for(int i=0;i<db.length;i++)
              if(db[i].equalsIgnoreCase(possDB))
              {
                fname = fname.substring(0,fname.indexOf("\n"));
                break;
              }
          }
        }
      }

      if( (new File(fname)).exists() && withSoap )   // Sequence file
      {
        BufferedReader in = new BufferedReader(new FileReader(fname));
        while((line = in.readLine()) != null)
          fc = fc.concat(line + "\n");
      }
      else                                   // Database or remote file
      {
        fc = fname;
        if(fc.endsWith(":") || fc.endsWith(":*"))
        {
          int n = JOptionPane.showConfirmDialog(f,
                 "Do you really want to extract\n"+
                 "the whole of " + fc + " database?",
                 "Confirm the sequence entry",
                 JOptionPane.YES_NO_OPTION);
          if(n == JOptionPane.NO_OPTION)
            return null;
        }
      }
    }
    catch (IOException ioe)
    {
      System.out.println("Error in reading the sequence for Ajax");
    }

    return fc;

  }

}

