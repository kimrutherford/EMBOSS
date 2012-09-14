/* @include embiep ************************************************************
**
** Isoelectric point routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.15 $
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

#ifndef EMBIEP_H
#define EMBIEP_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define EMBIEPSIZE 28			/* Usual alpha plus two array */
#define EMBIEPCYSTEINE 2	      /* Array index for C */
#define EMBIEPLYSINE 10		      /* Array index for K */
#define EMBIEPAMINO 26			/* Amino terminus array index  */
#define EMBIEPCARBOXYL 27		/* Carboxyl terminus array index */



/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

void   embIepCalcK (double *K, double *pK);
double *embIeppKNew (void);
void   embIeppKDel (double *pK);
void   embIepCompC (const char *s, ajint amino, ajint carboxyl,
		    ajint sscount, ajint modlysine,
		    ajint *c);
void   embIepCompS (const AjPStr str, ajint amino, ajint carboxyl,
		    ajint sscount, ajint modlysine,
		    ajint *c);
double embIepGetCharge (const ajint *c, const double *pro, double *total);
void   embIepGetProto (const double *K, const ajint *c,
		       ajint *op, double H, double *pro);
AjBool embIepIepC (const char *s, ajint amino, ajint carboxyl,
		   ajint sscount, ajint modlysine,
		   double *pK, double *iep, AjBool termini);
AjBool embIepIepS (const AjPStr str, ajint amino, ajint carboxyl,
		   ajint sscount, ajint modlysine,
		   double *pK, double *iep, AjBool termini);
double embIepPhFromHconc (double H);
double embIepPhToHconc (double pH);
double embIepPhConverge (const ajint *c, const double *K,
			 ajint *op, double *pro);
void   embIepPkRead (double *pK);
double embIepPkToK (double pK);
double embIepPkFromK (double K);

/*
** End of prototype definitions
*/

#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif
#ifdef AJ_COMPILE_DEPRECATED

__deprecated void    embIepComp (const char *s, ajint amino, ajint carboxyl,
                                 ajint *c);
__deprecated AjBool  embIepIEP (const char *s, ajint amino, ajint carboxyl,
                                double *pK, double *iep, AjBool termini);

#endif

AJ_END_DECLS

#endif  /* !EMBIEP_H */
