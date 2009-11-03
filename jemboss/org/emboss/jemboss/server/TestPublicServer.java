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

import org.emboss.jemboss.soap.GetWossname;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.JembossJarUtil;
import java.security.Security; //ssl
import java.net.*;

/**
*
* This can be used to test the public server. It is run from
* the command line:
*
* java org.emboss.jemboss.server.TestPublicServer
*
*/
public class TestPublicServer
{
    private static JembossParams mysettings;

    public TestPublicServer()
    {

      if(mysettings.getPublicSoapURL().startsWith("https") &&
         !mysettings.getUseHTTPSProxy())
      {
        System.setProperty("https.proxyHost", "");
        System.setProperty("http.proxyHost", "");
        System.setProperty("proxyHost", "");  
        String settings[] = new String[1];
        settings[0] = new String("proxy.override=true");
        mysettings.updateJembossPropStrings(settings);
      }

      if(mysettings.getPublicSoapURL().startsWith("https"))
      {
      //SSL settings
//      System.setProperty ("javax.net.debug", "all");
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
//      System.setProperty("java.protocol.handler.pkgs",
//                        "com.sun.net.ssl.internal.www.protocol");

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


      try
      {
        GetWossname ewoss = new GetWossname(mysettings);
        System.out.println(ewoss.getWossnameText());
        System.out.println("\n\nAbove are the wossname results from the server :\n"+
                                   mysettings.getPublicSoapURL());
      }
      catch(Exception e)
      {
        System.out.println("\nFailed to connect to :\n"+
                         mysettings.getPublicSoapURL());
        e.printStackTrace();
        System.exit(0);
      }

    }

    public static void main(String [] args) 
    {
      mysettings = new JembossParams();
      new TestPublicServer();
    }
}

