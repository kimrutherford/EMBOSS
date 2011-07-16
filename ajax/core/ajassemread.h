#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajassemread_h
#define ajassemread_h

#include "ajax.h"

/*
** Prototype definitions
*/

void             ajAsseminClear(AjPAssemin thys);
void             ajAsseminDel(AjPAssemin* pthis);
AjPAssemin       ajAsseminNew(void);
void             ajAsseminQryC(AjPAssemin thys, const char* txt);
void             ajAsseminQryS(AjPAssemin thys, const AjPStr str);
void             ajAsseminTrace(const AjPAssemin thys);

void             ajAsseminprintBook(AjPFile outf);
void             ajAsseminprintHtml(AjPFile outf);
void             ajAsseminprintText(AjPFile outf, AjBool full);
void             ajAsseminprintWiki(AjPFile outf);

AjBool           ajAsseminRead(AjPAssemin assemin, AjPAssem thys);

AjBool           ajAsseminformatTerm(const AjPStr format);
AjBool           ajAsseminformatTest(const AjPStr format);
void             ajAsseminExit(void);
const char*      ajAsseminTypeGetFields(void);
const char*      ajAsseminTypeGetQlinks(void);

AjPTable         ajAssemaccessGetDb(void);
const char*      ajAssemaccessMethodGetQlinks(const AjPStr method);
ajuint           ajAssemaccessMethodGetScope(const AjPStr method);
AjBool           ajAssemaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
