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

import org.emboss.jemboss.parser.acd.*;
import java.io.*;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Enumeration;

/**
*
* ACD (Ajax command line definition) file parser.
*
*/
public class ParseAcd
{

 		       /** number of ACD fields */
  private int numofFields = 0;
 		       /** number of parameters in a field */
  private int numofParams;
 		       /** vector containing the application fields */
  private Vector vappF = new Vector();
 		       /** string value of a parameter */
  private String svalue;
 		       /** double value of a parameter */
  private double nvalue;
 		       /** parameter data type */
  private String attr;
 		       /** array of dependent variables */
  private Dependent dep[];
 		       /** number of dependent variables */
  private int numOfDependents;
 		       /** number of sections */
  private int nsection = 0;
 		       /** number of subsections */
  private int nsubsection = 0;

// data types in ACD

                      /** number of text fields */
  private int ntextf;
                      /** number of integer fields */
  private int nint;
                      /** number of float fields */
  private int nfloat;
                      /** number of checkbox fields */
  private int nbool;
                      /** number of sequence fields */
  private int nseqs;
                      /** number of filelist fields */
  private int nflist;
                      /** number of list & selection fields 
                          - single selection */
  private int nlist;
                      /** number of list & selection fields 
                          - multiple selection */
  private int mlist;
                      /** number of range fields */
  private int nrange;
                      
                      /** default for list or select data type */
  private Vector listdefault;
  
  private int nmultiTextField;

// Groups the program belongs to

  /**
  *
  * The constructor takes the ACD as a string. 
  * @param acdText 	String representation of 
  *			the ACD file
  * @param groups  	boolean determing whether 
  *			just to retieve the groups 
  *
  */
  public ParseAcd(String acdText, boolean groups) 
  {

    Vector variables = new Vector();  //for "variable:" fields
    ApplicationFields appF;
    int colonPos=0;
    int braketPos=0;
    int ttype;
    String param;
    String line;

    try 
    {
      BufferedReader in = new BufferedReader(new StringReader(acdText));
      //line = new String();
      in.mark(2500);
      //line = in.readLine();

// loop over all fields defined in the ACD
      while((line = in.readLine()) != null)
      {
        line = line.trim();    // removes leading & trailing whitespace
     
        if(line.startsWith("#") || (line.length() ==0))
        {
          in.mark(2500);
          continue;
        }

        colonPos  = line.indexOf(':');
        if(colonPos < 0) colonPos  = line.indexOf('=');
        if(colonPos < 0) continue;

        String dataType = line.substring(0,colonPos).toLowerCase();
        if(dataType.equals("toggle"))
          dataType = "boolean";
        braketPos = line.indexOf('[');

        if(braketPos >= 0) 
          param = line.substring(colonPos+1,braketPos).trim();
        else 
          param = line.substring(colonPos+1).trim();

        if(line.startsWith("var:") || line.startsWith("variable"))
        {
          param = param.trim();
          ApplicationFields var = new ApplicationFields();
          var.setNumberOfParam(2);
      
          int ns = param.indexOf(" ");
          if(ns > -1)
          {
            String value = param.substring(0,ns);
            value = value.replace('"',' ').trim();
            var.setParam(0,dataType,value);
            value = param.substring(ns);
            value = value.replace('"',' ').trim();
            var.setParam(1, "value", value);
          }
          variables.add(var);
          in.mark(2500);
          continue;
        }
        else if(line.startsWith("endsection"))
        {
          appF = new ApplicationFields();
          appF.setNumberOfParam(1);
          appF.setParam(0, dataType, param);
          numofFields++;
          vappF.add(appF);
          in.mark(2500);
          continue;
        }
        else if(line.startsWith("section"))
        {
          if(param.equals("input") || param.equals("required") ||
             param.equals("output") || param.equals("advanced") ||
             param.equals("additional") )
            nsection++;
          else
            nsubsection++;
        }
 
        numofParams = 1;

        StreamTokenizer st = new StreamTokenizer(in);
        if(! line.endsWith("[")) 
        {                                        // rewind to start 
          in.reset();                            // tokeniser on same line
          st.nextToken(); st.nextToken();
          st.nextToken(); st.nextToken();
        }

        in.mark(2500);
        do 
        {
          ttype = parseParam(in, st);
        } while(attr != null);

        appF = new ApplicationFields();
        vappF.add(appF);
        appF.setNumberOfParam(numofParams);
        numofParams = 0;

        //set param name to lowercase as ACD is case insensitive
        appF.setParam(numofParams, dataType, param.toLowerCase());
 
        in.reset();
        do 
        {
          ttype = parseParam(in, st);
          // is the value a number or string
          if( ttype != java.io.StreamTokenizer.TT_NUMBER &&
               attr != null) 
          {
             if(variables.size()>0)
               svalue = resolveVariables(variables,svalue);

             //set variables to lower case as ACD is case insensitive
             if(svalue.indexOf("$") > -1 || svalue.indexOf("@") > -1)
               svalue = svalue.toLowerCase();

             appF.setParam(numofParams, attr, svalue);
//           System.out.println(" ATTR " + attr + " SVALUE " + 
//                          getParamValueStr(numofFields,numofParams));
          }   
          else if ( ttype == java.io.StreamTokenizer.TT_NUMBER) 
          {
//           System.out.println(" ATTR " + attr + " NVALUE " + nvalue);
             appF.setParam(numofParams, attr, nvalue);
          }
        } while (attr != null);

// set gui handle (based on data type)
        setGuiHandleNumber(dataType,appF);

        numofFields++;
        in.mark(2500);

      };
   
    }
    catch (IOException e) 
    {
      System.out.println("PareAcd: Parsing acd file error" );
    }

  }


