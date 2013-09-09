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


package org.emboss.jemboss.gui.filetree;

import java.awt.Color;
import java.awt.Dimension;
import javax.swing.*;
import javax.swing.text.*;
import java.io.*;

/**
*
* Display for text, sequence editing, html, gif, png, jpeg.
*
*/
public class FileEditorDisplay extends JTextPane
{

   /** png file content */
   private byte[] pngContent = null;
   /** file name */
   private String filename;

   /**
   *
   * @param filename	file name to display
   *
   */
   public FileEditorDisplay(final String filename)
   {
 
     this.filename = filename;
     initStylesForTextPane();

     String text = "";
     try
     {
       BufferedReader in = new BufferedReader(new FileReader(filename));
       String line;
       try {
    	   while((line = in.readLine()) != null)
    		   text = text.concat(line + "\n");
       }
       finally {
    	   in.close();
       }
     }
     catch (IOException ioe)
     {
       System.out.println("Cannot read file: " + filename);
     }

     if(filename.endsWith(".html"))
       setText(text,"html");
     else if( filename.toLowerCase().endsWith(".png") || 
              filename.toLowerCase().endsWith(".gif") ||
              filename.toLowerCase().endsWith(".jpeg") ||
              filename.endsWith(".tif") )
       setText(text,"png");
     else if( filename.toLowerCase().endsWith(".z")  ||
              filename.toLowerCase().endsWith(".gz") ||
              filename.toLowerCase().endsWith(".zip")||
              filename.toLowerCase().endsWith(".tar")||
              filename.toLowerCase().endsWith(".ppt")||
              filename.toLowerCase().endsWith(".doc")||
              filename.toLowerCase().endsWith(".ps") )
       setText("Jemboss Cannot display "+filename,"regular");
     else
       setText(text,"regular");
   }

   /**
   *
   * @param filename    file name to display
   * @param contents	contents of file
   *
   */
   public FileEditorDisplay(final String filename,
                            Object contents)
   {
     
     this.filename = filename;
     initStylesForTextPane();

     if(contents.getClass().equals(String.class))
     {
       if(!filename.endsWith(".html"))
         setText((String)contents,"regular");
       else
         setText((String)contents,"html");
     }
     else if( filename.toLowerCase().endsWith(".png") ||
              filename.toLowerCase().endsWith(".gif") ||
              filename.toLowerCase().endsWith(".jpeg") ||
              filename.endsWith(".tif") )
     {
       ImageIcon icon = new ImageIcon((byte [])contents);
       insertIcon(icon);
       pngContent = (byte [])contents;
     }
     else if( filename.toLowerCase().endsWith(".z")  ||
              filename.toLowerCase().endsWith(".gz") ||
              filename.toLowerCase().endsWith(".zip")||
              filename.toLowerCase().endsWith(".tar")||
              filename.toLowerCase().endsWith(".ppt")||
              filename.toLowerCase().endsWith(".doc")||
              filename.toLowerCase().endsWith(".ps") )
       setText("Jemboss Cannot display "+filename,"regular");
     else
     {
       try
       {
         setText(new String((byte [])contents),"regular");
       }
       catch (Exception exp)
       {
         setText("Cannot display "+filename,"regular");
       }
     }

   }

  /**
  *
  * Override to avoid line wrapping.
  * @param d	dimension
  *
  */
   public void setSize(Dimension d)
   {
     if (d.width < getParent().getSize().width)
       d.width = getParent().getSize().width;

     super.setSize(d);
   }

  /**
  *
  * Override to avoid line wrapping.
  *
  */
   public boolean getScrollableTracksViewportWidth()
   {
     return false;
   }
                           
// public JTextPane getJTextPane()
// {
//   return this; 
// }  


  /**
  *
  *  Set the text & style in the JTextPane.
  *  @param text 	text to be put into the JTextPane.
  *  @param type 	style to use ("regular", "sequence", "png", "html").
  *
  */
  public void setText(String text, String type)
  {

    Document doc = new DefaultStyledDocument();
     
    if(type.equalsIgnoreCase("regular"))
    {
      try
      {
        doc.insertString(0, text, getStyle("regular"));
      }
      catch (BadLocationException ble)
      {
        System.err.println("Couldn't insert initial text.");
      }
    }
    else if(type.equalsIgnoreCase("sequence"))
    {
      try
      { 
        String hdr = findHeader(text);
        doc.insertString(doc.getLength(), hdr, getStyle("bold"));
        for (int i=hdr.length(); i < text.length(); i++)
        {
          String rescol = new String("darkGray ");
          if(text.substring(i,i+1).equalsIgnoreCase("A"))
            rescol = "green";
          else if(text.substring(i,i+1).equalsIgnoreCase("T"))
            rescol = "red";
          else if(text.substring(i,i+1).equalsIgnoreCase("G"))
            rescol = "black";
          else if(text.substring(i,i+1).equalsIgnoreCase("C"))
            rescol = "blue";
          doc.insertString(doc.getLength(), text.substring(i,i+1),
                            getStyle(rescol));
        }
      }
      catch (BadLocationException ble)
      {
        System.err.println("Couldn't insert initial text.");
      }
    } 
    else if(type.equalsIgnoreCase("png"))
    {
      ImageIcon icon = new ImageIcon(filename,filename); 
      insertIcon(icon);
      pngContent = loadPNGContent(filename);
    }
    else if(type.equalsIgnoreCase("html"))
    {
      setContentType("text/html");
      setText(text);
      setEditable(false);
    }
    setDocument(doc);
  }

