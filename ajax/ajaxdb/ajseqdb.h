#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h



/*
** Prototype definitions
*/

void       ajSeqDbExit(void);
void       ajSeqdbInit(void);
SeqPAccess ajSeqMethod (const AjPStr method);
void       ajSeqPrintAccess (AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