  /**
  *
  * Used to replace the "variable:" or "var:" shorthand notation in
  * the ACD to the full expression it represent
  * @param variables	ApplicationFields of var ACD fields
  * @param svalue 	value to be resolved if necessary
  *
  */
  private String resolveVariables(Vector variables,String svalue)
  {
    String res=svalue;

    Enumeration enumer = variables.elements();
    while(enumer.hasMoreElements())
    {
      ApplicationFields var = (ApplicationFields)enumer.nextElement();
      String vName  = var.getParamValueStr(0);
      String vValue = var.getParamValueStr(1);

      //if we have something that might need resolving
      if((res.indexOf("$") >-1) || (res.indexOf("@") >-1))
      {
        AcdVariableResolve avresolve = new AcdVariableResolve(res,vName,vValue);
        res = avresolve.getResult();
      }
    }

//  if(!res.equals(svalue))
//    System.out.println("START VAL "+svalue+" END VAL "+res);
    return res;
  }


  /**
  *
  * Gets the handle for a gui component on the Jemboss form.
  * @param  field	field number in the ACD file
  * @return 		handle integer.
  *
  */
  public int getGuiHandleNumber(int field) 
  {
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    return aF.getGuiHandleNumber();
  }


  /**
  *
  * Gets the number of float, string, seqout, outfile,
  * infile, regexp, codon & featout data types in the ACD.
  * These make up the number of JTextAreas in the Jemboss form.
  * @return 	number of TextArea's in the Jemboss form.
  *
  */
  public int getNumTextf() 
  {
    return ntextf;
  }

  /**
  *
  * Gets the number of int data types in the ACD.
  * @return 	number of TextFieldInt in the Jemboss form.
  *
  */
  public int getNumNint()
  {
    return nint;
  }

  /**
  *
  * Gets the number of float data types in the ACD.
  * @return 	number of TextFieldFloat in the Jemboss form.
  *
  */
  public int getNumNfloat()
  {
    return nfloat;
  }


  /**
  *
  * Gets the number of boolean data types in the ACD.
  * @return 	number of check boxes in the Jemboss form.
  *
  */
  public int getNumBool() 
  {
    return nbool;
  }


  /**
  *
  * Gets the number of seqset, seqall & sequence data types in the ACD
  * @return 	number of sequence inputs in the Jemboss form.
  *
  */
  public int getNumSeq() 
  {
    return nseqs;
  }

  /**
  *
  * Gets the number of filelist data types in the ACD
  * @return 	number of file lists in the Jemboss form.
  *
  */
  public int getNumFileList()
  {
    return nflist;
  }

  /**
  *
  * Gets the number of list & selection data types in the ACD,
  * using single list selection
  * @return 	number of selection lists in the Jemboss form.
  *
  */
  public int getNumList() 
  {
    return nlist;
  }

  /**
  *
  * Gets the number of list & selection data types in the ACD,
  * using multiple list selection
  * @return 	number of selection lists in the Jemboss form.
  *
  */
  public int getNumMList()
  {
    return mlist;
  }


