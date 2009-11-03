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

import java.security.Security; //ssl
import java.net.*;

import org.emboss.jemboss.soap.AuthPopup; 
import org.emboss.jemboss.soap.JembossSoapException;

/**
*
* Display the local and remote file managers
*
*/
public class FileManager
{

  public FileManager()
  {
    String fs = new String(System.getProperty("file.separator"));
    JembossParams mysettings = new JembossParams();


    if(mysettings.getPublicSoapURL().startsWith("https"))
    {
      //SSL settings
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
                       fs + ".jembossClientKeystore";

      try
      {
        new JembossJarUtil("resources/client.jar").writeByteFile(
                     "client.keystore",jembossClientKeyStore);
        System.setProperty("javax.net.ssl.trustStore",
                            jembossClientKeyStore);
      }
      catch(Exception exp){}
    }

    if(mysettings.getPublicSoapURL().startsWith("https"))
    {
      System.setProperty("https.proxyHost", "");
      System.setProperty("http.proxyHost", "");
      System.setProperty("proxyHost", "");
      String settings[] = new String[1];
      settings[0] = new String("proxy.override=true");
      mysettings.updateJembossPropStrings(settings);
    }

    AuthPopup fap = new AuthPopup(mysettings,0);
    fap.addBottomPanel();
    fap.setSize(380,170);
    fap.pack();
    fap.setVisible(true);
  }

  public static void main(String args[])
  {
    new FileManager();
  }


}


