#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfeatread_h
#define ajfeatread_h


extern AjPTable feattabDbMethods;



/*
** Prototype definitions
*/

void          ajFeattaballClear(AjPFeattaball thys);
void          ajFeattaballDel(AjPFeattaball* pthis);
AjPFeattaball ajFeattaballNew(void);
const AjPStr  ajFeattaballGetfeattableId(const AjPFeattaball thys);

void          ajFeattabinClear (AjPFeattabin thys);
void          ajFeattabinDel (AjPFeattabin* pthis);
AjPFeattabin  ajFeattabinNew (void);
AjPFeattabin  ajFeattabinNewCSF (const char* fmt, const AjPStr name,
				 const char* type, AjPFilebuff buff);
AjPFeattabin  ajFeattabinNewSS (const AjPStr fmt, const AjPStr name,
				const char* type);
AjPFeattabin  ajFeattabinNewSSF (const AjPStr fmt, const AjPStr name,
				 const char* type, AjPFilebuff buff);
void          ajFeattabinSetRange  (AjPFeattabin thys,
				     ajint fbegin, ajint fend) ;
AjBool        ajFeattabinSetTypeC(AjPFeattabin thys, const char* type);
AjBool        ajFeattabinSetTypeS(AjPFeattabin thys, const AjPStr type);

void          ajFeatinPrintFormat(AjPFile outf, AjBool full);
void          ajFeatinPrinthtmlFormat(AjPFile outf);
void          ajFeatinPrintbookFormat(AjPFile outf);
void          ajFeatinPrintwikiFormat(AjPFile outf);
void          ajFeatreadExit(void);
const char*   ajFeattabinTypeGetFields(void);
const char*   ajFeattabinTypeGetQlinks(void);

AjPTable      ajFeattabaccessGetDb(void);
const char*   ajFeattabaccessMethodGetQlinks(const AjPStr method);
ajuint        ajFeattabaccessMethodGetScope(const AjPStr method);
AjBool        ajFeattabaccessMethodTest(const AjPStr method);
AjBool        ajFeattabinformatTerm(const AjPStr term);
AjBool        ajFeattabinformatTest(const AjPStr format);

void          ajFeattabinQryC(AjPFeattabin thys, const char* txt);
void          ajFeattabinQryS(AjPFeattabin thys, const AjPStr txt);

AjBool        ajFeattabinRead(AjPFeattabin ftin, AjPFeattable ftable);

AjBool        ajFeattaballNext(AjPFeattaball thys, AjPFeattable *Pfeattable);
AjPFeattable  ajFeattableNewRead  (AjPFeattabin ftin);
AjPFeattable  ajFeattableNewReadUfo (AjPFeattabin tabin, const AjPStr Ufo);

/*
** End of prototype definitions
*/

__deprecated void          ajFeattabInClear (AjPFeattabIn thys);
__deprecated void          ajFeattabInDel (AjPFeattabIn* pthis);
__deprecated AjPFeattabIn  ajFeattabInNew (void);
__deprecated AjPFeattabIn  ajFeattabInNewCSF (const char* fmt, const AjPStr name,
				 const char* type, AjPFilebuff buff);
__deprecated AjPFeattabIn  ajFeattabInNewSS (const AjPStr fmt, const AjPStr name,
				const char* type);
__deprecated AjPFeattabIn  ajFeattabInNewSSF (const AjPStr fmt, const AjPStr name,
				 const char* type, AjPFilebuff buff);
__deprecated AjBool        ajFeattabInSetType(AjPFeattabIn thys, const AjPStr type);
__deprecated AjBool        ajFeattabInSetTypeC(AjPFeattabIn thys, const char* type);

#endif

#ifdef __cplusplus
}
#endif
