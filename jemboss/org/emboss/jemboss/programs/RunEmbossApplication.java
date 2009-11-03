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

package org.emboss.jemboss.programs;

import java.io.*;


/**
*
* RunEmbossApplication class used to run an EMBOSS process
*
*/
public class RunEmbossApplication
{

  /** running process */
  private Process p;
  /** standard out */
  private String stdout;
  /** running directory */
  private File project;
  /** status of the process */
  private String status;

  /**
  *
  * @param embossCommand	emboss command to run
  * @param envp			environment
  * @param project		running directory
  *
  */
  public RunEmbossApplication(String embossCommand, String[] envp, File project)
  {
    this.project = project;
    status = "0";

    Runtime embossRun = Runtime.getRuntime();
    try
    {
      p = embossRun.exec(embossCommand,envp,project);
    }
    catch(IOException ioe)
    {
      System.out.println("RunEmbossApplication Error executing: "+
                          embossCommand);
      status = "1";
    }
  }

  /**
  *
  * @return true if there is any standard out
  *
  */
  public boolean isProcessStdout()
  {
    stdout = "";

    BufferedInputStream stdoutStream = null;
    BufferedReader stdoutRead = null;
    try
    {
      String line;
      stdoutStream =
         new BufferedInputStream(p.getInputStream());
      stdoutRead =
         new BufferedReader(new InputStreamReader(stdoutStream));

      if((line = stdoutRead.readLine()) != null)
      {
        stdout = stdout.concat(line + "\n");
        while((line = stdoutRead.readLine()) != null)
          stdout = stdout.concat(line + "\n");
    
        if(project != null)
        {
          PrintWriter out = null;
          try
          {
            File so = new File(project.getCanonicalPath() + "/stdout");
            so.createNewFile();
            out = new PrintWriter(new FileWriter(so));
            out.println(stdout);
          }
          catch(IOException ioe)
          {
            System.err.println("RunEmbossApplication: Error writing" + 
                              project.getCanonicalPath() + "/stdout");
          }
          finally
          {
            if(out!=null)
              out.close();
          }
        }
      }
    }
    catch (IOException io)
    {
      System.err.println("RunEmbossApplication: Error in "+ 
                                "collecting standard out");
    }
    finally
    {
      try
      {
        if(stdoutStream!=null)
          stdoutStream.close();
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication: Error closing stream");
      } 
      try
      {
        if(stdoutRead!=null)
          stdoutRead.close();
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication: Error closing reader");
      }
    }
 
    boolean std = false;
    if(!stdout.equals(""))
      std = true;

    return std;
  }


  /**
  *
  * @return standard out
  *
  */
  public String getProcessStdout()
  {
    return stdout;
  }

  /**
  *
  * @return process
  *
  */
  public Process getProcess()
  {
    return p;
  }

  /**
  *
  * @return status
  *
  */
  public String getStatus()
  {
    return status;
  }

}

