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


import java.util.*;
import java.awt.Color;

/**
*
* General sequence properties
*
*/
public class SequenceProperties 
{

  /** Taylor colour scheme, W.R.Taylor Protein 
            Eng vol.10,  7 pp743-746, 1997 */
  public static Hashtable taylorColor = new Hashtable();

  /** AQUAMARINE from EMBOSS data Ecolour.map */
  public static Color AQUAMARINE = Color.decode("#99CCFF");
  /** WHEAT from EMBOSS data Ecolour.map      */
  public static Color WHEAT      = Color.decode("#FFFFCC");
  /** PINK from EMBOSS data Ecolour.map       */
  public static Color PINK       = Color.decode("#FFCCCC");
  /** BROWN from EMBOSS data Ecolour.map      */
  public static Color BROWN      = Color.decode("#993300");
  /** BLUEVIOLET from EMBOSS data Ecolour.map */
  public static Color BLUEVIOLET = Color.decode("#9933FF");
  /** TURQUOISE from EMBOSS data Ecolour.map  */
  public static Color TURQUOISE  = Color.decode("#33FFCC");
  /** MAGENTA from EMBOSS data Ecolour.map    */
  public static Color MAGENTA    = Color.decode("#FF00FF");
  /** SALMON from EMBOSS data Ecolour.map     */
  public static Color SALMON     = Color.decode("#FF9966");

//
// W.R.Taylor Protein Eng. vol.10 no. 7 pp743-746, 1997
//
  static
  {
    taylorColor.put("Q",MAGENTA);  // Glutamine  (magenta)
    taylorColor.put("E",new Color((float)1.0,(float)0.0,(float)0.4));  // Glutamate  (violet)
    taylorColor.put("D",new Color((float)1.0,(float)0.0,(float)0.0));  // Aspartate  (red)
    taylorColor.put("S",new Color((float)1.0,(float)0.2,(float)0.0));  // Serine     (scarlet)
    taylorColor.put("T",new Color((float)1.0,(float)0.4,(float)0.0));  // Threonine  (vermillion)
    taylorColor.put("G",new Color((float)1.0,(float)0.6,(float)0.0));  // Glycine    (orange)
    taylorColor.put("P",new Color((float)1.0,(float)0.8,(float)0.0));  // Proline    (tangerine)
    taylorColor.put("C",Color.yellow);                                 // Cystine    (yellow)
    taylorColor.put("A",new Color((float)0.8,(float)1.0,(float)0.0));  // Alanine    (lemon)
    taylorColor.put("V",new Color((float)0.6,(float)1.0,(float)0.0));  // Valine     (lemon-lime)
    taylorColor.put("I",new Color((float)0.4,(float)1.0,(float)0.0));  // Isoleucine (lime)
    taylorColor.put("L",new Color((float)0.2,(float)1.0,(float)0.0));  // Leucine    (grass)
    taylorColor.put("M",Color.green);                                  // Methionine (green)
    taylorColor.put("F",new Color((float)0.0,(float)1.0,(float)0.4));  // Phenylalanine (emarald)
    taylorColor.put("Y",new Color((float)0.0,(float)1.0,(float)0.8));  // Tyrosine   (turquoise)
    taylorColor.put("W",Color.cyan);                                   // Tryptophan (cyan)
    taylorColor.put("H",new Color((float)0.0,(float)0.4,(float)1.0));  // Histidine  (peacock)
    taylorColor.put("R",Color.blue);                                   // Arginine   (blue)
    taylorColor.put("K",new Color((float)0.4,(float)0.0,(float)1.0));  // Lysine     (indigo)
    taylorColor.put("N",new Color((float)0.8,(float)0.0,(float)1.0));  // Asparagine (purple)
    taylorColor.put("B",Color.white);  // Aspartic, Asparagine
    taylorColor.put("Z",Color.white);  // Glutamic, Glutamine
    taylorColor.put("X",Color.white);  // Unknown
    taylorColor.put("-",Color.white);  // 
    taylorColor.put("*",Color.white);  // Terminator
    taylorColor.put(".",Color.white);  
  }
 
