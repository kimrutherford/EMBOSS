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


package org.emboss.jemboss.parser;

/**
*
* Variable that is dependent on the value of another variable
*
*/
public class Dependent
{

  /** field number in the ACD file */
  private int field;
  /** parameter number */
  private int param;
  /** expression */
  private String exp;
  /** parameter type */
  private String type;

  /**
  *
  * @param field	field number in the ACD file
  * @param param	parameter number
  * @param exp		expression
  * @param type		parameter type 
  *
  */
  public Dependent(int field, int param, String exp, String type)
  {
    this.field = field;
    this.param = param;
    this.exp   = exp;
    this.type  = type;
  }

  /**
  *
  * Get the field
  * @return 	field number
  *
  */
  public int getDependentField()
  {
    return field;
  }

  /**
  *
  * Get the parameter number
  * @return     parameter number
  *
  */
  public int getDependentParam()
  {
    return param;
  }

  /**
  *
  * Get the expression
  * @return 	expression
  *
  */
  public String getDependentExp()
  {
    return exp;
  }

  /**
  *
  * Get type of parameter
  * @return	type of parameter
  *
  */
  public String getDependentType()
  {
    return type;
  }

  /**
  *
  * Set the value of the expression
  * @param exp 	value of expression
  *
  */
  public void setDependentExp(String exp)
  {
    this.exp = exp;
  }

}


