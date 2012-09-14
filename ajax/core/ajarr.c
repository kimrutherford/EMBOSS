/* @source ajarr **************************************************************
**
** AJAX ARRAY functions
**
** These functions control all aspects of AJAX array utilities
**
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.37 $
** @modified Mar 12 1999 ajb First version
** @modified May 10 2000 ajb added dynamically allocated numeric arrays
** @modified $Date: 2011/11/08 15:07:45 $ by $Author: rice $
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


#include "ajlib.h"

#include "ajarr.h"
#include "ajmath.h"
#include "ajreg.h"

#include <stdio.h>
#include <sys/types.h>
#include <string.h>




#ifndef HAVE_MEMMOVE
/* @header memmove ***********************************************************
**
******************************************************************************/

static void* memmove (void *dst, const void* src, size_t len)
{
    return (void *)bcopy (src, dst, len);
}
#endif


#define RESERVED_SIZE 32

static ajlong arrAlloc     = 0;
static ajlong arr2dAlloc   = 0;
static ajlong arr3dAlloc   = 0;
static ajlong arrFree      = 0;
static ajlong arr2dFree      = 0;
static ajlong arr3dFree      = 0;
static ajlong arrFreeCount = 0;
static ajlong arr2dFreeCount = 0;
static ajlong arr3dFreeCount = 0;
static ajlong arrCount     = 0;
static ajlong arr2dCount     = 0;
static ajlong arr3dCount     = 0;
static ajlong arrTotal     = 0;
static ajlong arr2dTotal     = 0;
static ajlong arr3dTotal     = 0;
static ajlong arrResize    = 0;
static ajlong arr2dResize    = 0;
static ajlong arr3dResize    = 0;
static AjPRegexp floatRegNum = NULL;


static AjBool arrChararrResize(AjPChar *thys, ajuint elem);
static AjBool arrIntResize(AjPInt *thys, ajuint elem);
static AjBool arrInt2dResize(AjPInt2d *thys, ajuint elem);
static AjBool arrInt3dResize(AjPInt3d *thys, ajuint elem);
static AjBool arrFloatResize(AjPFloat *thys, ajuint elem);
static AjBool arrFloat2dResize(AjPFloat2d *thys, ajuint elem);
static AjBool arrFloat3dResize(AjPFloat3d *thys, ajuint elem);
static AjBool arrDoubleResize(AjPDouble *thys, ajuint elem);
static AjBool arrDouble2dResize(AjPDouble2d *thys, ajuint elem);
static AjBool arrDouble3dResize(AjPDouble3d *thys, ajuint elem);
static AjBool arrShortResize(AjPShort *thys, ajuint elem);
static AjBool arrShort2dResize(AjPShort2d *thys, ajuint elem);
static AjBool arrShort3dResize(AjPShort3d *thys, ajuint elem);
static AjBool arrLongResize(AjPLong *thys, ajuint elem);
static AjBool arrLong2dResize(AjPLong2d *thys, ajuint elem);
static AjBool arrLong3dResize(AjPLong3d *thys, ajuint elem);
static AjBool arrUintResize(AjPUint *thys, ajuint elem);
static AjBool arrUint2dResize(AjPUint2d *thys, ajuint elem);
static AjBool arrUint3dResize(AjPUint3d *thys, ajuint elem);




/* @func ajChararrNew *********************************************************
**
** Default constructor for empty AJAX character arrays.
**
** @return [AjPChar] Pointer to an empty character array structure
** @category new [AjPChar] Default constructor
**
** @release 2.0.0
** @@
******************************************************************************/

AjPChar ajChararrNew(void)
{
    AjPChar thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(char));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(char);

    return thys;
}




/* @func ajChararrNewRes ******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPChar] Pointer to an empty character array struct
**                   of specified size.
** @category new [AjPChar] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPChar ajChararrNewRes(ajuint size)
{
    AjPChar thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(char));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(char);

    return thys;
}




/* @func ajChararrDel *********************************************************
**
** Default destructor for AJAX character arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPChar*] Pointer to the char array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPChar] Default destructor
**
** @release 2.0.0
** @@
******************************************************************************/

void ajChararrDel(AjPChar *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajChararrDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajChararrGet *********************************************************
**
** Retrieve an element from an AJAX character array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPChar] Pointer to the char array.
** @param  [r] elem [ajuint] array element.
**
** @return [char] contents of array element
** @category cast [AjPChar] Retrieve a character from an array
**
** @release 2.0.0
** @@
******************************************************************************/

char ajChararrGet(const AjPChar thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad char array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajChararrPut *********************************************************
**
** Load a character array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPChar*] Pointer to the char array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [char] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPChar] Load a character array element
**
** @release 2.0.0
** @@
******************************************************************************/

AjBool ajChararrPut(AjPChar *thys, ajuint elem, char v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;

	(*thys)->Ptr[elem] = v;

	return ajFalse;
    }

    arrChararrResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @func ajChararrChararr *****************************************************
**
** Returns the current char* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPChar] Source array
** @return [char*] Current array pointer, or a null string if undefined.
** @category cast [AjPChar] Retrieve internal pointer
**
** @release 2.0.0
** @@
******************************************************************************/

char* ajChararrChararr(const AjPChar thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajChararrLen *********************************************************
**
** Get length of dynamic character array
**
** @param [r] thys [const AjPChar] Source array
** @return [ajuint] length
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ajChararrLen(const AjPChar thys)
{
    return thys->Len;
}




/* @func ajIntNew *************************************************************
**
** Default constructor for empty AJAX integer arrays.
**
** @return [AjPInt] Pointer to an empty integer array structure
** @category new [AjPInt] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPInt ajIntNew(void)
{
    AjPInt thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(ajint));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(ajint);

    return thys;
}




/* @func ajIntNewRes **********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPInt] Pointer to an empty integer array struct of specified size.
** @category new [AjPInt] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPInt ajIntNewRes(ajuint size)
{
    AjPInt thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(ajint));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(ajint);

    /*ajDebug("ajIntNewRes size %d*%d %d\n",
	    size, sizeof(ajint), size*sizeof(ajint));*/

    return thys;
}





/* @func ajIntDel *************************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPInt*] Pointer to the ajint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPInt] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajIntDel(AjPInt *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajIntDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajIntGet *************************************************************
**
** Retrieve an element from an AJAX integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPInt] Pointer to the ajint array.
** @param  [r] elem [ajuint] array element.
**
** @return [ajint] contents of array element
** @category cast [AjPInt] Retrieve an integer from an array
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajIntGet(const AjPInt thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad ajint array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajIntPut *************************************************************
**
** Load an integer array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [ajint] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPInt] Load an integer array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajIntPut(AjPInt *thys, ajuint elem, ajint v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    arrIntResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @func ajIntInc *************************************************************
**
** Increment an integer array element.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] elem [ajuint] array element.
**
** @return [void]
**
** @release 1.8.0
** @@
******************************************************************************/

void ajIntInc(AjPInt *thys, ajuint elem)
{
    if(!thys || !*thys || elem>(*thys)->Len)
	ajErr("Attempt to write to illegal array value %d\n",elem);


    ++(*thys)->Ptr[elem];

    return;
}




/* @func ajIntDec *************************************************************
**
** Decrement an integer array element.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] elem [ajuint] array element.
**
** @return [void]
**
** @release 1.8.0
** @@
******************************************************************************/

void ajIntDec(AjPInt *thys, ajuint elem)
{
    if(!thys || !*thys || elem>(*thys)->Len)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    --(*thys)->Ptr[elem];

    return;
}




/* @func ajUintNew ************************************************************
**
** Default constructor for empty AJAX unsigned integer arrays.
**
** @return [AjPUint] Pointer to an empty unsigned integer array structure
** @category new [AjPUint] Default constructor
**
** @release 4.1.0
** @@
******************************************************************************/

AjPUint ajUintNew(void)
{
    AjPUint thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(ajuint));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(ajuint);

    return thys;
}




/* @func ajUintNewRes *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPUint] Pointer to an empty unsigned integer array struct
**                   of specified size.
** @category new [AjPUint] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPUint ajUintNewRes(ajuint size)
{
    AjPUint thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(ajuint));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(ajuint);

    /*ajDebug("ajUintNewL size %d*%d %d\n",
	    size, sizeof(ajuint), size*sizeof(ajuint));*/

    return thys;
}




