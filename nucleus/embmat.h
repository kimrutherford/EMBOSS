/* @include embmat ************************************************************
**
** General match routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.17 $
** @modified $Date: 2013/03/05 23:08:59 $ by $Author: rice $
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

#ifndef EMBMAT_H
#define EMBMAT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajmem.h"
#include "ajmess.h"
#include "ajstr.h"
#include "ajsys.h"
#include "ajbase.h"
#include "ajfileio.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define PRINTS_MAT "PRINTS/prints.mat"

typedef ajuint *PMAT_INT[26];




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data EmbPMatPrints ********************************************************
**
** NUCLEUS data structure for PRINTS protein fingerprints
**
** @attr cod [AjPStr] gc line
** @attr acc [AjPStr] gx line
** @attr tit [AjPStr] gt line
** @attr len [ajuint*] Lengths of motifs
** @attr thresh [ajuint*] % of maximum score for matrix
** @attr max [ajuint*] Maximum score for matrix
** @attr matrix [PMAT_INT*] Matrices
** @attr n [ajuint] Number of motifs in fingerprint
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSMatPrints
{
    AjPStr cod;
    AjPStr acc;
    AjPStr tit;
    ajuint    *len;
    ajuint    *thresh;
    ajuint    *max;
    PMAT_INT  *matrix;
    ajuint    n;
    char      Padding[4];
} EmbOMatPrint;
#define EmbPMatPrints EmbOMatPrint*




/* @data EmbPMatMatch *********************************************************
**
** NUCLEUS data structure for sequence matrix matches
**
** @attr seqname [AjPStr] Sequence name
** @attr cod [AjPStr] Matrix name
** @attr acc [AjPStr] Matrix accession number
** @attr tit [AjPStr] Matrix title
** @attr pat [AjPStr] Pattern
** @attr n [ajuint] Number of motifs in fingerprint
** @attr len [ajuint] Lengths of motifs
** @attr thresh [ajuint] % of maximum score for matrix
** @attr max [ajuint] Maximum score for matrix
** @attr element [ajuint] Number of matching element
** @attr start [ajuint] Start of match
** @attr end [ajuint] End of match
** @attr score [ajuint] Score of match
** @attr hpe [ajuint] Hits per element (so far)
** @attr hpm [ajuint] Hits per motif (so far)
** @attr all [AjBool] Can be set if all elements match
** @attr ordered [AjBool] Can be set if "all" and in order
** @attr forward [AjBool] on forward strand
** @attr mm [ajuint] Number of mismatches
** @attr cut1 [ajint] First 5' cut
** @attr cut2 [ajint] First 3' cut
** @attr cut3 [ajint] Reverse 5' cut
** @attr cut4 [ajint] Reverse 3' cut
** @attr len1 [ajuint] Fragment length 5'
** @attr len2 [ajuint] Fragment length 3'
** @attr len3 [ajuint] Fragment length reverse 5'
** @attr len4 [ajuint] Fragment length reverse 3'
** @attr circ12 [AjBool] Circular for cut1 and/or cut2
** @attr circ34 [AjBool] Circular for cut3 and/or cut4
** @attr iso [AjPStr] Holds names of isoschizomers
** @@
******************************************************************************/

typedef struct EmbSMatMatch
{
    AjPStr seqname;
    AjPStr cod;
    AjPStr acc;
    AjPStr tit;
    AjPStr pat;
    ajuint  n;
    ajuint  len;
    ajuint  thresh;
    ajuint  max;
    ajuint  element;
    ajuint  start;
    ajuint  end;
    ajuint  score;
    ajuint  hpe;
    ajuint  hpm;
    AjBool all;
    AjBool ordered;
    AjBool forward;
    ajuint  mm;
    ajint  cut1;
    ajint  cut2;
    ajint  cut3;
    ajint  cut4;
    ajuint len1;
    ajuint len2;
    ajuint len3;
    ajuint len4;
    AjBool circ12;
    AjBool circ34;
    AjPStr iso;
} EmbOMatMatch;
#define EmbPMatMatch EmbOMatMatch*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

void   embMatMatchDel (EmbPMatMatch *s);
void   embMatPrintsInit (AjPFile *fp);
void   embMatProtDelInt (EmbPMatPrints *s);
EmbPMatPrints embMatProtReadInt (AjPFile fp);
ajuint    embMatProtScanInt (const AjPStr s, const AjPStr n,
			    const EmbPMatPrints m, AjPList *l,
			    AjBool *all, AjBool *ordered, AjBool overlap);

/*
** End of prototype definitions
*/


AJ_END_DECLS

#endif  /* !EMBMAT_H */
