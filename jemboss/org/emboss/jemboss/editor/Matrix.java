/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.editor;

import java.io.*;
import java.util.StringTokenizer;
import java.util.Hashtable;
import java.util.Arrays;
import javax.swing.JOptionPane;

import org.apache.regexp.*;
import org.emboss.jemboss.JembossJarUtil;

/**
*
* Reads and hold a sequence scoring matrix 
*
*/
public class Matrix
{
  private int matrix[][];
  private int idimension;
  private int jdimension;

  private int i=0;
  private int k=0;

  /** hashtable of the residue positions in the table */
  private Hashtable residueMatrixPosition;
  private Object[] keys = null;
  //private String cons = "";
  private String matrixString = null;
  private String matrixFileName = null;
  
  public static final String DEFAULT_MATRIX = "EBLOSUM62";
  private String matrixFilesLocation;
  private int locationType;
  private static int LOCATION_JAR_ARCHIVE = 1;
  private static int LOCATION_DATA_DIR = 2;

  /**
  *
  * @param matrixFile	matrix file
  *
  */
  public Matrix(File matrixFile)
  {
    this.matrixFileName = matrixFile.getName();
    matrixRead(matrixFile);
  }

  /**
  *
  * @param matrixJar		jar file containing scoring matrix
  * @param matrixFileName	matrix file
  *
  */
  public Matrix(String matrixFilesLocation, String matrixFileName)
  {
    this.matrixFileName = matrixFileName;
    this.matrixFilesLocation = matrixFilesLocation;
    File mfl = new File(matrixFilesLocation);
    if (mfl.isDirectory()){
        keys = mfl.list(new FilenameFilter(){
            public boolean accept(File dir, String name) {
                if (name.startsWith("EPAM") ||
                        name.startsWith("EBLOSUM") ||
                        name.startsWith("EDNA"))
                    return true;
                return false;
            }});
        File m = new File(matrixFilesLocation+"/"+matrixFileName);
        try {
          matrixString = readFileAsString(m);
          matrixReadString(matrixString);
          locationType = LOCATION_DATA_DIR;
          Arrays.sort(keys);
          return;
      } catch (IOException e) {
      }
    }
    String matrixJar = matrixFilesLocation;
    try
    {
      Hashtable matrixHash = (new JembossJarUtil(matrixJar)).getHash();
      keys = matrixHash.keySet().toArray();
      Arrays.sort(keys);
      if(matrixHash.containsKey(matrixFileName))
      {
        matrixString = new String((byte[])matrixHash.get(matrixFileName));
        matrixReadString(matrixString);
        locationType = LOCATION_JAR_ARCHIVE;
      }
      else
        System.err.println("Matrix file "+matrixFileName+
                    " not found in jar file "+matrixJar);
    }
    catch(Exception exp)
    {
      JOptionPane.showMessageDialog(null,
                 "Failed to read "+matrixFileName+
                 "\nfrom the matrix archive "+matrixJar,
                 "Missing matrix archive",
                  JOptionPane.ERROR_MESSAGE);
    }
  }

  
  public void changeMatrix(String matrixFileName){
      i = 0;
      k = 0;
      if (locationType == LOCATION_DATA_DIR){
          File m = new File(matrixFilesLocation+"/"+matrixFileName);
          try {
            matrixString = readFileAsString(m);
            matrixReadString(matrixString);
        } catch (IOException e) {
        }
        
        this.matrixFileName = matrixFileName;
        
        return;
      }
      String matrixJar = matrixFilesLocation;
      try
      {
        //TODO: shouldn't read the matrixHash each time
        Hashtable matrixHash = (new JembossJarUtil(matrixJar)).getHash();
        if(matrixHash.containsKey(matrixFileName))
        {
          matrixString = new String((byte[])matrixHash.get(matrixFileName));
          matrixReadString(matrixString);
        }
        else
          System.err.println("Matrix file "+matrixFileName+
                      " not found in jar file "+matrixJar);
      }
      catch(Exception exp)
      {
        JOptionPane.showMessageDialog(null,
                   "Failed to read "+matrixFileName+
                   "\nfrom the matrix archive "+matrixJar,
                   "Missing matrix archive",
                    JOptionPane.ERROR_MESSAGE);
      }
      
      this.matrixFileName = matrixFileName;
  }
  
  
  final String readFileAsString(File source) throws IOException
  {
      final DataInputStream dis = new DataInputStream(new BufferedInputStream(new FileInputStream(source)));
      final byte[] buffer = new byte[(int)source.length()];
      dis.readFully(buffer);
      dis.close();
      return new String(buffer);
  }
  
  /**
  *
  * Get the scoring matrix as a 2 dimensional integer
  * array
  * @return	2 dimentional scoring matrix
  * 
  */
  public int[][] getMatrix()
  {
    return matrix;
  }

  /**
  *
  * Get the scoring matrix as text 
  * @return     scoring matrix
  *
  */
  public String getMatrixTable()
  {
    return matrixString;
  }

  /**
  *
  * Get the current scoring matrix name
  * @return 	name of matrix
  *
  */
  public String getCurrentMatrixName()
  {
    int index = matrixFileName.lastIndexOf("/");
    if(index > -1)
      return matrixFileName.substring(index+1);
    return matrixFileName;
  }

  /**
  *
  * Scoring matrix filenames in jar file
  *
  */
  public Object[] getKeys()
  {
    return keys;
  }