/* @func ajUintDel ************************************************************
**
** Default destructor for AJAX integer integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPUint*] Pointer to the ajuint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPUint] Default destructor
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUintDel(AjPUint *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajUintDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajUintGet ************************************************************
**
** Retrieve an element from an AJAX unsigned integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPUint] Pointer to the ajuint array.
** @param  [r] elem [ajuint] array element.
**
** @return [ajuint] contents of array element
** @category cast [AjPUint] Retrieve an integer from an array
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajUintGet(const AjPUint thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad ajuint array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajUintPut ************************************************************
**
** Load an unsigned integer array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint*] Pointer to the ajuint array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [ajuint] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPUint] Load an integer array element
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajUintPut(AjPUint *thys, ajuint elem, ajuint v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    arrUintResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @func ajUintInc ************************************************************
**
** Increment an unsigned integer array element.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint*] Pointer to the ajuint array.
** @param  [r] elem [ajuint] array element.
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUintInc(AjPUint *thys, ajuint elem)
{
    if(!thys || !*thys || elem>(*thys)->Len)
	ajErr("Attempt to write to illegal array value %d\n",elem);


    ++(*thys)->Ptr[elem];

    return;
}




/* @func ajUintDec ************************************************************
**
** Decrement an unsigned integer array element.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint*] Pointer to the ajuint array.
** @param  [r] elem [ajuint] array element.
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUintDec(AjPUint *thys, ajuint elem)
{
    if(!thys || !*thys || elem>(*thys)->Len)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    --(*thys)->Ptr[elem];

    return;
}




/* @funcstatic arrChararrResize ***********************************************
**
** Resize a character array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPChar*] Pointer to the char array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrChararrResize(AjPChar *thys, ajuint size)
{
    AjPChar p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize character array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajChararrResize %d (%d) -> %d (%d)\n",
	    (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajChararrNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(char));

    (*thys)->Len = size+1;


    ajChararrDel(&p);

    arrResize++;

    return ajTrue;
}




/* @funcstatic arrIntResize ***************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrIntResize(AjPInt *thys, ajuint size)
{
    AjPInt p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajIntResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajIntNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(ajint));

    (*thys)->Len = size+1;


    ajIntDel(&p);

    arrResize++;

    return ajTrue;
}




/* @func ajIntInt *************************************************************
**
** Returns the current ajint* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPInt] Source array
** @return [ajint*] Current array pointer, or a null string if undefined.
** @category cast [AjPInt] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

ajint* ajIntInt(const AjPInt thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajIntLen *************************************************************
**
** Get length of dynamic 1d ajint array
**
** @param [r] thys [const AjPInt] Source array
** @return [ajuint] length
**
** @release 1.0.0
** @@
******************************************************************************/

ajuint ajIntLen(const AjPInt thys)
{
    return thys->Len;
}




/* @funcstatic arrUintResize **************************************************
**
** Resize an unsigned integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint*] Pointer to the ajuint array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrUintResize(AjPUint *thys, ajuint size)
{
    AjPUint p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize unsigned integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajIntResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajUintNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(ajuint));

    (*thys)->Len = size+1;


    ajUintDel(&p);

    arrResize++;

    return ajTrue;
}




/* @func ajUintUint ***********************************************************
**
** Returns the current ajuint* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPUint] Source array
** @return [ajuint*] Current array pointer, or a null string if undefined.
** @category cast [AjPUint] Retrieve internal pointer
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint* ajUintUint(const AjPUint thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajUintLen ************************************************************
**
** Get length of dynamic 1d ajuint array
**
** @param [r] thys [const AjPUint] Source array
** @return [ajuint] length
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajUintLen(const AjPUint thys)
{
    return thys->Len;
}




/* @func ajFloatNew ***********************************************************
**
** Default constructor for empty AJAX float arrays.
**
** @return [AjPFloat] Pointer to an empty float array structure
** @category new [AjPFloat] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFloat ajFloatNew(void)
{
    AjPFloat thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(float));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(float);

    return thys;
}




/* @func ajFloatNewRes ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPFloat] Pointer to an empty float array struct of specified size.
** @category new [AjPFloat] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPFloat ajFloatNewRes(ajuint size)
{
    AjPFloat thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(float));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(float);

    return thys;
}




/* @func ajFloatDel ***********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPFloat*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFloat] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloatDel(AjPFloat *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajFloatDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajFloatGet ***********************************************************
**
** Retrieve an element from an AJAX float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPFloat] Pointer to the float array.
** @param  [r] elem [ajuint] array element.
**
** @return [float] contents of array element
** @category cast [AjPFloat] Retrieve a float from an array
**
** @release 1.0.0
** @@
******************************************************************************/

float ajFloatGet(const AjPFloat thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad float array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajFloatPut ***********************************************************
**
** Load a float array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat*] Pointer to the float array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPFloat] Load a float array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajFloatPut(AjPFloat *thys, ajuint elem, float v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;

	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    arrFloatResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}





/* @funcstatic arrFloatResize *************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat*] Pointer to the float array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrFloatResize(AjPFloat *thys, ajuint size)
{
    AjPFloat p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajFloatResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajFloatNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(float));

    (*thys)->Len = size+1;


    ajFloatDel(&p);

    arrResize++;

    return ajTrue;
}




/* @func ajFloatFloat *********************************************************
**
** Returns the current float* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPFloat] Source array
** @return [float*] Current array pointer, or a null string if undefined.
** @category cast [AjPFloat] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

float* ajFloatFloat(const AjPFloat thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajFloatLen ***********************************************************
**
** Get length of dynamic 1d float array
**
** @param [r] thys [const AjPFloat] Source array
** @return [ajuint] length
**
** @release 1.0.0
** @@
******************************************************************************/

ajuint ajFloatLen(const AjPFloat thys)
{
    return thys->Len;
}




/* @func ajDoubleNew **********************************************************
**
** Default constructor for empty AJAX double arrays.
**
** @return [AjPDouble] Pointer to an empty double array structure
** @category new [AjPDouble] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPDouble ajDoubleNew(void)
{
    AjPDouble thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(double));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(double);

    return thys;
}




/* @func ajDoubleNewRes *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPDouble] Pointer to an empty double array struct
**                     of specified size.
** @category new [AjPDouble] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPDouble ajDoubleNewRes(ajuint size)
{
    AjPDouble thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(double));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(double);

    return thys;
}




/* @func ajDoubleDel **********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPDouble*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPDouble] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajDoubleDel(AjPDouble *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajDoubleDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajDoubleGet **********************************************************
**
** Retrieve an element from an AJAX double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPDouble] Pointer to the double array.
** @param  [r] elem [ajuint] array element.
**
** @return [double] contents of array element
** @category cast [AjPDouble] Retrieve a double from an array
**
** @release 1.0.0
** @@
******************************************************************************/

double ajDoubleGet(const AjPDouble thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad double array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajDoublePut **********************************************************
**
** Load a double array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble*] Pointer to the double array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPDouble] Load a double array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajDoublePut(AjPDouble *thys, ajuint elem, double v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;

	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    arrDoubleResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @funcstatic arrDoubleResize ************************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble*] Pointer to the double array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrDoubleResize(AjPDouble *thys, ajuint size)
{
    AjPDouble p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajDoubleResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajDoubleNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(double));

    (*thys)->Len = size+1;


    ajDoubleDel(&p);

    arrResize++;

    return ajTrue;
}




/* @func ajDoubleDouble *******************************************************
**
** Returns the current double* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPDouble] Source array
** @return [double*] Current array pointer, or a null string if undefined.
** @category cast [AjPDouble] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

double* ajDoubleDouble(const AjPDouble thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajDoubleLen **********************************************************
**
** Get length of dynamic 1d double array
**
** @param [r] thys [const AjPDouble] Source array
** @return [ajuint] length
**
** @release 1.0.0
** @@
******************************************************************************/

ajuint ajDoubleLen(const AjPDouble thys)
{
    return thys->Len;
}




/* @func ajShortNew ***********************************************************
**
** Default constructor for empty AJAX short arrays.
**
** @return [AjPShort] Pointer to an empty short array structure
** @category new [AjPShort] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPShort ajShortNew(void)
{
    AjPShort thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(short));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(short);

    return thys;
}




/* @func ajShortNewRes ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPShort] Pointer to an empty short array struct of specified size.
** @category new [AjPShort] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPShort ajShortNewRes(ajuint size)
{
    AjPShort thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(short));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(short);

    return thys;
}




/* @func ajShortDel ***********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPShort*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPShort] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajShortDel(AjPShort *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajShortDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajShortGet ***********************************************************
**
** Retrieve an element from an AJAX short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPShort] Pointer to the short array.
** @param  [r] elem [ajuint] array element.
**
** @return [short] contents of array element
** @category cast [AjPShort] Retrieve a short from an array
**
** @release 1.0.0
** @@
******************************************************************************/

short ajShortGet(const AjPShort thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad short array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajShortPut ***********************************************************
**
** Load a short array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort*] Pointer to the short integer array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPShort] Load a short array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajShortPut(AjPShort *thys, ajuint elem, short v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;

	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    arrShortResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @funcstatic arrShortResize *************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort*] Pointer to the short integer array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrShortResize(AjPShort *thys, ajuint size)
{
    AjPShort p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajShortResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajShortNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(short));

    (*thys)->Len = size+1;


    ajShortDel(&p);

    arrResize++;

    return ajTrue;
}




/* @func ajShortShort *********************************************************
**
** Returns the current short* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPShort] Source array
** @return [short*] Current array pointer, or a null string if undefined.
** @category cast [AjPShort] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

short* ajShortShort(const AjPShort thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajShortLen ***********************************************************
**
** Get length of dynamic 1d short array
**
** @param [r] thys [const AjPShort] Source array
** @return [ajuint] length
**
** @release 1.0.0
** @@
******************************************************************************/

ajuint ajShortLen(const AjPShort thys)
{
    return thys->Len;
}




