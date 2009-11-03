/******************************************************************************
** @source AJAX time functions
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 2003 Jon Ison    
** @version 1.0
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/


#include <time.h>

#include "ajax.h"

static clock_t timeClockSave = 0;

ajlong timeClockOverflow = 0L;
ajlong timeClockMax = 0L;


/* @datastatic TimePFormat ****************************************************
**
** Internal structure for known Ajax time formats
**
** @alias TimeSFormat
** @alias TimeOFormat
**
** @attr Name [const char*] format name
** @attr Format [const char*] C run time library time format string
** @attr Uppercase [AjBool] Convert to upper case on output
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TimeSFormat
{
    const char* Name;
    const char* Format;
    AjBool Uppercase;
    ajint Padding;
} TimeOFormat;
#define TimePFormat TimeOFormat*


static AjPTime timeTodayData = NULL;
static AjPTime timeTodaySaved = NULL;


static TimeOFormat timeFormat[] =  /* formats for strftime */
{
    {"GFF", "%Y-%m-%d", AJFALSE, 0},
    {"yyyy-mm-dd", "%Y-%m-%d", AJFALSE, 0},
    {"dd Mon yyyy", "%d %b %Y", AJFALSE, 0},
    {"day", "%d-%b-%Y", AJFALSE, 0},
    {"time", "%H:%M:%S", AJFALSE, 0},
    {"daytime", "%d-%b-%Y %H:%M", AJFALSE, 0},
    {"log", "%a %b %d %H:%M:%S %Y", AJFALSE, 0},
#ifndef WIN32
    {"report", "%a %e %b %Y %H:%M:%S", AJFALSE, 0},
#else
    {"report", "%a %#d %b %Y %H:%M:%S", AJFALSE, 0},
#endif
    {"dbindex", "%d/%m/%y", AJFALSE, 0},
    {"dtline", "%d-%b-%Y", AJTRUE, 0},
    { NULL, NULL, AJFALSE, 0}
};


static const char* TimeFormat(const char *timefmt, AjBool* makeupper);


/* @filesection ajtime *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/


/* @datasection [AjPTime] Time object ****************************************
**
** @nam2rule Time     Function is for handling time and usually processes an
**                    AjSTime object.
*/



/* @section constructors ******************************************************
**
** Functions for constructing time objects.
**
** @fdata [AjPTime]
**
** @nam3rule  New            Construct a new time object.
** @nam4rule  NewDay         Construct with specified day.
** @nam5rule  NewDayFmt      Construct with specified day in specified format.
** @nam4rule  NewToday       Construct with current time in default format.
** @nam5rule  NewTodayFmt    Construct with current time in specified format.
** @nam4rule  NewTime        Copy constructor
**
** @argrule Fmt timefmt [const char*] Known EMBOSS time format
** @argrule Day mday [ajint] Day of the month (1-31)
** @argrule Day mon  [ajint] Day of the month (1-12)
** @argrule Day year [ajint] Year as a 4 digit number
** @argrule NewTime src [const AjPTime] Time object to be copied
**
** @valrule   *  [AjPTime] New time object.
** @fcategory new
**
******************************************************************************/




/* @func ajTimeNew ************************************************************
**
** Constructor for AjPTime object.
**
** @return [AjPTime] An AjPTime object
** @@
******************************************************************************/

AjPTime ajTimeNew(void)
{
    AjPTime thys = NULL;

    AJNEW0(thys);

    return thys ;
}




/* @func ajTimeNewDayFmt ******************************************************
**
** Constructor for user specification of an arbitrary AjPTime object.
** Except for 'timefmt', the arguments are based upon the UNIX
** 'tm' time structure defined in the time.h header file.
** The range validity of numbers given are not checked.
**
** @param  [rN] timefmt [const char*] Time format to use
** @param  [rN] mday    [ajint]   Day of the month [1-31]
** @param  [rN] mon     [ajint]   Month [1-12]
** @param  [rN] year    [ajint]   Four digit year
** @return [AjPTime] An AjPTime object
** @@
******************************************************************************/

AjPTime ajTimeNewDayFmt( const char *timefmt,
			ajint mday, ajint mon, ajint year)
{
    AjPTime thys;

    thys = ajTimeNewTodayFmt(timefmt) ;

    thys->time.tm_mday  = mday ;
    thys->time.tm_mon   = mon-1;

    if(year > 1899)
        year = year-1900;

    thys->time.tm_year  = year ;

    mktime(&thys->time);

    return thys ;
}




/* @obsolete ajTimeSet
** @rename ajTimeNewDayFmt
*/

