/******************************************************************************
** @source AJAX format functions
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
** @version 1.0
** @modified Copyright (C) 2001 Alan Bleasby. Added ajFmtScanS functs
** @modified Copyright (C) 2003 Jon Ison. Added ajFmtScanC functs
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
#include "ajax.h"


typedef void (*Fmt_T) (ajint code, VALIST ap,
		       int put(int c, void *cl), void *cl,
		       const ajuint* flags, ajint width, ajint precision);

typedef void (*Fmt_S) (const char *fmt, const char **pos, VALIST ap, int width,
		       AjBool convert, AjBool *ok);


/*static Fmt_T  fmtRegister(ajint code, Fmt_T cvt);*/

/* @datastatic FmtPBuf ********************************************************
**
** Format definitions
**
** @alias FmtSVuf
** @alias FmtOBuf
**
** @attr buf [char*] buffer to write
** @attr bp [char*] next position in buffer
** @attr size [ajint] size of buffer from malloc
** @attr fixed [AjBool] if ajTrue, cannot reallocate
** @@
******************************************************************************/

typedef struct FmtSBuf
{
    char* buf;
    char* bp;
    ajint size;
    AjBool fixed;
} FmtOBuf;

#define FmtPBuf FmtOBuf*

#define pad(n,c) do { ajint nn = (n); \
                   while(nn-- > 0) \
                     put((c), cl); } while(0)

static AjBool c_notin(ajint c, const char *list);
static AjBool c_isin(ajint c, const char *list);
static ajint fmtVscan(const char *thys,const char *fmt,va_list ap);

