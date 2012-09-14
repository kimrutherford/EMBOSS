/* @include ajfmt *************************************************************
**
** AJAX format functions
**
** String formatting routines. Similar to printf, fprintf, vprintf
** etc but the set of conversion specifiers is not fixed, and cannot
** store more characters than it can hold.
** There is also ajFmtScanS / ajFmtScanC which is an extended sscanf.
**
** Special formatting provided here:
**   %B : AJAX boolean
**   %D : AJAX date
**   %S : AJAX string
**   %z : Dynamic char* allocation in ajFmtScanS
**
** Other differences are:
**   %s : accepts null strings and prints null in angle brackets
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.22 $
** @modified Copyright (C) 2001 Alan Bleasby. Added ajFmtScanS functions
** @modified Copyright (C) 2003 Jon Ison. Added ajFmtScanC functions
** @modified $Date: 2012/03/22 21:19:08 $ by $Author: mks $
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

#ifndef AJFMT_H
#define AJFMT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajfile.h"
#include "ajstr.h"

#include <stdarg.h>

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

void   ajFmtFmt(int (*put) (int c, void *cl), void *cl,
                const char *fmt, ...);
void   ajFmtVfmt(int (*put) (int c, void *cl), void *cl,
                 const char *fmt, va_list ap);
void   ajFmtError(const char *fmt, ...);
void   ajFmtVError(const char *fmt, va_list ap);
void   ajFmtPrint(const char *fmt, ...);
void   ajFmtVPrint(const char *fmt, va_list ap);
void   ajFmtPrintFp(FILE *stream,
                    const char *fmt, ...);
void   ajFmtVPrintFp(FILE *stream,
                     const char *fmt, va_list ap);
void   ajFmtPrintF(AjPFile file,
                   const char *fmt, ...);
void   ajFmtVPrintF(AjPFile file,
                    const char *fmt, va_list ap);
ajint  ajFmtPrintCL(char *buf, ajint size,
                    const char *fmt, ...);
ajint  ajFmtVPrintCL(char *buf, ajint size,
                     const char *fmt, va_list ap);
void   ajFmtPrintSplit(AjPFile outf, const AjPStr str,
                       const char *prefix, ajint len,
                       const char *delim);
char*  ajFmtString(const char *fmt, ...);
char*  ajFmtVString(const char *fmt, va_list ap);
void   ajFmtPutd(const char *str, ajint len,
                 int (*put) (int c, void *cl), void *cl,
                 const ajuint *flags, ajint width, ajint precision);
void   ajFmtPuts(const char *str, ajint len,
                 int (*put) (int c, void *cl), void *cl,
                 const ajuint *flags, ajint width, ajint precision);
AjPStr ajFmtStr(const char *fmt, ...);
AjPStr ajFmtPrintS(AjPStr *pthis, const char *fmt, ...);
AjPStr ajFmtVPrintS(AjPStr *pthis, const char *fmt, va_list ap);
AjPStr ajFmtPrintAppS(AjPStr *pthis, const char *fmt, ...);
ajint  ajFmtVfmtCL(char *buf, ajint size, const char *fmt,
                   va_list ap);
ajint  ajFmtVfmtStrCL(char **buf, ajint pos, size_t *size,
                      const char *fmt, va_list ap);

ajint  ajFmtScanS(const AjPStr thys, const char *fmt, ...);
ajint  ajFmtScanC(const char *thys, const char *fmt, ...);
ajint  ajFmtScanF(AjPFile thys, const char *fmt, ...);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJFMT_H */