  /**
  *
  *  Need to read png in, in case it is saved out
  *  @param filename	file name to read
  *  @return 		contents of file as a byte array
  *
  */
  private byte[] loadPNGContent(String filename)
  {

    DataInputStream dis;
    FileInputStream fis;
    int nby = 0;
    byte data[] = new byte[1];
    try
    {
      fis = new FileInputStream(filename);
      dis = new DataInputStream(fis);
      while(true)
      {
        dis.readByte();
        nby++;
      }
    }
    catch (EOFException eof){}
    catch (IOException ioe){}

    if(nby >0)
    {
      try
      {
        data = new byte[nby];
        fis = new FileInputStream(filename);
        dis = new DataInputStream(fis);
        nby=0;
        while(true)
        {
          data[nby]=dis.readByte();
          nby++;
        }
      }
      catch (EOFException eof){}
      catch (IOException ioe){}
    }

    return data; 
  }

  /**
  *
  *  Returns the content of a png file or null.
  *  @return 	byte content of a png file
  *
  */
  public byte[] getPNGContent()
  {
    return pngContent;
  }


  /**
  *
  *  Initialise styles for a JTextPane.
  *
  */
  protected void initStylesForTextPane() 
  {
     //Initialize some styles.
    Style def = StyleContext.getDefaultStyleContext().
                             getStyle(StyleContext.DEFAULT_STYLE);

    Style regular = addStyle("regular", def);
    StyleConstants.setFontFamily(def, "monospaced");

    Style s = addStyle("italic", regular);
    StyleConstants.setItalic(s, true);

    s = addStyle("bold", regular);
    StyleConstants.setBold(s, true);

    s = addStyle("small", regular);
    StyleConstants.setFontSize(s, 10);

    s = addStyle("large", regular);
    StyleConstants.setFontSize(s, 16);

    s = addStyle("red", regular);
    StyleConstants.setForeground(s,Color.red);
    StyleConstants.setBold(s, true);
    StyleConstants.setFontFamily(def, "monospaced");

    s = addStyle("green", regular);
    StyleConstants.setForeground(s,Color.green);
    StyleConstants.setBold(s, true);
    StyleConstants.setFontFamily(def, "monospaced");

    s = addStyle("blue", regular);
    StyleConstants.setForeground(s,Color.blue);
    StyleConstants.setBold(s, true);
    StyleConstants.setFontFamily(def, "monospaced");

    s = addStyle("orange", regular);
    StyleConstants.setForeground(s,Color.orange);
    StyleConstants.setBold(s, true);
    StyleConstants.setFontFamily(def, "monospaced");

    s = addStyle("black", regular);
    StyleConstants.setForeground(s,Color.black);
    StyleConstants.setBold(s, true);
    StyleConstants.setFontFamily(def, "monospaced");

    s = addStyle("darkGray", regular);
    StyleConstants.setForeground(s,Color.darkGray);
    StyleConstants.setBold(s, true);
    StyleConstants.setFontFamily(def, "monospaced");

    s = addStyle("icon", regular);
    StyleConstants.setAlignment(s, StyleConstants.ALIGN_CENTER);

  }

  /**
  *
  *  Sequence header text.
  *  @param text	contents of the file.
  *
  */
  private String findHeader(String text)
  {
    String hdr = "";
    String tmphdr = "";
    BufferedReader in = new BufferedReader(new StringReader(text));

    try
    {
      String line = in.readLine();
      if(line.startsWith(">"))                   //fasta
        hdr = line;
      else
      {
        tmphdr = line;
        while((line = in.readLine()) != null)
        {
          tmphdr = tmphdr.concat("\n" + line );
         
          if(line.equals("//") || line.startsWith("SQ ") 
                               || line.endsWith(".."))   //msf, embl, gcg
          {
            hdr = tmphdr;
            break;
          }              
        }
      }
    }
    catch( IOException ioe)
    {
      System.out.println("Cannot read " + text);
    }
 
    return hdr;
  }

}

