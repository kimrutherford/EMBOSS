/******************************************************************************
** @source AJAX maths functions
**
** @author Copyright (C) 1998 Alan Bleasby
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

#include "ajax.h"

#include <math.h>
#include <time.h>
#include <float.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/time.h>
#endif



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
** @param [r] round [ajint] Rounding multiple.
** @return [ajint] Result.
******************************************************************************/

ajint ajRound(ajint i, ajint round)
{
    return round * ((ajint)(i+round-1)/round);
}




/* @func ajRoundF *************************************************************
**
** Rounds a floating point number to have bits free for cumulative addition
**
** @param [r] a [float] Float to round.
** @param [r] nbits [ajint] Number of bits to free.
** @return [float] Result.
******************************************************************************/

float ajRoundF(float a, ajint nbits)
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
    if(z > 0.5) w += 1.0;		/* round up ?*/
    if(z < -0.5) w -= 1.0;		/* round down? */

    b = ldexp(w, -bitsused);		/* divide by 2**n */
    c = ldexp(b, i);	                /* divide by the orig. power of two */

    /*  ajDebug("\najRoundF(%.10e) c: %.10e bitsused: %d\n", a, c, bitsused);
	ajDebug("       x: %f i: %d y: %f w: %.1f\n", x, i, y, w);*/

    return (float) c;
}




/* @func ajRecToPol  **********************************************************
**
** Converts cartesian co-ordinates to polar
**
** @param [r] x [float] X co-ordinate
** @param [r] y [float] Y co-ordinate
** @param [w] radius [float*] Radius
** @param [w] angle [float*] Angle
** @return [void]
******************************************************************************/
void ajRecToPol(float x, float y, float *radius, float *angle)
{
    *radius = (float) sqrt((double)(x*x+y*y));
    *angle  = (float) ajRadToDeg((float)atan2((double)y,(double)x));

    return;
}




/* @func ajPolToRec  **********************************************************
**
** Converts polar co-ordinates to cartesian
**
** @param [r] radius [float] Radius
** @param [r] angle [float] Angle
** @param [w] x [float*] X co-ordinate
** @param [w] y [float*] Y co-ordinate
** @return [void]
******************************************************************************/

void ajPolToRec(float radius, float angle, float *x, float *y)
{
    *x = radius*(float)cos((double)ajDegToRad(angle));
    *y = radius*(float)sin((double)ajDegToRad(angle));

    return;
}




/* @func ajDegToRad  **********************************************************
**
** Converts degrees to radians
**
** @param [r] degrees [float] Degrees
** @return [float] Radians
******************************************************************************/

float ajDegToRad(float degrees)
{
    return degrees*(float)(AJM_PI/180.0);
}




/* @func ajRadToDeg   *********************************************************
**
** Converts radians to degrees
**
** @param [r] radians [float] Radians
** @return [float] Degrees
******************************************************************************/

float ajRadToDeg(float radians)
{
    return radians*(float)(180.0/AJM_PI);
}




/* @func ajGaussProb   ********************************************************
**
** Returns a probability given a Gaussian distribution
**
** @param [r] mean [float] mean
** @param [r] sd [float] sd
** @param [r] score [float] score
** @return [double] probability
******************************************************************************/

double ajGaussProb(float mean, float sd, float score)
{
    return pow(AJM_E,(double)(-0.5*((score-mean)/sd)*((score-mean)/sd)))
	/ (sd * (float)2.0 * AJM_PI);
}




/* @func ajGeoMean   **********************************************************
**
** Calculate a geometric mean
**
** @param [r] s [const float*] array of values
** @param [r] n [ajint] number of values
** @return [float] geometric mean
******************************************************************************/

float ajGeoMean(const float *s, ajint n)
{
    float x;
    ajint i;

    for(i=0,x=1.0;i<n;++i) x*=s[i];

    return (float)pow((double)x,(double)(1.0/(float)n));
}




/* @func ajPosMod   ***********************************************************
**
** Modulo always returning positive number
**
** @param [r] a [ajint] value1
** @param [r] b [ajint] value2
** @return [ajint] value1 modulo value2
******************************************************************************/

ajint ajPosMod(ajint a, ajint b)
{
    ajint t;

    if(b<=0)
	ajFatal("ajPosMod given non-positive divisor");
    t=a%b;

    return (t<0) ? t+b : t;
}




/* @func ajRandomSeed *********************************************************
**
** Seed for the ajRandomNumberD routine
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
	/* Needs looking at to try to get usec resolution */
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
        if(i >= 0) aj_rand_poly[i] = floor(AjRandomXmod*x);
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
******************************************************************************/

ajint ajRandomNumber(void)
{
    double td;
    ajint  rn;
    
    td = floor(ajRandomNumberD()*32768.0);
    rn = (ajint) td;

    return rn;
}




/* @func ajRandomNumberD ******************************************************
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
******************************************************************************/

double ajRandomNumberD(void)
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
    while(!aj_rand_other);

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




/* @func ajSp32Crc ************************************************************
**
** Calculates the SwissProt style CRC32 checksum for a protein sequence.
** This seems to be a bit reversal of a standard CRC32 checksum.
**
** @param [r] seq [const AjPStr] Sequence as a string
** @return [ajuint] CRC32 checksum.
** @@
******************************************************************************/

ajuint ajSp32Crc(const AjPStr seq)
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
    ajDebug("CRC32 calculated %08lX\n", crc);

    return (ajuint) crc;
}


/* @funcstatic spcrc32gen *****************************************************
**
** Generates data for a CRC32 calculation in a static data structure.
**
** @return [void]
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



/* @func ajSp64Crc *********************************************************
**
** Calculate 64-bit crc
**
** @param [r] thys [const AjPStr] sequence
**
** @return [unsigned long long] 64-bit CRC
******************************************************************************/

unsigned long long ajSp64Crc(const AjPStr thys)
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

/* @func ajMathPos ************************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** @param [r] len [ajuint] String length.
** @param [r] ipos [ajint] Position (0 start, negative from the end).
** @return [ajuint] string position between 0 and (length minus 1).
** @@
******************************************************************************/

ajuint ajMathPos(ajuint len, ajint ipos)
{
    return ajMathPosI(len, 0, ipos);
}




/* @func ajMathPosI ***********************************************************
**
** Converts a position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position.
** Usually this is the start position when the end of a range
** is being tested.
**
** @param [r] len [ajuint] maximum length.
** @param [r] imin [ajuint] Start position (0 start, no negative values).
** @param [r] ipos [ajint] Position (0 start, negative from the end).
** @return [ajuint] string position between 0 and (length minus 1).
** @@
******************************************************************************/

ajuint ajMathPosI(ajuint len, ajuint imin, ajint ipos)
{
    ajuint jpos;

    if(ipos < 0)
	jpos = len + ipos;
    else
	jpos = ipos;

    if(jpos >= len)
	jpos = len - 1;

    if(jpos < imin)
	jpos = imin;

    return jpos;
}


/* @func ajNumLengthDouble ****************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] dnumber [double] Double precision value
** @return [ajuint] Number of digits
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


/* @func ajNumLengthFloat ****************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] fnumber [float] Single precision value
** @return [ajuint] Number of digits
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


/* @func ajNumLengthInt ****************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] inumber [ajlong] Integer
** @return [ajuint] Number of digits
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



/* @func ajNumLengthUint ****************************************************
**
** Returns the length of a number written as an integer
**
** @param [r] inumber [ajulong] Unsigned integer
** @return [ajuint] Number of digits
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


