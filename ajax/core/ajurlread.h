#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajurlread_h
#define ajurlread_h

#include "ajax.h"

/*
** Prototype definitions
*/

void             ajUrlallClear(AjPUrlall thys);
void             ajUrlallDel(AjPUrlall* pthis);
AjPUrlall        ajUrlallNew(void);
const AjPStr     ajUrlallGeturlId(const AjPUrlall thys);

void             ajUrlinClear(AjPUrlin thys);
void             ajUrlinDel(AjPUrlin* pthis);
AjPUrlin         ajUrlinNew(void);
void             ajUrlinQryC(AjPUrlin thys, const char* txt);
void             ajUrlinQryS(AjPUrlin thys, const AjPStr str);
void             ajUrlinTrace(const AjPUrlin thys);

void             ajUrlinprintBook(AjPFile outf);
void             ajUrlinprintHtml(AjPFile outf);
void             ajUrlinprintText(AjPFile outf, AjBool full);
void             ajUrlinprintWiki(AjPFile outf);

AjBool           ajUrlallNext(AjPUrlall thys, AjPUrl *Purl);
AjBool           ajUrlinRead(AjPUrlin urlin, AjPUrl thys);

AjBool           ajUrlinformatTerm(const AjPStr term);
AjBool           ajUrlinformatTest(const AjPStr format);
void             ajUrlinExit(void);
const char*      ajUrlinTypeGetFields(void);
const char*      ajUrlinTypeGetQlinks(void);

AjPTable         ajUrlaccessGetDb(void);
const char*      ajUrlaccessMethodGetQlinks(const AjPStr method);
ajuint           ajUrlaccessMethodGetScope(const AjPStr method);
AjBool           ajUrlaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
