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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.text.DecimalFormat;
import java.awt.geom.AffineTransform;

import org.emboss.jemboss.editor.SequenceProperties;
import org.emboss.jemboss.gui.filetree.FileEditorDisplay;
import org.emboss.jemboss.gui.form.TextFieldInt;
import org.emboss.jemboss.gui.form.TextFieldFloat;
import org.emboss.jemboss.gui.ScrollPanel;
 
/**
*
* Use java 2D to draw data points from EMBOSS applications.
*
*/
public class Graph2DPlot extends ScrollPanel
{
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

  private Object[][] emboss_data;
  private float xmin = 0;
  private float xmax = 0;
  private float ymin = 0;
  private float ymax = 0;
  
  private float xmin_screen = 0;
  private float xmax_screen = 0;
  private float ymin_screen = 0;
  private float ymax_screen = 0;

  private StringBuffer graph_data;
  private Color graph_colour = Color.black;
  private TextFieldFloat graph_line;

  private TextFieldFloat xstart;
  private TextFieldFloat xend;

  private TextFieldFloat ystart;
  private TextFieldFloat yend;

  private int xborder = 100;
  private int yborder = 100;

  private int WID = 700;
  private int HGT = 700;

  private int width  = WID;
  private int height = HGT;
 
  private JComboBox x_formatList = null;
  private JComboBox y_formatList = null;

  private TextFieldInt xticks = null;
  private TextFieldInt yticks = null;

  private JTextField maintitle_field = null;
  private JTextField xtitle_field = null;
  private JTextField ytitle_field = null;

  //
  private static int LINE = 1;
  private static int TEXT = 2;
  private static int RECTANGLE = 3;
  private static int FILLED_RECTANGLE = 4;
  private static int TEXTLINE = 5;
  private static int AXIS = 6;

  // draw rectangle around graph
  private boolean draw_axes  = true;
  private boolean rectangle = false;

  private boolean screen_min_max = false;
  private Image offscreen = null;

  private String maintitle = "";
  private String subtitle = "";
  private String xtitle = "";
  private String ytitle = "";
  private String fileName = null;

  private Color plplot_colour[] = { 
                   Color.black, Color.red, 
                   Color.yellow, Color.green, 
                   SequenceProperties.AQUAMARINE, Color.pink, 
                   SequenceProperties.WHEAT, Color.gray, 
                   SequenceProperties.BROWN, Color.blue, 
                   SequenceProperties.BLUEVIOLET, Color.cyan, 
                   SequenceProperties.TURQUOISE, SequenceProperties.MAGENTA, 
                   SequenceProperties.SALMON, Color.white };
  /**
  *
  * Contructor for graph object.
  *
  */
  public Graph2DPlot()
  {
    setPreferredSize(new Dimension(width,height));
    setToolTipText("");
  }


  /**
  *
  * Set the data to plot.
  *
  */
  public void setData(Object[][] emboss_data)
  {
    this.emboss_data = emboss_data;
    calcMinMax();
  }

  /**
  *
  * Pass in EMBOSS data graphics file
  *
  */
  public void setFileData(String s, String fileName)
  {
    this.fileName = fileName;
    try
    {
      StringReader reader = new StringReader(s);
      emboss_data = readGraph(reader);
    }
    catch(FileNotFoundException fnne){}
    catch(IOException ioe){}

    if(fileName.indexOf("plotorf") > -1)
    {
      WID = 1000;
      HGT = 400;
      width  = WID;
      height = HGT;
    }

//  if(xmin == 0 && xmax == 0 && ymin == 0 && ymax == 0)
    calcMinMax();
  }

  /**
  *
  * Pass in EMBOSS data graphics file
  *
  */
  public void setFileData(File filename)
  {
    this.fileName = filename.getName();
    try
    {
      FileReader reader = new FileReader(filename);
      emboss_data = readGraph(reader);
    }
    catch(FileNotFoundException fnne){}
    catch(IOException ioe){}  

    if(fileName.indexOf("plotorf") > -1)
    {
      WID = 1000;
      HGT = 400;
      width  = WID;
      height = HGT;
    }

//  if(xmin == 0 && xmax == 0 && ymin == 0 && ymax == 0)
    calcMinMax();
  }

  /**
  *
  * Create a JMenuBar for this graph
  *
  */
  public JMenuBar getMenuBar(boolean bexit, final JFrame frame)
  {
    JMenuBar menubar = new JMenuBar();
    menubar.add(Box.createRigidArea(new Dimension(5,24)));
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

    JMenuItem showMenu = new JMenuItem("Display data...");
    showMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JFrame frame = new JFrame("EMBOSS data file");
        FileEditorDisplay fed = new FileEditorDisplay("graph_data.dat",
                                                  graph_data.toString());
        JScrollPane jsp = new JScrollPane(fed);
        fed.setCaretPosition(0);
        frame.getContentPane().add(jsp);
        frame.pack();
        frame.setSize(640,580);
        frame.setVisible(true);
      }
    });
    fileMenu.add(showMenu);


    JMenuItem printMenu = new JMenuItem("Print...");
    printMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        PrintPlot pp = new PrintPlot(Graph2DPlot.this);
        pp.print();
      }
    });
    fileMenu.add(printMenu);
    fileMenu.add(new JSeparator());

    if(bexit)
    {
      JMenuItem fileMenuExit = new JMenuItem("Exit");
      fileMenuExit.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          System.exit(0);
        }
      });
      fileMenu.add(fileMenuExit);
    }
    else
    {
      JMenuItem fileMenuExit = new JMenuItem("Close");
      fileMenuExit.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          frame.setVisible(false);
          frame.dispose();
        }
      });
      fileMenu.add(fileMenuExit);
    }

    menubar.add(fileMenu);

    JMenu optionsMenu = new JMenu("Options");
    JMenuItem axesOptions = new JMenuItem("Axes, Labels...");
    axesOptions.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        String formats[] = { "default", "##0.0", "##0.00",
                             "#0.##E0", "##0.##E0", "###0.##E0" };
        Box xbdown = Box.createVerticalBox();
        Box ybdown = Box.createVerticalBox();

        Dimension dim = new Dimension(130,20);

        Box bacross = Box.createHorizontalBox();    
        x_formatList = new JComboBox(formats);
        x_formatList.setPreferredSize(dim);
        x_formatList.setEditable(true);
        bacross.add(x_formatList);
        bacross.add(new JLabel(" X-axis Number Format  "));
        xbdown.add(bacross);
        xbdown.add(Box.createVerticalStrut(5));

        bacross = Box.createHorizontalBox();
        y_formatList = new JComboBox(formats);
        y_formatList.setPreferredSize(dim);
        y_formatList.setEditable(true);
        bacross.add(y_formatList);
        bacross.add(new JLabel(" Y-axis Number Format"));
        ybdown.add(bacross);
        ybdown.add(Box.createVerticalStrut(5));

