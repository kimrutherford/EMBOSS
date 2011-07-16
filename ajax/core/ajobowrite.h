#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajobowrite_h
#define ajobowrite_h


/*
** Prototype definitions
*/

AjBool        ajObooutWrite(AjPOutfile outf, const AjPObo obo);

void          ajObooutprintBook(AjPFile outf);
void          ajObooutprintHtml(AjPFile outf);
void          ajObooutprintText(AjPFile outf, AjBool full);
void          ajObooutprintWiki(AjPFile outf);

AjBool        ajObooutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajObooutformatTest(const AjPStr format);

void          ajObooutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
