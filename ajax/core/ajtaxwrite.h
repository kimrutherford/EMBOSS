#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtaxwrite_h
#define ajtaxwrite_h


/*
** Prototype definitions
*/

AjBool        ajTaxoutWrite(AjPOutfile outf, const AjPTax tax);

void          ajTaxoutprintBook(AjPFile outf);
void          ajTaxoutprintHtml(AjPFile outf);
void          ajTaxoutprintText(AjPFile outf, AjBool full);
void          ajTaxoutprintWiki(AjPFile outf);

AjBool        ajTaxoutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajTaxoutformatTest(const AjPStr format);

void          ajTaxoutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
