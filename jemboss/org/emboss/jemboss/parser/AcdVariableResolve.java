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

import org.apache.regexp.*;
import java.io.*;
import javax.swing.*;

import org.emboss.jemboss.gui.form.*;

/**
*
*  Resolves variable references.
*
*/

public class AcdVariableResolve
{

  /** expression to resolve */
  private String exp;

  /**
  *
  * Given an expression this is searched for occurences of
  * a variable name for which the value is known. The value
  * is subsituted into the expression.
  * @param exp		expression to resolve
  * @param varName	variable name
  * @param varValue	variable value
  *
  */
  public AcdVariableResolve(String exp, String varName, String varValue) 
  {

    RECompiler rec = new RECompiler();

    try
    {
      exp = exp.replace('\n',' ');
      REProgram varexp = rec.compile("^(.*)\\$\\((" + varName + ")\\)");

// resolve variable references 
      RE regvarexp = new RE(varexp);
      if(regvarexp.match(exp))
      {
        String var = regvarexp.getParen(2);    // variable name
        int iend   = regvarexp.getParenEnd(1);
        String newvar = exp.substring(0,iend);
        iend   = regvarexp.getParenEnd(2);
        newvar = newvar.concat(varValue);
        newvar = newvar.concat(exp.substring(iend+1));
//      System.out.println("VARIABLE  " + var  + " -->  " + varValue);
//      System.out.println("EXPRESSION" + exp + " -->  " +  newvar);
        exp = new String(newvar); 
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }
    this.exp = exp;
  }

  /**
  *
  * Get the resolved expression
  * @return	result value of the expression
  *
  */
  public String getResult()
  {
    return exp;
  }

}

