#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtextread_h
#define ajtextread_h


extern AjPTable textDbMethods;



/*
** Prototype definitions
*/

AjPTextin    ajTextinNew(void);
AjPTextin    ajTextinNewDatatype(const AjEDataType datatype);
void         ajTextinDel(AjPTextin* pthis);
void         ajTextinDelNofile(AjPTextin* pthis);
void         ajTextinClear(AjPTextin thys);
void         ajTextinClearNofile(AjPTextin thys);
const AjPStr ajTextinGetQryS(const AjPTextin thys);
void         ajTextinQryC(AjPTextin thys, const char* txt);
void         ajTextinQryS(AjPTextin thys, const AjPStr str);
void         ajTextinTrace(const AjPTextin thys);
const char*  ajTextinTypeGetFields(void);
const char*  ajTextinTypeGetQlinks(void);

AjPTable     ajTextaccessGetDb(void);
const char*  ajTextaccessMethodGetQlinks(const AjPStr method);
ajuint       ajTextaccessMethodGetScope(const AjPStr method);
AjBool       ajTextaccessMethodTest (const AjPStr method);

AjBool       ajTextinAccessAsis (AjPTextin textin);
AjBool       ajTextinAccessFile (AjPTextin textin);
AjBool       ajTextinAccessOffset (AjPTextin textin);
AjBool       ajTextinformatTerm(const AjPStr term);
AjBool       ajTextinformatTest(const AjPStr format);
AjBool       ajTextinRead(AjPTextin textin, AjPText text);

void         ajTextinprintBook(AjPFile outf);
void         ajTextinprintHtml(AjPFile outf);
void         ajTextinprintText(AjPFile outf, AjBool full);
void         ajTextinprintWiki(AjPFile outf);

AjBool       ajTextallNext(AjPTextall thys, AjPText *Ptext);

void         ajTextallClear(AjPTextall thys);
void         ajTextallDel(AjPTextall* pthis);
AjPTextall   ajTextallNew(void);

void         ajTextinExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
