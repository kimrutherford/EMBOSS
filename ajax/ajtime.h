#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtime_h
#define ajtime_h
#include <time.h>

/* @data AjPTime **************************************************************
**
** AJAX time and data data structure. The same structure is used for both.
**
** @alias AjPDate
**
** @attr time [struct tm] Time in C standard structure
** @attr format [const char*] Format string if specified
** @attr uppercase [AjBool] Convert to upper case on output
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSTime {
  struct tm time;
  const char *format;
  AjBool uppercase;
  char Padding[4];
} AjOTime, AjODate;

#define AjPTime AjOTime*
#define AjPDate AjODate*




/*
** Prototype definitions
*/

double        ajTimeDiff(const AjPTime thys, const AjPTime newtime);
void          ajTimeExit(void);
AjBool        ajTimeSetLocal(AjPTime thys, const time_t timer);

AjPTime       ajTimeNewDayFmt(const char *timefmt,
			      ajint mday, ajint mon, ajint year) ;
AjPTime       ajTimeNewToday (void);
AjPTime       ajTimeNewTodayFmt (const char* timefmt);

void          ajTimeDel(AjPTime *thys);

const AjPTime ajTimeRefToday (void);
const AjPTime ajTimeRefTodayFmt(const char* timefmt);

void          ajTimeTrace (const AjPTime thys);
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
__deprecated AjPTime       ajTimeToday (void);
__deprecated AjPTime       ajTimeTodayF (const char* timefmt);
__deprecated const AjPTime ajTimeTodayRef (void);
__deprecated const AjPTime ajTimeTodayRefF (const char* timefmt);
__deprecated AjPTime       ajTimeSet(const char *timefmt,
				     ajint mday, ajint mon, ajint year) ;
__deprecated time_t        ajTimeMake(const AjPTime thys);
__deprecated AjBool        ajTimeLocal(const time_t timer, AjPTime thys);

#endif

#ifdef __cplusplus
}
#endif
