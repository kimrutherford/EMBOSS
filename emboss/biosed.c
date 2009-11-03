/* @source biosed application
**
** Replace or delete sequence sections
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
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

#include "emboss.h"


static void biosed_replace(AjPStr *substr,
			   const AjPStr target, const AjPStr replace,
			   ajint targetpos, ajint begin);
static void biosed_delete(AjPStr *substr, const AjPStr target,
			   ajint targetpos, ajint begin);




/* @prog biosed ***************************************************************
**
** Replace or delete sequence sections
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq       seq = NULL;
    AjPSeqall    seqall;
    AjPStr	 target;
    AjPStr	 replace;
    AjBool       delete;
    AjPSeqout    outseq;
    ajint        targetpos;

    AjPStr       substr = NULL;
    AjPStr       str    = NULL;

    ajint        begin;
    ajint        end;

    const char *p = NULL;

    embInit("biosed", argc, argv);

    seqall  = ajAcdGetSeqall("sequence");
    delete  = ajAcdGetToggle("delete");
    target  = ajAcdGetString("target");
    targetpos  = ajAcdGetInt("position");
    replace = ajAcdGetString("replace");
    outseq  = ajAcdGetSeqout("outseq");

    substr = ajStrNew();
    str    = ajStrNew();

    ajStrFmtUpper(&target);
    ajStrFmtUpper(&replace);

    while(ajSeqallNext(seqall,&seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);

	p = ajSeqGetSeqC(seq);
	ajStrAssignSubC(&substr,p,begin-1,end-1);
	ajStrFmtUpper(&substr);

	if(!delete)
	    biosed_replace(&substr,target,replace,targetpos, begin);
	else
	    biosed_delete(&substr,target,targetpos, begin);

	ajSeqAssignSeqS(seq,substr);
	ajSeqoutWriteSeq(outseq,seq);
    }

    ajStrDel(&substr);
    ajStrDel(&str);
    ajSeqoutClose(outseq);

    ajSeqoutDel(&outseq);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&replace);
    ajStrDel(&target);

    embExit();

    return 0;
}




/* @funcstatic biosed_replace *************************************************
**
** Generic (unoptimised) replacement of all matching string subsections
**
** @param [w] substr [AjPStr *] sequence
** @param [r] target [const AjPStr] target pattern
** @param [r] replace [const AjPStr] replacement subsequence
** @param [r] targetpos [ajint] target position
** @param [r] begin [ajint] first base of sequence
**
** @return [void]
** @@
******************************************************************************/

static void biosed_replace(AjPStr *substr,
			   const AjPStr target, const AjPStr replace,
			   ajint targetpos, ajint begin)
{
    AjPStr str = NULL;
    AjPStr tmp = NULL;

    ajint  tlen;
    ajint  end = 0;

    const char   *p = NULL;
    const char   *q  = NULL;
    const char   *v;

    str = ajStrNew();
    tmp = ajStrNew();
    p   = ajStrGetPtr(*substr);
    v   = ajStrGetPtr(target);

    tlen = ajStrGetLen(target);

    if(targetpos)
    {
	q = p;
	p += (targetpos - begin);
	end = (ajint)(p - q) - 1;
	if(ajCharPrefixS(p, target)) {
	    if(end > -1)
	    {
		ajStrAssignSubC(&tmp,q,0,end);
		ajStrAppendS(&str,tmp);
	    }
	    p += tlen;
	    ajStrAppendS(&str,replace);
	    ajStrAppendC(&str,p);
	    ajStrAssignS(substr,str);
	}
    }
    else {
	while((q=strstr(p,v)))
	{
	    end = (ajint)(q-p) - 1;
	    
	    if(end > -1)
	    {
		ajStrAssignSubC(&tmp,p,0,end);
		ajStrAppendS(&str,tmp);
	    }
	    
	    ajStrAppendS(&str,replace);
	    p = q+tlen;
	}
	ajStrAppendC(&str,p);
	ajStrAssignS(substr,str);
    }

    ajStrDel(&str);
    ajStrDel(&tmp);

    return;
}




/* @funcstatic biosed_delete **************************************************
**
** Generic (unoptimised) delete of all matching string subsections
**
** @param [w] substr [AjPStr*] sequence
** @param [r] target [const AjPStr] target pattern
** @param [r] targetpos [ajint] target position
** @param [r] begin [ajint] first base of sequence
**
** @return [void]
** @@
******************************************************************************/

static void biosed_delete(AjPStr *substr, const AjPStr target,
			   ajint targetpos, ajint begin)
{
    AjPStr str = NULL;
    char   *p  = NULL;
    char   *q  = NULL;
    const char   *v  = NULL;
    char   *t  = NULL;
    ajint  tlen = 0;
    AjPStr tmp = NULL;
    ajint end;

    str = ajStrNew();
    tmp = ajStrNew();
    ajStrAssignS(&str,*substr);
    p = ajStrGetuniquePtr(&str);
    v = ajStrGetPtr(target);
    tlen = ajStrGetLen(target);

    if(targetpos)
    {
	q = p;
	p += (targetpos - begin);
	end = (ajint)(p - q) - 1;
	if(ajCharPrefixS(p, target))
	{
	    if(end > -1)
	    {
		ajStrAssignSubC(&tmp,q,0,end);
	    }
	    p += tlen;
	    ajStrAppendC(&tmp,p);
	    ajStrAssignS(substr,tmp);
	}
    }
    else
    {
	while((q=strstr(p,v)))
	{
	    t = q + tlen;
	    p = t;
	    while((*q++=*p++));
	    p = t-1;
	}
	ajStrAssignC(substr,ajStrGetPtr(str));
    }
    ajStrDel(&str);
    ajStrDel(&tmp);

    return;
}
