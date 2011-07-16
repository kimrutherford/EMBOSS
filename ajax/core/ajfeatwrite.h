#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfeatwrite_h
#define ajfeatwrite_h


/*
** Prototype definitions
*/

AjBool        ajFeatOutFormatDefault (AjPStr* pformat);
void          ajFeatoutPrintFormat(AjPFile outf, AjBool full);
void          ajFeatoutPrinthtmlFormat(AjPFile outf);
void          ajFeatoutPrintbookFormat(AjPFile outf);
void          ajFeatoutPrintwikiFormat(AjPFile outf);
void          ajFeatwriteExit(void);
AjBool        ajFeattableWriteDebug(AjPFeattabOut ftout,
                               const AjPFeattable ftable);
AjBool        ajFeattablePrint(const AjPFeattable ftable,
                               AjPFile file);
AjBool        ajFeattableWriteDasgff(AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteDdbj (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteEmbl (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteGenbank (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteRefseq (AjPFeattabOut ftout,
                                      const AjPFeattable features);
AjBool        ajFeattableWriteRefseqp (AjPFeattabOut ftout,
                                       const AjPFeattable features);
AjBool        ajFeattableWriteGff2 (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteGff3 (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWritePir (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteSwiss (AjPFeattabOut ftout,
                                    const AjPFeattable features);
void          ajFeattabOutClear(AjPFeattabOut *thys);
void          ajFeattabOutDel (AjPFeattabOut* pthis);
AjPFile       ajFeattabOutFile (const AjPFeattabOut thys);
AjPStr        ajFeattabOutFilename (const AjPFeattabOut thys);
AjBool        ajFeattabOutIsLocal(const AjPFeattabOut thys);
AjBool        ajFeattabOutIsOpen (const AjPFeattabOut thys);
AjPFeattabOut ajFeattabOutNew (void);
AjPFeattabOut ajFeattabOutNewCSF (const char* fmt, const AjPStr name,
				  const char* type, AjPFile buff);
AjPFeattabOut ajFeattabOutNewSSF (const AjPStr fmt, const AjPStr name,
				  const char* type, AjPFile buff);
AjBool        ajFeattabOutOpen (AjPFeattabOut thys, const AjPStr ufo);
AjBool        ajFeattabOutSet (AjPFeattabOut thys, const AjPStr ufo);
void          ajFeattabOutSetBasename (AjPFeattabOut thys,
				       const AjPStr basename);
AjBool        ajFeattabOutSetSeqname(AjPFeattabOut thys, const AjPStr name);
AjBool        ajFeattabOutSetType(AjPFeattabOut thys, const AjPStr type);
AjBool        ajFeattabOutSetTypeC(AjPFeattabOut thys, const char* type);
AjBool        ajFeattableWriteUfo (AjPFeattabOut ftout, const AjPFeattable ft,
			      const AjPStr Ufo);
AjBool        ajFeattableWrite (AjPFeattabOut ftout, const AjPFeattable ft) ;

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
