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
import java.awt.event.*;
import java.util.*;
import java.io.File;
import javax.swing.border.*;
import java.net.URL;

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.form.TextFieldInt;
import org.emboss.jemboss.gui.ScrollPanel;
import org.emboss.jemboss.gui.Browser;

/**
*  
* Displays a grapical representation of a collection of
* sequences.
*
*/
public class AlignJFrame extends JFrame
{

  protected JScrollPane jspSequence; // Sequence scrollpane
  protected static GraphicSequenceCollection gsc;
  private static Matrix mat;
  private PrettyPlotJFrame ppj = null;
  protected static JTextField statusField = new JTextField();
  private File sequenceFile = null;
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  protected JCheckBoxMenuItem residueColor;
  protected Hashtable currentColour;
  protected boolean useExitMenu = false;  // whether to use 'Exit' or 'Close'
  protected JMenuBar menuBar;
  PatternJFrame findPattern;
  JCheckBoxMenuItem drawColorBox;
 
  /**
  *
  * @param vseqs	vector containing Sequence objects
  *
  */ 
  public AlignJFrame(Vector vseqs)
  {
    this();
    if(vseqs != null && vseqs.size() > 0)
      openMethod(vseqs);
  }


  /**
  *
  * @param seqFile	sequence file
  *
  */
  public AlignJFrame(File seqFile)
  {
    this();

    SequenceReader sr = new SequenceReader(seqFile);
    sequenceFile = sr.getSequenceFile();

    if(sr.getSequenceVector()==null)
    {
        statusField.setText("Jemboss not able to read sequences in "+
                sequenceFile.getName());
        return;
    }
    
    openMethod(sr.getSequenceVector());
    setTitle("Jemboss Alignment Viewer    :: "+
              sequenceFile.getName());
  }


  /**
  *
  * @param seqString	formatted sequence string
  * @param name 	name of sequence set
  *
  */
  public AlignJFrame(String seqString, String name)
  {
    this();

    SequenceReader sr = new SequenceReader(seqString);
    sequenceFile = null;
    openMethod(sr.getSequenceVector());
    setTitle("Jemboss Alignment Viewer    :: "+name);
  }


  public AlignJFrame()
  {
    this(false);
  }


  /**
  *
  * @param useExitMenu	true if an exit menu is to be displayed
  *			otherwise a close menu is used
  *
  */
  public AlignJFrame(boolean useExitMenu)
  {
      this(useExitMenu, new Matrix("resources/resources.jar",
              Matrix.DEFAULT_MATRIX));
  }
  
  public AlignJFrame(boolean useExitMenu, Matrix mat_){
    super("Jemboss Alignment Editor");

    this.useExitMenu = useExitMenu;

    final Dimension dScreen = getToolkit().getScreenSize();
  
    mat = mat_;
    jspSequence = new JScrollPane();
    jspSequence.getViewport().setBackground(Color.white);

    final JButton leftbutt = new JButton("Lock");
    jspSequence.setCorner(JScrollPane.LOWER_LEFT_CORNER,
                                              leftbutt);
    leftbutt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(leftbutt.getText().equals("Lock"))
        {
          gsc.setSequenceLock(true);
          leftbutt.setText("Unlock");
        }
        else
        {
          gsc.setSequenceLock(false);
          leftbutt.setText("Lock");
        }
      }
    });

    final JPanel mainPane = (JPanel)getContentPane();

// set up a menu bar
    menuBar = new JMenuBar();

// File menu
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

// open sequence file
    final JMenuItem calculateCons = new JMenuItem("Consensus");
    final JMenuItem calculatePlotCon = new JMenuItem("Consensus plot");

    JMenuItem openSequence = new JMenuItem("Open...");
    openSequence.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        SequenceReader sr = new SequenceReader();

        if(sr.isReading())
        {
          sequenceFile = sr.getSequenceFile();
          openMethod(sr.getSequenceVector());
          calculateCons.setText("Calculate consensus");
          calculatePlotCon.setText("Calculate consensus plot");
          setTitle("Jemboss Alignment Viewer    :: "+
                    sequenceFile.getName());
        }
      }
    });
    fileMenu.add(openSequence);

// save 
    JMenuItem saveAsMenu = new JMenuItem("Save As...");
    saveAsMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new SequenceSaver(gsc.getSequenceCollection(),sequenceFile);
      }
    });
    fileMenu.add(saveAsMenu);

    JMenuItem saveConsMenu = new JMenuItem("Save Consensus");
    saveConsMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      { 
        Vector v = new Vector();
        v.add(gsc.getConsensusSequence());
        new SequenceSaver(v);
      }
    });
    fileMenu.add(saveConsMenu);

  
