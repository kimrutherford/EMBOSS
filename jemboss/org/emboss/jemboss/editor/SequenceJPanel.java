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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;

/**
*
* Sequence panel for drawing a sequence or the 
* sequence numbers
*
*/
public class SequenceJPanel extends JPanel
                            implements ActionListener
{

  /** sequence to display */
  protected Sequence seq;
  /** font size */
  private int fontSize = 12;
  /** font */
  private Font font = new Font("Monospaced", 
                      Font.PLAIN, fontSize);
  /** boundary width around each residue */
  private int boundWidth;
  /** half boundary width (boundWidth/2) */
  protected int boundWidth2;
  /** residue width */
  protected int resWidth;
  /** sequence height */
  protected int seqHeight; 
  /** residue number when pressed by mouse */
  private int pressedResidue;
  /** sequence numbering interval */
  protected int interval;
  /** sequence length     */
  private int seqLength;
  /** sequence height pad */
  private int ypad=0;
  /** colour scheme		       */
  private Hashtable colorTable;
  /** pad/gap character                */
  private String padChar = new String("-");
  /** pattern to search for            */
  private String pattern;
  /** draw the sequence                */
  private boolean drawSequence = false;
  /** draw a black box around residues */
  private boolean drawBlackBox = false;
  /** colour the residues              */
  private boolean drawColorBox = false;
  /** draw the sequence numbers        */
  protected boolean drawNumber   = false;
  /** colour as per prettyplot  */
  private boolean prettyPlot   = false;
  /** high light search pattern */
  private boolean highlightPattern = false;
  /** sequence alignment panel  */
  protected JComponent viewPane;
  /** pop up menu */
  private JPopupMenu popup;
  /** observer panels in this group */
  private Vector observers;

  
  /**
  *
  * @param seq		sequence to display
  * @param viewPane	sequence alignment panel
  * @param drawSequence	draw the sequence if true
  * @param drawBlackBox draw a black box around residues
  * @param drawColorBox	colour the residues
  * @param colorTable	colour scheme
  * @param fontSize	font size
  * @param ypad		sequence height pad
  *
  */
  public SequenceJPanel(Sequence seq, JComponent viewPane,
                        boolean drawSequence, boolean drawBlackBox,
                        boolean drawColorBox, Hashtable colorTable,
                        int fontSize, int ypad)
  {
    this.drawSequence = drawSequence;
    this.drawBlackBox = drawBlackBox;
    this.drawColorBox = drawColorBox;
    this.viewPane = viewPane;
    this.seq  = seq;
    this.ypad = ypad;

    setOpaque(false);
    if(colorTable != null)
      this.colorTable = colorTable;
    else
      setDefaultColorHashtable();

//  setBackground(Color.white);
    if(fontSize != 0)
    {
      this.fontSize = fontSize;
      font = new Font("Monospaced",
                      Font.PLAIN, fontSize);
    }

    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    boundWidth2 = boundWidth/2;

//tjc
//    resWidth  = metrics.stringWidth("A")+boundWidth;
    resWidth  = metrics.stringWidth("A")+boundWidth2;
    if(seq != null)
      seqLength = seq.getLength();
 
//tjc 
//  seqHeight = resWidth;
    seqHeight = metrics.stringWidth("A")+boundWidth;
    init();

    // Popup menu
    addMouseListener(new PopupListener());
    popup = new JPopupMenu();
    JLabel labName = null;

    if(seq.getID() != null)
      labName = new JLabel(" "+seq.getID());
    else 
      labName = new JLabel(" "+seq.getName());

    popup.add(labName);
    popup.add(new JSeparator());
    JMenuItem menuItem = new JMenuItem("Delete ");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    popup.add(new JSeparator());
    menuItem = new JMenuItem("Reverse Complement ");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("Reverse ");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("Complement ");
    menuItem.addActionListener(this);
    popup.add(menuItem);

    //set size
    setPreferredSize(getPreferredSize());
//  setMaximumSize(getPreferredSize());
//  setMinimumSize(getPreferredSize());
  }

  /**
  *
  * Constructor with default font size.
  * @param seq          sequence to display
  * @param viewPane     sequence alignment panel
  * @param drawSequence draw the sequence if true
  * @param drawBlackBox draw a black box around residues
  * @param drawColorBox colour the residues
  * @param colorTable   colour scheme
  * @param ypad         sequence height pad
  *
  */
  public SequenceJPanel(Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int ypad)
  {
    this(seq,viewPane,drawSequence,drawBlackBox,drawColorBox,
         colorTable,0,ypad);
  }

  /**
  *
  * Constructor for sequence numbering 
  * @param interval	numbering interval
  * @param seqLength	length of the sequence
  *
  */
  public SequenceJPanel(int interval, int seqLength)
  {
    this.drawNumber = true;
    this.interval   = interval;
    this.seqLength  = seqLength+1;
    setOpaque(false);
//  setBackground(Color.white);
    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    boundWidth2 = boundWidth/2;
    resWidth  = metrics.stringWidth("A")+boundWidth2;
    seqHeight = metrics.stringWidth("A")+boundWidth;

    setPreferredSize(getPreferredSize());
  }

  /**
  *
  * Add the mouse listener's
  *
  */
  public void init()
  {
    if(!drawNumber)
    {
      addMouseListener(new MouseAdapter()
      {
        public void mousePressed(MouseEvent e)
        {
          Point loc = e.getPoint();
          pressedResidue = (int)(loc.x/resWidth);
        }
      });

      addMouseMotionListener(new MouseMotionAdapter()
      {
        public void mouseDragged(MouseEvent e)
        {
          Point loc = e.getPoint();
          int resPos = (int)(loc.x/resWidth);
          notifyGroup(resPos,pressedResidue);
          update(resPos,pressedResidue);
        }
      });
    }

  }


  /**
  *
  *  Implement update() for observers i.e. members of the
  *  same sequence group
  *
  */
  protected void update(int resPos, int pressed)
  {
    this.pressedResidue = pressed;
    if(resPos == pressedResidue+1 && resPos > 0)
    {
      seq.insertResidue(padChar,pressedResidue);
      ((GraphicSequenceCollection)viewPane).setMaxSequenceLength(seq.getLength());
      pressedResidue = pressedResidue+1;
      repaint();
      viewPaneResize();
    }
    else if(resPos == pressedResidue-1 && resPos > -1)
    {
      if(seq.getResidueAt(resPos).equals(padChar))
      {
        seq.deleteResidue(resPos);
        pressedResidue = pressedResidue-1;
        repaint();
      }
    }
  }


  /**
  *
  *  Attach sequence display to the same group  
  *
  */
  protected void attach(SequenceJPanel sjp)
  {
    if(observers == null)
      observers = new Vector();
    observers.add(sjp);
//  System.out.println("Attach "+sjp.getName()+" to "+seq.getName());
  }

  
  /**
  *
  * Detach sequence display from this group
  *
  */
  protected void detach(SequenceJPanel sjp)
  {
    if(observers != null)
      if(observers.contains(sjp))
        observers.remove(sjp);
  }


  /**
  *
  * Detach all sequence displays from this group
  *
  */
  protected void detachAll()
  {
    observers = null;
  }
                                                                                                     
                                                                                                     
  /**
  *
  * Update all observers
  *
  */
  protected void notifyGroup(int res, int pressed)
  {
    if(observers != null)
    {
      Enumeration enumer = observers.elements();
      while(enumer.hasMoreElements())
        ((SequenceJPanel)enumer.nextElement()).update(res,pressed);
    }
  }


  /**
  *
  * Override paintComponent
  * @param g	graphics
  *
  */
  public void paintComponent(Graphics g)
  {
// let UI delegate paint first (incl. background filling)
    super.paintComponent(g);
    g.setFont(font);
    FontMetrics metrics = g.getFontMetrics();
    boundWidth  = metrics.stringWidth("A");
    boundWidth2 = boundWidth/2;
    int boundWidth4 = boundWidth2/2;
    resWidth = metrics.stringWidth("A")+boundWidth2;

    seqHeight = metrics.stringWidth("A")+boundWidth;

    String seqS = null;
    if(!drawNumber)
    {
      seqS = seq.getSequence();
      seqLength = seqS.length();
    }

    int istart = 0;
    int istop  = seqLength;
    if (viewPane != null)
    {
      Rectangle viewRect = ((GraphicSequenceCollection)viewPane).getViewRect();
      istart = viewRect.x/resWidth;
      istop  = istart + viewRect.width/resWidth + 2;
      if(istop > seqLength)
        istop = seqLength;
    }

// highlight patterns by making bold
    Vector pvec = null;
    Font fontBold = null;
//  System.out.println("istart "+istart+" istop "+istop);
    if(drawSequence && highlightPattern)
    {
//    pvec = getPatternPositions(istart,istop,seqS);
      int patlen = pattern.length();
      int pstart = istart-patlen;
      if(pstart < 0)
        pstart = 0;
      int pstop = istop+patlen;
      if(pstop > seqLength)
        pstop = seqLength;
      pvec = getPatternPositions(pstart,seqS.substring(pstart,pstop));
      fontBold = new Font(font.getName(),Font.BOLD,
                          font.getSize()+3);
    }

    boolean leftResidue = false;
// draw 
    for(int i=istart;i<istop;i++)
    {
      int ipos = i*resWidth;
      if(drawColorBox)
      {
        g.setColor(getColor(seqS.substring(i,i+1)));
        g.fillRect(ipos,0,resWidth,seqHeight);
      }

      g.setColor(Color.black);
      if(drawSequence)
      {
        if(highlightPattern)
        {
          if(pvec.contains(new Integer(i)))
            g.setFont(fontBold);
        }

        String res = seqS.substring(i,i+1);
        if(prettyPlot)
          leftResidue = prettyDraw(i,ipos,istop,res,seqHeight,
                                   resWidth,leftResidue,g);
        
        g.drawString(res, 
                     ipos+boundWidth4,
                     seqHeight-boundWidth2);
        if(highlightPattern)
          g.setFont(font);
      }
      else if(drawNumber && (int)((i+1)%interval) == 0)
      {
        String snum = Integer.toString(i+1);
        int numWidth = metrics.stringWidth(snum);
        g.drawString(snum,
                     ((int)((i+0.5)*resWidth)-(numWidth/2)),
                     seqHeight-boundWidth2);
      }
      if(drawBlackBox)
      {
        g.setColor(Color.black);
        g.drawRect(ipos,0,resWidth,seqHeight);
      }
    }
  }


  /**
  *
  * @param i		residue number
  * @param ipos		x position
  * @param res		residue
  * @param seqHeight	sequence height
  * @param resWidth     sequence width
  * @param leftResidue	true if the left residue is an identical match
  * @param g		graphics
  *
  */
  private boolean prettyDraw(int i, int ipos, int istop, String res,
                             int seqHeight, int resWidth, 
                             boolean leftResidue, Graphics g)
  {
    String sName = seq.getName();
    GraphicSequenceCollection gsc = (GraphicSequenceCollection)viewPane;
    Color col = gsc.getColor(res,i,sName);
    Color colBackground = gsc.getPrettyBackground(col);

    if(colBackground != null)
    {
      g.setColor(colBackground);
      g.fillRect(ipos,0,resWidth,seqHeight);
    }

    if(!col.equals(Color.black) && gsc.isPrettyBox())
    {
      g.setColor(Color.black);
      // draw left line of cell
      if(!leftResidue)
        g.drawLine(ipos,0,ipos,seqHeight);
      leftResidue = true;

      if(i+2 > istop)
        g.drawLine(ipos+resWidth,0,ipos+resWidth,seqHeight);     
      else
      {
        String resRight = seq.getSequence().substring(i+1,i+2);
        Color rightCol = gsc.getColor(resRight,i+1,sName);

        // draw right line of cell
        if(rightCol.equals(Color.black))
          g.drawLine(ipos+resWidth,0,ipos+resWidth,seqHeight);
      }

      // decide to draw top &/or bottom lines of cell
      int upAndDown = gsc.testUpAndDown(i,seq);

//    System.out.println(i+" upAndDown = "+upAndDown+" name "+sName);
      if(upAndDown == 1 || upAndDown == 3)
        g.drawLine(ipos,0,ipos+resWidth,0);
      if(upAndDown == 2 || upAndDown == 3)
        g.drawLine(ipos,seqHeight-1,ipos+resWidth,seqHeight-1);
    }
    else
      leftResidue = false;


    g.setColor(col);
    return leftResidue;
  }


  /**
  *
  * Find all occurences of the pattern in the sequence between
  * the start and stop positions. Returning all positions of these
  * in a vector.
  * @param istart	start search from
  * @param istop	stop search from
  * @param seqS		sequence string to search
  * @return		vector of all positions which match 
  *
  */
  private Vector getPatternPositions(int istart, int istop,
                                     String seqS)
  {
    Vector pvec = new Vector();
    int patlen = pattern.length();
    int pstart = istart-patlen;
    int pstop  = istop+patlen;
    if(pstart < 0)
      pstart = 0;
    if(pstop > seqLength)
      pstop = seqLength;

    int ipat;
    while( (ipat = seqS.substring(pstart,pstop).indexOf(pattern)) > -1)
    {
      for(int i=0;i<patlen;i++)
        pvec.add(new Integer(ipat+pstart+i));
      pstart = ipat+pstart+1;
    }
//  System.out.println("Showing "+pattern+" :: "+ipat+pstart+" :: "+
//                     "istart "+istart+" istop "+istop);
    return pvec;
  }


  /**
  *
  * Find all occurences of the pattern in the sequence between
  * the start and stop positions. Returning all positions of these
  * in a vector.
  * @param subseqStart	index of the start of the (sub)sequence
  * @param subseq       sequence string to search
  * @return             vector of all positions which match
  *
  */
  private Vector getPatternPositions(int subseqStart,
                                     String subseq)
  {
    subseq = subseq.toLowerCase();
    Vector pvec = new Vector();
    int patlen = pattern.length();
    int pstart = 0;
    int ipat;
    while( (ipat = subseq.substring(pstart).indexOf(pattern)) > -1)
    {
      for(int i=0;i<patlen;i++)
        pvec.add(new Integer(ipat+pstart+subseqStart+i));
      pstart = ipat+pstart+1;
    }
//  System.out.println("Showing "+pattern+" :: "+ipat+pstart+" :: "+
//                     "istart "+istart+" istop "+istop);
    return pvec;
  }


  /**
  *
  * Set pattern to high light
  * @param pattern	pattern to high light
  *
  */
  protected void showPattern(String pattern)
  {
    highlightPattern = true;
    this.pattern = pattern.toLowerCase();
  }
 
 
  /**
  *
  * Set prettyplot display
  * @param prettyPlot	true if the prettyplot display is to
  *			be used
  */
  public void setPrettyPlot(boolean prettyPlot)
  {
    this.prettyPlot = prettyPlot;
  }

 
  /**
  *
  * Determine if using prettyplot display
  * @return  	true if the prettyplot display is to
  *           	be used
  */
  public boolean isPrettyPlot()
  {
    return prettyPlot;
  }


  /**
  *
  * Determine the tool tip to display
  * @param e	mouse event
  * @return	tool tip
  *
  */
  public String getToolTipText(MouseEvent e)
  {
    Point loc = e.getPoint();
    int resPos = (int)(loc.x/resWidth);
    if(resPos < 0)
      return null;

    if(resPos > seq.getSequence().length()-1)
      return null;

    String res = seq.getSequence().substring(resPos,resPos+1);
    String ls = System.getProperty("line.separator");

    if(seq.getID() != null)
      return seq.getID()+"\nResidue: "+res+ls+
           "Position: "+Integer.toString(resPos+1);

    return seq.getName()+"\nResidue: "+res+ls+
           "Position: "+Integer.toString(resPos+1);
  }


  /**
  *
  * Get the tool tip location
  * @param e    mouse event
  * @return     point on the display
  *
  */
  public Point getToolTipLocation(MouseEvent e)
  {
    Point loc = e.getPoint();
    int x = (int)((loc.x+resWidth)/resWidth)*resWidth;
    int y = seqHeight-(resWidth/2);
    return new Point(x,y);
  }


  /**
  *
  * Get a residue colour
  * @param s	residue
  * @return	colour of the residue s
  *
  */
  private Color getColor(String s)
  {
    s = s.toUpperCase();
    if(colorTable.containsKey(s))
      return (Color)colorTable.get(s);
       
    return getBackground();
  }
 

  /**
  *
  * Set a default colour scheme 
  *
  */ 
  public void setDefaultColorHashtable()
  {
    colorTable = new Hashtable();
    colorTable.put("A",Color.green);
    colorTable.put("T",Color.red);
    colorTable.put("C",Color.blue);
    colorTable.put("G",Color.white);
  }


  /**
  *
  * Set whether to draw boxes around residues
  * @param drawBlackBox	true to draw boxes
  *
  */
  public void setDrawBoxes(boolean drawBlackBox)
  {
    if(drawNumber)
      return;
    this.drawBlackBox = drawBlackBox;
//  paintComponent(getGraphics());
  }


  /**
  *
  * Set whether to colour residues
  * @param drawColorBox true to colour residues
  *
  */
  public void setDrawColor(boolean drawColorBox)
  {
    if(drawNumber)
      return;
    this.drawColorBox = drawColorBox;
//  paintComponent(getGraphics());
  }


  /**
  *
  * Set the font size and set the size
  * @param size		font size
  *
  */
  public void setFontSize(int size)
  {
    fontSize = size;
    font = new Font("Monospaced",
                      Font.PLAIN, fontSize);
    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    resWidth = metrics.stringWidth("A")+boundWidth/2;
    seqHeight = metrics.stringWidth("A")+boundWidth;
    setPreferredSize(getPreferredSize());
    repaint();
//  paintComponent(getGraphics());
  }
 

  /**
  *
  * Get the width of a residue
  * @return	residue width
  *
  */
  public int getResidueWidth()
  {
    return resWidth;
  }


  /**
  *
  * Set the colour scheme 
  * @param colorHash	colour scheme to use
  *
  */
  public void setColorScheme(Hashtable colorHash)
  {
    this.colorTable = colorHash;
    setPrettyPlot(false);
  }


  /**
  *
  * Get the font size
  * @return font size
  *
  */
  public int getFontSize()
  {
    return fontSize;
  }


  /**
  *
  * Get the preferred size dimension
  * @return 	preferred dimension for this component
  *
  */
  public Dimension getPreferredSize()
  {
    return new Dimension(getSequenceWidth(),getSequenceHeight());
  }


  /**
  *
  * Re-size the sequence alignment panel if the width of 
  * this sequence pane increases
  *
  */
  protected void viewPaneResize()
  {
    Dimension dpane = viewPane.getPreferredSize();
    GraphicSequenceCollection gsc = (GraphicSequenceCollection)viewPane;
    
    int xpane = (int)dpane.getWidth();
    int xsize = gsc.getPanelWidth();
    if(xsize > xpane)
      viewPane.setPreferredSize(new Dimension(xsize,
                              gsc.getPanelHeight()));
    gsc.setJScrollPaneViewportView();
  }


  /**
  *
  * Get the sequence panel height
  * @return 	sequence height
  *
  */
  public int getSequenceHeight()
  {
    return seqHeight+ypad;
  }


  /**
  *
  * Get the sequence panel width
  * @return     sequence width
  *
  */
  public int getSequenceWidth()
  {
    return resWidth*seqLength;
  }


  /**
  *
  * Get the width of a residue
  * @return 	residue width
  *
  */
  public int getSequenceResidueWidth()
  {
    return resWidth;
  }


  /**
  *
  * Set the sequence length
  * @param s	sequence length
  *
  */
  public void setSequenceLength(int s)
  {
    seqLength = s;
  }


  /**
  *
  * Get the sequence name associated with this display
  * @return	sequence name
  *
  */
  public String getName()
  {
      if(seq==null)
          return null;
    return seq.getName();
  }


  /**
  *
  * Render the sequence name graphic
  * @param g2d	graphic
  *
  */
  public void getNamePrintGraphic(Graphics g2d)
  {
    if(seq == null)
      return;

    String name = seq.getName();
    if(seq.getID() != null)
      name = seq.getID();

    g2d.setFont(font);
    g2d.setColor(Color.black);
    g2d.drawString(name,0,seqHeight-boundWidth2);
  }
 

  /**
  *
  * Render the sequence graphic
  * @param g2d  	graphic
  * @param MAXSEQNAME	maximum size for a sequence name
  * @param istart	sequence start position for graphic
  * @param istop	sequence stop position for graphic
  *
  */
  public void getSequencePrintGraphic(Graphics g2d, int MAXSEQNAME, 
                                      int istart, int istop) 
  {
    //String sName = null;
    String seqS  = null; 

    g2d.setFont(font);
    if(drawSequence)
    {
      //sName = seq.getName();
      //if(seq.getID() != null)
        //sName = seq.getID();

      seqS  = seq.getSequence();
      int seqLength = seq.getLength();
      if(seqLength < istop)
        istop = seqLength;
    }

    int boundWidth4 = boundWidth2/2;
    boolean leftResidue = false;
    FontMetrics metrics = getFontMetrics(font);
    for(int i=istart;i<istop;i++)
    {
      int ipos = (i-istart)*(resWidth)+MAXSEQNAME;
      if(drawColorBox)
      {
        g2d.setColor(getColor(seqS.substring(i,i+1)));
        g2d.fillRect(ipos,0,resWidth,seqHeight);
      }
      g2d.setColor(Color.black);
      if(drawBlackBox)
        g2d.drawRect((i-istart)*(resWidth)+MAXSEQNAME,0,resWidth,seqHeight);


      if(drawSequence)
      {
        String res = seqS.substring(i,i+1);
        if(prettyPlot)
          leftResidue = prettyDraw(i,ipos,istop,res,seqHeight,
                                   resWidth,leftResidue,g2d);

        g2d.drawString(res,
                      ((i-istart)*resWidth)+boundWidth4+MAXSEQNAME,
                      seqHeight-boundWidth2);
      
      }
      else if(drawNumber && (int)((i+1-istart)%interval) == 0)
      {
        String snum = Integer.toString(i+1);
        int numWidth = metrics.stringWidth(snum);
        g2d.drawString(snum,
                     ((int)((i-istart+0.5)*resWidth)-(numWidth/2))+MAXSEQNAME,
                     seqHeight-boundWidth2);
      }
    }
  }
 

  /**
  *
  * Override actionPerformed for popup menu actions
  * @param e	action event
  * 
  */
  public void actionPerformed(ActionEvent e)
  {
    JMenuItem source = (JMenuItem)(e.getSource());
    if(source.getText().startsWith("Delete "))
    {
      ((GraphicSequenceCollection)viewPane).deleteSequence(seq.getName());
    }
    else if(source.getText().startsWith("Reverse Complement"))
    {
      seq.reverseComplementSequence();
      paintComponent(getGraphics());
    }
    else if(source.getText().startsWith("Reverse "))
    {
      seq.reverseSequence();
      paintComponent(getGraphics());
    }
    else if(source.getText().startsWith("Complement "))
    {
      seq.complementSequence();
      paintComponent(getGraphics());
    }
  }


  /**
  *
  * Popup listener
  *
  */
  class PopupListener extends MouseAdapter
  {
    public void mousePressed(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    public void mouseReleased(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    private void maybeShowPopup(MouseEvent e)
    {
      if(e.isPopupTrigger())
        popup.show(e.getComponent(),
                e.getX(), e.getY());
    }
  }

}

