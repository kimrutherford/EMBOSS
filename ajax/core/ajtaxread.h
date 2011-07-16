#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtaxread_h
#define ajtaxread_h

#include "ajax.h"

/*
** Prototype definitions
*/

void             ajTaxallClear(AjPTaxall thys);
void             ajTaxallDel(AjPTaxall* pthis);
AjPTaxall        ajTaxallNew(void);
const AjPStr     ajTaxallGettaxId(const AjPTaxall thys);

void             ajTaxinClear(AjPTaxin thys);
void             ajTaxinDel(AjPTaxin* pthis);
AjPTaxin         ajTaxinNew(void);
void             ajTaxinQryC(AjPTaxin thys, const char* txt);
void             ajTaxinQryS(AjPTaxin thys, const AjPStr str);
void             ajTaxinTrace(const AjPTaxin thys);

void             ajTaxinprintBook(AjPFile outf);
void             ajTaxinprintHtml(AjPFile outf);
void             ajTaxinprintText(AjPFile outf, AjBool full);
void             ajTaxinprintWiki(AjPFile outf);

AjBool           ajTaxallNext(AjPTaxall thys, AjPTax *Ptax);
AjBool           ajTaxinRead(AjPTaxin taxin, AjPTax thys);

AjBool           ajTaxinformatTest(const AjPStr format);
void             ajTaxinExit(void);
const char*      ajTaxinTypeGetFields(void);
const char*      ajTaxinTypeGetQlinks(void);

AjPTable         ajTaxaccessGetDb(void);
const char*      ajTaxaccessMethodGetQlinks(const AjPStr method);
ajuint           ajTaxaccessMethodGetScope(const AjPStr method);
AjBool           ajTaxaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
