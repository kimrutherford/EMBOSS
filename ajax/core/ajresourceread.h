#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajresourceread_h
#define ajresourceread_h

#include "ajax.h"

/*
** Prototype definitions
*/

void           ajResourceallClear(AjPResourceall thys);
void           ajResourceallDel(AjPResourceall* pthis);
AjPResourceall ajResourceallNew(void);
AjPResourcein  ajResourceinNewDrcat(const AjPStr dbname);
const AjPStr   ajResourceallGetresourceId(const AjPResourceall thys);

void           ajResourceinClear(AjPResourcein thys);
void           ajResourceinDel(AjPResourcein* pthis);
AjPResourcein  ajResourceinNew(void);
void           ajResourceinQryC(AjPResourcein thys, const char* txt);
void           ajResourceinQryS(AjPResourcein thys, const AjPStr str);
void           ajResourceinTrace(const AjPResourcein thys);

void           ajResourceinprintBook(AjPFile outf);
void           ajResourceinprintHtml(AjPFile outf);
void           ajResourceinprintText(AjPFile outf, AjBool full);
void           ajResourceinprintWiki(AjPFile outf);

AjBool         ajResourceallNext(AjPResourceall thys, AjPResource *Presource);
AjBool         ajResourceinRead(AjPResourcein resourcein, AjPResource thys);

AjBool         ajResourceinformatTerm(const AjPStr term);
AjBool         ajResourceinformatTest(const AjPStr format);
void           ajResourceinExit(void);
const char*    ajResourceinTypeGetFields(void);
const char*    ajResourceinTypeGetQlinks(void);

AjPTable       ajResourceaccessGetDb(void);
const char*    ajResourceaccessMethodGetQlinks(const AjPStr method);
ajuint         ajResourceaccessMethodGetScope(const AjPStr method);
AjBool         ajResourceaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
