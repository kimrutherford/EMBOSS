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
import java.awt.Dimension;
import java.awt.Color;
import java.awt.event.*;

import org.emboss.jemboss.gui.form.TextFieldInt;
import org.emboss.jemboss.gui.form.TextFieldFloat;
import org.emboss.jemboss.gui.form.LabelTextBox;


/**
*
* JFrame to define the colour of residue if identical or 
* a positive match. 
*
*/
public class PrettyPlotJFrame extends JFrame
                              implements ActionListener
{

  /** field to define min number of identities */
  private TextFieldInt textInt;
  /** field to define threshold for positive matches */
  private TextFieldFloat textFloat;
  /** colour panel for identical matches */
  private JColorChooser idColour;
  /** colour panel for positive scoring matches */
  private JColorChooser matchColour;
  /** colour panel for identical matches background */
  private JColorChooser idColourBackground;
  /** colour panel for positive scoring matches background */
  private JColorChooser matchColourBackground;
  /** graphic sequence panel */
  private GraphicSequenceCollection gsc;
  /** define if the identities and matches are to be boxed */
  private JCheckBox prettyBox;


  /**
  *
  * @param 
  *
  */
  public PrettyPlotJFrame(int minID, float plurality, 
                          Color colID, Color colMatch,
                          Color colIDBack, Color colMatchBack,
                          boolean lboxPretty)
  {
    textInt = new TextFieldInt();
    textInt.setText(Integer.toString(minID));
    textFloat = new TextFieldFloat();
    textFloat.setValue(plurality);
    idColour = new JColorChooser(colID);
    matchColour = new JColorChooser(colMatch);
    idColourBackground = new JColorChooser(colIDBack);
    matchColourBackground = new JColorChooser(colMatchBack);
    prettyBox = new JCheckBox();
    prettyBox.setSelected(lboxPretty);
  }

  /**
  *
  * @param gsc	graphic sequence panel
  *
  */
  public PrettyPlotJFrame(GraphicSequenceCollection gsc)
  {
    super("Colour Matches");

    this.gsc = gsc;
    Box bdown = Box.createVerticalBox();
    bdown.add(Box.createVerticalStrut(4));

    Box bacross = Box.createHorizontalBox();
    textInt = new TextFieldInt();
    textInt.setText(Integer.toString(gsc.getNumberSequences()));
    Dimension d = new Dimension(50, 30);
    textInt.setPreferredSize(d);
    textInt.setMaximumSize(d);
    bacross.add(textInt);
    LabelTextBox idLabel = new LabelTextBox(
                "Identity Number",
                "Minimum number of identities in a column");

    bacross.add(idLabel);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
    bdown.add(Box.createVerticalStrut(4));

// match threshold
   
    bacross = Box.createHorizontalBox();
    textFloat = new TextFieldFloat();
    textFloat.setValue(AlignJFrame.getTotalWeight(
                       gsc.getSequenceCollection())/2);
    textFloat.setPreferredSize(d);
    textFloat.setMinimumSize(d);
    bacross.add(textFloat);
    LabelTextBox floatLabel = new LabelTextBox(
                "Threshold for positive matches",
                "Minimum number of positive matches. The default\n"+
                "is half the total weight of all the sequences.");

    bacross.add(floatLabel);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
    bdown.add(Box.createVerticalStrut(4));

// identity colour
    bacross = Box.createHorizontalBox();
    idColour = new JColorChooser(Color.red);
    final JButton idButton = new JButton("");   
    Dimension buttSize = new Dimension(25,20);
    idButton.setPreferredSize(buttSize);
    final JDialog dialog = JColorChooser.createDialog(idButton,
                                        "Pick a Color",
                                        true,
                                        idColour,
                                        null, null);

    idButton.setBackground(Color.red);
    idButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        dialog.show();
        idButton.setBackground(idColour.getColor());
      }
    });
    bacross.add(idButton);
    idLabel = new LabelTextBox(
                "Identity Colour","");
    bacross.add(idLabel);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
    bdown.add(Box.createVerticalStrut(4));

// identity colour background
    bacross = Box.createHorizontalBox();
    idColourBackground = new JColorChooser(Color.white);
    final JButton idBackButton = new JButton("");
    idBackButton.setPreferredSize(buttSize);
    final JDialog dialogID = JColorChooser.createDialog(idBackButton,
                                        "Pick a Color",
                                        true,
                                        idColourBackground,
                                        null, null);
                                                                                             
    idBackButton.setBackground(Color.white);                                                       
    idBackButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        dialogID.show();
        idBackButton.setBackground(idColourBackground.getColor());
      }
    });
    bacross.add(idBackButton);
    idLabel = new LabelTextBox(
                "Identity Background Colour","");
    bacross.add(idLabel);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
    bdown.add(Box.createVerticalStrut(4));

