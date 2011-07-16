#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajvarread_h
#define ajvarread_h

#include "ajax.h"

/*
** Prototype definitions
*/

void             ajVarallClear(AjPVarall thys);
void             ajVarallDel(AjPVarall* pthis);
AjPVarall        ajVarallNew(void);
const AjPStr     ajVarallGetvarId(const AjPVarall thys);

void             ajVarinClear(AjPVarin thys);
void             ajVarinDel(AjPVarin* pthis);
AjPVarin        ajVarinNew(void);
void             ajVarinQryC(AjPVarin thys, const char* txt);
void             ajVarinQryS(AjPVarin thys, const AjPStr str);
void             ajVarinTrace(const AjPVarin thys);

void             ajVarinprintBook(AjPFile outf);
void             ajVarinprintHtml(AjPFile outf);
void             ajVarinprintText(AjPFile outf, AjBool full);
void             ajVarinprintWiki(AjPFile outf);

AjBool           ajVarallNext(AjPVarall thys, AjPVar *Pvar);
AjBool           ajVarinRead(AjPVarin varin, AjPVar thys);

AjBool           ajVarinformatTerm(const AjPStr term);
AjBool           ajVarinformatTest(const AjPStr format);
void             ajVarinExit(void);
const char*      ajVarinTypeGetFields(void);
const char*      ajVarinTypeGetQlinks(void);

AjPTable         ajVaraccessGetDb(void);
const char*      ajVaraccessMethodGetQlinks(const AjPStr method);
ajuint           ajVaraccessMethodGetScope(const AjPStr method);
AjBool           ajVaraccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