__deprecated AjPTime ajTimeSet(const char *timefmt,
			       ajint mday, ajint mon, ajint year)
{
    return ajTimeNewDayFmt(timefmt, mday, mon, year);
}




/* @func ajTimeNewTime ********************************************************
**
** Constructor for AjPTime object, making a copy of an existing time object
**
** @param  [r] src [const AjPTime] Time object to be copied
** @return [AjPTime] An AjPTime object
** @@
******************************************************************************/

AjPTime ajTimeNewTime(const AjPTime src)
{
    AjPTime thys = NULL;

    AJNEW0(thys);

    thys->time = src->time;
    thys->format = src->format;
    thys->uppercase = src->uppercase;

    return thys ;
}




/* @func ajTimeNewToday *******************************************************
**
** AJAX function to return today's time as an AjPTime object
** @return [AjPTime] Pointer to time object containing today's date/time
** @@
******************************************************************************/

AjPTime ajTimeNewToday(void)
{
    AjPTime thys = NULL;
    time_t tim;
    
    tim = time(0);

    AJNEW0(thys);

    if(!ajTimeSetLocal(thys, tim))
        return NULL;

    thys->format = NULL;

    return thys;
}




/* @obsolete ajTimeToday
** @rename ajTimeNewToday
*/

__deprecated AjPTime ajTimeToday(void)
{
    return ajTimeNewToday();
}




/* @func ajTimeNewTodayFmt ****************************************************
**
** AJAX function to return today's time as an AjPTime object
** with a specified output format
**
** @param [r] timefmt [const char*] A controlled vocabulary of time formats
**
** @return [] [AjPTime] Pointer to time object containing today's date/time
** @@
**
******************************************************************************/

AjPTime ajTimeNewTodayFmt(const char* timefmt)
{
    AjPTime thys = NULL;
    time_t tim;
    
    tim = time(0);

    if(!thys)
	AJNEW0(thys);

    if(!ajTimeSetLocal(thys, tim))
        return NULL;

    thys->format = TimeFormat(timefmt, &thys->uppercase);

    return thys;
}




/* @obsolete ajTimeTodayF
** @rename ajTimeNewTodayFmt
*/

__deprecated AjPTime ajTimeTodayF(const char* timefmt)
{
    return ajTimeNewTodayFmt(timefmt);
}




/* @section destructors *******************************************************
**
** Functions for destructing time objects.
**
** @fdata [AjPTime]
**
** @nam3rule  Del         Destruct a time object.
**
** @argrule   *  Ptime [AjPTime*] Time object to be deleted
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/

/* @func ajTimeDel ************************************************************
**
** Destructor for AjPTime object.
**
** @param [w] Ptime [AjPTime*] Time object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajTimeDel(AjPTime *Ptime)
{
    /* Check arg's */
    if(Ptime==NULL)
	return;

    if(*Ptime==NULL)
	return;

    AJFREE(*Ptime);
    *Ptime = NULL;
    
    return;
}




/* @section get time **********************************************************
**
** Functions for retrieving elements of a time object
**
** @fdata [AjPTime]
**
** @nam3rule  Get            Return elements from a time object.
** @nam4rule  GetTimetype    Return time as a standard C time_t structure
**
** @argrule Get thys [const AjPTime] Time object
**
** @valrule   *  [time_t]  Standard C time_t structure
** @fcategory use
**
******************************************************************************/

/* @func ajTimeGetTimetype ****************************************************
**
** An AjPTime object version of the mktime function that returns
** a standard time_t value
**
** @param [r] thys [const AjPTime] Time object
** @return [time_t] Standard time value
** @@
******************************************************************************/

time_t ajTimeGetTimetype(const AjPTime thys)
{
    struct tm tm = thys->time;		/* mktime resets wday and yday */

    return mktime(&tm);
}




/* @obsolete ajTimeMake
** @rename ajTimeGetTimetype
*/
__deprecated time_t ajTimeMake(const AjPTime thys)
{
    return ajTimeGetTimetype(thys);
}




/* @section get reference to internals ****************************************
**
** Functions for retrieving references to tiome internals
**
** @fdata [AjPTime]
**
** @nam3rule  Ref            Reference time object.
** @nam4rule  RefToday       Reference time object with current time in 
**                           default format.
** @nam5rule  RefTodayFmt    Reference time in defined format.
**
** @argrule Fmt timefmt [const char*] A controlled vocabulary of time formats
** 
** @valrule   *  [const AjPTime]  Reference to Time object.
** @fcategory misc
**
******************************************************************************/

