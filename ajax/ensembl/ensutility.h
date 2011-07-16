
#ifndef ENSUTILITY_H
#define ENSUTILITY_H

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

void ensInit(void);

void ensExit(void);

AjBool ensTraceQuery(const AjPQuery qry, ajuint level);

AjBool ensTraceSeqin(const AjPSeqin seqin, ajuint level);

AjBool ensTraceSeqdesc(const AjPSeqDesc seqdesc, ajuint level);

AjBool ensTraceSeq(const AjPSeq seq, ajuint level);

AjBool ensTraceTextin(const AjPTextin textin, ajuint level);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSUTILITY_H */
