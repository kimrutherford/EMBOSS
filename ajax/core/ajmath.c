/* @source ajmath *************************************************************
**
** AJAX maths functions
**
** @author Copyright (C) 1998 Alan Bleasby
** @version $Revision: 1.35 $
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

#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/time.h>
#endif

#include "ajmath.h"
#include "ajmess.h"
#include "ajstr.h"
#include "ajnam.h"



#define AjRandomXmod 1000009711.0
#define AjRandomYmod 33554432.0
#define AjRandomTiny 1.0e-17
#define AJCRC64LEN 256

static ajulong seqCrcTable[256];



static AjBool aj_rand_i = 0;
static ajint    aj_rand_index;
static double aj_rand_poly[101];
static double aj_rand_other;


static void spcrc32gen(void);
static void spcrc64calctab(unsigned long long *crctab);




/* @func ajRound **************************************************************
**
** Rounds an integer to be a multiple of a given number.
**
** @param [r] i [ajint] Integer to round.
** @param [r] vround [ajint] Rounding multiple.
** @return [ajint] Result.
**
** @release 1.0.0
******************************************************************************/

ajint ajRound(ajint i, ajint vround)
{
    return vround * ((ajint)(i+vround-1)/vround);
}




/* @func ajRoundFloat *********************************************************
**
** Rounds a floating point number to have bits free for cumulative addition
**
** @param [r] a [float] Float to round.
** @param [r] nbits [ajint] Number of bits to free.
** @return [float] Result.
**
** @release 6.2.0
******************************************************************************/

float ajRoundFloat(float a, ajint nbits)
{
    double w;
    double x;
    double y;
    double z;
    double b;
    double c;
    
    ajint i;
    ajint bitsused;

    /* save 16 bits for cumulative error */
    bitsused = FLT_MANT_DIG - nbits;
    /* usually leave 8 bits */
    if(bitsused < 8)
	bitsused = 8;

    /* a is between 0.5 and 1.0 */
    x = frexp(a, &i);
    /* i is the power of two */

    /* multiply by 2**n, convert to an integer, divide again */
    /* so we only keep n (or whatever) bits */

    y = ldexp(x, bitsused);		/* multiply by 2**n */
    z = modf(y, &w);		        /* change to an integer + remainder */

    if(z > 0.5)
        w += 1.0;		        /* round up ?*/

    if(z < -0.5)
        w -= 1.0;		        /* round down? */

    b = ldexp(w, -bitsused);		/* divide by 2**n */
    c = ldexp(b, i);	                /* divide by the orig. power of two */

    /*  ajDebug("\najRoundFloat(%.10e) c: %.10e bitsused: %d\n",
                a, c, bitsused);
	ajDebug("       x: %f i: %d y: %f w: %.1f\n", x, i, y, w);*/

    return (float) c;
}




/* @func ajCvtRecToPol ********************************************************
**
** Converts Cartesian coordinates to polar
**
** @param [r] x [float] X coordinate
** @param [r] y [float] Y coordinate
** @param [w] radius [float*] Radius
** @param [w] angle [float*] Angle
** @return [void]
**
** @release 6.2.0
******************************************************************************/

void ajCvtRecToPol(float x, float y, float *radius, float *angle)
{
    *radius = (float) sqrt((double)(x*x+y*y));
    *angle  = (float) ajCvtRadToDeg((float)atan2((double)y,(double)x));

    return;
}




/* @func ajCvtPolToRec  *******************************************************
**
** Converts polar coordinates to Cartesian
**
** @param [r] radius [float] Radius
** @param [r] angle [float] Angle
** @param [w] x [float*] X coordinate
** @param [w] y [float*] Y coordinate
** @return [void]
**
** @release 6.2.0
******************************************************************************/

void ajCvtPolToRec(float radius, float angle, float *x, float *y)
{
    *x = radius*(float)cos((double)ajCvtDegToRad(angle));
    *y = radius*(float)sin((double)ajCvtDegToRad(angle));

    return;
}





/* @func ajCvtDegToRad  *******************************************************
**
** Converts degrees to radians
**
** @param [r] degrees [float] Degrees
** @return [float] Radians
**
** @release 6.2.0
******************************************************************************/

