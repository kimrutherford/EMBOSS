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

package org.emboss.jemboss.parser.acd;

import java.io.*;

/**
*
* Object for an field in the ACD file. Each field
* contains a number of parameter which are stored
* as ApplicationParam objects.
*
*/
public class ApplicationFields 
{

  /** number of parameters for this field */
  private int numofParams;
  /** the gui handle */
  private int guiHandleNumber;
  /** parameters for this field */
  private ApplicationParam appP[];


  public ApplicationFields() 
  {
  }

  /** 
  *
  * Set the number of parmeters in this field
  * @param numofParams	number of parmeters in this field
  *
  */
  public void setNumberOfParam(int numofParams) 
  {
   this.numofParams = numofParams;
   appP = new ApplicationParam[numofParams];
  }

  /** 
  *
  * Set the value of a parameter 
  * @param index	parameter number
  * @param attr		parameter attribute (type, help, optional...)
  * @param sval		parameter value
  *
  */
  public void setParam(int index, String attr, String sval) 
  {
    appP[index] = new ApplicationParam(attr, sval);
  }

  /** 
  *
  * Set the value of a parameter 
  * @param index        parameter number        
  * @param attr         parameter attribute (type, help, optional...)       
  * @param nvalue       parameter value
  *
  */
  public void setParam(int index, String attr, double nvalue) 
  {
    appP[index] = new ApplicationParam(attr, nvalue);
  }

//public void resetParam(int index, String sval) 
//{
//  appP[index].reset(sval);
//}

//public void resetParam(int index, double nvalue) 
//{
//  appP[index].reset(nvalue);
//}

  /**
  *
  * Get a parameter attribute, given the parameter number in
  * the field
  * @param index	parameter number
  * @return		parameter attribute
  *
  */
  public String getParamAttribute(int index) 
  {
    return appP[index].getAttribute();
  }

  /**
  *
  * Get a parameter value, given the parameter number in
  * the field
  * @param index        parameter number
  * @return		parameter value 
  *
  */
  public String getParamValueStr(int index) 
  {
    return appP[index].getValueStr();
  }

  /**
  *
  * Get a parameter value, given the parameter number in
  * the field
  * @param index        parameter number
  * @return             parameter value 
  *
  */
  public double getParamValueDbl(int index) 
  {
    return appP[index].getValueDbl();
  }

  /**
  *
  * Determine if the parameter value is a string
  * @return 	true if the parameter value is a string 
  *
  */
  public boolean isParamValueStr(int index) 
  {
     return appP[index].isParamValueString();
  }

  /**
  *
  * Get the number of parameters in this field
  * @return  	number of parameters
  *
  */
  public int getNumberOfParam() 
  {
   return numofParams;
  }

  /**
  *
  * Set the gui handle
  * @param guiHandleNumber	gui handle
  *
  */
  public void setGuiHandleNumber(int guiHandleNumber) 
  {
    this.guiHandleNumber = guiHandleNumber;
  }

  /**
  *
  * Get the gui handle
  * @return guiHandleNumber      gui handle
  *
  */
  public int getGuiHandleNumber() 
  {
    return guiHandleNumber;
  }

}