/* @func ajLongNew ************************************************************
**
** Default constructor for empty AJAX ajlong arrays.
**
** @return [AjPLong] Pointer to an empty ajlong array structure
** @category new [AjPLong] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPLong ajLongNew(void)
{
    AjPLong thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(ajlong));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    arrTotal++;
    arrAlloc += RESERVED_SIZE*sizeof(ajlong);

    return thys;
}




/* @func ajLongNewRes *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size
** @return [AjPLong] Pointer to an empty ajlong array struct of specified size.
** @category new [AjPLong] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPLong ajLongNewRes(ajuint size)
{
    AjPLong thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(ajlong));
    thys->Len = 0;
    thys->Res = size;

    arrTotal++;
    arrAlloc += size*sizeof(ajlong);

    return thys;
}




/* @func ajLongDel ************************************************************
**
** Default destructor for AJAX ajlong arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPLong*] Pointer to the ajlong array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPLong] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajLongDel(AjPLong *thys)
{
    if(!thys || !*thys)
	return;

    /*ajDebug("ajLongDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arrFreeCount++;

    return;
}




/* @func ajLongGet ************************************************************
**
** Retrieve an element from an AJAX ajlong array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPLong] Pointer to the ajlong array.
** @param  [r] elem [ajuint] array element.
**
** @return [ajlong] contents of array element
** @category cast [AjPLong] Retrieve a ajlong from an array
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong ajLongGet(const AjPLong thys, ajuint elem)
{
    if(!thys || elem>=thys->Len)
	ajErr("Attempt to access bad ajlong array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajLongPut ************************************************************
**
** Load a long integer array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong*] Pointer to the long integer array.
** @param  [r] elem [ajuint] array element.
** @param  [r] v [ajlong] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPLong] Load a ajlong array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajLongPut(AjPLong *thys, ajuint elem, ajlong v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;

	(*thys)->Ptr[elem] = v;

	return ajFalse;
    }

    arrLongResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @funcstatic arrLongResize **************************************************
**
** Resize a long integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong*] Pointer to the long integer array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrLongResize(AjPLong *thys, ajuint size)
{
    AjPLong p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize ajlong array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajLongResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    *thys = ajLongNewRes(s);

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(ajlong));

    (*thys)->Len = size+1;


    ajLongDel(&p);

    arrResize++;

    return ajTrue;
}




/* @func ajLongLong ***********************************************************
**
** Returns the current ajlong* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPLong] Source array
** @return [ajlong*] Current array pointer, or a null string if undefined.
** @category cast [AjPLong] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong* ajLongLong(const AjPLong thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajLongLen ************************************************************
**
** Get length of dynamic 1d ajlong array
**
** @param [r] thys [const AjPLong] Source array
** @return [ajuint] length
**
** @release 1.0.0
** @@
******************************************************************************/

ajuint ajLongLen(const AjPLong thys)
{
    return thys->Len;
}




/* @func ajFloatParse *********************************************************
**
** Parses a string into a floating point array.
**
** The array size is already set.
**
** @param [r] str [const AjPStr] Input string
** @param [w] array [AjPFloat*] Array
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajFloatParse (const AjPStr str, AjPFloat* array)
{
    ajuint i = 0;
    float t = 0.0;

    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    AjPStr numstr = NULL;

    if (!floatRegNum)
	floatRegNum = ajRegCompC ("[+-]?[0-9.]+");

    ajStrAssignS(&tmpstr, str);

    while (ajRegExec (floatRegNum, tmpstr))
    {
	ajRegSubI (floatRegNum, 0, &numstr);
	ajRegPost (floatRegNum, &tmpstr2);
        ajStrAssignS(&tmpstr, tmpstr2);
	ajStrToFloat (numstr, &t);
	ajFloatPut(array,i,t);
	i++;
    }

    ajStrDel(&numstr);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    if(!i)
	return ajFalse;

    return ajTrue;
}




/* @func ajFloatStr ***********************************************************
**
** Writes a floating point array as a string
**
** @param [r] array [const AjPFloat] Array
** @param [r] precision [ajint] floating point precision
** @param [w] str [AjPStr*] Output string
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloatStr (const AjPFloat array, ajint precision, AjPStr* str)
{
    ajuint i;

    for (i=0; i < array->Len; i++)
    {
	if (i)
	    ajStrAppendK(str, ' ');

	ajFmtPrintAppS (str, "%.*f", precision, ajFloatGet(array,i));
    }

    return;
}




/* @func ajFloatTrace *********************************************************
**
** Writes a floating point array to the debug file
**
** @param [r] array [const AjPFloat] Array
** @param [r] precision [ajint] floating point precision
** @param [r] text [const char*] Report title
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloatTrace (const AjPFloat array, ajint precision, const char* text)
{
    ajuint i;

    ajDebug ("%s\n", text);

    for (i=0; i < array->Len; i++)
	ajDebug ("%3d: %.*f\n", i, precision, ajFloatGet(array,i));

    ajDebug ("\n");

    return;
}




/* @func ajArrCommaList *******************************************************
**
** Creates an AjPStr array from a string of comma separated tokens
**
** @param [r] s [const AjPStr] Line containing comma separated strings
** @param [w] a [AjPStr **] array pointer to create and load
**
** @return [ajuint] number of array elements created
**
** @release 1.0.0
** @@
******************************************************************************/
ajuint ajArrCommaList(const AjPStr s, AjPStr **a)
{
    AjPStr    x;
    AjPStrTok t;
    ajuint n;
    ajuint i;


    n = ajStrParseCountC(s,",\n");

    if(!n)
	return 0;

    AJCNEW(*a, n);

    x = ajStrNew();
    t = ajStrTokenNewC(s,",\n");

    for(i=0;i<n;++i)
    {
	ajStrTokenNextParseC(&t,",\n", &x);
        ajStrTrimWhite(&x);
	(*a)[i] = ajStrNewS(x);
    }

    ajStrDel(&x);
    ajStrTokenDel(&t);

    return n;
}




/* @func ajArrDoubleLine ******************************************************
**
** Creates a double array from a string of columns
**
** @param [r] line [const AjPStr] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] startcol [ajuint] Start token (1 to n)
** @param [r] endcol [ajuint] End token (1 to n)
** @return [double*] Allocated array of integers
**
** @release 1.0.0
** @@
******************************************************************************/

double* ajArrDoubleLine(const AjPStr line, const char *delim,
			ajuint startcol, ajuint endcol)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;
    static double *ret;
    ajuint ncols;
    ajuint i;


    t = ajStrTokenNewC(line, delim);
    tmp = ajStrNew();
    ncols = (endcol-startcol)+1;
    AJCNEW(ret, ncols);

    for(i=0;i<startcol-1;++i)
	if(!ajStrTokenNextParseC(&t,delim,&tmp))
	    ajFatal("Token missing %u of %u at start of line:\n%S",
		    (i+1), (startcol-1), line);

    for(i=0;i<ncols;++i)
    {
	if(!ajStrTokenNextParseC(&t,delim,&tmp))
	    ajFatal("Token missing %u of %u expected in line:\n%S",
		    (startcol+i), endcol, line);

	if(!ajStrToDouble(tmp,&ret[i]))
	    ajFatal("Bad float conversion %u of %u (%S) in line:\n%S",
		    (startcol+i), endcol, tmp, line);
    }

    ajStrDel(&tmp);
    ajStrTokenDel(&t);

    return ret;
}




/* @func ajArrIntLine *********************************************************
**
** Creates an Int array from a string of columns
**
** @param [r] line [const AjPStr] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] startcol [ajuint] Start token (1 to n)
** @param [r] endcol [ajuint] End token (1 to n)
** @return [ajint*] Allocated array of integers
**
** @release 1.0.0
** @@
******************************************************************************/

ajint* ajArrIntLine(const AjPStr line, const char *delim,
		    ajuint startcol, ajuint endcol)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;
    static ajint *ret;
    ajuint ncols;
    ajuint i;


    t     = ajStrTokenNewC(line, delim);
    tmp   = ajStrNew();
    ncols = (endcol-startcol)+1;

    AJCNEW(ret, ncols);

    for(i=0;i<startcol-1;++i)
	if(!ajStrTokenNextParseC(&t,delim,&tmp))
	    ajFatal("Token missing %u of %u at start of line:\n%S",
		    (i+1), (startcol-1), line);

    for(i=0;i<ncols;++i)
    {
	if(!ajStrTokenNextParseC(&t,delim,&tmp))
	    ajFatal("Token missing %u of %u expected in line:\n%S",
		    (startcol+i), endcol, line);

	if(!ajStrToInt(tmp,&ret[i]))
	    ajFatal("Bad integer array conversion %u of %u (%S) in line:\n%S",
		    (startcol+i), endcol, tmp, line);
    }

    ajStrDel(&tmp);
    ajStrTokenDel(&t);

    return ret;
}




/* @func ajArrFloatLine *******************************************************
**
** Creates a Float array from a string of columns
**
** @param [r] line [const AjPStr] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] startcol [ajuint] Start token (1 to n)
** @param [r] endcol [ajuint] End token (1 to n)
** @return [float*] Allocated array of integers
**
** @release 1.0.0
** @@
******************************************************************************/

