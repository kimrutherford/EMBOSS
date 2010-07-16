#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensutility_h
#define ensutility_h

#include "ajax.h"
#include "ensregistry.h"
#include "enstranslation.h"




/*
** Prototype definitions
*/

void ensInit(void);

void ensExit(void);

AjBool ensSeqinTrace(const AjPSeqin seqin, ajuint level);

AjBool ensSeqQueryTrace(const AjPSeqQuery seqquery, ajuint level);

AjBool ensSeqDescTrace(const AjPSeqDesc seqdesc, ajuint level);

AjBool ensSeqTrace(const AjPSeq seq, ajuint level);

/*
** End of prototype definitions
*/




#endif /* ensutility_h */

#ifdef __cplusplus
}
#endif