  public static Hashtable residueColor = new Hashtable();
  static
  {
    residueColor.put("A",Color.white);
    residueColor.put("B",Color.white);
    residueColor.put("C",Color.yellow);
    residueColor.put("D",Color.red);
    residueColor.put("E",Color.red);
    residueColor.put("F",BROWN);  
    residueColor.put("G",Color.gray);
    residueColor.put("H",Color.blue);
    residueColor.put("I",Color.white);
    residueColor.put("J",Color.white);
    residueColor.put("K",Color.blue);
    residueColor.put("L",Color.white);
    residueColor.put("M",Color.yellow);
    residueColor.put("N",Color.green);
    residueColor.put("O",Color.white);
    residueColor.put("P",new Color(219,112,147)); //violet
    residueColor.put("Q",Color.green);
    residueColor.put("R",Color.blue);
    residueColor.put("S",Color.cyan);
    residueColor.put("T",Color.cyan);
    residueColor.put("U",Color.white);
    residueColor.put("V",Color.white);
    residueColor.put("W",new Color(161,120,41)); //brown
    residueColor.put("X",Color.white);
    residueColor.put("Y",new Color(161,120,41)); //brown
    residueColor.put("Z",Color.white);
    residueColor.put("-",Color.white);
    residueColor.put("*",Color.white);
    residueColor.put(".",Color.white);
  }

  /** rasmol colour scheme */
  public static Hashtable rasmolColor = new Hashtable();
  static
  {
    rasmolColor.put("D",Color.red);
    rasmolColor.put("E",Color.red);
    rasmolColor.put("F",Color.blue);
    rasmolColor.put("Y",Color.blue);
    rasmolColor.put("K",BLUEVIOLET); 
    rasmolColor.put("R",BLUEVIOLET);
    rasmolColor.put("G",Color.gray);
    rasmolColor.put("A",BROWN);      //brown
    rasmolColor.put("H",AQUAMARINE);
    rasmolColor.put("C",Color.yellow);
    rasmolColor.put("M",Color.yellow);
    rasmolColor.put("U",Color.yellow);
    rasmolColor.put("S",SALMON);
    rasmolColor.put("T",SALMON);
    rasmolColor.put("N",Color.cyan);
    rasmolColor.put("Q",Color.cyan);
    rasmolColor.put("L",Color.green);
    rasmolColor.put("V",Color.green);
    rasmolColor.put("I",Color.green);
    rasmolColor.put("W",PINK);
    rasmolColor.put("P",WHEAT);
  }

  /** Acidic/Basic (Red=acidic, Blue=basic) */
  public static Hashtable acidColor = new Hashtable();
  static
  {
    acidColor.put("D",Color.red);
    acidColor.put("E",Color.red);
    acidColor.put("R",Color.blue);
    acidColor.put("K",Color.blue);
    acidColor.put("H",Color.blue);
  }

  /** Polar (Red=Polar) */
  public static Hashtable polarColor = new Hashtable();
  static
  {
    polarColor.put("R",Color.red);
    polarColor.put("N",Color.red);
    polarColor.put("D",Color.red);
    polarColor.put("Q",Color.red);
    polarColor.put("E",Color.red);
    polarColor.put("H",Color.red);
    polarColor.put("K",Color.red);
    polarColor.put("S",Color.red);
    polarColor.put("T",Color.red);
    polarColor.put("W",Color.red);
    polarColor.put("Y",Color.red);
  }

