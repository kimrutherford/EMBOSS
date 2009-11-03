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
*  Based on EmbreoFileRoots
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.util.*;
import java.io.*;

import org.emboss.jemboss.JembossParams;

/**
*
* Retrieve the list of available filesystem roots
*
*/
public class FileRoots implements Serializable 
{

  /** server directory roots */
  private Hashtable fileRoots;
  /** currently selected root */
  private String currentRoot;
  /** currently selected directory */
  private String currentDir;
  /** file system roots */
  private Vector rootVector;
  /** default root */
  private String defaultRoot;
  /** default root index */
  private int defaultRootIndex;

  /**
  *
  * @param mysettings 	jemboss properties
  * @throws JembossSoapException if connection fails
  *
  */
  public FileRoots(JembossParams mysettings) throws JembossSoapException
  {

    fileRoots = null;
    defaultRootIndex = -1;
    PrivateRequest rReq = new PrivateRequest(mysettings, "EmbreoFile", 
                                                "embreo_roots");

    fileRoots = rReq.getHash();
    if(fileRoots.containsKey("status"))
      fileRoots.remove("status");
    if(fileRoots.containsKey("msg"))
      fileRoots.remove("msg");

    if (fileRoots.containsKey("default-root")) 
    {
      defaultRoot = (String) fileRoots.get("default-root");
      currentRoot = defaultRoot;
      fileRoots.remove("default-root");
    }
    else
      defaultRoot = null;
    
    rootVector = new Vector();
    Enumeration enumer = fileRoots.keys();
    while(enumer.hasMoreElements()) 
    {
      String s = (String)enumer.nextElement();
      rootVector.add(s);
    }

    if (defaultRoot != null ) 
      defaultRootIndex = rootVector.indexOf(defaultRoot);
    
  }

  /**
  *
  * Get the list of file manager roots
  * @return	file manager roots
  *
  */
  public Hashtable getRoots() 
  {
    return fileRoots;
  }

  /**
  *
  * Get the list of file manager roots
  * @return	file manager roots
  * 
  */
  public Vector getRootVector() 
  {
    return rootVector;
  }

  /**
  *
  * Get the default root, if defined, else return null
  * @return	default root
  *
  */
  public String getDefaultRoot() 
  {
    return defaultRoot;
  }

  /**
  *
  * Get the index of the default root in the Vector, 
  * else return -1
  * @return 	default root index
  *
  */
  public int getDefaultRootIndex() 
  {
    return defaultRootIndex;
  }

  /**
  *
  * Get the currently selected directory
  * @return 	currently selected directory
  *
  */
  public String getCurrentDir() 
  {
    return currentDir;
  }

  /**
  *
  * Set the current directory
  * @param newDir	name of the new current directory
  *
  */
  public void setCurrentDir(String newDir) 
  {
    currentDir = newDir;
  }

  /**
  *
  * Get the currently selected root
  * @return	selected root 
  *
  */
  public String getCurrentRoot() 
  {
    return currentRoot;
  }

  /**
  *
  * Set the current root
  * @param newRoot 	name of the new root.
  *
  */
  public void setCurrentRoot(String newRoot)
  {
    currentRoot = newRoot;
  }

  //for Serializable
   private void writeObject(java.io.ObjectOutputStream out) throws IOException 
   {

     out.defaultWriteObject();
   }

   private void readObject(java.io.ObjectInputStream in)
     throws IOException, ClassNotFoundException 
   {
     in.defaultReadObject();
   }
}
