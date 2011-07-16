#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajvarwrite_h
#define ajvarwrite_h


/*
** Prototype definitions
*/

AjBool        ajVaroutWrite(AjPOutfile outf, const AjPVar var);

void          ajVaroutprintBook(AjPFile outf);
void          ajVaroutprintHtml(AjPFile outf);
void          ajVaroutprintText(AjPFile outf, AjBool full);
void          ajVaroutprintWiki(AjPFile outf);

AjBool        ajVaroutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajVaroutformatTest(const AjPStr format);

void          ajVaroutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
