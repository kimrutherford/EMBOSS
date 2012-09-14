/* @include ajcigar ************************************************************
**
** Handling of CIGAR strings
**
** @version $Revision: 1.5 $
** @modified Nov 2011 uludag First version
** @modified $Date: 2012/07/14 14:52:39 $ by $Author: rice $
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

#ifndef AJCIGAR_H
#define AJCIGAR_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */


typedef struct AjSCigar
{
    ajint* length;
    char* cigoperator;
    ajint n;
    ajuint Padding;
} AjOCigar;

#define AjPCigar AjOCigar*



/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjPCigar ajCigarNewS(const AjPStr cigar);
void     ajCigarDel(AjPCigar *Pcigar);
ajint	 ajCigarGetReferenceLength(const AjPCigar c);
ajint    ajCigarCountInsertedBases(const AjPCigar c);
ajint    ajCigarCountDeletedBases(const AjPCigar c);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJIGAR_H */
