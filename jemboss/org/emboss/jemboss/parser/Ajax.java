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
*  @author: Copyright (C) Alan Bleasby 
*
********************************************************************/

package org.emboss.jemboss.parser;

import java.io.*;
import java.util.*;
import javax.swing.*;

import org.emboss.jemboss.programs.ListFile;

/**
*
* Used with JNI to access EMBOSS ajax library. 
* This is used to determine sequence attributes and
* authenticate the server methods.
*
*/
public class Ajax
{

  /** true if the sequence is protein */
  public static boolean protein;
  /** sequence length */
  public static int     length;
  /** sequence weight */
  public static float   weight;

  /** determine sequence attributes     */
  public native boolean seqType(String usa);
  /** determine sequence set attributes */
  public native boolean seqsetType(String usa);

  /** true if the sequence is protein */
  public boolean protein_soap;
  /** sequence length */
  public int     length_soap;
  /** sequence weight */
  public float   weight_soap;

  /** determine sequence attributes as the user */
  public native boolean seqAttrib(String username,
               byte[] password, String environment,
               String usa);    
  /** determine sequence set attributes as the user */
  public native boolean seqsetAttrib(String username,
               byte[] password, String environment,
               String usa);

  /** user home dir */
  public String home;

  /** authentication method */
  public synchronized native boolean userAuth(String username,
          byte[] password, String environment);
  /** fork emboss proceess */
  public synchronized native boolean forkEmboss(String username,
               byte[] password, String environment,
               String commandline, String directory);
  /** fork emboss batch proceess */
  public synchronized native boolean forkBatch(String username,
               byte[] password, String environment,
               String commandline, String directory);
  /** make a directory */
  public synchronized native boolean makeDir(String username,
               byte[] password, String environment,
               String directory);
  /** delete a file */
  public synchronized native boolean delFile(String username,
               byte[] password, String environment,
               String filename);
  /** rename a file */
  public synchronized native boolean renameFile(String username,
               byte[] password, String environment,
               String filename, String filename2);
  /** delete a directory */
  public synchronized native boolean delDir(String username,
               byte[] password, String environment,
               String directory);
  /** list files in a directory */
  public synchronized native boolean listFiles(String username,
               byte[] password, String environment,
               String directory);
  /** list directorys */
  public synchronized native boolean listDirs(String username,
               byte[] password, String environment,
               String directory);
  /** get the contents of a file */
  public synchronized native byte[] getFile(String username,
               byte[] password, String environment,
               String filename);
  /** create a file */
  public synchronized native boolean putFile(String username,
               byte[] password, String environment,
               String filename, byte[] bytearray);

  /** stdout */
  private String outStd;
  /** stderr */
  private String errStd;
  private int size;
  private int prnt;
  private int fileok;

//public native boolean fork(String cmdLine, String envp,
//                        String dir, int uid, int gid);

  static
  {
    try
    {
      System.loadLibrary("ajax");
    }
    catch(UnsatisfiedLinkError e) 
    {
      e.printStackTrace();
      if(e.getMessage().indexOf("already loaded in another classloader") 
                               != -1)
        System.err.println(e);
      else
        throw e;
    }
  }

  /**
  *
  * Sets the sequence length
  * @param length	 sequence length
  *
  */
  public void setLength(int length)
  {
    this.length = length;
  }

  /**
  *
  * Sets the sequence weight
  * @param weight 	sequence weight
  *
  */
  public void setWeight(float weight)
  {
    this.weight = weight;
  }

  /**
  *
  * Sets whether sequence is protein (true)
  * @param protein 	sequence type
  *
  */
  public void setProtein(boolean protein)
  {
    this.protein = protein;
  }

  /**
  *
  * Get the stdout
  * @return 	stdout
  *
  */
  public String getOutStd()
  {
    return outStd;
  }
 
  /**
  *
  * Get the stderr
  * @return     stderr
  *
  */
  public String getErrStd()
  {
    return errStd;
  }

  /**
  *
  * Clear the stderr
  * 
  */
  public void setErrStd()
  {
    errStd = new String();  
  }

  /**
  *
  *
  *
  */
  public int getSize()
  {
    return size;
  }

  public int getPrnt()
  {
    return prnt;
  }

  public int getFileok()
  {
    return fileok;
  }

}