static void scvt_uS(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_d(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_x(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_f(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_s(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_o(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_u(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_p(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_uB(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_c(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_b(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_z(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);


static void cvt_s(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_uB(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_uD(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_uF(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_uS(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_x(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_b(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_c(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_d(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_f(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_o(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_p(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);
static void cvt_u(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision);




#if defined(HAVE64)
static ajlong sc_long(const char *str);
static ajulong sc_ulong(const char *str);
static ajulong sc_hex(const char *str);
static ajulong sc_octal(const char *str);
#endif




/* @funcstatic c_isin *********************************************************
**
** Checks whether a character is in a set
**
** @param [r] c [ajint] character
** @param [r] list [const char*] set of characters
** @return [AjBool] ajTrue if character is in the set
** @@
******************************************************************************/

static AjBool c_isin(ajint c, const char *list)
{
    while(*list)
	if(c == (ajint)*(list++))
	    return ajTrue;

    return ajFalse;
}




/* @funcstatic c_notin ********************************************************
**
** Checks whether a character is not in a set
**
** @param [r] c [ajint] character
** @param [r] list [const char*] set of characters
** @return [AjBool] ajTrue if character is not in the set
** @@
******************************************************************************/

static AjBool c_notin(ajint c, const char *list)
{
    while(*list)
	if(c == (ajint)*(list++))
	    return ajFalse;

    return ajTrue;
}




/* @funcstatic cvt_s **********************************************************
**
** Conversion for %s to print char* text
**
** As for C RTL but prints null if given a null pointer.
**
** @param [r] code [ajint] Format code specified (usually s)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_s(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    char *str = va_arg(VA_V(ap), char *);

    (void) code;

    if(str)
	ajFmtPuts(str, strlen(str), put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("<null>", 6, put, cl, flags,
		  width, precision);

    return;
}




/* @funcstatic cvt_d **********************************************************
**
** Conversion for %d to print integer
**
** @param [r] code [ajint] Format code specified (usually d)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_d(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    long val = 0;
#if defined(HAVE64)
    ajlong hval = 0;
#endif
    ajulong m = 0;

    char buf[43];
    char *p = buf + sizeof buf;

    (void) code;

    if(flags['l'])
    {
	val  = (long) va_arg(VA_V(ap), long);
#if defined(HAVE64)
	hval = val;
#endif
    }
    else if(flags['L'])
    {
#if defined(HAVE64)
	hval = (ajlong) va_arg(VA_V(ap),ajlong);
	val  = hval;
#else
	val = (long) va_arg(VA_V(ap), ajlong);
	/*ajDebug("Warning: Use of %%Ld on a 32 bit model");*/
#endif
    }
    else if(flags['h'])
    {
	/* ANSI C converts short to ajint */
	val  = (long) va_arg(VA_V(ap), int);
#if defined(HAVE64)
	hval = val;
#endif
    }
    else
    {
	val  = (long) va_arg(VA_V(ap), int);
#if defined(HAVE64)
	hval = val;
#endif
    }


#if defined(HAVE64)
    if(hval == INT_MIN)
	m = INT_MAX + 1U;
    else if(hval < 0)
	m = -hval;
    else
	m = hval;

    do
	*--p = ajSysCastItoc((ajint)(m%10 + '0'));
    while((m /= 10) > 0);

    if(hval < 0)
	*--p = '-';
#else
    if(val == INT_MIN)
	m = INT_MAX + 1U;
    else if(val < 0)
	m = -val;
    else
	m = val;

    do
	*--p = ajSysCastItoc((ajint)(m%10 + '0'));
    while((m /= 10) > 0);

    if(val < 0)
	*--p = '-';
#endif

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_u **********************************************************
**
** Conversion for %u to print unsigned integer
**
** @param [r] code [ajint] Format code specified (usually u)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_u(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    unsigned long m = 0;
#if defined(HAVE64)
    ajulong  hm = 0;
#endif
    char buf[43];
    char *p;

    (void) code;

    p = buf + sizeof buf;

    if(flags['l'])
	m  = va_arg(VA_V(ap), unsigned long);
    else if(flags['h'])
	/* ANSI C converts short to int */
	m  = va_arg(VA_V(ap), unsigned int);
    else if(flags['L'])
    {
#if defined(HAVE64)
	hm = va_arg(VA_V(ap), ajulong);
#else
	m  = (unsigned long) va_arg(VA_V(ap), ajulong);
	/*ajDebug("Warning: Use of %%L on 32 bit model");*/
#endif
    }
    else
    {
	m  = va_arg(VA_V(ap), unsigned int);
#if defined(HAVE64)
	hm = m;
#endif
    }

#if !defined(HAVE64)
    do
	*--p = ajSysCastItoc(m%10 + '0');
    while((m /= 10) > 0);
#else
    do
	*--p = ajSysCastItoc((int)(hm%(ajulong)10 + '0'));
    while((hm /= (ajulong)10) > 0);
#endif
    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_o **********************************************************
**
** Conversion for %o to print unsigned integer as octal
**
** @param [r] code [ajint] Format code specified (usually o)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_o(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    unsigned long m = 0;
    char buf[43];
    char *p;
#if defined(HAVE64)
    ajulong hm = 0;
#endif

    (void) code;

    p = buf + sizeof buf;

    if(flags['l'])
	m = va_arg(VA_V(ap), unsigned long);
    if(flags['h'])
	/* ANSI C converts short to ajint */
	m = va_arg(VA_V(ap), unsigned int);
    else if(flags['L'])
    {
#if defined(HAVE64)
	hm = (ajulong) va_arg(VA_V(ap), ajulong);
#else
	m = (unsigned long) va_arg(VA_V(ap), ajulong);
	/*ajDebug("Warning: Use of %%Lo on a 32 bit model");*/
#endif
    }
    else
    {
	m = va_arg(VA_V(ap), unsigned int);
#if defined(HAVE64)
	hm = m;
#endif
    }

#if !defined(HAVE64)
    do
	*--p = ajSysCastItoc((m&0x7) + '0');
    while((m>>= 3) != 0);
#else
    do
	*--p = ajSysCastItoc((int)((hm&0x7) + '0'));
    while((hm>>= 3) != 0);
#endif

    if(flags['#'])
	*--p = '0';

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_x **********************************************************
*
** Conversion for %x to print unsigned integer as hexadecimal
**
** @param [r] code [ajint] Format code specified (usually x)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_x(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    unsigned long m = 0;
#if defined(HAVE64)
    ajulong hm = 0;
#endif
    char buf[43];
    char *p;

    p = buf + sizeof buf;

    if(flags['l'])
    {
	m = va_arg(VA_V(ap), unsigned long);
#if defined(HAVE64)
	hm = m;
#endif
    }
    else if(flags['h'])
    {
	/* ANSI C converts short to int */
	m = va_arg(VA_V(ap), unsigned int);
#if defined(HAVE64)
	hm = m;
#endif
    }
    else if(flags['L'])
    {
#if defined(HAVE64)
	hm = va_arg(VA_V(ap), ajulong);
#else
	m = (unsigned long) va_arg(VA_V(ap), ajulong);
	/*ajDebug("Warning: Use of %%Lx on a 32 bit model");*/
#endif
    }
    else
    {
	m = va_arg(VA_V(ap), unsigned int);
#if defined(HAVE64)
	hm = m;
#endif
    }
    if(code == 'X')
    {
#if !defined(HAVE64)
	do
	    *--p = "0123456789ABCDEF"[m&0xf];
	while((m>>= 4) != 0);
#else
	do
	    *--p = "0123456789ABCDEF"[hm&0xf];
	while((hm>>= 4) != 0);
#endif
    }
    else
    {
#if !defined(HAVE64)
	do
	    *--p = "0123456789abcdef"[m&0xf];
	while((m>>= 4) != 0);
#else
	do
	    *--p = "0123456789abcdef"[hm&0xf];
	while((hm>>= 4) != 0);
#endif
    }

    while(precision > buf+sizeof(buf)-p)
	*--p = '0';

    if(flags['#'])
    {
	*--p = 'x';
	*--p = '0';
    }

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_p **********************************************************
**
** Conversion for %p to print pointers of type void* as hexadecimal
**
** @param [r] code [ajint] Format code specified (usually p)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_p(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    unsigned long m;
    char buf[43];
    char *p;
    precision = INT_MIN;

    (void) code;

    m = (unsigned long)va_arg(VA_V(ap), void*);
    p = buf + sizeof buf;

    do
	*--p = "0123456789abcdef"[m&0xf];
    while((m>>= 4) != 0);

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_c **********************************************************
**
** Conversion for %c to print an integer (or a character)
** as a single character.
**
** Arguments passed in the variable part of an argument list are promoted
** by default, so char is always promoted to ajint by the time it reaches here.
**
** @param [r] code [ajint] Format code specified (usually c)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_c(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    ajuint minusflag = flags['-'];

    (void) code;
    (void) precision;

    if(width == INT_MIN)
	width = 0;

    if(width < 0)
    {
	minusflag = 1;
	width      = -width;
    }

    if(!minusflag)
	pad(width - 1, ' ');

    put(ajSysCastItouc(va_arg(VA_V(ap), int)), cl);

    if(minusflag)
	pad(width - 1, ' ');

    return;
}




/* @funcstatic cvt_f **********************************************************
**
** Conversion for %f to print a floating point number.
**
** Because it is generally faster than hand crafted code, the standard
** conversion in sprintf is used, and the resulting string is then
** written out.
**
** Precision is limited to 99 decimal places so it will fit in 2 characters
** of the format for sprintf.
**
** @param [r] code [ajint] Format code specified (usually f)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_f(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    char buf[DBL_MAX_10_EXP+1+1+99+1];

    if(precision < 0)
    {
	if(code == 'f') precision = 6;
	else if(code == 'g') precision = 6;
	else if(code == 'e') precision = 6;
	else precision = DBL_DIG;
    }

    if(code == 'g' && precision == 0)
	precision = 1;

    {
	/* use sprintf to convert to string */
	/* using code and precision */
	static char fmt[12] = "%.dd";
	ajint i = 2;

	assert(precision <= 99);

	if(precision > 9)
	    fmt[i++] = ajSysCastItoc((precision/10)%10 + '0');
	fmt[i++] = ajSysCastItoc(precision%10 + '0');
	fmt[i++] = ajSysCastItoc(code);
	fmt[i]   = '\0';

	sprintf(buf, fmt, va_arg(VA_V(ap), double));

	if(code == 'g')
            precision = 0;
    }

    /* now write string and support width */
    ajFmtPutd(buf, strlen(buf), put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_uS *********************************************************
**
** Conversion for %S to print a string
**
** @param [r] code [ajint] Format code specified (usually S)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uS(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    AjPStr str1;

   (void) code;

    str1 = va_arg(VA_V(ap), AjPStr);

    if(str1)
	ajFmtPuts(str1->Ptr, str1->Len, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("<null>", 6, put, cl, flags,
		  width, precision);

    return;
}




/* @funcstatic cvt_b **********************************************************
**
** Conversion for %b to print a boolean as a 1 letter code (Y or N)
**
** @param [r] code [ajint] Format code specified (usually b)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_b(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    AjBool bl;

   (void) code;

    bl = va_arg(VA_V(ap), AjBool);

    if(bl)
	ajFmtPuts("Y", 1, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("N", 1, put, cl, flags,
		  width, precision);

    return;
}




/* @funcstatic cvt_uB *********************************************************
**
** Conversion for %B to print a boolean as text (Yes or No)
**
** @param [r] code [ajint] Format code specified (usually B)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uB(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    AjBool bl;

   (void) code;

    bl = va_arg(VA_V(ap), AjBool);

    if(bl)
	ajFmtPuts("Yes", 3, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("No", 2, put, cl, flags,
		  width, precision);

    return;
}




/* @funcstatic cvt_uD *********************************************************
**
** Conversion for %D to print a datetime value
**
** @param [r] code [ajint] Format code specified (usually D)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uD(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    AjPTime timeobj;
    struct tm *mytime;
    int lenyr;

    char buf[280];
    char yr[280];

    (void) code;

    timeobj   =  va_arg(VA_V(ap), AjPTime);
    mytime = &timeobj->time;

    if(!timeobj)
    {
	ajFmtPuts("<null>", 6, put, cl, flags,
		  width, precision);
	return;
    }

    if(timeobj->format)
	strftime(buf,280, timeobj->format,mytime);
    else
    {
	/* Long-winded but gets around some compilers' %y warnings */
	strftime(yr,280,"%Y",mytime);
	lenyr = strlen(yr);
	memmove(yr,&yr[lenyr-2],3);
	strftime(buf,280, "%d/%m/", mytime);
	strcat(buf,yr);
    }

    if(timeobj->uppercase)
	ajCharFmtUpper(buf);

    ajFmtPuts(&buf[0], strlen(buf), put, cl, flags,
	      width, precision);

    return;
}




/* @funcstatic cvt_uF *********************************************************
**
** Conversion for %F to print a file object
**
** @param [r] code [ajint] Format code specified (usually F)
** @param [r] ap [VALIST] Original arguments at current position
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Standard - where to write results
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uF(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  const ajuint* flags, ajint width, ajint precision)
{
    AjPFile fil;

    (void) code;

    fil = va_arg(VA_V(ap), AjPFile);

    if(fil && fil->Name)
	ajFmtPuts(fil->Name->Ptr, fil->Name->Len, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("<null>", 6, put, cl, flags,
		  width, precision);

    return;
}




static const Except_T Fmt_Overflow = { "Formatting Overflow" };

/* @funclist Fmt_T ************************************************************
**
** Conversion functions called for each conversion code.
**
** Usually, code "x" will call "cvt_x" but there are exceptions. For example,
** floating point conversions all use cvt_f which sends everything to
** the standard C library. Also, cvt_d is used by alternative codes.
**
** @return [void]
******************************************************************************/

static Fmt_T cvt[256] =
{
 /*   0-  7 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*   8- 15 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  16- 23 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  24- 31 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  32- 39 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  40- 47 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  48- 55 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  56- 63 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  64- 71 */      0,     0,cvt_uB,     0,cvt_uD,     0,cvt_uF,     0,
 /*  72- 79 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  80- 87 */      0,     0,     0,cvt_uS,     0,     0,     0,     0,
 /*  88- 95 */  cvt_x,     0,     0,     0,     0,     0,     0,     0,
 /*  96-103 */      0,     0, cvt_b, cvt_c, cvt_d, cvt_f, cvt_f, cvt_f,
 /* 104-111 */      0,     0,     0,     0,     0,     0, cvt_d, cvt_o,
 /* 112-119 */  cvt_p,     0,     0, cvt_s,     0, cvt_u,     0,     0,
 /* 120-127 */  cvt_x,     0,     0,     0,     0,     0,     0,     0
};




/* @funclist Fmt_S ************************************************************
**
** Conversion functions called for each scan conversion code.
**
** Usually, code "x" will call "cvt_x" but there are exceptions. For example,
** floating point conversions all use cvt_f which sends everything to
** the standard C library. Also, cvt_d is used by alternative codes.
**
** @return [void]
******************************************************************************/

static Fmt_S scvt[256] =
{
 /*   0-  7 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*   8- 15 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  16- 23 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  24- 31 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  32- 39 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  40- 47 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  48- 55 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  56- 63 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  64- 71 */      0,     0,scvt_uB,     0,     0,     0,     0,     0,
 /*  72- 79 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  80- 87 */      0,     0,     0, scvt_uS,    0,     0,     0,     0,
 /*  88- 95 */ scvt_x,     0,     0,     0,     0,     0,     0,     0,
 /*  96-103 */      0,     0,scvt_b,scvt_c,scvt_d,scvt_f,scvt_f,scvt_f,
 /* 104-111 */      0,     0,     0,     0,     0,     0,scvt_d,scvt_o,
 /* 112-119 */ scvt_p,     0,     0,scvt_s,     0,scvt_u,     0,     0,
 /* 120-127 */ scvt_x,     0,scvt_z,     0,     0,     0,     0,     0
};




/* ****************************************************************************
**
** Legal flag characters for conversions:
**  '-' left justify value within field.
**  '+' always put a sign character '+' of '-' for a numeric value.
**  ' ' always put a sign character ' ' or '-' for a numeric value.
**  '0' pad width with zeroes rather than spaces
**  '#' alternative forms of the e,f,g,E,G formats
**      for C this also changes o,x,X but is not yet implemented here.
**
******************************************************************************/

static const char *Fmt_flags = "-+ 0#";




/* @funcstatic fmtOutC ********************************************************
**
** General output function to print a single character to a file
**
** @param [r] c [int] Character to be written
** @param [u] cl [void*] Output file - will be cast to FILE* internally
** @return [ajint] 0 on success
******************************************************************************/

static ajint fmtOutC(int c, void* cl)
{
    FILE *f = cl;

    return putc(c, f);
}




/* @funcstatic fmtInsert ******************************************************
**
** Inserts a character in a buffer, raises a Fmt_Overflow exception if
** the buffer is too small.
**
** @param [r] c [int] Character to be written
** @param [u] cl [void*] Output file - will be cast to FmtPBuf internally
** @return [ajint] 0 on success
******************************************************************************/

static ajint fmtInsert(int c, void* cl)
{
    FmtPBuf p;

    p = cl;

    if(p->bp >= p->buf + p->size)
    {
        if(p->fixed)
	  AJRAISE(Fmt_Overflow);

	AJRESIZE(p->buf, 2*p->size);
	p->bp = p->buf + p->size;
	p->size *= 2;
    }

    *p->bp++ = ajSysCastItoc(c);

    return c;
}




/* @funcstatic fmtAppend ******************************************************
**
** Appends a character to a buffer, resizing it if necessary
**
** @param [r] c [ajint] Character to be written
** @param [u] cl [void*] Output file - will be cast to FmtPBuf internally
** @return [ajint] 0 on success
******************************************************************************/

static ajint fmtAppend(ajint c, void* cl)
{
    FmtPBuf p;
    
    p = cl;

    if(p->bp >= p->buf + p->size)
    {
        if(p->fixed)
	  AJRAISE(Fmt_Overflow);

	AJRESIZE(p->buf, 2*p->size);
	p->bp = p->buf + p->size;
	p->size *= 2;
    }

    *p->bp++ = ajSysCastItoc(c);

    return c;
}




/* @func ajFmtPuts ************************************************************
**
** Format and emit the converted numeric (ajFmtPutd) or string
** (ajFmtPuts) in str[0..len-1] according to Fmt's defaults
** and the values of flags,width and precision. It is a c.r.e
** for str=null, len less than 0 or flags=null.
**
** @param [r] str [const char*] Text to write.
** @param [r] len [ajint] Text length.
** @param [f] put [int function] Standard function.
** @param [u] cl [void*] Standard - where to write the output
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @cre attempting to write over len chars to str
**
** @@
******************************************************************************/

void ajFmtPuts(const char* str, ajint len, int put(int c, void* cl), void* cl,
		const ajuint* flags, ajint width, ajint precision)
{
    ajuint minusflag = flags['-'];

    assert(len >= 0);
    assert(flags);

    if(width == INT_MIN)
	width = 0;

    if(width < 0)
    {
	minusflag = 1;
	width = -width;
    }

    if(precision >= 0 && precision < len)
	len = precision;

    if(!minusflag)
	pad(width - len, ' ');
    {
	ajint i;

	for(i = 0; i < len; i++)
	    put((unsigned char)*str++, cl);
    }

    if(minusflag)
	pad(width - len, ' ');

    return;
}




/* @func ajFmtFmt *************************************************************
**
** formats and emits the "..." arguments according to the format string fmt
**
** @param [f] put [ajint function] Standard function.
** @param [u] cl [void*] Standard - where to write the output
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
**
** @@
******************************************************************************/

void ajFmtFmt(ajint put(ajint c, void* cl), void* cl, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(put, cl, fmt, ap);
    va_end(ap);

    return;
}




/* @func ajFmtPrint ***********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stdout.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtPrint(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, stdout, fmt, ap);
    va_end(ap);

    return;
}




/* @func ajFmtVPrint **********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stdout.
**
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtVPrint(const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, stdout, fmt, ap);

    return;
}




/* @func ajFmtError  **********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stderr.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtError(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, stderr, fmt, ap);
    va_end(ap);

    return;
}




/* @func ajFmtVError  *********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stderr.
**
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtVError(const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, stderr, fmt, ap);

    return;
}




/* @func ajFmtPrintF  *********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [u] file [AjPFile] Output file.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtPrintF(AjPFile file, const char* fmt, ...)
{
    va_list ap;

    if(!file)
	return;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, file->fp, fmt, ap);
    va_end(ap);

    return;
}




/* @func ajFmtVPrintF *********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [u] file [AjPFile] Output file.
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtVPrintF(AjPFile file, const char* fmt, va_list ap)
{
    if(!file)
	return;

    ajFmtVfmt(fmtOutC, file->fp, fmt, ap);

    return;
}




/* @func ajFmtVPrintFp ********************************************************
**
** Format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [u] stream [FILE*] Output file.
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtVPrintFp(FILE* stream, const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, stream, fmt, ap);

    return;
}




/* @func ajFmtPrintFp *********************************************************
**
** format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [u] stream [FILE*] Output file.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtPrintFp(FILE* stream, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, stream, fmt, ap);
    va_end(ap);

    return;
}




/* @func ajFmtVPrintCL ********************************************************
**
** formats the "..." arguments into buf[1...size-1] according to fmt,
** appends a num character, and returns the length of buf. It is a
** c.r.e for size to be less than or equal to 0. Raises Fmt_Overflow
** if more than size-1 characters are emitted.
**
** @param [w] buf [char*] char string to be written to.
** @param [r] size [ajint] length of buffer
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list
** @return [ajint] number of characters written to buf.
** @@
******************************************************************************/

ajint ajFmtVPrintCL(char* buf, ajint size, const char* fmt, va_list ap)
{
    ajint len;

    len = ajFmtVfmtCL(buf, size, fmt, ap);

    return len;
}




/* @func ajFmtPrintCL *********************************************************
**
** formats the "..." arguments into buf[1...size-1] according to fmt,
** appends a num character, and returns the length of buf. It is a
** c.r.e for size to be lass than or equal to 0. Raises Fmt_Overflow
** if more than size-1 characters are emitted.
**
** @param [w] buf [char*] char string to be written to.
** @param [r] size [ajint] length of buffer
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
**
** @return [] [ajint] number of characters written to buf.
**
** @@
******************************************************************************/

ajint ajFmtPrintCL(char* buf, ajint size, const char* fmt, ...)
{
    va_list ap;
    ajint len;

    va_start(ap, fmt);
    len = ajFmtVfmtCL(buf, size, fmt, ap);
    va_end(ap);

    return len;
}




/* @func ajFmtStr *************************************************************
**
** Formats the "..." arguments into a New AjPStr according to fmt.
** It starts with an initial size of 20 then doubles until the
** fmt output fits.
**
** The caller is reponsible for deleting the AjPStr afterwards.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
**
** @return [AjPStr] fnew AjPStr with Ptr holding formatted chars
** @@
******************************************************************************/

AjPStr ajFmtStr(const char* fmt, ...)
{
    va_list ap;
#if defined(__amd64__) || defined (__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif
    ajint len = 32;
    AjPStr fnew;

    fnew = ajStrNewRes(len);
    va_start(ap, fmt);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap, ap);
#endif

    fnew->Len = ajFmtVfmtStrCL(&fnew->Ptr, 0, &fnew->Res, fmt, ap);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
	__va_copy(ap, save_ap);
#endif

    va_end(ap);

    return fnew;
}




/* @func ajFmtPrintS **********************************************************
**
** Formats the "..." arguments into an AjPStr according to fmt.
** If AjPStr is not large enough then if it is the only one i.e
** Use = 1 then increase till it fits. Else return 0 if it does not
** fit. If it fits return the address of the new AjPStr.
**
** @param [u] pthis [AjPStr*] String to be written too.
** @param [r] fmt [const char*] Format for string.
** @param [v] [...] Variable length argument list
**
** @return [AjPStr] Output string
**
** @error on unsuccessful writing return 0
**
** @@
** NOTE: unsafe may be best to pass a pointer to the pointer new
** as it passes back 0 if not able to be done
******************************************************************************/

AjPStr ajFmtPrintS(AjPStr* pthis, const char* fmt, ...)
{
    AjPStr thys;
    va_list ap;

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif

    va_start(ap, fmt);

    ajStrSetRes(pthis, 32);
    thys = *pthis;

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap, ap);
#endif

    thys->Len = ajFmtVfmtStrCL(&thys->Ptr, 0, &thys->Res, fmt, ap);

    va_end(ap);

    return thys;
}




/* @func ajFmtVPrintS *********************************************************
**
** Formats the "..." arguments into an AjPStr according to fmt.
** If AjPStr is not large enough then if it is the only one i.e
** Use = 1 then increase till it fits. Else return 0 if it does not
** fit. If it fits return the address of the new AjPStr.
**
** @param [u] pthis [AjPStr*] String to be written too.
** @param [r] fmt [const char*] Format for string.
** @param [v] ap [va_list] Variable length argument list
**
** @return [AjPStr] Output string
**
** @error on unsuccessful writing return 0
**
** @@
** NOTE: unsafe may be best to pass a pointer to the pointer new
** as it passes back 0 if not able to be done
******************************************************************************/

AjPStr ajFmtVPrintS(AjPStr* pthis, const char* fmt, va_list ap)
{
    AjPStr thys;

    ajStrSetRes(pthis, 32);
    thys = *pthis;

    thys->Len = ajFmtVfmtStrCL(&thys->Ptr, 0, &thys->Res, fmt, ap);

    return thys;
}




/* @func ajFmtPrintAppS *******************************************************
**
** Formats the "..." arguments and appends to an AjPStr according to fmt.
** If AjPStr is not large enough then if it is the only one i.e
** Use = 1 then increase till it fits. Else return 0 if it does not
** fit. If it fits return the address of the new AjPStr.
**
** @param [u] pthis [AjPStr*] String to be written too.
** @param [r] fmt [const char*] Format for string.
** @param [v] [...] Variable length argument list
**
** @return [AjPStr] Output string.
**
** @error on unsuccessful writing return 0
**
** @@
******************************************************************************/

AjPStr ajFmtPrintAppS(AjPStr* pthis, const char* fmt, ...)
{
    AjPStr thys;
    va_list ap;
#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif
    ajint len;

    va_start(ap, fmt);

    ajStrSetRes(pthis, 32);
    thys = *pthis;

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap, ap);
#endif

    len = ajFmtVfmtStrCL(&thys->Ptr, thys->Len, &thys->Res,
			      fmt, ap);

    thys->Len += len;

    va_end(ap);

    return thys;
}




/* @func ajFmtVfmtStrCL *******************************************************
**
** Same as ajFmtPrintCL but takes arguments from the list ap.
**
** @param [w] pbuf [char**] char string to be written to.
** @param [r] pos [ajint] position in buffer to start writing
** @param [u] size [ajuint*] allocated size of the buffer
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list.
**
** @return [] [ajint] number of characters written to buf.
**
** @@
******************************************************************************/

ajint ajFmtVfmtStrCL(char **pbuf, ajint pos, ajuint* size,
		     const char* fmt, va_list ap)
{
    FmtOBuf cl;

    assert(*pbuf);
    assert(*size > 0);
    assert(fmt);

    cl.buf   = *pbuf;
    cl.bp    = cl.buf + pos;
    cl.size  = *size;
    cl.fixed = ajFalse;

    ajFmtVfmt(fmtAppend, &cl, fmt, ap);
    fmtAppend(0, &cl);

    *size = cl.size;
    *pbuf = cl.buf;

    return cl.bp - cl.buf - 1 - pos;
}




/* @func ajFmtVfmtCL **********************************************************
**
** Same as ajFmtPrintCL but takes arguments from the list ap.
**
** @param [w] buf [char*] char string to be written to.
** @param [r] size [ajint] length of buffer
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list.
**
** @return [] [ajint] number of characters written to buf.
**
** @@
******************************************************************************/

ajint ajFmtVfmtCL(char* buf, ajint size, const char* fmt, va_list ap)
{
    FmtOBuf cl;

    assert(buf);
    assert(size > 0);
    assert(fmt);

    cl.buf   = cl.bp = buf;
    cl.size  = size;
    cl.fixed = ajTrue;

    ajFmtVfmt(fmtInsert, &cl, fmt, ap);
    fmtInsert(0, &cl);

    return cl.bp - cl.buf - 1;
}




/* @func ajFmtString **********************************************************
**
** formats the "..." arguments into a null-terminated string according to
** fmt and returns that string.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
**
** @return [char*] Output string.
**
** @@
******************************************************************************/

char* ajFmtString(const char* fmt, ...)
{
    char *str;
    va_list ap;

    assert(fmt);
    va_start(ap, fmt);
    str = ajFmtVString(fmt, ap);
    va_end(ap);

    return str;
}




/* @func ajFmtVString *********************************************************
**
** as ajFmtString but takes arguments from the list ap.
**
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list.
**
** @return [char*] Output string.
**
** @@
******************************************************************************/

char* ajFmtVString(const char* fmt, va_list ap)
{
    FmtOBuf cl;

    assert(fmt);

    cl.size = 256;
    cl.buf = cl.bp = AJALLOC(cl.size);
    cl.fixed = ajFalse;

    ajFmtVfmt(fmtAppend, &cl, fmt, ap);
    fmtAppend(0, &cl);

    return AJRESIZE(cl.buf, cl.bp - cl.buf);
}




/* @func ajFmtVfmt ************************************************************
**
** as ajFmtPrint but takes arguments from the list ap.
**
** @param [f] put [int function] Standard function
** @param [u] cl [void*] Where we are going to write the results
** @param [r] fmt [const char*] Format string
** @param [v] ap [va_list] Variable argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtVfmt(int put(int c, void* cl), void* cl, const char* fmt,
		va_list ap)
{
    assert(put);
    assert(fmt);
    assert(cl);

    while(*fmt)
    {
	if(*fmt != '%' || *++fmt == '%') /* %% just outputs '%' */
	    put((unsigned char)*fmt++, cl);
	else
	{
	    /* we have a % - get working on the format */
	    unsigned char c;
	    ajint flags[256];
	    ajint width = INT_MIN, precision = INT_MIN;

	    memset(flags, '\0', sizeof flags);

	    if(Fmt_flags)
	    {
		/* look for any conversion flags */
		unsigned char cc = *fmt;

		for( ; (int)cc && strchr(Fmt_flags, cc); cc = *++fmt)
		{
		    assert(flags[(int)cc] < 255);
		    flags[(int)cc]++;
		}
	    }

	    if(*fmt == '*' || isdigit((int)*fmt))
	    {
		ajint n;

		if(*fmt == '*')
		{
		    /* '*' width = ajint arg */
		    n = va_arg(ap, int);
		    assert(n != INT_MIN);
		    fmt++;
		}
		else
		    for(n = 0; isdigit((int)*fmt); fmt++)
		    {
			ajint d = *fmt - '0';
			assert(n <= (INT_MAX - d)/10);
			n = 10*n + d;
		    }

		width = n;
	    }

	    if(*fmt == '.' && (*++fmt == '*' || isdigit((int)*fmt)))
	    {
		ajint n;

		if(*fmt == '*')
		{			/* '*' precision = ajint arg */
		    n = va_arg(ap, int);
		    assert(n != INT_MIN);
		    fmt++;
		}
		else
		    for(n = 0; isdigit((int)*fmt); fmt++)
		    {
			ajint d = *fmt - '0';
			assert(n <= (INT_MAX - d)/10);
			n = 10*n + d;
		    }

		precision = n;
	    }

	    if(*fmt == 'l' || *fmt == 'L'|| *fmt == 'h')
	    {
		/* size modifiers */
		assert(flags[(int)*fmt] < 255); /* store as flags - */
		/* values do not clash */
		flags[(int)*fmt]++;
		fmt++;
	    }

	    /* finally, next character is the code */
	    c = *fmt++;

	    /* Calling funclist Fmt_T() */

	    if(!cvt[(int)c])
		ajDie("Bad format %%%c", c);
	    (*cvt[(int)c])(c, VA_P(ap), put, cl, (ajuint *)flags, width,
			   precision);
	}
    }

    return;
}




/* #funcstatic fmtRegister ****************************************************
**
** Registers 'newcvt' as the conversion routine for format code 'code'
**
** #param [r] code [ajint] value of char to be replaced
** #param [f] newcvt [Fmt_T] new routine for conversion
**
** #return [Fmt_T] old value
** ##
******************************************************************************/

/*
//static Fmt_T fmtRegister(ajint code, Fmt_T newcvt)
//{
//    Fmt_T old;
//
//    assert(0 < code && code < (ajint)(sizeof(cvt)/sizeof(cvt[0])));
//    old = cvt[code];
//    cvt[code] = newcvt;
//
//    return old;
//}
//
*/


/* @func ajFmtPutd ************************************************************
**
** Given a string containing a number in full, converts it using the width
** and precision values.
**
** @param [r] str [const char*] Text to write.
** @param [r] len [ajint] Text length.
** @param [f] put [int function] Standard function.
** @param [u] cl [void*] Standard - where to write the output
** @param [r] flags [const ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

void ajFmtPutd(const char* str, ajint len, int put(int c, void* cl), void* cl,
	       const ajuint* flags, ajint width, ajint precision)
{
    ajint sign;

    ajuint minusflag = flags['-'];

    assert(str);
    assert(len >= 0);
    assert(flags);

    if(width == INT_MIN)
	width = 0;

    if(width < 0)
    {
	minusflag = 1;
	width = -width;
    }


    if(len > 0 && (*str == '-' || *str == '+'))
    {
	sign = *str++;
	len--;
    }
    else if(flags['+'])
	sign = '+';
    else if(flags[' '])
	sign = ' ';
    else
	sign = 0;

    {
	ajint n;
	ajint j=0;

	if(precision < 0)
	    precision = 1;

	if(len < precision)
	    n = precision;
	/*else if(precision == 0 && len == 1 && str[0] == '0')
	    n = 0;*/
	else
	    n = len;

	if(sign)
	    n++;

	if(flags['#'] && flags['0'])
	{
	    /* make space for the padding */
	    if(*str == '0' && *(str+1) == 'x')
	    {
		put((unsigned char)*str++, cl);
		put((unsigned char)*str++, cl);
		j += 2;
	    }
	}

	if(flags['-'])
	{
	    if(sign)
		put(sign, cl);
	}
	else if(flags['0'])
	{
	    if(sign)
		put(sign, cl);
	    pad(width - n, '0');
	}
	else
	{
	    pad(width - n, ' ');
	    if(sign)
		put(sign, cl);
	}

	/* pad for precision - should be turned off for %g */
	pad(precision - len, '0');

	{
	    ajint i;

	    for(i = j; i < len; i++)
		put((unsigned char)*str++, cl);
	}

	if(minusflag)
	    pad(width - n, ' ');
    }

    return;
}




/* @func ajFmtPrintSplit ******************************************************
**
** Block and print a string. String is split at given delimiters
**
** @param [u] outf [AjPFile] output stream
** @param [r] str [const AjPStr] text to write
** @param [r] prefix [const char *] prefix string
** @param [r] len [ajint] maximum span
** @param [r] delim [const char *] delimiter string
** @return [void]
** @@
******************************************************************************/

void ajFmtPrintSplit(AjPFile outf, const AjPStr str,
		     const char *prefix, ajint len,
		     const char *delim)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr tmp       = NULL;
    AjPStr tmp2      = NULL;

    ajint    n = 0;
    ajint    l = 0;
    ajint    c = 0;

    if(!outf)
	return;

    token = ajStrNew();
    tmp   = ajStrNewC("");

    handle = ajStrTokenNewC(str,delim);

    while(ajStrTokenNextParse(&handle,&token))
    {
	if(!c)
	    ajFmtPrintF(outf,"%s",prefix);

	if((l=n+ajStrGetLen(token)) < len)
	{
	    if(c++)
		ajStrAppendC(&tmp," ");

	    ajStrAppendS(&tmp,token);

	    if(c!=1)
		n = ++l;
	    else
		n = l;
	}
	else
	{
	    ajFmtPrintF(outf,"%S\n",tmp);
	    ajStrAssignS(&tmp,token);
	    ajStrAppendC(&tmp," ");
	    n = ajStrGetLen(token) + 1;
	    c = 0;
	}
    }

    if(c)
	ajFmtPrintF(outf,"%S\n",tmp);
    else
    {
	n = ajStrGetLen(tmp);
	ajStrAssignSubS(&tmp2,tmp,0,n-2);
	ajFmtPrintF(outf,"%s%S\n",prefix,tmp2);
	ajStrDel(&tmp2);
    }


    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);

    return;
}




/* @func ajFmtScanS ***********************************************************
**
** Scan a string according to fmt and load the ... variable pointers
** Like C function sscanf.
**
** @param [r] thys [const AjPStr] String.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [ajint] number of successful conversions
** @@
******************************************************************************/

ajint ajFmtScanS(const AjPStr thys, const char* fmt, ...)
{
    va_list ap;
    ajint   n;
#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif

    va_start(ap, fmt);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap,ap);
#endif

    n = fmtVscan(thys->Ptr,fmt,ap);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(ap,save_ap);
#endif


    va_end(ap);

    return n;
}




/* @func ajFmtScanC  **********************************************************
**
** Scan a string according to fmt and load the ... variable pointers
** Like C function sscanf.
**
** @param [r] thys [const char*] String.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [ajint] number of successful conversions
** @@
******************************************************************************/

ajint ajFmtScanC(const char* thys, const char* fmt, ...)
{
    va_list ap;
    ajint   n;
#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif

    va_start(ap, fmt);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap,ap);
#endif

    n = fmtVscan(thys,fmt,ap);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(ap,save_ap);
#endif


    va_end(ap);

    return n;
}




/* @func ajFmtScanF **********************************************************
**
** Scan an AjPFile object according to fmt and load the ... variable pointers
** Like C function fscanf.
**
** @param [u] thys [AjPFile] Input file object
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [ajint] number of successful conversions
** @@
******************************************************************************/

ajint ajFmtScanF(AjPFile thys, const char* fmt, ...)
{
    va_list ap;
    ajint   n;
    FILE* file;


#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif

    if(!thys)
	return 0;

    file = ajFileGetFileptr(thys);

    va_start(ap, fmt);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap,ap);
#endif

    n = fscanf(file,fmt,ap);

#if defined(__amd64__) || defined(__EM64T__) || \
    defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(ap,save_ap);
#endif


    va_end(ap);

    return n;
}




/* @funcstatic fmtVscan *******************************************************
**
** Scan a string according to fmt and load the va_list variable pointers
**
** @param [r] thys [const char*] String.
** @param [r] fmt [const char*] Format string.
** @param [v] ap [va_list] Variable length argument list
** @return [ajint] number of successful conversions
** @@
******************************************************************************/

static ajint fmtVscan(const char *thys,const char *fmt,va_list ap)
{
    ajint n;
    const char *p;
    const char *q;
    static const char *wspace = " \n\t";
    AjBool convert      = ajTrue;
    AjBool ok           = ajTrue;
    ajint width = 0;
    ajint v     = 0;
    ajint d     = 0;

    n = 0;

    /*  we update it as a pointer */
    p = (const char*) thys;
    q = fmt;

    while(*p && *q)
    {
	/* Ignore all whitespace */
	if(c_isin((ajint)*p,wspace))
	{
	    ++p;
	    continue;
	}

	if(c_isin((ajint)*q,wspace))
	{
	    ++q;
	    continue;
	}

	/* If *q isn't '%' then it must match *p */
	if(*q != '%')
	{
	    if(*q!=*p)
		break;
	    else
	    {
		++p;
		++q;
		continue;
	    }
	}

	/* Check for %% */
	if(*(++q)=='%')
	{
	    if(*p!='%')
		break;
	    else
	    {
		++p;
		++q;
		continue;
	    }
	}


	/*
	**  *p now points to a string character to be matched
	**  *q points to first character after fmt '%'
	*/

	/* Check for %* format */
	convert = ajTrue;
	if(*q=='*')
	{
	    ++q;
	    convert = ajFalse;
	}

	/* If *q is a numeral then calculate the width else set to INT_MIN */
	if(isdigit((int)*q))
	{
	    for(v=0;isdigit((int)*q);++q)
	    {
		d = *q - '0';
		v = 10*v + d;
	    }
	    width = v;
	}
	else
	    width = INT_MIN;

	/* Just ignore size modifier for now */
	if(*q== 'l' || *q== 'L'|| *q== 'h')
	    ++q;

	/* *q is the conversion function to call */
	ok = ajTrue;

	/* Calling funclist Fmt_S() */
	if(!scvt[(int)*q])
	    ajDie("Bad scan format %%%c", q);
	(*scvt[(int)*q])(q,&p,VA_P(ap),width,convert,&ok);

	if(!ok)
	    break;

	if(convert)
	    ++n;

	/*
	**  p will already have been incremented by the convert function
	**  so just increment q
	*/
	++q;
    }

    return n;
}




/* @funcstatic scvt_uS ********************************************************
**
** Conversion for %S to load a string
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_uS(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    AjPStr *val = NULL;
    static const char *wspace=" \n\t";
    ajint c = 0;

    (void) fmt;				/* make it used */

    p = *pos;

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width;++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (AjPStr *) va_arg(VA_V(ap), AjPStr *);
	    ajStrAssignSubC(val,p,0,q-p-1);
	}

	*pos = q;
	*ok = ajTrue;
    }

    return;
}




/* @funcstatic scvt_d *********************************************************
**
** Conversion for %d to load an integer
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_d(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    long *val    = NULL;
    ajlong *hval = NULL;
    static const char *wspace=" \n\t";
    static const char *dig="+-0123456789";
    ajint c=0;
    AjPStr t = NULL;
    long  n   = 0;
    ajlong hn = 0;
    char  flag;

    p = *pos;
    flag = *(fmt-1);

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (long *) va_arg(VA_V(ap), long *);
	    else
		hval = (ajlong *) va_arg(VA_V(ap), ajlong *);

	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(flag!='L')
		sscanf(ajStrGetPtr(t),"%ld",&n);

	    else			/* flag == 'L' define hn */
	    {
#if defined(HAVE64)
		hn = sc_long(ajStrGetPtr(t));
#else
		val = hval;
		sscanf(ajStrGetPtr(t),"%ld",&n);
		hn = n;
		/*ajDebug("Warning: Use of %%Ld on a 32 bit model");*/
#endif
	    }
	    ajStrDel(&t);

	    if(flag=='h')
		*(short*)val = (short)n;
	    else if(flag=='l')
		*(long*)val = n;
	    else if(flag=='L')
		*(ajlong*)hval = hn;
	    else
		*(int*)val = (int)n;
	}

	*pos = q;
	*ok = ajTrue;
    }
    return;
}




/* @funcstatic scvt_x *********************************************************
**
** Conversion for %x to load an unsigned hexadecimal
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_x(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    unsigned long *val = NULL;
    ajulong *hval      = NULL;
    static const char *wspace=" \n\t";
    static const char *dig="0123456789abcdefABCDEFx";
    ajint c = 0;
    AjPStr t  = NULL;
    unsigned long  n = 0;
    ajulong hn       = 0;
    char  flag;

    p = *pos;
    flag=*(fmt-1);

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (unsigned long *) va_arg(VA_V(ap), unsigned long *);
	    else
		hval = (ajulong *) va_arg(VA_V(ap), ajulong *);

	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(flag!='L')
	    {
		if(sscanf(ajStrGetPtr(t),"%lx",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}
	    }
	    else			/* flag == 'L' define hn */
	    {
#if defined(HAVE64)
		hn = sc_hex(ajStrGetPtr(t));
#else
		val = (unsigned long *) hval;

		if(sscanf(ajStrGetPtr(t),"%lx",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}


		hn = n;
		/*ajDebug("Warning: Use of %%Lx on a 32 bit model");*/
#endif
	    }
	    ajStrDel(&t);

	    if(flag=='h')
		*(unsigned short*)val = (unsigned short)n;
	    else if(flag=='l')
		*(unsigned long*)val = n;
	    else if(flag=='L')
		*(ajulong*)hval = hn;
	    else
		*(unsigned int*)val = (unsigned int)n;
	}

	*pos = q;
	*ok = ajTrue;
    }

    return;
}




/* @funcstatic scvt_f *********************************************************
**
** Conversion for %f to load a float/double
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_f(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    double *val;
    float  *fval;
    static const char *wspace = " \n\t";
    static const char *dig = "+-0123456789.eE";
    ajint c=0;
    AjPStr t = NULL;
    double  n = (double)0.;
    float   fn = 0.;
    char  flag;

    p = *pos;
    flag = *(fmt-1);

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(flag=='l')
	    {
		val = (double*) va_arg(VA_V(ap), double *);

		if(sscanf(ajStrGetPtr(t),"%lf",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}

		*(double *)val = n;
	    }
	    else
	    {
		fval = (float*) va_arg(VA_V(ap), float *);

		if(sscanf(ajStrGetPtr(t),"%f",&fn)!=1)
		{
		    ajStrDel(&t);

		    return;
		}

		*(float *)fval = fn;
	    }

	    ajStrDel(&t);
	}

	*pos = q;
	*ok = ajTrue;
    }

    return;
}




/* @funcstatic scvt_s *********************************************************
**
** Conversion for %s to load a char *
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_s(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    char *val = NULL;
    static const char *wspace = " \n\t";
    ajint c = 0;
    AjPStr t = NULL;

    (void) fmt;				/* make it used */

    p = *pos;

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width;++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (char *) va_arg(VA_V(ap), char *);
	    ajStrAssignSubC(&t,p,0,q-p-1);
	    strcpy(val,ajStrGetPtr(t));
	    ajStrDel(&t);
	}

	*pos = q;
	*ok = ajTrue;
    }

    return;
}




/* @funcstatic scvt_o *********************************************************
**
** Conversion for %o to load an unsigned octal
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_o(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    unsigned long *val = NULL;
    ajulong  *hval = NULL;
    static const char *wspace = " \n\t";
    static const char *dig = "01234567";
    ajint c = 0;
    AjPStr t  = NULL;
    unsigned long  n = 0;
    ajulong hn       = 0;
    char  flag;

    p = *pos;
    flag = *(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (unsigned long *) va_arg(VA_V(ap), unsigned long *);
	    else
		hval = (ajulong *)  va_arg(VA_V(ap), ajulong *);

	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(flag!='L')
	    {
		if(sscanf(ajStrGetPtr(t),"%lo",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}
	    }
	    else			/* flag == 'L' define hn */
	    {
#if defined(HAVE64)
		hn = sc_octal(ajStrGetPtr(t));
#else
		val = (unsigned long *) hval;

		if(sscanf(ajStrGetPtr(t),"%lo",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}

		hn = n;
		/*ajDebug("Warning: Use of %%Lo on a 32 bit model");*/
#endif
	    }

	    if(flag=='h')
		*(unsigned short*)val = (unsigned short)n;
	    else if(flag=='l')
		*(unsigned long*)val = n;
	    else if(flag=='L')
		*(ajulong*)hval = hn;
	    else
		*(unsigned int*)val = (unsigned int)n;
	}

	*pos = q;
	*ok = ajTrue;
    }

    ajStrDel(&t);

    return;
}