// print
    JMenu printMenu = new JMenu("Print");
    fileMenu.add(printMenu);

    JMenuItem print = new JMenuItem("Print Postscript...");
    print.setToolTipText("Print using available printers in your computer\n" +
    		"or export alignment image to a postscript file (if you have " +
    		" installed postscript printers)");
    print.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new PrintAlignment(gsc);
      }
    });
    printMenu.add(print);

//
    JMenuItem printImage = new JMenuItem("Print Image Files (png/jpeg)...");
    printImage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
        pai.print();
      }
    });
    printMenu.add(printImage);
 
    JMenuItem printOneImage = new JMenuItem("Print to Single Image File...");
    printOneImage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {   
        PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
        String fsave = pai.showOptions();
        if(fsave == null)
          return;

        pai.print(fsave,0.,0.,0.,0.);
      }
    });
    printMenu.add(printOneImage);

// print preview
    JMenu printPreviewMenu = new JMenu("Print Preview");
    fileMenu.add(printPreviewMenu);

    JMenuItem printPreview = new JMenuItem("Multiple Pages...");
    printPreview.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
        pai.printPreview();
      }
    });
    printPreviewMenu.add(printPreview);

    JMenuItem printSinglePreview = new JMenuItem("Single Page...");
    printSinglePreview.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {  
        PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
        /*String type = */pai.showPrintPreviewOptions();
        pai.printSinglePagePreview();
      }
    });
    printPreviewMenu.add(printSinglePreview);

// close
    fileMenu.add(new JSeparator());
    if(!useExitMenu)
    {
      JMenuItem close = new JMenuItem("Close");
      close.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

      close.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          dispose();
          findPattern.dispose();
        }
      });
      fileMenu.add(close);
    }
    else         // exit
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
    menuBar.add(fileMenu);
  
// Edit menu
    JMenu editMenu = new JMenu("Edit");
    menuBar.add(editMenu);

    JMenuItem insertAnn = new JMenuItem("Insert Annotation Sequence");
    editMenu.add(insertAnn);
    insertAnn.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ScrollPanel pane = new ScrollPanel(new BorderLayout());
        Box bacross = Box.createVerticalBox();
        JRadioButton openFile = new JRadioButton("Read from File");
        JRadioButton cut      = new JRadioButton("Cut and Paste");
        ButtonGroup group = new ButtonGroup();
        group.add(openFile);
        group.add(cut);
        cut.setSelected(true);
        bacross.add(openFile);
        bacross.add(cut);
        pane.add(bacross,BorderLayout.CENTER);
        int selectedValue = JOptionPane.showConfirmDialog(null,
                          pane, "Cut and Paste/Read from File",
                          JOptionPane.OK_CANCEL_OPTION,
                          JOptionPane.QUESTION_MESSAGE);
        if(selectedValue == JOptionPane.OK_OPTION)
        {
          SequenceReader sr = null;
          if(openFile.isSelected())
            sr = new SequenceReader();
          else
          {
            Paste pastePane = new Paste();
            selectedValue = JOptionPane.showConfirmDialog(null,
                          pastePane, "Cut and Paste",
                          JOptionPane.OK_CANCEL_OPTION,
                          JOptionPane.QUESTION_MESSAGE);
            if(selectedValue == JOptionPane.OK_OPTION)
              sr = new SequenceReader(pastePane.getSequence());
          }
          if(sr != null && sr.isReading())
          {
            sequenceFile = sr.getSequenceFile();
            gsc.addAnnotationSequence(sr.getSequence(0));
            Dimension dpane = gsc.getPanelSize();
            gsc.setPreferredSize(dpane);
            gsc.setNamePanelWidth(gsc.getNameWidth());
            jspSequence.setViewportView(gsc);
          }
        }
      }
    });

    JMenuItem trimMenu = new JMenuItem("Trim Sequences");
    editMenu.add(trimMenu);
    trimMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ScrollPanel pane = new ScrollPanel(new BorderLayout());
        Box bacross = Box.createHorizontalBox();
        TextFieldInt start = new TextFieldInt();
        start.setValue(1);
        TextFieldInt end   = new TextFieldInt();  
        end.setValue(gsc.getMaxSeqLength());
        bacross.add(start);
        bacross.add(new JLabel(" start "));
        bacross.add(end);
        bacross.add(new JLabel(" end"));
        pane.add(bacross,BorderLayout.CENTER);
        int selectedValue = JOptionPane.showConfirmDialog(null,
                          pane, "Select Sequence Range to Use",
                          JOptionPane.OK_CANCEL_OPTION,      
                          JOptionPane.QUESTION_MESSAGE);
        if(selectedValue == JOptionPane.OK_OPTION)
        {
          Vector vseq = gsc.getSequenceCollection();
          Enumeration enumer = vseq.elements();
          while(enumer.hasMoreElements())
          {
            Sequence s = (Sequence)enumer.nextElement();
            s.trim(start.getValue(),end.getValue());  
          }
          gsc.setMaxSeqLength();
          gsc.repaint();
        }
      }
    });

    JMenuItem unlock = new JMenuItem("Unlock All Sequences");
    editMenu.add(unlock);
    unlock.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setSequenceLock(false);
      }
    });

