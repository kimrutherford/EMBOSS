#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajurlwrite_h
#define ajurlwrite_h


/*
** Prototype definitions
*/

AjBool        ajUrloutWrite(AjPOutfile outf, const AjPUrl url);

void          ajUrloutprintBook(AjPFile outf);
void          ajUrloutprintHtml(AjPFile outf);
void          ajUrloutprintText(AjPFile outf, AjBool full);
void          ajUrloutprintWiki(AjPFile outf);

AjBool        ajUrloutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajUrloutformatTest(const AjPStr format);

void          ajUrloutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
