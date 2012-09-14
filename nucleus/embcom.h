/* @include embcom ************************************************************
**
** General routines for program Complex
**
** NB: THESE ROUTINES DO NOT CONFORM TO THE LIBRARY WRITING STANDARD AND
**     THEREFORE SHOULD NOT BE USED AS A TEMPLATE FOR WRITING EMBOSS CODE
**
** @author Copyright (c) 1999 Donata Colangelo
** @version $Revision: 1.11 $
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

#ifndef EMBCOM_H
#define EMBCOM_H


/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajfile.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data EmbPComTrace *********************************************************
**
** Complex utility trace object
**
** @attr ind [ajint] Index
** @attr pc [float] Percent
******************************************************************************/

typedef struct EmbSComTrace {
    ajint ind;
    float pc;
} EmbOComTrace;
#define EmbPComTrace EmbOComTrace*




/* @data EmbPComUjwin *********************************************************
**
** Complex utility UJwin object
**
** @attr Ujwin [float*] UJ windows
******************************************************************************/

typedef struct EmbSComUjwin {
  float *Ujwin;
} EmbOComUjwin;
#define EmbPComUjwin EmbOComUjwin*




/* @data EmbPComUjsim *********************************************************
**
** Complex utility UJsim object
**
** @attr Ujsim [EmbPComUjwin] UJ sim
******************************************************************************/

typedef struct EmbSComUjsim {
  EmbPComUjwin Ujsim;
} EmbOComUjsim;
#define EmbPComUjsim EmbOComUjsim*




/* @data EmbPComSeqsim ********************************************************
**
** Complex utility SEQsim object
**
** @attr Sqsim [char*] Sequence sim
******************************************************************************/

typedef struct EmbSComSeqsim {
  char *Sqsim;
} EmbOComSeqsim;
#define EmbPComSeqsim EmbOComSeqsim*



/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

void embComComplexity(const char *seq,const char *name,
		      ajint len, ajint jmin, ajint jmax,
		      ajint l, ajint step, ajint sim, ajint freq, ajint omnia,
		      AjPFile fp, AjPFile pf,
		      ajint print, float *MedValue);
void embComWriteValueOfSeq(AjPFile fp, ajint n,const char *name, ajint len,
			   float MedValue);
void embComWriteFile(AjPFile fp, ajint jmin, ajint jmax, ajint lwin,
		     ajint step, ajint sim);
void embComUnused (void);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBCOM_H */