float ajCvtDegToRad(float degrees)
{
    return degrees*(float)(AJM_PI/180.0);
}




/* @func ajCvtRadToDeg   ******************************************************
**
** Converts radians to degrees
**
** @param [r] radians [float] Radians
** @return [float] Degrees
**
** @release 6.2.0
******************************************************************************/

float ajCvtRadToDeg(float radians)
{
    return radians*(float)(180.0/AJM_PI);
}




/* @func ajCvtGaussToProb *****************************************************
**
** Returns a probability given a Gaussian distribution
**
** @param [r] mean [float] mean
** @param [r] sd [float] sd
** @param [r] score [float] score
** @return [double] probability
**
** @release 6.2.0
******************************************************************************/

double ajCvtGaussToProb(float mean, float sd, float score)
{
    return pow(AJM_E,(double)(-0.5*((score-mean)/sd)*((score-mean)/sd)))
	/ (sd * (float)2.0 * AJM_PI);
}




/* @func ajMathGmean **********************************************************
**
** Calculate a geometric mean
**
** @param [r] s [const float*] array of values
** @param [r] n [ajint] number of values
** @return [float] geometric mean
**
** @release 6.2.0
******************************************************************************/

float ajMathGmean(const float *s, ajint n)
{
    float x;
    ajint i;

    for(i=0,x=1.0;i<n;++i) x*=s[i];

    return (float)pow((double)x,(double)(1.0/(float)n));
}




/* @func ajMathModulo *********************************************************
**
** Modulo always returning positive number
**
** @param [r] a [ajint] value1
** @param [r] b [ajint] value2
** @return [ajint] value1 modulo value2
**
** @release 6.2.0
******************************************************************************/

ajint ajMathModulo(ajint a, ajint b)
{
    ajint t;

    if(b <= 0)
	ajFatal("ajMathModulo given non-positive divisor");

    t = a%b;

    return (t<0) ? t+b : t;
}




/* @func ajRandomSeed *********************************************************
**
** Seed for the ajRandomDouble routine
**
** Based on dprand and sdprand and used with the permission of the
** author....
** Copyright (C) 1992  N.M. Maclaren
** Copyright (C) 1992  The University of Cambridge
**
**  This software may be reproduced and used freely, provided that all
**  users of it agree that the copyright holders are not liable for any
**  damage or injury caused by use of this software and that this condition
**  is passed onto all subsequent recipients of the software, whether
**  modified or not.
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void ajRandomSeed(void)
{
    ajint ix;
    ajint iy;
    ajint iz;
    ajint i;
    
    double x = 0.0;
    ajint seed;
#ifndef WIN32
    struct timeval tv;
#endif
    AjPStr timestr = NULL;
    
    /*
     *  seed should be set to an integer between 0 and 9999 inclusive; a value
     *  of 0 will initialise the generator only if it has not already been
     *  done.
     */

    if(!aj_rand_i)
        aj_rand_i = 1;
    else
        return;

    if(ajNamGetValueC("timetoday", &timestr))
    {
	seed = 0;   /* always zero microseconds in a defined time */
	ajStrDel(&timestr);
    }
    else
    {
#ifndef WIN32
	gettimeofday(&tv,NULL);
	seed = (tv.tv_usec % 9999)+1;
#else
	/* Needs looking at to try to get micro second resolution */
	seed = (ajint) ((time(0) % 9999) + 1);
#endif
    }


    /*
     *  aj_rand_index must be initialised to an integer between 1 and 101
     *  inclusive, aj_rand_poly[0...100] to integers between 0 and 1000009710
     *  inclusive (not all 0), and aj_rand_other to a non-negative proper
     *  fraction with denominator 33554432.  It uses the Wichmann-Hill
     *  generator to do this.
     */

    ix = (seed >= 0 ? seed : -seed) % 10000 + 1;
    iy = 2*ix+1;
    iz = 3*ix+1;

    for(i = -11; i < 101; ++i)
    {
        if(i >= 0)
            aj_rand_poly[i] = floor(AjRandomXmod*x);

        ix = (171*ix) % 30269;
        iy = (172*iy) % 30307;
        iz = (170*iz) % 30323;
        x = ((double)ix)/30269.0+((double)iy)/30307.0+((double)iz)/30323.0;
        x = x-floor(x);
    }

    aj_rand_other = floor(AjRandomYmod*x)/AjRandomYmod;
    aj_rand_index = 0;

    return;
}




