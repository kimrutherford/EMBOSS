#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajresourcewrite_h
#define ajresourcewrite_h


/*
** Prototype definitions
*/

AjBool        ajResourceoutWrite(AjPOutfile outf,
                                       const AjPResource resource);

AjBool        ajResourceoutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajResourceoutformatTest(const AjPStr format);

void          ajResourceoutprintBook(AjPFile outf);
void          ajResourceoutprintHtml(AjPFile outf);
void          ajResourceoutprintText(AjPFile outf, AjBool full);
void          ajResourceoutprintWiki(AjPFile outf);

void          ajResourceoutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
