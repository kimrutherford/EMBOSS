#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h



/* @data SeqPAccess ***********************************************************
**
** Ajax Sequence Access database reading object.
**
** Holds information needed to read a sequence from a database.
** Access methods are defined for each known database type.
**
** Sequences are read from the database using the defined
** database access function, which is usually a static function
** within ajseq.c
**
** This should be a static data object but is needed for the definition
** of AjPSeqin.
**
** @alias SeqSAccess
** @new ajSeqMethod returns a copy of a known access method definition.
** @other AjPSeqin Sequence input
**
** @attr Name [const char*] Access method name used in emboss.default
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Access [(AjBool*)] Access function
** @attr AccessFree [(AjBool*)] Access cleanup function
** @attr Desc [const char*] Description
** @@
******************************************************************************/

typedef struct SeqSAccess {
  const char *Name;
  AjBool Alias;
  AjBool Entry;
  AjBool Query;
  AjBool All;
  AjBool (*Access) (AjPSeqin seqin);
  AjBool (*AccessFree) (void* qry);
  const char* Desc;
} SeqOAccess;

#define SeqPAccess SeqOAccess*



/*
** Prototype definitions
*/

AjBool     ajSeqAccessAsis (AjPSeqin seqin);
AjBool     ajSeqAccessFile (AjPSeqin seqin);
AjBool     ajSeqAccessOffset (AjPSeqin seqin);
void       ajSeqDbExit(void);
SeqPAccess ajSeqMethod (const AjPStr method);
AjBool     ajSeqMethodTest (const AjPStr method);
void       ajSeqPrintAccess (AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
