/****************************************************************
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  Based on EmbreoMakeFileSafe
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.io.*;

/**
*
* Create a sanitised filename. All undesirable characters in the
* filename are replaced by underscores.
*
*/
public class MakeFileSafe 
{

  private String safeFileName;

  /**
  *
  * @param unSafeFileName unsanitised name of the file
  *
  */
  public MakeFileSafe(String unSafeFileName) 
  {
    char c;
    int len = unSafeFileName.length();
    StringBuffer dest = new StringBuffer(len);

    for(int i=0 ; i<len; i++) 
    {
      c = unSafeFileName.charAt(i);
      if(c == ':') 
	dest.append('_');
      else if(c == '/') 
	dest.append('_');
      else if(c == ' ') 
	dest.append('_');
      else if(c == '>')
	dest.append('_');
      else if(c == '<')
	dest.append('_');
      else if(c == ';')
	dest.append('_');
      else if(c == '\\')
	dest.append('_');
      else 
	dest.append(c);
      
    }
    safeFileName = dest.toString();
  }

  /**
  *
  * Get the safe/sanitised file name
  * @return 	sanitised file name
  *
  */
  public String getSafeFileName() 
  {
    return safeFileName;
  }
}