float* ajArrFloatLine(const AjPStr line, const char *delim,
		      ajuint startcol, ajuint endcol)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;
    static float *ret;
    ajuint ncols;
    ajuint i;
    AjPStr tmpline = NULL;

    tmpline = ajStrNew();
    ajStrAssignS(&tmpline,line);
    
    ajStrRemoveWhiteExcess(&tmpline);

    t     = ajStrTokenNewC(tmpline, delim);
    tmp   = ajStrNew();
    ncols = (endcol-startcol)+1;

    AJCNEW(ret, ncols);

    for(i=0;i<startcol-1;++i)
	if(!ajStrTokenNextParseC(&t,delim,&tmp))
	    ajFatal("Token missing %u of %u at start of line:\n%S",
		    (i+1), (startcol-1), line);

    for(i=0;i<ncols;++i)
    {
	if(!ajStrTokenNextParseC(&t,delim,&tmp))
	    ajFatal("Token missing %u of %u expected in line:\n%S",
		    (startcol+i), endcol, line);

	if(!ajStrToFloat(tmp,&ret[i]))
	    ajFatal("Bad float conversion %u of %u (%S) in line:\n%S",
		    (startcol+i), endcol, tmp, line);
    }

    ajStrDel(&tmp);
    ajStrDel(&tmpline);
    ajStrTokenDel(&t);

    return ret;
}




/* @func ajInt2dNew ***********************************************************
**
** Default constructor for empty AJAX 2D integer arrays.
**
** @return [AjPInt2d] Pointer to an empty integer array structure
** @category new [AjPInt2d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPInt2d ajInt2dNew(void)
{
    AjPInt2d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPInt));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    arr2dTotal++;
    arr2dAlloc += RESERVED_SIZE;

    return thys;
}




/* @func ajInt2dNewRes ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPInt2d] Pointer to an empty integer 2d array struct of
**                    specified size.
** @category new [AjPInt2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPInt2d ajInt2dNewRes(ajuint size)
{
    AjPInt2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    arr2dAlloc++;

    return thys;
}




/* @func ajInt2dNewResRes2 ****************************************************
**
** Constructor given an initial reserved size in both dimensions
**
** @param [r] size [ajuint] Reserved size 1st dim
** @param [r] size2 [ajuint] Reserved size 2nd dim
** @return [AjPInt2d] Pointer to an empty integer 2d array struct of
**                    specified size.
** @category new [AjPInt2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPInt2d ajInt2dNewResRes2(ajuint size, ajuint size2)
{
    AjPInt2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
        thys->Ptr[i] = ajIntNewRes(size2);

    arr2dAlloc++;

    /*ajDebug("ajInt2dNewLL %d*%d %d; %d*%d*%d %d\n",
	    size, sizeof(AjPInt*), size*sizeof(AjPInt*),
	    size, size2, sizeof(ajint), size*size2*sizeof(ajint));*/

    return thys;
}




/* @func ajInt2dDel ***********************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPInt2d*] Pointer to the ajint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPInt2d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajInt2dDel(AjPInt2d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    /*ajDebug("ajInt2dDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajIntDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr2dFreeCount++;

    return;
}




/* @func ajInt2dGet ***********************************************************
**
** Retrieve an element from an AJAX 2d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPInt2d] Pointer to the ajint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
**
** @return [ajint] contents of array element
** @category cast [AjPInt2d] Retrieve an integer from an array
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajInt2dGet(const AjPInt2d thys, ajuint elem1, ajuint elem2)
{
    AjPInt t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajint array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajIntGet(t,elem2);
}




/* @func ajInt2dPut ***********************************************************
**
** Load an integer 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt2d*] Pointer to the ajint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] v [ajint] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPInt2d] Load an integer array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajInt2dPut(AjPInt2d *thys, ajuint elem1, ajuint elem2, ajint v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	/*ajDebug("ajInt2dPut [%u][%u] %d ([%u] %x)\n",
		elem1, elem2, v, (*thys)->Len, (*thys)->Ptr[elem1]);*/
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajIntNew();

	return ajIntPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    arrInt2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajIntNew();

    ajIntPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic arrInt2dResize *************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt2d*] Pointer to the ajint array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrInt2dResize(AjPInt2d *thys, ajuint size)
{
    AjPInt2d nthys;
    AjPInt2d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajInt2dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPInt));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPInt));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajIntDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr2dResize++;

    return ajTrue;
}




/* @func ajInt2dLen ***********************************************************
**
** Get lengths of 2d ajint array
**
**
** @param  [r] thys [const AjPInt2d] Pointer to the ajint array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajInt2dLen(const AjPInt2d thys, ajuint* len1, ajuint* len2)
{
    AjPInt t;
    ajuint i;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajInt2dInt ***********************************************************
**
** Convert AjPInt2d to ajint**
**
** @param  [r] thys [const AjPInt2d] Pointer to the ajint array.
**
** @return [ajint**] converted value.
** @category cast [AjPInt2d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

ajint** ajInt2dInt(const AjPInt2d thys)
{
    AjPInt t = NULL;
    ajint **array;
    ajuint d1;
    ajuint d2;
    ajuint i;

    ajInt2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);

	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(ajint));
    }

    return array;
}




/* @func ajInt3dNew ***********************************************************
**
** Default constructor for empty AJAX 3D integer arrays.
**
** @return [AjPInt3d] Pointer to an empty integer array structure
** @category new [AjPInt3d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPInt3d ajInt3dNew(void)
{
    AjPInt3d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPInt2d));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajInt3dNewRes ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPInt3d] Pointer to an empty integer 3d array struct of
**                    specified size.
** @category new [AjPInt3d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPInt3d ajInt3dNewRes(ajuint size)
{
    AjPInt3d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt2d));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajInt3dDel ***********************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPInt3d*] Pointer to the ajint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPInt3d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajInt3dDel(AjPInt3d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    /*ajDebug("ajInt3dDel Len %u Res %u\n",
	    (*thys)->Len, (*thys)->Res);*/

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajInt2dDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr3dFreeCount++;

    return;
}




/* @func ajInt3dGet ***********************************************************
**
** Retrieve an element from an AJAX 3d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPInt3d] Pointer to the ajint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
**
** @return [ajint] contents of array element
** @category cast [AjPInt3d] Retrieve an integer from an array
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajInt3dGet(const AjPInt3d thys, ajuint elem1, ajuint elem2, ajuint elem3)
{
    AjPInt2d t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajint array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajInt2dGet(t,elem2,elem3);
}




/* @func ajInt3dPut ***********************************************************
**
** Load an integer 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt3d*] Pointer to the ajint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
** @param  [r] v [ajint] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPInt3d] Load an integer array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajInt3dPut(AjPInt3d *thys,
		  ajuint elem1, ajuint elem2, ajuint elem3, ajint v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajInt2dNew();

	return ajInt2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    arrInt3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajInt2dNew();

    ajInt2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic arrInt3dResize *************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt3d*] Pointer to the ajint array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrInt3dResize(AjPInt3d *thys, ajuint size)
{
    AjPInt3d nthys;
    AjPInt3d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajInt3dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPInt2d));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPInt2d));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajInt2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr3dResize++;

    return ajTrue;
}




/* @func ajInt3dLen ***********************************************************
**
** Get lengths of 3d ajint array
**
**
** @param  [r] thys [const AjPInt3d] Pointer to the ajint array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
** @param  [w] len3 [ajuint*] Length of 3rd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajInt3dLen(const AjPInt3d thys, ajuint* len1, ajuint* len2, ajuint* len3)
{
    AjPInt2d t2;
    AjPInt   t1;
    ajuint i;
    ajuint j;
    ajuint v;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;

    *len3=0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}

    return;
}




/* @func ajInt3dInt ***********************************************************
**
** Convert AjPInt3d to ajint***
**
** @param  [r] thys [const AjPInt3d] Pointer to the ajint array.
**
** @return [ajint***] converted values.
** @category cast [AjPInt3d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

ajint*** ajInt3dInt(const AjPInt3d thys)
{
    AjPInt2d t2 = NULL;
    AjPInt   t1 = NULL;
    ajint ***array;
    ajuint d1;
    ajuint d2;
    ajuint d3;
    ajuint i;
    ajuint j;

    ajInt3dLen(thys,&d1,&d2,&d3);

    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];

	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len) continue;
		if((t1=t2->Ptr[j]))
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(ajint));
	    }
	}
    }

    return array;
}




/* @func ajUint2dNew **********************************************************
**
** Default constructor for empty AJAX 2D integer arrays.
**
** @return [AjPUint2d] Pointer to an empty integer array structure
** @category new [AjPUint2d] Default constructor
**
** @release 4.1.0
** @@
******************************************************************************/

AjPUint2d ajUint2dNew(void)
{
    AjPUint2d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPUint));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    arr2dTotal++;
    arr2dAlloc += RESERVED_SIZE;

    return thys;
}




