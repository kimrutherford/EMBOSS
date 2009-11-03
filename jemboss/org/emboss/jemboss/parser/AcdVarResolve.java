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
*  Resolves any variable reference in a string.
*
*/

public class AcdVarResolve
{

  /** expression */
  private String exp;

  /**
  *
  * @param exp 		expression to resolve
  * @param varName	variable name
  * @param parseAcd	ACD parser for the application
  * @param nof		number of fields
  * @param textf	text fields
  * @param textInt	int fields
  * @param textFloat	float fields
  * @param fieldOption	combo popup fields
  * @param checkBox	boolean fields
  *
  */
  public AcdVarResolve(String exp, String val, String varName, 
                       ParseAcd parseAcd, int nof, 
                       TextFieldSink textf[], TextFieldInt textInt[],
                       TextFieldFloat textFloat[], JembossComboPopup fieldOption[],
                       JCheckBox  checkBox[]) 
  {

    RECompiler rec = new RECompiler();

    try
    {
      exp = exp.replace('\n',' ');
      REProgram varexp = rec.compile("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");

// resolve variable references first 
      RE regvarexp = new RE(varexp);
      while (regvarexp.match(exp))
      {
        String var = regvarexp.getParen(2);    // variable name
        String res = "";
        if(var.endsWith("acdprotein"))
          res = (new Boolean(SectionPanel.ajaxProtein)).toString();
        else if (var.endsWith(".length") || var.endsWith(".end"))
          res = (new Integer(SectionPanel.ajaxLength)).toString();
        else if (var.endsWith(".begin"))
          res = "0";
        else if (var.endsWith(".totweight"))  
          res = (new Float(SectionPanel.ajaxWeight)).toString();
        else if(var.equals(varName))
          res = val;
        else
        {
          res = new String("UnresolvedToken"+var); 
          for(int i =0;i<nof;i++)
          {
            if(parseAcd.getParamValueStr(i,0).equals(var))
            {
              //get the value
              String att = parseAcd.getParameterAttribute(i,0).toLowerCase();
              int h = parseAcd.getGuiHandleNumber(i);
              if(att.startsWith("datafile")|| att.startsWith("featout")  ||
                att.startsWith("string")   || att.startsWith("seqout") ||
                att.startsWith("outfile")  || att.startsWith("matrix") ||
                att.startsWith("infile")   || att.startsWith("regexp") ||
                att.startsWith("codon") )
              {
                if(!(textf[h].getText()).equals("")) 
                  res = textf[h].getText();
              }
              else if ( att.startsWith("int") ) 
              {
                if(textInt[h].getText() != null)
                  res = Integer.toString(textInt[h].getValue());
              }      
              else if ( att.startsWith("float") )
              {
                if(textFloat[h].getText() != null) 
                  res = Double.toString(textFloat[h].getValue());
              }
              else if ( att.startsWith("select") )   //defined by a number
                res = (new Integer(fieldOption[h].getSelectedIndex()+1)).toString();
              else if ( att.startsWith("list") )     //defined by a label
              {

                int index = fieldOption[h].getSelectedIndex();
                res = parseAcd.getListLabel(i,index);
              }
              else if ( att.startsWith("bool") )
              {
                if(checkBox[h].isSelected())
                  res = new String("true");
                else
                  res = new String("false");
              }
              break;
            }
          }
        }
          
        int iend   = regvarexp.getParenEnd(1);
        String newvar = exp.substring(0,iend);
        iend   = regvarexp.getParenEnd(2);
        newvar = newvar.concat(res);
        newvar = newvar.concat(exp.substring(iend+1));
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
  * @return 	result
  *
  */
  public String getResult()
  {
    return exp;
  }

}

