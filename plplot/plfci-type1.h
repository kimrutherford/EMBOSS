/*
  This header file contains the lookup table used for converting between
  FCIs (font characterization integers) and font names for the standard
  35 type 1 fonts.

  Copyright (C) 2005  Alan W. Irwin

  This file is part of PLplot.
  
  PLplot is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Library Public License as published
  by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  PLplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with PLplot; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

/* There are no good choices for script fonts for Type 1 so I just
 * defaulted to the Times variants in that case. */

/* There is only one Type 1 symbol font in the standard 35 so use
 * it for all variations. */

/* N.B. When updating this table by hand be sure to keep it in 
 * ascending order in fci! 
 */
#define N_Type1Lookup 30
const FCI_to_FontName_Table Type1Lookup[N_Type1Lookup] = {
     {0, PL_FCI_MARK | 0x000, (const unsigned char *)"Helvetica"},
     {0, PL_FCI_MARK | 0x001, (const unsigned char *)"Times-Roman"},
     {0, PL_FCI_MARK | 0x002, (const unsigned char *)"Courier"},
     {0, PL_FCI_MARK | 0x003, (const unsigned char *)"Times-Roman"},
     {0, PL_FCI_MARK | 0x004, (const unsigned char *)"Symbol"},
     {0, PL_FCI_MARK | 0x010, (const unsigned char *)"Helvetica-Oblique"},
     {0, PL_FCI_MARK | 0x011, (const unsigned char *)"Times-Italic"},
     {0, PL_FCI_MARK | 0x012, (const unsigned char *)"Courier-Oblique"},
     {0, PL_FCI_MARK | 0x013, (const unsigned char *)"Times-Italic"},
     {0, PL_FCI_MARK | 0x014, (const unsigned char *)"Symbol"},
     {0, PL_FCI_MARK | 0x020, (const unsigned char *)"Helvetica-Oblique"},
     {0, PL_FCI_MARK | 0x021, (const unsigned char *)"Times-Italic"},
     {0, PL_FCI_MARK | 0x022, (const unsigned char *)"Courier-Oblique"},
     {0, PL_FCI_MARK | 0x023, (const unsigned char *)"Times-Italic"},
     {0, PL_FCI_MARK | 0x024, (const unsigned char *)"Symbol"},
     {0, PL_FCI_MARK | 0x100, (const unsigned char *)"Helvetica-Bold"},
     {0, PL_FCI_MARK | 0x101, (const unsigned char *)"Times-Bold"},
     {0, PL_FCI_MARK | 0x102, (const unsigned char *)"Courier-Bold"},
     {0, PL_FCI_MARK | 0x103, (const unsigned char *)"Times-Bold"},
     {0, PL_FCI_MARK | 0x104, (const unsigned char *)"Symbol"},
     {0, PL_FCI_MARK | 0x110, (const unsigned char *)"Helvetica-BoldOblique"},
     {0, PL_FCI_MARK | 0x111, (const unsigned char *)"Times-BoldItalic"},
     {0, PL_FCI_MARK | 0x112, (const unsigned char *)"Courier-BoldOblique"},
     {0, PL_FCI_MARK | 0x113, (const unsigned char *)"Times-BoldItalic"},
     {0, PL_FCI_MARK | 0x114, (const unsigned char *)"Symbol"},
     {0, PL_FCI_MARK | 0x120, (const unsigned char *)"Helvetica-BoldOblique"},
     {0, PL_FCI_MARK | 0x121, (const unsigned char *)"Times-BoldItalic"},
     {0, PL_FCI_MARK | 0x122, (const unsigned char *)"Courier-BoldOblique"},
     {0, PL_FCI_MARK | 0x123, (const unsigned char *)"Times-BoldItalic"},
     {0, PL_FCI_MARK | 0x124, (const unsigned char *)"Symbol"}
};
