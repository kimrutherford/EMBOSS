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
* Run showdb and find the available scoring matrices
* and codon usage tables on the Jemboss server
*
*/
public class ShowDB 
{

  /** status message */
  private String statusmsg;
  /** status */
  private String status;
  /** showdb output */
  private String dbText;
  /** scoring matrices */
  private Vector matrices;
  /** codon usage tables */
  private Vector codons;

  /**
  *
  * @param mysettings 	jemboss properties
  *
  */
  public ShowDB(JembossParams mysettings)
  {
 
    PublicRequest dbReq = null;   
    try
    {
      dbReq = new PublicRequest(mysettings, "show_db");
    }
    catch (JembossSoapException jse) {}

    statusmsg = dbReq.getVal("msg");
    status = dbReq.getVal("status");
    dbText= dbReq.getVal("showdb");
    String mat = dbReq.getVal("matrices");

    matrices = new Vector();
    StringTokenizer stMat = new StringTokenizer(mat,"\n");
    while (stMat.hasMoreTokens())
      matrices.add(stMat.nextToken());

    mat = dbReq.getVal("codons");
    codons = new Vector();
    stMat = new StringTokenizer(mat,"\n");
    while (stMat.hasMoreTokens())
      codons.add(stMat.nextToken());
  }

  /**
  *
  * Get the status of the request
  * @return 	0 for success
  *
  */
  public String getStatus()
  {
    return status;
  }

  /**
  *
  * Get the status message
  * @return 	status message or contains a
  * 		description of any error.
  *
  */
  public String getStatusMsg() 
  {
    return statusmsg;
  }

  /**
  *
  * Get the output from showdb
  * @return  output from showdb
  *
  */
  public String getDBText() 
  {
    return dbText;
  }

  /**
  *
  * Get the available matrices 
  * @return  	matrices names
  *
  */
  public Vector getMatrices()
  {
    return matrices;
  }
 
  /**
  *
  * Get the codon usage tables
  * @return  	codon usage table names
  *
  */
  public Vector getCodonUsage()
  {
    return codons;
  }

}