/* @func ajUint2dNewRes *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPUint2d] Pointer to an empty integer 2d array struct of
**                    specified size.
** @category new [AjPUint2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPUint2d ajUint2dNewRes(ajuint size)
{
    AjPUint2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPUint));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    arr2dAlloc++;

    return thys;
}




/* @func ajUint2dNewResRes2 ***************************************************
**
** Constructor given an initial reserved size in both dimensions
**
** @param [r] size [ajuint] Reserved size 1st dim
** @param [r] size2 [ajuint] Reserved size 2nd dim
** @return [AjPUint2d] Pointer to an empty integer 2d array struct of
**                    specified size.
** @category new [AjPUint2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPUint2d ajUint2dNewResRes2(ajuint size, ajuint size2)
{
    AjPUint2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPUint));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
        thys->Ptr[i] = ajUintNewRes(size2);

    arr2dAlloc++;

    /*ajDebug("ajUint2dNewLL %d*%d %d; %d*%d*%d %d\n",
	    size, sizeof(AjPUint*), size*sizeof(AjPUint*),
	    size, size2, sizeof(ajuint), size*size2*sizeof(ajuint));*/
    return thys;
}




/* @func ajUint2dDel **********************************************************
**
** Default destructor for AJAX unsigned integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPUint2d*] Pointer to the ajuint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPUint2d] Default destructor
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUint2dDel(AjPUint2d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajUintDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr2dFreeCount++;

    return;
}




/* @func ajUint2dGet **********************************************************
**
** Retrieve an element from an AJAX 2d unsigned integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPUint2d] Pointer to the ajuint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
**
** @return [ajuint] contents of array element
** @category cast [AjPUint2d] Retrieve an integer from an array
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajUint2dGet(const AjPUint2d thys, ajuint elem1, ajuint elem2)
{
    AjPUint t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajuint array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajUintGet(t,elem2);
}




/* @func ajUint2dPut **********************************************************
**
** Load an unsigned integer 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint2d*] Pointer to the ajuint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] v [ajuint] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPUint2d] Load an integer array element
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajUint2dPut(AjPUint2d *thys, ajuint elem1, ajuint elem2, ajuint v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajUintNew();

	return ajUintPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    arrUint2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajUintNew();

    ajUintPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic arrUint2dResize ************************************************
**
** Resize an unsigned integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint2d*] Pointer to the ajuint array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrUint2dResize(AjPUint2d *thys, ajuint size)
{
    AjPUint2d nthys;
    AjPUint2d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajUint2dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPUint));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPUint));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajUintDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr2dResize++;

    return ajTrue;
}




/* @func ajUint2dLen **********************************************************
**
** Get lengths of 2d ajuint array
**
**
** @param  [r] thys [const AjPUint2d] Pointer to the ajuint array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUint2dLen(const AjPUint2d thys, ajuint* len1, ajuint* len2)
{
    AjPUint t;
    ajuint i;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajUint2dUint *********************************************************
**
** Convert AjPUint2d to ajuint**
**
** @param  [r] thys [const AjPUint2d] Pointer to the ajuint array.
**
** @return [ajuint**] converted value.
** @category cast [AjPUint2d] Retrieve internal pointer
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint** ajUint2dUint(const AjPUint2d thys)
{
    AjPUint t = NULL;
    ajuint **array;
    ajuint d1;
    ajuint d2;
    ajuint i;

    ajUint2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);

	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(ajuint));
    }

    return array;
}




/* @func ajUint3dNew **********************************************************
**
** Default constructor for empty AJAX 3D unsigned integer arrays.
**
** @return [AjPUint3d] Pointer to an empty unsigned integer array structure
** @category new [AjPUint3d] Default constructor
**
** @release 4.1.0
** @@
******************************************************************************/

AjPUint3d ajUint3dNew(void)
{
    AjPUint3d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPUint2d));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajUint3dNewRes *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPUint3d] Pointer to an empty unsigned integer 3d array struct of
**                    specified size.
** @category new [AjPUint3d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPUint3d ajUint3dNewRes(ajuint size)
{
    AjPUint3d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt2d));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajUint3dDel **********************************************************
**
** Default destructor for AJAX unsigned integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPUint3d*] Pointer to the ajuint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPUint3d] Default destructor
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUint3dDel(AjPUint3d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajUint2dDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr3dFreeCount++;

    return;
}




/* @func ajUint3dGet **********************************************************
**
** Retrieve an element from an AJAX 3d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPUint3d] Pointer to the ajuint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
**
** @return [ajuint] contents of array element
** @category cast [AjPUint3d] Retrieve an integer from an array
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajUint3dGet(const AjPUint3d thys,
		  ajuint elem1, ajuint elem2, ajuint elem3)
{
    AjPUint2d t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajuint array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajUint2dGet(t,elem2,elem3);
}




/* @func ajUint3dPut **********************************************************
**
** Load an unsigned integer 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint3d*] Pointer to the ajint array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
** @param  [r] v [ajuint] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPUint3d] Load an integer array element
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajUint3dPut(AjPUint3d *thys,
		   ajuint elem1, ajuint elem2, ajuint elem3, ajuint v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajUint2dNew();

	return ajUint2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    arrUint3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajUint2dNew();

    ajUint2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic arrUint3dResize ************************************************
** Resize an unsigned integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPUint3d*] Pointer to the ajuint array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrUint3dResize(AjPUint3d *thys, ajuint size)
{
    AjPUint3d nthys;
    AjPUint3d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize unsigned integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajUint3dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPUint2d));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPUint2d));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajUint2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr3dResize++;

    return ajTrue;
}




/* @func ajUint3dLen **********************************************************
**
** Get lengths of 3d ajuint array
**
**
** @param  [r] thys [const AjPUint3d] Pointer to the ajuint array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
** @param  [w] len3 [ajuint*] Length of 3rd dim
**
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajUint3dLen(const AjPUint3d thys,
		 ajuint* len1, ajuint* len2, ajuint* len3)
{
    AjPUint2d t2;
    AjPUint   t1;
    ajuint i;
    ajuint j;
    ajuint v;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;

    *len3=0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}

    return;
}




/* @func ajUint3dUint *********************************************************
**
** Convert AjPUint3d to ajuint***
**
** @param  [r] thys [const AjPUint3d] Pointer to the ajuint array.
**
** @return [ajuint***] converted values.
** @category cast [AjPUint3d] Retrieve internal pointer
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint*** ajUint3dUint(const AjPUint3d thys)
{
    AjPUint2d t2 = NULL;
    AjPUint   t1 = NULL;
    ajuint ***array;
    ajuint d1;
    ajuint d2;
    ajuint d3;
    ajuint i;
    ajuint j;

    ajUint3dLen(thys,&d1,&d2,&d3);

    AJCNEW0(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];

	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len)
                    continue;

		if((t1=t2->Ptr[j]))
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(ajuint));
	    }
	}
    }

    return array;
}




/* @func ajFloat2dNew *********************************************************
**
** Default constructor for empty AJAX 2D float arrays.
**
** @return [AjPFloat2d] Pointer to an empty float array structure
** @category new [AjPFloat2d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFloat2d ajFloat2dNew(void)
{
    AjPFloat2d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPFloat));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat2dNewRes ******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPFloat2d] Pointer to an empty float 2d array struct of
**                    specified size.
** @category new [AjPFloat2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPFloat2d ajFloat2dNewRes(ajuint size)
{
    AjPFloat2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPFloat));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat2dDel *********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPFloat2d*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFloat2d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloat2dDel(AjPFloat2d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajFloatDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr2dFreeCount++;

    return;
}




/* @func ajFloat2dGet *********************************************************
**
** Retrieve an element from an AJAX 2d float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPFloat2d] Pointer to the float array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
**
** @return [float] contents of array element
** @category cast [AjPFloat2d] Retrieve a float from an array
**
** @release 1.0.0
** @@
******************************************************************************/

float ajFloat2dGet(const AjPFloat2d thys, ajuint elem1, ajuint elem2)
{
    AjPFloat t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad float array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajFloatGet(t,elem2);
}




/* @func ajFloat2dPut *********************************************************
**
** Load a float 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPFloat2d] Load a float array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajFloat2dPut(AjPFloat2d *thys, ajuint elem1, ajuint elem2, float v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajFloatNew();
	return ajFloatPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    arrFloat2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajFloatNew();

    ajFloatPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic arrFloat2dResize ***********************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrFloat2dResize(AjPFloat2d *thys, ajuint size)
{
    AjPFloat2d nthys;
    AjPFloat2d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajFloat2dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPFloat));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPFloat));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajFloatDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr2dResize++;

    return ajTrue;
}




/* @func ajFloat2dLen *********************************************************
**
** Get lengths of 2d float array
**
** @param  [r] thys [const AjPFloat2d] Pointer to the float array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloat2dLen(const AjPFloat2d thys, ajuint* len1, ajuint* len2)
{
    AjPFloat t;
    ajuint i;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajFloat2dFloat *******************************************************
**
** Convert AjPFloat2d to float**
**
** @param  [r] thys [const AjPFloat2d] Pointer to the float array.
**
** @return [float**] converted values.
** @category cast [AjPFloat2d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

float** ajFloat2dFloat(const AjPFloat2d thys)
{
    AjPFloat t = NULL;
    float **array;
    ajuint d1;
    ajuint d2;
    ajuint i;

    ajFloat2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);

	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(float));
    }

    return array;
}




/* @func ajFloat3dNew *********************************************************
**
** Default constructor for empty AJAX 3D float arrays.
**
** @return [AjPFloat3d] Pointer to an empty float array structure
** @category new [AjPFloat3d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFloat3d ajFloat3dNew(void)
{
    AjPFloat3d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPFloat2d));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat3dNewRes ******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPFloat3d] Pointer to an empty float 3d array struct of
**                    specified size.
** @category new [AjPFloat3d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPFloat3d ajFloat3dNewRes(ajuint size)
{
    AjPFloat3d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPFloat2d));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat3dDel *********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPFloat3d*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFloat3d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloat3dDel(AjPFloat3d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajFloat2dDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr3dFreeCount++;

    return;
}




/* @func ajFloat3dGet *********************************************************
**
** Retrieve an element from an AJAX 3d float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPFloat3d] Pointer to the float array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
**
** @return [float] contents of array element
** @category cast [AjPFloat3d] Retrieve a float from an array
**
** @release 1.0.0
** @@
******************************************************************************/

