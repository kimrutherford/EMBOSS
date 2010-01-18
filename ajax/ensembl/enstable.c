/******************************************************************************
** @source Ensembl Table functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.3 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "enstable.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection enstable *****************************************************
**
** @nam1rule ens Function belongs to the AJAX Ensembl library
** @nam2rule Table Ensembl Table objects
**
******************************************************************************/

/* @datasection [none] Ensembl Table **************************************
**
** Functions for Ensembl Tables
**
** @nam2rule Table Ensembl function for AJAX tables.
**
******************************************************************************/

/* @section functions *********************************************************
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule Cmp Comparison function
** @nam3rule Hash Hash function
** @nam4rule Uint Unsigned integer table key
**
** @argrule Cmp x [const void*] Unsigned integer first key
** @argrule Cmp y [const void*] Unsigned integer ssecond key
** @argrule Hash key [const void*] Unsigned integer key
** @argrule Hash hashsize [ajuint] Hash table size
**
** @valrule Cmp [ajint] 0 for a match, -1 or +1 for one key greater
** @valrule Hash [ajuint] Unsigned integer hash value
**
******************************************************************************/



/* @func ensTableCmpUint ******************************************************
**
** AJAX Table function to compare AJAX unsigned integer (ajuint)
** hash key values.
**
** @param [r] x [const void*] AJAX unsigned integer value address
** @param [r] y [const void*] AJAX unsigned integer value address
**
** @return [ajint] 0 if the values are identical, 1 if they are not
** @@
******************************************************************************/

ajint ensTableCmpUint(const void *x, const void *y)
{
    const ajuint *a = NULL;
    const ajuint *b = NULL;
    
    a = (const ajuint *) x;
    b = (const ajuint *) y;
    
    /*
     ajDebug("ensTableCmpUint *a %u *b %u result %d\n", *a, *b, (*a != *b));
     */
    
    return (*a != *b);
}




/* @func ensTableHashUint *****************************************************
**
** AJAX Table function to handle AJAX unsigned integer (ajuint)
** hash key values.
**
** @param [r] key [const void*] AJAX unsigned integer key value address
** @param [r] hashsize [ajuint] Hash size (maximum hash value)
**
** @return [ajuint] Hash value
** @@
******************************************************************************/

ajuint ensTableHashUint(const void *key, ajuint hashsize)
{
    const ajuint *a = NULL;
    
    if(!key)
	return 0;
    
    if(!hashsize)
	return 0;
    
    a = (const ajuint *) key;
    
    /*
     ajDebug("ensTableHashUint result %u\n", ((*a >> 2) % hashsize));
     */
    
    return ((*a >> 2) % hashsize);
}
