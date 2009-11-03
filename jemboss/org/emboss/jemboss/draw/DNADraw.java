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


package org.emboss.jemboss.draw;

import javax.swing.*;
import java.awt.*;
import java.awt.print.*;
import java.awt.event.*;
import java.awt.geom.AffineTransform;
import java.util.*;
import java.awt.datatransfer.*;
import java.awt.dnd.*;
import java.net.URL;

import org.emboss.jemboss.gui.ScrollPanel;
import org.emboss.jemboss.gui.Browser;

public class DNADraw extends ScrollPanel
                     implements Printable, DragGestureListener,
                     DragSourceListener, DropTargetListener
{

  public static JScrollPane jsp;
  private DNADraw current_dna;
  private JFrame mainFrame;

  private Point location    = new Point(75,75);
  private Dimension border  = new Dimension(150,150);
  private Dimension panelSize = new Dimension(600,600);
  private Dimension linearPanelSize = new Dimension(800,350);
  private Hashtable lineAttr;
  private Vector minorTicks;
  private Vector majorTicks;
  private Vector block;
  private Vector restrictionEnzyme;

  private int startTick    = 0;
  private int minorTick    = 100;
  private int majorTick    = 500;

//
// store the tick positions -- there appears to be
// a bug in AffineTransform when it comes to using
// elements from the matrix when printing
//
  private int[] tickMajorXPositions;
  private int[] tickMajorYPositions;

  private int[] tickMinorXPositions;
  private int[] tickMinorYPositions;

  private int[] reXPositions;
  private int[] reYPositions;
  private boolean close = false;

  public DNADraw()
  {
    super(new BorderLayout()); 
    current_dna = this;
    setBackground(Color.white);
    setPreferredSize(panelSize);
    setOpaque(false);

    DragSource dragSource = DragSource.getDefaultDragSource();
    dragSource.createDefaultDragGestureRecognizer(
       this,                             // component where drag originates
       DnDConstants.ACTION_COPY_OR_MOVE, // actions
       this);      
    setDropTarget(new DropTarget(this,this));
    lineAttr = new Hashtable();
    lineAttr.put("start",new Integer(0));
    lineAttr.put("end",new Integer(4000));
    lineAttr.put("lsize",new Integer(5));
    lineAttr.put("circular",new Boolean(true));

    MouseListener mouseListener = new MouseAdapter()
    {
      public void mouseClicked(MouseEvent me)
      {
        if(me.getClickCount() == 2 &&
           !me.isPopupTrigger())
        {
          for(int i=0; i<getComponentCount(); i++)
          {
            if(getComponent(i) instanceof Block)
            {
              final Block drawBlock = (Block)getComponent(i);
              if(drawBlock.isOverMe(me.getX(),me.getY()))
              {
                final JFrame f = new JFrame("Properties");
                JButton butt = new JButton("Delete");
                butt.addActionListener(new ActionListener()
                {
                  public void actionPerformed(ActionEvent e)
                  {
                    remove(drawBlock);
                    current_dna.repaint(); 
                    f.setVisible(false);
                    f.dispose();       
                    Enumeration enumBk = block.elements();
                    int nelement = 0;
                    while(enumBk.hasMoreElements())
                    {
                      Vector v = (Vector)enumBk.nextElement();
                      if(v.contains(drawBlock))
                        block.removeElementAt(nelement);   
                      nelement++;
                    }
                  }
                }); 
                drawBlock.showProperties(f,DNADraw.this,butt);
              }
            }
          }

        }
      }
    };
    this.addMouseListener(mouseListener);
  }


  public DNADraw(Vector minorTicks, Vector majorTicks,
                 Vector block,
                 Vector restrictionEnzyme)
  {
    this();
    this.minorTicks = minorTicks;
    this.block = block;
    this.restrictionEnzyme = restrictionEnzyme;
  }


  public DNADraw(Vector block, Vector restrictionEnzyme,
                 Hashtable lineAttr, int startTick,
                 int minorTick, int majorTick)
  {
    this();
    this.block = block;
    this.restrictionEnzyme = restrictionEnzyme;
    this.lineAttr     = lineAttr;
    this.startTick    = startTick;
    this.minorTick    = minorTick;
    this.majorTick    = majorTick;

    if(!isCircular())
      setPreferredSize(linearPanelSize);
    
    calculateTickPosistions();
  }


  /**
  *
  * Get the width/diameter of the DNA map
  *
  */
  protected double getDiameter()
  {
    return getWidth()-border.getWidth();
  }


  protected Point getLocationPoint()
  {
    return location;
  }


  protected void zoomIn()
  {
    int wid = getWidth();
    wid     = wid+(int)(wid*0.1);
    int hgt = getHeight();
    if(isCircular())
      hgt = hgt+(int)(hgt*0.1);
    zoom(wid,hgt);
  }


  protected void zoomOut()
  {
    int wid = getWidth();
    wid     = wid-(int)(wid*0.1);
    int hgt = getHeight();  
    if(isCircular())
      hgt = hgt-(int)(hgt*0.1);
    zoom(wid,hgt);
  }

  
  private void zoom(int wid, int hgt)
  {
    if(isCircular())
    {
      panelSize = new Dimension(wid,hgt);
      setPreferredSize(panelSize);
      setSize(panelSize);
    }
    else
    {
      linearPanelSize = new Dimension(wid,hgt);
      setPreferredSize(linearPanelSize);
      setSize(linearPanelSize);
    }
    repaint();
  }

  
  protected void paintComponent(Graphics g)
  {
    super.paintComponent(g);
    Graphics2D g2 = (Graphics2D)g;

    if(isCircular())
      drawCircularPanel(g2,true);
    else
      drawLinearPanel(g2);
  }


  protected boolean isCircular()
  {
    return ((Boolean)lineAttr.get("circular")).booleanValue();
  }


  protected void addBlock(Block b)
  {
    add(b);
    validate();
  }


  protected void drawLinearPanel(Graphics2D g2)
  {
    FontMetrics fm = g2.getFontMetrics();
    double hgt = fm.getAscent();
    g2.setColor(Color.black);
    double widDash = 4;

    int lineSize = 5;
    try
    {
      lineSize = getLineSize();
    }
    catch(NullPointerException npe)
    {
      System.out.println("No line size specified using default!");
    }
    g2.setStroke(new BasicStroke((float)lineSize));

    double widthPanel  = getWidth();
    double ddiameter  = widthPanel-border.getWidth();
    int diameter = (int)ddiameter;
    int ymid = getHeight()/2;

    g2.setStroke(new BasicStroke((float)lineSize));
    g2.drawLine(location.x,ymid,
                diameter,ymid);   

    int start = getStart();
    int end   = getEnd();

    g2.setColor(Color.black);
    g2.setStroke(new BasicStroke(1.f));

    if(majorTicks == null || minorTicks == null)
      calculateTickPosistions();

    Enumeration enumTk = minorTicks.elements();
    while(enumTk.hasMoreElements())
    {
      int tick = ((Integer)enumTk.nextElement()).intValue();
      int x = ((diameter-location.x)*(tick-start)/(end-start))+location.x;
      int y = ymid+(int)((lineSize+widDash)/2);
      g2.drawLine(x,ymid,x,y);
    }

    enumTk = majorTicks.elements();
    while(enumTk.hasMoreElements())
    {
      int tick = ((Integer)enumTk.nextElement()).intValue();
      int x = ((diameter-location.x)*(tick-start)/(end-start))+location.x;
      int y = ymid+(lineSize/2)+(int)widDash;
      g2.drawLine(x,ymid,x,y);
      String label = Integer.toString(tick);
      x-=(fm.stringWidth(label)/2);
      y+=hgt;
      g2.drawString(label,x,y);
    }

    if(restrictionEnzyme != null)
    {
      enumTk = restrictionEnzyme.elements();
      while(enumTk.hasMoreElements())
      {
        Vector re = (Vector)enumTk.nextElement();
        String reLabel = (String)re.elementAt(0);
        int pos = ((Integer)re.elementAt(1)).intValue();
        g2.setColor((Color)re.elementAt(2));
        int x = ((diameter-location.x)*(pos-start)/(end-start))+location.x;
        int y = ymid-(lineSize/2)-(int)widDash;
        g2.drawLine(x,ymid,x,y);
        x-=(fm.stringWidth(reLabel)/2);
        y-=hgt;
        g2.drawString(reLabel,x,y);
      }
    }

  }

  protected void drawCircularPanel(Graphics2D g2, boolean record)
  {
    g2.setColor(Color.black);
 
    FontMetrics fm = g2.getFontMetrics();
    double hgt = fm.getAscent();
    double widthPanel  = getWidth(); 
    double heightPanel = getHeight();

    double rad = 360.d;
    double pi  = Math.PI;
    double widDash = 4;

    double ddiameter  = widthPanel-border.getWidth();
    double ddiameter2 = ddiameter/2.d;
    int diameter = (int)ddiameter;

    int lineSize = 5;
    try
    {
      lineSize = getLineSize();
    }
    catch(NullPointerException npe)
    {
      System.out.println("No line size specified using default!");
    }

    g2.setStroke(new BasicStroke((float)lineSize));
    g2.drawArc(location.x,location.y,
               diameter,diameter,0,360);

    AffineTransform origin = g2.getTransform();
    AffineTransform newOrig;

    if(restrictionEnzyme != null)
    {
      if(record)
      {
        int nsize = restrictionEnzyme.size();
        reXPositions = new int[nsize];
        reYPositions = new int[nsize];
      }
      Enumeration enumRes = restrictionEnzyme.elements();
      while(enumRes.hasMoreElements())
      {
        Vector re = (Vector)enumRes.nextElement();
        String reLabel = (String)re.elementAt(0);
        int pos = ((Integer)re.elementAt(1)).intValue();
        g2.setColor((Color)re.elementAt(2));
        double ang = getAngleFromPosition(pos,rad);

        newOrig = (AffineTransform)(origin.clone());
        newOrig.rotate(Math.toRadians(-ang),
                       widthPanel/2.d,heightPanel/2.d);

        int widLabel  = (lineSize+fm.stringWidth(reLabel))/2;
        int widREDash = (int)(widDash+widDash+lineSize)+widLabel;

        int x = 0;
        int y = 0;
        if(record)
        {
          x = (int)( ddiameter2 + (newOrig.getScaleX()*
                       (ddiameter2 + 10 + widLabel + widREDash) ) -
                       widLabel );
          y = (int)( ddiameter2 + (newOrig.getShearY()*
                       (ddiameter2 + 10 + widREDash + (hgt/2.d)) ) +
                       hgt/2.d );

          int index = restrictionEnzyme.indexOf(re);
          reXPositions[index] = x;
          reYPositions[index] = y;
        }
        else
        {
          int index = restrictionEnzyme.indexOf(re);
          x = reXPositions[index];
          y = reYPositions[index];
        }

        g2.drawString(reLabel,location.x+x,location.y+y);
        g2.setTransform(newOrig);
        g2.setStroke(new BasicStroke(1.f));
        int xLine = location.x+(int)(ddiameter);
        int yLine = location.y+(int)(ddiameter/2.d);
        g2.drawLine(xLine,yLine,(int)(xLine+widREDash),yLine);
        g2.setTransform(origin);
      }
    }

    if(majorTicks == null || minorTicks == null)
      calculateTickPosistions();
    //major ticks
    drawCircularTicks(g2,ddiameter,ddiameter2,diameter,origin,
                      widthPanel,heightPanel,rad,pi,widDash,fm,
                      lineSize,record,majorTicks,false);


    //minor ticks
    drawCircularTicks(g2,ddiameter,ddiameter2,diameter,origin,
                      widthPanel,heightPanel,rad,pi,widDash/2,fm,
                      lineSize,record,minorTicks,true);
  }


  private void drawCircularTicks(Graphics2D g2, double ddiameter,
            double ddiameter2, int diameter, AffineTransform origin,
            double widthPanel,double heightPanel, double rad, double pi,
            double widDash, FontMetrics fm, int lineSize,
            boolean record, Vector ticks, boolean smallTicks)
  {

    double hgt = fm.getAscent();

    g2.setColor(Color.black);
    if(record)
    {
      int nsize = ticks.size();

      if(smallTicks)
      {
        tickMinorXPositions = new int[nsize];
        tickMinorYPositions = new int[nsize];
      }
      else
      {
        tickMajorXPositions = new int[nsize];
        tickMajorYPositions = new int[nsize];
      }
    }

    AffineTransform newOrig;
    Enumeration enumTk = ticks.elements();
    while(enumTk.hasMoreElements())
    {
      int tick = ((Integer)enumTk.nextElement()).intValue();
      double theta = Math.toRadians(-getAngleFromPosition(tick,rad));
      if(theta > pi)
        theta = theta - pi*2.d;

      newOrig = (AffineTransform)(origin.clone());

      // rotate and add tick mark
      newOrig.rotate(theta,widthPanel/2.d,heightPanel/2.d);
      String label = Integer.toString(tick);
      double wid = fm.stringWidth(label);

      int x = 0;
      int y = 0;
      if(record)
      {
        x = (int)( (ddiameter2) + (newOrig.getScaleX()*
                   (widDash+lineSize+3+(diameter+wid)/2.d)) - (wid/2.d));

        y = (int)( (ddiameter2) + (newOrig.getShearY()*
                   (widDash+lineSize+3+(diameter+hgt)/2.d)) + (hgt/2.d));

        int index = ticks.indexOf(new Integer(tick));

        if(smallTicks)
        {
          tickMinorXPositions[index] = x;
          tickMinorYPositions[index] = y;
        }
        else
        {
          tickMajorXPositions[index] = x;
          tickMajorYPositions[index] = y;
        }
      }
      else    // use stored positions for printing
      {
        int index = ticks.indexOf(new Integer(tick));
        if(smallTicks)
        {
          x = tickMinorXPositions[index];
          y = tickMinorYPositions[index];
        }
        {
          x = tickMajorXPositions[index];
          y = tickMajorYPositions[index];
        }
      }

      if(!smallTicks)        // add tick label
        g2.drawString(label,
                    location.x+x,
                    location.y+y);
      
      g2.setTransform(newOrig);

      g2.setStroke(new BasicStroke(1.f));
      int xLine = location.x+(int)(ddiameter);
      int yLine = location.y+(int)(ddiameter/2.d);
      g2.drawLine(xLine,yLine,(int)(xLine+lineSize+widDash),yLine);

/*
      System.out.println("THETA "+Math.toDegrees(theta));
      System.out.println("m00 "+newOrig.getScaleX()+
                         " m01 "+newOrig.getShearX()+
                         " m02 "+newOrig.getTranslateX());
      System.out.println("m10 "+newOrig.getScaleY()+
                         " m12 "+newOrig.getTranslateY());
*/
      g2.setTransform(origin);
    }

    return;
  }


  /**
  *
  * Calculate the tick marks to be drawn
  *
  */
  protected void calculateTickPosistions()
  {
    minorTicks = new Vector();
    majorTicks = new Vector();
    int start = getStart();
    int end   = getEnd();

    if(majorTick == 0)
      return;

    for(int i=startTick; i<end; i+=majorTick)
      if(i >= start)
        majorTicks.add(new Integer(i));

    if(minorTick == 0)
      return;

    for(int i=startTick; i<end; i+=minorTick)
    {
      Integer tk = new Integer(i);
      if(i >= start && !majorTicks.contains(tk))
        minorTicks.add(tk);
    }
  }


  /**
  *
  * Return the position tick marks start at
  *
  */
  protected int getStartTick()
  { 
    return startTick;
  }

  
  /**
  *
  * Set the position tick marks start at
  *
  */
  protected boolean setStartTick(int startTick)
  {
    this.startTick = startTick;
    if((startTick >= getStart()) && (startTick < getEnd()))
      return true;
    
    return false;
  }

  
  /**
  *
  * Return the interval for the tick marks
  *
  */
  protected int getTickInterval()
  {
    return majorTick;
  }


  /**
  *
  * Set the interval for the tick marks
  *
  */
  protected boolean setTickInterval(int majorTick)
  {
    if(majorTick < (getEnd()-getStart()))
    {
      this.majorTick = majorTick;
      return true;
    }
    return false;
  }


  /**
  *
  * Return the interval for the tick marks
  *
  */
  protected int getMinorTickInterval()
  {
    return minorTick;
  }


  /**
  *
  * Set the interval for the tick marks
  *
  */
  protected boolean setMinorTickInterval(int minorTick)
  {
    if(minorTick < (getEnd()-getStart()))
    {
      this.minorTick = minorTick;
      return true;
    }
    return false;
  }


  /**
  *
  * Return an angle in degrees
  *
  */
  protected double getAngleFromPosition(int pos,double rad)
  {
    int start = getStart();
    int end   = getEnd();
    return - ((pos-start)*rad)/(end-start);
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
    drawAll(g2,false);
    return Printable.PAGE_EXISTS;
  }

  public void drawAll(Graphics2D g2, boolean l)
  {
    if(((Boolean)lineAttr.get("circular")).booleanValue())
      drawCircularPanel(g2,l);   //repaint the page for printing
    else
      drawLinearPanel(g2);
    for(int i=0; i<getComponentCount(); i++)
    {
      if(getComponent(i) instanceof Block)
      {
        System.out.println("Printing block "+i);
        ((Block)getComponent(i)).draw(g2);
      }
    }
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
    catch (Exception PrintException) {}
  }


  protected void setRestrictionEnzyme(Vector restrictionEnzyme)
  {
    this.restrictionEnzyme = restrictionEnzyme;
  }

  protected void setGeneticMarker(Vector block)
  {
    this.block = block;
  }

  protected Hashtable getLineAttributes()
  {
    return lineAttr;
  }


  protected void setLineAttributes(Hashtable lineAttr)
  {
    this.lineAttr = lineAttr;
  }


  protected void setLineSize(int lineSize)
  {
    lineAttr.put("lsize",new Integer(lineSize));
  }


  protected int getLineSize()
  {
    return ((Integer)lineAttr.get("lsize")).intValue();
  }

  public void setPlasmidLocation(int x,int y)
  {
    location.setLocation(x,y);
  } 

  
  public JMenuBar createMenuBar()
  {
    JMenuBar menuBar = new JMenuBar();

// file menu
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(fileMenu);

    JMenuItem openMenu = new JMenuItem("Open");
    openMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        EmbossCirdnaReader dnaRead = new EmbossCirdnaReader();
        block = dnaRead.getBlock();
        restrictionEnzyme = dnaRead.getRestrictionEnzyme();

        lineAttr.put("start",new Integer(dnaRead.getStart()));
        lineAttr.put("end",new Integer(dnaRead.getEnd()));
   
        current_dna = new DNADraw(block,restrictionEnzyme,
                                  lineAttr,0,100,100);
        jsp.setViewportView(current_dna);
      }
    });
    fileMenu.add(openMenu);
    fileMenu.add(new JSeparator());
 
