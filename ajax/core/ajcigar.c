/* @source ajcigar ************************************************************
**
** Handling of CIGAR strings
**
** @version $Revision: 1.5 $
** @modified Nov 2011 uludag, first version
** @modified $Date: 2012/07/14 14:52:39 $ by $Author: rice $
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

#include "ajlib.h"

#include "ajcigar.h"
#include "ajlist.h"

#include "stdlib.h"

#include <ctype.h>




/* @func ajCigarGetReferenceLength ********************************************
**
** Get reference length covered by a CIGAR string
**
** @param [r] c [const AjPCigar] cigar object
**
** @return [ajint] length of the aligned reference sequence
** @@
******************************************************************************/

ajint ajCigarGetReferenceLength(const AjPCigar c)
{
    ajint i=0;
    ajint length = 0;

    for (i=0;i<c->n;i++)
    {
	switch (c->cigoperator[i])
	{
	    case 'M':
	    case 'D':
	    case 'N':
	    case '=':
	    case 'X':
		length += c->length[i];
	}
    }

    return length;
}




/* @func ajCigarCountInsertedBases ********************************************
**
** Count bases inserted in a CIGAR string
**
** @param [r] c [const AjPCigar] cigar object
**
** @return [ajint] length of the bases in the read but not in the reference seq
** @@
******************************************************************************/

ajint ajCigarCountInsertedBases(const AjPCigar c)
{
    ajint i=0;
    ajint length = 0;

    for (i=0; i<c->n; i++)
    {
	if (c->cigoperator[i]=='I')
		length += c->length[i];
    }

    return length;
}




/* @func ajCigarCountDeletedBases *********************************************
**
** Count bases deleted in a CIGAR string
**
** @param [r] c [const AjPCigar] cigar object
**
** @return [ajint] length of the bases in the reference sequence
**		   but not in the read
** @@
******************************************************************************/

ajint ajCigarCountDeletedBases(const AjPCigar c)
{
    ajint i=0;
    ajint length = 0;

    for (i=0; i<c->n; i++)
    {
	if (c->cigoperator[i]=='D')
		length += c->length[i];
    }

    return length;
}




/* @func ajCigarNewS **********************************************************
**
** Cigar data constructor
**
** @param [r] cigar [const AjPStr] cigar string
**
** @return [AjPCigar] New object
** @@
******************************************************************************/

AjPCigar ajCigarNewS(const AjPStr cigar)
{
    AjPCigar c=NULL;
    const char* s=NULL;
    long x=0;
    int i =0;
    int n=0;
    char op;
    char* t;

    AJNEW0(c);

    n = ajStrGetLen(cigar);
    s = ajStrGetPtr(cigar);

    AJCNEW0(c->cigoperator, n);
    AJCNEW0(c->length, n);

    for (i=0; *s && i<n; i++)
    {
	x = strtol(s, &t, 10);
	op = toupper((int)*t);

	if (op == 'M' || op == 'I' || op == 'D' || op == 'N' ||
	    op == 'S' || op == 'H' || op == 'P' || op == '=' ||
	    op == 'X')
	{
	    c->cigoperator[i]=op;
	    c->length[i]=x;
	}
	else
	    ajWarn("invalid CIGAR operator: %c",op);

	s = t + 1;
    }

    c->n=i;

    AJCRESIZE(c->cigoperator, c->n);
    AJCRESIZE(c->length, c->n);

    return c;
}




/* @func ajCigarDel ************************************************************
**
** Delete cigar objects
**
** @param [d] Pcigar [AjPCigar*] Cigar data object to delete
** @return [void] 
** @@
******************************************************************************/

void ajCigarDel(AjPCigar *Pcigar)
{
    AjPCigar cigar;

    if(!Pcigar) return;
    if(!(*Pcigar)) return;

    cigar = *Pcigar;

    AJFREE(cigar->cigoperator);
    AJFREE(cigar->length);

    AJFREE(*Pcigar);
    *Pcigar = NULL;

    return;
}
