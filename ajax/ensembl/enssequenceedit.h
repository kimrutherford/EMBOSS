
#ifndef ENSSEQUENCEEDIT_H
#define ENSSEQUENCEEDIT_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensattribute.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPSequenceedit *****************************************************
**
** Ensembl Sequence Edit.
**
** @alias EnsSSequenceedit
** @alias EnsOSequenceedit
**
** @cc Bio::EnsEMBL::Attribute
** @attr Attribute [EnsPAttribute] Ensembl Attribute
** @attr Sequence [AjPStr] Alternative sequence
** @attr Start [ajuint] Start coordinate
** @attr End [ajuint] End coordinate
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSSequenceedit
{
    EnsPAttribute Attribute;
    AjPStr Sequence;
    ajuint Start;
    ajuint End;
    ajuint Use;
    ajuint Padding;
} EnsOSequenceedit;

#define EnsPSequenceedit EnsOSequenceedit*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Sequence Edit */

EnsPSequenceedit ensSequenceeditNewAttribute(EnsPAttribute attribute);

EnsPSequenceedit ensSequenceeditNewCpy(const EnsPSequenceedit se);

EnsPSequenceedit ensSequenceeditNewIni(EnsPAttributetype at,
                                       AjPStr sequence,
                                       ajuint start,
                                       ajuint end);

EnsPSequenceedit ensSequenceeditNewRef(EnsPSequenceedit se);

void ensSequenceeditDel(EnsPSequenceedit* Pse);

EnsPAttribute ensSequenceeditGetAttribute(const EnsPSequenceedit se);

ajuint ensSequenceeditGetEnd(const EnsPSequenceedit se);

AjPStr ensSequenceeditGetSequence(const EnsPSequenceedit se);

ajuint ensSequenceeditGetStart(const EnsPSequenceedit se);

AjBool ensSequenceeditTrace(const EnsPSequenceedit se, ajuint level);

ajint ensSequenceeditCalculateDifference(const EnsPSequenceedit se);

size_t ensSequenceeditCalculateMemsize(const EnsPSequenceedit se);

AjBool ensSequenceeditApplyString(const EnsPSequenceedit se,
                                  ajint offset,
                                  AjPStr* Psequence);

AjBool ensListSequenceeditSortStartAscending(AjPList ses);

AjBool ensListSequenceeditSortStartDescending(AjPList ses);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSEQUENCEEDIT_H */
