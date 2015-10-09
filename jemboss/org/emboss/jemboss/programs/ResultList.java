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
*  Based on EmbreoResList
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.programs;

import java.util.*;

import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.soap.PrivateRequest;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.server.JembossServer;

/**
*
* Holds the list of stored results.
* Used by ShowSavedResults to show (show_saved_results),
* delete (delete_saved_results) & list results (list_saved_results).
*/
public class ResultList 
{
  private String statusmsg;
  private String status;
  private Hashtable proganswer;

  /**
  *
  * Holds the list of stored results
  * @param mysettings 	jemboss properties
  * @throws JembossSoapException if authentication fails
  *
  */
  public ResultList(JembossParams mysettings) throws JembossSoapException 
  {
    this(mysettings,null,"list_saved_results");
  }

  /**
  *
  * Holds the list of stored results
  * @param mysettings   jemboss properties
  * @param dataset	dataset to manipulate
  * @param methodname	method to invoke on this dataset
  * @throws JembossSoapException if authentication fails
  *
  */
   public ResultList(JembossParams mysettings, String dataset,
                     String methodname) throws JembossSoapException
   {
     this(mysettings,dataset,"",methodname);
   }

  /**
  *
  * Manipulate a dataset.
  * @param mysettings JembossParams defining server parameters
  * @param dataset    Which dataset to manipulate
  * @param options    A project file to retrieve
  * @param methodname What method to invoke on this dataset
  * @throws JembossSoapException if authentication fails
  *
  */
   public ResultList(JembossParams mysettings, String dataset, 
                     String options, String methodname) throws JembossSoapException 
   {
 
     if(org.emboss.jemboss.Jemboss.withSoap)
     {
       PrivateRequest eRun;
       Vector params = new Vector();

       if(dataset != null) 
       {
         params.addElement(dataset);
         params.addElement(options);
       }

       try 
       {
         eRun = new PrivateRequest(mysettings,methodname, params);
       } 
       catch (JembossSoapException e) 
       {
         throw new JembossSoapException("Authentication Failed");
       }

       proganswer = eRun.getHash();
     }
     else   // handle standalone
     {
       JembossServer js = new JembossServer(mysettings.getResultsHome());
       Vector vans;

       if(methodname.equals("list_saved_results"))
         vans = js.list_saved_results();
       else if(methodname.equals("show_saved_results"))
         vans = js.show_saved_results(dataset, options);
       else
         vans = js.delete_saved_results(dataset, options);

       proganswer = org.emboss.jemboss.gui.form.BuildJembossForm.convert(vans,true);
     }

     try
     {
       status = proganswer.get("status").toString();
     }
     catch(NullPointerException npe){}
    
     try
     {
       statusmsg = proganswer.get("msg").toString();
     }
     catch(NullPointerException npe){}
 
     proganswer.remove("status");      //delete out of the hash
     proganswer.remove("msg");
   }

  /**
  *
  * The status of the request
  * @return 	0 for success, anything else for failure
  *
  */
  public String getStatus() 
  {
    return status;
  }

  /**
  *
  * A status message. In the case of an error,
  * contains a description of the error.
  * @return 	a status message
  *
  */
  public String getStatusMsg() 
  {
    return statusmsg;
  }
  
  /**
  *
  * @return Hashtable of the results
  *
  */
  public Hashtable hash() 
  {
    return proganswer;
  }

  /**
  *
  * @return Enumeration of the elements of the results Hashtable
  *
  */
  public Enumeration elements() 
  {
    return proganswer.elements();
  }

  /**
  *
  * @return Enumeration of the keys of the results Hashtable
  *
  */
  public Enumeration keys() 
  {
    return proganswer.keys();
  }


  /**
  *
  * @param key	key of the element in the result
  *		Hashtable to return
  * @return 	element in the result Hashtable
  *
  */
  public Object get(Object key) 
  {
    return proganswer.get(key);
  }

  /**
  *
  * Replace the current results hash
  * @param newres  	new results hash
  *
  */
  public void updateRes(Hashtable newres)
  {
    proganswer = newres;
  }

}