// x-ticks
        bacross = Box.createHorizontalBox();
        if(xticks == null)
        {
          xticks = new TextFieldInt();
          xticks.setValue(10);
        }
        xticks.setPreferredSize(dim);
        xticks.setMaximumSize(dim);

        bacross.add(xticks);
        bacross.add(new JLabel(" Number of X ticks  "));
        bacross.add(Box.createHorizontalGlue());
        xbdown.add(bacross);
        xbdown.add(Box.createVerticalStrut(5));

// y-ticks
        bacross = Box.createHorizontalBox();
        if(yticks == null)
        {
          yticks = new TextFieldInt();
          yticks.setValue(10);
        }
        yticks.setPreferredSize(dim);
        yticks.setMaximumSize(dim);

        bacross.add(yticks);
        bacross.add(new JLabel(" Number of Y ticks"));
        bacross.add(Box.createHorizontalGlue());
        ybdown.add(bacross);
        ybdown.add(Box.createVerticalStrut(5));

// x-ticks
        bacross = Box.createHorizontalBox();
        if(xstart == null)
        {
          xstart = new TextFieldFloat();
          xstart.setValue(xmin);
        }
        xstart.setPreferredSize(dim);
        xstart.setMaximumSize(dim);

        bacross.add(xstart);
        bacross.add(new JLabel(" Start X Tick  "));
        bacross.add(Box.createHorizontalGlue());
        xbdown.add(bacross);
        xbdown.add(Box.createVerticalStrut(5));

// y-ticks
        bacross = Box.createHorizontalBox();
        if(ystart == null)
        {
          ystart = new TextFieldFloat();
          ystart.setValue(ymin);
        }
        ystart.setPreferredSize(dim);
        ystart.setMaximumSize(dim);

        bacross.add(ystart);
        bacross.add(new JLabel(" Start Y Tick"));
        bacross.add(Box.createHorizontalGlue());
        ybdown.add(bacross);
        ybdown.add(Box.createVerticalStrut(5));
      
// x-ticks
        bacross = Box.createHorizontalBox();
        if(xend == null)
        {
          xend = new TextFieldFloat();
          xend.setValue(xmax);
        }
        xend.setPreferredSize(dim);
        xend.setMaximumSize(dim);

        bacross.add(xend);
        bacross.add(new JLabel(" End X Tick  "));
        bacross.add(Box.createHorizontalGlue());
        xbdown.add(bacross);
        xbdown.add(Box.createVerticalStrut(5));
        xbdown.add(new JSeparator());
        xbdown.add(Box.createVerticalStrut(5));

// y-ticks
        bacross = Box.createHorizontalBox();
        if(yend == null)
        {
          yend = new TextFieldFloat();
          yend.setValue(ymax);
        }
        yend.setPreferredSize(dim);
        yend.setMaximumSize(dim);

        bacross.add(yend);
        bacross.add(new JLabel(" End Y Tick"));
        bacross.add(Box.createHorizontalGlue());
        ybdown.add(bacross);
        ybdown.add(Box.createVerticalStrut(5));
        ybdown.add(new JSeparator());
        ybdown.add(Box.createVerticalStrut(5));

// x-label
        bacross = Box.createHorizontalBox();
        if(xtitle_field == null)
          xtitle_field = new JTextField(xtitle);

        xtitle_field.setPreferredSize(dim);
        xtitle_field.setMaximumSize(dim);

        bacross.add(xtitle_field);
        bacross.add(new JLabel(" X Axis Label  "));
        bacross.add(Box.createHorizontalGlue());
        xbdown.add(bacross);
        xbdown.add(Box.createVerticalStrut(5));
        xbdown.add(new JSeparator());
        xbdown.add(Box.createVerticalStrut(5));

// y-label
        bacross = Box.createHorizontalBox();
        if(ytitle_field == null)
          ytitle_field = new JTextField(ytitle);

        ytitle_field.setPreferredSize(dim);
        ytitle_field.setMaximumSize(dim);

        bacross.add(ytitle_field);
        bacross.add(new JLabel(" Y Axis Label"));
        bacross.add(Box.createHorizontalGlue());
        ybdown.add(bacross);
        ybdown.add(Box.createVerticalStrut(5));
        ybdown.add(new JSeparator());
        ybdown.add(Box.createVerticalStrut(5));

// width
        bacross = Box.createHorizontalBox();
        TextFieldInt xwidth = new TextFieldInt();
        xwidth.setValue(WID);
        xwidth.setPreferredSize(dim);
        xwidth.setMaximumSize(dim);

        bacross.add(xwidth);
        bacross.add(new JLabel(" Graph Width "));
        bacross.add(Box.createHorizontalGlue());
        xbdown.add(bacross);
        xbdown.add(Box.createVerticalStrut(5));
        
// height
        bacross = Box.createHorizontalBox();
        TextFieldInt yheight = new TextFieldInt();
        yheight.setValue(HGT);
        yheight.setPreferredSize(dim);
        yheight.setMaximumSize(dim);

        bacross.add(yheight);
        bacross.add(new JLabel(" Graph Height "));
        bacross.add(Box.createHorizontalGlue());
        ybdown.add(bacross);
        ybdown.add(Box.createVerticalStrut(5));
       