float ajFloat3dGet(const AjPFloat3d thys,
		   ajuint elem1, ajuint elem2, ajuint elem3)
{
    AjPFloat2d t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad float array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajFloat2dGet(t,elem2,elem3);
}




/* @func ajFloat3dPut *********************************************************
**
** Load a float 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPFloat3d] Load a float array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajFloat3dPut(AjPFloat3d *thys,
		    ajuint elem1, ajuint elem2, ajuint elem3, float v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajFloat2dNew();
	return ajFloat2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    arrFloat3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajFloat2dNew();

    ajFloat2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic arrFloat3dResize ***********************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrFloat3dResize(AjPFloat3d *thys, ajuint size)
{
    AjPFloat3d nthys;
    AjPFloat3d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajFloat3dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPFloat2d));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPFloat2d));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajFloat2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr3dResize++;

    return ajTrue;
}




/* @func ajFloat3dLen *********************************************************
**
** Get lengths of 3d float array
**
** @param  [r] thys [const AjPFloat3d] Pointer to the float array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
** @param  [w] len3 [ajuint*] Length of 3rd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFloat3dLen(const AjPFloat3d thys,
		  ajuint* len1, ajuint* len2, ajuint* len3)
{
    AjPFloat2d t2;
    AjPFloat   t1;
    ajuint i;
    ajuint j;
    ajuint v;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;

	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}

    return;
}




/* @func ajFloat3dFloat *******************************************************
**
** Convert AjPFloat3d to float***
**
** @param  [r] thys [const AjPFloat3d] Pointer to the float array.
**
** @return [float***] converted values.
** @category cast [AjPFloat3d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

float*** ajFloat3dFloat(const AjPFloat3d thys)
{
    AjPFloat2d t2 = NULL;
    AjPFloat   t1 = NULL;
    float ***array;
    ajuint d1;
    ajuint d2;
    ajuint d3;
    ajuint i;
    ajuint j;

    ajFloat3dLen(thys,&d1,&d2,&d3);

    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];

	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);

	    if(t2)
	    {
		if(j>=t2->Len)
                    continue;

		if((t1=t2->Ptr[j]))
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(float));
	    }
	}
    }

    return array;
}




/* @func ajDouble2dNew ********************************************************
**
** Default constructor for empty AJAX 2D double arrays.
**
** @return [AjPDouble2d] Pointer to an empty double array structure
** @category new [AjPDouble2d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPDouble2d ajDouble2dNew(void)
{
    AjPDouble2d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPDouble));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble2dNewRes *****************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPDouble2d] Pointer to an empty double 2d array struct of
**                    specified size.
** @category new [AjPDouble2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPDouble2d ajDouble2dNewRes(ajuint size)
{
    AjPDouble2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPDouble));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble2dDel ********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPDouble2d*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPDouble2d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajDouble2dDel(AjPDouble2d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajDoubleDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr2dFreeCount++;

    return;
}




/* @func ajDouble2dGet ********************************************************
**
** Retrieve an element from an AJAX 2d double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPDouble2d] Pointer to the double array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
**
** @return [double] contents of array element
** @category cast [AjPDouble2d] Retrieve a double from an array
**
** @release 1.0.0
** @@
******************************************************************************/

double ajDouble2dGet(const AjPDouble2d thys, ajuint elem1, ajuint elem2)
{
    AjPDouble t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad double array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajDoubleGet(t,elem2);
}




/* @func ajDouble2dPut ********************************************************
**
** Load a double 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPDouble2d] Load a double array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajDouble2dPut(AjPDouble2d *thys, ajuint elem1, ajuint elem2, double v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajDoubleNew();

	return ajDoublePut(&(*thys)->Ptr[elem1],elem2,v);
    }

    arrDouble2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajDoubleNew();

    ajDoublePut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic arrDouble2dResize **********************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrDouble2dResize(AjPDouble2d *thys, ajuint size)
{
    AjPDouble2d nthys;
    AjPDouble2d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajDouble2dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPDouble));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPDouble));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajDoubleDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr2dResize++;

    return ajTrue;
}




/* @func ajDouble2dLen ********************************************************
**
** Get lengths of 2d double array
**
** @param  [r] thys [const AjPDouble2d] Pointer to the double array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajDouble2dLen(const AjPDouble2d thys, ajuint* len1, ajuint* len2)
{
    AjPDouble t;
    ajuint i;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajDouble2dDouble *****************************************************
**
** Convert AjPDouble2d to double**
**
** @param  [r] thys [const AjPDouble2d] Pointer to the double array.
**
** @return [double**] converted values.
** @category cast [AjPDouble2d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

double** ajDouble2dDouble(const AjPDouble2d thys)
{
    AjPDouble t = NULL;
    double **array;
    ajuint d1;
    ajuint d2;
    ajuint i;

    ajDouble2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);

	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(double));
    }

    return array;
}




/* @func ajDouble3dNew ********************************************************
**
** Default constructor for empty AJAX 3D double arrays.
**
** @return [AjPDouble3d] Pointer to an empty double array structure
** @category new [AjPDouble3d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPDouble3d ajDouble3dNew(void)
{
    AjPDouble3d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPDouble2d));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble3dNewRes *****************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPDouble3d] Pointer to an empty double 3d array struct of
**                    specified size.
** @category new [AjPDouble3d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPDouble3d ajDouble3dNewRes(ajuint size)
{
    AjPDouble3d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPDouble2d));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble3dDel ********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPDouble3d*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPDouble3d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajDouble3dDel(AjPDouble3d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajDouble2dDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr3dFreeCount++;

    return;
}




/* @func ajDouble3dGet ********************************************************
**
** Retrieve an element from an AJAX 3d double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPDouble3d] Pointer to the double array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
**
** @return [double] contents of array element
** @category cast [AjPDouble3d] Retrieve a double from an array
**
** @release 1.0.0
** @@
******************************************************************************/

double ajDouble3dGet(const AjPDouble3d thys,
                     ajuint elem1, ajuint elem2, ajuint elem3)
{
    AjPDouble2d t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad double array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajDouble2dGet(t,elem2,elem3);
}




/* @func ajDouble3dPut ********************************************************
**
** Load a double 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPDouble3d] Load a double array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajDouble3dPut(AjPDouble3d *thys,
		     ajuint elem1, ajuint elem2, ajuint elem3, double v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajDouble2dNew();

	return ajDouble2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    arrDouble3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajDouble2dNew();

    ajDouble2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic arrDouble3dResize **********************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrDouble3dResize(AjPDouble3d *thys, ajuint size)
{
    AjPDouble3d nthys;
    AjPDouble3d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajDouble3dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPDouble2d));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPDouble2d));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajDouble2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr3dResize++;

    return ajTrue;
}




/* @func ajDouble3dLen ********************************************************
**
** Get lengths of 3d double array
**
** @param  [r] thys [const AjPDouble3d] Pointer to the double array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
** @param  [w] len3 [ajuint*] Length of 3rd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajDouble3dLen(const AjPDouble3d thys,
		   ajuint* len1, ajuint* len2, ajuint* len3)
{
    AjPDouble2d t2;
    AjPDouble   t1;
    ajuint i;
    ajuint j;
    ajuint v;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;

    *len3 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;

	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}

    return;
}




/* @func ajDouble3dDouble *****************************************************
**
** Convert AjPDouble3d to double***
**
** @param  [r] thys [const AjPDouble3d] Pointer to the double array.
**
** @return [double***] converted values.
** @category cast [AjPDouble3d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

double*** ajDouble3dDouble(const AjPDouble3d thys)
{
    AjPDouble2d t2 = NULL;
    AjPDouble   t1 = NULL;
    double ***array;
    ajuint d1;
    ajuint d2;
    ajuint d3;
    ajuint i;
    ajuint j;

    ajDouble3dLen(thys,&d1,&d2,&d3);

    AJCNEW0(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];

	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);

	    if(t2)
	    {
		if(j>=t2->Len)
                    continue;

		if((t1=t2->Ptr[j]))
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(double));
	    }
	}
    }

    return array;
}




/* @func ajShort2dNew *********************************************************
**
** Default constructor for empty AJAX 2D short arrays.
**
** @return [AjPShort2d] Pointer to an empty short array structure
** @category new [AjPShort2d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPShort2d ajShort2dNew(void)
{
    AjPShort2d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPShort));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort2dNewRes ******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPShort2d] Pointer to an empty short 2d array struct of
**                    specified size.
** @category new [AjPShort2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPShort2d ajShort2dNewRes(ajuint size)
{
    AjPShort2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPShort));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort2dDel *********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPShort2d*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPShort2d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajShort2dDel(AjPShort2d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajShortDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr2dFreeCount++;

    return;
}




/* @func ajShort2dGet *********************************************************
**
** Retrieve an element from an AJAX 2d short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPShort2d] Pointer to the short array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
**
** @return [short] contents of array element
** @category cast [AjPShort2d] Retrieve a short from an array
**
** @release 1.0.0
** @@
******************************************************************************/