  /** Hydrophobic (Red=Hydrophobic) */
  public static Hashtable hydrophobicColor = new Hashtable();
  static
  {
    hydrophobicColor.put("A",Color.red);
    hydrophobicColor.put("C",Color.red);
    hydrophobicColor.put("G",Color.red);
    hydrophobicColor.put("I",Color.red);
    hydrophobicColor.put("L",Color.red);
    hydrophobicColor.put("M",Color.red);
    hydrophobicColor.put("F",Color.red);
    hydrophobicColor.put("P",Color.red);
    hydrophobicColor.put("S",Color.red);
    hydrophobicColor.put("T",Color.red);
    hydrophobicColor.put("W",Color.red);
    hydrophobicColor.put("Y",Color.red);
    hydrophobicColor.put("V",Color.red);
  }

  /** Aromatic/Aliphatic (Red=Aromatic, Blue=Aliphatic) */
  public static Hashtable aromaticColor = new Hashtable();
  static
  {
    aromaticColor.put("H",Color.red);
    aromaticColor.put("F",Color.red);
    aromaticColor.put("W",Color.red);
    aromaticColor.put("Y",Color.red);
    aromaticColor.put("I",Color.blue);
    aromaticColor.put("L",Color.blue);
    aromaticColor.put("V",Color.blue);
  }

  /** Surface/Buried (Red=Surface, Blue=Buried) */
  public static Hashtable surfaceColor = new Hashtable();
  static
  {
    surfaceColor.put("R",Color.red);
    surfaceColor.put("N",Color.red);
    surfaceColor.put("D",Color.red);
    surfaceColor.put("E",Color.red);
    surfaceColor.put("Q",Color.red);
    surfaceColor.put("G",Color.red);
    surfaceColor.put("H",Color.red);
    surfaceColor.put("K",Color.red);
    surfaceColor.put("P",Color.red);
    surfaceColor.put("S",Color.red);
    surfaceColor.put("T",Color.red);
    surfaceColor.put("Y",Color.red);
    surfaceColor.put("A",Color.blue);
    surfaceColor.put("C",Color.blue);
    surfaceColor.put("I",Color.blue);
    surfaceColor.put("L",Color.blue);
    surfaceColor.put("M",Color.blue);
    surfaceColor.put("F",Color.blue);
    surfaceColor.put("W",Color.blue);
    surfaceColor.put("V",Color.blue);
  }

  /** Positive/Negative charge (Red=Positive, Blue=Negative) */
  public static Hashtable chargeColor = new Hashtable();
  static
  {
    chargeColor.put("R",Color.red);
    chargeColor.put("H",Color.red);
    chargeColor.put("K",Color.red);
    chargeColor.put("D",Color.blue);
    chargeColor.put("E",Color.blue);
  }

  /** Tiny/Small/Large (Red=Tiny, Green=Small, Blue=Large) */
  public static Hashtable sizeColor = new Hashtable();
  static
  {
    sizeColor.put("A",Color.red);
    sizeColor.put("G",Color.red);
    sizeColor.put("S",Color.red);
    
    sizeColor.put("N",Color.green);
    sizeColor.put("D",Color.green);
    sizeColor.put("C",Color.green);
    sizeColor.put("P",Color.green);
    sizeColor.put("T",Color.green);
    sizeColor.put("V",Color.green);
    
    sizeColor.put("R",Color.blue);
    sizeColor.put("E",Color.blue);
    sizeColor.put("Q",Color.blue);
    sizeColor.put("H",Color.blue);
    sizeColor.put("I",Color.blue);
    sizeColor.put("L",Color.blue);
    sizeColor.put("K",Color.blue);
    sizeColor.put("M",Color.blue);
    sizeColor.put("F",Color.blue);
    sizeColor.put("W",Color.blue);
    sizeColor.put("Y",Color.blue);
  }

  /** nucletide colour scheme */
  public static Hashtable baseColor = new Hashtable();
  static
  {
    baseColor = new Hashtable();
    baseColor.put("A",Color.red);
    baseColor.put("T",Color.blue);
    baseColor.put("U",Color.blue);
    baseColor.put("C",Color.green);
    baseColor.put("G",Color.yellow);
    baseColor.put("N",Color.black);
  }


}