// positive matches colour
    bacross = Box.createHorizontalBox();
    matchColour = new JColorChooser(Color.blue);
    final JButton matchButton = new JButton("");
    matchButton.setPreferredSize(buttSize);
    final JDialog dialogMatch = JColorChooser.createDialog(matchButton,
                                        "Pick a Color",
                                        true,
                                        matchColour,
                                        null, null);
                                                                                             
    matchButton.setBackground(Color.blue);
    matchButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        dialogMatch.show();
        matchButton.setBackground(matchColour.getColor());
      }
    });
    bacross.add(matchButton);
    idLabel = new LabelTextBox(
                "Positive Match Colour",
                "");
    bacross.add(idLabel);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

// positive matches colour background
    bacross = Box.createHorizontalBox();
    matchColourBackground = new JColorChooser(Color.white);
    final JButton matchBackButton = new JButton("");
    matchBackButton.setPreferredSize(buttSize);
    final JDialog dialogMatchBack = JColorChooser.createDialog(matchButton,
                                        "Pick a Color",
                                        true,
                                        matchColourBackground,
                                        null, null);
                                                                                             
    matchBackButton.setBackground(Color.white);
    matchBackButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        dialogMatchBack.show();
        matchBackButton.setBackground(matchColourBackground.getColor());
      }
    });
    bacross.add(matchBackButton);
    idLabel = new LabelTextBox(
                "Positive Match Background Colour",
                "");
    bacross.add(idLabel);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);
 
// box-in the identical and similar matches
    bacross = Box.createHorizontalBox();
    prettyBox = new JCheckBox("Box");
    prettyBox.setSelected(true);
    bacross.add(prettyBox);
    bacross.add(Box.createHorizontalGlue());
    bdown.add(bacross);

    bacross = Box.createHorizontalBox();
    JButton draw = new JButton("Set");
    draw.addActionListener(this);
    draw.setActionCommand("SET");
    bacross.add(draw);
    bdown.add(bacross);
    
// set up a menu bar
    JMenuBar menuBar = new JMenuBar();

// File menu
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

// exit
    fileMenu.add(new JSeparator());
    JMenuItem fileMenuExit = new JMenuItem("Close");
    fileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(fileMenuExit);
    menuBar.add(fileMenu);
    setJMenuBar(menuBar);

    getContentPane().add(bdown);
    pack();
  }


  /**
  *
  * Get the users defined identity limit
  * @param	number of identical residues
  *
  */
  public int getMinimumIdentity(int nseqs)
  {
    return Integer.parseInt(textInt.getText());
  }


  /**
  *
  * Get the match threshold value
  * @return 	threshold value
  *
  */
  public double getMatchThreshold()
  {
    return textFloat.getValue();
  }


  /**
  *
  * Get the users defined colour to draw identical
  * residues
  * @return 	colour
  *
  */
  public Color getIDColour()
  {
    return idColour.getColor();
  }


  /**
  *
  * Get the users defined background colour to draw identical
  * residues
  * @return     colour
  *
  */
  public Color getIDBackgroundColour()
  {
    return idColourBackground.getColor();
  }


  /**
  *
  * Get the users defined colour to draw positive
  * match residues
  * @return     colour
  *
  */
  public Color getMatchColour()
  {
    return matchColour.getColor();
  }


  /**
  *
  * Get the users defined background colour to draw positive
  * match residues
  * @return     colour
  *
  */
  public Color getMatchBackgroundColour()
  {
    return matchColourBackground.getColor();
  }


  /**
  *
  * Determine if boxes are to be drawn around the 
  * identical matches
  * @param	draw boxes if true
  *
  */
  protected boolean isPrettyBox()
  {
    return prettyBox.isSelected();
  }

   
  /**
  *
  * Set the current graphic display
  * @param gsc	Sequence graphic     
  *
  */
  protected void setGraphicSequenceCollection(GraphicSequenceCollection gsc)
  {
    this.gsc = gsc;
    textInt.setText(Integer.toString(gsc.getNumberSequences()));
  }

  /**
  *
  *  @param ae	the action event generated
  *
  */
  public void actionPerformed(ActionEvent ae)
  {
    if(ae.getActionCommand().equals("SET"))
    {
      gsc.setPrettyPlot(true,this);
      gsc.setDrawBoxes(false);
      gsc.setDrawColor(false);
    }
  }

}

