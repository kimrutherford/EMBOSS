/* @source showorf application
**
** Pretty translation of DNA sequences for publication
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
#include <string.h>
#include <ctype.h>


#define POFF 1000000




static void showorf_SixTranslate(const AjPStr substr, const AjPStr revstr,
				 ajint len,
				 AjPStr *pseqs, const AjPCod codon);
static void showorf_DoTrans(const AjPStr s, const AjPStr r, ajint n, ajint len,
			    AjPStr *pseqs, const AjPCod codon);
static void showorf_MakeRuler(ajint len, ajint begin, char *ruler,
			      ajint *npos);
static void showorf_CalcProteinPos(ajint **ppos, AjPStr const *pseqs,
				   ajint len);
static void showorf_showTrans(ajint *const *ppos, const ajint *npos,
			      AjPStr const *pseqs,
			      const AjPStr substr, ajint len,
			      const ajint *mark,
			      const char *ruler, ajint begin, AjPFile outf,
			      AjBool isrule, AjBool isp, AjBool isn,
			      ajint width, const char *name);
static void showorf_showTransb(ajint * const *ppos, const ajint *npos,
			       AjPStr const *pseqs,
			       const AjPStr substr, const ajint *mark,
			       const char *ruler, AjPFile outf,
			       AjBool isrule, AjBool isp, AjBool isn,
			       ajint start, ajint end);




/* @prog showorf **************************************************************
**
** Pretty output of DNA translations
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq a;
    AjPFile outf;
    AjPCod codon;
    AjPStr codestr;
    AjPStr substr;
    AjPStr revstr;
    AjPStr *frames;
    AjPStr *f;
    AjBool isrule;
    AjBool isp;
    AjBool isn;


    AjPStr pseqs[6];
    char *ruler;

    ajint *npos = NULL;
    ajint *ppos[6];
    ajint mark[6];

    ajint beg;
    ajint end;
    ajint len;
    ajint i;
    ajint v = 0;
    ajint gcode;

    const char *p;

    ajint width;

    embInit("showorf", argc, argv);

    a         = ajAcdGetSeq("sequence");
    codestr   = ajAcdGetListSingle("table");
    width     = ajAcdGetInt("width");
    outf      = ajAcdGetOutfile("outfile");
    frames    = ajAcdGetList("frames");
    isrule    = ajAcdGetBoolean("ruler");
    isp       = ajAcdGetBoolean("plabel");
    isn       = ajAcdGetBoolean("nlabel");

    ajStrToInt(codestr, &gcode);
    codon     = ajCodNewCodenum(gcode);

    for(i=0;i<6;++i)
	mark[i]=0;
    f = frames;
    while(*f)
    {
	p = ajStrGetPtr(*f);
	sscanf(p,"%d",&v);
	if(!v)
	{
	    for(i=0;i<6;++i)
		mark[i] = 0;
	    break;
	}
	mark[v-1] = 1;
	++f;
    }


    beg = ajSeqGetBegin(a);
    end = ajSeqGetEnd(a);

    substr = ajStrNew();
    ajStrAssignSubC(&substr,ajSeqGetSeqC(a),beg-1,end-1);
    len = ajStrGetLen(substr);
    
    revstr = ajStrNewC(ajStrGetPtr(substr));
    ajSeqstrReverse(&revstr);

    /* Allocate memory for translations and positions */
    for(i=0;i<6;++i)
    {
	pseqs[i] = ajStrNewC(ajStrGetPtr(substr));
	AJCNEW(ppos[i], len);
    }
    AJCNEW (ruler, len);
    AJCNEW (npos, len);

    showorf_SixTranslate(substr,revstr,len,pseqs,codon);
    showorf_MakeRuler(len,beg,ruler,npos);
    showorf_CalcProteinPos(ppos,pseqs,len);
    showorf_showTrans(ppos,npos,pseqs,substr,len,mark,ruler,beg,
		      outf,isrule,isp,isn,width,ajSeqGetNameC(a));


    for(i=0;i<6;++i)
    {
	ajStrDel(&pseqs[i]);
	AJFREE (ppos[i]);
    }
    AJFREE (ruler);
    AJFREE (npos);

    ajStrDel(&revstr);
    ajStrDel(&substr);
    ajCodDel(&codon);

    ajSeqDel(&a);
    ajFileClose(&outf);
    ajStrDel(&codestr);
    ajStrDelarray(&frames);

    embExit();

    return 0;
}




