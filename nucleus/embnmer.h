/* @include embnmer ***********************************************************
**
** General routines for n-mer alignment.
**
** @author Copyright (c) 1999 Gary Williams
** @version $Revision: 1.8 $
** @modified $Date: 2011/10/18 14:24:25 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef EMBNMER_H
#define EMBNMER_H

/*
* This set of routines was written primarily for the compseq program.
* Feel free to use it for anything you want.
*
* compseq counts the frequency of n-mers (or words) in a sequence.
*
* The easiest way to do this was to make a big ajulong array
* to hold the counts of each type of word.
*
* I needed a way of converting a sequence word into an integer so that I
* could increment the appropriate element of the array.
* (embNmerNuc2int and embNmerProt2int)
*
* I also needed a way of converting an integer back to a short sequence
* so that I could display the word.
* (embNmerInt2nuc and embNmerInt2prot)
*
* embNmerGetNoElements returns the number of elements required to store the
* results.
* In other words it is the number of possible words of size n.
*
* Gary Williams
*
*/





/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

ajulong embNmerNuc2int (const char *seq, ajint wordsize, ajint offset,
			AjBool *otherflag);
ajint   embNmerInt2nuc (AjPStr *seq, ajint wordsize,
			ajulong value);
ajulong embNmerProt2int (const char *seq, ajint wordsize, ajint offset,
			 AjBool *otherflag, AjBool ignorebz);
ajint   embNmerInt2prot (AjPStr *seq, ajint wordsize,
			ajulong value, AjBool ignorebz);
AjBool  embNmerGetNoElements (ajulong *no_elements,
			      ajint word, AjBool seqisnuc,
			      AjBool ignorebz);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBNMER_H */