// print
    JMenu printMenu = new JMenu("Print");
    fileMenu.add(printMenu);

    JMenuItem print = new JMenuItem("Print postscript");
    print.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        doPrintActions();
      }
    });
    printMenu.add(print);

    JMenuItem printImage = new JMenuItem("Print png/jpeg Image...");
    printImage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        PrintDNAImage pdnai = new PrintDNAImage(current_dna);
        pdnai.print();
      }
    });
    printMenu.add(printImage);

// print preview
    JMenuItem printPreview = new JMenuItem("Print Preview...");
    printPreview.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        PrintDNAImage pdnai = new PrintDNAImage(current_dna);
        pdnai.printPreview();
      }
    });
    fileMenu.add(printPreview);
    fileMenu.add(new JSeparator());


    JMenuItem fileMenuExit = new JMenuItem("Exit");
    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(!close)
          System.exit(0);
        else
        {
          mainFrame.setVisible(false);
          mainFrame.dispose();
        }
      }
    });
    fileMenu.add(fileMenuExit);

// view menu
    JMenu viewMenu = new JMenu("View");
    menuBar.add(viewMenu);
  
    JMenuItem zoomIn = new JMenuItem("Zoom In");
    zoomIn.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_I, ActionEvent.CTRL_MASK));
    zoomIn.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        zoomIn();
      }
    });
    viewMenu.add(zoomIn);

    JMenuItem zoomOut = new JMenuItem("Zoom Out");
    zoomOut.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_O, ActionEvent.CTRL_MASK));
    zoomOut.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        zoomOut();
      }
    });
    viewMenu.add(zoomOut);