/* @funcstatic showorf_SixTranslate *******************************************
**
** Translate in all six frames
**
** @param [r] substr [const AjPStr] sequence
** @param [r] revstr [const AjPStr] reverse sequence
** @param [r] len [ajint] sequence length
** @param [w] pseqs [AjPStr*] translated sequences
** @param [r] codon [const AjPCod] codon usage
** @@
******************************************************************************/

static void showorf_SixTranslate(const AjPStr substr, const AjPStr revstr,
				 ajint len,
				 AjPStr *pseqs,
				 const AjPCod codon)
{
    ajint i;

    for(i=0;i<6;++i)
    {
	showorf_DoTrans(substr, revstr, i, len, pseqs, codon);
	if(i>2)
	    ajStrReverse(&pseqs[i]);
    }

    return;
}




/* @funcstatic showorf_DoTrans ************************************************
**
** Translate a single frame
**
** @param [r] s [const AjPStr] sequence
** @param [r] r [const AjPStr] reverse sequence
** @param [r] n [ajint] frame
** @param [r] len [ajint] sequence length
** @param [w] pseqs [AjPStr*] translations
** @param [r] codon [const AjPCod] codon usage
** @@
******************************************************************************/


static void showorf_DoTrans(const AjPStr s, const AjPStr r, ajint n, ajint len,
			    AjPStr *pseqs, const AjPCod codon)
{
    const char *p;
    char *q;

    ajint po;

    ajint i;
    ajint c;
    ajint idx;

    char tri[4];


    if(n<3)
    {
	p  = ajStrGetPtr(s);
	po = n%3;
    }
    else
    {
	p  = ajStrGetPtr(r);
	po = len%3;
	po -= n%3;
	if(po<0)
	    po += 3;
    }

    for(i=0,q=ajStrGetuniquePtr(&pseqs[n]);i<po;++i)
	q[i] = ' ';


    for(i=po,c=0,tri[3]='\0';i<len;++i,++c)
    {
	if(c%3 || len-i<3)
	{
	    q[i] = ' ';
	    continue;
	}

	tri[0] = (char)toupper((int)p[i]);
	tri[1] = (char)toupper((int)p[i+1]);
	tri[2] = (char)toupper((int)p[i+2]);
	if(!strcmp(tri,"NNN"))
	{
	    q[i] = 'X';
	    continue;
	}
	idx = ajCodIndexC(tri);
	if(codon->aa[idx]==27)
	    q[i] = '*';
	else
	    q[i] = (char)(codon->aa[idx]+'A');
    }

    return;
}




/* @funcstatic showorf_MakeRuler **********************************************
**
** Create ruler string with a tick every ten bases
**
** @param [r] len [ajint] ruler length
** @param [r] begin [ajint] start point
** @param [w] ruler [char*] ruler string
** @param [w] npos [ajint*] ruler numbering
** @@
******************************************************************************/


static void showorf_MakeRuler(ajint len, ajint begin, char *ruler, ajint *npos)
{
    ajint i;

    for(i=0;i<len;++i)
    {
	npos[i] = i+begin;
	if(!((i+begin)%10))
	    ruler[i] = '|';
	else
	    ruler[i] = '-';
    }

    return;
}




/* @funcstatic showorf_CalcProteinPos *****************************************
**
** Calculate numbering of translated sequences
**
** @param [w] ppos [ajint**] protein positions
** @param [r] pseqs [AjPStr const *] protein sequences
** @param [r] len [ajint] length
** @@
******************************************************************************/

