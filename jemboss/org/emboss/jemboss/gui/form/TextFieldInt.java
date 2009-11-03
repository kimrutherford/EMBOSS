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

package org.emboss.jemboss.gui.form;

import javax.swing.text.*; 

import java.awt.Toolkit;
import java.text.NumberFormat;
import java.text.ParseException;
import java.util.Locale;

/**
*
* JTextfield for int fields in the EMBOSS form
*
*/
public class TextFieldInt extends TextFieldSink 
{

  private Toolkit toolkit;
  private NumberFormat integerFormatter;

  public TextFieldInt() 
  {
    super();
    toolkit = Toolkit.getDefaultToolkit();
    integerFormatter = NumberFormat.getNumberInstance(Locale.UK);
    integerFormatter.setParseIntegerOnly(true);
  }

  public int getValue() 
  {
    int retVal = 0;
    try 
    {
      retVal = integerFormatter.parse(getText()).intValue();
    } 
    catch (ParseException e) 
    {
      // This should never happen because insertString allows
      // only properly formatted data to get in the field.
      //toolkit.beep();
    }
    return retVal;
  }

  public void setValue(int value) 
  {
    setText(integerFormatter.format(value));
  }

  protected Document createDefaultModel() 
  {
    return new WholeNumberDocument();
  }

  protected class WholeNumberDocument extends PlainDocument 
  {
    public void insertString(int offs, 
                             String str,
                             AttributeSet a) 
              throws BadLocationException 
    {
      char[] source = str.toCharArray();
      char[] result = new char[source.length];
      int j = 0;

      for (int i = 0; i < result.length; i++) 
      {
        if (Character.isDigit(source[i]) ||
            source[i] == '-')
          result[j++] = source[i];
        else 
        {
          if(source[i] != ',')
          {
            toolkit.beep();
            System.err.println("invalid character entered: " + source[i]);
          }
        }
      }
      super.insertString(offs, new String(result, 0, j), a);
    }
  }
}

