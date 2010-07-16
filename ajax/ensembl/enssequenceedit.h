#ifdef __cplusplus
extern "C"
{
#endif

#ifndef enssequenceedit_h
#define enssequenceedit_h

#include "ensattribute.h"




/* @data EnsPSequenceEdit *****************************************************
**
** Ensembl Sequence Edit.
**
** @alias EnsSSequenceEdit
** @alias EnsOSequenceEdit
**
** @cc Bio::EnsEMBL::Attribute
** @attr Attribute [EnsPAttribute] Ensembl Attribute.
** @attr AltSeq [AjPStr] Alternative sequence.
** @attr Start [ajuint] Start coordinate.
** @attr End [ajuint] End coordinate.
** @attr Use [ajuint] Use counter.
** @attr Padding [ajuint] Padding to alignment boundary.
** @@
******************************************************************************/

typedef struct EnsSSequenceEdit
{
    EnsPAttribute Attribute;
    AjPStr AltSeq;
    ajuint Start;
    ajuint End;
    ajuint Use;
    ajuint Padding;
} EnsOSequenceEdit;

#define EnsPSequenceEdit EnsOSequenceEdit*




/*
** Prototype definitions
*/

/* Ensembl Sequence Edit */

EnsPSequenceEdit ensSequenceEditNew(AjPStr code,
                                    AjPStr name,
                                    AjPStr description,
                                    AjPStr altseq,
                                    ajuint start,
                                    ajuint end);

EnsPSequenceEdit ensSequenceEditNewA(EnsPAttribute attribute);

void ensSequenceEditDel(EnsPSequenceEdit* Pse);

EnsPAttribute ensSequenceEditGetAttribute(const EnsPSequenceEdit se);

AjPStr ensSequenceEditGetAltSeq(const EnsPSequenceEdit se);

ajuint ensSequenceEditGetStart(const EnsPSequenceEdit se);

ajuint ensSequenceEditGetEnd(const EnsPSequenceEdit se);

ajint ensSequenceEditGetLengthDifference(EnsPSequenceEdit se);

AjBool ensSequenceEditTrace(const EnsPSequenceEdit se, ajuint level);

AjBool ensSequenceEditApplyEdit(EnsPSequenceEdit se,
                                ajint offset,
                                AjPStr* Psequence);

AjBool ensSequenceEditSortByStartAscending(AjPList ses);

AjBool ensSequenceEditSortByStartDescending(AjPList ses);

/*
** End of prototype definitions
*/




#endif /* enssequenceedit_h */

#ifdef __cplusplus
}
#endif
