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

import org.emboss.jemboss.soap.MakeFileSafe;
import org.emboss.jemboss.soap.JembossRun;
import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.programs.ListFile;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.JembossJarUtil;
import java.security.Security; //ssl
import java.net.*;
import java.util.Hashtable;
import java.util.Enumeration;
import java.io.*;


/**
*
* This can be used to test the private server. It is run from
* the command line and requires the username, password and a 
* sequence file as input:
*
* java org.emboss.jemboss.server.TestPrivateServer username passwd app ~/pax.msf
*
* If it connects correctly to the server it will run an emboss application,
* and return the output.
*
* This works with 'cons' and 'seqret'.
*
*/

public class TestPrivateServer
{
  private static JembossParams mysettings;
  private static String filename;
  private static String prog;

  public TestPrivateServer()
  {

    if(mysettings.getPublicSoapURL().startsWith("https"))
    {
    //SSL settings
//    System.setProperty ("javax.net.debug", "all");
      com.sun.net.ssl.internal.ssl.Provider p =
                   new com.sun.net.ssl.internal.ssl.Provider();
      Security.addProvider(p);

      //have to do it this way to work with JNLP
      URL.setURLStreamHandlerFactory( new URLStreamHandlerFactory()
      {
        public URLStreamHandler createURLStreamHandler(final String protocol)
        {
          if(protocol != null && protocol.compareTo("https") == 0)
          {
            return new com.sun.net.ssl.internal.www.protocol.https.Handler();
          }
          return null;
        }
      });

      //location of keystore
      System.setProperty("javax.net.ssl.trustStore",
                        "resources/client.keystore");

      String jembossClientKeyStore = System.getProperty("user.home") +
                          "/.jembossClientKeystore";

      try
      {
        new JembossJarUtil("resources/client.jar").writeByteFile(
                     "client.keystore",jembossClientKeyStore);
        System.setProperty("javax.net.ssl.trustStore",
                            jembossClientKeyStore);
      }
      catch(Exception exp){}

    }

    Hashtable filesToMove = new Hashtable();
    String embossCommand = prog + " ";

    String val = "sequence"; 

    embossCommand = filesForSoap(filename,embossCommand,val,filesToMove);
    embossCommand = embossCommand.concat(" -auto");

    if(mysettings.getUseAuth() == true)
      if(mysettings.getServiceUserName() == null)
        System.out.println("OOPS! Authentication required!");

    // submit to private server
    try
    {
      JembossRun thisrun = new JembossRun(embossCommand,"",
                                    filesToMove,mysettings);
      Hashtable h = thisrun.hash();
      Enumeration enums = h.keys();
      while (enums.hasMoreElements())
      {
        String thiskey = (String)enums.nextElement().toString();
        Object res = h.get(thiskey);
        if(res instanceof String)
          System.out.println((String)res);
        else
          System.out.println(new String((byte[])h.get(thiskey)));
      }

//    new ShowResultSet(h,filesToMove);      

    }
    catch (JembossSoapException eae)
    {
      System.out.println("SOAP ERROR :"+embossCommand);
    }

  }


  private String filesForSoap(String fn, String options, String val,
                             Hashtable filesToMove)
  {

    String sfn;

    if (fn.startsWith("@")||fn.startsWith("list::")||
        fn.startsWith("internalList::"))        // list file
    {
      String lfn = "";
      if (fn.startsWith("@"))
        lfn = fn.substring(1);
      else if(fn.startsWith("list::"))
        lfn = fn.substring(6);

      File inFile = new File(lfn);
      if( (inFile.exists() && inFile.canRead() &&
           inFile.isFile())||
           fn.startsWith("internalList::") )    // local list file
      {
        ListFile.parse(fn, filesToMove);
        if(fn.startsWith("internalList::"))
          options = options.concat(" -" + val + " list::internalList");
        else
        {
          MakeFileSafe sf = new MakeFileSafe(lfn);
          String sfs = sf.getSafeFileName();
          options = options.concat(" -" + val + " list::" +  sfs);
        }
      }
      else                                      // presume remote
      {
//      System.out.println("Can't find list file "+lfn);
        options = options.concat(" -" + val + " list::" +  lfn);
      }

      sfn=lfn;
    }
    else                                        // not list file
    {
      MakeFileSafe sf = new MakeFileSafe(fn);
      sfn = sf.getSafeFileName();

      File inFile = new File(fn);
      if(inFile.exists() && inFile.canRead()
                         && inFile.isFile())    // read & add to transfer list
      {
        filesToMove.put(sfn,getLocalFile(inFile));
        options = options.concat(" -" + val + " " +  sfn);
      }
      else     //presume remote
      {
//      System.out.println("Can't find plain file "+fn);
        options = options.concat(" -" + val + " " +  fn);
      }
    }

    return options;
  }

  private byte[] getLocalFile(File name)
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



  public static void main(String [] args) 
  {
    mysettings = new JembossParams();
    mysettings.setServiceUserName(args[0]);
    mysettings.setServicePasswd(args[1].toCharArray());
    prog     = args[2];
    filename = args[3];

    new TestPrivateServer();
  }
}

