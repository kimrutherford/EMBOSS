/* @source ajsort *************************************************************
**
** AJAX sort functions
**
** @author Copyright (C) 2001 Alan Bleasby
** @version $Revision: 1.18 $
** @modified $Date: 2012/03/28 21:11:23 $ by $Author: mks $
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

#include "ajsort.h"




/* @func ajSortFloatDecI ******************************************************
**
** Based on an array of floats, sort an ajint element array.
**
** @param [r] a [const float*] Array of floats used in sort tests
** @param [u] p [ajuint*] Array of unsigned ints to be sorted
**                       depending on floats.
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortFloatDecI(const float *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]<a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortIntDecI ********************************************************
**
** Based on an array of ints, sort an ajint element array.
**
** @param [r] a [const ajint*] Array of ints used in sort tests
** @param [u] p [ajuint*] Array of ints to be sorted depending on ajints.
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortIntDecI(const ajint *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]<a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortUintDecI *******************************************************
**
** Based on an array of ints, sort an ajuint element array.
**
** @param [r] a [const ajuint*] Array of unsigned ints used in sort tests
** @param [u] p [ajuint*] Array of ints to be sorted depending on unsigned ints
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSortUintDecI(const ajuint *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]<a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortFloatIncI ******************************************************
**
** Based on an array of floats, sort (ascending) an ajint element array.
**
** @param [r] a [const float*] Array of floats used in sort tests
** @param [u] p [ajuint*] Array of ints to be sorted depending on floats.
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortFloatIncI(const float *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]>=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortIntIncI ********************************************************
**
** Based on an array of ints, sort (ascending) a second ajint element array.
**
** @param [r] a [const ajint*] Array of ints used in sort tests
** @param [u] p [ajuint*] Array of ints to be sorted depending on ints.
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortIntIncI(const ajint *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]>=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortUintIncI *******************************************************
**
** Based on an array of uints, sort (ascending) a second ajint element array.
**
** @param [r] a [const ajuint*] Array of ints used in sort tests
** @param [u] p [ajuint*] Array of uints to be sorted depending on uints.
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSortUintIncI(const ajuint *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]>=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSorttwoIntIncI *****************************************************
**
** Based on an array of ints, sort (ascending) both this array and
** a second ajint element array.
**
** @param [u] a [ajint*] Array of ints used in sort tests and itself sorted
** @param [u] p [ajuint*] Second array of ints to be sorted
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSorttwoIntIncI(ajint *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajint t;
    ajuint u;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;

		u = p[j];
		p[j] = p[j+s];
		p[j+s] = u;
	    }

    return;
}




/* @func ajSorttwoUintIncI ****************************************************
**
** Based on an array of unsigned ints, sort (ascending) both this array and
** a second ajint element array.
**
** @param [u] a [ajuint*] Array of ints used in sort tests and itself sorted
** @param [u] p [ajuint*] Second array of ints to be sorted
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSorttwoUintIncI(ajuint *a, ajuint *p, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;
    ajuint u;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;

		u = p[j];
		p[j] = p[j+s];
		p[j+s] = u;
	    }

    return;
}




/* @func ajSortFloatDec *******************************************************
**
** Sort a float array.
**
** @param [u] a [float*] Array of floats to sort
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortFloatDec(float *a, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    float t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortIntDec  ********************************************************
**
** Sort an ajint array.
**
** @param [u] a [ajint*] Array of ints to sort
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortIntDec(ajint *a, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortUintDec  *******************************************************
**
** Sort an ajuint array.
**
** @param [u] a [ajuint*] Array of unsigned ints to sort
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSortUintDec(ajuint *a, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortFloatInc *******************************************************
**
** Sort a float array (ascending).
**
** @param [u] a [float*] Array of floats to sort
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortFloatInc(float *a, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    float t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortIntInc  ********************************************************
**
** Sort an ajint array (ascending)
**
** @param [u] a [ajint*] Array of ints to sort
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSortIntInc(ajint *a, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortUintInc  *******************************************************
**
** Sort an unsigned ajuint array (ascending)
**
** @param [u] a [ajuint*] Array of ints to sort
** @param [r] n [ajuint] Number of elements to sort
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSortUintInc(ajuint *a, ajuint n)
{
    ajuint s;
    ajuint i;
    ajint j;
    ajuint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajSortTwoIntIncI
** @rename ajSorttwoIntIncI
*/
__deprecated void ajSortTwoIntIncI(ajint *a, ajuint *p, ajuint n)
{
    ajSorttwoIntIncI(a, p, n);
    return;
}




/* @obsolete ajSortTwoUintIncI
** @rename ajSorttwoUintIncI
*/
__deprecated void ajSortTwoUintIncI(ajuint *a, ajuint *p, ajuint n)
{
    ajSorttwoUintIncI(a, p, n);
    return;
}

#endif /* AJ_COMPILE_DEPRECATED */