/* @funcstatic scvt_u *********************************************************
**
** Conversion for %u to load an unsigned integer
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_u(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    unsigned long *val = NULL;
    ajulong *hval      = NULL;
    static const char *wspace = " \n\t";
    static const char *dig = "+0123456789";
    ajint c=0;
    AjPStr t = NULL;
    unsigned long n = 0;

    ajulong hn = 0;
    char  flag;

    p = *pos;
    flag = *(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (unsigned long *) va_arg(VA_V(ap), unsigned long *);
	    else
		hval = (ajulong *)  va_arg(VA_V(ap), ajulong *);

	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(flag!='L')
	    {
		if(sscanf(ajStrGetPtr(t),"%lu",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}
	    }
	    else			/* flag == 'L' define hn */
	    {
#if defined(HAVE64)
		hn = sc_ulong(ajStrGetPtr(t));
#else
		val = (unsigned long *) hval;

		if(sscanf(ajStrGetPtr(t),"%lu",&n)!=1)
		{
		    ajStrDel(&t);

		    return;
		}

		hn = n;
		/*ajDebug("Warning: Use of %%Lu on a 32 bit model");*/
#endif
	    }


	    if(flag=='h')
		*(unsigned short*)val = (unsigned short)n;
	    else if(flag=='l')
		*(unsigned long*)val = n;
	    else if(flag=='L')
		*(ajulong*)hval = hn;
	    else
		*(unsigned int*)val = (unsigned int)n;
	}

	*pos = q;
	*ok = ajTrue;
    }

    ajStrDel(&t);

    return;
}




