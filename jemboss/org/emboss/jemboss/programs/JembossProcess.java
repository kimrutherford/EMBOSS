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
*  Based on EmbreoResult
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.programs;

import java.util.Hashtable;

/**
*
* Monitor for Jemboss batch processes
*
*/
public class JembossProcess
{

  /** true when process is complete */
  private boolean completed = false;
  /** true when process is running  */
  private boolean running = true;
 
//private boolean hasresults = false;
  /** project name */
  private String project = "";
  /** project id   */
  private String jobid = "";
  /** project description */
  private String description = "";

//private Hashtable results;

  /**
  *
  * @param project	project name
  * @param jobid	project id 
  * @param description	project description
  *
  */
  public JembossProcess(String project, String jobid, String description) 
  {
    this.project = project;
    this.jobid = jobid;
    this.description = description;
  }

  /**
  *
  * @param project      project name
  * @param jobid        project id 
  *
  */
  public JembossProcess(String project, String jobid) 
  {
    this.project = project;
    this.jobid = jobid;
  }

  /**
  *
  * @param jobid        project id 
  *
  */
  public JembossProcess(String jobid) 
  {
    this.jobid = jobid;
  }

  /**
  *
  * Determine if a job has completed
  * @return	true if the process is complete
  *
  */
  public boolean isCompleted() 
  {
    return completed;
  }

  /**
  *
  * Determine if a job is running
  * @return     true if the process is running
  *
  */
  public boolean isRunning() 
  {
    return running;
  }

//public boolean hasResults() 
//{
//  return hasresults;
//}

  /**
  *
  * Get the project name
  * @return 	project name
  *
  */
  public String getProject() 
  {
    return project;
  }

  /**
  *
  * Get the project id
  * @return     project id
  *
  */
  public String getJob() 
  {
    return jobid;
  }

  /**
  *
  * Get the project description
  * @return     project description
  *
  */
  public String getDescription() 
  {
    return description;
  }

  /**
  *
  * Called when a process has completed
  *
  */
  public void complete() 
  {
    completed = true;
    running = false;
  }

  /**
  *
  * Set the description of a project
  * @param s	project description
  *
  */
  public void setDescription(String s) 
  {
    description = s;
  }

//public void setResults(Hashtable newres) 
//{
//  results = new Hashtable();
//  results = newres;
//  hasresults = true;
//}

//public Hashtable getResults() 
//{
//  return results;
//}

}
