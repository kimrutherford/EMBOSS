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

package org.emboss.jemboss.gui.startup;

import java.io.*;
import java.util.*;

/**
*
* Parses the output from showdb to find the available databases
*
*/
public class Database 
{
  /** available databases */
  private String db[];

  /**
  *
  * @param showdb	output from showdb
  *
  */
  public Database(String showdb) 
  {

    BufferedReader in;
    String line;
    try
    {
      int numdb =0;
      in = new BufferedReader(new StringReader(showdb));
      while((line = in.readLine()) != null)
        if(!line.startsWith("#"))
          numdb++;
 
      db = new String[numdb];
      int i = 0;
      in = new BufferedReader(new StringReader(showdb));
      while((line = in.readLine()) != null) 
        if(!line.startsWith("#"))
          db[i++] = new String(line.substring(0,line.indexOf(" ")));
      
    }
    catch(IOException ioe) 
    {
      System.out.println("Database class reports problem reading showdb");
    }
    
  }

  /**
  * 
  * Get the available databases
  * @return	available databases
  *
  */
  public String[] getDB() 
  {
    return db;
  } 

}