// graph colour
        Box graphBox = Box.createVerticalBox();
        bacross = Box.createHorizontalBox();
        final JButton button_colour = new JButton();
        button_colour.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            Color newColour= JColorChooser.showDialog(null, "Graph Colour",
                                                      graph_colour);
            if(newColour != null)
            {
              graph_colour = newColour;
              button_colour.setBackground(graph_colour);
            }
          }
        });

        final Dimension buttonSize = new Dimension(22,24);
        button_colour.setBackground(graph_colour);
        button_colour.setPreferredSize(buttonSize);
        button_colour.setMaximumSize(buttonSize);
        bacross.add(button_colour);
        bacross.add(new JLabel(" Graph Colour "));
        bacross.add(Box.createHorizontalGlue());
        graphBox.add(bacross);
        graphBox.add(Box.createVerticalStrut(5));

// graph line size
        bacross = Box.createHorizontalBox();
        if(graph_line == null)
        {
          graph_line = new TextFieldFloat();
          graph_line.setValue(1.f);
        }
        graph_line.setPreferredSize(dim);
        graph_line.setMaximumSize(dim);

        bacross.add(graph_line);
        bacross.add(new JLabel(" Graph Line Width "));
        bacross.add(Box.createHorizontalGlue());
        graphBox.add(bacross);
        
// main title label
        Box bdown = Box.createVerticalBox();
        bacross = Box.createHorizontalBox();
        if(maintitle_field == null)
          maintitle_field = new JTextField(maintitle);

        dim = new Dimension(260,20);
        maintitle_field.setPreferredSize(dim);
        maintitle_field.setMaximumSize(dim);

        bacross.add(maintitle_field);
        bacross.add(new JLabel(" Main Title"));
        bacross.add(Box.createHorizontalGlue());
        bdown.add(bacross);
        bdown.add(Box.createVerticalStrut(5));
        bdown.add(new JSeparator());
        bdown.add(Box.createVerticalStrut(5));
        
        JPanel pane = new JPanel(new BorderLayout());
        pane.add(bdown, BorderLayout.NORTH);
        pane.add(xbdown, BorderLayout.CENTER);
        pane.add(ybdown, BorderLayout.EAST);
        pane.add(graphBox, BorderLayout.SOUTH);

        Object[] options = { "OK", "APPLY", "CANCEL"};
        int select = 1;
        while(select == 1)
        {
          select = JOptionPane.showOptionDialog(null, pane,
                                "Graph Options",
                                 JOptionPane.YES_NO_CANCEL_OPTION,
                                 JOptionPane.QUESTION_MESSAGE,
                                 null, options, options[0]);
          if(select < 2)
          {
            if(WID != (int)xwidth.getValue() || 
               HGT != (int)yheight.getValue())
            {
              WID = (int)xwidth.getValue();
              HGT = (int)yheight.getValue();

              width  = WID;
              height = HGT;
              setPreferredSize(new Dimension(WID,HGT));
              setSize(new Dimension(WID,HGT));
              removeAll();
              revalidate();
            }

            offscreen = null;
            repaint();
          }
        }
      }
    });
    optionsMenu.add(axesOptions);
    menubar.add(optionsMenu);
    return menubar;
  }
  
  
  public JToolBar getToolBar(){
    JToolBar toolbar = new JToolBar();
// font menu
    String sizes[] = {"10", "12", "14", "16", "18"};
    final JComboBox fntSize = new JComboBox(sizes);
    fntSize.setMaximumSize(fntSize.getMinimumSize());
    fntSize.setToolTipText("Font size");
    fntSize.setSelectedItem(Integer.toString(getFont().getSize()));
    toolbar.add(fntSize);
    fntSize.setEditable(true);
    fntSize.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setCursor(cbusy);
        String fsize = (String)fntSize.getSelectedItem();
        Font fnt = getFont();
        fnt = fnt.deriveFont(Float.parseFloat(fsize));
        setFont(fnt);
        offscreen = null;
        repaint();
        setCursor(cdone);
      }
    });

    toolbar.addSeparator();