short ajShort2dGet(const AjPShort2d thys, ajuint elem1, ajuint elem2)
{
    AjPShort t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad short array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajShortGet(t,elem2);
}




/* @func ajShort2dPut *********************************************************
**
** Load a short 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPShort2d] Load a short array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajShort2dPut(AjPShort2d *thys, ajuint elem1, ajuint elem2, short v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajShortNew();

	return ajShortPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    arrShort2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajShortNew();

    ajShortPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic arrShort2dResize ***********************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrShort2dResize(AjPShort2d *thys, ajuint size)
{
    AjPShort2d nthys;
    AjPShort2d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajShort2dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPShort));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPShort));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajShortDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr2dResize++;

    return ajTrue;
}




/* @func ajShort2dLen *********************************************************
**
** Get lengths of 2d short array
**
** @param  [r] thys [const AjPShort2d] Pointer to the short array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajShort2dLen(const AjPShort2d thys, ajuint* len1, ajuint* len2)
{
    AjPShort t;
    ajuint i;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajShort2dShort *******************************************************
**
** Convert AjPShort2d to short**
**
** @param  [r] thys [const AjPShort2d] Pointer to the short array.
**
** @return [short**] converted values
** @category cast [AjPShort2d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

short** ajShort2dShort(const AjPShort2d thys)
{
    AjPShort t = NULL;
    short **array;
    ajuint d1;
    ajuint d2;
    ajuint i;

    ajShort2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);

	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(short));
    }

    return array;
}




/* @func ajShort3dNew *********************************************************
**
** Default constructor for empty AJAX 3D short arrays.
**
** @return [AjPShort3d] Pointer to an empty short array structure
** @category new [AjPShort3d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPShort3d ajShort3dNew(void)
{
    AjPShort3d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPShort2d));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort3dNewRes ******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPShort3d] Pointer to an empty short 3d array struct of
**                    specified size.
** @category new [AjPShort3d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPShort3d ajShort3dNewRes(ajuint size)
{
    AjPShort3d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPShort2d));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort3dDel *********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPShort3d*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPShort3d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajShort3dDel(AjPShort3d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajShort2dDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr3dFreeCount++;

    return;
}




/* @func ajShort3dGet *********************************************************
**
** Retrieve an element from an AJAX 3d short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPShort3d] Pointer to the short array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
**
** @return [short] contents of array element
** @category cast [AjPShort3d] Retrieve a short from an array
**
** @release 1.0.0
** @@
******************************************************************************/

short ajShort3dGet(const AjPShort3d thys,
		   ajuint elem1, ajuint elem2, ajuint elem3)
{
    AjPShort2d t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad short array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajShort2dGet(t,elem2,elem3);
}




/* @func ajShort3dPut *********************************************************
**
** Load a short 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPShort3d] Load a short array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajShort3dPut(AjPShort3d *thys,
		    ajuint elem1, ajuint elem2, ajuint elem3, short v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajShort2dNew();

	return ajShort2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    arrShort3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajShort2dNew();

    ajShort2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic arrShort3dResize ***********************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrShort3dResize(AjPShort3d *thys, ajuint size)
{
    AjPShort3d nthys;
    AjPShort3d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajShort3dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPShort2d));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPShort2d));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajShort2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr3dResize++;

    return ajTrue;
}




/* @func ajShort3dLen *********************************************************
**
** Get lengths of 3d short array
**
** @param  [r] thys [const AjPShort3d] Pointer to the short array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
** @param  [w] len3 [ajuint*] Length of 3rd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajShort3dLen(const AjPShort3d thys,
		  ajuint* len1, ajuint* len2, ajuint* len3)
{
    AjPShort2d t2;
    AjPShort   t1;
    ajuint i;
    ajuint j;
    ajuint v;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;

    *len3 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;

	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}

    return;
}




/* @func ajShort3dShort *******************************************************
**
** Convert AjPShort3d to short***
**
** @param  [r] thys [const AjPShort3d] Pointer to the short array.
**
** @return [short***] converted values.
** @category cast [AjPShort3d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

short*** ajShort3dShort(const AjPShort3d thys)
{
    AjPShort2d t2 = NULL;
    AjPShort   t1 = NULL;
    short ***array;
    ajuint d1;
    ajuint d2;
    ajuint d3;
    ajuint i;
    ajuint j;

    ajShort3dLen(thys,&d1,&d2,&d3);

    AJCNEW0(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];

	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);

	    if(t2)
	    {
		if(j>=t2->Len)
                    continue;

		if((t1=t2->Ptr[j]))
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(short));
	    }
	}
    }

    return array;
}




/* @func ajLong2dNew **********************************************************
**
** Default constructor for empty AJAX 2D ajlong arrays.
**
** @return [AjPLong2d] Pointer to an empty ajlong array structure
** @category new [AjPLong2d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPLong2d ajLong2dNew(void)
{
    AjPLong2d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPLong));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajLong2dNewRes *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPLong2d] Pointer to an empty ajlong 2d array struct of
**                    specified size.
** @category new [AjPLong2d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPLong2d ajLong2dNewRes(ajuint size)
{
    AjPLong2d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPLong));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajLong2dDel **********************************************************
**
** Default destructor for AJAX ajlong arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPLong2d*] Pointer to the ajlong array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPLong2d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajLong2dDel(AjPLong2d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajLongDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr2dFreeCount++;

    return;
}




/* @func ajLong2dGet **********************************************************
**
** Retrieve an element from an AJAX 2d ajlong array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPLong2d] Pointer to the ajlong array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
**
** @return [ajlong] contents of array element
** @category cast [AjPLong2d] Retrieve a ajlong from an array
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong ajLong2dGet(const AjPLong2d thys, ajuint elem1, ajuint elem2)
{
    AjPLong t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajlong array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajLongGet(t,elem2);
}




/* @func ajLong2dPut **********************************************************
**
** Load a ajlong 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong2d*] Pointer to the ajlong array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] v [ajlong] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPLong2d] Load a ajlong array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajLong2dPut(AjPLong2d *thys, ajuint elem1, ajuint elem2, ajlong v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajLongNew();

	return ajLongPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    arrLong2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajLongNew();

    ajLongPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic arrLong2dResize ************************************************
**
** Resize a ajlong array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong2d*] Pointer to the ajlong array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrLong2dResize(AjPLong2d *thys, ajuint size)
{
    AjPLong2d nthys;
    AjPLong2d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize ajlong array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajLong2dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPLong));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPLong));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajLongDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr2dResize++;

    return ajTrue;
}




/* @func ajLong2dLen **********************************************************
**
** Get lengths of 2d ajlong array
**
** @param  [r] thys [const AjPLong2d] Pointer to the ajlong array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajLong2dLen(const AjPLong2d thys, ajuint* len1, ajuint* len2)
{
    AjPLong t;
    ajuint i;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajLong2dLong *********************************************************
**
** Convert AjPLong2d to ajlong**
**
** @param  [r] thys [const AjPLong2d] Pointer to the ajlong array.
**
** @return [ajlong**] converted values.
** @category cast [AjPLong2d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong** ajLong2dLong(const AjPLong2d thys)
{
    AjPLong t = NULL;
    ajlong **array;
    ajuint d1;
    ajuint d2;
    ajuint i;

    ajLong2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);

    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);

	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(ajlong));
    }

    return array;
}




/* @func ajLong3dNew **********************************************************
**
** Default constructor for empty AJAX 3D ajlong arrays.
**
** @return [AjPLong3d] Pointer to an empty ajlong array structure
** @category new [AjPLong3d] Default constructor
**
** @release 1.0.0
** @@
******************************************************************************/

AjPLong3d ajLong3dNew(void)
{
    AjPLong3d thys;
    ajuint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPLong2d));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajLong3dNewRes *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajuint] Reserved size 1st dim
** @return [AjPLong3d] Pointer to an empty ajlong 3d array struct of
**                    specified size.
** @category new [AjPLong3d] Constructor with reserved size
**
** @release 6.2.0
** @@
******************************************************************************/

AjPLong3d ajLong3dNewRes(ajuint size)
{
    AjPLong3d thys;
    ajuint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPLong2d));
    thys->Len = 0;
    thys->Res = size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajLong3dDel **********************************************************
**
** Default destructor for AJAX ajlong arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPLong3d*] Pointer to the ajlong array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPLong3d] Default destructor
**
** @release 1.0.0
** @@
******************************************************************************/

void ajLong3dDel(AjPLong3d *thys)
{
    ajint i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajLong2dDel(&((*thys)->Ptr[i]));

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    arr3dFreeCount++;

    return;
}




