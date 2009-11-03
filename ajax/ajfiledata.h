#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfiledata_h
#define ajfiledata_h

#include "ajfile.h"
#include <sys/stat.h>

#endif

/*
** Prototype definitions
*/


AjPFile  ajDatafileNewInNamePathS(const AjPStr name, const AjPStr dir);
AjPFile  ajDatafileNewInNameC(const char *s);
AjPFile  ajDatafileNewInNameS(const AjPStr filename);
AjPFile  ajDatafileNewOutNameS(const AjPStr name);


const AjPStr ajDatafileValuePath(void);

void        ajDatafileExit(void);

__deprecated void    ajFileDataDirNew(const AjPStr tfile,
                                      const AjPStr dir, AjPFile *fnew);
__deprecated void    ajFileDataDirNewC (const char *s, const char* d,
                                        AjPFile *f);
__deprecated void    ajFileDataNew  (const AjPStr filename,
                                     AjPFile *newfileptr);
__deprecated void    ajFileDataNewC (const char *s, AjPFile *f);
__deprecated void    ajFileDataNewWrite (const AjPStr tfile, AjPFile *fnew);


__deprecated AjBool  ajFilePathData(AjPStr *Ppath);

/*
** End of prototype definitions
*/

#ifdef __cplusplus
}
#endif
