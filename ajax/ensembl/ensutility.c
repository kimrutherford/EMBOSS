/******************************************************************************
** @source Ensembl Utility functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.2 $
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

#include "ensutility.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */




/* @filesection ensutility ****************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/

/* @datasection [none] Internals **********************************************
**
** Functions contorl ensembl internals
**
******************************************************************************/

/* @section Initialise ********************************************************
**
** @fdata [none]
**
** @nam2rule Init Initialise ensembl internals
**
** @valrule * [void]
**
** @fcategory internals
**
******************************************************************************/


/* @func ensInit **************************************************************
**
** Initialises the Ensembl library.
**
** @return [void]
** @@
******************************************************************************/

void ensInit(void)
{
    ensRegistryInit();
    
    ensTranslationInit();
    
    return;
}




/* @section Exit **************************************************************
**
** @fdata [none]
**
** @nam2rule Exit Clear ensembl internals
**
** @valrule * [void]
**
** @fcategory internals
**
******************************************************************************/


/* @func ensExit **************************************************************
**
** Frees the Ensembl library.
**
** @return [void]
** @@
******************************************************************************/

void ensExit(void)
{
    ensRegistryExit();
    
    ensTranslationExit();
    
    return;
}
