
#ifndef ENSTABLE_H
#define ENSTABLE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/*
** AJAX Table of
** AJAX String key data.
*/

AjPTable ensTablestrNewLen(ajuint size);

/*
** AJAX Table of
** AJAX unsigned integer key data.
*/

ajint ensTableuintCmp(const void* x, const void* y);

ajuint ensTableuintHash(const void* key, ajuint hashsize);

void ensTableuintKeydel(void** key);

AjPTable ensTableuintNewLen(ajuint size);

/*
** AJAX Table of
** void key data and
** void value data.
*/

AjBool ensTableClear(AjPTable table);

AjBool ensTableDelete(AjPTable* Ptable);

/*
** AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
*/

AjPTable ensTableuintliststrNewLen(ajuint size);

AjBool ensTableuintliststrClear(AjPTable table);

AjBool ensTableuintliststrDelete(AjPTable* Ptable);

AjBool ensTableuintliststrRegister(AjPTable table,
                                   ajuint identifier,
                                   AjPStr str);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSTABLE_H */
