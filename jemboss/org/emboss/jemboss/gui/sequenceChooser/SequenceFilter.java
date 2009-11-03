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

package org.emboss.jemboss.gui.sequenceChooser; 

import java.io.File;
import javax.swing.*;
import javax.swing.filechooser.*;

/**
*
* Sequence format file name suffix filter
*
*/
public class SequenceFilter extends FileFilter 
{
    
    public final static String fasta = "fasta";
    public final static String msf   = "msf";
    public final static String gcg   = "gcg";
    public final static String embl  = "embl";
    public final static String swiss = "swiss";

    /**
    *  Accept all directories and all sequence files
    *  (*.fasta, *.msf, *.gcg, *.embl, *.swiss)
    *
    */
    public boolean accept(File f) 
    {
        if (f.isDirectory()) 
            return true;

        String extension = getExtension(f);
        if (extension != null) 
        {
          if (extension.equals(fasta) ||
              extension.equals(msf) ||
              extension.equals(gcg) ||
              extension.equals(embl) ||
              extension.equals(swiss)) 
            return true;
          else 
            return false;
        }

        return false;
    }
    
    /** 
    *
    *  @return 	description of this filter
    *
    */
    public String getDescription() 
    {
        return "*.fasta, *.msf, *.gcg, *.embl, *.swiss";
    }

    /**
    *
    * Get the extension of a file
    * @param f	file 
    * @return 	extension of the file
    *
    */
    public static String getExtension(File f) 
    {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 &&  i < s.length() - 1) 
        {
            ext = s.substring(i+1).toLowerCase();
        }
        return ext;
    }

}