// zoom
    String zoom[] = {"70", "80", "90", "100", "150", "200"};
    final JComboBox zoomSize = new JComboBox(zoom);
    zoomSize.setMaximumSize(zoomSize.getMinimumSize());
    zoomSize.setToolTipText("Zoom (%)");
    zoomSize.setSelectedItem("100");
    toolbar.add(zoomSize);
    zoomSize.setEditable(true);
    zoomSize.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setCursor(cbusy);

        float scale_factor = 
            (Float.parseFloat((String)zoomSize.getSelectedItem()))/
            (100.f);
        width  = (int)(WID*scale_factor);
        height = (int)(HGT*scale_factor);

        setPreferredSize(new Dimension(width,height));
        setSize(new Dimension(width,height));

        removeAll();
        revalidate();    
        offscreen = null;
        repaint();
        setCursor(cdone);
      }
    });
    toolbar.add(new JLabel("%"));
    return toolbar;
  }
  

  /**
  *
  * Calculate minima and maxima
  *
  */
  private void calcMinMax()
  {
    int xnum = emboss_data[0].length;

    if(xmin == 0 && xmax == 0 && ymin == 0 && ymax == 0)
    {
      xmin = 1000000;
      xmax = -1000000;
      ymin = xmin;
      ymax = xmax;
    }

    float x;
    float y;

    int ncoords = emboss_data.length;

    for(int i=0; i<xnum; i++)
    {
      if(ncoords == 2)
      {
        x = ((Float)emboss_data[0][i]).floatValue();
        y = ((Float)emboss_data[1][i]).floatValue();
      }
      else
      {
        if( ((Integer)emboss_data[0][i]).intValue() == TEXT ||
            ((Integer)emboss_data[0][i]).intValue() == TEXTLINE ||
            isTick(((Float)emboss_data[1][i]).floatValue(), 
                   ((Float)emboss_data[2][i]).floatValue(),
                   ((Float)emboss_data[3][i]).floatValue(), 
                   ((Float)emboss_data[4][i]).floatValue(),
                   ((Float)emboss_data[5][i]).floatValue(),false))
          continue;

        x = ((Float)emboss_data[1][i]).floatValue();
        y = ((Float)emboss_data[2][i]).floatValue();

        if(xmin > x)
          xmin = x;
        if(xmax < x)
          xmax = x;
        if(ymin > y)
          ymin = y;
        if(ymax < y)
          ymax = y;

        x = ((Float)emboss_data[3][i]).floatValue();
        y = ((Float)emboss_data[4][i]).floatValue();
      }

      if(xmin > x)
        xmin = x;
      if(xmax < x)
        xmax = x;
      if(ymin > y)
        ymin = y;
      if(ymax < y)
        ymax = y;
    }
 
    if(screen_min_max)
    {

      if(xmin > xmax)
      {
        xmin = xmin_screen;
        xmax = xmax_screen;
      }

      if(ymin > ymax)
      {
        ymin = ymin_screen;
        ymax = ymax_screen;
      }
      
      if(ncoords > 2)
      {
        draw_axes = false;
      
        for(int i=0; i<xnum; i++)
        {
          if( ((Integer)emboss_data[0][i]).intValue() == LINE ||
              ((Integer)emboss_data[0][i]).intValue() == RECTANGLE )
          {
            float x1 = ((Float)emboss_data[1][i]).floatValue();
            float y1 = ((Float)emboss_data[2][i]).floatValue();
            float x2 = ((Float)emboss_data[3][i]).floatValue();
            float y2 = ((Float)emboss_data[4][i]).floatValue();
      
            if( Math.abs(x2-x1) > (xmax-xmin)*0.8 && 
                (Math.abs(y2-y1) < 0.1 || ((Integer)emboss_data[0][i]).intValue() == RECTANGLE) )
            {
              if(x2 > x1)
              {
                xmax = x2;
                xmin = x1;
              }
              else
              {
                xmax = x1;
                xmin = x2;
              }

              draw_axes = true;
              if(((Integer)emboss_data[0][i]).intValue() == RECTANGLE)
                rectangle = true;

              emboss_data[0][i] = new Integer(AXIS);
            }      
            else if( Math.abs(y2-y1) > (ymax-ymin)*0.8 && Math.abs(x2-x1) < 0.1)  // looks like axis line
            {
              if(y2 > y1)
              {
                ymax = y2;
                ymin = y1;
              }
              else
              {
                ymax = y1;
                ymin = y2;
              }

              draw_axes = true;
              if(((Integer)emboss_data[0][i]).intValue() == RECTANGLE)
                rectangle = true;

              emboss_data[0][i] = new Integer(AXIS);
            }
            
          }       
        }
      }

      if(!draw_axes && ymin > ymin_screen)
        ymin = ymin_screen;
    }

//  System.out.println("xmin "+xmin);
//  System.out.println("xmax "+xmax);
//  System.out.println("ymin "+ymin);
//  System.out.println("ymax "+ymax);
  }

  public int getWidth()
  {
    return width;
  }

  public int getHeight()
  {
    return height;
  }

  /**
  *
  * Override paintComponent to draw graph.
  *
  */
  public void paintComponent(Graphics g)
  {
    super.paintComponent(g);

    FontMetrics fm = getFontMetrics(getFont());
    int font_height = fm.getHeight();
    int tick_height = 10;
    int ytick_label_width = getYLabelWidth(fm);

    xborder = ytick_label_width+font_height+tick_height+5;
    yborder = font_height+font_height+tick_height+5;

    if(offscreen == null)
    {
      try
      {
      offscreen = createImage(getWidth(),getHeight());
      Graphics og = offscreen.getGraphics();
     
      og.setFont(getFont()); 
      og.setColor(Color.white);
      og.fillRect(0,0,getWidth(),getHeight());

      if(emboss_data.length == 2)   // 2d plot
        drawPoints(og);
      else
        drawGraphics(og,fm);   

      og.setColor(Color.black);
      
      drawTitles(og);
      
      if(draw_axes)
        drawAxes(og,tick_height);
      } catch (OutOfMemoryError e) {
          String msg ="Memory error: "+e+
          "\nPlease check Jemboss JVM startup options";
          JOptionPane.showMessageDialog(null, msg, "Error", JOptionPane.ERROR_MESSAGE);
          throw e;
      }
    }
    g.drawImage(offscreen, 0, 0, null);
  }

  /**
  *
  * Use to print the graph.
  *
  */
  public void printComponent(Graphics g)
  {
    super.paintComponent(g);
 
    FontMetrics fm = getFontMetrics(getFont());
    int font_height = fm.getHeight();
    int tick_height = 10;
    int ytick_label_width = getYLabelWidth(fm);
    
    xborder = ytick_label_width+font_height+tick_height+5;
    yborder = font_height+font_height+tick_height+5;
   
    g.setFont(getFont()); 
    g.setColor(Color.white);
    g.fillRect(0,0,getWidth(),getHeight());

    if(emboss_data.length == 2)   // 2d plot
      drawPoints(g);
    else
      drawGraphics(g,fm);

    g.setColor(Color.black);
    if(draw_axes)
      drawAxes(g,tick_height);
  }

  
  /**
  * 
  * Draws main-title and sub-title
  * 
  */
  private void drawTitles(Graphics g){
      Graphics2D g2d = (Graphics2D)g;
      FontMetrics fm = getFontMetrics(getFont());
      int font_height = fm.getHeight();
      
      if(maintitle_field != null)
          maintitle = maintitle_field.getText();

        int title_width = fm.stringWidth(maintitle);
        g2d.drawString(maintitle, (int)((getWidth()-title_width)/2),
                       font_height+1);
        
        title_width = fm.stringWidth(subtitle);
        g2d.drawString(subtitle,(int)((getWidth()-title_width)/2),
                font_height*2+2);      
  }
  
  
  /**
  *
  * Draw the x and y axes.
  *
  */
  private void drawAxes(Graphics g, int tick_height)
  {
    Graphics2D g2d = (Graphics2D)g;
    FontMetrics fm = getFontMetrics(getFont());
    int font_height = fm.getHeight();
    int font_height2 = font_height/2;

    // x-axis
    g2d.drawLine(xborder,getHeight()-yborder,
                 getWidth()-xborder,getHeight()-yborder);

    if(xtitle_field != null) 
      xtitle = xtitle_field.getText();

    int xtitle_width = fm.stringWidth(xtitle);
    g2d.drawString(xtitle, (int)((getWidth()-xtitle_width)/2), 
                   getHeight()-3);

    // y-axis
    g2d.drawLine(xborder,yborder,xborder,getHeight()-yborder);

    // compleate rectangle
    if(rectangle)
    {
      g2d.drawLine(xborder,yborder,
                   getWidth()-xborder,yborder);

      g2d.drawLine(getWidth()-xborder,yborder,
                   getWidth()-xborder,getHeight()-yborder);
    }
    
    // draw x ticks
    int n_xticks = 10;
    if(xticks != null)
      n_xticks = xticks.getValue();

    g2d.translate(xborder, getHeight()-yborder);

    if(xstart == null)
    {
      xstart = new TextFieldFloat();
      xstart.setValue(xmin);
    }
    if(xend == null)
    {
      xend = new TextFieldFloat();
      xend.setValue(xmax);
    }

    float xfactor = (getWidth()-(2*xborder))/(float)(xend.getValue()-xstart.getValue());
    float xinterval  = (float)((xend.getValue()-xstart.getValue())/n_xticks);
    float xinterval2 = xinterval/2.f;
     
    DecimalFormat myformat = null;

    if(x_formatList == null ||
       ((String)x_formatList.getSelectedItem()).equals("default"))
      myformat = getFormat(xmax);
    else
      myformat = new DecimalFormat((String)x_formatList.getSelectedItem());

    for(int i = 0; i<=n_xticks; i++)
    {
      float xpos = (float)((i*xinterval)+xstart.getValue());
      int xpos_major = (int)((i*xinterval)*xfactor);
      g2d.drawLine(xpos_major,0,xpos_major,tick_height);

      if(i > 0)
      {
        int xpos_minor = (int)(xpos_major-(xinterval2*xfactor));
        g2d.drawLine(xpos_minor,0,xpos_minor,tick_height/2);
      }

      g2d.drawString( myformat.format(xpos),
                      xpos_major-font_height2, font_height+tick_height );
    }


    // draw y ticks
    int n_yticks = 10;
    if(yticks != null)
      n_yticks = yticks.getValue();

    if(ystart == null)
    {
      ystart = new TextFieldFloat();
      ystart.setValue(ymin);
    }

    if(yend == null)
    {
      yend = new TextFieldFloat();
      yend.setValue(ymax);
    }

    float yfactor = (float)((getHeight()-(2*yborder))/(yend.getValue()-ystart.getValue()));
    float yinterval  = (float)(yend.getValue()-ystart.getValue())/n_yticks;
    float yinterval2 = yinterval/2.f;

    String numS = null;

    if(y_formatList == null || 
       ((String)y_formatList.getSelectedItem()).equals("default"))
      myformat = getFormat(ymax);
    else
      myformat = new DecimalFormat((String)y_formatList.getSelectedItem());

    int ymax_tick_width = 0;
    float y1 = (float)ystart.getValue();

    for(int i = 0; i <= n_yticks; i++) 
    {
      float ypos = (float)(i*yinterval);
      int ypos_major = -(int)(ypos*yfactor);

      g2d.drawLine(0,ypos_major,-tick_height,ypos_major);

      if(i > 0)
      {
        int ypos_minor = (int)(ypos_major+(yinterval2*yfactor));
        g2d.drawLine(0,ypos_minor,-tick_height/2,ypos_minor);
      }

      numS = myformat.format(y1 + (i*yinterval));
      int string_width = fm.stringWidth(numS);
      g2d.drawString(numS, -string_width-tick_height, ypos_major);
      if(string_width > ymax_tick_width)
        ymax_tick_width = string_width;
    }
    g2d.translate(-xborder, -getHeight()+yborder);

    // y axis label
    if(ytitle_field != null)
      ytitle = ytitle_field.getText();

    int ytitle_width = fm.stringWidth(ytitle);
    AffineTransform origin = g2d.getTransform();
    AffineTransform newOrig = (AffineTransform)(origin.clone());
    newOrig.rotate(Math.toRadians(-90.),0,0);
    g2d.setTransform(newOrig);

    g2d.drawString(ytitle, (int)((-getHeight()+ytitle_width)/2),
                   xborder-ymax_tick_width-tick_height-3);

    g2d.setTransform(origin);

  }

  /**
  *
  *  Find the width of the Y axis numbers.
  *
  */
  private int getYLabelWidth(FontMetrics fm)
  {
    DecimalFormat myformat = null;
 
    if(y_formatList == null ||
       ((String)y_formatList.getSelectedItem()).equals("default"))
      myformat = getFormat(ymax);
    else
      myformat = new DecimalFormat((String)y_formatList.getSelectedItem());

    int width = fm.stringWidth(myformat.format(ymax));

    if(width < fm.stringWidth(myformat.format(ymin)))
      width = fm.stringWidth(myformat.format(ymax));
    
    if(width < fm.stringWidth(myformat.format(xmax)))
      width = fm.stringWidth(myformat.format(xmax));

    if(width < fm.stringWidth(myformat.format(xmin)))
      width = fm.stringWidth(myformat.format(xmin));

    return width;
  }

  /**
  *
  * Determine the tool tip to display
  * @param e    mouse event
  * @return     tool tip
  *
  */
  public String getToolTipText(MouseEvent e)
  {
    Point loc = e.getPoint();

    float xpos = ((loc.x-xborder) * (xmax-xmin) / (getWidth()-(2*xborder))) 
                 + (float)xstart.getValue();
    float ypos = ((getHeight()-yborder-loc.y) * (ymax-ymin) / (getHeight()-(2*yborder)))
                 + (float)ystart.getValue();

    DecimalFormat xformat = null;
    DecimalFormat yformat = null;

    if(x_formatList == null ||
       ((String)x_formatList.getSelectedItem()).equals("default"))
      xformat = getFormat(xmax);
    else
      xformat = new DecimalFormat((String)x_formatList.getSelectedItem());

    if(y_formatList == null ||
       ((String)y_formatList.getSelectedItem()).equals("default"))
      yformat = getFormat(ymax);
    else
      yformat = new DecimalFormat((String)y_formatList.getSelectedItem());
   
    return xformat.format(xpos)+","+yformat.format(ypos); 
  }


  /**
  *
  * Use to define a format for numbers. 
  *
  */
  private DecimalFormat getFormat(float max)
  {
    DecimalFormat myformat = null;

    if(max < 10)
      myformat = new DecimalFormat("##0.00");
    else if(max < 10000)
      myformat = new DecimalFormat("##0.0");
    else if(max < 100000)
      myformat = new DecimalFormat("##0.#E0");
    else if(max < 1000000)
      myformat = new DecimalFormat("#0.##E0");
    return myformat;
  }

  /**
  *
  * Draw an EMBOSS graphics set of data.
  *
  */
  private void drawGraphics(Graphics g, FontMetrics fm)
  {
    Graphics2D g2d = (Graphics2D)g;
    BasicStroke stroke = (BasicStroke)g2d.getStroke();

    if(graph_line == null)
    {
      graph_line = new TextFieldFloat();
      graph_line.setValue(1.f);
    }
    g2d.setStroke(new BasicStroke((float)graph_line.getValue()));

    int xnum  = emboss_data[0].length;
    
    if(xstart == null)
    {
      xstart = new TextFieldFloat();
      xstart.setValue(xmin);
    }
    if(xend == null)
    {
      xend = new TextFieldFloat();
      xend.setValue(xmax);
    }

    if(ystart == null)
    {
      ystart = new TextFieldFloat();
      ystart.setValue(ymin);
    }

    if(yend == null)
    {
      yend = new TextFieldFloat();
      yend.setValue(ymax);
    }


    float xfactor = (getWidth()-(2*xborder))/(float)(xend.getValue()-xstart.getValue());
    float yfactor = (getHeight()-(2*yborder))/(float)(yend.getValue()-ystart.getValue());

    float x1;
    float y1;
    float x2;
    float y2;

    float xendPoint = (float)(xend.getValue()-xstart.getValue())*xfactor;
    float yendPoint = (float)(yend.getValue()-ystart.getValue())*yfactor;
 
    g2d.translate(xborder, getHeight()-yborder);
    for(int i=0; i<xnum; i++)
    {
      x1 =  ( ((Float)emboss_data[1][i]).floatValue() - (float)xstart.getValue())*xfactor;
      y1 = -( ((Float)emboss_data[2][i]).floatValue() - (float)ystart.getValue())*yfactor;
      if( ((Integer)emboss_data[0][i]).intValue() == LINE)   // line coordinates
      {
        x2 =  ( ((Float)emboss_data[3][i]).floatValue() - (float)xstart.getValue())*xfactor;
        y2 = -( ((Float)emboss_data[4][i]).floatValue() - (float)ystart.getValue())*yfactor;

        if( ( x1 >= 0 && x2 >= 0 &&
              x1 <= xendPoint && x2 <= xendPoint &&
              y1 <= 0 && y2 <= 0 &&
              y1 >= -yendPoint && y2 >= -yendPoint ) || !draw_axes )
        {
          int colourID = (int) ((Float)emboss_data[5][i]).floatValue();

          if(graph_colour != null &&
             graph_colour != Color.black)
            g.setColor(graph_colour);
          else if(colourID >= 0 || colourID < 16)
            g.setColor(plplot_colour[colourID]);

          g.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
        }
      }
      else if( ((Integer)emboss_data[0][i]).intValue() == RECTANGLE ||
               ((Integer)emboss_data[0][i]).intValue() == FILLED_RECTANGLE )
      {
        x2 =  ( ((Float)emboss_data[3][i]).floatValue() - (float)xstart.getValue())*xfactor;
        y2 = -( ((Float)emboss_data[4][i]).floatValue() - (float)ystart.getValue())*yfactor;
       
        int colourID = (int) ((Float)emboss_data[5][i]).floatValue();

        if(graph_colour != null &&
           graph_colour != Color.black)
          g.setColor(graph_colour);
        else if(colourID >= 0 || colourID < 16)
          g.setColor(plplot_colour[colourID]);

        if(x1 > x2)
        {
          float x1_tmp = x1;
          x1 = x2;
          x2 = x1_tmp;
        }
 
        if(((Integer)emboss_data[0][i]).intValue() == RECTANGLE)
          g.drawRect((int)x1,(int)y2,(int)Math.abs(x2-x1),(int)Math.abs(y1-y2));
        else
          g.fillRect((int)x1,(int)y2,(int)Math.abs(x2-x1),(int)Math.abs(y1-y2));
      }
      else if( ((Integer)emboss_data[0][i]).intValue() == TEXT)   
      {
        boolean number = true;
        try
        {
          String text = (String)emboss_data[5][i];
          Float.parseFloat(text);
        }
        catch(NumberFormatException nfe)
        {
          number = false;
        }

        if( x1 >= 0 && y1 <= 0 &&
            x1 <= xendPoint && y1 >= -yendPoint )
        {
          int colourID = (int) ((Float)emboss_data[3][i]).floatValue();
          int textWidth = justify((String)emboss_data[5][i],fm);

          g.setColor(plplot_colour[colourID]);
          g.drawString((String)emboss_data[5][i],(int)(x1-textWidth),(int)y1);
        }
        else if(y1 > 0 && !number)  // looks like x-axis title
        {
          if(xtitle_field == null)
            xtitle_field = new JTextField((String)emboss_data[5][i]);
        }
        else if(y1 < yendPoint && !number)  // looks like main title
        {
          if(maintitle_field == null)
            maintitle_field = new JTextField((String)emboss_data[5][i]);
        }
      }
      else if( ((Integer)emboss_data[0][i]).intValue() == TEXTLINE)
      {
        if( x1 >= 0 && y1 <= 0 &&
            x1 <= xendPoint && y1 >= -yendPoint && emboss_data[5][i] instanceof Float)
        {
          int colourID = (int) ((Float)emboss_data[5][i]).floatValue();
//        int textWidth = justify((String)emboss_data[6][i],fm);

          g.setColor(plplot_colour[colourID]);
          g.drawString((String)emboss_data[6][i],(int)x1,(int)y1);
        }
        else if( emboss_data[6][i] instanceof String)  // looks like y-axis title
        {
          if(ytitle_field == null)
            ytitle_field = new JTextField((String)emboss_data[6][i]);
        }
        else if(y1 < yendPoint && emboss_data[5][i] instanceof String)  // looks like main title
        {
          if(maintitle_field == null)
            maintitle_field = new JTextField((String)emboss_data[5][i]);
        }
      }
    }
    g2d.translate(-xborder, -getHeight()+yborder);
    g2d.setStroke(stroke);
  }

 
  private int justify(String s, FontMetrics fm)
  {
    int textWidth = 0;
    if(s.length() == 1)
    {
      if(fileName.indexOf("prettyplot") > -1)
        return textWidth; 
    }

    return fm.stringWidth(s)/2;
  }

  /**
  *
  * Draw XY graph points.
  *
  */
  private void drawPoints(Graphics g)
  {
    Graphics2D g2d = (Graphics2D)g;

    g2d.setColor(graph_colour);
    BasicStroke stroke = (BasicStroke)g2d.getStroke();

    if(graph_line == null)
    {
      graph_line = new TextFieldFloat();
      graph_line.setValue(1.f);
    }
    g2d.setStroke(new BasicStroke((float)graph_line.getValue()));

    int xnum  = emboss_data[0].length;

    if(xstart == null)
    {
      xstart = new TextFieldFloat();
      xstart.setValue(xmin);
    }
    if(xend == null)
    {
      xend = new TextFieldFloat();
      xend.setValue(xmax);
    }

    if(ystart == null)
    {
      ystart = new TextFieldFloat();
      ystart.setValue(ymin);
    }

    if(yend == null)
    {
      yend = new TextFieldFloat();
      yend.setValue(ymax);
    }

    float xfactor = (getWidth()-(2*xborder))/(float)(xend.getValue()-xstart.getValue());
    float yfactor = (getHeight()-(2*yborder))/(float)(yend.getValue()-ystart.getValue());

    float x1 =  ( ((Float)emboss_data[0][0]).floatValue() - (float)xstart.getValue())*xfactor;
    float y1 = -( ((Float)emboss_data[1][0]).floatValue() - (float)ystart.getValue())*yfactor;
    float x2;
    float y2;

    //float xendPoint =  (float)(xend.getValue()-xstart.getValue())*xfactor;
    //float yendPoint = -(float)(yend.getValue()-ystart.getValue())*yfactor;

    g2d.translate(xborder, getHeight()-yborder);
    for(int i=1; i<xnum; i++)
    {
      x2 =  ( ((Float)emboss_data[0][i]).floatValue() - (float)xstart.getValue())*xfactor;
      y2 = -( ((Float)emboss_data[1][i]).floatValue() - (float)ystart.getValue())*yfactor;

      if( x1 >= 0 && x2 >= 0 &&
          y1 <= 0 && y2 <= 0 )
        g.drawLine((int)x1,(int)y1,(int)x2,(int)y2);   

      x1 = x2;
      y1 = y2;
    }
    g2d.translate(-xborder, -getHeight()+yborder);

    g2d.setStroke(stroke);
  }

  /**
  *
  * Read graph data.
  *
  */
  public Object[][] readGraph(Reader read) throws IOException
  {
    BufferedReader in = new BufferedReader(read);
    String line;
    Vector vx = new Vector();
    Vector vy = new Vector();
 
    graph_data = new StringBuffer();
    int npoints = 0;
    boolean xygraph = false;

    while((line = in.readLine()) != null )
    {
      graph_data.append(line+"\n");
      if((line.startsWith("Line") || line.startsWith("Rectangle") ||
          line.startsWith("Filled Rectangle")) && !xygraph)
      {
        vx.add(line);
      }
      else if(line.startsWith("##Points "))
      {    
        int ind = line.indexOf(" ");
        npoints = Integer.parseInt(line.substring(ind).trim());
      }
      else if(line.startsWith("##Screen "))
      {
        StringTokenizer tok = new StringTokenizer(line," ");
        tok.nextToken();
        tok.nextToken();
        xmin_screen = Float.parseFloat(tok.nextToken());
        tok.nextToken();
        ymin_screen = Float.parseFloat(tok.nextToken());
        tok.nextToken();
        xmax_screen = Float.parseFloat(tok.nextToken());
        tok.nextToken();
        ymax_screen = Float.parseFloat(tok.nextToken());
        screen_min_max = true;
      }
      else if(line.startsWith("##Xmin "))
      {
        StringTokenizer tok = new StringTokenizer(line," ");
        tok.nextToken();
        xmin = Float.parseFloat(tok.nextToken());
        tok.nextToken();
        xmax = Float.parseFloat(tok.nextToken());
        tok.nextToken();
        ymin = Float.parseFloat(tok.nextToken());
        tok.nextToken();
        ymax = Float.parseFloat(tok.nextToken());
      }
      else if(line.startsWith("Shaded"))
          System.err.println("unsupported line: '"+line+"'");
      else if(!line.startsWith("#") && !line.equals("") && 
              !line.startsWith("Text") && !line.startsWith("Line"))
      {
        line = line.trim();
        int ind = line.indexOf("\t");
        if(ind == -1)
          ind = line.indexOf(" ");
        
        xygraph = true;
        vx.add(new Float(line.substring(0,ind).trim()));
        vy.add(new Float(line.substring(ind).trim()));
      }
      else if(line.startsWith("##Xtitle"))
      {
        int ind = line.indexOf(" ");
        xtitle = line.substring(ind).trim();
        if(xtitle.equals("<null>"))
          xtitle = "x";
      }
      else if(line.startsWith("##Ytitle"))
      {
        int ind = line.indexOf(" ");
        ytitle = line.substring(ind).trim();
        if(ytitle.equals("<null>"))
          ytitle = "y";
      }
      else if(line.startsWith("##Maintitle "))
      {
        int ind = line.indexOf(" ");
        maintitle = line.substring(ind).trim();
      }
      else if(line.startsWith("##Subtitle "))
      {
        int ind = line.indexOf(" ");
        subtitle = line.substring(ind).trim();
      }
      else if(line.startsWith("Text"))
        vx.add(line);
    }

    npoints = vx.size();
    Object[][] emboss_data;

    if(vy.size() > 0)
    {
      emboss_data = new Object[2][npoints];
      for(int i=0; i<npoints; i++)
      {
        emboss_data[0][i] = (Float)vx.get(i);
        emboss_data[1][i] = (Float)vy.get(i); 
      }
    }
    else
    {
      emboss_data = new Object[7][npoints];
      for(int i=0; i<npoints; i++)
        setGraphicRow(emboss_data,(String)vx.get(i),i);
    }

    return emboss_data;
  }

  /**
  *
  * Split up text, e.g.
  * Line x1 352.000000 y1 346.000000 x2 364.000000 y2 358.000000 colour 0
  * Rectangle x1 0.000000 y1 0.000000 x2 518.000000 y2 518.000000 colour 0
  *
  */
  private void setGraphicRow(Object[][] emboss_data, String line, int i)
  {
    StringTokenizer tok = new StringTokenizer(line," "); 
    String type = tok.nextToken(); 
    if(type.equals("Line"))
      emboss_data[0][i] = new Integer(LINE);
    else if(type.startsWith("Textline"))
      emboss_data[0][i] = new Integer(TEXTLINE);
    else if(type.startsWith("Text"))
      emboss_data[0][i] = new Integer(TEXT);
    else if(type.equals("Rectangle"))
      emboss_data[0][i] = new Integer(RECTANGLE);
    else if(type.equals("Filled"))
    {
      emboss_data[0][i] = new Integer(FILLED_RECTANGLE);
      tok.nextToken();
    }

    tok.nextToken();
    emboss_data[1][i] = Float.valueOf(tok.nextToken());
    tok.nextToken();
    emboss_data[2][i] = Float.valueOf(tok.nextToken());
    tok.nextToken();
    emboss_data[3][i] = Float.valueOf(tok.nextToken());
    tok.nextToken();
    emboss_data[4][i] = Float.valueOf(tok.nextToken());

    if(type.equals("Line") || type.equals("Rectangle") || 
       type.equals("Filled"))
    {
      tok.nextToken();
      emboss_data[5][i] = Float.valueOf(tok.nextToken());
    }
    else if(type.startsWith("Textline"))
    {
      tok.nextToken();
      emboss_data[5][i] = Float.valueOf(tok.nextToken());
      tok.nextToken();
      tok.nextToken();
      String text = new String();
      while(tok.hasMoreTokens())
        text = text+" "+tok.nextToken();
      emboss_data[6][i] = text.trim();
    }
    else if(type.startsWith("Text") )
    {
      String text = new String();
      while(tok.hasMoreTokens())
        text = text+" "+tok.nextToken();
      emboss_data[5][i] = text.trim();
    }
  }

  /**
  *
  * Determine if this looks like a tick line.
  *
  */