/* @func ajTimeRefToday *******************************************************
**
** AJAX function to return today's time as an AjPTime object reference
** @return [const AjPTime] Pointer to static time object containing
**                         today's date/time
** @@
******************************************************************************/

const AjPTime ajTimeRefToday(void)
{
    time_t tim;

    if(!timeTodaySaved)
    {
	AJNEW0(timeTodaySaved);

        tim = time(0);

        if(!ajTimeSetLocal(timeTodaySaved, tim))
            return NULL;

        timeTodaySaved->format = NULL;
    }
    
    return timeTodaySaved;
}




/* @obsolete ajTimeTodayRef
** @rename ajTimeRefToday
*/

__deprecated const AjPTime ajTimeTodayRef(void)
{
    return ajTimeRefToday();
}




/* @func ajTimeRefTodayFmt ****************************************************
**
** AJAX function to return today's time as a static AjPTime object
** with a specified output format
**
** @param [r] timefmt [const char*] A controlled vocabulary of time formats
**
** @return [] [const AjPTime] Pointer to static time object containing
**                            today's date/time
** @@
**
******************************************************************************/

const AjPTime ajTimeRefTodayFmt(const char* timefmt)
{
    time_t tim;
    
    tim = time(0);

    if(!timeTodayData)
	AJNEW0(timeTodayData);

    if(!ajTimeSetLocal(timeTodayData, tim))
        return NULL;

    timeTodayData->format = TimeFormat(timefmt, &timeTodayData->uppercase);

    return timeTodayData;
}




/* @obsolete ajTimeTodayRefF
** @rename ajTimeRefTodayFmt
*/

__deprecated const AjPTime ajTimeTodayRefF(const char* timefmt)
{
    return ajTimeRefTodayFmt(timefmt);
}




/* @section set time ***********************************************************
**
** Functions for setting the time.
**
** @fdata [AjPTime]
**
** @nam3rule  Set          Set the time.
** @nam4rule  SetLocal     A localtime()/localtime_r() replacement for AjPTime
**                           objects
** @suffix C Char* argument
** @suffix S AjPStr argument
**
** @argrule   *  thys [AjPTime]   Time object to set. 
** @argrule   Local  timer [const time_t] Populated standard C time structure
** @argrule   C  timestr [const char*] Time as a string
** @argrule   S  timestr [const AjPStr] Time as a string
** RefF
** @valrule   *  [AjBool]  True if time was set.
**
** @fcategory modify
**
******************************************************************************/

/* @func ajTimeSetC ***********************************************************
**
** Constructor for user specification of an AjPTime object.
** using the time set as a string in format
** yyyy-mm-dd hh:mm:ss
**
** used so that graphs which include the date can remain constant for
** documentation and testing.
**
** @param [w] thys [AjPTime] Time object
** @param [r] timestr [const char*] Time in format yyyy-mm-dd hh:mm:ss
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajTimeSetC(AjPTime thys, const char* timestr)
{
    ajint year;
    ajint mon;
    ajint mday;
    ajint hour;
    ajint min;
    ajint sec;

    if(!timestr)
	return ajFalse;

    if(!ajFmtScanC(timestr, "%4d-%2d-%2d %2d:%2d:%2d",
	       &year, &mon, &mday, &hour, &min, &sec))
	return ajFalse;

    if(year > 1899)
        year = year-1900;

    thys->time.tm_year  = year ;
    thys->time.tm_mon   = mon-1;
    thys->time.tm_mday  = mday ;
    thys->time.tm_hour = hour;
    thys->time.tm_min = min;
    thys->time.tm_sec = sec;
    thys->time.tm_isdst = -1;

    mktime(&thys->time);

    return ajTrue;
}




/* @func ajTimeSetS ***********************************************************
**
** Constructor for user specification of an AjPTime object.
** using the time set as a string in format
** yyyy-mm-dd hh:mm:ss
**
** used so that graphs which include the date can remain constant for
** documentation and testing.
**
** @param [w] thys [AjPTime] Time object
** @param [r] timestr [const AjPStr] Time in format yyyy-mm-dd hh:mm:ss
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajTimeSetS(AjPTime thys, const AjPStr timestr)
{
    ajint year;
    ajint mon;
    ajint mday;
    ajint hour;
    ajint min;
    ajint sec;

    if(!ajStrGetLen(timestr))
	return ajFalse;

    if(!ajFmtScanS(timestr, "%4d-%2d-%2d %2d:%2d:%2d",
	       &year, &mon, &mday, &hour, &min, &sec))
	return ajFalse;

    if(year > 1899)
        year = year-1900;

    thys->time.tm_year  = year ;
    thys->time.tm_mon   = mon-1;
    thys->time.tm_mday  = mday ;
    thys->time.tm_hour = hour;
    thys->time.tm_min = min;
    thys->time.tm_sec = sec;
    thys->time.tm_isdst = -1;

    mktime(&thys->time);

    return ajTrue;
}




/* @func ajTimeSetLocal *******************************************************
**
** A localtime()/localtime_r() replacement for AjPTime objects
**
** @param  [w] thys [AjPTime] Time object
** @param  [r] timer [const time_t] Populated standard C time structure
**
** @return [AjBool] true if successful
** @@
******************************************************************************/

