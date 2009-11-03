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

import java.io.*;
import java.util.Date;
import org.emboss.jemboss.programs.RunEmbossApplication2;

/**
*
* Use to run an EMBOSS application in batch mode by
* running the process in a separate thread
*
*/
public class JembossThread extends Thread
{

  /** runs the emboss process */
  private RunEmbossApplication2 rea;
  /** project directory */
  private String project;

  /**
  *
  * @param rea		runs the emboss process
  * @param project	project directory
  *
  */
  public JembossThread(RunEmbossApplication2 rea,
                       String project)
  {
    this.rea = rea;
    this.project = project;
  }
 
  public void run() 
  { 
    rea.waitFor();
    createFinishedFile();
  }
 
  /**
  *
  * Creates a file named "finished" in the project directory,
  * that contains a time stamp.
  *
  */
  private void createFinishedFile()
  {
    String fs = new String(System.getProperty("file.separator"));
    File finished = new File(project + fs + ".finished");

    try
    {
      PrintWriter fout = new PrintWriter(new FileWriter(finished));
      fout.println((new Date()).toString());
      fout.close();
    }
    catch (IOException ioe) {}
  }
 

}


