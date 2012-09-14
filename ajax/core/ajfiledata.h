/* @include ajfiledata ********************************************************
**
** AJAX datafile routines
**
** @author Copyright (C) 1999 Peter Rice
** @version $Revision: 1.5 $
** @modified Peter Rice pmr@ebi.ac.uk Data file functions from ajfile.c
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJFILEDATA_H
#define AJFILEDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"

#include <sys/stat.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjPFile  ajDatafileNewInNamePathS(const AjPStr name, const AjPStr dir);
AjPFile  ajDatafileNewInNameC(const char *s);
AjPFile  ajDatafileNewInNameS(const AjPStr filename);
AjPFile  ajDatafileNewOutNameS(const AjPStr name);


const AjPStr ajDatafileValuePath(void);

void        ajDatafileExit(void);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /** AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void    ajFileDataDirNew(const AjPStr tfile,
                                      const AjPStr dir, AjPFile *fnew);
__deprecated void    ajFileDataDirNewC(const char *s, const char* d,
                                       AjPFile *f);
__deprecated void    ajFileDataNew(const AjPStr filename,
                                   AjPFile *newfileptr);
__deprecated void    ajFileDataNewC(const char *s, AjPFile *f);
__deprecated void    ajFileDataNewWrite(const AjPStr tfile, AjPFile *fnew);


__deprecated AjBool  ajFilePathData(AjPStr *Ppath);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJFILEDATA_H */
