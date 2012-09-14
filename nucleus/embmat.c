/* @source embmat *************************************************************
**
** General match routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.22 $
** @modified $Date: 2011/11/08 15:12:52 $ by $Author: rice $
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

#include "embmat.h"
#include "ajlist.h"
#include "ajfile.h"
#include "ajfiledata.h"


#include <stdlib.h>
#include <limits.h>


static void matPushHitInt(const AjPStr n, const EmbPMatPrints m,
			  AjPList *l, ajint pos,
			  ajint score, ajint elem, ajint hpe, ajint hpm);




/* @funcstatic matPushHitInt **************************************************
**
** Put a matching protein matrix (EmbPMatPrints) on the heap
** as an EmbPMatMatch structure
**
** @param [r] n [const AjPStr] Sequence name
** @param [r] m [const EmbPMatPrints] Matching fingerprint element
** @param [w] l [AjPList *] List to push hits to
** @param [r] pos [ajint] Sequence position of element
** @param [r] score [ajint] Score of element
** @param [r] elem [ajint] Element number (0 to n)
** @param [r] hpe [ajint] Hits per element (so far)
** @param [r] hpm [ajint] Hits per motif (so far)
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

static void matPushHitInt(const AjPStr n, const EmbPMatPrints m,
			  AjPList *l, ajint pos,
			  ajint score, ajint elem, ajint hpe, ajint hpm)
{
    EmbPMatMatch mat;

    AJNEW0 (mat);
    mat->seqname = ajStrNewC(ajStrGetPtr(n));
    mat->cod     = ajStrNewC(ajStrGetPtr((m)->cod));
    mat->acc     = ajStrNewC(ajStrGetPtr((m)->acc));
    mat->tit     = ajStrNewC(ajStrGetPtr((m)->tit));
    mat->pat     = ajStrNew();
    mat->n       = (m)->n;
    mat->len     = (m)->len[elem];
    mat->thresh  = (m)->thresh[elem];
    mat->max     = (m)->max[elem];
    mat->element = elem;
    mat->start   = pos;
    mat->score   = score;
    mat->hpe     = hpe;
    mat->hpm     = hpm;
    mat->all	 = ajFalse;
    mat->ordered = ajFalse;

    ajListPush(*l,(void *)mat);

    return;
}




/* @func embMatMatchDel *******************************************************
**
** Deallocate a MatMatch structure
**
** @param [w] s [EmbPMatMatch *] Structure to delete
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embMatMatchDel(EmbPMatMatch *s)
{

    ajStrDel(&(*s)->seqname);
    ajStrDel(&(*s)->cod);
    ajStrDel(&(*s)->acc);
    ajStrDel(&(*s)->tit);
    ajStrDel(&(*s)->pat);
    ajStrDel(&(*s)->iso);
    AJFREE(*s);
    *s = NULL;

    return;
}




/* @func embMatPrintsInit *****************************************************
**
** Initialise file pointer to the EMBOSS PRINTS data file
**
** @param [w] fp [AjPFile *] file pointer
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embMatPrintsInit(AjPFile *fp)
{
    *fp = ajDatafileNewInNameC(PRINTS_MAT);

    if(!*fp)
	ajFatal("%s file not found. Create it with printsextract.", PRINTS_MAT);

    return;
}




/* @func embMatProtReadInt ****************************************************
**
** Fill a protein matrix structure (EmbPMatPrints) from a data file
** Gets next entry.
**
** @param [u] fp [AjPFile] data file pointer
**
** @return [EmbPMatPrints] matrix structure
** @category new [EmbPMatPrints] Read protein matrix structure from a data file
**
** @release 1.0.0
******************************************************************************/

EmbPMatPrints embMatProtReadInt(AjPFile fp)
{
    EmbPMatPrints ret;
    AjPStr line;

    ajint i;
    ajuint j;
    ajuint m;
    const char *p;

    line = ajStrNewC("#");

    p = ajStrGetPtr(line);

    while(!*p || *p=='#' || *p=='!' || *p=='\n')
    {
	if(!ajReadlineTrim(fp,&line))
	{
	    ajStrDel(&line);
	    return NULL;
	}

	p = ajStrGetPtr(line);
    }

    ajDebug("embMatProtReadint starting\n");
    ajDebug ("Line: %S\n", line);

    AJNEW0 (ret);

    ret->cod = ajStrNew();
    ajStrAssignS(&ret->cod,line);

    ajReadlineTrim(fp,&line);
    ret->acc = ajStrNew();
    ajStrAssignS(&ret->acc,line);
    ajReadlineTrim(fp,&line);
    ajStrToUint(line,&ret->n);
    ajReadlineTrim(fp,&line);
    ret->tit = ajStrNew();
    ajStrAssignS(&ret->tit,line);

    ajDebug ("Lineb: %S\n", line);
    AJCNEW(ret->len, ret->n);
    AJCNEW(ret->max, ret->n);
    AJCNEW(ret->thresh, ret->n);
    AJCNEW(ret->matrix, ret->n);

    for(m=0;m<ret->n;++m)
    {
	ajReadlineTrim(fp,&line);
	ajStrToUint(line,&ret->len[m]);
	ajReadlineTrim(fp,&line);
	ajStrToUint(line,&ret->thresh[m]);
	ajReadlineTrim(fp,&line);
	ajStrToUint(line,&ret->max[m]);
	ajDebug ("m: %d/%d len:%d thresh:%d max:%d\n",
		 m, ret->n, ret->len[m], ret->thresh[m], ret->max[m]);

	for(i=0;i<26;++i)
	{
	    AJCNEW0(ret->matrix[m][i], ret->len[m]);
	    ajReadlineTrim(fp,&line);
	    ajDebug ("Linec [%d][%d]: %S\n", m, i, line);
	    p = ajStrGetPtr(line);

	    for(j=0;j<ret->len[m];++j)
	    {
		if(!j)
		    p = ajSysFuncStrtok(p," ");
		else
		    p = ajSysFuncStrtok(NULL," ");

		sscanf(p,"%u",&ret->matrix[m][i][j]);
	    }
	}
    }

    ajReadlineTrim(fp,&line);
    ajDebug ("Linec: %S\n", line);

    ajStrDel(&line);

    return ret;
}