//private boolean isTick(String line, boolean checkBounds)
//{
//  StringTokenizer tok = new StringTokenizer(line," ");
//  String type = tok.nextToken();

//  if(!type.equals("Line"))
//    return false;

//  tok.nextToken();
//  float x1 = Float.parseFloat(tok.nextToken());
//  tok.nextToken();
//  float y1 = Float.parseFloat(tok.nextToken());
//  tok.nextToken();
//  float x2 = Float.parseFloat(tok.nextToken());
//  tok.nextToken();
//  float y2 = Float.parseFloat(tok.nextToken());

//  return isTick(x1,y1,x2,y2,checkBounds);
//}

  /**
  *
  * Determine if this looks like a tick line.
  *
  */
  private boolean isTick(float x1,float y1,float x2,float y2, float colourID,
                         boolean checkBounds)
  {
    if(colourID != 0)  // assume tick lines are black
      return false;

    if(x1 == x2 || y1 == y2)
    {
      if(checkBounds) // check this is out of the graph boundary
      {
        if(isOutOfXBounds(x1) || isOutOfXBounds(x2) ||
           isOutOfYBounds(y1) || isOutOfYBounds(y2) ) 
          return true;
      }
      else
        return true;

    }

    return false;
  }

  private boolean isOutOfXBounds(float x)
  {
    if( x < xmin || x > xmax)
      return true;
    return false;
  }


  private boolean isOutOfYBounds(float y)
  {
    if( y < ymin || y > ymax)
      return true;
    return false;
  }


  public static void main(String arg[])
  {
//  float[][] emboss_data = { {0,1,2}, {0,1,3} };
    JFrame frame = new JFrame("Jemboss Graphics");
    Graph2DPlot gp = new Graph2DPlot();
    JScrollPane scroll = new JScrollPane(gp);
    scroll.setPreferredSize(new Dimension(400,400));
    frame.getContentPane().add(scroll);
//  gp.setData(emboss_data);
    File file = new File(arg[0]);
    gp.setFileData(file);
    frame.setJMenuBar(gp.getMenuBar(false, frame));
    frame.pack();
    frame.setVisible(true);
    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
  }

}

