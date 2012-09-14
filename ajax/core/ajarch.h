/* @include ajarch ************************************************************
**
** AJAX master architecture-specific include file
**
** @author Copyright (C) 1998 Peter Rice and Alan Bleasby
** @version $Revision: 1.26 $
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
**
******************************************************************************/

#ifndef AJARCH_H
#define AJARCH_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include <sys/types.h>
#include <stdio.h>





/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

#if defined(AJ_Linux64) || defined(AJ_Solaris64) || defined(AJ_IRIX64)  \
    || defined(AJ_OSF164) || defined(AJ_FreeBSD64)
#define HAVE64
#endif




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

#if !defined(AJ_LinuxLF) && !defined(AJ_SolarisLF) && !defined(AJ_IRIXLF) \
    && !defined(AJ_AIXLF) && !defined(AJ_HPUXLF) && !defined(AJ_MACOSXLF) \
    && !defined(AJ_FreeBSDLF) && !defined(__CYGWIN__) && !defined(WIN32)




/* @datatype ajint ************************************************************
**
** AJAX Signed Integer
**
** @attr typedef [int] Value
** @@
******************************************************************************/
typedef int ajint;




/* @datatype ajlong ***********************************************************
**
** AJAX Long Signed Integer
**
** @attr typedef [long] Value
** @@
******************************************************************************/
typedef long ajlong;




/* @datatype ajuint ***********************************************************
**
** AJAX Unsigned Integer
**
** @attr typedef [unsigned int] Value
** @@
******************************************************************************/
typedef unsigned int ajuint;




/* @datatype ajshort **********************************************************
**
** AJAX Short Signed Integer
**
** @attr typedef [short] Value
** @@
******************************************************************************/
typedef short ajshort;




/* @datatype ajushort *********************************************************
**
** AJAX Short Unsigned Integer
**
** @attr typedef [unsigned short] Value
** @@
******************************************************************************/
typedef unsigned short ajushort;




/* @datatype ajulong **********************************************************
**
** AJAX Long Unsigned Integer
**
** @attr typedef [unsigned long] Value
** @@
******************************************************************************/
typedef unsigned long ajulong;
#endif

#ifdef AJ_LinuxLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_LinuxLF */

#ifdef AJ_FreeBSDLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_FreeBSDLF */

#ifdef AJ_SolarisLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_SolarisLF */

#ifdef AJ_HPUXLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#if !defined(HPUX64PTRS)
#define fopen(a,b) (FILE*)fopen64(a,b)
#endif /* !HPUX64PTRS */
#endif /* AJ_HPUXLF */

#ifdef AJ_IRIXLF
#define HAVE64
typedef int ajint;
typedef off64_t ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long ajulong;
#define ftell(a) ftell64(a)
#define fseek(a,b,c) fseek64(a,b,c)
#endif /* AJ_IRIXLF */

#ifdef AJ_AIXLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_AIXLF */

#ifdef AJ_MACOSXLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* AJ_MACOSXLF */

#ifdef __CYGWIN__
#define __int64 long long
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define fseek(a,b,c) fseeko(a,b,c)
#endif /* __CYGWIN__ */

#ifdef WIN32
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define fseek(a,b,c) _fseeki64(a,b,c)
#endif /* WIN32 */




/* @datatype AjBool ***********************************************************
**
** Boolean data type
**
** Used to store true (ajTrue) and false (ajFalse) values.
**
** ajFalse is defined as zero, and the data type is equivalent to "ajint".
**
** For definitions, macros AJTRUE and AJFALSE are also defined.
**
** On output, conversion code "%b" writes "Y" or "N"
** while conversion code "%B" writes "Yes" or "No".
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjBool;




/* @datatype AjStatus *********************************************************
**
** Status code returned with bit fields.
**
** Intended as a general return code for functions, but currently unused
** because AjBool is enough. Indicates OK, info,
** warning, error and fatal returns
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjStatus;




/* @datatype AjEnum ***********************************************************
**
** Undocumented
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjEnum;




/* @datatype AjMask ***********************************************************
**
** Undocumented
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjMask;




/* @datatype AjInt4 ***********************************************************
**
** 4 bytes integer
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjInt4;




/* @datatype AjIntArray *******************************************************
**
** Array of integers
**
** @attr typedef [ajint*] Value
** @@
******************************************************************************/

typedef ajint* AjIntArray;




/* @datatype AjFloatArray *****************************************************
**
** Array of floats
**
** @attr typedef [float*] Value
** @@
******************************************************************************/

typedef float* AjFloatArray;




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/*
** End of prototype definitions
*/





#endif /* !AJARCH_H */
