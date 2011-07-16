#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajassemwrite_h
#define ajassemwrite_h


/*
** Prototype definitions
*/

AjBool        ajAssemoutWriteFormat(AjPFile outf, const AjPAssem assem,
                                    const AjPStr fmt);

void          ajAssemoutprintBook(AjPFile outf);
void          ajAssemoutprintHtml(AjPFile outf);
void          ajAssemoutprintText(AjPFile outf, AjBool full);
void          ajAssemoutprintWiki(AjPFile outf);

void          ajAssemoutExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
