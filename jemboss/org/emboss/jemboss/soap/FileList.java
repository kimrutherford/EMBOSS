/***************************************************************
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
*  Based on EmbreoFileList
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.util.*;
import org.emboss.jemboss.JembossParams;


/**
*
* Retrieves a directory listing from an Jemboss server.
* The directory listing consists of a hash with two entries, a full
* list of files (including directories) and a list of those files
* that are directories.
*
*/
public class FileList 
{
  
  /** file list */
  private String flist = null;
  /** directory list */
  private String directories = null;
  /** vector containing directories */
  private Vector vdir;

  /**
  *
  * @param mysettings 	jemboss properties
  * @param fileRoot 	file system root being used
  * @param dir 		directory to list files in, relative to fileRoot
  * @throws JembossSoapException if authentication fails
  *
  */
   public FileList(JembossParams mysettings, String fileRoot, String dir)
                      throws JembossSoapException 
   {

     if (mysettings.getDebug()) 
       System.out.println("FileList: start " + fileRoot + " :/: " + dir);
     
     Vector params = new Vector();
     String options= "fileroot=" + fileRoot;
     params.addElement(options);
     params.addElement(dir);

     PrivateRequest eRun;
     try
     {
       eRun = new PrivateRequest(mysettings,"EmbreoFile","directory_shortls", params);
     }
     catch (JembossSoapException e) 
     {
       throw new JembossSoapException("Directory listing call failed: "+e.getMessage());
     }

     flist = eRun.getHash().get("list").toString();
     directories = eRun.getHash().get("dirlist").toString();

     vdir = new Vector();
     StringTokenizer tok = new StringTokenizer(directories,"\n");
     while (tok.hasMoreTokens())
       vdir.add(tok.nextToken());

     if (mysettings.getDebug()) 
       System.out.println("FileList: done");
     
   }


  /**
  *
  * Gets the list of files as a Vector
  * @return 	list of files as a Vector
  *
  */
  public Vector fileVector() 
  {
    Vector v = new Vector();
    StringTokenizer tok = new StringTokenizer(flist,"\n");
    while (tok.hasMoreTokens()) 
      v.add(tok.nextToken());

    Object list[] = v.toArray();
    java.util.Arrays.sort(list);
    for(int i=0; i<list.length; i++)
      v.set(i,(String)list[i]);
    
    return v;
  }


  /**
  *
  * Gets whether this name is a directory
  * @return 	true if it is a directory
  *
  */
  public boolean isDirectory(String d) 
  {
    return vdir.contains(d);
  }

}