static void showorf_CalcProteinPos(ajint **ppos, AjPStr const *pseqs,
				   ajint len)
{
    ajint i;
    ajint j;

    ajint pos;
    ajint v;

    const char *p;


    for(i=0;i<3;++i)
    {
	pos = 0;
	v   = 1;

	p = ajStrGetPtr(pseqs[i]);
	while(p[pos]==' ')
	    ppos[i][pos++]=0;

	while(pos<len)
	{
	    if(p[pos]=='*')
	    {
		ppos[i][pos]=0;
		++pos;
		while(p[pos]==' ')
		{
		    ppos[i][pos] = 0;
		    ++pos;
		}
		v = 1;
		continue;
	    }

	    if(p[pos]!=' ')
	    {
		ppos[i][pos]=v+POFF;
		++pos;
		for(j=0;j<2 && p[pos];++j,++pos)
		    ppos[i][pos] = v;
		if(p[pos]==' ')
		    v = 1;
		else
		    ++v;
	    }
	    else
		ppos[i][pos++] = 0;
	}
    }





    for(i=3;i<6 && len;++i)
    {
	pos = len-1;
	v   = 1;

	p=ajStrGetPtr(pseqs[i]);
	while(p[pos]==' ')
	    ppos[i][pos--]=0;

	while(pos>-1)
	{
	    if(p[pos]=='*')
	    {
		ppos[i][pos]=0;
		--pos;
		while(p[pos]==' ')
		{
		    ppos[i][pos]=0;
		    --pos;
		}
		v = 1;
		continue;
	    }

	    if(p[pos]!=' ')
	    {
		ppos[i][pos]=v+POFF;
		--pos;
		for(j=0;j<2 && p[pos];++j,--pos)
		    ppos[i][pos] = v;
		if(pos<0) continue;
		if(p[pos]==' ')
		    v = 1;
		else
		    ++v;
	    }
	    else
		ppos[i][pos--] = 0;
	}
    }

    return;
}




/* @funcstatic showorf_showTrans **********************************************
**
** Output translations using given width
**
** @param [r] ppos [ajint* const *] protein positions
** @param [r] npos [const ajint*] lengths
** @param [r] pseqs [AjPStr const *] protein sequences
** @param [r] substr [const AjPStr] mRNA sequence
** @param [r] len [ajint] mRNA length
** @param [r] mark [const ajint*] Undocumented
** @param [r] ruler [const char*] ruler
** @param [r] begin [ajint] start pos in mRNA
** @param [u] outf [AjPFile] outfile
** @param [r] isrule [AjBool] show ruler
** @param [r] isp [AjBool] protein
** @param [r] isn [AjBool] dna
** @param [r] width [ajint] print width
** @param [r] name [const char*] seq name
** @@
******************************************************************************/


static void showorf_showTrans(ajint * const *ppos, const ajint *npos,
			      AjPStr const *pseqs,
			      const AjPStr substr, ajint len,
			      const ajint *mark,
			      const char *ruler, ajint begin, AjPFile outf,
			      AjBool isrule, AjBool isp, AjBool isn,
			      ajint width, const char *name)
{
    ajint pos;

    ajFmtPrintF(outf,"SHOWORF of %s from %d to %d\n\n",name,begin,
		begin+len-1);


    pos = 0;
    while(pos<len)
    {
	if(pos+width<len)
	{
	    showorf_showTransb(ppos,npos,pseqs,substr,mark,ruler,
			       outf,isrule,isp,isn,pos,pos+width-1);
	    pos += width;
	    continue;
	}
	showorf_showTransb(ppos,npos,pseqs,substr,mark,ruler,
			   outf,isrule,isp,isn,pos,len-1);
	break;
    }

    return;
}




/* @funcstatic showorf_showTransb *********************************************
**
** Low level output
**
** @param [r] ppos [ajint* const *] protein positions
** @param [r] npos [const ajint*] lengths
** @param [r] pseqs [AjPStr const *] protein sequences
** @param [r] substr [const AjPStr] mRNA sequence
** @param [r] mark [const ajint*] Undocumented
** @param [r] ruler [const char*] ruler
** @param [u] outf [AjPFile] outfile
** @param [r] isrule [AjBool] show ruler
** @param [r] isp [AjBool] protein
** @param [r] isn [AjBool] dna
** @param [r] start [ajint] start
** @param [r] end [ajint] end
** @@
******************************************************************************/


