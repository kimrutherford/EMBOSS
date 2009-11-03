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

import javax.swing.*;
import java.awt.*;
import java.awt.print.*;

public class PrintableJTable extends JTable implements Printable
{

  public PrintableJTable()
  {
    super();
  }

  public PrintableJTable(Object[][] rowData,String[] columnNames)
  {
    super(rowData,columnNames);
  }

  /**
  *
  * The method @print@ must be implemented for @Printable@ interface.
  * Parameters are supplied by system.
  *
  */
  public int print(Graphics g, PageFormat pf, int pageIndex)
                                       throws PrinterException 
  {
    Graphics2D g2 = (Graphics2D)g;
    g2.setColor(Color.black);    //set default foreground color to black

    RepaintManager.currentManager(this).setDoubleBufferingEnabled(false);
    Dimension d = this.getSize();    //get size of document
    double panelWidth  = d.width;    //width in pixels
    double panelHeight = d.height;   //height in pixels
    double pageHeight = pf.getImageableHeight();   //height of printer page
    double pageWidth  = pf.getImageableWidth();    //width of printer page
    double scale = pageWidth/panelWidth;
    int totalNumPages = (int)Math.ceil(scale * panelHeight / pageHeight);
    // Make sure not print empty pages
    if(pageIndex >= totalNumPages) 
     return Printable.NO_SUCH_PAGE;

    // Shift Graphic to line up with beginning of print-imageable region
    g2.translate(pf.getImageableX(), pf.getImageableY());
    // Shift Graphic to line up with beginning of next page to print
    g2.translate(0f, -pageIndex*pageHeight);
    // Scale the page so the width fits...
    g2.scale(scale, scale);
    this.paint(g2);   //repaint the page for printing
    return Printable.PAGE_EXISTS;
  }


  public void doPrintActions()
  {
    PrinterJob pj=PrinterJob.getPrinterJob();
    pj.setPrintable(this);
    pj.printDialog();
    try
    { 
     pj.print();
    }
    catch (Exception printException)
    {
      printException.printStackTrace();
    }
  }


}