// View menu
    JMenu viewMenu = new JMenu("View");
    viewMenu.setMnemonic(KeyEvent.VK_V);

// find pattern
    JMenuItem findMenu = new JMenuItem("Find pattern");
    viewMenu.add(findMenu);
    findPattern = new PatternJFrame();
    final PatternJFrame patFrame = findPattern;
    findMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        Point pos = getLocationOnScreen();
        pos.y = pos.y - patFrame.getHeight();
        if(pos.y+patFrame.getHeight() > dScreen.getHeight())
          pos.x = (int)(dScreen.getWidth()-patFrame.getHeight());
        
        patFrame.setLocation(pos);
        patFrame.setGraphic(gsc);
        patFrame.setVisible(true);
        patFrame.setExtendedState(patFrame.getExtendedState() & ~ICONIFIED);
        patFrame.requestFocusInWindow();
        patFrame.toFront();
      }
    });
    viewMenu.add(new JSeparator());

// matrix display
    JMenuItem showMatrix = new JMenuItem("Matrix Display");
    viewMenu.add(showMatrix);
    final MatrixJFrame matFrame = new MatrixJFrame(mat,statusField,
                                                   this);
    showMatrix.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        matFrame.setMatrix(mat);
        matFrame.setVisible(true);
        matFrame.toFront();
      }
    });

// colour display
    JMenuItem showColour = new JMenuItem("Colour Display");
    viewMenu.add(showColour);
    final ColourJFrame colFrame = new ColourJFrame(this);
    showColour.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        Point pos = getLocationOnScreen();
        pos.x = pos.x + getWidth();
        if(pos.x+colFrame.getWidth() > dScreen.getWidth())
          pos.x = (int)(dScreen.getWidth()-colFrame.getWidth());

        colFrame.setLocation(pos);
        colFrame.setCurrentColour(currentColour);
        colFrame.setVisible(true);
        colFrame.toFront();
      }
    });
    viewMenu.add(new JSeparator());
     
    colourMenus(viewMenu);
   
//pretty plot
    final JMenuItem pretty = new JMenuItem("Colour Identical/Matches");
    viewMenu.add(pretty);
    viewMenu.add(new JSeparator());

//draw black box
    final JCheckBoxMenuItem drawBoxes = new JCheckBoxMenuItem("Draw boxes",false);
    drawBoxes.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setDrawBoxes(drawBoxes.isSelected());
      }
    });
    viewMenu.add(drawBoxes);

//draw colored boxes
    viewMenu.add(drawColorBox);

    pretty.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(ppj == null)
          ppj = new PrettyPlotJFrame(gsc);
        else
          ppj.setGraphicSequenceCollection(gsc);
        gsc.setMatrix(mat);
        ppj.setVisible(true);
      }
    });
    menuBar.add(viewMenu);

// calculate menu
    JMenu calculateMenu = new JMenu("Calculate");
    menuBar.add(calculateMenu);

// consensus sequence
    final ConsensusOptions options = new ConsensusOptions(jspSequence);
    calculateCons.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setCursor(cbusy);
        try{
        gsc.deleteSequence(Consensus.DEFAULT_SEQUENCE_NAME);

        float wgt = getTotalWeight(gsc.getSequenceCollection());
        float plu = 0.f;
        try
        {
          plu = options.getPlurality();
        }
        catch(NumberFormatException nfe)
        {
          plu = wgt/2.f;
        }

        float cas = 0.f;
        try
        {
          cas = options.getCase();
        }
        catch(NumberFormatException nfe)
        {
          cas = wgt/2.f;
        }

        Consensus conseq = new Consensus(mat,   
                    gsc.getSequenceCollection(),
                    plu,cas,
                    options.getIdentity());

        int fontSize = gsc.getFontSize();
        gsc.addSequence(conseq.getConsensusSequence(),true,0,fontSize);

//      if(pretty.isSelected())
//        gsc.setPrettyPlot(pretty.isSelected(),ppj);

        Dimension dpane = gsc.getPanelSize();
        gsc.setPreferredSize(dpane);
        gsc.setNamePanelWidth(gsc.getNameWidth());
        jspSequence.setViewportView(gsc);
        calculateCons.setText("Recalculate consensus");
        } finally {
            setCursor(cdone);
        }
      }
    });
    calculateMenu.add(calculateCons);

    
    JMenuItem consOptions = new JMenuItem("Set consensus options...");
    consOptions.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        try
        {
//          Vector vseq = gsc.getSequenceCollection();
//        Enumeration enumer = vseq.elements();
          float wgt = getTotalWeight(gsc.getSequenceCollection());
          options.setCase(wgt/2.f);
          options.setPlurality(wgt/2.f);
          options.setGraphicSequenceCollection(gsc);
        }
        catch(NullPointerException npe){}
        options.setMatrix(mat);
        options.setVisible(true);
      }
    });
    calculateMenu.add(consOptions);
    calculateMenu.add(new JSeparator());

