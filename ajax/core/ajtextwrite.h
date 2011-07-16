#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtextwrite_h
#define ajtextwrite_h

#include "ajax.h"

/*
** Prototype definitions
*/

AjBool        ajTextoutWrite(AjPOutfile outf, const AjPText text);

void          ajTextoutprintBook(AjPFile outf);
void          ajTextoutprintHtml(AjPFile outf);
void          ajTextoutprintText(AjPFile outf, AjBool full);
void          ajTextoutprintWiki(AjPFile outf);

AjBool        ajTextoutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajTextoutformatTest(const AjPStr format);

void          ajTextoutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
