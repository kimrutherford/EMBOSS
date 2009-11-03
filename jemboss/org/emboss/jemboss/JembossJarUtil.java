/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss;

import java.util.*;
import java.util.zip.*;
import java.io.*;


/**
*
* Unpacks a Jar file into a Hashtable
*
*/
public class JembossJarUtil 
{

  /** Hashtable containing the unpacked contents of the jar file */
  private Hashtable jarStore = new Hashtable();

  /**
  *
  * Given the path to a jar file unpack to a hashtable
  * @param jarFile	path to jar file to unpack
  * @throws Exception  	if it is not possible to read jar file
  *
  */
  public JembossJarUtil(String jarFile) throws Exception
  {

    try 
    {
      // extracts just sizes only
      ClassLoader cl = this.getClass().getClassLoader();
      ZipInputStream zis= new ZipInputStream(
                     cl.getResourceAsStream(jarFile));
      ZipEntry ze=null;
      Hashtable htSizes = new Hashtable();

      while((ze=zis.getNextEntry())!=null)
      {
        int ret=0;
        int cnt=0;
        int rb = 0;
        while(ret != -1)
        {
          byte[] b1 = new byte[1];
          ret=zis.read(b1,rb,1);
          cnt++;
        }
        htSizes.put(ze.getName(),new Integer(cnt));
      }
      zis.close();

      // extract resources and put them into the hashtable
      zis = new ZipInputStream(cl.getResourceAsStream(jarFile));
      ze=null;
      while ((ze=zis.getNextEntry())!=null) 
      {
        if(ze.isDirectory()) 
          continue;
         
        int size=(int)ze.getSize(); // -1 means unknown size
        if(size==-1) 
          size=((Integer)htSizes.get(ze.getName())).intValue();
         
        byte[] b=new byte[(int)size];
        int rb=0;
        int chunk=0;
        while (((int)size - rb) > 0) 
        {
          chunk=zis.read(b,rb,(int)size - rb);
          if(chunk==-1) 
            break;
          rb+=chunk;
        }

        // add to internal resource hashtable 
        jarStore.put(ze.getName(),b);

//      System.out.println(ze.getName());
      }
      zis.close();
    }
    catch (Exception e) { throw new Exception();}

//  catch (NullPointerException e) 
//  {
//    System.out.println("JembossJarUtil Error: jarStore");
//  } 
//  catch (FileNotFoundException e) 
//  {
//    e.printStackTrace();
//  }
//  catch (IOException e)
//  {
//    e.printStackTrace();
//  }
  }


  /**
  *
  * Return the hashtable
  * @return jarStore	the hashtable containing the contents 
  *  			of the jar
  *
  */
  public Hashtable getHash()
  {
    return jarStore;
  }

  /**
  *
  * Return an element of the hashtable
  * @param el	key of an element in the hashtable
  * @return 	the hashtable containing the contents 
  *            	of the jar
  *
  */
  public Object getElement(String el)
  {
    return jarStore.get(el);
  }

  /**
  *
  * Write contents of an element in the hashtable
  * @param el   key of an element in the hashtable
  * @param f    path of file to write to
  * @return     true if written file
  *
  */
  public boolean writeByteFile(String el, String f)
  {
    try
    {
      FileOutputStream out = new FileOutputStream(f);
      out.write((byte []) jarStore.get(el));
      out.close();
    }
    catch(FileNotFoundException fnfe) {return false;}
    catch(IOException ioe) {return false;}
 
    return true;
  }

}
