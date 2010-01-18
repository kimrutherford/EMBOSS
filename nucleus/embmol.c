/* @source embmol.c
**
** Routines for molecular weight matching.
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "embprop.h"
#include "embmol.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>




static ajint embMolFragSort(const void* a, const void* b);




/* @func embMolGetFrags *******************************************************
**
** Create a sorted list of molwt fragments
**
** @param [r] thys [const AjPStr] sequence
** @param [r] rno [ajint] 1=Trypsin 2=LysC 3=ArgC 4=AspN 5=V8b 6=V8p
**                        7=Chy 8=CNBr
** @param [r] mwdata [EmbPPropMolwt const *] molecular weight data
** @param [r] mono [AjBool] true for monoisotopic data
** @param [w] l [AjPList*] list for results
** @return [ajint] number of fragments
******************************************************************************/

ajint embMolGetFrags(const AjPStr thys, ajint rno, EmbPPropMolwt const *mwdata,
		     AjBool mono, AjPList *l)
{
    static struct enz
    {
	const char *ename;
	const char *residues;
	const char *type;
	const char *partial;
    } zyme[]=
    {
	{"Trypsin","KR","CC","KRIFL"},
	{"Lys-C","K","C",""},
	{"Arg-C","R","C",""},
	{"Asp-N","D","N",""},
	{"V8b","E","C","KR"},
	{"V8p","DE","CC",""},
	{"Chymotrypsin","FYWLM","CCCCC",""},
	{"CNBr","M","C",""}
    };

    EmbPMolFrag frag = NULL;
    EmbPMolFrag *ptr = NULL;

    ajint len;
    ajint pos;
    const char *p;

    static AjPInt defcut =NULL;
    ajint defcnt;

    ajint beg;
    ajint end;
    ajint i;
    double mw;


    if(!defcut)
	defcut=ajIntNew();

    --rno;

    len = ajStrGetLen(thys);
    p   = ajStrGetPtr(thys);

    defcnt=0;

    /* Positions of complete digest cuts */
    for(pos=0;pos<len;++pos)
    {
	if(!strchr(zyme[rno].residues,(ajint)p[pos]))
	    continue;

	if(len==pos+1)
	    continue;

	if(p[pos+1]=='P' && rno!=3 && rno!=7)
	    continue;

	if(rno==4 && p[pos+1]=='E')
	    continue;

	ajIntPut(&defcut,defcnt++,pos);
    }


    /* Molwts of definite cuts */
    beg = 0;
    for(i=0;i<defcnt;++i)
    {
	end = ajIntGet(defcut,i);

	if(strchr(zyme[rno].type,(ajint)'N'))
	    --end;

	mw = embPropCalcMolwt(p,beg,end, mwdata, mono);

	if(rno==7)
	    mw -= (double)(17.0079 + 31.095);

	AJNEW0(frag);
	frag->begin = beg+1;
	frag->end   = end+1;
	frag->mwt   = mw;
	ajListPush(*l,(void *)frag);
	beg = end+1;
    }

    if(defcnt)
    {
	mw = embPropCalcMolwt(p,beg,len-1,mwdata,mono);

	if(rno==7)
	    mw -= (double)(17.0079 + 31.095);

	AJNEW0(frag);
	frag->begin = beg+1;
	frag->end   = len;
	frag->mwt   = mw;
	ajListPush(*l,(void *)frag);
    }

    /* Overlaps */
    if(defcnt)
    {
	ajListReverse(*l);
	ajListToarray(*l,(void ***)&ptr);

	for(i=0;i<defcnt-1;++i)
	{
	    beg = ptr[i]->begin;
	    end = ptr[i+1]->end;
	    AJNEW0(frag);
	    frag->begin = beg;
	    frag->end   = end;
	    mw = embPropCalcMolwt(p,beg-1,end-1,mwdata,mono);
	    frag->mwt = mw + EMBMOLPARDISP;
	    ajListPush(*l,(void *)frag);
	}

	AJFREE(ptr);
    }


    ajListSort(*l,embMolFragSort);
    ajIntDel(&defcut);

    return ajListGetLength(*l);
}




/* @funcstatic embMolFragSort *************************************************
**
** Sort routine for molwt fragments
**
** @param [r] a [const void*] EmbPMolFrag pointer
** @param [r] b [const void*] EmbPMolFrag pointer
**
** @return [ajint] molwt difference
******************************************************************************/

static ajint embMolFragSort(const void* a, const void* b)
{
    return (ajint)((*(EmbPMolFrag const *)a)->mwt -
		   (*(EmbPMolFrag const *)b)->mwt);
}
