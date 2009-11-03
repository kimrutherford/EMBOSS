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

import java.awt.print.*;
import javax.swing.*;

/**
*
* Class for printing the alignment
*
*/
public class PrintAlignment
{

  /**
  *
  * @param gsc		sequence panel
  *
  */
  public PrintAlignment(GraphicSequenceCollection gsc)
  {
    PrinterJob printerJob = PrinterJob.getPrinterJob();
    Book book = new Book();
    PageFormat format = new PageFormat();
    format = printerJob.pageDialog(format);
   
    int nresPerPage=  showOptions(gsc,format);
    gsc.setNumberOfResiduesPerLine(nresPerPage);

    book.append(gsc,format, 
                gsc.getNumberPages(format,nresPerPage));
    printerJob.setPageable(book);
    if(printerJob.printDialog()) 
    {
      try
      {
        printerJob.print();
      } 
      catch (PrinterException exception)
      {
        System.err.println("Printing error: " + exception);
      }
    }
  }

  /**
  *
  * Provide some options for the image created
  * @param gsc		sequence panel
  * @param format	page format
  * @return		number of residues per line
  *
  */
  private int showOptions(GraphicSequenceCollection gsc,
                          PageFormat format)
  {
    JPanel joptions = new JPanel();
// no. of residues per line
    Box XBox = Box.createHorizontalBox();
    String mres = Integer.toString(gsc.getResiduesPerLine(format));
    JLabel jres = new JLabel("Residues per line: [max:"+mres+"]");
    JTextField maxResiduesField = new JTextField(mres);
    XBox.add(jres);
    XBox.add(maxResiduesField);
    XBox.add(Box.createHorizontalGlue());
    joptions.add(XBox);

    JOptionPane.showMessageDialog(null,joptions,"Options",
                               JOptionPane.PLAIN_MESSAGE);

    return Integer.parseInt(maxResiduesField.getText());
  }

}

