/* @include embmol ************************************************************
**
** Routines for molecular weight matching.
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.13 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
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

#ifndef EMBMOL_H
#define EMBMOL_H


/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "embprop.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define EMBMOLPARDISP (double)1000000.0




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data EmbPMolFrag **********************************************************
**
** Nucleus sequence molecular fragment object.
**
** @attr begin [ajint] Start
** @attr end [ajint] End
** @attr mwt [double] Molecular weight
** @@
******************************************************************************/

typedef struct EmbSMolFrag
{
    ajint begin;
    ajint end;
    double mwt;
} EmbOMolFrag;
#define EmbPMolFrag EmbOMolFrag*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

ajint embMolGetFrags(const AjPStr thys, ajint rno, EmbPPropMolwt const *mwdata,
		     AjBool mono, AjPList *l);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBMOL_H */