/* @func ajRandomNumber *******************************************************
**
** Generate a pseudo-random number between 0-32767
**
** @return [ajint] Random number
**
** @release 1.0.0
******************************************************************************/

ajint ajRandomNumber(void)
{
    double td;
    ajint  rn;
    
    td = floor(ajRandomDouble()*32768.0);
    rn = (ajint) td;

    return rn;
}




/* @func ajRandomDouble *******************************************************
**
** Generate a random number between 0-1.0
**
** Based on dprand and sdprand and used with the permission of the
** author....
** Copyright (C) 1992  N.M. Maclaren
** Copyright (C) 1992  The University of Cambridge
**
**  This software may be reproduced and used freely, provided that all
**  users of it agree that the copyright holders are not liable for any
**  damage or injury caused by use of this software and that this condition
**  is passed onto all subsequent recipients of the software, whether
**  modified or not.
**
** @return [double] Random number
**
** @release 6.2.0
******************************************************************************/

double ajRandomDouble(void)
{
    static double offset = 1.0/AjRandomYmod;
    static double xmod2  = 2.0*AjRandomXmod;
    static double xmod4  = 4.0*AjRandomXmod;

    ajint n;
    double x, y;

    /*
     *  This returns a uniform (0,1) random number, with extremely good
     *  uniformity properties.  It assumes that double precision provides
     *  at least 33 bits of accuracy, and uses a power of two base.
     */

    if(!aj_rand_i)
	ajRandomSeed();

    /*
     *  See [Knuth] for why this implements the algorithm described in the
     *  paper.
     *  Note that this code is tuned for machines with fast double precision,
     *  but slow multiply and divide; many, many other options are possible.
     */

    if((n = aj_rand_index-64) < 0)
	n += 101;

    x = aj_rand_poly[aj_rand_index]+aj_rand_poly[aj_rand_index];
    x = xmod4-aj_rand_poly[n]-aj_rand_poly[n]-x-x-aj_rand_poly[aj_rand_index];

    if(x <= 0.0)
    {
        if(x < -AjRandomXmod)
	    x += xmod2;

        if(x < 0.0)
	    x += AjRandomXmod;
    }
    else
    {
        if(x >= xmod2)
	{
            x = x-xmod2;

            if(x >= AjRandomXmod)
		x -= AjRandomXmod;
        }

        if(x >= AjRandomXmod)
	    x -= AjRandomXmod;
    }

    aj_rand_poly[aj_rand_index] = x;

    if(++aj_rand_index >= 101)
	aj_rand_index = 0;

    /*
     *  Add in the second generator modulo 1, and force to be non-zero.
     *  The restricted ranges largely cancel themselves out.
     */

    do
    {
        y = 37.0*aj_rand_other+offset;
        aj_rand_other = y-floor(y);
    }
    while(E_FPZERO(aj_rand_other,U_DEPS));

    if((x = x/AjRandomXmod+aj_rand_other) >= 1.0)
	x -= 1.0;


    return x+AjRandomTiny;
}




/* @funcstatic spcrc64calctab *************************************************
**
** Initialise the crc table.
** Polynomial x64+x4+x3+x1+1
**
** @param [w] crctab [unsigned long long*] CRC lookup table
**
** @return [void]
**
** @release 2.6.0
******************************************************************************/

static void spcrc64calctab(unsigned long long *crctab)
{
    unsigned long long v;
    ajint i;
    ajint j;

    for(i=0;i<AJCRC64LEN;++i)
    {
	v = (unsigned long long)i;
	for(j=0;j<8;++j)
	    if(v&1)
#ifndef WIN32
		v = 0xd800000000000000ULL ^ (v>>1);
#else
		v = 0xd800000000000000 ^ (v>>1);
#endif
	    else
		v >>= 1;
	crctab[i] = v;
    }
}