/* @funcstatic scvt_p *********************************************************
**
** Conversion for %p to load a pointer of type void * as hexadecimal
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_p(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    void **val;
    static const char *wspace = " \n\t";
    static const char *dig = "0123456789abcdefABCDEFx";
    ajint c = 0;
    AjPStr t = NULL;
    unsigned long n = 0;

    (void) fmt;				/* make it used */

    p = *pos;

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (void **) va_arg(VA_V(ap), void **);
	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(sscanf(ajStrGetPtr(t),"%lx",&n)!=1)
		return;

	    *val = (void *)n;
	}

	*pos = q;
	*ok = ajTrue;
    }

    ajStrDel(&t);

    return;
}




/* @funcstatic scvt_uB ********************************************************
**
** Conversion for %B to load a boolean (integer or YyNnTtFf)
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_uB(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q     = NULL;
    AjBool *val = NULL;
    static const char *wspace = " \n\t";
    static const char *dig = "+-0123456789";
    static const char *tr = "YyTt";
    static const char *fa = "NnFf";
    ajint c = 0;
    AjPStr t = NULL;
    AjBool n = ajFalse;

    (void) fmt;				/* make it used */

    p = *pos;

    *ok = ajFalse;

    q = p;

    if(!strncmp(q,"Yes",3))
    {
	*pos = q+3;

	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajTrue;
	}

	*ok = ajTrue;

	return;
    }

    if(!strncmp(q,"No",2))
    {
	*pos = q+2;

	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajFalse;
	}

	*ok = ajTrue;

	return;
    }


    if(c_isin((int)*q,tr) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajTrue;
	}

	*pos = ++q;
	*ok = ajTrue;

	return;
    }

    if(c_isin((int)*q,fa) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajFalse;
	}

	*pos = ++q;
	*ok = ajTrue;

	return;
    }


    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    ajStrAssignSubC(&t,p,0,q-p-1);
	    sscanf(ajStrGetPtr(t),"%d",&n);

	    if(n)
		*(AjBool*)val = ajTrue;
	    else
		*(AjBool*)val = ajFalse;

	    ajStrDel(&t);
	}

	*pos = q;
	*ok = ajTrue;
    }

    return;
}