// %age identity between pairs
    JMenuItem calculateId = new JMenuItem("Identity table");
    calculateId.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        IDTableJFrame idtab = new IDTableJFrame(gsc.getSequenceCollection());
        idtab.setVisible(true);
      }
    });
    calculateMenu.add(calculateId);
    calculateMenu.add(new JSeparator());

// consensus plot
    calculatePlotCon.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setCursor(cbusy);
        gsc.showConsensusPlot(mat,2);
        setCursor(cdone);
        calculatePlotCon.setText("Recalculate Consensus plot");
      }
    });
    calculateMenu.add(calculatePlotCon);
    calculateMenu.add(new JSeparator());

// sort by id
    JMenuItem test = new JMenuItem("Sort by ID");
    calculateMenu.add(test);
    test.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.idSort();
        jspSequence.setViewportView(gsc);
      }
    });
    
// font menu
    String sizes[] = {"10", "12", "14", "16", "18"};
    final JComboBox fntSize = new JComboBox(sizes);
    fntSize.setToolTipText("font size");
    fntSize.setSelectedItem("12");
    menuBar.add(fntSize);
    fntSize.setEditable(true);
    Dimension dfont = new Dimension(50,20);
    fntSize.setPreferredSize(dfont);
    fntSize.setMaximumSize(dfont);
    fntSize.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        String fsize = (String)fntSize.getSelectedItem();
        if(gsc !=null)
          gsc.setFontSizeForCollection(Integer.parseInt(fsize));
      }
    });

    setJMenuBar(menuBar);

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
          URL inURL = cl.getResource("resources/readmeAlign.html");
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