/* @func ajMathCrc32 **********************************************************
**
** Calculates the SwissProt style CRC32 checksum for a protein sequence.
** This seems to be a bit reversal of a standard CRC32 checksum.
**
** @param [r] seq [const AjPStr] Sequence as a string
** @return [ajuint] CRC32 checksum.
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ajMathCrc32(const AjPStr seq)
{
    register ajulong crc;
    ajint c;
    const char* cp;
    static ajint calls = 0;

    if(!calls)
    {
	spcrc32gen();
	calls = 1;
    }

    cp = ajStrGetPtr(seq);

    crc = 0xFFFFFFFFL;

    while( *cp )
    {
	c = toupper((ajint) *cp);
	crc = ((crc >> 8) & 0x00FFFFFFL) ^ seqCrcTable[ (crc^c) & 0xFF ];
	cp++;
    }

    ajDebug("ajMathCrc32 calculated %08lX\n", crc);

    return (ajuint) crc;
}




/* @funcstatic spcrc32gen *****************************************************
**
** Generates data for a CRC32 calculation in a static data structure.
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

static void spcrc32gen(void)
{
    ajulong crc;
    ajulong poly;
    ajint   i;
    ajint   j;

    poly = 0xEDB88320L;

    for(i=0; i<256; i++)
    {
	crc = i;

	for(j=8; j>0; j--)
	    if(crc&1)
		crc = (crc >> 1) ^ poly;
	    else
		crc >>= 1;

	seqCrcTable[i] = crc;
    }

    return;
}




/* @func ajMathCrc64 **********************************************************
**
** Calculate 64-bit crc
**
** @param [r] thys [const AjPStr] sequence
**
** @return [unsigned long long] 64-bit CRC
**
** @release 6.2.0
******************************************************************************/

unsigned long long ajMathCrc64(const AjPStr thys)
{
    static ajint initialised = 0;
    static unsigned long long crctab[AJCRC64LEN];
    unsigned long long crc;
    ajint i;
    ajint len;
    const char *p = NULL;
    

    if(!initialised)
    {
	spcrc64calctab(crctab);
	++initialised;
    }

#ifndef WIN32
    crc = 0ULL;
#else
    crc = 0U;
#endif
    p = ajStrGetPtr(thys);
    len = ajStrGetLen(thys);
    
    for(i=0;i<len;++i)
	crc = crctab[(crc ^ (unsigned long long)p[i]) & 0xff] ^ (crc>>8);

    return crc;
}




/* @func ajCvtSposToPos *******************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** @param [r] len [size_t] String length.
** @param [r] ipos [ajlong] Position (0 start, negative from the end).
** @return [size_t] string position between 0 and (length minus 1).
**
** @release 6.2.0
** @@
******************************************************************************/

size_t ajCvtSposToPos(size_t len, ajlong ipos)
{
    return ajCvtSposToPosStart(len, 0, ipos);
}




/* @func ajCvtSposToPosStart **************************************************
**
** Converts a position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position.
** Usually this is the start position when the end of a range
** is being tested.
**
** @param [r] len [size_t] maximum length.
** @param [r] imin [size_t] Start position (0 start, no negative values).
** @param [r] ipos [ajlong] Position (0 start, negative from the end).
** @return [size_t] string position between 0 and (length minus 1).
**
** @release 6.2.0
** @@
******************************************************************************/

size_t ajCvtSposToPosStart(size_t len, size_t imin, ajlong ipos)
{
    ajlong jpos;
    ajlong jmin = imin;
    ajlong jlen = len;

    if(ipos < 0)
	jpos = len + ipos;
    else
	jpos = ipos;

    if(jpos >= jlen)
	jpos = len - 1;

    if(jpos < jmin)
	jpos = imin;

    return (size_t) jpos;
}