// options menu
    JMenu optionMenu = new JMenu("Options");
    menuBar.add(optionMenu);


    JMenuItem wizard = new JMenuItem("DNA Wizard");
    wizard.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {   
        Wizard wiz = new Wizard(current_dna);
        current_dna = wiz.getDNADraw();
        jsp.setViewportView(current_dna);
      }
    });
    optionMenu.add(wizard);
    optionMenu.add(new JSeparator());

    JMenuItem line = new JMenuItem("DNA attributes");
    line.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JFrame f = new JFrame("DNA Attributes");
        LineAttribute la = new LineAttribute(current_dna);
        JScrollPane laScroll = new JScrollPane(la);
        JPanel laPane = (JPanel)f.getContentPane();
        laPane.add(laScroll,BorderLayout.CENTER);
        f.setJMenuBar(la.createMenuBar(f));
        f.pack();
        f.setVisible(true);
      }
    });
    optionMenu.add(line);

    
    JMenuItem tickMarks = new JMenuItem("Tick marks");
    tickMarks.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JFrame f = new JFrame("Tick Marks");
        Ticks tk = new Ticks(current_dna,true);
        JScrollPane tkScroll = new JScrollPane(tk);
        JPanel tkPane = (JPanel)f.getContentPane();
        tkPane.add(tkScroll,BorderLayout.CENTER);
        f.setJMenuBar(tk.createMenuBar(f));
        f.pack();
        f.setVisible(true);
      }
    });
    optionMenu.add(tickMarks);


    JMenuItem gmarker = new JMenuItem("Genetic Features");
    gmarker.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JFrame f = new JFrame("Genetic Features");
        GeneticMarker gm = new GeneticMarker(current_dna,
                                             block);
        JScrollPane gmScroll = new JScrollPane(gm);
        JPanel gmPane = (JPanel)f.getContentPane();
        gmPane.add(gmScroll,BorderLayout.CENTER);
        f.setJMenuBar(gm.createMenuBar(f));
        f.pack();
        f.setVisible(true);
      }
    });
    optionMenu.add(gmarker);

    JMenuItem reSites = new JMenuItem("Restriction Enzyme");
    reSites.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JFrame f = new JFrame("Restriction Enzyme");
        RestrictionEnzyme re = new RestrictionEnzyme(current_dna,
                                              restrictionEnzyme);
        JScrollPane reScroll = new JScrollPane(re);
        JPanel rePane = (JPanel)f.getContentPane();
        rePane.add(reScroll,BorderLayout.CENTER);
        f.setJMenuBar(re.createMenuBar(f));
        f.pack();
        f.setVisible(true);
      }
    });
    optionMenu.add(reSites);

