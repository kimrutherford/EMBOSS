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
*  Based on EmbreoListFile but adding in list files from list pane.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/


package org.emboss.jemboss.programs;


import java.io.*;
import java.util.Hashtable;

import org.emboss.jemboss.soap.MakeFileSafe;

/**
*
* Parse an EMBOSS list file, reading it in line by line, loading other
* list files recursively if necessary, and loading any files
* referred to into the filesToMove hash
*
*/
public class ListFile 
{

  /**
  *
  * @param fn  		name of the list file
  * @param filesToMove  hash to put the files into
  *
  */
  public static void parse(String fn, Hashtable filesToMove) 
  {

    String lfn = trim(fn);
    File inFile = new File(lfn);


    if((inFile.exists() && inFile.canRead() && inFile.isFile()) 
        || fn.startsWith("internalList::")) 
    {
      MakeFileSafe sf;
      String sfs = new String("internalList");  //default name
      if(inFile.exists())
      {
        sf = new MakeFileSafe(lfn);
        sfs = sf.getSafeFileName();
      }

      BufferedReader in=null;
      try 
      {
        if(fn.startsWith("internalList::"))
        {
          fn = fn.substring(14);
          in = new BufferedReader(new StringReader(fn)); 
          in.readLine();
        }
        else
          in = new BufferedReader(new FileReader(inFile));
        
        String listfile = "";
        String line;
        while((line = in.readLine()) != null) 
        {
          String arrayOfFiles[] = expandWildcards(line);
          if(arrayOfFiles != null)
          {
            for(int i=0;i<arrayOfFiles.length;i++)
              listfile = addFileToList(arrayOfFiles[i],listfile,
                                                   filesToMove);
          }
          else
          {
            if (line.startsWith("@")||line.startsWith("list::")) 
            {
              lfn = trim(line);
	      
              if((new File(lfn)).exists()) 
              {
                MakeFileSafe lf = new MakeFileSafe(lfn);
	        String slf = lf.getSafeFileName();

	        if(!filesToMove.containsKey(slf)) 
	          ListFile.parse(lfn, filesToMove);
	        listfile = listfile.concat("@"+slf+"\n");
	      }
              else 
              {
	        listfile = listfile.concat(line+"\n");  //remote list file
	      }
	    } 
            else               // plain sequence file
            {
              listfile = addFileToList(line,listfile,filesToMove);    
	    }
          }
        }
	
        filesToMove.put(sfs,listfile.getBytes());   // add list file itself
      }
      catch (IOException e) {}
      finally
      {
    	  if (in!=null)
			try {
				in.close();
			} catch (IOException e) {
			}
      }
    } 
    
    return;
  }

  /**
  *
  * Add a file to the list
  * @param line		file to add
  * @param listfile	list file
  * @param filesToMove	files to move to the server
  * @return		updated list file
  *
  */
  private static String addFileToList(String line, String listfile,
                                           Hashtable filesToMove)
  {
    File pFile = new File(line);
    if(pFile.exists() && pFile.canRead() && pFile.isFile())
    {
      MakeFileSafe pf = new MakeFileSafe(line);
      String spf = pf.getSafeFileName();

      if(!filesToMove.containsKey(spf))   // read if we haven't already
          filesToMove.put(spf,getLocalFile(pFile));
      
      // add the server reference to the listfile
      listfile = listfile.concat(spf+"\n");
    }
    else    // presumably remote
    {
      listfile = listfile.concat(line+"\n");
    }
    return listfile;
  }

  /**
  *
  * Read file in as a byte array.
  * @param name	file to read
  * @return 	byte array contents of file
  *
  */
  private static byte[] getLocalFile(File name)
  {
    byte[] b = null;
    try
    {
      long s = name.length();
      b = new byte[(int)s];
      FileInputStream fi = new FileInputStream(name);
      fi.read(b);
      fi.close();
    }
    catch (IOException ioe)
    {
      System.out.println("Cannot read file: " + name);
    }
    return b;
  }


  /**
  *
  * Wildcard (*) is allowed in the filelist data types.
  * This function will expand this to find all files with
  * that prefix.
  * @param line	line to expand
  * @return	list file
  *
  */
  private static String[] expandWildcards(String line)
  {

    int wildIndex = line.indexOf("*");
    if(wildIndex > -1)
    {
      String fs = System.getProperty("file.separator");
      int index = line.lastIndexOf(fs);
      if(index>0)
      {
        String path = line.substring(0,index);
        File pathDir = new File(path);

        if(pathDir.exists())   // are these local files
        {
          final String prefix = line.substring(index+1,wildIndex);
          String suff = "";
          if(wildIndex+1 < line.length())
            suff = line.substring(wildIndex+1,line.length());
          final String suffix = suff;

          String listFiles[] = pathDir.list(new FilenameFilter()
          {
            public boolean accept(File cwd, String name)
            {
              return (name.startsWith(prefix) &&
                      name.endsWith(suffix));
            };
          });
 
          for(int i=0;i<listFiles.length;i++)
            listFiles[i] = path.concat(fs+listFiles[i]);
      
          if(listFiles.length > 0)
            return listFiles;
        }
      }
    }
    return null;
  }

  /**
  *
  * Given a list file @file or list::file return just the file name
  * @param fn 	file name
  * @return	trimmed file name
  *
  */
  private static String trim(String fn) 
  {
    String lfn;
    if (fn.startsWith("@")) 
      lfn = new String(fn.substring(1));
    else if (fn.startsWith("list::")) 
      lfn = new String(fn.substring(6));
    else 
      lfn = fn;
    return lfn;
  }
 
}