/* @func ajLong3dGet **********************************************************
**
** Retrieve an element from an AJAX 3d ajlong array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPLong3d] Pointer to the ajlong array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
**
** @return [ajlong] contents of array element
** @category cast [AjPLong3d] Retrieve a ajlong from an array
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong ajLong3dGet(const AjPLong3d thys,
		   ajuint elem1, ajuint elem2, ajuint elem3)
{
    AjPLong2d t;

    if(!thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajlong array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];

    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajLong2dGet(t,elem2,elem3);
}




/* @func ajLong3dPut **********************************************************
**
** Load a ajlong 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong3d*] Pointer to the ajlong array.
** @param  [r] elem1 [ajuint] array element.
** @param  [r] elem2 [ajuint] array element.
** @param  [r] elem3 [ajuint] array element.
** @param  [r] v [ajlong] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPLong3d] Load a ajlong array element
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajLong3dPut(AjPLong3d *thys,
		   ajuint elem1, ajuint elem2, ajuint elem3, ajlong v)
{
    if(!thys || !*thys)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;

	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajLong2dNew();

	return ajLong2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    arrLong3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajLong2dNew();

    ajLong2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic arrLong3dResize ************************************************
**
** Resize a ajlong array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong3d*] Pointer to the ajlong array.
** @param  [r] size [ajuint] new size.
**
** @return [AjBool] true if the array was extended.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool arrLong3dResize(AjPLong3d *thys, ajuint size)
{
    AjPLong3d nthys;
    AjPLong3d p = NULL;
    ajuint    s;
    ajuint    clen;
    ajuint    limit;
    ajuint    i;


    if(!thys || !*thys)
	ajErr("Illegal attempt to resize ajlong array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);

    if(s <= clen)
	return ajFalse;

    /*ajDebug("ajLong3dResize %d (%d) -> %d (%d)\n",
            (*thys)->Len, clen, size, s);*/

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPLong2d));
    nthys->Res = s;

    if(size < p->Len)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPLong2d));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajLong2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    arr3dResize++;

    return ajTrue;
}




/* @func ajLong3dLen **********************************************************
**
** Get lengths of 3d ajlong array
**
** @param  [r] thys [const AjPLong3d] Pointer to the ajlong array.
** @param  [w] len1 [ajuint*] Length of 1st dim
** @param  [w] len2 [ajuint*] Length of 2nd dim
** @param  [w] len3 [ajuint*] Length of 3rd dim
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajLong3dLen(const AjPLong3d thys,
		 ajuint* len1, ajuint* len2, ajuint* len3)
{
    AjPLong2d t2;
    AjPLong   t1;
    ajuint i;
    ajuint j;
    ajuint v;

    *len1 = thys->Len;
    *len2 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;

    *len3 = 0;

    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}

    return;
}




/* @func ajLong3dLong *********************************************************
**
** Convert AjPLong3d to ajlong***
**
** @param  [r] thys [const AjPLong3d] Pointer to the ajlong array.
**
** @return [ajlong***] converted values.
** @category cast [AjPLong3d] Retrieve internal pointer
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong*** ajLong3dLong(const AjPLong3d thys)
{
    AjPLong2d t2 = NULL;
    AjPLong   t1 = NULL;
    ajlong ***array;
    ajuint d1;
    ajuint d2;
    ajuint d3;
    ajuint i;
    ajuint j;

    ajLong3dLen(thys,&d1,&d2,&d3);

    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];

	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);

	    if(t2)
	    {
		if(j>=t2->Len)
                    continue;

		if((t1=t2->Ptr[j]))
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(ajlong));
	    }
	}
    }

    return array;
}




/* @func ajArrExit ************************************************************
**
** Cleanup of array handling internals, and debug report of memory use
**
** @return [void]
**
** @release 4.0.0
******************************************************************************/

void ajArrExit(void)
{

    ajRegFree(&floatRegNum);

    ajDebug("Array usage (bytes): %Ld allocated, %Ld freed, %Ld in use\n",
            arrAlloc, arrFree,
            (arrAlloc - arrFree));
    ajDebug("Array usage (number): %Ld allocated, %Ld freed, %Ld resized,"
            " %Ld in use\n",
            arrTotal, arrFreeCount, arrResize, arrCount);

    ajDebug("Array usage 2D (bytes): %Ld allocated, %Ld freed, %Ld in use\n",
            arr2dAlloc, arr2dFree,
            (arr2dAlloc - arr2dFree));
    ajDebug("Array usage 2D (number): %Ld allocated, %Ld freed, %Ld resized,"
            " %Ld in use\n",
            arr2dTotal, arr2dFreeCount, arr2dResize, arr2dCount);

    ajDebug("Array usage 3D (bytes): %Ld allocated, %Ld freed, %Ld in use\n",
            arr3dAlloc, arr3dFree,
            (arr3dAlloc - arr3dFree));
    ajDebug("Array usage 3D (number): %Ld allocated, %Ld freed, %Ld resized,"
            " %Ld in use\n",
            arr3dTotal, arr3dFreeCount, arr3dResize, arr3dCount);



    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajChararrNewL
** @rename ajChararrNewRes
*/
__deprecated AjPChar ajChararrNewL(ajuint size)
{
    return ajChararrNewRes(size);
}




/* @obsolete ajIntNewL
** @rename ajIntNewRes
*/
__deprecated AjPInt ajIntNewL(ajuint size)
{
    return ajIntNewRes(size);
}




/* @obsolete ajUintNewL
** @rename ajUintNewRes
*/
__deprecated AjPUint ajUintNewL(ajuint size)
{
    return ajUintNewRes(size);
}




/* @obsolete ajFloatNewL
** @rename ajFloatNewRes
*/
__deprecated AjPFloat ajFloatNewL(ajuint size)
{
    return ajFloatNewRes(size);
}




/* @obsolete ajDoubleNewL
** @rename ajDoubleNewRes
*/
__deprecated AjPDouble ajDoubleNewL(ajuint size)
{
    return ajDoubleNewRes(size);
}




/* @obsolete ajShortNewL
** @rename ajShortNewRes
*/
__deprecated AjPShort ajShortNewL(ajuint size)
{
    return ajShortNewRes(size);
}




/* @obsolete ajLongNewL
** @rename ajLongNewRes
*/
__deprecated AjPLong ajLongNewL(ajuint size)
{
    return ajLongNewRes(size);
}




/* @obsolete ajInt2dNewL
** @rename ajInt2dNewRes
*/
__deprecated AjPInt2d ajInt2dNewL(ajuint size)
{
    return ajInt2dNewRes(size);
}




/* @obsolete ajInt2dNewLL
** @rename ajInt2dNewRes2
*/
__deprecated AjPInt2d ajInt2dNewLL(ajuint size, ajuint size2)
{
    return ajInt2dNewResRes2(size, size2);
}




/* @obsolete ajInt3dNewL
** @rename ajInt3dNewRes
*/
__deprecated AjPInt3d ajInt3dNewL(ajuint size)
{
    return ajInt3dNewRes(size);
}




/* @obsolete ajUint2dNewL
** @rename ajUint2dNewRes
*/
__deprecated AjPUint2d ajUint2dNewL(ajuint size)
{
    return ajUint2dNewRes(size);
}




/* @obsolete ajUint2dNewLL
** @rename ajUint2dNewRes2
*/
__deprecated AjPUint2d ajUint2dNewLL(ajuint size, ajuint size2)
{
    return ajUint2dNewResRes2(size, size2);
}




/* @obsolete ajUint3dNewL
** @rename ajUint3dNewRes
*/
__deprecated AjPUint3d ajUint3dNewL(ajuint size)
{
    return ajUint3dNewRes(size);
}




/* @obsolete ajFloat2dNewL
** @rename ajFloat2dNewRes
*/
__deprecated AjPFloat2d ajFloat2dNewL(ajuint size)
{
    return ajFloat2dNewRes(size);
}




/* @obsolete ajDouble2dNewL
** @rename ajDouble2dNewRes
*/
__deprecated AjPDouble2d ajDouble2dNewL(ajuint size)
{
    return ajDouble2dNewRes(size);
}




/* @obsolete ajDouble3dNewL
** @rename ajDouble3dNewRes
*/
__deprecated AjPDouble3d ajDouble3dNewL(ajuint size)
{
    return ajDouble3dNewRes(size);
}




/* @obsolete ajShort2dNewL
** @rename ajShort2dNewRes
*/
__deprecated AjPShort2d ajShort2dNewL(ajuint size)
{
    return ajShort2dNewRes(size);
}




/* @obsolete ajShort3dNewL
** @rename ajShort3dNewRes
*/
__deprecated AjPShort3d ajShort3dNewL(ajuint size)
{
    return ajShort3dNewRes(size);
}




/* @obsolete ajLong2dNewL
** @rename ajLong2dNewRes
*/
__deprecated AjPLong2d ajLong2dNewL(ajuint size)
{
    return ajLong2dNewRes(size);
}




/* @obsolete ajLong3dNewL
** @rename ajLong3dNewRes
*/
__deprecated AjPLong3d ajLong3dNewL(ajuint size)
{
    return ajLong3dNewRes(size);
}
#endif