AjBool ajTimeSetLocal(AjPTime thys, const time_t timer)
{
    struct tm *result;
    AjPStr timestr = NULL;

    if(ajNamGetValueC("timetoday", &timestr))
    {
	if(ajTimeSetS(thys, timestr))
	{
	    ajStrDel(&timestr);

	    return ajTrue;
	}

	ajStrDel(&timestr);
    }

#if defined(__ppc__) || defined(WIN32)
    result = localtime(&timer);
    if(!result)
	return ajFalse;

    thys->time.tm_sec = result->tm_sec;
    thys->time.tm_min = result->tm_min;
    thys->time.tm_mday = result->tm_mday;
    thys->time.tm_wday = result->tm_wday;
    thys->time.tm_hour = result->tm_hour;
    thys->time.tm_mon  = result->tm_mon;
    thys->time.tm_year = result->tm_year;
#else
    result = (struct tm *)localtime_r(&timer,&thys->time);

    if(!result)
	return ajFalse;
#endif

    return ajTrue;
}




/* @obsolete ajTimeLocal
** @replace ajTimeSetLocal (1,2/2,1)
*/

__deprecated AjBool ajTimeLocal(const time_t timer, AjPTime thys)
{
    return ajTimeSetLocal(thys, timer);
}




/* @section comparison ********************************************************
**
** Functions for comparing time objects.
**
** @fdata [AjPTime]
**
** @nam3rule  Diff  Return time difference.
**
** @argrule   *   thys [const AjPTime] Time object.
** @argrule   *   newtime [const AjPTime] Later time object.
** 
** @valrule   Diff  [double] Time difference in seconds
** @fcategory use
**
******************************************************************************/


/* @func ajTimeDiff ***********************************************************
**
** Difference between two time objects
**
** @param [r] thys [const AjPTime] Original time object
** @param [r] newtime [const AjPTime] Later time object
** @return [double] Difference in seconds
******************************************************************************/

double ajTimeDiff(const AjPTime thys, const AjPTime newtime)
{
  double ret = 0.0;
  struct tm oldt;
  struct tm newt;
  time_t oldtm;
  time_t newtm;

  oldt = thys->time;
  newt = newtime->time;
  oldtm = mktime(&oldt);
  newtm = mktime(&newt);

  ret = difftime(newtm,oldtm);

  return ret;
}




/* @funcstatic TimeFormat *****************************************************
**
** AJAX function to return the ANSI C format for an AJAX time string
**
** @param [r] timefmt [const char*] AJAX time format
** @param [w] makeupper [AjBool*] If true, convert time to upper case
** @return [const char*] ANSI C time format, or NULL if none found
** @@
******************************************************************************/

static const char* TimeFormat(const char *timefmt, AjBool* makeupper)
{
    ajint i;
    AjBool ok    = ajFalse;
    const char *format = NULL ;

    for(i=0; timeFormat[i].Name; i++)
	if(ajCharMatchCaseC(timefmt, timeFormat[i].Name))
	{
	    ok = ajTrue;
	    break;
	}

    if(ok)
    {
	format = timeFormat[i].Format;
	*makeupper = timeFormat[i].Uppercase;
    }
    else
    {
	*makeupper = ajFalse;
	ajWarn("Unknown date/time format %s", timefmt);
    }

    return format;
}




/* @section debug *************************************************************
**
** Functions for debugging time objects.
**
** @fdata [AjPTime]
**
** @nam3rule  Trace  Write information on contents of AjPTime object to 
**                      debug file.
**
** @argrule   *   thys [const AjPTime] Time object to debug.
** 
** @valrule   *  [void]
** @fcategory use
**
******************************************************************************/


/* @func ajTimeTrace **********************************************************
**
** Debug report on the contents of an AjPTime object
**
** @param [r] thys [const AjPTime] Time object
** @return [void]
** @@
******************************************************************************/

