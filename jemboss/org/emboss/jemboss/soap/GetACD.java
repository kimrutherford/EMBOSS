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
*  @author: Copyright (C) Tim Carver
*
********************************************************************/

package org.emboss.jemboss.soap;

import java.io.*;
import java.util.*;

import org.emboss.jemboss.JembossParams;


/**
*
* Retrieve an ACD file using soap
*
*/
public class GetACD
{

  /** acd store */
  static Hashtable acdStore;
  static 
  {
    acdStore = new Hashtable();
  }

  /** acd text */
  private String helpText;

  /**
  *
  * @param acdProg    name of the application described by the ACD
  * @param mysettings jemboss properties
  *
  */
   public GetACD(String acdProg, JembossParams mysettings)
   {

     if (mysettings.getDebug()) 
       System.out.println("GetACD: retrieving "+acdProg);

     if (acdStore.containsKey(acdProg))
     {
       helpText = (String) acdStore.get(acdProg);
       if (mysettings.getDebug()) 
	 System.out.println("GetACD: retrieved from cache.");
     }
     else
     {
       Vector params = new Vector();
       params.addElement(acdProg);
//     params.addElement(new Parameter("option", String.class,
//                                  acdProg, null));

       try
       {
         PublicRequest hReq = new PublicRequest(mysettings, "show_acd", params);
         helpText = hReq.getVal("acd");
       }
       catch (JembossSoapException jse){}

       acdStore.put(acdProg,helpText);
     }

   }

  /**
  *
  * Return the text of the ACD file
  * @return 	acd text
  *
  */
   public String getAcd() 
   {
     return helpText;
   }

}
