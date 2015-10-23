/* @include ajmath ************************************************************
**
** AJAX maths functions
**
** @author Copyright (C) 1998 Alan Bleasby
** @version $Revision: 1.16 $
** @modified $Date: 2012/12/07 09:56:21 $ by $Author: rice $
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

#ifndef AJMATH_H
#define AJMATH_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define AJM_E                  2.718281828459
#define AJM_PI                 3.141592653589
#define AJM_PI_2               1.570796326794
#define AJM_PI_4               0.785398163397
#define AJM_1_PI               0.318309886183
#define AJM_2_PI               0.636619772367




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




#define AJMAX(a,b) ((a > b) ? a : b)
#define AJMIN(a,b) ((a < b) ? a : b)




/*
** Prototype definitions
*/

float              ajCvtDegToRad(float degrees);
double             ajCvtGaussToProb(float mean, float sd, float score);
void               ajCvtPolToRec(float radius, float angle,
                                 float *x, float *y);
float              ajCvtRadToDeg(float radians);
size_t             ajCvtSposToPos(size_t len, ajlong ipos);
size_t             ajCvtSposToPosStart(size_t len, size_t imin, ajlong ipos);
float              ajMathGmean(const float *s, ajint n);
ajint              ajMathModulo(ajint a, ajint b);
ajint              ajRandomNumber(void);
double             ajRandomDouble(void);
void               ajRandomSeed(void);
void               ajCvtRecToPol(float x, float y,
                                 float *radius, float *angle);
ajint              ajRound(ajuint i, ajuint vround);
float              ajRoundFloat(float a, ajuint nbits);
unsigned long long ajMathCrc64(const AjPStr thys);
ajuint             ajMathCrc32(const AjPStr thys);
ajuint             ajNumLengthDouble(double dnumber);
ajuint             ajNumLengthFloat(float fnumber);
ajuint             ajNumLengthInt(ajlong inumber);
ajuint             ajNumLengthUint(ajulong inumber);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated float              ajRoundF(float a, ajint nbits);
__deprecated double             ajRandomNumberD(void);
__deprecated unsigned long long ajSp64Crc(const AjPStr thys);
__deprecated ajuint             ajSp32Crc(const AjPStr thys);
__deprecated ajint              ajPosMod(ajint a, ajint b);
__deprecated float              ajGeoMean(const float *s, ajint n);
__deprecated double             ajGaussProb(float mean, float sd, float score);
__deprecated ajuint             ajMathPosI(ajuint len, ajuint imin, ajint ipos);
__deprecated ajuint             ajMathPos(ajuint len, ajint ipos);
__deprecated float              ajDegToRad(float degrees);
__deprecated float              ajRadToDeg(float radians);
__deprecated void               ajPolToRec(float radius, float angle,
                                           float *x, float *y);
__deprecated void               ajRecToPol(float x, float y,
                                           float *radius, float *angle);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJMATH_H */