/* @func ajNumLengthDouble ****************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] dnumber [double] Double precision value
** @return [ajuint] Number of digits
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajNumLengthDouble(double dnumber)
{
    double dnum;
    ajuint ilen = 1;
    double td = 0.;

    dnum = fabs(dnumber);

    if(dnum >= 10.0)
    {
	td = log10(dnum);
	ilen += (ajuint) td;
    }

    if(dnumber < 0.0)
	ilen++;

    return ilen;
}




/* @func ajNumLengthFloat *****************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] fnumber [float] Single precision value
** @return [ajuint] Number of digits
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajNumLengthFloat(float fnumber)
{
    double dnum;
    ajuint ilen = 1;
    double td = 0.;
    
    dnum = fabs((double)fnumber);


    if(dnum >= 10.0)
    {
	td = log10(dnum);
	ilen += (ajuint) td;
    }

    if(fnumber < 0.0)
	ilen++;

    return ilen;
}




/* @func ajNumLengthInt *******************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] inumber [ajlong] Integer
** @return [ajuint] Number of digits
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajNumLengthInt(ajlong inumber)
{
    ajuint ilen = 1;
    ajulong maxnum = ULONG_MAX/10;
    ajulong i;
    ajulong iabs;

    if(inumber < 0)
	iabs = -inumber;
    else
	iabs = inumber;

    if(!iabs)
	return ilen;

    if(inumber < 0)
	ilen++;		/* space for the sign */

    for(i=10;i<maxnum;i*=10)
    {
	if(iabs >= i)
	    ilen++;
	else
	    break;
    }

    return ilen;
}




/* @func ajNumLengthUint ******************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] inumber [ajulong] Unsigned integer
** @return [ajuint] Number of digits
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajNumLengthUint(ajulong inumber)
{
    ajuint ilen = 1;
    ajulong maxnum = ULONG_MAX/10;
    ajulong i;

    if(!inumber)
	return ilen;

    for(i=10;i<maxnum;i*=10)
    {
	if(inumber >= i)
	    ilen++;
	else
	    return ilen;
    }

    return ilen;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajRoundF
** @rename ajRoundFloat
*/

__deprecated float ajRoundF(float a, ajint nbits)
{
    return ajRoundFloat(a, nbits);
}




/* @obsolete ajRecToPol
** @rename ajCvtRecToPol
*/

__deprecated void ajRecToPol(float x, float y, float *radius, float *angle)
{
    ajCvtRecToPol(x, y, radius, angle);
    return;
}




/* @obsolete ajPolToRec
** @rename ajCvtPolToRec
*/

__deprecated void ajPolToRec(float radius, float angle, float *x, float *y)
{
    ajCvtPolToRec(radius, angle, x, y);
    return;
}




/* @obsolete ajDegToRad
** @rename ajCvtDegToRad
*/

__deprecated float ajDegToRad(float degrees)
{
    return degrees*(float)(AJM_PI/180.0);
}




/* @obsolete ajRadToDeg
** @rename ajCvtRadToDeg
*/

__deprecated float ajRadToDeg(float radians)
{
    return radians*(float)(180.0/AJM_PI);
}




/* @obsolete ajGaussProb
** @rename ajCvtGaussToProb
*/

__deprecated double ajGaussProb(float mean, float sd, float score)
{
    return pow(AJM_E,(double)(-0.5*((score-mean)/sd)*((score-mean)/sd)))
	/ (sd * (float)2.0 * AJM_PI);
}




/* @obsolete ajGeoMean
** @rename ajMathGmean
*/

__deprecated float ajGeoMean(const float *s, ajint n)
{
    return ajMathGmean(s, n);
}




/* @obsolete ajPosMod
** @rename ajMathModulo
*/

__deprecated ajint ajPosMod(ajint a, ajint b)
{
    return ajMathModulo(a, b);
}




/* @obsolete ajRandomNumberD
** @rename ajRandomDouble
*/

__deprecated double ajRandomNumberD(void)
{
    return ajRandomDouble();
}




/* @obsolete ajSp32Crc
** @rename ajMathCrc32
*/

__deprecated ajuint ajSp32Crc(const AjPStr seq)
{
    return ajMathCrc32(seq);
}




/* @obsolete ajMathPos
** @rename ajCvtSposToPos
*/

__deprecated ajuint ajMathPos(ajuint len, ajint ipos)
{
    return (ajuint) ajCvtSposToPosStart(len, 0, ipos);
}




/* @obsolete ajMathPosI
** @rename ajCvtSposToPosStart
*/

__deprecated ajuint ajMathPosI(ajuint len, ajuint imin, ajint ipos)
{
    return ajCvtSposToPosStart(len, imin, ipos);
}




/* @obsolete ajSp64Crc
** @rename ajMathCrc64
*/

__deprecated unsigned long long ajSp64Crc(const AjPStr thys)
{
    return ajMathCrc64(thys);
}
#endif