static void showorf_showTransb(ajint * const *ppos, const ajint *npos,
			       AjPStr const *pseqs,
			       const AjPStr substr,
			       const ajint *mark,
			       const char *ruler, AjPFile outf,
			       AjBool isrule, AjBool isp, AjBool isn,
			       ajint start, ajint end)
{
    AjPStr s;
    static const char *fr[]=
    {
	"F1","F2","F3","R1","R2","R3"
    };
    ajint i;
    ajint b;
    ajint e = 0;
    ajint v;
    ajint pos;

    s = ajStrNew();

    if(isrule)
    {
	ajStrAssignSubC(&s,ruler,start,end);
	ajFmtPrintF(outf,"           %s\n",ajStrGetPtr(s));
    }

    if(isn)
	ajFmtPrintF(outf,"%10d ",npos[start]);
    else
	ajFmtPrintF(outf,"           ");

    ajStrAssignSubS(&s,substr,start,end);
    ajFmtPrintF(outf,"%s ",ajStrGetPtr(s));

    if(isn)
	ajFmtPrintF(outf,"%d",npos[end]);

    ajFmtPrintF(outf,"\n");

    for(i=0;i<3;++i)
    {
	if(!mark[i])
	    continue;
	ajFmtPrintF(outf,"%s ",fr[i]);
	if(isp)
	{
	    pos = start;
	    b = e = 0;
	    while(pos<=end)
	    {
		if(!(v=ppos[i][pos]))
		{
		    ++pos;
		    continue;
		}
		if(v<POFF)
		{
		    while(ppos[i][pos]==v && pos<=end)
			++pos;
		    if(pos>end)
			break;
		    continue;
		}
		b = v-POFF;
		break;
	    }
	    pos = end;
	    while(pos>=start)
	    {
		if(!(v=ppos[i][pos]))
		{
		    --pos;
		    continue;
		}
		if(v>POFF)
		    v -= POFF;
		else
		    while(ppos[i][pos]==v)
		    {
			--pos;
			if(pos<start)
			{
			    v = 0;
			    break;
			}
		    }
		e = v;
		break;
	    }

	    if(!b)
		ajFmtPrintF(outf,"        ");
	    else
		ajFmtPrintF(outf,"%7d ",b);
	}
	else
	    ajFmtPrintF(outf,"        ");

	ajStrAssignSubS(&s,pseqs[i],start,end);
	ajFmtPrintF(outf,"%s ",ajStrGetPtr(s));
	if(isp && e)
	    ajFmtPrintF(outf,"%d",e);
	ajFmtPrintF(outf,"\n");
    }



    for(i=3;i<6;++i)
    {
	if(!mark[i])
	    continue;
	ajFmtPrintF(outf,"%s ",fr[i]);
	if(isp)
	{
	    pos = start;
	    b = e = 0;
	    while(pos<=end)
	    {
		if(!(v=ppos[i][pos]))
		{
		    ++pos;
		    continue;
		}
		if(v>POFF)
		    v -= POFF;
		else
		    while(ppos[i][pos]==v)
		    {
			++pos;
			if(pos>end)
			{
			    v = 0;
			    break;
			}
		    }

		b = v;
		break;
	    }
	    pos = end;
	    while(pos>=start)
	    {
		if(!(v=ppos[i][pos]))
		{
		    --pos;
		    continue;
		}
		if(v<POFF)
		{
		    while(ppos[i][pos]==v && pos>=start)
			--pos;
		    if(pos<start)
			break;
		    continue;
		}
		e = v-POFF;
		break;
	    }

	    if(!b)
		ajFmtPrintF(outf,"        ");
	    else
		ajFmtPrintF(outf,"%7d ",b);
	}
	else
	    ajFmtPrintF(outf,"        ");

	ajStrAssignSubS(&s,pseqs[i],start,end);
	ajFmtPrintF(outf,"%s ",ajStrGetPtr(s));

	if(isp && e)
	    ajFmtPrintF(outf,"%d",e);

	ajFmtPrintF(outf,"\n");
    }

    ajFmtPrintF(outf,"\n");

    ajStrDel(&s);

    return;
}
