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

import java.util.*;
import org.emboss.jemboss.JembossParams;

/**
*
* Calls the EMBOSS ajax library
*
*/
public class CallAjax 
{

  /** sequence length */
  private int length;
  /** sequence weight */
  private float weight;
  /** type of sequence protein or nucleotide */
  private boolean protein;
  /** request to private server */
  private PrivateRequest epr;

  /**
  *
  * @param fileContent	file content or server file name
  * @param seqtype	sequence or seqset
  * @param mysettings	jemboss properties
  * @throws JembossSoapException if connection fails
  *
  */
  public CallAjax(String fileContent, String seqtype,
                            JembossParams mysettings) 
      throws JembossSoapException 
  {

    Vector params = new Vector();
    params.addElement(fileContent);
    params.addElement(seqtype);
    // use authenticated call_ajax method
    epr = new PrivateRequest(mysettings,"call_ajax",params);

    Hashtable res = epr.getHash();
    Enumeration enumRes = res.keys();
    while(enumRes.hasMoreElements()) 
    {
      String thiskey = (String)enumRes.nextElement().toString();
      if(thiskey.equals("length"))
        length = ((Integer)res.get(thiskey)).intValue();
      if(thiskey.equals("weight"))
        weight = ((Float)res.get(thiskey)).floatValue();
      if(thiskey.equals("protein"))
        protein = ((Boolean)res.get(thiskey)).booleanValue();
    }
  }

  /**
  *
  * Get the status
  * @return 	status
  *
  */
  public String getStatus() 
  {
    return (String)epr.getVal("status");
  }
  
  /**
  *
  * Get the message
  * @return     message
  *
  */
  public String getStatusMsg() 
  {
    return (String)epr.getVal("msg");
  }

  /**
  *
  * Get the sequence length
  * @return 	sequence length
  *
  */
  public int getLength()
  {
    return length;
  }
  
  /**
  *
  * Get the sequence weight
  * @return     sequence weight
  *
  */
  public float getWeight()
  {
    return weight;
  }

  /**
  *
  * Deternmine the sequence type
  * @return 	true if a protein
  *
  */
  public boolean isProtein()
  {
    return protein;
  }

}
