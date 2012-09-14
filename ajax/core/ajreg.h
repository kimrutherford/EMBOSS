/* @include ajreg *************************************************************
**
** AJAX REG (ajax regular expression) functions
**
** Uses the Perl-Comparible Regular Expressions Library (PCRE)
** included as a sepoarate library in the EMBOSS distribution.
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.24 $
** @modified Jun 25 pmr First version
** @modified 1999-2011 pmr Replace Henry Spencer library with PCRE
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

#ifndef AJREG_H
#define AJREG_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

#include "pcre_config.h"
#include "pcre_internal.h"
#include "pcreposix.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define AJREG_OVECSIZE 30




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPRegexp ************************************************************
**
** PCRE expression internals, wrapped for AJAX calls
**
** @alias AjSRegexp
** @alias AjORegexp
**
** @attr pcre [real_pcre*] PCRE compiled expression
** @attr extra [pcre_extra*] PCRE study data (if available, else NULL)
** @attr ovector [int*] Output vector offsets
** @attr orig [const char*] Original string
** @attr ovecsize [int] Output vector size
** @attr matches [int] Number of matches
******************************************************************************/

typedef struct AjSRegexp
{
    real_pcre *pcre;
    pcre_extra *extra;
    int *ovector;
    const char* orig;
    int ovecsize;
    int matches;
} AjORegexp;

#define AjPRegexp AjORegexp*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

/* constructors */

AjPRegexp ajRegComp(const AjPStr rexp);
AjPRegexp ajRegCompC(const char* rexp);

AjPRegexp ajRegCompCase(const AjPStr rexp);
AjPRegexp ajRegCompCaseC(const char* rexp);

/* execute expression match */

AjBool ajRegExec(AjPRegexp prog, const AjPStr str);
AjBool ajRegExecC(AjPRegexp prog, const char* str);

AjBool ajRegExecall(AjPRegexp prog, const AjPStr str);
AjBool ajRegExecallC(AjPRegexp prog, const char* str);

/* test substrings */

ajint  ajRegGetMatches(const AjPRegexp rp);
ajint  ajRegLenI(const AjPRegexp rp, ajint isub);
ajint  ajRegOffset(const AjPRegexp rp);
ajint  ajRegOffsetI(const AjPRegexp rp, ajint isub);

/* get substrings */

AjBool ajRegPre(const AjPRegexp rp, AjPStr* dest);
AjBool ajRegPost(const AjPRegexp rp, AjPStr* post);
AjBool ajRegPostC(const AjPRegexp rp, const char** post);
AjBool ajRegSubI(const AjPRegexp rp, ajint isub, AjPStr* dest);

/* destructor */

void   ajRegFree(AjPRegexp* pexp);
void   ajRegTrace(const AjPRegexp rexp);

void   ajRegExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJREG_H */