// set size of sequence panel
    Dimension d = new Dimension(700,300);
    jspSequence.setPreferredSize(d);

    JPanel seqNamePanel = new JPanel(new BorderLayout());
    seqNamePanel.add(jspSequence,BorderLayout.CENTER);

    mainPane.add(jspSequence,BorderLayout.CENTER);

    Border loweredbevel = BorderFactory.createLoweredBevelBorder();
    Border raisedbevel = BorderFactory.createRaisedBevelBorder();
    Border compound = BorderFactory.createCompoundBorder(raisedbevel,loweredbevel);
    statusField.setBorder(compound);
    statusField.setEditable(false);
    statusField.setText("Current matrix: "+mat.getCurrentMatrixName());
    mainPane.add(statusField,BorderLayout.SOUTH);

    addWindowListener(new winExit());
    pack();
    setLocation( (int)(dScreen.getWidth()-getWidth())/3,
                 (int)(dScreen.getHeight()-getHeight())/3 );
  }


  public static float getTotalWeight(Vector vseq)
  {
    float wgt = 0.f;
    vseq = gsc.getSequenceCollection();
    Enumeration enumer = vseq.elements();
    while(enumer.hasMoreElements())
    {
      Sequence s = (Sequence)enumer.nextElement();
      if(!s.getName().equals(Consensus.DEFAULT_SEQUENCE_NAME))
        wgt+=s.getWeight();
    }
    return wgt;
  }


  /**
  *
  * Set the scoring matrix
  * @param mat 	scoring matrix to use
  *
  */
  public void setMatrix(Matrix mat)
  {
    AlignJFrame.mat = mat;
  }

  /**
  *
  * Force a re-display of the sequences with a new colour
  * scheme.
  * @param hash		hash of the colour scheme
  *
  */
  public void repaintSequences(Hashtable hash)
  {
    gsc.setColorScheme(hash);
    gsc.repaint();
  }

  /**
  *
  * Given a Vector of Sequence display them in the editor
  * @param seqVector	vector containing Sequence objects
  *
  */
  protected void openMethod(Vector seqVector)
  {
    gsc = new GraphicSequenceCollection(seqVector,
                                   jspSequence,true,false,true,true,
                                   statusField);
// set colour scheme
    gsc.setColorScheme(SequenceProperties.residueColor);
    currentColour = (Hashtable)SequenceProperties.residueColor.clone();
    residueColor.setSelected(true);
    jspSequence.setViewportView(gsc);

    colourScheme("Residue colour");
  }


  /**
  *
  * Update the status bar with the selected colour scheme
  * being used.
  * @param colScheme 	name of colour scheme
  *
  */
  protected void colourScheme(String colScheme)
  {
    String status = statusField.getText();
    int ncol = status.indexOf("Colour Scheme: ");
    if(ncol > -1)
      statusField.setText(status.substring(0,ncol)+
                          "Colour Scheme: "+colScheme);
    else
      statusField.setText(status+"              "+
                          "Colour Scheme: "+colScheme);
    drawColorBox.setSelected(true);
  }

  /**
  *
  * Constructs the colour menus
  *
  */
  private void colourMenus(JMenu viewMenu)
  {
    final ButtonGroup group = new ButtonGroup();
    // an invisble radio button to workaround earlier versions of java
    // which is selected only when user deselect "Colour boxes" option
    final JRadioButton invisibleb = new JRadioButton();
    group.add(invisibleb);
    drawColorBox = new JCheckBoxMenuItem("Colour boxes",true);
    drawColorBox.addItemListener(new ItemListener()
    {
        ButtonModel lastSelection;
        public void itemStateChanged(ItemEvent e) {
            boolean selected = e.getStateChange() == ItemEvent.SELECTED;
            gsc.setDrawColor(selected);
            if(!selected){
            	lastSelection = group.getSelection();
            	//java6 has a group.clearSelection(); method that in the future
            	//can replace the invisible button workaround 
            	invisibleb.setSelected(true);
            } else if (invisibleb.isSelected()){
            	lastSelection.setSelected(true);
            }
        }
    });
    
// property colour menus
    JMenu propertyMenu = new JMenu("Colour by Property");
    viewMenu.add(propertyMenu);

    JCheckBoxMenuItem acidColor =
                new JCheckBoxMenuItem("Red=acidic, Blue=basic");
    propertyMenu.add(acidColor);
    group.add(acidColor);
    acidColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.acidColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.acidColor;
        colourScheme("Red=acidic, Blue=basic");
      }
    });

    JCheckBoxMenuItem polarColor =
                new JCheckBoxMenuItem("Red=polar");
    propertyMenu.add(polarColor);
    group.add(polarColor);
    polarColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.polarColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.polarColor;
        colourScheme("Red=polar");
      }
    });

    JCheckBoxMenuItem hydrophobicColor = 
               new JCheckBoxMenuItem("Red=Hydrophobic");
    propertyMenu.add(hydrophobicColor);
    group.add(hydrophobicColor);
    hydrophobicColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.hydrophobicColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.hydrophobicColor;
        colourScheme("Red=Hydrophobic");
      }
    });

    JCheckBoxMenuItem aromaticColor = 
               new JCheckBoxMenuItem("Red=Aromatic, Blue=Aliphatic");
    propertyMenu.add(aromaticColor);
    group.add(aromaticColor);
    aromaticColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.aromaticColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.aromaticColor;
        colourScheme("Red=Aromatic, Blue=Aliphatic");
      }
    });

    JCheckBoxMenuItem surfaceColor =
               new JCheckBoxMenuItem("Red=Surface, Blue=Buried");
    propertyMenu.add(surfaceColor);
    group.add(surfaceColor);
    surfaceColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.surfaceColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.surfaceColor;
        colourScheme("Red=Surface, Blue=Buried");
      }
    });

    JCheckBoxMenuItem chargeColor  =
               new JCheckBoxMenuItem("Red=Positive, Blue=Negative");
    propertyMenu.add(chargeColor);
    group.add(chargeColor);
    chargeColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.chargeColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.chargeColor;
        colourScheme("Red=Positive, Blue=Negative");
      }
    });

    JCheckBoxMenuItem sizeColor  =
               new JCheckBoxMenuItem("Red=Tiny, Green=Small, Blue=Large");
    propertyMenu.add(sizeColor);
    group.add(sizeColor);
    sizeColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.sizeColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.sizeColor;
        colourScheme("Red=Tiny, Green=Small, Blue=Large");
      }
    });