/* @func embMatProtDelInt *****************************************************
**
** Deallocate a protein matrix structure (EmbPMatPrints)
**
** @param [d] s [EmbPMatPrints *] matrix structure
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embMatProtDelInt(EmbPMatPrints *s)
{
    ajint n;
    ajint i;
    ajint j;

    n = (*s)->n;

    for(i=0;i<n;++i)
	for(j=0;j<26;++j)
	    AJFREE((*s)->matrix[i][j]);

    AJFREE((*s)->matrix);
    AJFREE((*s)->len);
    AJFREE((*s)->thresh);
    AJFREE((*s)->max);

    ajStrDel(&(*s)->cod);
    ajStrDel(&(*s)->acc);
    ajStrDel(&(*s)->tit);
    AJFREE(*s);

    return;
}




/* @func embMatProtScanInt ****************************************************
**
** Scan a protein sequence with a fingerprint
**
** @param [r] s [const AjPStr] Sequence
** @param [r] n [const AjPStr] name of sequence
** @param [r] m [const EmbPMatPrints] Fingerprint matrix
** @param [w] l [AjPList *] List to push hits to
** @param [w] all [AjBool *] Set if all elements match
** @param [w] ordered [AjBool *] Set if all elements are in order
** @param [r] overlap [AjBool] True if overlaps are allowed
**
** @return [ajuint] number of hits
**
** @release 1.0.0
******************************************************************************/

ajuint embMatProtScanInt(const AjPStr s, const AjPStr n, const EmbPMatPrints m,
			AjPList *l,
			AjBool *all, AjBool *ordered, AjBool overlap)
{
    EmbPMatMatch mm;
    AjPStr t;
    char   *p;
    char   *q;
    ajint slen;
    ajint score;
    ajint mlen;
    ajint elem;
    ajint minpc;
    ajint maxscore;
    ajint limit;
    ajint sum;
    ajint hpe;
    ajint hpm;

    ajint lastelem;
    ajint lastpos;
    ajint op;

    ajint i;
    ajint j;

    t = ajStrNewC(ajStrGetPtr(s));
    ajStrFmtUpper(&t);
    p = q = ajStrGetuniquePtr(&t);
    slen = ajStrGetLen(t);

    for(i=0;i<slen;++i,++p)
	*p = ajSysCastItoc(ajBasecodeToInt((ajint)*p));

    p = q;

    *all = *ordered = ajTrue;
    lastelem = lastpos = INT_MAX;

    hpm=0;

    for(elem=(m)->n - 1;elem >= 0;--elem)
    {
	hpe = 0;

	mlen     = (m)->len[elem];
	minpc    = (m)->thresh[elem];
	maxscore = (m)->max[elem];

	limit = slen-mlen;

	for(i=0;i<limit;++i)
	{
	    sum = 0;

	    for(j=0;j<mlen;++j)
		sum += (m)->matrix[elem][(ajint) p[i+j]][j];

	    score = (sum*100)/maxscore;

	    if(score>=minpc)
	    {
		if(elem<lastelem && *ordered)
		{
		    if(lastelem == INT_MAX)
		    {
			lastelem = elem;
			lastpos  = i;
		    }
		    else
		    {
			lastelem = elem;
			op = i;

			if(!overlap)
			    op += mlen;

			if(op >= lastpos)
			    *ordered = ajFalse;

			lastpos = i;
		    }
		}

		++hpe;
		++hpm;
		matPushHitInt(n,m,l,i,score,elem,hpe,hpm);
	    }
	}

	if(!hpe)
	    *all = ajFalse;
    }

    if(hpm)
    {
	ajListPop(*l,(void **)&mm);

	if(*all)
	{
	    mm->all = ajTrue;

	    if(*ordered)
		mm->ordered = ajTrue;
	    else
		mm->ordered = ajFalse;
	}
	else
	{
	    mm->all = ajFalse;

	    if(*ordered)
		mm->ordered = ajTrue;
	    else
		mm->ordered = ajFalse;
	}

	ajListPush(*l,(void *)mm);
    }

    ajStrDel(&t);

    return hpm;
}
