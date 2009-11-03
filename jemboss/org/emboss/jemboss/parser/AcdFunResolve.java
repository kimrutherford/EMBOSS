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

/**
*
*  This is a rewrite in Java of the expression functions in ajacd.c of the
*  EMBOSS ajax library, written by Peter Rice.
*  This resolves any function reference in a string.
*  It will then looks for function references and resolves them.
* 
*/

public class AcdFunResolve
{

  /** result as a String of the expression used in constructor */
  public String result;

  /**
  *
  * @param exp 	String with possible expression to expand
  *
  */
  public AcdFunResolve(String exp)
  {
    RECompiler rec = new RECompiler();
    try
    {
      REProgram funexp = rec.compile("^(.*)\\@\\(([^()]+)\\)");

 // resolve functions 
      RE regfunexp = new RE(funexp);
      while (regfunexp.match(exp))
      {
        String var = regfunexp.getParen(2);    // function
        result = acdFunResolve(var);
        int iend   = regfunexp.getParenEnd(1);
        String newvar = exp.substring(0,iend);
        iend   = regfunexp.getParenEnd(2);
        newvar = newvar.concat(result);
        newvar = newvar.concat(exp.substring(iend+1));
        exp = new String(newvar);
//      System.out.println("FUN " + exp);
      }

    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    if(result == null)
      result = exp;

  }

  /**
  *
  * Returns the value of the result of the expression.
  * @return result
  *
  */
  public String getResult()
  {
    if( result.equalsIgnoreCase("False") ||
        result.equalsIgnoreCase("True") )
      result = result.toLowerCase();
    return result;
  }

  /**           
  *
  * Looks for and resolves expressions
  * @param exp 	String with possible expression 
  * @return 	result
  *
  */
  public String acdFunResolve(String var)
  {
    result = "";

    acdExpPlus(var);
    acdExpMinus(var);
    acdExpStar(var);
    acdExpDiv(var);
    acdExpNot(var);
    acdExpEqual(var);
    acdExpNotEqual(var);
    acdExpGreater(var);
    acdExpLesser(var);
    acdExpOr(var);
    acdExpAnd(var);
    acdExpCond(var);

    return result;
  }

  /**  
  *
  * Looks for and resolves an expression <code>@( num + num )</code>
  * @param exp String with possible expression
  * @return true if successfully resolved
  *
  */
  private boolean acdExpPlus(String exp)
  {
    
    RECompiler rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[+][ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        ia+=ib;
        result = Integer.toString(ia);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }


    rec = new RECompiler();
    try
    {
      REProgram rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[+]" +
                                    "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        da+=db;
        result = Double.toString(da);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }


  /** 
  *
  * Looks for and resolves an expression <code>@( num - num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpMinus(String exp)
  {

    RECompiler rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*(-)[ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(3))).intValue();
        ia-=ib;
        result = Integer.toString(ia);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*(-)" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");

      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(3))).doubleValue();
        da-=db;
        result = Double.toString(da);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }


/** 
*
* Looks for and resolves an expression <code>@( num * num )</code>
* @param exp 	String with possible expression
* @return 	true if successfully resolved
* 
*/
  private boolean acdExpStar(String exp)
  {

    RECompiler rec = new RECompiler();

    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[*][ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        ia=ia*ib;
        result = Integer.toString(ia);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[*]" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        da=da*db;
        result = Double.toString(da);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }


  /** 
  *
  * Looks for and resolves an expression <code>@( num * num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  * 
  */
  private boolean acdExpDiv(String exp)
  {

    RECompiler rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[/][ \t]*([-+]*[:digit:]+)[ \t)]*$");
                                   
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        ia=ia/ib;
        result = Integer.toString(ia);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[/]" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        da=da/db;
        result = Double.toString(da);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }

  /** 
  *
  * Looks for and resolves an expression <code>@(! bool )</code> 
  * or <code>@(NOT bool)</code> or <code>@(not bool)</code>.
  * An invalid bool value is treated as false, so it will
  * return a true value.
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpNot(String exp)
  {
    boolean ba;

    RECompiler rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*(!|[Nn][Oo][Tt])[ \t]*([a-zA-Z0-9]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        if(regexp.getParen(2).equalsIgnoreCase("false"))
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }

  /** 
  *
  * Looks for and resolves an expression <code>@( num == num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpEqual(String exp)
  {

    RECompiler rec = new RECompiler();   // int = int
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[=][=][ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        if(ia==ib)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();           // float == float
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[=][=]" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        if(da==db)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();           // string == string
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([a-zA-Z]+[a-zA-Z0-9]*)[ \t]*[=][=]" +
                                     "[ \t]*([a-zA-Z]+[a-zA-Z0-9]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        String sa = regexp.getParen(1);
        String sb = regexp.getParen(2);
        if(sa.equalsIgnoreCase(sb))
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }

  /** 
  *
  * Looks for and resolves an expression <code>@( num != num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpNotEqual(String exp)
  {

    RECompiler rec = new RECompiler();   // int != int
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[!][=][ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        if(ia!=ib)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();           // float != float
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[!][=]" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        if(da!=db)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();           // string != string
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([a-zA-Z]+[a-zA-Z0-9]*)[ \t]*[!][=]" +
                                     "[ \t]*([a-zA-Z]+[a-zA-Z0-9]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        String sa = regexp.getParen(1);
        String sb = regexp.getParen(2);
        if(!sa.equalsIgnoreCase(sb))
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }


  /** 
  *
  * Looks for and resolves an expression <code>@( num > num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpGreater(String exp)
  {

    RECompiler rec = new RECompiler();   // int > int
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[>][ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        if(ia>ib)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();           // float > float
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[>]" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        if(da>db)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }



  /** 
  *
  * Looks for and resolves an expression <code>@( num < num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpLesser(String exp)
  {

    RECompiler rec = new RECompiler();   // int < int
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+)[ \t]*[<][ \t]*([-+]*[:digit:]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(1))).intValue();
        int ib = (new Integer(regexp.getParen(2))).intValue();
        if(ia<ib)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    rec = new RECompiler();           // float < float
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t]*[<]" +
                                     "[ \t]*([-+]*[:digit:]+[.]*[:digit:]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        double da = (new Double(regexp.getParen(1))).doubleValue();
        double db = (new Double(regexp.getParen(2))).doubleValue();
        if(da<db)
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }


  /** 
  *
  * Looks for and resolves an expression <code>@( num | num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpOr(String exp)
  {

    RECompiler rec = new RECompiler(); // string | string 
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([a-zA-Z]+)[ \t]*[|]" +
                                     "[ \t]*([a-zA-Z]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        String ba = regexp.getParen(1);
        String bb = regexp.getParen(2);
        if ( ba.equalsIgnoreCase("true") || bb.equalsIgnoreCase("true") )
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }


    return false;
  }


  /** 
  *
  * Looks for and resolves an expression <code>@( num & num )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpAnd(String exp)
  {

    RECompiler rec = new RECompiler(); // string & string
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([a-zA-Z]+[a-zA-Z0-9]*)[ \t]*[&]" +
                                     "[ \t]*([a-zA-Z]+[a-zA-Z0-9]*)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        String ba = regexp.getParen(1);
        String bb = regexp.getParen(2);
        if ( ba.equalsIgnoreCase("true") && bb.equalsIgnoreCase("true") )
          result = "true";
        else
          result = "false";
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }
  

  /** 
  *
  * Looks for and resolves an expression <code>@( bool ? trueval : falseval )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpCond(String exp)
  {

    RECompiler rec = new RECompiler(); 
    try
    {
      REProgram  rep = rec.compile("^[ \t(]*([a-zA-Z]+)[ \t]*[?]" +
                                    "[ \t]*([^: \t]+)[ \t]*(:)" +
                                    "[ \t]*([^: \t)]+)[ \t)]*$");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        if(regexp.getParen(1).equalsIgnoreCase("true"))
          result = regexp.getParen(2);
        else
          result = regexp.getParen(4);
        return true;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }


  /**  
  *
  * Looks for and resolves an expression as a switch/case statement
  * <code>@( var = casea : vala, caseb: valb else: val )</code>
  * @param exp 	String with possible expression
  * @return 	true if successfully resolved
  *
  */
  private boolean acdExpCase(String exp)
  {

    RECompiler rec = new RECompiler();
    try
    {
      REProgram caseexp = rec.compile("^[ \t]*([A-Za-z0-9]+)[ \t]*[=]");
      REProgram listexp = rec.compile("^[ \t]*([^: \t]+)[ \t]*[:]+" +
                                         "[ \t]*([^: \t,]+)[ \t,]*");

      RE caseregexp = new RE(caseexp);
      if(caseregexp.match(exp))
      {
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return false;
  }

}

