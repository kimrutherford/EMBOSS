#ifdef __cplusplus
extern "C"
{
#endif




/******************************************************************************
**
** AJAX regular expression functions
**
******************************************************************************/

#ifndef ajreg_h
#define ajreg_h

#include "ajax.h"
#include "pcre_config.h"
#include "pcre_internal.h"
#include "pcreposix.h"

#define AJREG_OVECSIZE 30

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
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSRegexp {
    real_pcre *pcre;
    pcre_extra *extra;
    int *ovector;
    const char* orig;
    int ovecsize;
    char Padding[4];
} AjORegexp;

#define AjPRegexp AjORegexp*




/*
** Prototype definitions
*/

/* constructors */

AjPRegexp ajRegComp (const AjPStr exp);
AjPRegexp ajRegCompC (const char* exp);

AjPRegexp ajRegCompCase (const AjPStr exp);
AjPRegexp ajRegCompCaseC (const char* exp);

/* execute expression match */

AjBool ajRegExec (AjPRegexp prog, const AjPStr str);
AjBool ajRegExecC (AjPRegexp prog, const char* str);

/* test substrings */

ajint  ajRegLenI (const AjPRegexp rp, ajint isub);
ajint  ajRegOffset (const AjPRegexp rp);
ajint  ajRegOffsetI (const AjPRegexp rp, ajint isub);

/* get substrings */

AjBool ajRegPre (const AjPRegexp rp, AjPStr* dest);
AjBool ajRegPost (const AjPRegexp rp, AjPStr* post);
AjBool ajRegPostC (const AjPRegexp rp, const char** post);
AjBool ajRegSubI (const AjPRegexp rp, ajint isub, AjPStr* dest);

/* destructor */

void   ajRegFree (AjPRegexp* pexp);
void   ajRegTrace (const AjPRegexp exp);

void   ajRegExit (void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