// other colour schemes
    JCheckBoxMenuItem taylor = new JCheckBoxMenuItem("Taylor Colour");
    viewMenu.add(taylor);
    group.add(taylor);
    taylor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.taylorColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.taylorColor;
        colourScheme("Taylor");
      }
    });

    residueColor = new JCheckBoxMenuItem("Residue Colour");
    viewMenu.add(residueColor);
    group.add(residueColor);
    residueColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.residueColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.residueColor;
        colourScheme("Residue");
      }
    });

    JCheckBoxMenuItem rasmolColor = new JCheckBoxMenuItem("Rasmol Colour");
    viewMenu.add(rasmolColor);
    group.add(rasmolColor);
    rasmolColor.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.rasmolColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.rasmolColor;
        colourScheme("Rasmol");
      }
    });

    JCheckBoxMenuItem nuc = new JCheckBoxMenuItem("Nucleotide Colour");
    viewMenu.add(nuc);
    group.add(nuc);
    nuc.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        gsc.setColorScheme(SequenceProperties.baseColor);
        jspSequence.setViewportView(gsc);
        currentColour = SequenceProperties.baseColor;
        colourScheme("Nucleotide");
      }
    });
    viewMenu.add(new JSeparator());
  }


  /**
  *
  *
  *  red, blue, cyan, darkGray, gray , green, lightGray,
  *  magenta , orange, pink, white, yellow, black
  *
  */
  private static Color resolveColor(String[] args,int index)
  {
    if(args[index].equalsIgnoreCase("red"))
      return Color.red;
    else if(args[index].equalsIgnoreCase("blue"))
      return Color.blue;
    else if(args[index].equalsIgnoreCase("black"))
      return Color.black;
    else if(args[index].equalsIgnoreCase("cyan"))
      return Color.cyan;
    else if(args[index].equalsIgnoreCase("darkGray"))
      return Color.darkGray;
    else if(args[index].equalsIgnoreCase("gray"))
      return Color.gray;
    else if(args[index].equalsIgnoreCase("green"))
      return Color.green;
    else if(args[index].equalsIgnoreCase("lightGray"))
      return Color.lightGray;
    else if(args[index].equalsIgnoreCase("magenta"))
      return Color.magenta;
    else if(args[index].equalsIgnoreCase("orange"))
      return Color.orange;
    else if(args[index].equalsIgnoreCase("pink"))
      return Color.pink;
    else if(args[index].equalsIgnoreCase("white"))
      return Color.white;
    else if(args[index].equalsIgnoreCase("yellow"))
      return Color.yellow;
    else if(args[index].equalsIgnoreCase("black"))
      return Color.black;

    return null;
  }
  
  /**
  *
  * Extends WindowAdapter to close window
  *
  */
  class winExit extends WindowAdapter
  {
     public void windowClosing(WindowEvent we)
     {
        dispose();
        findPattern.dispose();
     }
  }


  public static void main(String args[])
  {
    AlignJFrame ajFrame = null;

    if( args.length > 0 )
    {
      for(int i=0;i<args.length;i++)
      {
        if(args[i].indexOf("-help") > -1)
        {
          System.out.println(
              "\n                  Jemboss Alignment Editor\n\n"+
              "DESCRIPTION\n"+
              "The Jemboss Alignment Editor can be used interactively to\n"+
              "edit a sequence alignment (read in fasta or MSF format). It can\n"+
              "also be used from the command line to produce image files\n"+
              "of the alignment.\n\nUSAGE\n"+
              "java org/emboss/jemboss/editor/AlignJFrame file [options]\n\n"+
              "file       This is the multiple sequence alignment in\n"+
              "           fasta or MSF format.\n\n"+
              "OPTIONS\n"+
              "-calc      Calculate consensus and display under the alignment.\n"+
              "           The following 3 flags can be used to define values\n"+
              "           used in the calculations.\n"+
              "           -plu       (plurality) minimum positive match score\n"+
              "                      value for there to be a consensus.\n"+
              "           -numid     minimum number of identities for there to\n"+
              "                      be a consensus.\n"+
              "           -case      minimum positive match score for setting\n"+
              "                      the consensus to upper-case.\n"+
              "-color     Used to define a colour scheme, below is the list of\n"+
              "           available colour schemes:\n"+
              "           taylor\n"+
              "           residue\n"+
              "           rasmol\n"+
              "           acid\n"+
              "           polar\n"+
              "           hydrophobic\n"+
              "           aromatic\n"+
              "           surface\n"+
              "           charge\n"+
              "           size\n"+
              "           base\n\n"+
              "       java org.emboss.jemboss.editor.AlignJFrame file -color size\n\n"+
              "-font      Set the font size.\n"+
              "-id        Display a percentage ID pair table.\n"+
              "-noshow    Turns of the alignment display.\n"+
              "-nres      Number of residues to each line is a print out.\n"+
              "-pretty    EMBOSS prettyplot colour scheme. The -matrix flag option\n"+
              "           can be used to define a scoring matrix for identifying\n"+
              "           positive matches.\n"+
              "           -noBox         switch off box drawing around identical and\n"+
              "                          positive matches.\n"+
              "           -minID         define the minimum number of identities. The\n"+
              "                          default for this is the number of sequences\n"+
              "                          in the file.\n"+
              "           -match         define a threshold value for the number of\n"+
              "                          positive matches, the default is half the\n"+
              "                          total wgt.\n"+
              "           -colID         define a lettering colour for the identities.\n"+
              "           -colIDBack     define a background colour for identities.\n"+
              "           -colMatch      define a lettering  colour for positive matches.\n"+  
              "           -colMatchBack  define a background colour for positive\n"+
              "                           matches.\n"+
              "           Available colour options:\n"+  
              "           red, blue, cyan, darkGray, gray , green, lightGray,\n"+
              "           magenta , orange, pink, white, yellow, black\n"+     
              "-print     Print the alignment image. The following 2 flags can be\n"+
              "           used along with the print flag\n"+
              "           -prefix     prefix for image output file.\n"+
              "           -onePage    fit the alignment to one page. This option must be\n"+
              "                       be used with the -nres flag to define the residues\n"+
              "                       per line.\n"+
              "           -type       png or jpeg (default is jpeg).\n"+
              "           -antialias  turn anti-aliasing on.\n"+
              "           -landscape  Print as landscape (the default is portrait).\n"+
              "           -margin     Define the left, right, top and bottom margin\n"+
              "                       (in cm).\n"+
              "       java org.emboss.jemboss.editor.AlignJFrame file -matrix EBLOSUM62 \\\n"+
              "                -noshow -print -margin 0.5 0.5 0.5 0.5\n\n"+
              "-matrix    To define a scoring matrix. Used with the -pretty and -calc\n"+
              "           option.\n"+
              "-list      List the available scoring matrix files.\n\n"+
              "EXAMPLE\n"+
              "java org.emboss.jemboss.editor.AlignJFrame file -matrix EBLOSUM80 \\\n"+
              "                             -pretty -noshow -id -print -type png\n\n"+
              "java org.emboss.jemboss.editor.AlignJFrame file -matrix EPAM250 \\\n"+
              "                -pretty -colIDBack black -colID white -print \\\n"+
              "                -margin 0.5 0.5 0.5 0.0 -noshow\n");
          System.exit(0);
        }
      }

      File seqFile = new File(args[0]);
      if(seqFile.canRead())
      {
        JembossParams jp = new JembossParams();
        String ed = jp.getEmbossData();
        Matrix m = new Matrix(ed, Matrix.DEFAULT_MATRIX);
        ajFrame = new AlignJFrame(true, m);
        SequenceReader sr = new SequenceReader(seqFile);
        sr.getSequenceFile();
        ajFrame.openMethod(sr.getSequenceVector());
        ajFrame.setTitle("Jemboss Alignment Viewer    :: "+
                         seqFile.getName());
        gsc.setMatrix(mat);
      }

      String prefix = "output";
      String type   = "jpeg";
      boolean show  = true;
      boolean print = false;
      int nresiduesPerLine = 0;

      float wgt = getTotalWeight(gsc.getSequenceCollection());
      double lmargin = -0.5;  // left margin
      double rmargin = -0.5;  // right margin
      double tmargin = -0.5;  // top margin
      double bmargin = -0.5;  // bottom margin
      float plu = wgt/2.f;
      float cas = wgt/2.f;
      int ident = 0;
      int minID = gsc.getNumberSequences();
      float match = wgt/2.f;
      Color colID        = Color.red;
      Color colMatch     = Color.blue;
      Color colIDBack    = Color.white;
      Color colMatchBack = Color.white;
      boolean prettyBox  = true;
      boolean landscape  = false;
      boolean onePage    = false;
      boolean antialias  = false;

      for(int i=0;i<args.length;i++)
      {
        if(args[i].indexOf("-matrix") > -1)
        {
          mat = new Matrix("resources/resources.jar",
                           args[i+1]);
          gsc.setMatrix(mat);
          statusField.setText("Current matrix: "+args[i+1]);
        }
        else if(args[i].indexOf("-plu") > -1)
          plu = Float.parseFloat(args[i+1]);
        else if(args[i].indexOf("-match") > -1)
          match = Float.parseFloat(args[i+1]);
        else if(args[i].indexOf("-case") > -1)
          cas = Float.parseFloat(args[i+1]);
        else if(args[i].indexOf("-numid") > -1)
          ident = Integer.parseInt(args[i+1]);
        else if(args[i].indexOf("-colIDBack") > -1)
        {
          Color col = resolveColor(args,i+1);
          if(col != null)
            colIDBack = col;
        }
        else if(args[i].indexOf("-colMatchBack") > -1)
        {
          Color col = resolveColor(args,i+1);
          if(col != null)
            colMatchBack = col;
        }
        else if(args[i].indexOf("-colMatch") > -1)
        {
          Color col = resolveColor(args,i+1);
          if(col != null)
            colMatch = col;
        }
        else if(args[i].indexOf("-colID") > -1)
        {
          Color col = resolveColor(args,i+1);
          if(col != null)
            colID = col;
        }
        else if(args[i].indexOf("-minID") > -1)
          minID = Integer.parseInt(args[i+1]);
        else if(args[i].indexOf("-noBox") > -1)
          prettyBox = false;
        else if(args[i].indexOf("-font") > -1)
          gsc.setFontSizeForCollection(Integer.parseInt(args[i+1]));
        else if(args[i].indexOf("-landscape") > -1)
          landscape = true;
        else if(args[i].indexOf("-margin") > -1)
        {
          lmargin = Double.parseDouble(args[i+1]);
          rmargin = Double.parseDouble(args[i+2]);
          tmargin = Double.parseDouble(args[i+3]);
          bmargin = Double.parseDouble(args[i+4]);
        }
        else if(args[i].indexOf("-onePage") > -1)
          onePage = true;
        else if(args[i].indexOf("-antialias") > -1)
          antialias = true;
      }

      for(int i=0;i<args.length;i++)
      {
        if(args[i].indexOf("-color") > -1)
        {
          i++;
          if(args[i].startsWith("taylor"))      
            gsc.setColorScheme(SequenceProperties.taylorColor);
          else if(args[i].startsWith("residue"))
            gsc.setColorScheme(SequenceProperties.residueColor);
          else if(args[i].startsWith("rasmol"))
            gsc.setColorScheme(SequenceProperties.rasmolColor);
          else if(args[i].startsWith("acid"))
            gsc.setColorScheme(SequenceProperties.acidColor);
          else if(args[i].startsWith("polar"))
            gsc.setColorScheme(SequenceProperties.polarColor);
          else if(args[i].startsWith("hydro"))
            gsc.setColorScheme(SequenceProperties.hydrophobicColor);
          else if(args[i].startsWith("aromatic"))
            gsc.setColorScheme(SequenceProperties.aromaticColor);
          else if(args[i].startsWith("surface"))
            gsc.setColorScheme(SequenceProperties.surfaceColor);
          else if(args[i].startsWith("charge"))
            gsc.setColorScheme(SequenceProperties.chargeColor);
          else if(args[i].startsWith("size"))
            gsc.setColorScheme(SequenceProperties.sizeColor);
          else if(args[i].startsWith("base"))
            gsc.setColorScheme(SequenceProperties.baseColor);
        }
        else if(args[i].indexOf("-calc") > -1)
        {
          Consensus conseq = new Consensus(mat,
                    gsc.getSequenceCollection(),
                    plu,cas,ident);

          int fontSize = gsc.getFontSize();
          gsc.addSequence(conseq.getConsensusSequence(),true,5,fontSize);

          Dimension dpane = gsc.getPanelSize();
          gsc.setPreferredSize(dpane);
          gsc.setNamePanelWidth(gsc.getNameWidth());
        }
        else if(args[i].indexOf("-list") > -1)
          System.out.println("AVAILABLE DATABASES:\n"+
                             mat.getKeyNamesString());
        else if(args[i].indexOf("-id") > -1)
        {
          IDTableJFrame idtab = new IDTableJFrame(gsc.getSequenceCollection()); 
          idtab.printTable();
        }
        else if(args[i].indexOf("-noshow") > -1)
          show = false;
        else if(args[i].indexOf("-print") > -1)
          print = true;
        else if(args[i].indexOf("-nres")  > -1)
          nresiduesPerLine = Integer.parseInt(args[i+1]);
        else if(args[i].indexOf("-pretty")  > -1)          
        {
          PrettyPlotJFrame pretty = new PrettyPlotJFrame(minID,
                  match,colID,colMatch,
                  colIDBack,colMatchBack,prettyBox);
          gsc.setPrettyPlot(true,pretty);
          gsc.setDrawBoxes(false);
          gsc.setDrawColor(false);
        }  
        else if(args[i].indexOf("-preview")  > -1)
        {
          PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
          pai.printPreview();
          show = true;
        }
        else if(args[i].indexOf("-type")  > -1)
        {
          if( args[i+1].equalsIgnoreCase("png") ||
              args[i+1].equalsIgnoreCase("jpeg") )
            type = args[i+1];
          else
            System.out.println("UNKOWN PRINT FORMAT: "+args[i+1]+
                               " reverting to default format "+type);
        }
        else if(args[i].indexOf("-prefix")  > -1)
          prefix = args[i+1];
      }

      if(print)
      {
        if(onePage)
        {
          PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
          if(antialias)
            pai.setAntiAlias(true);
          pai.print(nresiduesPerLine,type,prefix,                               
                    lmargin,rmargin,tmargin,bmargin);
        }
        else
        {
          PrintAlignmentImage pai = new PrintAlignmentImage(gsc);
          if(antialias)
            pai.setAntiAlias(true);
          pai.print(nresiduesPerLine,type,prefix,landscape,
                    lmargin,rmargin,tmargin,bmargin);
        }
      }
      if(!show)
        System.exit(0);
    }
    else
    {
      ajFrame = new AlignJFrame(true);
    }
    ajFrame.setVisible(true);
  }


}