  /**
  *
  * Scoring matrix names available
  *
  */
  public Object[] getKeyNames()
  {
    try
    {
      int nkeys = keys.length;
      Object[] kname = new Object[nkeys];
      for(int i=0;i<nkeys;i++)
      {
        String k = (String)keys[i];
        if(k.indexOf("MANIFEST.MF") == -1)
        {
          int pos  = k.indexOf("/")+1;
          kname[i] = k.substring(pos);
        }
      }
      return kname;
    }
    catch(NullPointerException npe)
    {
      JOptionPane.showMessageDialog(null, 
                 "No matrix files found!", 
                 "Matrix files missing",
                  JOptionPane.ERROR_MESSAGE); 
      return null;  
    }
  }


  /**
  *
  * Scoring matrix names available
  *
  */
  public String getKeyNamesString()
  {
    try
    {
      int nkeys = keys.length;
      StringBuffer kname = new StringBuffer();
      for(int i=0;i<nkeys;i++)
      {
        String k = (String)keys[i];
        if(k.indexOf("MANIFEST.MF") == -1)
        {
          int pos = k.indexOf("/")+1;
          kname.append(k.substring(pos)+"\n");
        }
      }
      return kname.toString();
    }
    catch(NullPointerException npe)
    {
      JOptionPane.showMessageDialog(null,
                 "No matrix files found!",
                 "Matrix files missing",
                  JOptionPane.ERROR_MESSAGE);
      return null;
    }
  }


  /**
  *
  * Get the hashtable of the residue positions in the scoring table
  * @return 	hashtable of the residue positions
  *
  */
  public Hashtable getResidueMatrixPosition()
  {
    return residueMatrixPosition;
  }

  /**
  *
  * Get the residue position in the scoring table
  * @param s	residue
  * @return	position in scoring table
  *
  */
  public int getMatrixIndex(String s)
  {
    s = s.toUpperCase();
    
    if(residueMatrixPosition==null)
        return -1;
    
    if(!residueMatrixPosition.containsKey(s))
      if(s.equals(".") || s.equals("-") 
                       || s.equals("~"))
        s = "X";

    if(!residueMatrixPosition.containsKey(s))
      return -1;
    return ((Integer)residueMatrixPosition.get(s)).intValue();
  }

  /**
  *
  * Get number of rows in the scoring table
  * @return 	number of rows in the scoring table
  *
  */
  public int getIDimension()
  {
    return idimension;
  }

  /**
  *
  * Get number of columns in the scoring table
  * @return	number of columns in the scoring table
  * 
  */
  public int getJDimension()
  {
    return jdimension;
  }

  /**
  *
  * Get regular expression for values in the scoring table
  * @return	RE
  *
  */
  private RE getRegularExpression()
  {
    RE regexp = null;

    try
    {
      RECompiler rec = new RECompiler();
      REProgram  rep = rec.compile("[:digit:]");
      regexp = new RE(rep);
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }
    return regexp;
  }

  /**
  *
  * Count as a matrix row and number of columns
  * @param line		line from matrix file
  * @param regexp	regular expression for columns
  *
  */
  private void matrixLineCount(String line, RE regexp)
  {
    String delim = " :\t\n";
    if(!line.startsWith("#") && !line.equals("")
                             && regexp.match(line))
    {
      jdimension = 0;
      idimension++;
      line = line.trim();
      StringTokenizer st = new StringTokenizer(line,delim);
      while (st.hasMoreTokens())
      {
        st.nextToken();
        jdimension++;
      }
    }
  }

  /**
  *
  * Parse a line from a scoring matrix file. Add to the
  * matrix array the values in the table.
  * @param line		line from matrix file
  * @param regexp	regular expression for values
  *
  */
  private void matrixLineParse(String line, RE regexp)
  {
    String delim = " :\t\n";
    int j = 0;

    if(!line.startsWith("#") && !line.equals(""))
    {
      line = line.trim();
      StringTokenizer st = new StringTokenizer(line,delim);
      if(!regexp.match(line))
      {
        while (st.hasMoreTokens())
        {
          residueMatrixPosition.put(st.nextToken(),new Integer(k));
          k++;
        }
      }
      else
      {
        st.nextToken();
        while (st.hasMoreTokens())
        {
          String s = st.nextToken();
          matrix[i][j] = Integer.parseInt(s);
          j++;
        }
        i++;
      }
    }
  }

  /**
  *
  * Read and parse a scoring matrix file 
  * @param matrixFile	matrix file
  * @return		matrix 2-d integer array
  *
  */
  private int[][] matrixRead(File matrixFile)
  {
    //String delim = " :\t\n";
    String line  = "";
    BufferedReader in;
    residueMatrixPosition = new Hashtable();

    idimension = 1;
    RE regexp = getRegularExpression();

// determine dimensions of the matrix
    try
    {
      in = new BufferedReader(new FileReader(matrixFile));
      while((line = in.readLine()) != null )
        matrixLineCount(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }

//  initialise matrix
    matrix = new int[idimension][jdimension];

// read the matrix 
    try
    {
      in = new BufferedReader(new FileReader(matrixFile));
      while((line = in.readLine()) != null )
        matrixLineParse(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }
    return matrix;
  }

  /**
  *
  * Read and parse a scoring matrix 
  * @param matrixString 	matrix as text
  * @return             	matrix 2-d integer array
  *
  */
  private int[][] matrixReadString(String matrixString)
  {
    //String delim = " :\t\n";
    String line  = "";
    BufferedReader in;
    residueMatrixPosition = new Hashtable();

    idimension = 1;
    RE regexp = getRegularExpression();

// determine dimensions of the matrix
    try
    {
      in = new BufferedReader(new StringReader(matrixString));
      while((line = in.readLine()) != null )
        matrixLineCount(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }

//  initialise matrix
    matrix = new int[idimension][jdimension];

// read the matrix
    try
    {
      in = new BufferedReader(new StringReader(matrixString));
      while((line = in.readLine()) != null )
        matrixLineParse(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }
    return matrix;
  }

}