void ajTimeTrace(const AjPTime thys) 
{
    ajDebug("Time value trace '%D'\n", thys);
    ajDebug("format: '%s'\n", thys->format);

    return;
}




/* @section exit
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPTime]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/


/* @func ajTimeExit ***********************************************************
**
** Cleans up time processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajTimeExit(void)
{
    ajTimeDel(&timeTodayData);
    ajTimeDel(&timeTodaySaved);

    return;
}




/* @datasection [none] time internals ***********************************
**
** Function is for processing time internals.
**
** @nam2rule Time Time processing
**
*/



/* @section reset **************************************************************
**
** Functions for memory cleanup
**
** @fdata [none]
**
** @nam3rule Reset Reset internals
** 
** @valrule   *  [void] No return value
** 
** @fcategory misc
**
******************************************************************************/

/* @func ajTimeReset **********************************************************
**
** Resets the clock time to zero
**
** @return [void]
******************************************************************************/

void ajTimeReset(void)
{
    time_t tim;

    if(!timeTodaySaved)
	AJNEW0(timeTodaySaved);

    tim = time(0);

    if(ajTimeSetLocal(timeTodaySaved, tim))
        timeTodaySaved->format = NULL;
    
    return;
}




/* @datasection [none] Cpu clock object ****************************************
**
** @nam2rule Clock     Function is for handling CPU clock time
**
******************************************************************************/

/* @section get cpu time *******************************************************
**
** Functions for retrieving CPU time
**
** @fdata [none]
**
** @nam3rule Diff Return time difference in seconds as a double
** @nam3rule Now Return current cpu clock ticks as a long integer
** @nam3rule Seconds Return current cpu clock time in seconds as a long integer
**
** @argrule Diff starttime [ajlong] start time
** @argrule Diff nowtime [ajlong] current time
** @valrule Diff [double] CPU time in seconds
** @valrule Now [ajlong] CPU ticks
** @valrule Seconds [double] CPU time in seconds
**
** @fcategory use
**
******************************************************************************/

/* @func ajClockDiff ********************************************************
**
** Returns the cpu time in seconds between two clock values
**
** @param [r] starttime [ajlong] start time
** @param [r] nowtime [ajlong] current time
** @return [double] Total cpu clock time in seconds
**
******************************************************************************/

double ajClockDiff(ajlong starttime, ajlong nowtime)
{
    double x;

    x = (nowtime - starttime);

    return x/(double)CLOCKS_PER_SEC;
}




/* @func ajClockNow ************************************************************
**
** Returns the clock time as a long even for systems where the clock_t type
** is 4 bytes
**
** @return [ajlong] Total clock ticks
******************************************************************************/

ajlong ajClockNow(void)
{
    clock_t now;

    now = clock();

    if(now < timeClockSave)
    {
        /* ajUser("ajClockNow overflow now:%Ld "
               "timeClockSave:%Ld timeClockOverflow:%Ld",
               (ajlong)now, (ajlong)timeClockSave, timeClockOverflow); */
        if(!timeClockMax)
        {
            if(sizeof(now) == 4)
                timeClockMax = UINT_MAX;
            else
                timeClockMax = UINT_MAX;
        }
        
        
        timeClockOverflow += timeClockMax;
        /* ajUser("timeClockOverflow:%Ld timeClockMax:%Ld",
           timeClockOverflow, timeClockMax); */
    }
    
    timeClockSave = now;

    if(timeClockOverflow)
        return (timeClockOverflow + now);
    
    return now; 
}




/* @func ajClockSeconds ********************************************************
**
** Returns the cpu time in seconds since the start
**
** @return [double] Total cpu clock time in seconds
**
******************************************************************************/

double ajClockSeconds(void)
{
    double x;

    x = ajClockNow();

    return x/(double)CLOCKS_PER_SEC;
}




/* @datasection [none] cpu time internals ***********************************
**
** Function is for processing cpu time internals.
**
** @nam2rule Clock Cpu time processing
**
*/




/* @section reset **************************************************************
**
** Functions for CPU time memory cleanup
**
** @fdata [none]
**
** @nam3rule Reset Reset internals
**
** @valrule   *  [void] No return value
** 
** @fcategory misc
**
******************************************************************************/

/* @func ajClockReset **********************************************************
**
** Resets the clock time to zero
**
** @return [void]
******************************************************************************/

void ajClockReset(void)
{
    timeClockSave = clock();
    timeClockOverflow = 0;

    return;
}