// help manu
    JMenu helpMenu = new JMenu("Help");
    menuBar.add(helpMenu);
   
    JMenuItem aboutMenu = new JMenuItem("About");
    helpMenu.add(aboutMenu);
    aboutMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ClassLoader cl = this.getClass().getClassLoader();
        try
        {
          URL inURL = cl.getResource("resources/readmeDNADraw.html");
          new Browser(inURL,"resources/readmeAlign.html");
        }
        catch (Exception ex)
        {
          JOptionPane.showMessageDialog(null,
                              "Jemboss Alignment Viewer Guide not found!",
                              "Error", JOptionPane.ERROR_MESSAGE);
        }
      }
    });

    return menuBar;
  }

  
  public void setCloseAndDispose(boolean close, JFrame mainFrame)
  {
    this.mainFrame = mainFrame;
    this.close = close;
  }

  protected Vector getGeneticMarker()
  {
    return block;
  }


  protected Vector getRestrictionEnzyme()
  {
    return restrictionEnzyme;
  }


  protected int getStart()
  {
    return ((Integer)lineAttr.get("start")).intValue();
  }

 
  protected int getEnd()
  {
    return ((Integer)lineAttr.get("end")).intValue();
  }

  
  protected void setStart(int start)
  {
    lineAttr.put("start",new Integer(start));
    calculateTickPosistions();
  }


  protected void setEnd(int end)
  {
    lineAttr.put("end",new Integer(end));
    calculateTickPosistions();
  }

