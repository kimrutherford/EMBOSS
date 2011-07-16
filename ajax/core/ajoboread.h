#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajoboread_h
#define ajoboread_h

#include "ajax.h"

/*
** Prototype definitions
*/

AjPOboData       ajObodataNew(void);
AjPOboData       ajObodataParseObofile(AjPFile infile, const char* validations);
AjPObo           ajObodataFetchId(const AjPOboData thys, const AjPStr query);
AjPObo           ajObodataFetchName(const AjPOboData thys, const AjPStr query);

AjBool           ajObolineCutComment(AjPStr *Pline, AjPStr *Pcomment);
AjBool           ajObolineCutModifier(AjPStr *Pline, AjPStr *Pmodifier);
AjBool           ajObolineCutDbxref(AjPStr *Pline, AjPStr *Pdbxref);
AjBool           ajObolineEscape(AjPStr *Pline);

void             ajOboallClear(AjPOboall thys);
void             ajOboallDel(AjPOboall* pthis);
AjPOboall        ajOboallNew(void);
const AjPStr     ajOboallGetoboId(const AjPOboall thys);
const AjPStr     ajOboallGetQryS(const AjPOboall thys);

void             ajOboinClear(AjPOboin thys);
void             ajOboinDel(AjPOboin* pthis);
const AjPStr     ajOboinGetQryS(const AjPOboin thys);
AjPOboin         ajOboinNew(void);
void             ajOboinQryC(AjPOboin thys, const char* txt);
void             ajOboinQryS(AjPOboin thys, const AjPStr str);
void             ajOboinTrace(const AjPOboin thys);
const char*      ajOboinTypeGetFields(void);
const char*      ajOboinTypeGetQlinks(void);

void             ajOboinprintBook(AjPFile outf);
void             ajOboinprintHtml(AjPFile outf);
void             ajOboinprintText(AjPFile outf, AjBool full);
void             ajOboinprintWiki(AjPFile outf);

AjBool           ajOboallNext(AjPOboall thys, AjPObo *Pobo);
AjBool           ajOboinRead(AjPOboin oboin, AjPObo thys);

ajuint           ajOboqueryGetallObofields(const AjPQuery query,
                                           const AjPStr *id, const AjPStr *acc,
                                           const AjPStr *nam, const AjPStr *des,
                                           const AjPStr *up, const AjPStr *div);

AjBool           ajOboinformatTerm(const AjPStr term);
AjBool           ajOboinformatTest(const AjPStr format);
void             ajOboinExit(void);

AjPTable         ajOboaccessGetDb(void);
const char*      ajOboaccessMethodGetQlinks(const AjPStr method);
ajuint           ajOboaccessMethodGetScope(const AjPStr method);
AjBool           ajOboaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
