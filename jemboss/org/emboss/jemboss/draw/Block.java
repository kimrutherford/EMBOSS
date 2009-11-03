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

import org.emboss.jemboss.gui.form.TextFieldInt;

import javax.swing.*;
import java.awt.geom.AffineTransform;
import java.awt.*;
import java.util.Vector;
import java.awt.datatransfer.*;
import java.io.*;
import java.awt.event.*;
import javax.swing.event.*;

public class Block extends JPanel
                   implements Transferable
{

  private DNADraw current_dna;
  private Vector marker;
  private double angStart;
  private double angEnd;
  private double fracRadii = 1.d;
  private Rectangle rect = new Rectangle();
  final public static DataFlavor BLOCK =
         new DataFlavor(Block.class, "Block");
  static DataFlavor blockFlavors[] = { BLOCK };
  

  public Block(Vector marker)
  {
    super();
    setOpaque(false);
    this.marker = marker;
  }


  public Block(Vector marker, 
               DNADraw current_dna)
  {
    this(marker);
    this.current_dna = current_dna;
    setPreferredSize(current_dna.getPreferredSize());
  }


  public Vector getMarker()
  {
    return marker;
  }


  protected void paintComponent(Graphics g)
  {
    super.paintComponent(g);
    Graphics2D g2  = (Graphics2D)g;
    draw(g2);
  }


  protected void draw(Graphics2D g2)
  {
    if(current_dna.isCircular())
      drawCircular(g2);
    else
      drawLinear(g2);   
  }

  
  protected String getLabel()
  {
    return (String)marker.elementAt(0);
  }


  protected int getStart()
  {
    return ((Integer)marker.elementAt(1)).intValue();
  }

 
  protected int getEnd()
  {
    return ((Integer)marker.elementAt(2)).intValue();
  }

  protected void showProperties(final JFrame f, final DNADraw draw,
                                JButton delete)
  {
//  final JFrame f = new JFrame("Properties");
//set up menu bar
    JMenuBar menuBar = new JMenuBar();
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(fileMenu);
                                                                             
    JMenuItem closeMenu = new JMenuItem("Close");
    closeMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));
                                                                             
    closeMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        f.dispose();
      }
    });
    fileMenu.add(closeMenu);

//set up window
    f.setJMenuBar(menuBar);
    JPanel pane = (JPanel)f.getContentPane();
    String markerLabel = (String)marker.elementAt(0);
    int bstart = ((Integer)marker.elementAt(1)).intValue();
    int bend   = ((Integer)marker.elementAt(2)).intValue();
    Color colour = (Color)marker.elementAt(3);
    float strokeSize  = ((Float)marker.elementAt(4)).floatValue();
    boolean arrowHead = ((Boolean)marker.elementAt(5)).booleanValue();
    boolean arrowTail = ((Boolean)marker.elementAt(6)).booleanValue();
         
    Box bdown = Box.createVerticalBox();
    pane.add(bdown);
     
    bdown.add(Box.createVerticalStrut(4));
    Dimension d = new Dimension(200,30); 
    Box bacross = Box.createHorizontalBox(); 
    final JTextField annotation = new JTextField();
    annotation.setPreferredSize(d);
    annotation.setMaximumSize(d);
    annotation.setText(markerLabel);
    annotation.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        marker.setElementAt(annotation.getText(),0);
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(annotation);
    bacross.add(new JLabel(" Label"));
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    d = new Dimension(65,30);
    bacross = Box.createHorizontalBox();  
    final TextFieldInt start = new TextFieldInt();
    start.setPreferredSize(d);
    start.setMaximumSize(d);
    start.setValue(bstart);
    start.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        marker.setElementAt(new Integer(start.getValue()),1); 
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(start); 
    bacross.add(new JLabel(" Start"));
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bacross = Box.createHorizontalBox();
    final TextFieldInt end = new TextFieldInt();
    end.setPreferredSize(d);
    end.setMaximumSize(d);
    end.setValue(bend);
    end.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        marker.setElementAt(new Integer(end.getValue()),2);
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(end);
    bacross.add(new JLabel(" End"));
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