/* @funcstatic scvt_c *********************************************************
**
** Conversion for %c to load a character
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_c(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    char *val = NULL;
    char n = '\0';

    (void) fmt;				/* make it used */

    p = *pos;
    q = p;

    *ok = ajFalse;

    if(!(width==INT_MIN || width==1))
	return;

    if(convert)
    {
	n = *q;
	val = (char *) va_arg(VA_V(ap), char *);
	*val = n;
    }


    *pos = ++q;
    *ok  = ajTrue;

    return;
}




/* @funcstatic scvt_b *********************************************************
**
** Conversion for %B to load a boolean (YyNnTtFf)
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_b(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    AjBool *val = NULL;
    static const char *tr = "YyTt";
    static const char *fa = "NnFf";

    (void) fmt;				/* make it used */

    *ok = ajFalse;

    p = *pos;
    q = p;

    if(c_isin((int)*q,tr) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajTrue;
	}

	*pos = ++q;
	*ok = ajTrue;

	return;
    }

    if(c_isin((int)*q,fa) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajFalse;
	}

	*pos = ++q;
	*ok = ajTrue;
    }

    return;
}




/* @funcstatic scvt_z *********************************************************
**
** Conversion for %z to load a char **
**
** @param [r] fmt [const char*] Format string at conv char posn
** @param [w] pos [const char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_z(const char *fmt, const char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    const char *p;
    const char *q;
    char **val = NULL;
    static const char *wspace = " \n\t";
    ajint c = 0;
    AjPStr t = NULL;

    (void) fmt;				/* make it used */
 
    p = *pos;

    *ok = ajFalse;

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width;++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (char **) va_arg(VA_V(ap), char **);
	    ajStrAssignSubC(&t,p,0,q-p-1);

	    if(!*val)
		*val = ajCharNewRes(ajStrGetLen(t)+1);

	    strcpy(*val,ajStrGetPtr(t));
	    ajStrDel(&t);
	}

	*pos = q;
	*ok = ajTrue;
    }

    return;
}