////////////////////
// DRAG AND DROP
////////////////////
// drag source
  public void dragGestureRecognized(DragGestureEvent e)
  {
    // ignore if mouse popup trigger
    InputEvent ie = e.getTriggerEvent();
    if(ie instanceof MouseEvent)
      if(((MouseEvent)ie).isPopupTrigger())
        return;

    Point loc = e.getDragOrigin();
    Component c = getComponentAt(loc);
 
    if(c instanceof Block)
    {
//    System.out.println("BLOCK DRAG GESTURE");
    
      for(int i=0; i<getComponentCount(); i++)
      {
        if(getComponent(i) instanceof Block)
        {
          Block drawBlock = (Block)getComponent(i);
          if(drawBlock.isOverMe(loc.x,loc.y))
             e.startDrag(DragSource.DefaultCopyDrop,  // cursor
                   (Transferable)drawBlock,           // transferable data
                   this);                             // drag source listener
        }
      }
    }
  }

  public void dragDropEnd(DragSourceDropEvent e) {}
  public void dragEnter(DragSourceDragEvent e) {}
  public void dragExit(DragSourceEvent e) {}
  public void dragOver(DragSourceDragEvent e) {}
  public void dropActionChanged(DragSourceDragEvent e) {}

// drop sink
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(Block.BLOCK))
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
  }

  public void drop(DropTargetDropEvent e)
  {
    Transferable t = e.getTransferable();
    if(t.isDataFlavorSupported(Block.BLOCK))
    {
      try
      {
        Point loc = e.getLocation();
        Block drawBlock = (Block)t.getTransferData(Block.BLOCK);
        
        for(int i=0; i<getComponentCount(); i++)
          if(getComponent(i) instanceof Block)
          {
            Block c = (Block)getComponent(i);
            if( c.getLabel().equals(drawBlock.getLabel()) &&
                c.getStart() == drawBlock.getStart() &&
                c.getEnd() == drawBlock.getEnd() )
              c.setBlockLocation(loc.x,loc.y);
          }
      }
      catch(Exception ufe){} 
    }
  }

  public void dragOver(DropTargetDragEvent e)
  {
  }

  public void dropActionChanged(DropTargetDragEvent e) {}
  public void dragExit(DropTargetEvent e){}

  public static void main(String arg[])
  {
    Wizard wiz = new Wizard(null);
    DNADraw dna = wiz.getDNADraw();

    JFrame f = new JFrame("DNA Viewer");

    Dimension d = f.getToolkit().getScreenSize();

    Vector minTicks = new Vector();
    minTicks.add(new Integer(1200));
    minTicks.add(new Integer(1600));
    minTicks.add(new Integer(2000));
    minTicks.add(new Integer(2500));
    minTicks.add(new Integer(3000));
    minTicks.add(new Integer(3500));
    minTicks.add(new Integer(4070));
    minTicks.add(new Integer(4500));
    minTicks.add(new Integer(5070));
    Vector majTicks = new Vector();

    Vector marker = new Vector();
    Vector block = new Vector();
    marker.add(new String("CDS"));
    marker.add(new Integer(1200));
    marker.add(new Integer(1600));
    marker.add(Color.red);
    marker.add(new Float(10.0f));
    marker.add(new Boolean(false));
    marker.add(new Boolean(true));
    block.add(marker);

    Vector restrictionEnzyme = new Vector(); 
    Vector re = new Vector();
    re.add(new String("EcoR1"));
    re.add(new Integer(2555));
    re.add(Color.blue);
    restrictionEnzyme.add(re);
    re = new Vector();
    re.add(new String("EcoR1"));
    re.add(new Integer(3444));
    re.add(Color.blue);
    restrictionEnzyme.add(re);

  
    if(dna == null)
      dna = new DNADraw(minTicks,majTicks,block,restrictionEnzyme);
    jsp = new JScrollPane(dna);
    jsp.getViewport().setBackground(Color.white);
    f.getContentPane().add(jsp);
    f.setJMenuBar(dna.createMenuBar());
    
    f.pack();
    f.setLocation(((int)d.getWidth()-f.getWidth())/4,
                  ((int)d.getHeight()-f.getHeight())/2);

    f.setVisible(true);
  }

}