//Set up the dialog that the button brings up.
    final JButton button = new JButton("");
    button.setBackground(colour);
    final JColorChooser colorChooser = new JColorChooser();
    ActionListener okListener = new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        marker.setElementAt(colorChooser.getColor(),3);
        button.setBackground(colorChooser.getColor());
        if(draw != null)
          draw.repaint();
      }
    };
    final JDialog dialog = JColorChooser.createDialog(button,
                                "Pick a Color",true,
                                colorChooser, okListener,
                                null);
 
    //Here's the code that brings up the dialog.
    button.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        dialog.show();
      }
    });
    bacross = Box.createHorizontalBox();
    bacross.add(button);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);


    bacross = Box.createHorizontalBox();
    final JSlider slider = new JSlider(1,25,(int)strokeSize);
    bacross.add(slider);
    bacross.add(new JLabel(" Line width"));
    bacross.add(Box.createHorizontalGlue());
    slider.addChangeListener(new ChangeListener()
    {
      public void stateChanged(ChangeEvent e)
      {
        marker.setElementAt(new Float((float)slider.getValue()),4);
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bacross = Box.createHorizontalBox();
    bacross.add(new JLabel("Arrow :"));
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bacross = Box.createHorizontalBox();
    final JRadioButton head = new JRadioButton("Head");
    final JRadioButton tail = new JRadioButton("Tail");
    final JRadioButton none = new JRadioButton("None");
    head.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(head.isSelected())
        {
          marker.setElementAt(new Boolean(true),5);   
          marker.setElementAt(new Boolean(false),6);
        }
        else
        {
          marker.setElementAt(new Boolean(false),5);
          marker.setElementAt(new Boolean(true),6);
        }
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(head);

    tail.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(tail.isSelected())
        {
          marker.setElementAt(new Boolean(true),6);
          marker.setElementAt(new Boolean(false),5);
        }
        else
        {
          marker.setElementAt(new Boolean(false),6);
          marker.setElementAt(new Boolean(true),5);
        }
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(tail);

    none.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(none.isSelected())
        {
          marker.setElementAt(new Boolean(false),6);
          marker.setElementAt(new Boolean(false),5);
        }
        if(draw != null)
          draw.repaint();
      }
    });
    bacross.add(none);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bacross = Box.createHorizontalBox();
    bacross.add(delete);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    ButtonGroup group = new ButtonGroup();
    group.add(head);
    group.add(tail);
    group.add(none);
    if(arrowHead)
      head.setSelected(true);
    else if(arrowTail)
      tail.setSelected(true);

    f.pack();
    f.setVisible(true);
  }

  protected void drawLinear(Graphics2D g2)
  {
    String markerLabel = (String)marker.elementAt(0);
    int bstart = ((Integer)marker.elementAt(1)).intValue();
    int bend   = ((Integer)marker.elementAt(2)).intValue();
    Color colour = (Color)marker.elementAt(3);
    float strokeSize  = ((Float)marker.elementAt(4)).floatValue();
    float strokeSize2 = strokeSize/2.f;
    boolean arrowHead = ((Boolean)marker.elementAt(5)).booleanValue();
    boolean arrowTail = ((Boolean)marker.elementAt(6)).booleanValue();

    FontMetrics fm = g2.getFontMetrics();
    //double hgt = fm.getAscent();
    g2.setColor(colour);

    double ddiameter   = current_dna.getDiameter();
    //double widthPanel  = current_dna.getWidth();
    double heightPanel = current_dna.getHeight();
    Point location = current_dna.getLocationPoint();
    int ymid  = (int)(heightPanel/2.);

    int shift = (int)fracRadii;
    if(shift > 1 && shift < (int)heightPanel)
      ymid  = shift;

    int start = current_dna.getStart();
    int end   = current_dna.getEnd();
    g2.setStroke(new BasicStroke(strokeSize));

    int xend   = (((int)ddiameter-location.x)*(bend-start)/
                   (end-start))+location.x-(int)strokeSize2;
    int xstart = (((int)ddiameter-location.x)*(bstart-start)/
                   (end-start))+location.x+(int)strokeSize2;
    if(arrowHead)
    {
      xend-=strokeSize2;
      int[] xPoints = {xend,xend,xend+(int)strokeSize};
      int[] yPoints = {ymid+(int)strokeSize,ymid-(int)strokeSize,ymid};
      g2.fillPolygon(xPoints,yPoints,3);
      g2.drawLine(xstart,ymid,xend,ymid);
   
    }
    else if(arrowTail)
    {
      xstart+=strokeSize2;
      int[] xPoints = {xstart,xstart,xstart-(int)strokeSize};
      int[] yPoints = {ymid+(int)strokeSize,ymid-(int)strokeSize,ymid};
      g2.fillPolygon(xPoints,yPoints,3);
      g2.drawLine(xstart,ymid,xend,ymid);
    }
    else
      g2.drawLine(xstart,ymid,xend,ymid);

    rect.setLocation(xstart,ymid-(int)strokeSize2);
    rect.setSize(xend-xstart,(int)strokeSize);
    int xmid = xstart+(int)(((xend-xstart)/2.)-(fm.stringWidth(markerLabel)/2.));
    g2.drawString(markerLabel,xmid,ymid-strokeSize);
    
  }


  protected void drawCircular(Graphics2D g2)
  {
    String markerLabel = (String)marker.elementAt(0);
    int bstart = ((Integer)marker.elementAt(1)).intValue();
    int bend   = ((Integer)marker.elementAt(2)).intValue();
    Color colour = (Color)marker.elementAt(3);
    float strokeSize  = ((Float)marker.elementAt(4)).floatValue();
    boolean arrowHead = ((Boolean)marker.elementAt(5)).booleanValue();
    boolean arrowTail = ((Boolean)marker.elementAt(6)).booleanValue();

    FontMetrics fm = g2.getFontMetrics();
    double hgt = fm.getAscent();
    g2.setColor(colour);
  
    double ddiameter   = current_dna.getDiameter();
    double widthPanel  = current_dna.getWidth();
    double heightPanel = current_dna.getHeight();
    double dradii      = ddiameter/2.d;
    double rad = 360.d;
    Point location = current_dna.getLocationPoint();

    AffineTransform origin  = g2.getTransform();
    AffineTransform newOrig = (AffineTransform)(origin.clone());

    //angle taken by brush stroke
    double stroke  = Math.toDegrees(Math.asin(strokeSize/(dradii)));
    double stroke2 = stroke/2.d;
    angStart = current_dna.getAngleFromPosition(bstart,rad) - stroke2;
    angEnd   = current_dna.getAngleFromPosition(bend,rad) + stroke2 - angStart;

    int shift = (int)(dradii*(1.d-fracRadii));
    double bdiameter = ddiameter*fracRadii;
 
    if(arrowHead)
    {
      angEnd += stroke2;
      newOrig.rotate(Math.toRadians(-angStart-angEnd),
                     widthPanel/2.d,heightPanel/2.d);
      angEnd += stroke2/3.d;
      int xmid = location.x+(int)(ddiameter-shift);
      int ymid = location.y+(int)(dradii);
      int[] xPoints = {xmid-(int)strokeSize,xmid+(int)strokeSize,xmid};
      int[] yPoints = {ymid,ymid,ymid+(int)strokeSize};
      g2.setTransform(newOrig);
      g2.fillPolygon(xPoints,yPoints,3);
    }
    else if(arrowTail)
    {
      angStart += stroke2;
      newOrig.rotate(Math.toRadians(-angStart),
                     widthPanel/2.d,heightPanel/2.d);
      angStart -= stroke;
      angEnd += stroke2;

      int xmid = location.x+(int)(ddiameter-shift);
      int ymid = location.y+(int)(dradii);
      int[] xPoints = {xmid-(int)strokeSize,xmid+(int)strokeSize,xmid};
      int[] yPoints = {ymid+(int)strokeSize,ymid+(int)strokeSize,ymid};
      g2.setTransform(newOrig);
      g2.fillPolygon(xPoints,yPoints,3);
    }
 
    g2.setStroke(new BasicStroke(strokeSize));
    g2.setTransform(origin);
    g2.drawArc(location.x+shift,location.y+shift,
               (int)(bdiameter),(int)(bdiameter),
               Math.round(Math.round(angStart)),
               Math.round(Math.round(angEnd)));

    newOrig = (AffineTransform)(origin.clone());
    newOrig.rotate(Math.toRadians(-angStart-(angEnd/2.d)),
                   widthPanel/2.d,heightPanel/2.d);
   
    int xblock = 0;
    int yblock = 0;

    int widMarker = fm.stringWidth(markerLabel)/2;
    xblock = (int)( dradii - (shift/2.d) + (newOrig.getScaleX()*
                   (dradii - 3 - widMarker - ((shift+strokeSize)/2.d))) -
                    widMarker );

    xblock = (int)( dradii + (newOrig.getScaleX()*
                   (dradii - shift - 3 - widMarker - (strokeSize/2.d))) -
                    widMarker );

    yblock = (int)( dradii + (newOrig.getShearY()*
                   (dradii - shift - 3 - ((strokeSize + hgt)/2.d))) +
                    hgt/2.d );
    g2.drawString(markerLabel,
                  location.x+xblock,
                  location.y+yblock);
  }


  /**
  *
  * Routine to set the position of the block
  * @param x    x position 
  * @param y    y position
  *
  */
  public void setBlockLocation(int x, int y)
  {
    if(current_dna.isCircular())
    {
      double dradii = current_dna.getDiameter()/2.d;
      Point location = current_dna.getLocationPoint();

      double x_origin = location.x+dradii;
      double y_origin = location.y+dradii;
      double len = Math.sqrt(Math.pow((y_origin-y),2)+
                             Math.pow((x_origin-x),2));   
      fracRadii = len/dradii;
    }
    else
      fracRadii = y;
    
    repaint();
  }


  /**
  *
  * Routine to check whether a given location is over
  * the drawn block.
  * @param x	x position 
  * @param y	y position
  *
  */
  public boolean isOverMe(int x, int y)
  {
    if(current_dna.isCircular())
    {
      double dradii = current_dna.getDiameter()/2.d;
      Point location = current_dna.getLocationPoint();

      double x_origin = location.x+dradii;
      double y_origin = location.y+dradii;

      double ttheta = (y_origin-y)/(x_origin-x);
      double ang = 180-Math.toDegrees(Math.atan(ttheta));
      if(x>x_origin)
        ang+=180;
      if(ang>360)
        ang-=360;

/*
    System.out.println("ANGLE "+ang);
    System.out.println("START "+(360+angStart));
    System.out.println("END   "+(360+angStart+angEnd));
*/


      // check it is within the right angle and
      // at the correct distance from the origin
      if( ang < (360+angStart) &&
          ang > (360+angStart+angEnd) )
      {
        double len = Math.sqrt(Math.pow((y_origin-y),2)+
                               Math.pow((x_origin-x),2));
        double rat = (len/dradii)/fracRadii;
        if(rat < 1.1 && rat > 0.9)
          return true;
      }
    }
    else
      return rect.contains(x,y);
    
    return false;
  }

  // Transferable
  public DataFlavor[] getTransferDataFlavors()
  {
    return blockFlavors;
  }

  public boolean isDataFlavorSupported(DataFlavor f)
  {
    if(f.equals(BLOCK))
      return true;
    return false;
  }

  public Object getTransferData(DataFlavor d)
      throws UnsupportedFlavorException, IOException
  {
    if(d.equals(BLOCK))
      return this;
    else throw new UnsupportedFlavorException(d);
  }

}

