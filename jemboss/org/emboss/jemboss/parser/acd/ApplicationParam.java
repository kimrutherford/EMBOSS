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


/**
*
* Object for a parameter of a field in the EMBOSS
* ACD file
*
*/
public class ApplicationParam 
{

  /** parameter/attribute type */
  private String attribute=null;
  /** parameter string value */
  private String svalue=null;
  /** parameter number value */
  private double nvalue=0;


  /**
  *
  * @param attr	parameter/attribute type
  * @param sval	parameter string value
  *
  */
  public ApplicationParam(String attr, String sval) 
  {
    setAttribute(attr);
    setValue(sval);
  }

  /**
  *
  * @param attr parameter/attribute type         
  * @param nval parameter number value
  *
  */
  public ApplicationParam(String attr, double nval) 
  {
    setAttribute(attr);
    setValue(nval);
  }

  /**
  *
  * Set the attribute/parameter type
  * @param 	attribute/parameter type
  *
  */
  private void setAttribute(String attribute) 
  {
     this.attribute = attribute;
  }

//public void reset(String sval)
//{
//  setValue(sval);
//  nvalue=0;
//}
  
//public void reset(double nval)
//{
//  setValue(nval);
//  svalue=null;
//}

  /**
  *
  * Set the parameter string value
  * @param	parameter value 
  *
  */
  public void setValue(String svalue) 
  {
     this.svalue = svalue;
  }

  /**
  *
  * Set the parameter number value
  * @param      parameter value 
  *
  */
  public void setValue(double nvalue) 
  {
     this.nvalue = nvalue;
  }

  /**
  *
  * Get the parameter/attribute type
  * @return 	parameter/attribute type
  *
  */
  public String getAttribute() 
  {
     return attribute;
  }

  /**
  *
  * Get the parameter value
  * @return     parameter value
  *
  */
  public String getValueStr() 
  {
     return svalue;
  }

  /**
  *
  * Get the parameter value
  * @return     parameter value
  *
  */
  public double getValueDbl() 
  {
     return nvalue;
  }

  /**
  *
  * Determine if the parameter has a string value
  * @return	true if the parameter has a string value
  *
  */
  public boolean isParamValueString() 
  {
    if(svalue != null) {
     return true;
    } else {
     return false;
    }
  }

}



