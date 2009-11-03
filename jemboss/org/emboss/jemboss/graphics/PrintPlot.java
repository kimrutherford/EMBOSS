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

package org.emboss.jemboss.graphics;


import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import javax.swing.*;
import java.io.*;


import org.emboss.jemboss.gui.ScrollPanel;

public class PrintPlot extends ScrollPanel
{

  private Graph2DPlot plot;

  public PrintPlot(Graph2DPlot plot)
  {
    super();
    this.plot = plot;
  }

  /**
  *
  * Print to a jpeg or png file
  *
  */
  public void print()
  {
    // file chooser
    String cwd = System.getProperty("user.dir");
    JFileChooser fc = new JFileChooser(cwd);
    File fselect = new File(cwd+
                            System.getProperty("file.separator")+
                            "jemboss.png");
    fc.setSelectedFile(fselect);

    // file name prefix
    Box YBox = Box.createVerticalBox();
    JLabel labFormat = new JLabel("Select Format:");
    Font font = labFormat.getFont();
    labFormat.setFont(font.deriveFont(Font.BOLD));
    YBox.add(labFormat);

    Box bacross = Box.createHorizontalBox();
    JComboBox formatSelect =
       new JComboBox(javax.imageio.ImageIO.getWriterFormatNames());
    formatSelect.setSelectedItem("png");

    Dimension d = formatSelect.getPreferredSize();
    formatSelect.setMaximumSize(d);
    bacross.add(Box.createHorizontalGlue());
    bacross.add(formatSelect);
    YBox.add(bacross);

    // file prefix & format options
    fc.setAccessory(YBox);
    int n = fc.showSaveDialog(null);
    if(n == JFileChooser.CANCEL_OPTION)
      return;
    // remove file extension
    String fsave = fc.getSelectedFile().getAbsolutePath().toLowerCase();
    if(fsave.endsWith(".png") ||
       fsave.endsWith(".jpg") ||
       fsave.endsWith(".jpeg") )
    {
      int ind = fsave.lastIndexOf(".");
      fsave = fc.getSelectedFile().getAbsolutePath();
      fsave = fsave.substring(0,ind);
    }
    else
      fsave = fc.getSelectedFile().getAbsolutePath();

    // image type
    String ftype = (String)formatSelect.getSelectedItem();
    try
    {
      RenderedImage rendImage = createImage();
      writeImageToFile(rendImage, new File(fsave+"."+ftype),
                       ftype);
    }
    catch(NoClassDefFoundError ex)
    {
      JOptionPane.showMessageDialog(this,
            "This option requires Java 1.4 or higher.");
    }
  }

  /**
  *
  *  Returns a generated image
  *  @param pageIndex   page number
  *  @return            image
  *
  */
  private RenderedImage createImage()
  {
    // Create a buffered image in which to draw
    BufferedImage bufferedImage = new BufferedImage(
                                  plot.getWidth(),plot.getHeight(),
                                  BufferedImage.TYPE_INT_RGB);
    // Create a graphics contents on the buffered image
    Graphics2D g2d = bufferedImage.createGraphics();
    plot.printComponent(g2d);

//  paintComponent(g2d);

    return bufferedImage;
  }

  /**
  *
  * Write out the image
  * @param image        image
  * @param file         file to write image to
  * @param type         type of image
  *
  */
  private void writeImageToFile(RenderedImage image,
                               File file, String type)
  {
    try
    {
      javax.imageio.ImageIO.write(image,type,file);
    }
    catch ( IOException e )
    {
      System.out.println("Java 1.4+ is required");
      e.printStackTrace();
    }
  }

}

