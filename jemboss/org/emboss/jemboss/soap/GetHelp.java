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
* Get the help text for an application
*
*/
public class GetHelp 
{

  private String statusmsg;
  private String status;
  private String helpText;

  /**
  *
  * @param acdProg 	name of application to get the help text for
  * @param mysettings 	jemboss properties
  *
  */
   public GetHelp(String acdProg, JembossParams mysettings)
   {

     Vector params = new Vector();

     params.addElement(acdProg);
     try
     {
       PublicRequest hReq = new PublicRequest(mysettings, "show_help", params);
       helpText = hReq.getVal("helptext");
       status = hReq.getVal("status");
       statusmsg = hReq.getVal("msg");
     }
     catch (JembossSoapException jse){}

   }

  /**
  *
  * The status of the request
  * @return  	"0" for success, anything else for failure. 
  *
  */
  public String getStatus() 
  {
    return status;
  }

  /**
  *
  * A status message . In the case of an error, it will
  * contain a description of the error.
  * @return 	status message
  *
  */
  public String getStatusMsg() 
  {
    return statusmsg;
  }

  /**
  *
  * Get the help text
  * @return 	help text 
  *
  */
  public String getHelpText() 
  {
    return helpText;
  }

}