  /**
  *
  * @return 	number of range data types in the Jemboss form.
  *
  */
  public int getNumRange()
  {
    return nrange;
  }


  /**
  *
  * Gets the number of sections in the ACD
  * @return 	number of sections in the Jemboss form.
  *
  */
  public int getNumSection()
  {
    return nsection;
  }


  /**
  *
  * Gets the number of nested sections in the ACD
  * @return 	number of subsections in the Jemboss form.
  *
  */
  public int getNumSubsection()
  {
    return nsubsection;
  }


  /**
  *
  * Get a specified parameter attribute.
  * @param field	field number in the ACD file
  * @param param 	parameter number in that field
  * @return 		string of the value.
  *
  */
  public String getParameterAttribute(int field, int param)
  {
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    return aF.getParamAttribute(param);
  }


  /**
  *
  * Determine if the value of the parameter is a String.
  * @return 	true if the value of the parameter is a String.
  *
  */
  public boolean isParamValueStr(int field, int param) 
  {
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    return aF.isParamValueStr(param);
  }


  /**
  *
  * Gets the String value of a parameter.
  * @return 	String value of the parameter. 
  *
  */
  public String getParamValueStr(int field, int param) 
  {
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    return aF.getParamValueStr(param).trim();
  }


  /**
  *
  * Sets the gui handle depending on the data type.
  * @param dataType	data type
  * @param appF		application field
  *
  */
  private void setGuiHandleNumber(String dataType, ApplicationFields appF)
  {

    if ( dataType.startsWith("datafile") || dataType.startsWith("featout") ||
         dataType.startsWith("string")   || dataType.startsWith("seqout")  ||
         dataType.startsWith("outfile")  || dataType.startsWith("matrix")  ||
         dataType.startsWith("infile")   || dataType.startsWith("regexp")  ||
         dataType.startsWith("codon")    || dataType.startsWith("dirlist") )
    {
      appF.setGuiHandleNumber(ntextf);
      ntextf++;
    }
    else if(dataType.startsWith("pattern"))
    {
      appF.setGuiHandleNumber(nmultiTextField);
      nmultiTextField++;
    }
    else if (dataType.startsWith("int"))
    {
      appF.setGuiHandleNumber(nint);
      nint++;
    }
    else if (dataType.startsWith("float"))
    {
      appF.setGuiHandleNumber(nfloat);
      nfloat++;
    }
    else if (dataType.startsWith("bool"))
    {
      appF.setGuiHandleNumber(nbool);
      nbool++;
    }
    else if (dataType.startsWith("seqset") ||
             dataType.startsWith("seqall") ||
             dataType.startsWith("sequence") )
    {
      appF.setGuiHandleNumber(nseqs);
      nseqs++;
    }
    else if (dataType.startsWith("filelist") )
    {
      appF.setGuiHandleNumber(nflist);
      nflist++;
    }
    else if (dataType.startsWith("list") || dataType.startsWith("select"))
    {
      double max = 1.;
      if(isMaxParamValue(numofFields))
        max = Double.parseDouble(getMaxParam(numofFields));

      if(max > 1.0)       // multiple list selection
      {
        appF.setGuiHandleNumber(mlist);
        mlist++;
      }
      else                // single selection
      {
        appF.setGuiHandleNumber(nlist);
        nlist++;
      }
    }
    else if (dataType.startsWith("range"))
    {
      appF.setGuiHandleNumber(nrange);
      nrange++;
    }

  }


  /**
  *
  * Gets the double value of a parameter.
  * @return 	double value of the parameter. 
  *
  */
  public double getParamValueDbl(int field, int param) 
  {
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    return aF.getParamValueDbl(param);
  }


  /**
  *
  * Determine if there is a default parameter in a field of the ACD.
  * @param  field 	field number
  * @return 		true if the default parameter value is a String
  *
  */
  public boolean isDefaultParamValueStr(int field) 
  {
    int num = getNumofParams(field);
    int i;

    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("def")) 
        return isParamValueStr(field,i);
    }
    return false;
  }


  /**
  *
  * Return a double default parameter
  * @param  field 	field number
  * @return 		default parameter value as a double.
  *
  */
  public double getDefaultParamValueDbl(int field) 
  {
    int num = getNumofParams(field);
    
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("def")) 
        return aF.getParamValueDbl(i);
    }
    return 0.0;
  }


