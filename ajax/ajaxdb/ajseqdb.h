#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h



/*
** Prototype definitions
*/

void         ajSeqDbExit(void);
void         ajSeqdbInit(void);
FILE*        ajSeqHttpGet(const AjPSeqQuery qry, const AjPStr host,
                          ajint iport, const AjPStr get);
FILE*        ajSeqHttpGetProxy(const AjPSeqQuery qry, const AjPStr proxyname,
                               ajint proxyport, const AjPStr host,
                               ajint iport, const AjPStr get);
AjBool       ajSeqHttpProxy(const AjPSeqQuery qry, ajint* iport,
                            AjPStr* proxyname);
AjBool       ajSeqHttpVersion(const AjPSeqQuery qry, AjPStr* httpver);
AjPSeqAccess ajSeqMethod (const AjPStr method);
void         ajSeqPrintAccess (AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
