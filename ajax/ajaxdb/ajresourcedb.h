#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajresourcedb_h
#define ajresourcedb_h



/*
** Prototype definitions
*/

void           ajResourcedbExit(void);
void           ajResourcedbInit(void);
void           ajResourcedbPrintAccess(AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
