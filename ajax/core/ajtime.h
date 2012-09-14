/* @include ajtime ************************************************************
**
** AJAX time functions
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 2003 Jon Ison
** @version $Revision: 1.27 $
** @modified 2004-2011 Peter Rice
** @modified $Date: 2011/10/18 14:23:41 $ by $Author: rice $
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

#ifndef AJTIME_H
#define AJTIME_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

#include <time.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPTime **************************************************************
**
** AJAX time and data data structure. The same structure is used for both.
**
** @alias AjSTime
** @alias AjOTime
** @alias AjPDate
**
** @attr time [struct tm] Time in C standard structure
** @attr format [const char*] Format string if specified
** @attr uppercase [AjBool] Convert to upper case on output
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTime
{
    struct tm time;
    const char *format;
    AjBool uppercase;
    char Padding[4];
} AjOTime, AjODate;

#define AjPTime AjOTime*
#define AjPDate AjODate*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

double        ajTimeDiff(const AjPTime thys, const AjPTime newtime);
void          ajTimeExit(void);
AjBool        ajTimeSetLocal(AjPTime thys, const time_t timer);

AjPTime       ajTimeNewDayFmt(const char *timefmt,
                              ajint mday, ajint mon, ajint year);
AjPTime       ajTimeNewToday(void);
AjPTime       ajTimeNewTodayFmt(const char* timefmt);

void          ajTimeDel(AjPTime *thys);

const AjPTime ajTimeRefToday(void);
const AjPTime ajTimeRefTodayFmt(const char* timefmt);

void          ajTimeTrace(const AjPTime thys);
AjBool        ajTimeSetC(AjPTime thys, const char* timestr);
AjBool        ajTimeSetS(AjPTime thys, const AjPStr timestr);
AjPTime       ajTimeNew(void);
AjPTime       ajTimeNewTime(const AjPTime thys);
time_t        ajTimeGetTimetype(const AjPTime thys);
void          ajTimeReset(void);

ajlong        ajClockNow(void);
void          ajClockReset(void);
double        ajClockDiff(ajlong starttime, ajlong nowtime);
double        ajClockSeconds(void);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjPTime       ajTimeToday(void);
__deprecated AjPTime       ajTimeTodayF(const char* timefmt);
__deprecated const AjPTime ajTimeTodayRef(void);
__deprecated const AjPTime ajTimeTodayRefF(const char* timefmt);
__deprecated AjPTime       ajTimeSet(const char *timefmt,
                                     ajint mday, ajint mon, ajint year);
__deprecated time_t        ajTimeMake(const AjPTime thys);
__deprecated AjBool        ajTimeLocal(const time_t timer, AjPTime thys);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJTIME_H */