#if defined(HAVE64)
/* @funcstatic sc_long ********************************************************
**
** Load a 64 bit long from a char*
**
** @param [r] str [const char*] long number
** @return [ajlong] result
** @@
******************************************************************************/

static ajlong sc_long(const char *str)
{
    ajlong v = 0;
    ajint d;
    const char *p;
    char c;

    p = str;

    while((c=*(p++)))
    {
	d = c - '0';
	v = (ajlong)10*v + (ajlong)d;
    }

    return v;
}




/* @funcstatic sc_ulong *******************************************************
**
** Load a 64 bit unsigned long from a char*
**
** @param [r] str [const char*] long number
** @return [ajulong] result
** @@
******************************************************************************/

static ajulong sc_ulong(const char *str)
{
    ajulong v = 0;
    ajint d;
    const char *p;
    char c;

    p = str;

    while((c=*(p++)))
    {
	d = c - '0';
	v = (ajulong)10*v + (ajulong)d;
    }

    return v;
}




/* @funcstatic sc_hex *********************************************************
**
** Load a 64 bit unsigned long from a char* hexadecimal
**
** @param [r] str [const char*] long hex number
** @return [ajulong] result
** @@
******************************************************************************/

static ajulong sc_hex(const char *str)
{
    ajulong v = 0;
    ajint d;
    const char *p;
    char c;

    p = str+2;

    while((c=toupper((int)*(p++))))
    {
	if(c>='0' && c<='9')
	    d = c - '0';
	else
	    d = c - 'A' + 10;

	v = (ajulong)16*v + (ajulong)d;
    }

    return v;
}




/* @funcstatic sc_octal *******************************************************
**
** Load a 64 bit unsigned long from a char* octal
**
** @param [r] str [const char*] long hex number
** @return [ajulong] result
** @@
******************************************************************************/

static ajulong sc_octal(const char *str)
{
    ajulong v = 0;
    ajint d;
    const char *p;
    char c;

    p = str+1;

    while((c=toupper((int)*(p++))))
    {
	d = c - '0';
	v = (ajulong)8*v + (ajulong)d;
    }

    return v;
}
#endif
