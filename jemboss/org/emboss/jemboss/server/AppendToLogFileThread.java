/****************************************************************
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/
 
package org.emboss.jemboss.server;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;

/**
*
* Write to a log file in a separate thread
*
*/
public class AppendToLogFileThread extends Thread
{
 
  private String logEntry;
  private String logFileName;
  private boolean memLog;

  /**
  *
  * @param logEntry	entry to add to a log file
  * @param logFileName 	name of the log file
  * @param memLog	true if to include a log of memory usage
  *
  */
  public AppendToLogFileThread(String logEntry, String logFileName,
                               boolean memLog)
  {
    this.logEntry = logEntry;
    this.logFileName = logFileName;
    this.memLog = memLog;
  } 

  /**
  *
  * Override Thread run() method
  *
  */
  public void run()
  {
 
    if(memLog)   // include memory usage
    {
      long memTot  =  Runtime.getRuntime().totalMemory();
      long memFree = Runtime.getRuntime().freeMemory();
      logEntry = logEntry.concat("\nTotal Memory : "+memTot+
                  " bytes, Free Memory : "+memFree+" bytes \n");
    }

    BufferedWriter bw = null;
    try
    {
      bw = new BufferedWriter(new FileWriter(logFileName, true));
      bw.write(logEntry);
      bw.newLine();
      bw.flush();
    }
    catch (Exception ioe)
    {
      System.out.println("Error writing to log file "+logFileName);
      ioe.printStackTrace();
    }
    finally                     // always close the file
    {
      if(bw != null)
        try
        {
          bw.close();
        }
        catch (IOException ioe2) {}
    }
  }

}

