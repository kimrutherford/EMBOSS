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
*  Based on EmbreoPublicRequest
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

import javax.xml.namespace.QName;

import org.apache.axis.client.Call;
import org.apache.axis.encoding.XMLType;
import org.emboss.jemboss.JembossParams;

/**
*
* Make a axis call to a public server, using the default service
*
*/
public class PublicRequest
{

  private Hashtable proganswer;

  /**
  *
  * @param mysettings 	jemboss properties
  * @param method     	method to call
  * @throws JembossSoapException if call to web service fails
  *
  */
   public PublicRequest(JembossParams mysettings, String method)
               throws JembossSoapException
   {
     this(mysettings, mysettings.getPublicSoapService(), method);
   }

  /**
  *
  * @param mysettings 	jemboss properties
  * @param method     	method to call
  * @param args       	arguments
  * @throws JembossSoapException if call to web service fails
  *
  */
   public PublicRequest(JembossParams mysettings, String method, Vector args)
               throws JembossSoapException
   {
     this(mysettings, mysettings.getPublicSoapService(), method, args);
   }

  /**
  *
  * @param mysettings 	jemboss properties
  * @param service    	web service to call
  * @param method     	method to call
  * @throws JembossSoapException if call to web service fails
  *
  */
   public PublicRequest(JembossParams mysettings, String service, String method)
               throws JembossSoapException
   {
     this(mysettings, service, method, (Vector) null);
   }

  /**
  *
  * @param mysettings 	jemboss properties
  * @param service    	web service to call
  * @param method     	method to call
  * @param args       	arguments
  * @throws JembossSoapException if call to web service fails
  *
  */
   public PublicRequest(JembossParams mysettings, String service, 
                        String method, Vector args)
               throws JembossSoapException
   {

     try
     {
       String  endpoint = mysettings.getPublicSoapURL();
       org.apache.axis.client.Service serv = 
                        new org.apache.axis.client.Service();

       Call    call     = (Call) serv.createCall();
       call.setTargetEndpointAddress( new java.net.URL(endpoint) );
       call.setOperationName(new QName(service, method));

       Object params[] = null;
       if(args != null)
       {
         params = new Object[args.size()];
         Enumeration e = args.elements();
         for(int i=0;i<args.size();i++)
         {
           Object obj = e.nextElement();
           if(obj.getClass().equals(String.class))
           {
             params[i] = (String)obj;
             call.addParameter("Args", XMLType.XSD_STRING,
                             javax.xml.rpc.ParameterMode.IN);
           }
           else
           {
             params[i] = (byte[])obj;
             call.addParameter("Args", XMLType.XSD_BYTE,
                             javax.xml.rpc.ParameterMode.IN);   
           }
         }
       }
       call.setReturnType(org.apache.axis.Constants.SOAP_VECTOR);

       Vector vans;
       if(args != null)
         vans = (Vector)call.invoke( params );
       else
         vans = (Vector)call.invoke( new Object[] {});

       proganswer = new Hashtable();
       // assumes it's even sized
       int n = vans.size();
       for(int j=0;j<n;j+=2)
       {
         String s = (String)vans.get(j);
         proganswer.put(s,vans.get(j+1));
       }
     } 
     catch (Exception e) 
     {
       throw new JembossSoapException("Connection failed: "+e.getMessage());
     }

   }


  /**
  *
  * Gets an element out of the Jemboss result hash
  * @param val 	key to look up
  * @return 	element, or an empty String if there isn't
  * 		an element that matches the key
  *
  */
  public String getVal(String val)
  {
    if (proganswer.containsKey(val)) 
      return (String)proganswer.get(val);
    else 
      return "";
  }


  /**
  *
  * The hash returned by the Jemboss call. 
  * @param 	result
  *
  */
  public Hashtable getHash() 
  {
    return proganswer;
  }

}