//  Methods for dealing with dependent variables. 

  /**
  *
  * Always start by calling isDependents(), which calculates
  * the number of dependents and construct the Dependent array.
  *
  * @param attr 	String name of the attribute
  * @param field 	field number
  * @param numofFields	total number of fields  
  * @return  		true if there are dependent parameters.
  *
  */
  public boolean isDependents(String attr, int field, int numofFields)
  {

    numOfDependents = 0;

    for(int i=field+1;i<numofFields;i++) 
    {
      int num = getNumofParams(i);
      for(int j=0;j<num;j++) 
      {
        if(isParamValueStr(i,j))   
        {
          String val = getParamValueStr(i,j);
          if(val.startsWith("$") || val.startsWith("@")) 
            numOfDependents++;
        }
      }
    }

    dep = new Dependent[numOfDependents];
    
    numOfDependents = 0;
    for(int i=field+1;i<numofFields;i++) 
    {
      int num = getNumofParams(i);

      for(int j=0;j<num;j++) 
      {
        if(isParamValueStr(i,j)) 
        {
          String val = getParamValueStr(i,j);
          if(val.startsWith("$") || val.startsWith("@")) 
          {
            String type = getParameterAttribute(i,j);
            dep[numOfDependents] = new Dependent(i,j,val,type);
            numOfDependents++;            
          }
        } 
      }
    }

    if(numOfDependents>0)
      return true;
    else
      return false;
  }


  /**
  *
  * Gets the dependents associated with the ACD.
  * @return 	the Dependent array for those fields
  *         	with dependents
  *
  */
  public Dependent[] getDependents() 
  {
    return dep;
  }


  /**
  *
  * Gets the number of dependent parameters in the ACD.
  * @return 	number of Dependent making up the array
  *
  */
  public int getNumOfDependents() 
  {
    return numOfDependents;
  }


  /**
  *
  * Locates the min parameter in a field and returns it as a String.
  * @param field	field number
  * @return  		minimum value defined in a field.
  *
  */
  public String getMinParam(int field) 
  {
    int num = getNumofParams(field);
    String min = "";

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("min")) 
      {
        ApplicationFields aF = (ApplicationFields)vappF.get(field);
        if(isParamValueStr(field,i)) 
          min = aF.getParamValueStr(i);
        else 
        {
          Double val = new Double(aF.getParamValueDbl(i));
          if(getParameterAttribute(field,0).startsWith("int"))
            min = (new Integer(val.intValue())).toString();           
          else
            min = val.toString();
        }
        return min;
      }
    }
   
    return min;
  }


  /**
  *
  * Locates the max parameter in a field and returns it as a String.
  * @param field 	field number
  * @return  		maximum value defined in a field.
  *
  */
  public String getMaxParam(int field) 
  {
    int num = getNumofParams(field);
    String max = "";

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("max")) 
      {
        ApplicationFields aF = (ApplicationFields)vappF.get(field);
        if(isParamValueStr(field,i))
          max = aF.getParamValueStr(i);
        else 
        {
          Double val = new Double(aF.getParamValueDbl(i));
          if(getParameterAttribute(field,0).startsWith("int"))
            max = (new Integer(val.intValue())).toString();
          else
            max = val.toString();
        }
        return max;
      }
    }
    return max;
  }


  /**
  *
  * Determine if there is a min parameter in a field of the ACD.
  * @param field 	field number
  * @return  		true if there is a minimum value specified
  *          		in a field.
  *
  */
  public boolean isMinParamValue(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("min")) 
      {
        ApplicationFields aF = (ApplicationFields)vappF.get(field);
        if(isParamValueStr(field,i) )
          if(aF.getParamValueStr(i).startsWith("$"))
            return false;
        return true;
      }
    }
    return false;
  }


  /**
  *
  * Determine if there is a max parameter in a field of the ACD.
  * @param field 	field number
  * @return  		true if there is a maximum value specified
  *          		in a field.
  *
  */
  public boolean isMaxParamValue(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("max"))    
      {
//      if(isParamValueStr(field,i) )
//        if(appF[field].getParamValueStr(i).startsWith("$"))
//          return false;
        return true;
      }
    }
    return false;
  }


  /**
  *
  * Finds the prompt, info or help parameter in an ACD field, in that
  * order.
  * @param field	field number
  * @return 		information specified by the prompt parameter, if
  *          		present else the information parameter
  *          		or the help parameter.
  *
  */
  public String getInfoParamValue(int field) 
  {
    int num = getNumofParams(field);
    int i;

    ApplicationFields aF = (ApplicationFields)vappF.get(field);

    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("prompt")) 
        return aF.getParamValueStr(i);
    }
    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("info")) 
        return aF.getParamValueStr(i);
    }
    for(i=0;i<num;i++)
    {
      if (getParameterAttribute(field,i).startsWith("help"))
        return aF.getParamValueStr(i);
    }

    return "";
  }


  /**
  *
  * Finds the help parameter in an ACD field.
  * @param field 	field number
  * @return  		help parameter defined in a field or a blank
  *          		String if not specified.
  *
  */
  public String getHelpParamValue(int field) 
  {
    int num = getNumofParams(field);
    int i;

    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    for(i=0;i<num;i++) 
      if (getParameterAttribute(field,i).startsWith("help")) 
        return formatHelpText(aF.getParamValueStr(i));

//  for(i=0;i<num;i++) 
//    if (getParameterAttribute(field,i).startsWith("info")) 
//      return formatHelpText(appF[field].getParamValueStr(i));
    
    return "";
  }

  /**
  *
  * Finds if the program is identifies as being able to
  * run in a batch queue
  * @return 	true if the program is suitable for putting
  *         	in a batch queue 
  *
  */
  public boolean isBatchable()
  {

    int num = getNumofParams(0);
    if (num==0)
        return false;
    ApplicationFields aF = (ApplicationFields)vappF.get(0);

    for(int i=0;i<num;i++)
    {
      if(getParameterAttribute(0,i).startsWith("batch"))
        if(aF.getParamValueStr(i).equalsIgnoreCase("Y") ||
           aF.getParamValueStr(i).equalsIgnoreCase("Yes") )
             return true;
    }

    return false;
  }


  /**
  *
  * Finds any expected cpu level indicator (low, medium, high)
  * @return 	cpu level indicator
  *
  */
  public String getExpectedCPU()
  {
    String cpu = "";
    int num = getNumofParams(0);
    ApplicationFields aF = (ApplicationFields)vappF.get(0);
    for(int i=0;i<num;i++)
    {
      if(getParameterAttribute(0,i).startsWith("cpu"))
        return aF.getParamValueStr(i);
    }

    return cpu;
  }

  /**
  *
  * Find URL prefix if an embassy application
  * @return     url prefix 
  *
  */
  public String getUrlPrefix()
  {
    int num = getNumofParams(0);
    ApplicationFields aF = (ApplicationFields)vappF.get(0);
    for(int i=0;i<num;i++)
    {
      if(getParameterAttribute(0,i).startsWith("embassy"))
        return aF.getParamValueStr(i);
    }

    return null;
  }



  /**
  *
  * Limits the length of the line for the help text used in
  * the tool tips.
  *
  * @param help	help text
  * @return  	formated help text.
  *
  */
  protected String formatHelpText(String help) 
  {
    String helpText = "";
    int start = 0;
    int stop;

    help = help.replace('\n',' ');
    while((stop = help.indexOf(' ',55))>0)
    {
      helpText = helpText.concat(help.substring(start,stop) + "\n");   
      help = help.substring(stop+1,help.length());
    }
    helpText = helpText.concat(help);

    return helpText;
  }


  /**
  *
  * Determine if there is a optional parameter in a field of the ACD.
  * @param field 	field number
  * @return  		true if this is an "optional" field.
  *
  */
  public boolean isOptionalParamValue(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if(getParameterAttribute(field,i).startsWith("opt")) 
        return true;
    }
    return false;
  }


  /**
  *
  * Determine if data type of a field is seqout.
  * @param field 	field number
  * @return  		true if this is an "seqout" field.
  *
  */
  public boolean isOutputSequence(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("seqout"))
          return true;
    }
    return false;
  }


  /**
  *
  * Gets the name of the output sequence field (seqout).
  * @param field	field number
  * @return  		parameter name for the seqout data type.
  *
  */
  public String getOutputSequenceFile(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
      if (getParameterAttribute(field,i).startsWith("seqout")) 
          return getParamValueStr(field,i);
    return "";
  }


  /**
  *
  * Determine if a field is data type graph or xygraph.
  * @param field	field number
  * @return 		true if the field is of "graph" or 
  *			"xygraph" type.
  *
  */
  public boolean isOutputGraph(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if ( getParameterAttribute(field,i).startsWith("graph") ||
           getParameterAttribute(field,i).startsWith("xygraph") )
          return true;
    }
    return false;
  }


  /**
  *
  * Determine if a field is data type outfile.
  * @param field	field number
  * @return  		true if the field is of "outfile" type.
  *
  */
  public boolean isOutputFile(int field)   
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("outfile"))
          return true;
    }
    return false;
  }


  /**
  *
  * Gets a String default parameter.
  * @param field	field number
  * @return 		default parameter for this field.
  *
  */
  public String getDefaultParamValueStr(int field) 
  {
    int num = getNumofParams(field);

    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("def")) 
        return aF.getParamValueStr(i);
    }
    return "";
  }


  /**
  *
  * Used for a list data type to put the list items in a String array.
  * @param field	field number
  * @return		String array representation of the list type.
  *
  */
  public String[] getList(int field) 
  {
    int num = getNumofParams(field);

    String delim     = ";";  // list item delimeter default
    String codedelim = ":";  // label delimeter default
    String listAll = null;
    String list[];
    String item;
  
    ApplicationFields aF = (ApplicationFields)vappF.get(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("val")) 
        listAll = aF.getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("delim")) 
        delim = aF.getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("codedelim"))
        codedelim = aF.getParamValueStr(i);

    }

    if(delim == null || listAll == null)
       System.out.println("getList ERROR");

    StringTokenizer st = new StringTokenizer(listAll,delim);

    int n = 0;
    while (st.hasMoreTokens()) 
    {
      st.nextToken(delim);
      n++;
    }

    list = new String[n];
    
    st = new StringTokenizer(listAll);
    n = 0;

    boolean ldef = false;
    Vector def = new Vector();

    //put the default values into a vector
    if(isDefaultParamValueStr(field))
    {
      ldef = true;
      String sdef = getDefaultParamValueStr(field);
      StringTokenizer stdef = new StringTokenizer(sdef);
      while(stdef.hasMoreTokens())
        def.add(stdef.nextToken(delim));
    }
     
    listdefault = new Vector();  
    while (st.hasMoreTokens()) 
    {
      String key = st.nextToken(codedelim);
      item = st.nextToken(delim);

      if(ldef)
      {
        int newline = key.indexOf("\n");
        if(newline > -1)
          key = key.substring(newline+1,key.length());
        newline = key.indexOf(delim);
        if(newline > -1)
          key = key.substring(newline+1,key.length());

        key = key.trim();

        // check if it is a default value
        for(int i=0;i<def.size();i++)
          if(key.equals((String)def.get(i)))
            listdefault.add(new Integer(n));
      }
      item = item.substring(1,item.length()).trim();
      list[n] = new String(item); 
      n++;
    }

    return list;
  }


  /**
  *
  * For a list data type determine the appropriate String entry.
  * @param field	field number
  * @param index	index into the list
  * @return 		String for that entry.
  *
  */
  public String getListLabel(int field, int index) 
  {
    int num = getNumofParams(field);

    String delim     = ";";  // list item delimeter default
    String codedelim = ":";  // label delimeter default
    String listAll = null;
    String key="";

    ApplicationFields aF = (ApplicationFields)vappF.get(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("val"))
        listAll = aF.getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("delim"))
        delim = aF.getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("codedelim"))
        codedelim = aF.getParamValueStr(i);
    }

    if(delim == null || listAll == null)
       System.out.println("getList ERROR");

    StringTokenizer st = new StringTokenizer(listAll,delim);

    st = new StringTokenizer(listAll);

    for(int i=0;i<index+1;i++) 
    {
      key = st.nextToken(codedelim);
      st.nextToken(delim);
    }
    if(index>0)
      key = key.substring(1,key.length()).trim();
    else
      key = key.substring(0,key.length()).trim();

    return key;

  }


  /**
  *
  * Used for a selection type to put the list items in a String array.
  * @param field	field number
  * @return 		String array representation of the select type.
  *
  */
  public String[] getSelect(int field) 
  {
    int num = getNumofParams(field);

    String delim     = ";";  // select item delimeter default
    String listAll = null;
    String list[];
    String item;

    ApplicationFields aF = (ApplicationFields)vappF.get(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("val"))
        listAll = aF.getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("delim"))
        delim = aF.getParamValueStr(i);
    }

    if(delim == null || listAll == null)
       System.out.println("getSelect ERROR");

    StringTokenizer st = new StringTokenizer(listAll,delim);

    int n = 0;
    while (st.hasMoreTokens()) 
    {
      st.nextToken(delim);
      n++;
    }

    list = new String[n];
   
    boolean ldef = false;
    Vector def = new Vector();
    //put the default values into a vector
    if(isDefaultParamValueStr(field))
    {
      ldef = true;
      String sdef = getDefaultParamValueStr(field);
      StringTokenizer stdef = new StringTokenizer(sdef);
      while(stdef.hasMoreTokens())
        def.add(stdef.nextToken(delim).trim());
    }

    st = new StringTokenizer(listAll);
    n = 0;
    listdefault = new Vector();

    while (st.hasMoreTokens()) 
    {
      item = st.nextToken(delim);
      if(ldef)
      {
//      if (item.endsWith(getDefaultParamValueStr(field)))
//        listdefault.add(new Integer(n));

        for(int i=0;i<def.size();i++)
          if( item.equals((String)def.get(i)) ||
              Integer.toString(n+1).equals((String)def.get(i)) )
            listdefault.add(new Integer(n));
      }

      item = item.substring(0,item.length()).trim();
      list[n] = new String(item);
      n++;
    }

    return list;
  }


  /**
  *
  * Use this after getList or getSelect to retrieve default
  * @return 	default for list or select data type
  *
  */
  public Vector getListOrSelectDefault()
  {
    return listdefault;
  }


  /**
  *
  * Determine if there is a optional parameter in any field of the ACD.
  * @param 	true if there is an optional parameter in any field.
  *
  */
  public boolean isOptionalParam()
  {
    for(int i=0;i<numofFields;i++)
      if(isOptionalParamValue(i)) return true; 
    
    return false;
  }

  /**
  *
  * Gets the number of fields in the ACD file.
  * @return  	total number of fields in an ACD file
  *
  */
  public int getNumofFields() 
  {
    return numofFields;
  }


  /**
  *
  * Gets the number of parameters in a ACD field.
  * @param field	field number
  * @return 		number of parameters in the field.
  *
  */ 
  public int getNumofParams(int field) 
  {
    if (vappF.size()==0)
        return 0;
    ApplicationFields aF = (ApplicationFields)vappF.get(field);
    return aF.getNumberOfParam();
  }


  /**
  *
  * Parses a parameter in a ACD field 
  * @param in	BufferedReader 
  * @param st	tokenizer
  * @return 	string tokenizer type or zero
  *
  */
  public int parseParam(BufferedReader in, StreamTokenizer st) throws IOException 
  {
 
    char c;
    svalue = null;   
    st.eolIsSignificant(false);
   
// the following are not token delimeters
    st.wordChars((int)'-',(int)'-'); st.wordChars((int)'$',(int)'$');
    st.wordChars((int)'(',(int)'('); st.wordChars((int)')',(int)')');
    st.wordChars((int)'@',(int)'@'); st.wordChars((int)'?',(int)'?');
    st.wordChars((int)'!',(int)'!'); st.wordChars((int)'#', (int)'#');
// the following are token delimeters
    st.whitespaceChars((int)'\n',(int)'\n');
    st.whitespaceChars((int)' ',(int)' ');
    st.whitespaceChars((int)':',(int)':');
    st.whitespaceChars((int)'=',(int)'=');
    st.ordinaryChars((int)'\"', (int)'\"');
    st.ordinaryChars((int)'\'',(int)'\'');

    st.nextToken();
    attr = st.sval;
    if(attr == null) return 0;

//skip commented lines
    if(attr.startsWith("#")) 
    {
      while( (c = (char)in.read()) != '\n') {}
      st.nextToken();
      attr = st.sval;
    }
    if(attr == null) return 0;
 
    st.nextToken();
    svalue = st.sval;
    nvalue = st.nval;
 
   // cope with double quotes by forwarding to end quote
   // and remove unwanted white space
    if( svalue == null &&
        st.ttype != java.io.StreamTokenizer.TT_NUMBER ) 
    {
      svalue = "";
      char last = ' ';
      while( (c = (char)in.read()) != '\"') 
      {
        if( c != ' ' || last != ' ')
          svalue = svalue.concat(java.lang.String.valueOf(c));
        last = c;
      }
    }
 
    numofParams++;
    return st.ttype;
  }


  public int getNumMultiTextField()
  {
    return nmultiTextField;
  }
  
}


