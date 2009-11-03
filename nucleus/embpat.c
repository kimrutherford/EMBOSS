/* @source embpat.c
**
** General routines for pattern matching.
** Copyright (C) Alan Bleasby 1999
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
#include "embpat.h"
#include "embmat.h"
#include "stdlib.h"
#include "limits.h"


/* @datastatic PatPTypes ***************************************************
**
** Prosite pattern types
**
** @alias PatSTypes
** @alias PatOTypes
**
** @attr Name [const char*] Type name
** @attr Desc [const char*] Type description
** @@
******************************************************************************/

typedef struct PatSTypes
{
    const char *Name;
    const char *Desc;
} PatOTypes;

#define PatPTypes PatOTypes*

static PatOTypes patTypes[] = {
/* "Name",        "Description" */
  {"BMH",     "Boyer Moore Horspool pattern"},
  {"BYP",     "Baeza-Yates Perleberg pattern"},
  {"SO",     "Shift-OR pattern"},
  {"BYGC",     "Baeza-Yates Gonnet class pattern"},
  {"Regex",     "Prosite converted to regex"},
  {"TUB",     "Tarhio-Ukkonen-Bleasby"},
  {"OUB",     "Brute force processing"},
  {NULL, NULL}
};

static void   patRestrictPushHit(const EmbPPatRestrict enz,
				 AjPList l, ajuint pos,
				 ajuint begin, ajuint len,
				 AjBool forward, AjBool plasmid);

static void   patAminoCarboxyl(const AjPStr s,AjPStr *cs,
			       AjBool *amino, AjBool *carboxyl);
static AjBool patParenTest(const char *p, AjBool *repeat, AjBool *range);
static AjBool patExpandRepeat(AjPStr *s);
static void   patIUBTranslate(AjPStr *pat);
static void   patProteinTranslate(AjPStr *pat);
static AjBool patBruteClass(const char *p, char c);
static AjBool patBruteCompl(const char *p, char c);
static AjBool patBruteIsRange(const char *t, ajuint *x, ajuint *y);
static AjBool patBruteCharMatch(const char *t, char c);
static ajuint  patBruteNextPatChar(const char *t, ajuint ppos);
static AjBool patOUBrute(const char *seq, const char *pat, ajuint spos,
			 ajuint ppos, ajuint mm,
			 ajuint omm, ajuint level, AjPList l, AjBool carboxyl,
			 ajuint begin, ajuint *count, const AjPStr name,
			 ajuint st);




/* @funcstatic patStringFree **************************************************
**
** Free a pattern structure, through an ajListMap call
**
** @param [d] x [void**] pattern
** @param [r] cl [void*] Optional function - required by ajListMap but unused
** @return [void]
** @@
******************************************************************************/

static void patStringFree(void **x, void *cl)
{
    ajuint **ptr = (ajuint **)x;

    (void) cl;				/* make it used */
    AJFREE(*ptr);

    return;
}




/* @func embPatSeqCreateRegExp ************************************************
**
** Create a regular expression for a string and substitute the chars for
** Nucleotides or proteins as needed.
**
** @param [r] thys [const AjPStr] string to create reg expr from.
** @param [r] protein [AjBool] is it a protein.
**
** @return [AjPStr] the new regular expression.
******************************************************************************/

AjPStr embPatSeqCreateRegExp(const AjPStr thys, AjBool protein)
{
    return embPatSeqCreateRegExpC(ajStrGetPtr(thys), protein);
}




/* @func embPatSeqCreateRegExpC ***********************************************
**
** Create a regular expression for a string and substitute the chars for
** Nucleotides or proteins as needed.
**
** @param [r] ptr [const char *] text to create reg expr from.
** @param [r] protein [AjBool] is it a protein.
**
** @return [AjPStr] the new regular expression.
******************************************************************************/

AjPStr embPatSeqCreateRegExpC(const char *ptr, AjBool protein)
{

/* codes for A-Z including ambiguity codes */

    const char *nucpatternmatch[] =
    {
	"[Aa]", "[CcGgTtUu]", "[Cc]", "[AaGgTtUu]",
	"", "", "[Gg]", "[AaCcTtUu]",
	"", "", "", "",
	"[AaCc]", "[A-Za-z]", "", "",
	"", "[AaGg]", "[GgCc]", "[TtUu]",
	"[TtUu]","[AaCcGg]", "[AaTtUu]", "[A-Za-z]",
	"[CTU]", ""
    };

    const char *protpatternmatch[] =
    {
	"[Aa]","[DdNn]","[Cc]","[Dd]",
	"[Ee]","[Ff]","[Gg]","[AaCcTtUu]",
	"[Ii]","","[Kk]","[Ll]",
	"[Mm]","[A-Za-z]","","[Pp]",
	"[Qq]","[Rr]","[Ss]","[Tt]",
	"[Uu]","[Vv]","[Ww]","[A-Za-z]",
	"[Yy]","[EeQq]"
    };

    AjPStr regexp  = 0;
    ajuint match;
    char match2[2] = " ";
    const char* optr = ptr;

    regexp = ajStrNewRes((ajuint)strlen(ptr) * 4); /* just a rough guess */

    while(*ptr != '\0')
    {
	/* alphabetic characters converted to character sets */
	if((*ptr > 64 && *ptr < 91) || (*ptr > 96 && *ptr < 123))
	{
	    if(*ptr > 91)
		match = ((ajuint) *ptr) - 97;
	    else
		match = ((ajuint) *ptr) - 65;

	    if(protein)
		ajStrAppendC(&regexp,protpatternmatch[match]);
	    else
		ajStrAppendC(&regexp,nucpatternmatch[match]);
	}
	else
	{
	    match2[0] = *ptr;
	    ajStrAppendC(&regexp,match2);
	}
	ptr++;
    }

    ajDebug("embPatSeqCreateRegExpC ptr: '%s' returns regexp: %S'\n",
	    optr, regexp);

    return regexp;
}




/* @func embPatSeqMatchFind ***************************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] seq [const AjPSeq] Sequence to be searched.
** @param [r] reg [const AjPStr] regular expression string.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatSeqMatchFind(const AjPSeq seq, const AjPStr reg)
{
    return embPatSeqMatchFindC(seq, ajStrGetPtr(reg));
}




/* @func embPatSeqMatchFindC **************************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] seq [const AjPSeq] Sequence to be searched.
** @param [r] reg [const char*] regular expression text.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatSeqMatchFindC(const AjPSeq seq, const char *reg)
{
    AjPStr regexp = NULL;
    AjBool protein;
    EmbPPatMatch results;

    protein = ajSeqIsProt(seq);

    regexp  = embPatSeqCreateRegExpC(reg,protein);
    results = embPatMatchFind(regexp, ajSeqGetSeqS(seq), ajFalse, ajFalse);

    ajStrDel(&regexp);

    return results;
}




/* @func embPatMatchFind ******************************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] regexp [const AjPStr] Regular expression string.
** @param [r] strng [const AjPStr] String to be searched.
** @param [r] left [AjBool] has to match the start
** @param [r] right [AjBool] has to match the end
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatMatchFind(const AjPStr regexp, const AjPStr strng,
			     AjBool left, AjBool right)
{
    return embPatMatchFindC(regexp, ajStrGetPtr(strng), left, right);
}




/* @func embPatMatchFindC *****************************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] regexp [const AjPStr] Regular expression string.
** @param [r] sptr   [const char *] String to be searched.
** @param [r] left [AjBool] has to match the start
** @param [r] right [AjBool] has to match the end
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatMatchFindC(const AjPStr regexp, const char *sptr,
			     AjBool left, AjBool right)
{
    AjPRegexp regcomp = NULL;
    EmbPPatMatch results;
    AjPList poslist = ajListNew();
    AjPList lenlist = ajListNew();
    AjIList iter;
    ajuint *pos;
    ajuint *len;
    ajuint posi;
    ajuint i;
    const char *ptr;
    AjBool nterm = ajFalse;
/*    AjPListNode node;*/
    AjPStr regstr = NULL;

    if(*regexp->Ptr == '^')
	nterm  = ajTrue;

    regstr = ajStrNewS(regexp);
    if(left)
    {
	if(!nterm)
	    ajStrInsertC(&regstr, 0, "^");
	nterm = ajTrue;
    }
    if(right)
	ajStrAppendC(&regstr, "$");

    ajDebug("embPatMatchFindC regexp: '%S' regstr: '%S'\n",
	    regexp, regstr);

    ajDebug("embPatMatchFindC sptr '%s'\n",
	    sptr);

    regcomp = ajRegComp(regstr);

    ptr = sptr;

    AJNEW(results);

    while(*sptr != '\0' && ajRegExecC(regcomp,sptr))
    {
	AJNEW(pos);
	*pos = posi = ajRegOffset(regcomp);
	AJNEW(len);
	*len = ajRegLenI(regcomp,0);
	*pos += (ajuint) (sptr-ptr);
/*	node = ajListNodesNew(pos, NULL);*/
	ajListPush(poslist, pos);
/*	node = ajListNodesNew(len, NULL);*/
	ajListPush(lenlist, len);
	sptr += posi+1;
	if(nterm)
	    break;
    }

    ajRegFree(&regcomp);
    results->number  = ajListGetLength(poslist);

    ajDebug("embPatMatchFindC '%S' nterm:%B results: %d\n",
	    regstr, nterm, results->number);

    if(results->number)
    {
	AJCNEW(results->start, results->number);
	AJCNEW(results->len, results->number);

	i = 0;
	iter = ajListIterNewread(poslist);
	while(!ajListIterDone(iter))
	{
	    results->start[i] = *(ajuint *) ajListIterGet(iter);
	    i++;
	}
	ajListIterDel(&iter);

	i = 0;
	iter = ajListIterNewread(lenlist);
	while(!ajListIterDone(iter))
	{
	    results->len[i] = *(ajuint *) ajListIterGet(iter);
	    i++;
	}
	ajListIterDel(&iter);

	ajListMap(poslist,patStringFree, NULL);
	ajListMap(lenlist,patStringFree, NULL);
	ajListFree(&poslist);
	ajListFree(&lenlist);

    }
    else
    {
	ajListFree(&poslist);
	ajListFree(&lenlist);
    }

    ajStrDel(&regstr);

    return results;
}




/* @func embPatMatchGetLen****************************************************
**
** Returns the length from the pattern match structure for index'th item.
**
** @param [r] data [const EmbPPatMatch] results of match.
** @param [r] indexnum   [ajuint] index to structure.
**
** @return [ajuint] returns -1 if not available.
**
******************************************************************************/

ajuint embPatMatchGetLen(const EmbPPatMatch data, ajuint indexnum)
{
    if(data->number <= indexnum)
	return -1;

    return data->len[indexnum];
}




/* @func embPatMatchGetEnd****************************************************
**
** Returns the End point for the pattern match structure for index'th item.
**
** @param [r] data [const EmbPPatMatch] results of match.
** @param [r] indexnum   [ajuint] index to structure.
**
** @return [ajuint] returns -1 if not available.
**
******************************************************************************/

ajuint embPatMatchGetEnd(const EmbPPatMatch data, ajuint indexnum)
{
    if(data->number <= indexnum)
	return -1;

    return data->len[indexnum]+data->start[indexnum]-1;
}




/* @func embPatMatchGetNumber************************************************
**
** Returns the number of pattern matches in the structure.
**
** @param [r] data [const EmbPPatMatch] results of match.
**
** @return [ajuint] returns -1 if not available.
**
******************************************************************************/

ajuint embPatMatchGetNumber(const EmbPPatMatch data)
{
  return data->number;
}




/* @func embPatMatchGetStart**************************************************
**
** Returns the start position from the pattern match structure for
** index'th item.
**
** @param [r] data [const EmbPPatMatch] results of match.
** @param [r] indexnum  [ajuint] index to structure.
**
** @return [ajuint] returns -1 if not available.
**
******************************************************************************/

ajuint embPatMatchGetStart(const EmbPPatMatch data, ajuint indexnum)
{
    if(data->number <= indexnum)
	return -1;

    return data->start[indexnum];
}




/* @func embPatMatchDel *******************************************************
**
** Free all the memory from the pattern match search.
**
** @param [d] pthis [EmbPPatMatch*] results to be freed.
** @return [void]
** @category delete [EmbPPatMatch] Standard destructor
******************************************************************************/

void embPatMatchDel(EmbPPatMatch* pthis)
{
    EmbPPatMatch thys;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    if(thys->number)
    {
	AJFREE(thys->start);
	AJFREE(thys->len);
    }
    AJFREE(*pthis);

    return;
}



/* @func embPatPrositeToRegExp ************************************************
**
** Convert a prosite pattern to a regular expression
**
** Start and end are indicated by boolean options, as the pattern may have
** been processed for the other pattern matching methods
**
** @param [r] s [const AjPStr] prosite pattern
** @return [AjPStr] regular expression
******************************************************************************/

AjPStr embPatPrositeToRegExp(const AjPStr s)
{
    return embPatPrositeToRegExpEnds(s, AJFALSE, AJFALSE);
}

/* @func embPatPrositeToRegExpEnds ********************************************
**
** Convert a prosite pattern to a regular expression string.
**
** start and end say whether the ends should match in case this is a processed
** pattern, but there is still a check for angle brackets.
**
** @param [r] s [const AjPStr] prosite pattern
** @param [r] start [AjBool] must match start
** @param [r] end [AjBool] must match end
** @return [AjPStr] regular expression
******************************************************************************/

AjPStr embPatPrositeToRegExpEnds (const AjPStr s, AjBool start, AjBool end)
{
    AjPStr t;
    AjPStr c;
    AjBool isnt = start;
    AjBool isct = end;

    const char   *p;
    static const char *aa = "ACDEFGHIKLMNPQRSTVWY";
    static char ch[2];
    ajuint len;
    ajuint i;

    t   = ajStrNewC("");
    len = ajStrGetLen(s);
    if(!len)
	return t;

    c = ajStrNew();
    ajStrAssignS(&c, s);
    ajStrFmtUpper(&c);
    ajStrRemoveWhiteExcess(&c);
    ch[1]='\0';

    p = ajStrGetPtr(c);
    for(i=0;i<len;++i)
    {
	if(p[i]=='>')
	    isct = 1;

	if(p[i]=='<')
	    isnt = 1;
    }

    if(isnt)
	ajStrAppendC(&t,"^");

    while(*p)
    {
	if(*p=='?')
	{
	    ajStrAppendC(&t,"[^#]");
	    ++p;
	    continue;
	}

	if(*p=='X')
	{
	    ajStrAppendC(&t,"[^BJOUXZ]");
	    ++p;
	    continue;
	}

	if(*p=='(')
	{
	    ++p;
	    ajStrAppendC(&t,"{");
	    while(*p != ')')
	    {
		if(!*p)
		    ajFatal("Unmatched '(' in %S\n",s);

		if(*p=='<' || *p=='>')
		{
		    ++p;
		    continue;
		}

		*ch= *p;
		ajStrAppendC(&t,ch);
		++p;
	    }
	    ajStrAppendC(&t,"}");
	    ++p;
	    continue;
	}

	if(*p=='[')
	{
	    while(*p != ']')
	    {
		if(!*p)
		    ajFatal("Unmatched '[' in %S\n",s);

		if(*p=='<' || *p=='>')
		{
		    ++p;
		    continue;
		}
		*ch = *p;
		ajStrAppendC(&t,ch);
		++p;
	    }

	    ajStrAppendC(&t,"]");
	    ++p;
	    continue;
	}

	if(*p=='{')
	{
	    ++p;
	    ajStrAppendC(&t,"[^");
	    while(*p != '}')
	    {
		if(!*p)
		    ajFatal("Unmatched '{' in %S\n",s);

		if(*p=='<' || *p=='>')
		{
		    ++p;
		    continue;
		}
		*ch = *p;
		ajStrAppendC(&t,ch);
		++p;
	    }

	    ajStrAppendC(&t,"]");
	    ++p;
	    continue;
	}

	if(strchr(aa,*p))
	{
	    *ch = *p;
	    ajStrAppendC(&t,ch);
	    ++p;
	    continue;
	}

	if(!(*p==' ' || *p=='-' || *p=='>' || *p=='<'))
	    ajFatal("Unrecognised character in %S\n",s);
	++p;
    }

    if(isct)
	ajStrAppendC(&t,"$");

    ajStrAssignS(&c,t);
    ajStrDel(&t);

    return c;
}




/* @func embPatRestrictNew ****************************************************
**
** Create a new restriction object
**
** @return [EmbPPatRestrict] the allocated object
******************************************************************************/

EmbPPatRestrict embPatRestrictNew(void)
{
    EmbPPatRestrict thys;

    AJNEW0(thys);

    thys->cod  = ajStrNew();
    thys->pat  = ajStrNew();
    thys->bin  = ajStrNew();
    thys->org  = ajStrNew();
    thys->iso  = ajStrNew();
    thys->meth = ajStrNew();
    thys->sou  = ajStrNew();
    thys->sup  = ajStrNew();

    return thys;
}




/* @func embPatRestrictDel ****************************************************
**
** Delete a restriction object
**
** @param [d] thys [EmbPPatRestrict *] restriction object
** @return [void]
** @category delete [EmbPPatRestrict] Standard destructor
******************************************************************************/

void embPatRestrictDel(EmbPPatRestrict *thys)
{
    ajStrDel(&(*thys)->cod);
    ajStrDel(&(*thys)->pat);
    ajStrDel(&(*thys)->bin);
    ajStrDel(&(*thys)->org);
    ajStrDel(&(*thys)->iso);
    ajStrDel(&(*thys)->meth);
    ajStrDel(&(*thys)->sou);
    ajStrDel(&(*thys)->sup);
    AJFREE(*thys);

    return;
}




/* @func embPatRestrictReadEntry **********************************************
**
** Read next restriction enzyme from re file
**
** @param [w] re [EmbPPatRestrict] restriction object to fill
** @param [u] inf [AjPFile] input file pointer
** @return [AjBool] True if read successful
** @category input [EmbPPatRestrict] Read next restriction enzyme from file
******************************************************************************/

AjBool embPatRestrictReadEntry(EmbPPatRestrict re, AjPFile inf)
{
    AjPStr line;
    AjBool ret;
    const char *p = NULL;
    char *q = NULL;
    ajuint i;

    line = ajStrNew();
    while((ret=ajFileReadLine(inf,&line)))
    {
	p = ajStrGetPtr(line);
	if(!(!*p || *p=='#' || *p=='!'))
	    break;
    }

    if(!ret)
    {
	ajStrDel(&line);
	return ajFalse;
    }


    p = ajSysFuncStrtok(p,"\t \n");
    ajStrAssignC(&re->cod,p);
    p = ajSysFuncStrtok(NULL,"\t \n");
    ajStrAssignC(&re->pat,p);
    ajStrAssignC(&re->bin,p);

    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->len);
    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->ncuts);
    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->blunt);
    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->cut1);
    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->cut2);
    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->cut3);
    p = ajSysFuncStrtok(NULL,"\t \n");
    sscanf(p,"%d",&re->cut4);

    for(i=0,q=ajStrGetuniquePtr(&re->bin);i<re->len;++i)
	*(q+i)=ajAZToBinC(*(q+i));

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic patRestrictPushHit *********************************************
**
** Put a matching restriction enzyme on the heap
** as an EmbPMatMatch structure
**
** @param [r] enz [const EmbPPatRestrict] Enyme information
** @param [u] l [AjPList] List to add to
** @param [r] pos [ajuint] Sequence match position
** @param [r] begin [ajuint] Sequence offset
** @param [r] len [ajuint] Sequence length
** @param [r] forward [AjBool] True if forward strand
** @param [r] plasmid [AjBool] Allow circular DNA (currently ignored)
**
** @return [void]
******************************************************************************/

static void patRestrictPushHit(const EmbPPatRestrict enz,
			       AjPList l, ajuint pos,
			       ajuint begin, ajuint len,
			       AjBool forward, AjBool plasmid)
{

    EmbPMatMatch hit;
    ajuint v;

    (void) plasmid;			/* make it used */

    AJNEW0(hit);

    hit->seqname = ajStrNew();
    hit->cod = ajStrNewC(ajStrGetPtr(enz->cod));
    hit->pat = ajStrNewC(ajStrGetPtr(enz->pat));
    hit->acc = ajStrNew();
    hit->tit = ajStrNew();
    hit->iso = ajStrNew();
    hit->len = enz->len;

    if(forward)
    {
	hit->forward = 1;
	hit->start = pos+begin;
	hit->cut1 = pos+begin+enz->cut1-1;
	hit->cut2 = pos+begin+enz->cut2-1;
	if(hit->cut1 > (ajint)(len+begin-1))
	    hit->cut1-=len;

	if(hit->cut2 > (ajint)(len+begin-1))
	    hit->cut2-=len;

 	if(enz->cut1<1)
	    ++hit->cut1;

	if(enz->cut2<1)
	    ++hit->cut2;

	if(hit->cut1<1)
	{
	    hit->cut1+=len;
	    hit->circ12 = ajTrue;
	}
	if(hit->cut2<1)
	{
	    hit->cut2+=len;
	    hit->circ12 = ajTrue;
	}

	if(enz->ncuts == 4)
	{
	    hit->cut3 = pos+begin+enz->cut3-1;
	    hit->cut4 = pos+begin+enz->cut4-1;

	    if(hit->cut3>(ajint)(len+begin-1))
	    {
		hit->cut3-=len;
		hit->circ34 = ajTrue;
	    }

	    if(hit->cut4>(ajint)(len+begin-1))
	    {
		hit->cut4-=len;
		hit->circ34 = ajTrue;
	    }
	}
	else hit->cut3 = hit->cut4 = 0;
    }
    else
    {
	hit->forward = 0;
	hit->start = len+begin-pos-1;
	hit->cut1  = len+begin-pos-enz->cut1-1;
	hit->cut2  = len+begin-pos-enz->cut2-1;

	if(enz->cut1<1)
	    --hit->cut1;

	if(enz->cut2<1)
	    --hit->cut2;

	/* cuts beyond start of sequence */

	if(hit->cut1<1)
	{
	    hit->cut1+=len;
	    hit->circ12 = ajTrue;
	}
	if(hit->cut2<1)
	{
	    hit->cut2+=len;
	    hit->circ12 = ajTrue;
	}

	/* cuts beyond end of sequence */

	if(hit->cut1>(ajint)(len+begin-1))
	{
	    hit->cut1-=len;
	    hit->circ12 = ajTrue;
	}
	if(hit->cut2>(ajint)(len+begin-1))
	{
	    hit->cut2-=len;
	    hit->circ12 = ajTrue;
	}

	if(enz->ncuts == 4)
	{
	    /* ajDebug("so far, len:%d pos:%d begin:%d\n",
		    len, pos, begin); */
	    /* ajDebug("before, cut3:%d 4:%d circ34:%b\n",
		    hit->cut3, hit->cut4, hit->circ34); */

	    hit->cut3 = len+begin-pos-enz->cut3-1;
	    hit->cut4 = len+begin-pos-enz->cut4-1;

	    /* ajDebug("middle, cut3:%d 4:%d\n",
		    hit->cut3, hit->cut4); */

	    if(hit->cut3<0)
	    {
		hit->cut3+=len;
		hit->circ34 = ajTrue;
	    }

	    if(hit->cut4<0)
	    {
		hit->cut4+=len;
		hit->circ34 = ajTrue;
	    }
	    /* ajDebug("after, cut3:%d 4:%d circ34: %b\n",
		    hit->cut3, hit->cut4, hit->circ34); */
	}
	else
	    hit->cut3 = hit->cut4 = 0;

	/* Reverse them to refer to forward strand */
	v = hit->cut1;
	hit->cut1 = hit->cut2;
	hit->cut2 = v;
	v = hit->cut3;
	hit->cut3 = hit->cut4;
	hit->cut4 = v;
    }

    /* ajDebug("embPatRestrictPushHit forward:%b\n", forward); */
    /* ajDebug("cut1:%d 2:%d 3:%d 4:%d\n",
	    hit->cut1, hit->cut2, hit->cut3, hit->cut4); */
    ajListPush(l,(void *) hit);

    return;
}




/* @func embPatRestrictScan ***************************************************
**
** Scan a sequence with a restriction object
**
** @param [r] enz [const EmbPPatRestrict] Enyme information
** @param [r] substr [const AjPStr] Sequence as ASCII
** @param [r] binstr [const AjPStr] Sequence as binary IUB
** @param [r] revstr [const AjPStr] Sequence as ASCII reversed
** @param [r] binrev [const AjPStr] Sequence as binary IUB reversed
** @param [r] len [ajuint] Length of sequence
** @param [r] ambiguity [AjBool] Allow ambiguity (binary search)
** @param [r] plasmid [AjBool] Allow circular DNA
** @param [r] min [ajuint] Minimum # of matches allowed
** @param [r] max [ajuint] Maximum # of matches
** @param [r] begin [ajuint] Sequence offset
** @param [u] l [AjPList] List to push hits to
**
** @return [ajuint] Number of matches
******************************************************************************/

ajuint embPatRestrictScan(const EmbPPatRestrict enz,
			 const AjPStr substr, const AjPStr binstr,
			 const AjPStr revstr, const AjPStr binrev, ajuint len,
			 AjBool ambiguity, AjBool plasmid, ajuint min,
			 ajuint max, ajuint begin, AjPList l)
{
    ajuint limit;
    ajuint i;
    ajuint j;
    ajuint hits;
    ajuint rhits = 0;
    const char *p;
    const char *q;
    const char *t;
    ajint  mincut;
    ajint  maxcut;
    AjBool forward;
    ajint  v;
    AjPList tx     = NULL;
    AjPList ty     = NULL;
    EmbPMatMatch m = NULL;
    EmbPMatMatch z = NULL;

    if(plasmid)
	limit=len;
    else
	limit=len-enz->len+1;

    mincut=AJMIN(enz->cut1,enz->cut2);
    if(enz->ncuts==4)
    {
	mincut=AJMIN(mincut,enz->cut3);
	mincut=AJMIN(mincut,enz->cut4);
    }

    maxcut=AJMAX(enz->cut1,enz->cut2);
    if(enz->ncuts==4)
    {
	maxcut=AJMAX(maxcut,enz->cut3);
	maxcut=AJMAX(maxcut,enz->cut4);
    }

    ajDebug("embPatRestrictScan '%S' '%S' ncuts:%d blunt:%b\n",
	    enz->cod, enz->pat, enz->ncuts, enz->blunt);
    /* ajDebug("cut1:%d 2:%d 3:%d 4:%d\n",
	    enz->cut1, enz->cut2, enz->cut3, enz->cut4 ); */

    tx = ajListNew();
    ty = ajListNew();


    if(ambiguity)
    {
	p = ajStrGetPtr(binstr);
	t = ajStrGetPtr(enz->bin);

	forward = ajTrue;
	for(i=0,hits=0;i<limit;++i)
	{
	    for(j=0,q=t;j<enz->len;++j,++q)
	    {
		v=*(p+i+j);
		if(!(*q & v) || v==15)
		    break;
	    }

	    if(j==enz->len && !plasmid && (i+enz->cut1>=len ||
					      i+enz->cut2>=len))
		continue;

	    if(j==enz->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,tx,i,begin,len,forward, plasmid);
	    }

	}

	forward = ajFalse;
	p = ajStrGetPtr(binrev);
	for(i=0;i<limit;++i)
	{
	    for(j=0,q=t;j<enz->len;++j,++q)
	    {
		v = *(p+i+j);
		if(!(*q & v) || v==15)
		    break;
	    }

	    if(j==enz->len && !plasmid && (i+enz->cut1>=len ||
					      i+enz->cut2>=len))
		continue;

	    if(j==enz->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,tx,i,begin,len,forward, plasmid);
	    }

	}

    }
    else
    {
	p = ajStrGetPtr(substr);
	t = ajStrGetPtr(enz->pat);
	forward = ajTrue;
	for(i=0,hits=0;i<limit;++i)
	{
	    for(j=0,q=t;j<enz->len;++j,++q)
	    {
		v=*(p+i+j);
		if(*q != v || v=='N')
		    break;
	    }

	    if(j==enz->len && !plasmid && (i+enz->cut1>=len ||
					      i+enz->cut2>=len))
		continue;

	    if(j==enz->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,tx,i,begin,len,forward, plasmid);
	    }

	}

	forward = ajFalse;
	p = ajStrGetPtr(revstr);
	for(i=0;i<limit;++i)
	{
	    for(j=0,q=t;j<enz->len;++j,++q)
	    {
		v = *(p+i+j);
		if(*q != v || v=='N')
		    break;
	    }
	    if(j==enz->len && !plasmid && (i+enz->cut1>=len ||
					      i+enz->cut2>=len))
		continue;

	    if(j==enz->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,tx,i,begin,len,forward, plasmid);
	    }
	}
    }


    if(hits)
    {
	ajListSort(tx,embPatRestrictCutCompare);
	for(i=0,rhits=0,v=0;i<hits;++i)
	{
	    ajListPop(tx,(void **)&m);
	    if(m->cut1 != v)
	    {
		ajListPush(ty,(void *)m);
		++rhits;
		v = m->cut1;
	    }
	    else
	    {
		if(i)
		    if(m->forward)
		    {
			ajListPop(ty,(void **)&z);
			ajListPush(ty,(void *)m);
			m=z;
		    }
		embMatMatchDel(&m);
	    }
	}

	if(rhits<min || rhits>max)
	{
	    while(ajListPop(ty,(void **)&m));
	    ajListFree(&tx);
	    ajListFree(&ty);
	    return 0;
	}
	else
	{
	    while(ajListPop(ty,(void **)&m))
		ajListPush(l,(void *)m);
	    hits = rhits;
	}
    }

    ajListFree(&tx);
    ajListFree(&ty);

    return hits;
}




/* @func embPatKMPInit ********************************************************
**
** Initialise a Knuth-Morris-Pratt pattern.
**
** @param [r] pat [const AjPStr] pattern
** @param [r] len [ajuint] length of pattern
** @param [w] next [ajint *] offset table
**
** @return [void]
******************************************************************************/

void embPatKMPInit(const AjPStr pat, ajuint len, ajint *next)
{
    ajuint i;
    ajint k;
    ajuint t;
    const char *p;

    p = ajStrGetPtr(pat);
    t = len-1;

    i = 0;
    k = -1;
    next[0] = -1;
    while(i<t)
    {
	while(k>=0 && p[i]!=p[k])
	    k = next[k];
	++i;
	++k;
	if(p[i]==p[k])
	    next[i] = next[k];
	else
	    next[i] = k;
    }

    return;
}




/* @func embPatKMPSearch ******************************************************
**
** Perform a Knuth-Morris-Pratt search
**
** @param [r] str [const AjPStr] string to search
** @param [r] pat [const AjPStr] pattern to use
** @param [r] slen [ajuint] length of string
** @param [r] plen [ajuint] length of pattern
** @param [r] next [const ajint *] array from embPatKMPInit (can be -1)
** @param [r] start [ajuint] position within str to start search
**
** @return [ajuint] Index of match in str or -1 if not found
******************************************************************************/

ajuint embPatKMPSearch(const AjPStr str, const AjPStr pat,
		      ajuint slen, ajuint plen, const ajint *next,
		      ajuint start)
{
    ajuint i;
    ajint j;
    const char *p;
    const char *q;

    p = ajStrGetPtr(str);
    q = ajStrGetPtr(pat);

    i = start;
    j = 0;
    while(i<slen && j<(ajint)plen)
    {
	while(j>=0 && p[i]!=q[j])
	    j = next[j];
	++i;
	++j;
    }

    if(j==(ajint)plen)
	return i-plen;

    return -1;
}




/* @func embPatBMHInit ********************************************************
**
** Initialise a Boyer-Moore-Horspool pattern.
**
** @param [r] pat [const AjPStr] pattern
** @param [r] len [ajuint] pattern length
** @param [w] skip [ajint *] offset table (can be -1)
**
** @return [void]
******************************************************************************/

void embPatBMHInit(const AjPStr pat, ajuint len, ajint *skip)
{
    ajuint i;
    ajuint t;
    const char *p;

    p = ajStrGetPtr(pat);

    t = len-1;
    for(i=0;i<AJALPHA;++i)
	skip[i] = t;

    for(i=0;i<t;++i)
	skip[(ajuint)p[i]] = t-i;

    return;
}




/* @func embPatBMHSearch ******************************************************
**
** Perform a Boyer-Moore-Horspool search
**
** @param [r] str [const AjPStr] string to search
** @param [r] pat [const AjPStr] pattern to use
** @param [r] slen [ajuint] length of string
** @param [r] plen [ajuint] length of pattern
** @param [r] skip [const ajint *] array from embPatBMHInit (can be -1)
** @param [r] start [ajuint] position within str to start search
** @param [r] left [AjBool] has to match the start
** @param [r] right [AjBool] has to match the end
** @param [u] l [AjPList] list to push to
** @param [r] name [const AjPStr] name of entry
** @param [r] begin [ajuint] offset in orig sequence
**
** @return [ajuint] number of hits
******************************************************************************/

ajuint embPatBMHSearch(const AjPStr str, const AjPStr pat,
		      ajuint slen, ajuint plen, const ajint *skip,
		      ajuint start, AjBool left, AjBool right, AjPList l,
		      const AjPStr name, ajuint begin)
{
    ajuint i;
    ajuint j;
    ajuint jj;
    ajuint k = 0;
    const char *p;
    const char *q;
    AjBool flag;
    ajuint count;

    if(left && start)
	return 0;

    p = ajStrGetPtr(str);
    q = ajStrGetPtr(pat);

    flag = ajTrue;
    count = 0;

    i = start+(plen-1);
    jj = plen;
    j = jj-1;

    while(flag)
    {
	while(jj>0 && i<slen)
	{
	    k = i;
	    while(jj>0 && p[k]==q[j])
	    {
		--k;
		--j;
		--jj;
	    }

	    if(jj>0)
	    {
		i += skip[(ajuint)p[i]];
		jj = plen;
		j = jj-1;
	    }
	}

	if(jj == 0)
	{
	    if(left && k+1)
		return 0;

	    if(!right || (right && k+1+plen==slen))
	    {
		++count;
		embPatPushHit(l,name,k+1,plen,begin,0);
	    }
	    i = start+(plen-1)+k+2;
	    jj = plen;
	    j = jj-1;

	}
	else
	    flag=ajFalse;
    }

    return count;
}



/* @func embPatBYPInit ********************************************************
**
** Initialise a Baeza-Yates,Perleberg pattern.
**
** @param [r] pat [const AjPStr] pattern
** @param [r] len [ajuint] pattern length
** @param [w] offset [EmbPPatBYPNode] character index
** @param [w] buf [ajint *] mismatch count
**
** @return [void]
******************************************************************************/

void embPatBYPInit(const AjPStr pat, ajuint len, EmbPPatBYPNode offset,
		   ajint *buf)
{
    ajuint i;
    ajuint j;

    const char *p;
    EmbPPatBYPNode op;

    p = ajStrGetPtr(pat);

    for(i=0;i<AJALPHA;++i)
    {
	offset[i].offset = -1;
	offset[i].next   = NULL;
	buf[i] = len;
    }

    for(i=0,j=AJALPHA>>1;i<len;++i,++p)
    {
	buf[i] = AJALPHA;
	if(offset[(ajuint)*p].offset == -1)
	    offset[(ajuint)*p].offset = len-i-1;
	else
	{
	    op=offset[(ajuint)*p].next;
	    offset[(ajuint)*p].next=&offset[j++];
	    offset[(ajuint)*p].next->offset = len-i-1;
	    offset[(ajuint)*p].next->next = op;
	}
    }

    return;
}




/* @func embPatPushHit ********************************************************
**
** Put a matching BYP search hit on the heap
** as an EmbPMatMatch structure
**
** @param [u] l [AjPList] list to push to
** @param [r] name [const AjPStr] string name
** @param [r] pos [ajuint] Sequence match position
** @param [r] plen [ajuint] pattern length
** @param [r] begin [ajuint] Sequence offset
** @param [r] mm [ajuint] number of mismatches
**
** @return [void]
******************************************************************************/

void embPatPushHit(AjPList l, const AjPStr name, ajuint pos, ajuint plen,
		   ajuint begin, ajuint mm)
{
    EmbPMatMatch hit;

    AJNEW0(hit);

    hit->seqname = ajStrNewS(name);
    hit->len = plen;
    hit->cod = ajStrNew();
    hit->pat = ajStrNew();
    hit->acc = ajStrNew();
    hit->tit = ajStrNew();
    hit->start = pos+begin;
    hit->mm = mm;
    hit->end = pos+begin+plen-1;
    ajListPush(l,(void *) hit);

    return;
}




/* @func embPatBYPSearch ******************************************************
**
** Perform a Baeza-Yates,Perleberg search.
**
** @param [r] str [const AjPStr] search string
** @param [r] name [const AjPStr] search string
** @param [r] begin [ajuint] sequence offset
** @param [r] slen [ajuint] string length
** @param [r] plen [ajuint] pattern length
** @param [r] mm [ajuint] allowed mismatches (Hamming distance)
** @param [u] offset [EmbPPatBYPNode] character index
** @param [u] buf [ajint *] mismatch count array
** @param [u] l [AjPList] list to push hits to
** @param [r] amino [AjBool] if true, match at amino terminal end
** @param [r] carboxyl [AjBool] if true, match at carboxyl terminal end
** @param [r] pat [const AjPStr] original pattern
**
** @return [ajuint] number of matches
******************************************************************************/

ajuint embPatBYPSearch(const AjPStr str, const AjPStr name,
		      ajuint begin, ajuint slen,
		      ajuint plen, ajuint mm, EmbPPatBYPNode offset,
		      ajint *buf,
		      AjPList l, AjBool amino, AjBool carboxyl,
		      const AjPStr pat)
{
    const char *p;
    const char *q;
    ajuint  i;
    ajint  t;
    EmbPPatBYPNode off;
    ajint count;
    AjPStr pattern;

    p = ajStrGetPtr(str);
    pattern = ajStrNewS(pat);
    ajStrFmtUpper(&pattern);
    q = ajStrGetPtr(pattern);

    count = mm;
    for(i=0;i<plen;++i)
	if(*q++!=*p++)
	    if(--count<0)
		break;

    if(count>=0)
    {
	embPatPushHit(l,name,0,plen,begin,mm-count);
	count = 1;
    }
    else
	count = 0;

    p = ajStrGetPtr(str);

    for(i=0;i<slen;++i)
    {
	if((t=(off=&offset[(ajuint)*p++])->offset)>=0)
	{
	    buf[(i+t)&AJMOD256]--;
	    for(off=off->next;off!=NULL;off=off->next)
		buf[(i+off->offset)&AJMOD256]--;
	}

	if(buf[i&AJMOD256]<=(ajint)mm)
	{
	    if(amino && i-plen+1!=0)
		return count;

	    if(!carboxyl || (carboxyl && i+1==slen))
	    {
		++count;
		embPatPushHit(l,name,i-plen+1,plen,begin,buf[i&AJMOD256]);
	    }
	}

	buf[i&AJMOD256] = plen;
    }

    ajStrDel(&pattern);
    return count;
}




/* @funcstatic patAminoCarboxyl ***********************************************
**
** Checks for start and/or end angle bracket markers
** Removes them from the string and sets bools accordingly
**
** @param [r] s [const AjPStr] original pattern
** @param [u] cs [AjPStr *] modified pattern
** @param [w] amino [AjBool *] set if start marker (left angle bracket)
** @param [w] carboxyl [AjBool *] set if end marker (right angle bracket)
**
** @return [void]
******************************************************************************/

static void patAminoCarboxyl(const AjPStr s, AjPStr *cs,
			     AjBool *amino, AjBool *carboxyl)
{
    AjPStr t;
    const char *p;

    t = ajStrNewC("");
    p = ajStrGetPtr(s);

    while(*p)
    {
	if(*p==' ' || *p=='-' || *p=='.')
	{
	    ++p;
	    continue;
	}

	if(*p=='<')
	{
	    *amino = ajTrue;
	    ++p;
	    continue;
	}

	if(*p=='>')
	{
	    *carboxyl = ajTrue;
	    ++p;
	    continue;
	}

	ajStrAppendK(&t,*p);
	++p;
    }
    ajStrAssignS(cs,t);
    ajStrDel(&t);

    return;
}




/* @funcstatic patParenTest ***************************************************
**
** Checks parenthesis grammar. Sets repeat and range bools
**
** @param [r] p [const char *] pattern
** @param [w] repeat [AjBool *] set if any parenthesis e.g. (3)
** @param [w] range [AjBool *] set if range e.g. (5,8)
**
** @return [AjBool] True if grammar correct
******************************************************************************/

static AjBool patParenTest(const char *p, AjBool *repeat, AjBool *range)
{
    ajuint i;

    *repeat = ajTrue;
    p = p+2;
    if(sscanf(p,"%d",&i)!=1)
    {
	ajWarn("Illegal pattern. Missing repeat number");
	return ajFalse;
    }

    while(*p)
    {
	if(*p==')')
	    break;

	if(*p=='('||*p=='['||*p=='{'||*p=='}'||*p==']' ||
	   isalpha((ajuint)*p))
	{
	    ajWarn("Illegal pattern. Nesting not allowed");
	    return ajFalse;
	}

	if(*p==',')
	{
	    *range = ajTrue;
	    ++p;
	    if(sscanf(p,"%d",&i)!=1)
	    {
		ajWarn("Illegal pattern. Missing range number");
		return ajFalse;
	    }
	    continue;
	}
	++p;
    }

    if(!*p)
    {
	ajWarn("Illegal pattern. Missing parenthesis");
	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic patExpandRepeat ************************************************
**
** Expand repeats e.g. [ABC](2) to [ABC][ABC]
**
** @param [u] s [AjPStr *] pattern
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool patExpandRepeat(AjPStr *s)
{
    AjPStr t;
    const char *p;
    const char *q;
    ajuint count;
    ajuint i;

    t = ajStrNewC("");
    p = ajStrGetPtr(*s);

    while(*p)
    {
	if(*p=='[' || *p=='{')
	{
	    q = p;
	    while(!(*p==']' || *p=='}'))
		++p;

	    if(*(p+1)!='(')
		count = 1;
	    else
		sscanf(p+2,"%d",&count);

	    if(count<=0)
	    {
		ajWarn("Illegal pattern. Bad repeat count");
		return ajFalse;
	    }

	    for(i=0;i<count;++i)
	    {
		p = q;
		while(!(*p==']'||*p=='}'))
		{
		    ajStrAppendK(&t,*p);
		    ++p;
		}
		ajStrAppendK(&t,*p);
	    }

	    if(*(p+1)=='(')
		while(*p!=')')
		    ++p;

	    ++p;
	    continue;
	}

	if(*p=='(')
	{
	    sscanf(p+1,"%d",&count);
	    if(count<=0)
	    {
		ajWarn("Illegal pattern. Bad range number");
		return ajFalse;
	    }

	    for(i=1;i<count;++i)
		ajStrAppendK(&t,*(p-1));

	    while(*p!=')')
		++p;
	    ++p;
	    continue;
	}

	ajStrAppendK(&t,*p);
	++p;
    }

    ajStrAssignC(s,ajStrGetPtr(t));
    ajStrDel(&t);

    return ajTrue;
}




/* @funcstatic patIUBTranslate ************************************************
**
** Convert IUB nucleotide symbols to classes e.g. S to [GC]
**
** @param [u] pat [AjPStr *] pattern
**
** @return [void]
******************************************************************************/

static void patIUBTranslate(AjPStr *pat)
{
    AjPStr t;
    const char *p;

    t = ajStrNewC(ajStrGetPtr(*pat));
    p = ajStrGetPtr(t);
    ajStrSetClear(pat);

    while(*p)
    {
	if(*p=='B')
	{
	    ajStrAppendC(pat,"[TGC]");
	    ++p;
	    continue;
	}

	if(*p=='D')
	{
	    ajStrAppendC(pat,"[TGA]");
	    ++p;
	    continue;
	}

	if(*p=='H')
	{
	    ajStrAppendC(pat,"[TCA]");
	    ++p;
	    continue;
	}

	if(*p=='K')
	{
	    ajStrAppendC(pat,"[TG]");
	    ++p;
	    continue;
	}

	if(*p=='M')
	{
	    ajStrAppendC(pat,"[CA]");
	    ++p;
	    continue;
	}

	if(*p=='R')
	{
	    ajStrAppendC(pat,"[GA]");
	    ++p;
	    continue;
	}

	if(*p=='S')
	{
	    ajStrAppendC(pat,"[GC]");
	    ++p;
	    continue;
	}

	if(*p=='V')
	{
	    ajStrAppendC(pat,"[GCA]");
	    ++p;
	    continue;
	}

	if(*p=='W')
	{
	    ajStrAppendC(pat,"[TA]");
	    ++p;
	    continue;
	}

	if(*p=='Y')
	{
	    ajStrAppendC(pat,"[TC]");
	    ++p;
	    continue;
	}

	ajStrAppendK(pat,*p);
	++p;
    }

    ajStrDel(&t);

    return;
}




/* @funcstatic patProteinTranslate ********************************************
**
** Convert protein symbols to classes e.g. B to [DN]
**
** @param [u] pat [AjPStr *] pattern
**
** @return [void]
******************************************************************************/

static void patProteinTranslate(AjPStr *pat)
{
    AjPStr t;
    const char *p;

    t = ajStrNewC(ajStrGetPtr(*pat));
    p = ajStrGetPtr(t);
    ajStrSetClear(pat);

    while(*p)
    {
	if(*p=='B')
	{
	    ajStrAppendC(pat,"[DN]");
	    ++p;
	    continue;
	}

	if(*p=='Z')
	{
	    ajStrAppendC(pat,"[EQ]");
	    ++p;
	    continue;
	}

	if(*p=='J')
	{
	    ajStrAppendC(pat,"[IL]");
	    ++p;
	    continue;
	}


	ajStrAppendK(pat,*p);
	++p;
    }

    ajStrDel(&t);

    return;
}




/* @func embPatClassify *******************************************************
**
** Classify patterns according to type. The pattern is set up upper case,
** has start and end processing turned into boolean flags. Sets other boolean
** flags for properties of the pattern so that a suitable processing
** method can be selected.
**
** @param [r] pat      [const AjPStr] original pattern
** @param [w] cleanpat [AjPStr *] cleaned pattern
** @param [w] amino    [AjBool*] set if must match start of sequence
** @param [w] carboxyl [AjBool*] set if must match end of sequence
** @param [w] fclass   [AjBool*] set if class e.g. [ABC]
** @param [w] ajcompl  [AjBool*] set if complement e.g. {ABC}
** @param [w] dontcare [AjBool*] set if X (protein) or N (DNA)
** @param [w] range    [AjBool*] set if range specified e.g. (3,10)
** @param [r] protein  [AjBool] true if protein false if DNA
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool embPatClassify(const AjPStr pat, AjPStr *cleanpat,
		      AjBool *amino, AjBool *carboxyl,
		      AjBool *fclass, AjBool *ajcompl, AjBool *dontcare,
		      AjBool *range, AjBool protein)
{
    char *p;
    AjBool repeat;
    AjPStr tmppat = NULL;

    ajDebug("embPatClassify pat '%S' protein %b\n", pat, protein);

    repeat=*amino=*carboxyl=*fclass=*ajcompl=*dontcare=*range=ajFalse;

    ajStrAssignS(&tmppat, pat);
    ajStrRemoveWhiteExcess(&tmppat);
    patAminoCarboxyl(tmppat,cleanpat, amino,carboxyl);
    ajStrDel(&tmppat);

    ajDebug("cleaned pat '%S' amino %b carboxyl %b\n",
	    *cleanpat, *amino, *carboxyl);

    ajStrFmtUpper(cleanpat);

    if(!protein)
    {
	patIUBTranslate(cleanpat);
	ajDebug("IUB translated pat '%S'\n", *cleanpat);
    }
    else
    {
	patProteinTranslate(cleanpat);
	ajDebug("Protein codes translated pat '%S'\n", *cleanpat);
    }

    p = ajStrGetuniquePtr(cleanpat);

    while(*p)
    {
	if(isalpha((ajuint)*p))
	{
	    if((*p=='X' && protein) || (*p=='N' && !protein))
	    {
		*p='?';
		*dontcare = ajTrue;
	    }

	    if(*(p+1)=='(')
	    {
		if(!patParenTest(p,&repeat,range))
		    return ajFalse;

		while(*p!=')')
		    ++p;
	    }

	    ++p;
	    continue;
	}



	if(*p=='[')
	{
	    *fclass = ajTrue;
	    ++p;
	    while(*p)
	    {
		if(*p==']')
		    break;

		if(*p=='('||*p=='['||*p=='{'||*p=='}'||*p==')')
		{
		    ajWarn("Illegal pattern. Nesting '%c' in [] not allowed",
			   *p);
		    return ajFalse;
		}

		if(!isalpha((ajuint)*p))
		{
		    ajWarn("Illegal pattern. Non alpha character '%c'",
			   *p);
		    return ajFalse;
		}

		if((protein&&*p=='X')||(!protein&&*p=='N'))
		{
		    ajWarn("Illegal pattern. Dontcare character '%c' in []",
			   *p);
		    return ajFalse;
		}

		++p;
	    }

	    if(!*p)
	    {
		ajWarn("Illegal pattern. Missing ']'");
		return ajFalse;
	    }

	    if(*(p+1)=='(')
	    {
		if(!patParenTest(p,&repeat,range))
		    return ajFalse;

		while(*p!=')')
		    ++p;
	    }

	    ++p;
	    continue;
	}

	if(*p=='{')
	{
	    *ajcompl = ajTrue;
	    ++p;
	    while(*p)
	    {
		if(*p=='}')
		    break;

		if(*p=='('||*p=='['||*p=='{'||*p==']'||*p==')')
		{
		    ajWarn("Illegal pattern. Nesting '%c' in {} not allowed.",
			   *p);
		    return ajFalse;
		}

		if(!isalpha((ajuint)*p))
		{
		    ajWarn("Illegal pattern. Non alpha character '%c'", *p);
		    return ajFalse;
		}

		if((protein&&*p=='X')||(!protein&&*p=='N'))
		{
		    ajWarn("Illegal pattern. Ambiguous character '%c' in {}",
			   *p);
		    return ajFalse;
		}

		++p;
	    }

	    if(!*p)
	    {
		ajWarn("Illegal pattern. Missing '}'");
		return ajFalse;
	    }

	    if(*(p+1)=='(')
	    {
		if(!patParenTest(p,&repeat,range))
		    return ajFalse;

		while(*p!=')')
		    ++p;
	    }

	    ++p;
	    continue;
	}

	ajWarn("Illegal character '%c'",*p);
	return ajFalse;
    }

    ajDebug("patterns expanded pat '%S'\n", *cleanpat);

    if(repeat && !*range)
    {
	ajDebug("testing repeat expansion\n");
	if(!patExpandRepeat(cleanpat))
	    return ajFalse;

	ajDebug("repeats expanded pat '%S'\n", *cleanpat);
    }

    return ajTrue;
}




/* @func embPatSOInit *********************************************************
**
** Initialise a Shift-Or pattern.
**
** @param [r] pat [const AjPStr] pattern
** @param [w] table [ajuint *] SO table
** @param [w] limit [ajuint *] match limit
**
** @return [void]
******************************************************************************/

void embPatSOInit(const AjPStr pat, ajuint *table, ajuint *limit)
{
    ajuint  i;
    const char *p;

    if(ajStrGetLen(pat)>AJWORD)
	ajFatal("Pattern too ajlong for Shift-OR search");


    for(i=0;i<AJALPHA2;++i)
	table[i] = ~0;

    *limit = 0;
    for(i=1,p=ajStrGetPtr(pat);*p;i<<=AJBPS,++p)
    {
	table[(ajuint)*p] &= ~i;
	*limit |=  i;
    }

    *limit = ~(*limit>>AJBPS);

    return;
}




/* @func embPatSOSearch *******************************************************
**
** Perform a Shift-OR search.
**
** @param [r] str [const AjPStr] search string
** @param [r] name [const AjPStr] search string
** @param [r] first [ajint] first character of pattern (as an integer)
** @param [r] begin [ajuint] sequence offset
** @param [r] plen [ajuint] pattern length
** @param [r] table [const ajuint *] SO table
** @param [r] limit [ajuint] SO limit
** @param [u] l [AjPList] list to push hits to
** @param [r] amino [AjBool] must match start
** @param [r] carboxyl [AjBool] must match end
**
** @return [ajuint] number of matches
******************************************************************************/

ajuint embPatSOSearch(const AjPStr str, const AjPStr name,
		     ajint first, ajuint begin,
		     ajuint plen, const ajuint *table, ajuint limit, AjPList l,
		     AjBool amino, AjBool carboxyl)
{
    register ajuint state;
    register ajuint initial;
    const char *p;
    const char *q;
    ajuint pos;

    ajuint matches;
    ajuint slen;

    p = q = ajStrGetPtr(str);
    slen  = ajStrGetLen(str);
    matches = 0;
    initial = ~0;

    do
    {
	while(*p && *p != first)
	    ++p;

	state = initial;
	do
	{
	    state = (state<<AJBPS) | table[(ajuint)*p];
	    if(state < limit)
	    {
		pos = (ajuint) ((p-q)-plen+1);
		if(amino && pos)
		    return matches;

		if(!carboxyl || (carboxyl && pos==slen-plen))
		{
		    ++matches;
		    embPatPushHit(l,name,pos,plen,begin,0);
		}
	    }
	    ++p;
	}
	while(state!=initial);
    }
    while(*(p-1));

    return matches;
}




/* @func embPatBYGCInit *******************************************************
**
** Initialise a Baeza-Yates Gonnet class pattern.
**
** @param [r] pat [const AjPStr] pattern
** @param [w] m [ajuint *] real pattern length
** @param [w] table [ajuint *] SO table
** @param [w] limit [ajuint *] match limit
**
** @return [void]
******************************************************************************/

void embPatBYGCInit(const AjPStr pat, ajuint *m, ajuint *table,
		    ajuint *limit)
{
    const char *p;
    const char *q;
    ajuint initval;
    ajuint shift;
    ajuint i;

    p = q = ajStrGetPtr(pat);
    initval = ~0;
    shift   = 1;
    *m = 0;
    *limit  = 0;

    while(*p)
    {
	if(*p=='?')
	    initval &= ~shift;
	else if(*p=='{')
	{
	    initval &= ~shift;
	    while(*p!='}')
		++p;
	}
	else if(*p=='[')
	    while(*p!=']')
		++p;
	++p;
	++*m;
	*limit |= shift;
	shift<<=AJBPS;
    }

    for(i=0;i<AJALPHA2;++i)
	table[i] = initval;

    p = q;

    shift = 1;
    while(*p)
    {
	if(*p=='{')
	{
	    ++p;
	    while(*p!='}')
	    {
		table[(ajuint)*p] |= shift;
		++p;
	    }
	}
	else if(*p=='[')
	{
	    ++p;
	    while(*p!=']')
	    {
		table[(ajuint)*p] &= ~shift;
		++p;
	    }
	}
	else if(*p!='?')
	    table[(ajuint)*p] &= ~shift;

	shift <<= AJBPS;
	++p;
    }

    *limit = ~(*limit>>AJBPS);

    return;
}




/* @func embPatBYGSearch ******************************************************
**
** Perform a Baeza-Yates Gonnet search.
**
** @param [r] str [const AjPStr] search string
** @param [r] name [const AjPStr] search string
** @param [r] begin [ajuint] sequence offset
** @param [r] plen [ajuint] pattern length
** @param [r] table [const ajuint *] SO table
** @param [r] limit [ajuint] SO limit
** @param [u] l [AjPList] list to push hits to
** @param [r] amino [AjBool] must match start
** @param [r] carboxyl [AjBool] must match end
**
** @return [ajuint] number of matches
******************************************************************************/

ajuint embPatBYGSearch(const AjPStr str, const AjPStr name,
		      ajuint begin, ajuint plen,
		      const ajuint *table, ajuint limit, AjPList l,
		      AjBool amino, AjBool carboxyl)
{
    register ajuint state;
    register ajuint initial;
    const char *p;
    const char *q;
    ajuint pos;

    ajuint matches;
    ajuint slen;

    p = q = ajStrGetPtr(str);
    slen  = ajStrGetLen(str);
    matches = 0;
    initial = ~0;

    ajDebug("..pat initial %lx\n", initial);
    ajDebug("..pat strlen:%d str:'%S'\n", slen, str);

    do
    {
	state = initial;
	do
	{
	    state = (state<<AJBPS) | table[(ajuint)*p];
	    /* ajDebug("..pat table: %lx state %lx p:%c\n",
		    table[(ajuint)*p], state, *p); */
	    if(state < limit)
	    {
		pos = (ajuint) ((p-q)-plen+1);
		if(amino && pos)
		    return matches;

		if(!carboxyl || (carboxyl && pos==slen-plen))
		{
		    ++matches;
		    embPatPushHit(l,name,pos,plen,begin,0);
		  /* ajDebug("..pat hit matches:%d list:%d name:'%S' pos:%d\n",
			    matches, ajListGetLength(l), name, pos); */
		}
	    }
	    ++p;
	}
	while(state!=initial && *p);
    }
    while((ajuint) (p-q)<slen);

    return matches;
}




/* @func embPatTUInit *********************************************************
**
** Initialise a Tarhio-Ukkonen search
**
** @param [r] pat [const AjPStr] pattern
** @param [w] skipm [ajuint **] mismatch skip array
** @param [r] m [ajuint] real pattern length
** @param [r] k [ajuint] allowed mismatches
**
** @return [void]
******************************************************************************/

void embPatTUInit(const AjPStr pat, ajuint **skipm, ajuint m, ajuint k)
{
    const char *p;
    ajuint i;
    ajint j;
    ajint jj;
    ajuint x;

    ajuint ready[AJALPHA];

    p = ajStrGetPtr(pat);

    for(i=0;i<AJALPHA;++i)
    {
	ready[i] = m;
	for(j=m-k-1;j<(ajint)m;++j)
	    skipm[j][i] = m-k-1;
    }

    for(j=m-2;j>-1;--j)
    {
	jj = m-k-1;
	x = AJMAX(j+1,jj);
	for(i=ready[(ajuint)p[j]]-1;i>=x;--i)
	    skipm[i][(ajuint)p[j]] = i-j;
	ready[(ajuint)p[j]] = x;
    }

    return;
}




/* @func embPatTUSearch *******************************************************
**
** Perform a Tarhio-Ukkonen search
**
** @param [r] pat [const AjPStr] pattern
** @param [r] text [const AjPStr] text to search (incl ajcompl/class)
** @param [r] slen [ajuint] length of text
** @param [r] skipm [ajuint * const *] mismatch skip array
** @param [r] m [ajuint] real pattern length
** @param [r] k [ajuint] allowed mismatches
** @param [r] begin [ajuint] text offset
** @param [u] l [AjPList] list to push to
** @param [r] amino [AjBool] true if text start
** @param [r] carboxyl [AjBool] true if text end
** @param [r] name [const AjPStr] name of text
**
** @return [ajuint] number of hits
******************************************************************************/

ajuint embPatTUSearch(const AjPStr pat, const AjPStr text, ajuint slen,
		     ajuint * const *skipm, ajuint m,
		     ajuint k, ajuint begin, AjPList l, AjBool amino,
		     AjBool carboxyl, const AjPStr name)
{
    ajuint i;
    ajint j;
    ajint jj;
    ajuint h;
    ajuint mm;
    ajuint skip;
    const char *p;
    const char *q;
    ajuint  matches;

    p = ajStrGetPtr(pat);
    q = ajStrGetPtr(text);

    matches = 0;

    i = m-1;
    while(i<slen)
    {
	h = i;
	j = m-1;
	skip = m-k;
	mm = 0;
	while(j>-1 && mm<=k)
	{
	    jj = m-k-1;
	    if(j>=jj)
		skip = AJMIN(skip,(ajuint)skipm[j][(ajuint)q[h]]);

	    if(q[h]!=p[j])
		++mm;
	    --h;
	    --j;
	}

	if(mm<=k)
	{
	    if(amino && h+1)
		return matches;

	    if(!carboxyl || (carboxyl && h+1==slen-m))
	    {
		++matches;
		embPatPushHit(l,name,h+1,m,begin,mm);
	    }
	}
	i+=skip;
    }

    return matches;
}




/* @func embPatTUBInit ********************************************************
**
** Initialise a Tarhio-Ukkonen-Bleasby search
**
** @param [r] pat [const AjPStr] pattern
** @param [w] skipm [ajuint **] mismatch skip array
** @param [r] m [ajuint] real pattern length
** @param [r] k [ajuint] allowed mismatches
** @param [r] plen [ajuint] full pattern length (incl ajcompl & class)
**
** @return [void]
******************************************************************************/

void embPatTUBInit(const AjPStr pat, ajuint **skipm, ajuint m, ajuint k,
		   ajuint plen)
{
    const char *p;
    const char *q;
    const char *s;
    ajint i;
    ajint j;
    ajint jj;
    ajint x;
    ajint z;
    ajuint flag;
    ajuint ready[AJALPHA];

    p = ajStrGetPtr(pat);

    for(i=0;i<AJALPHA;++i)
    {
	ready[i] = m;
	for(j=m-k-1;j<(ajint)m;++j)
	    skipm[j][i] = m-k-1;
    }

    p += plen-1;
    if(*p=='}' || *p==']')
    {
	while(*p!='{' && *p!='[')
	    --p;
	--p;
    }
    else
	--p;

    for(j=m-2;j>-1;--j)
    {
	jj = m-k-1;
	x = AJMAX(j+1,jj);
	if(*p=='?')
	{
	    for(z='A';z<='Z';++z)
	    {
		for(i=ready[z]-1;i>=x;--i)
		    skipm[i][z] = i-j;
		ready[z] = x;
	    }
	    --p;
	    continue;
	}

	if(*p==']')
	{
	    --p;
	    while(*p!='[')
	    {
		for(i=ready[(ajuint)*p]-1;i>=x;--i)
		    skipm[i][(ajuint)*p] = i-j;
		ready[(ajuint)*p] = x;
		--p;
	    }
	    --p;
	    continue;
	}

	if(*p=='}')
	{
	    s=--p;
	    for(z='A';z<='Z';++z)
	    {
		q    = s;
		flag = 0;
		while(*q!='{')
		{
		    if(*q==z)
		    {
			flag = 1;
			break;
		    }
		    --q;
		}

		if(!flag)
		{
		    for(i=ready[z]-1;i>=x;--i)
			skipm[i][z] = i-j;
		    ready[z] = x;
		}
	    }
	    while(*p!='{')
		--p;
	    --p;
	    continue;
	}

	for(i=ready[(ajuint)*p]-1;i>=x;--i)
	    skipm[i][(ajuint)*p] = i-j;
	ready[(ajuint)*p] = x;
	--p;
    }

    return;
}




/* @func embPatTUBSearch ******************************************************
**
** Perform a Tarhio-Ukkonen-Bleasby search
**
** @param [r] pat [const AjPStr] pattern
** @param [r] text [const AjPStr] text to search (incl ajcompl/class)
** @param [r] slen [ajuint] length of text
** @param [r] skipm [ajuint * const *] mismatch skip array
** @param [r] m [ajuint] real pattern length
** @param [r] k [ajuint] allowed mismatches
** @param [r] begin [ajuint] text offset
** @param [u] l [AjPList] list to push to
** @param [r] amino [AjBool] true if text start
** @param [r] carboxyl [AjBool] true if text end
** @param [r] name [const AjPStr] name of text
** @param [r] plen [ajuint] total pattern length
**
** @return [ajuint] number of hits
******************************************************************************/

ajuint embPatTUBSearch(const AjPStr pat,const AjPStr text, ajuint slen,
		      ajuint * const *skipm, ajuint m,
		      ajuint k, ajuint begin, AjPList l, AjBool amino,
		      AjBool carboxyl, const AjPStr name, ajuint plen)
{
    ajuint i;
    ajint j;
    ajint jj;
    ajuint h;
    ajuint mm;
    ajuint skip;
    const char *p;
    const char *q;
    const char *s;

    ajuint  matches;
    ajint  a;
    ajuint  flag;

    s = ajStrGetPtr(pat);
    q = ajStrGetPtr(text);

    matches = 0;

    i = m-1;
    while(i<slen)
    {
	h = i;
	j = m-1;
	p = s+plen-1;
	skip = m-k;
	mm = 0;
	while(j>-1 && mm<=k)
	{
	    jj = m-k-1;
	    if(j>=jj)
		skip = AJMIN(skip,skipm[j][(ajuint)q[h]]);
	    a = q[h];
	    if(*p!='?')
	    {
		if(*p==']')
		{
		    flag = 0;
		    --p;
		    while(*p!='[')
		    {
			if(a==*p)
			    flag = 1;
			--p;
		    }

		    if(!flag)
			++mm;
		}
		else if(*p=='}')
		{
		    flag = 0;
		    --p;
		    while(*p!='{')
		    {
			if(a==*p)
			    flag=1;
			--p;
		    }

		    if(flag)
			++mm;
		}
		else
		    if(a!=*p)
			++mm;
	    }

	    --p;
	    --h;
	    --j;
	}

	if(mm<=k)
	{
	    if(amino && h+1)
		return matches;

	    if(!carboxyl || (carboxyl && h+1==slen-m))
	    {
		++matches;
		embPatPushHit(l,name,h+1,m,begin,mm);
	    }
	}

	i += skip;
    }

    return matches;
}




/* @funcstatic patBruteClass **************************************************
**
** Test if character matches one of a class
**
** @param [r] p [const char *] pattern class e.g. [abc]
** @param [r] c [char] character to match
**
** @return [AjBool] true if match success
******************************************************************************/

static AjBool patBruteClass(const char *p, char c)
{
    const char *s;

    s = p+1;
    while(*s!=']')
	if(*s++==c)
	    return ajTrue;

    return ajFalse;
}




/* @funcstatic patBruteCompl **************************************************
**
** Test if character matches one of a complement
**
** @param [r] p [const char *] pattern complement e.g. {abc}
** @param [r] c [char] character to match
**
** @return [AjBool] true if character not in complement
******************************************************************************/

static AjBool patBruteCompl(const char *p, char c)
{
    const char *s;

    s = p+1;
    while(*s!='}')
	if(*s++==c)
	    return ajFalse;

    return ajTrue;
}




/* @funcstatic patBruteIsRange ************************************************
**
** Check if character, class or complement has a range e.g. [abc](3,6)
**
** @param [r] t [const char *] pattern
** @param [w] x [ajuint *] first range value
** @param [w] y [ajuint *] second range value
**
** @return [AjBool] true if there is a range & sets x/y accordingly
******************************************************************************/

static AjBool patBruteIsRange(const char *t, ajuint *x, ajuint *y)
{
    const char *u;

    if((*t >='A' && *t<='Z') || *t=='?')
    {
	if(*(t+1)=='(')
	{
	    if(sscanf(t+2,"%d,%d",x,y)!=2)
	    {
		sscanf(t+2,"%d",x);
		*y=*x;
	    }

	    return ajTrue;
	}

	return ajFalse;
    }

    u = t;

    if(*t=='[')
    {
	while(*u!=']')
	    ++u;

	if(*(u+1)=='(')
	{
	    if(sscanf(u+2,"%d,%d",x,y)!=2)
	    {
		sscanf(u+2,"%d",x);
		*y = *x;
	    }

	    return ajTrue;
	}
	return ajFalse;
    }

    while(*u!='}')
	++u;

    if(*(u+1)=='(')
    {
	if(sscanf(u+2,"%d,%d",x,y)!=2)
	{
	    sscanf(u+2,"%d",x);
	    *y = *x;
	}

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic patBruteCharMatch **********************************************
**
** Check if text character matches a char/class or complement
**
** @param [r] t [const char *] text
** @param [r] c [char] character to match
**
** @return [AjBool] true if match
******************************************************************************/

static AjBool patBruteCharMatch(const char *t, char c)
{
    if(!c)
	return ajFalse;

    if(*t=='?')
	return ajTrue;

    if(*t>='A' && *t<='Z')
    {
	if(*t==c)
	    return ajTrue;

	return ajFalse;
    }

    if(*t=='[')
    {
	if(patBruteClass(t,c))
	    return ajTrue;

	return ajFalse;
    }

    if(patBruteCompl(t,c))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic patBruteNextPatChar ********************************************
**
** Get index of next char/class/complement
**
** @param [r] t [const char *] text
** @param [r] ppos [ajuint] current index
**
** @return [ajuint] next index
******************************************************************************/

static ajuint patBruteNextPatChar(const char *t, ajuint ppos)
{
    if(t[ppos]=='{')
	while(t[ppos]!='}')
	    ++ppos;

    if(t[ppos]=='[')
	while(t[ppos]!=']')
	    ++ppos;
    ++ppos;

    if(t[ppos]=='(')
    {
	while(t[ppos]!=')')
	    ++ppos;
	++ppos;
    }

    return ppos;
}




/* @funcstatic patOUBrute *****************************************************
**
** Match pattern to current sequence position
**
** @param [r] seq [const char *] text
** @param [r] pat [const char *] pattern
** @param [r] spos [ajuint] sequence index
** @param [r] ppos [ajuint] pattern index
** @param [r] mm [ajuint] mismatches left
** @param [r] omm [ajuint] allowed mismatches
** @param [r] level [ajuint] level of recursion
** @param [u] l [AjPList] list on which to push hits
** @param [r] carboxyl [AjBool] true if pattern must only match end of text
** @param [r] begin [ajuint] text offset
** @param [w] count [ajuint *] hit counter
** @param [r] name [const AjPStr] text entry name
** @param [r] st [ajuint] original text index
**
** @return [AjBool] true if hit found (Care! Function recursive.)
******************************************************************************/

static AjBool patOUBrute(const char *seq, const char *pat, ajuint spos,
			 ajuint ppos, ajuint mm,
			 ajuint omm, ajuint level, AjPList l, AjBool carboxyl,
			 ajuint begin, ajuint *count,
			 const AjPStr name, ajuint st)
{
    const char *t;
    ajuint x;
    ajuint y;
    ajuint i;

    if(level==1000)
	ajFatal("Pattern too complex: 1000 levels of recursion");


    while(pat[ppos])
    {
	t = pat+ppos;
	if(!seq[spos])
	    return ajFalse;

	if(!patBruteIsRange(t,&x,&y))
	{
	    if(!patBruteCharMatch(t,seq[spos]))
	    {
		if(mm==0)
		    return ajFalse;
		--mm;
	    }

	    ppos = patBruteNextPatChar(pat,ppos);
	    ++spos;
	    continue;
	}

	for(i=0;i<x;++i)
	{
	    if(!patBruteCharMatch(t,seq[spos++]))
	    {
		if(mm==0)
		    return ajFalse;
		--mm;
	    }
	    if(!seq[spos-1])
		return ajFalse;
	}

	ppos=patBruteNextPatChar(pat,ppos);
	for(i=0;i<y-x;++i)
	{
	    patOUBrute(seq,pat,spos,ppos,mm,omm,level,l,carboxyl,
		       begin,count,name,st);
	    if(!patBruteCharMatch(t,seq[spos]))
		return ajFalse;

	    ++spos;
	}
    }

    if(!carboxyl || (carboxyl && !seq[spos]))
    {
	*count += 1;
	embPatPushHit(l,name,st,spos-st,begin,omm-mm);

    }

    return ajTrue;
}




/* @func embPatBruteForce *****************************************************
**
** Match pattern to a sequence
**
** @param [r] seq [const AjPStr] text
** @param [r] pat [const AjPStr] pattern
** @param [r] amino [AjBool] true if must match start
** @param [r] carboxyl [AjBool] true if must match end
** @param [u] l [AjPList] list on which to push hits
** @param [r] begin [ajuint] text offset
** @param [r] mm [ajuint] allowed mismatches
** @param [r] name [const AjPStr] text entry name
**
** @return [ajuint] number of hits
******************************************************************************/

ajuint embPatBruteForce(const AjPStr seq, const AjPStr pat,
		       AjBool amino, AjBool carboxyl,
		       AjPList l, ajuint begin, ajuint mm, const AjPStr name)
{
    const char *s;
    const char *p;
    ajuint  sum;
    ajuint  len;
    ajuint  i;
    ajuint  count;

    ajDebug("embPatBruteForce amino:%B carboxyl:%B begin:%u mm:%u len:%u\n",
	    amino, carboxyl, begin, mm, ajStrGetLen(seq));

    sum=count = 0;
    s = ajStrGetPtr(seq);
    p = ajStrGetPtr(pat);

    if(amino)
    {
	patOUBrute(s,p,0,0,mm,mm,1,l,carboxyl,begin,&count,name,0);
	return count;
    }

    len = (ajuint) strlen(s);
    for(i=0;i<len;++i)
    {
	patOUBrute(s,p,i,0,mm,mm,1,l,carboxyl,begin,&count,name,i);
	sum += count;
	count = 0;
    }

    return sum;
}




/* @func embPatVariablePattern ***********************************************
**
** Match variable pattern against constant text.
** Used for matching many patterns against one sequence.
**
** @param [r] pattern [const AjPStr] pattern to match
** @param [r] text [const AjPStr] text to scan
** @param [r] patname [const AjPStr] ID or AC of pattern
** @param [u] l [AjPList] list on which to push hits
** @param [r] mode [ajuint] 1 for protein, 0 for nucleic acid
** @param [r] mismatch [ajuint] allowed mismatches
** @param [r] begin [ajuint] text offset
**
** @return [ajuint] number of hits
** @@
******************************************************************************/

ajuint embPatVariablePattern(const AjPStr pattern,
			    const AjPStr text,
			    const AjPStr patname, AjPList l, ajuint mode,
			    ajuint mismatch, ajuint begin)
{
    ajuint plen;
    ajuint slen = 0;

    AjBool amino;
    AjBool carboxyl;
    AjBool fclass;
    AjBool ajcompl;
    AjBool dontcare;
    AjBool range;
    ajint    *buf;			/* can be -1 */
    ajuint    hits;
    ajuint    m;
    ajuint    n;
    ajuint    i;
    ajuint    start;
    ajuint    end;

    AjOPatBYPNode off[AJALPHA];

    ajuint *sotable = NULL;
    ajuint solimit;

    EmbPPatMatch ppm;
    AjPStr regexp;

    ajuint **skipm;

    AjPStr cleanpattern = NULL;

    cleanpattern = ajStrNew();

    if(!embPatClassify(pattern,&cleanpattern,
		       &amino,&carboxyl,&fclass,&ajcompl,&dontcare,
		       &range,mode))
	ajFatal("Illegal pattern");

    plen = ajStrGetLen(cleanpattern);

    /*
    **  Select type of search depending on pattern
    */

    if(!range && !dontcare && !fclass && !ajcompl && !mismatch && plen>4)
    {
	/* Boyer Moore Horspool is the choice for long exact patterns */
	plen = ajStrGetLen(cleanpattern);
	AJCNEW(buf, AJALPHA);
	embPatBMHInit(cleanpattern,plen,buf);
	hits = embPatBMHSearch(text,cleanpattern,ajStrGetLen(text),
			       ajStrGetLen(cleanpattern),buf,
			       0,amino,carboxyl,l,
			       patname,begin);
        AJFREE(buf);
        ajStrDel(&cleanpattern);
	return hits;
    }

    else if(mismatch && !dontcare && !range && !fclass && !ajcompl)
    {
	/* Baeza-Yates Perleberg for exact patterns plus don't cares */
	plen = ajStrGetLen(cleanpattern);
	AJCNEW(buf, AJALPHA);
	embPatBYPInit(cleanpattern,plen,off,buf);
	hits = embPatBYPSearch(text,patname,begin,
			       ajStrGetLen(text),plen,mismatch,off,buf,l,
			       amino,carboxyl,cleanpattern);
	AJFREE(buf);
        ajStrDel(&cleanpattern);
	return hits;
    }


    if(!range && !dontcare && !fclass && !ajcompl && !mismatch)
    {
	/* Shift-OR is the choice for small exact patterns */
	AJCNEW(sotable, AJALPHA2);
	embPatSOInit(cleanpattern,sotable,&solimit);
	hits = embPatSOSearch(text,patname,*ajStrGetPtr(cleanpattern),
			      begin,plen,sotable,solimit,l,
			      amino,carboxyl);
	AJFREE(sotable);
        ajStrDel(&cleanpattern);
	return hits;
    }


    /* Next get m, the real pattern length */
    /* May as well set up a class search as well tho' it needs free's later */
    AJCNEW(sotable, AJALPHA2);
    embPatBYGCInit(cleanpattern,&m,sotable,&solimit);


    if(!range && (fclass || ajcompl) && !mismatch && m<=AJWORD)
    {
	/*
	**  Baeza-Yates Gonnet for classes and dontcares.
	**  No mismatches or ranges. Patterns less than (e.g.) 32
	**  Uses Shift-OR search engine
        */
	AJFREE(sotable);
	AJCNEW(sotable, AJALPHA2);
	embPatBYGCInit(cleanpattern,&m,sotable,&solimit);
	plen = m;
	hits = embPatBYGSearch(text,patname,
			       begin,plen,sotable,solimit,l,
			       amino,carboxyl);

	AJFREE(sotable);
        ajStrDel(&cleanpattern);
	return hits;
    }



    if(!mismatch && (range || m>AJWORD))
    {
	/*
	**  PCRE for ranges and simple classes longer than
        **  e.g. 32. No mismatches allowed
        **/
	AJFREE(sotable);
	regexp = embPatPrositeToRegExp(pattern); /* original pattern */
	ppm = embPatMatchFind(regexp,text,amino,carboxyl);
	n = embPatMatchGetNumber(ppm);
	for(i=0;i<n;++i)
	{
	    start = embPatMatchGetStart(ppm,i);
	    end   = embPatMatchGetEnd(ppm,i);
	    if(amino && start)
	    {
		n = 0;
		break;
	    }
	    if(!carboxyl || (carboxyl && start==slen-(end-start+1)))
		embPatPushHit(l,patname,start,end-start+1,
			      begin,0);
	}
	embPatMatchDel(&ppm);
	hits = n;
	ajStrDel(&regexp);
        ajStrDel(&cleanpattern);
	return hits;
    }


    if(mismatch && !range && (fclass || ajcompl))
    {
	/* Try a Tarhio-Ukkonen-Bleasby         */

	AJFREE(sotable);

	AJCNEW(skipm, m);
	for(i=0;i<m;++i)
	    AJCNEW(skipm[i], AJALPHA);

	embPatTUBInit(cleanpattern,skipm,m,mismatch,plen);
	hits = embPatTUBSearch(cleanpattern,text,ajStrGetLen(text),skipm,
			      m,mismatch,begin,
			      l,amino,carboxyl,patname,plen);
	for(i=0;i<m;++i)
	    AJFREE(skipm[i]);

	AJFREE(skipm);
        ajStrDel(&cleanpattern);
	return hits;
    }

    /*
    **  No choice left but to do a Bleasby recursive brute force
    */
    hits = embPatBruteForce(text,cleanpattern,amino,carboxyl,l,
			    begin,mismatch,patname);
    AJFREE(sotable);

    ajStrDel(&cleanpattern);
    return hits;
}




/* @func embPatRestrictPreferred ********************************************
**
** Replace RE names by the name of the prototype for that RE
**
** @param [u] l [AjPList] list of EmbPMatMatch hits
** @param [r] t [const AjPTable] table from embossre.equ file
**
** @return [void]
** @@
******************************************************************************/

void embPatRestrictPreferred(AjPList l, const AjPTable t)
{
    AjIList iter   = NULL;
    EmbPMatMatch m = NULL;
    AjPStr value   = NULL;

    iter = ajListIterNewread(l);

    while((m = (EmbPMatMatch)ajListIterGet(iter)))
    {
	value = ajTableFetch(t,m->cod);
	if(value)
	    ajStrAssignS(&m->cod,value);
    }

    ajListIterDel(&iter);

    return;
}




/* @func embPatRestrictRestrict ***********************************************
**
** Cut down the number of restriction enzyme hits from embPatRestrictScan
** Notably double reporting of symmetric palindromes and reporting
** of isoschizomers. Also provides an optional alphabetic sort.
**
** If we don't allow isoschizomers, then names of all isoschizomers
** found will be added to the string 'iso' in the returned list of
** EmbPMatMatch structures.  If 'isos' is AjTrue then they will be left alone.
**
** @param [u] l [AjPList] list of hits from embPatRestrictScan
** @param [r] hits [ajuint] number of hits from embPatRestrictScan
** @param [r] isos [AjBool] Allow isoschizomers
** @param [r] alpha [AjBool] Sort alphabetically
**
** @return [ajuint] adjusted number of hits
** @@
******************************************************************************/

ajuint embPatRestrictRestrict(AjPList l, ajuint hits, AjBool isos,
	AjBool alpha)
{
    EmbPMatMatch m  = NULL;
    EmbPMatMatch archetype = NULL; /* archetype of a set of isoschizomers */
    AjPStr  ps      = NULL;
    AjPList tlist   = NULL;
    AjPList newlist = NULL;

    ajuint i;
    ajuint v;
    ajuint tc   = 0;
    ajuint nc   = 0;
    ajint cut1 = 0;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    ajuint pos  = 0;

    ps      = ajStrNew();
    tlist   = ajListNew();
    newlist = ajListNew();


    /* Remove Mirrors for each enzyme separately */
    ajListSort(l,embPatRestrictNameCompare);
    tc = nc = 0;

    if(hits)
    {
	ajListPop(l,(void **)&m);
	ajStrAssignS(&ps,m->cod);
	ajListPush(l,(void *)m);
    }

    while(ajListPop(l,(void **)&m))
    {
	if(!ajStrCmpS(m->cod,ps))
	{
	    ajListPush(tlist,(void *)m);
	    ++tc;
	}
	else
	{
	    ajStrAssignS(&ps,m->cod);
	    ajListPush(l,(void *)m);
	    ajListSort(tlist,embPatRestrictStartCompare);
	    ajListSort(tlist,embPatRestrictCutCompare);
	    cut1 = cut2 = INT_MAX;
	    for(i=0;i<tc;++i)
	    {
		ajListPop(tlist,(void **)&m);
		if(cut1!=m->cut1)
		{
		    cut1=m->cut1;
		    ajListPush(newlist,(void *)m);
		    ++nc;
		}
		else
		    embMatMatchDel(&m);
	    }
	    tc = 0;
	}
    }
    ajListSort(tlist,embPatRestrictStartCompare);
    ajListSort(tlist,embPatRestrictCutCompare);

    cut1 = cut2 = INT_MAX;
    for(i=0;i<tc;++i)
    {
	ajListPop(tlist,(void **)&m);
	if(cut1!=m->cut1)
	{
	    cut1=m->cut1;
	    ajListPush(newlist,(void *)m);
	    ++nc;
	}
	else
	    embMatMatchDel(&m);
    }

    /* List l is currently empty - now reuse it  */

    hits = nc;
    ajListFree(&tlist);
    tlist = ajListNew();


    if(!isos)
    {
	/* Keep only first alphabetical isoschizomer */
	ajListSort(newlist,embPatRestrictStartCompare);
	if(hits)
	{
	    ajListPop(newlist,(void **)&m);
	    pos = m->start;
	    ajListPush(newlist,(void *)m);
	}
	tc = nc =0;

	while(ajListPop(newlist,(void **)&m))
	{
	    if(pos==m->start)
	    {
		/*
		** push groups of RE's that share the same start
		** position onto tlist to
		** be checked later to see if they are isoschizomers
		*/
		ajListPush(tlist,(void *)m);
		++tc;
	    }
	    else
	    {
		pos = m->start;

		ajListPush(newlist,(void *)m);

		/*
		** Now for list of all enz's which cut at same pos
		**  sorted by Name
		*/
		ajListSort(tlist,embPatRestrictNameCompare);

		/*
		** Now loop rejecting, for each left in the list,
		** anything similar
		*/

		/*
		** check for isoschizomers in the group sharing the
		** previous 'pos' here
		*/
		while(tc)
		{
		    ajListPop(tlist,(void **)&m);
		    cut1 = m->cut1;
		    cut2 = m->cut2;
		    cut3 = m->cut3;
		    cut4 = m->cut4;
		    ajStrAssignC(&ps,ajStrGetPtr(m->pat));

		    /*
		    ** first one of the group is not an isoschizomer,
		    ** by definition, so return it
		    */
		    ajListPush(l,(void *)m);
		    archetype = m;
		    ++nc;
		    --tc;


		    for(i=0,v=0;i<tc;++i)
		    {
			ajListPop(tlist,(void **)&m);

			if(m->cut1!=cut1 || m->cut2!=cut2 || m->cut3!=cut3 ||
			   m->cut4!=cut4 || ajStrCmpS(ps,m->pat))
			{
			    ajListPushAppend(tlist,(void *)m);
			    ++v;
			}

			else
			{
			    /*
			    ** same cut sites and pattern at the RE just
			    ** pushed onto 'l', so is an
			    **isoschizomer - add its name to the
			    ** archetype's list of isoschizomers and
			    ** delete
			    */
			    if(ajStrGetLen(archetype->iso) > 0)
			        ajStrAppendC(&archetype->iso, ",");

			    ajStrAppendS(&archetype->iso, m->cod);
			    embMatMatchDel(&m);
			}
		    }
		    tc = v;
		}
	    }
	}



	ajListSort(tlist,embPatRestrictNameCompare);
	while(tc)
	{
	    ajListPop(tlist,(void **)&m);
	    cut1 = m->cut1;
	    cut2 = m->cut2;
	    cut3 = m->cut3;
	    cut4 = m->cut4;
	    ajStrAssignC(&ps,ajStrGetPtr(m->pat));
	    /*
	    ** first one of the group is not an isoschizomer,
	    ** by definition, so return it
	    */
	    ajListPush(l,(void *)m);
	    archetype = m;
	    ++nc;
	    --tc;


	    for(i=0,v=0;i<tc;++i)
	    {
		ajListPop(tlist,(void **)&m);

		if(m->cut1!=cut1 || m->cut2!=cut2 || m->cut3!=cut3 ||
		   m->cut4!=cut4 || ajStrCmpS(ps,m->pat))
		{
		    ajListPushAppend(tlist,(void *)m);
		    ++v;
		}
		else
		{
		    /*
		    ** same cut sites and pattern as the RE
		    ** just pushed onto 'l', so is an
		    ** isoschizomer - add its name to the archetype's
		    ** list of isoschizomers and
		    ** delete
		    */
		    if(ajStrGetLen(archetype->iso) > 0)
		        ajStrAppendC(&archetype->iso, ",");

		    ajStrAppendS(&archetype->iso, m->cod);
		    embMatMatchDel(&m);
		}
	    }
	    tc = v;
	}
	hits = nc;

    }
    else
    {
	while(ajListPop(newlist,(void **)&m))
	{
	    ajListPush(l, (void*) m);
	}
	ajListFree(&newlist);
    }

    /* Finally sort on position of recognition sequence and print */
    ajListSort(l,embPatRestrictStartCompare);

    if(alpha)
	ajListSortTwo(l,embPatRestrictNameCompare, embPatRestrictStartCompare);

    ajStrDel(&ps);
    ajListFree(&tlist);
    ajListFree(&newlist);

    return hits;
}




/* @func embPatRestrictStartCompare *******************************************
**
** Sort restriction site hits on the basis of start position
**
** @param [r] a [const void *] First EmbPMatMatch hit
** @param [r] b [const void *] Second EmbPMatMatch hit
**
** @return [ajint] 0 if a and b are equal
**               -ve if a is less than b,
**               +ve if a is greater than b
******************************************************************************/

ajint embPatRestrictStartCompare(const void *a, const void *b)
{
    return (*(EmbPMatMatch const *)a)->start -
	(*(EmbPMatMatch const *)b)->start;
}




/* @func embPatRestrictCutCompare *********************************************
**
** Sort restriction site hits on the basis of cut position
**
** @param [r] a [const void *] First EmbPMatMatch hit
** @param [r] b [const void *] Second EmbPMatMatch hit
**
** @return [ajint] 0 if a and b are equal
**               -ve if a is less than b,
**               +ve if a is greater than b
******************************************************************************/

ajint embPatRestrictCutCompare(const void *a, const void *b)
{
    return (*(EmbPMatMatch const *)a)->cut1 - (*(EmbPMatMatch const *)b)->cut1;
}




/* @func embPatRestrictNameCompare ********************************************
**
** Sort restriction site hits on the basis of enzyme name
**
** @param [r] a [const void *] First EmbPMatMatch hit
** @param [r] b [const void *] Second EmbPMatMatch hit
**
** @return [ajint] 0 if a and b are equal
**               -ve if a is less than b,
**               +ve if a is greater than b
******************************************************************************/

ajint embPatRestrictNameCompare(const void *a, const void *b)
{
    return strcmp(ajStrGetPtr((*(EmbPMatMatch const *)a)->cod),
		  ajStrGetPtr((*(EmbPMatMatch const *)b)->cod));
}




/* @func embPatRestrictMatch **************************************************
**
** Main Restriction function. Scans sequence and rejects unwanted
** cutters
**
** @param [r] seq [const AjPSeq] sequence
** @param [r] begin [ajuint] start position in sequence
** @param [r] end [ajuint] end position in sequence
** @param [u] enzfile [AjPFile] file pointer to .enz file
** @param [r] enzymes [const AjPStr] comma separated list of REs
**                                  or NULL for all
** @param [r] sitelen [ajuint] minimum length of recognition site
** @param [r] plasmid [AjBool] Circular DNA
** @param [r] ambiguity [AjBool] Allow ambiguities
** @param [r] min [ajuint] minimum number of true cuts
** @param [r] max [ajuint] maximum number of true cuts
** @param [r] blunt [AjBool] Allow blunt cutters
** @param [r] sticky [AjBool] Allow sticky cutters
** @param [r] commercial [AjBool] Allow Only report REs with a supplier
** @param [u] l [AjPList] list for (EmbPMatMatch) hits
**
** @return [ajuint] number of hits
** @@
******************************************************************************/

ajuint embPatRestrictMatch(const AjPSeq seq, ajuint begin, ajuint end,
			   AjPFile enzfile,
			   const AjPStr enzymes, ajuint sitelen,
			   AjBool plasmid, AjBool ambiguity,
			   ajuint min, ajuint max,
			   AjBool blunt, AjBool sticky, AjBool commercial,
			   AjPList l)
{
    AjBool hassup;
    AjBool isall = ajTrue;
    AjPStr  name;
    const AjPStr  strand;
    AjPStr  substr;
    AjPStr  revstr;
    AjPStr  binstr;
    AjPStr  binrev;
    AjPStr  *ea;
    AjPStr  tmpstr = NULL;

    EmbPPatRestrict enz;


    ajuint len;
    ajuint plen;
    ajuint i;
    ajuint hits;
    ajuint ne;

    const char *cp;
    char *p;
    char *q;


    name   = ajStrNew();
    substr = ajStrNew();
    revstr = ajStrNew();
    binstr = ajStrNew();
    binrev = ajStrNew();

    enz = embPatRestrictNew();


    ne = 0;
    if(!enzymes)
	isall = ajTrue;
    else
    {
	ne = ajArrCommaList(enzymes,&ea);
	for(i=0;i<ne;++i)
	{
	    ajStrRemoveWhite(&ea[i]);
	    ajStrFmtUpper(&ea[i]);
	}

	if(ajStrMatchCaseC(ea[0],"all"))
	    isall = ajTrue;
	else
	    isall = ajFalse;
    }



    ajFileSeek(enzfile,0L,0);
    ajStrAssignS(&name,ajSeqGetNameS(seq));
    strand = ajSeqGetSeqS(seq);
    ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);
    ajStrFmtUpper(&substr);
    len = plen = ajStrGetLen(substr);
    ajStrAssignSubC(&revstr,ajStrGetPtr(strand),begin-1,end-1);
    ajStrFmtUpper(&revstr);
    ajSeqstrReverse(&revstr);

    ajStrAssignS(&binstr,substr);
    ajStrAssignS(&binrev,revstr);

    if(plasmid)
    {
	plen <<= 1;
	tmpstr = ajStrNew();
	ajStrAssignS(&tmpstr,substr);
	ajStrAppendC(&tmpstr,ajStrGetPtr(substr));
	ajStrAssignS(&substr,tmpstr);

	ajStrAssignS(&tmpstr,binstr);
	ajStrAppendC(&tmpstr,ajStrGetPtr(binstr));
	ajStrAssignS(&binstr,tmpstr);

	ajStrAssignS(&tmpstr,revstr);
	ajStrAppendC(&tmpstr,ajStrGetPtr(revstr));
	ajStrAssignS(&revstr,tmpstr);

	ajStrAssignS(&tmpstr,binrev);
	ajStrAppendC(&tmpstr,ajStrGetPtr(binrev));
	ajStrAssignS(&binrev,tmpstr);

	ajStrDel(&tmpstr);
    }

    q = ajStrGetuniquePtr(&binrev);
    p = ajStrGetuniquePtr(&binstr);
    for(i=0;i<plen;++i,++p,++q)
    {
	*p = (char)ajAZToBin(*p);
	*q = (char)ajAZToBin(*q);
    }


    hits = 0;
    while(embPatRestrictReadEntry(enz,enzfile))
    {
	if(!enz->ncuts)
	    continue;

	if(enz->len < sitelen)
	    continue;

	if(!blunt && enz->blunt)
	    continue;

	if(!sticky && !enz->blunt)
	    continue;

	cp = ajStrGetPtr(enz->pat);

	if(*cp>='A' && *cp<='Z')
	    hassup = ajTrue;
	else
	    hassup = ajFalse;

	if(!hassup && isall && commercial)
	    continue;

	ajStrFmtUpper(&enz->pat);

	if(!isall)
	{
	    for(i=0;i<ne;++i)
		if(ajStrMatchCaseS(ea[i],enz->cod))
		    break;
	    if(i==ne)
		continue;
	}

	hits += embPatRestrictScan(enz,substr,binstr,revstr,binrev,len,
				   ambiguity,plasmid,min,max,begin,l);
    }



    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    if(ne)
	AJFREE(ea);

    ajStrDel(&name);
    ajStrDel(&substr);
    ajStrDel(&revstr);
    ajStrDel(&binstr);
    ajStrDel(&binrev);
    embPatRestrictDel(&enz);

    return hits;
}




/* @func embPatGetType ********************************************************
**
** Return the type of a pattern
**
** @param [r] pattern [const AjPStr] original pattern
** @param [w] cleanpat [AjPStr *] cleaned pattern
** @param [r] mismatch [ajuint] number of allowed mismatches
** @param [r] protein [AjBool] true if protein
** @param [w] m [ajuint*] real length of pattern
** @param [w] left [AjBool*] must match left begin
** @param [w] right [AjBool*] must match right
**
** @return [ajuint] type of pattern
** @@
******************************************************************************/

ajuint embPatGetType(const AjPStr pattern, AjPStr *cleanpat,
		    ajuint mismatch, AjBool protein,
		    ajuint *m,
		    AjBool *left, AjBool *right)
{
    AjBool fclass;
    AjBool compl;
    AjBool dontcare;
    AjBool range;
    ajuint plen;
    ajuint type;
    const char *p;
    const char *q;

    ajStrAssignS(cleanpat,pattern);
    if(!embPatClassify(pattern,cleanpat,
		       left,right,&fclass,&compl,&dontcare,
		       &range,protein))
	return 0;

    /* Get real pattern length */
    p = ajStrGetPtr(*cleanpat);
    *m = 0;
    while(*p)
    {
	if(*p=='{')
	    while(*p!='}')
		++p;
	else if(*p=='[')
	    while(*p!=']')
		++p;
	++p;
	++*m;
    }


    plen = ajStrGetLen(*cleanpat);
    type = 0;

    /*
    **  Select type of search depending on pattern
    */

    if(!range && !dontcare && !fclass && !compl && !mismatch && plen>AJWORD)
    {
	/* Boyer Moore Horspool is the choice for ajlong exact patterns */
	type = 1;
    }
    else if(mismatch && !dontcare && !range && !fclass && !compl &&
	    plen<AJALPHA/2)
    {
	/* Baeza-Yates Perleberg for exact patterns plus mismatches */
	type = 2;
    }
    else if(!range && !dontcare && !fclass && !compl && !mismatch &&
	    plen<=AJWORD)
    {
	/* Shift-OR is the choice for small exact patterns */
	type = 3;
    }
    else if(!range && (fclass || compl || dontcare) && !mismatch && *m<=AJWORD)
    {
	/*
	 *  Baeza-Yates Gonnet for classes and dontcares.
	 *  No mismatches or ranges. Patterns less than (e.g.) 32
         */
	type = 4;
    }
    else if(!mismatch && (range || *m>AJWORD))
    {
        q = ajStrGetPtr(pattern);
	while(*q && *q!='?')
	    ++q;
	if(*q=='?')
	    type=7;
	else
	    type = 5;
    }
    else if(mismatch && !range && (fclass || compl))
    {
	/* Try a Tarhio-Ukkonen-Bleasby         */
	type = 6;
    }
    else if((mismatch && range) || !type)
    {
	/*
        **  No choice left but to do a Bleasby recursive brute force
        */
	type = 7;
    }

    ajDebug("embPatType %d '%S'\n", type, pattern);
    if (!ajStrMatchCaseS(pattern, *cleanpat))
	ajDebug("embPatType cleaned to '%S'\n", *cleanpat);

    return type;
}




/* @func embPatCompile ********************************************************
**
** Compile a pattern classified by embPatGetType
**
** @param [r] type [ajuint] pattern type
** @param [r] pattern [const AjPStr] original pattern
** @param [w] plen [ajuint*] pattern length
** @param [w] buf [ajint**] buffer for BMH and BYP search (can be -1)
** @param [w] off [EmbPPatBYPNode] offset buffer for B-Y/P search
** @param [w] sotable [ajuint**] buffer for SHIFT-OR
** @param [w] solimit [ajuint*] limit for SHIFT-OR
** @param [w] m [ajuint*] real length of pattern (from embPatGetType)
** @param [w] regexp [AjPStr *] PCRE regexp string
** @param [w] skipm [ajuint***] skip buffer for Tarhio-Ukkonen
** @param [r] mismatch [ajuint] number of allowed mismatches
**
** @return [void]
** @@
******************************************************************************/

void embPatCompile(ajuint type, const AjPStr pattern, ajuint* plen,
		   ajint** buf, EmbPPatBYPNode off, ajuint** sotable,
		   ajuint* solimit, ajuint* m, AjPStr* regexp, ajuint*** skipm,
		   ajuint mismatch)
{
    ajuint i = 0;

    *plen = ajStrGetLen(pattern);

    switch(type)
    {
    case 1:
	AJCNEW(*buf,AJALPHA);
	embPatBMHInit(pattern,*plen,*buf);
	break;
    case 2:
	AJCNEW(*buf,AJALPHA);
	embPatBYPInit(pattern,*plen,off,*buf);
	break;
    case 3:
	AJCNEW(*sotable,AJALPHA2);
	embPatSOInit(pattern,*sotable,solimit);
	*m = *plen;
	break;
    case 4:
	AJCNEW(*sotable,AJALPHA2);
	embPatBYGCInit(pattern,m,*sotable,solimit);
	break;
    case 5:
	*regexp = embPatPrositeToRegExp(pattern);
	break;
    case 6:
	AJCNEW(*skipm,*m);
	for(i=0;i<*m;++i)
	    AJCNEW((*skipm)[i],AJALPHA);
	embPatTUBInit(pattern,*skipm,*m,mismatch,*plen);
	break;
    case 7:
	break;
    default:
	ajFatal("embPatCompile: Cannot compile pattern");
	break;
    }

    return;
}




/* @func embPatFuzzSearch *****************************************************
**
** Fuzzy search after embPatGetType and embPatCompile
**
** @param [r] type [ajuint] pattern type
** @param [r] begin [ajuint] text displacement (1=start)
** @param [r] pattern [const AjPStr] processed pattern
** @param [r] name [const AjPStr] name associated with text
** @param [r] text [const AjPStr] text
** @param [u] l [AjPList] list to push hits onto
** @param [r] plen [ajuint] pattern length
** @param [r] mismatch [ajuint] number of allowed mismatches
** @param [r] left [AjBool] must match left
** @param [r] right [AjBool] must match right
** @param [u] buf [ajint*] buffer for BMH search
** @param [u] off [EmbPPatBYPNode] offset buffer for B-Y/P search
** @param [r] sotable [const ajuint*] buffer for SHIFT-OR
** @param [r] solimit [ajuint] limit for SHIFT-OR
** @param [r] regexp [const AjPStr] PCRE regexp string
** @param [r] skipm [ajuint* const *] skip buffer for Tarhio-Ukkonen-Bleasby
** @param [w] hits [ajuint*] number of hits
** @param [r] m [ajuint] real pat length (from embPatGetType/embPatCompile)
** @param [w] tidy [const void**] data to free
**
** @return [void]
** @@
******************************************************************************/

void embPatFuzzSearch(ajuint type, ajuint begin, const AjPStr pattern,
		      const AjPStr name, const AjPStr text, AjPList l,
		      ajuint plen, ajuint mismatch, AjBool left, AjBool right,
		      ajint *buf, EmbPPatBYPNode off, const ajuint *sotable,
		      ajuint solimit, const AjPStr regexp,
		      ajuint * const *skipm,
		      ajuint *hits, ajuint m, const void **tidy)
{
    EmbPPatMatch ppm;
    ajuint n;
    ajuint i;
    ajuint start;
    ajuint end;
    ajuint count = 0;

    ajDebug("embPatFuzzSearch type %d pattern: '%S'\n", type, pattern);

    switch(type)
    {
    case 1:
	*hits = embPatBMHSearch(text,pattern,ajStrGetLen(text),
			      ajStrGetLen(pattern),buf,0,left,right,l,
			      name,begin);
	*tidy = (const void *) buf;
	break;

    case 2:
	for(i=0;i<AJALPHA;++i)
	    buf[i] = plen;

	for(i=0;i<plen;++i)
	    buf[i] = AJALPHA;
	*hits=embPatBYPSearch(text,name,begin,
			      ajStrGetLen(text),plen,mismatch,off,buf,l,
			      left,right,pattern);
	*tidy = (const void *) buf;
	break;

    case 3:
	*hits = embPatSOSearch(text,name,*ajStrGetPtr(pattern),
			       begin,plen,sotable,solimit,l,
			     left,right);
	*tidy = (const void *) sotable;
	break;

    case 4:
	plen  = m;
	*hits = embPatBYGSearch(text,name,
				begin,plen,sotable,solimit,l,
				left,right);
	*tidy = (const void *) sotable;
	break;

    case 5:
	ppm = embPatMatchFind(regexp,text, left, right);
	n   = embPatMatchGetNumber(ppm);
	count = n;

	for(i=0;i<n;++i)
	{
	    start = embPatMatchGetStart(ppm,i);
	    end   = embPatMatchGetEnd(ppm,i);
	    ajDebug("embPatFuzzSearch embPatMatchFind left:%B start:%d\n",
		    left, start);
	    if(left && start)
	    {
		--count;
		continue;
	    }
	    if(right && start!=ajStrGetLen(text)-(end-start+1))
	    {
		--count;
		continue;
	    }
	    if(!right || (right && start==ajStrGetLen(text)-
			     (end-start+1)))
	    {
		ajDebug("embPatFuzzSearch type 5 push hit %B..%B %d..%d\n",
			left, right, start, end);
		embPatPushHit(l,name,start,end-start+1,
			      begin,0);
	    }
	    else
	    {
		ajDebug("embPatFuzzSearch type 5 skip hit %B..%B %d..%d\n",
			left, right, start, end);
	    }

	}
	embPatMatchDel(&ppm);
	*hits = count;
	break;

    case 6:
	*hits = embPatTUBSearch(pattern,text,ajStrGetLen(text),skipm,
				m,mismatch,begin,
				l,left,right,name,plen);
	*tidy = (const void *) skipm;
	break;

    case 7:
	*hits = embPatBruteForce(text,pattern,left,right,l,
				 begin,mismatch,name);
	break;

    default:
	ajFatal("Can't handle pattern type %S\n",pattern);
	break;
    }

    ajDebug("embPatFuzzSearch hits: %d\n", *hits);
    return;
}


/* @func embPatCompileII ******************************************************
**
** Compile a pattern classified by embPatGetType
**
** @param [u] thys [AjPPatComp] Prosite pattern stucture
** @param [r] mismatch [ajuint] number of allowed mismatches
**
** @return [void]
** @@
******************************************************************************/
void embPatCompileII (AjPPatComp thys, ajuint mismatch)
{
    ajuint i = 0;

    thys->plen = ajStrGetLen(thys->pattern);

    switch(thys->type)
    {
    case 1:
	if (!thys->buf)
	    AJCNEW(thys->buf,AJALPHA);
	embPatBMHInit(thys->pattern,thys->plen,thys->buf);
	break;
    case 2:
	if (!thys->buf)
	    AJCNEW(thys->buf,AJALPHA);
	embPatBYPInit(thys->pattern,thys->plen,thys->off,thys->buf);
	break;
    case 3:
	if (!thys->sotable)
	    AJCNEW(thys->sotable,AJALPHA2);
	embPatSOInit(thys->pattern,thys->sotable,&thys->solimit);
	thys->m = thys->plen;
	break;
    case 4:
	if (!thys->sotable)
	    AJCNEW(thys->sotable,AJALPHA2);
	embPatBYGCInit(thys->pattern,&thys->m,thys->sotable,&thys->solimit);
	break;
    case 5:
	if (!ajStrGetLen(thys->regex))
	    thys->regex = embPatPrositeToRegExp(thys->pattern);
	break;
    case 6:
	if (!thys->skipm)
	{
	    AJCNEW(thys->skipm,thys->m);
	    for(i=0;i<thys->m;++i)
		AJCNEW((thys->skipm)[i],AJALPHA);
	}
	embPatTUBInit(thys->pattern,thys->skipm,thys->m,mismatch,thys->plen);
	break;
    case 7:
	break;
    default:
	ajFatal("embPatCompileII: Cannot compile pattern");
	break;
    }

    return;
}

/* @func embPatFuzzSearchII ***************************************************
**
** Fuzzy search after embPatGetType and embPatCompile
**
** @param [u] thys [AjPPatComp] Prosite pattern stucture
** @param [r] begin [ajuint] Sequence displacement (1=start)
** @param [r] name [const AjPStr] Name associated with sequence
** @param [r] text [const AjPStr] Sequence
** @param [u] l [AjPList] List to push hits onto
** @param [r] mismatch [ajuint] number of allowed mismatches
** @param [w] hits [ajuint*] number of hits
** @param [w] tidy [const void**] data to free
**
** @return [void]
** @@
******************************************************************************/
void embPatFuzzSearchII (AjPPatComp thys, ajuint begin, const AjPStr name,
			 const AjPStr text, AjPList l, ajuint mismatch,
			 ajuint *hits, const void** tidy)
{
    EmbPPatMatch ppm;
    ajuint n;
    ajuint i;
    ajuint start;
    ajuint end;
    ajuint count = 0;

    ajDebug("embPatFuzzSearchII '%S' type %d '%s'\n",
	    thys->pattern, thys->type, patTypes[thys->type + 1]);

    switch(thys->type)
    {
    case 1:
	*hits = embPatBMHSearch(text,thys->pattern,ajStrGetLen(text),
			      ajStrGetLen(thys->pattern),
				thys->buf,0,thys->amino,
			      thys->carboxyl,l,name,begin);
	*tidy = (const void *) thys->buf;
	break;

    case 2:
	for(i=0;i<AJALPHA;++i)
	    thys->buf[i] = thys->plen;

	for(i=0;i<thys->plen;++i)
	    thys->buf[i] = AJALPHA;
	*hits=embPatBYPSearch(text,name,begin,ajStrGetLen(text),
			      thys->plen,mismatch,thys->off,thys->buf,l,
			      thys->amino,thys->carboxyl,thys->pattern);
	*tidy = (const void *) thys->buf;
	break;

    case 3:
	*hits = embPatSOSearch(text,name,*ajStrGetPtr(thys->pattern),begin,
			       thys->plen,thys->sotable,thys->solimit,l,
			       thys->amino,thys->carboxyl);
	*tidy = (const void *) thys->sotable;
	break;

    case 4:
	thys->plen  = thys->m;
	*hits = embPatBYGSearch(text,name,begin,
				thys->plen,thys->sotable,thys->solimit,l,
				thys->amino,thys->carboxyl);
	*tidy = (const void *) thys->sotable;
	break;

    case 5:
	ppm = embPatMatchFind(thys->regex, text,
			      thys->amino, thys->carboxyl);
	n   = embPatMatchGetNumber(ppm);
	count = n;

	for(i=0;i<n;++i)
	{
	    start = embPatMatchGetStart(ppm,i);
	    end   = embPatMatchGetEnd(ppm,i);
	    ajDebug("embPatFuzzSearch embPatMatchFind left:%B start:%d\n",
		    thys->amino, start);
	    if(thys->amino && start)
	    {
		--count;
		continue;
	    }
	    if(thys->carboxyl && start!=ajStrGetLen(text)-(end-start+1))
	    {
		--count;
		continue;
	    }
	    if(!thys->carboxyl || (thys->carboxyl && start==ajStrGetLen(text)-
			     (end-start+1)))
	    {
		ajDebug("embPatFuzzSearch type 5 push hit %B..%B %d..%d\n",
			thys->amino, thys->carboxyl, start, end);
		embPatPushHit(l,name,start,end-start+1,
			      begin,0);
	    }
	    else
	    {
		ajDebug("embPatFuzzSearch type 5 skip hit %B..%B %d..%d\n",
			thys->amino, thys->carboxyl, start, end);
	    }

	}
	embPatMatchDel(&ppm);
	*hits = count;
	break;

    case 6:
	*hits = embPatTUBSearch(thys->pattern,text,ajStrGetLen(text),
				thys->skipm,
				thys->m,mismatch,begin,l,
				thys->amino,thys->carboxyl,name,thys->plen);
	*tidy = (const void *) thys->skipm;
	break;

    case 7:
	*hits = embPatBruteForce(text,thys->pattern,thys->amino,thys->carboxyl,
				 l,begin,mismatch,name);
	break;

    default:
	ajFatal("Can't handle pattern type %S\n",thys->pattern);
	break;
    }

    return;
}

/* @func embPatGetTypeII ******************************************************
**
** Return the type of a pattern
**
** @param [u] thys [AjPPatComp] Prosite pattern stucture
** @param [r] pattern [const AjPStr] Original pattern
** @param [r] mismatch [ajuint] Number of allowed mismatches
** @param [r] protein [AjBool] True if protein
**
** @return [ajuint] type of pattern
** @@
******************************************************************************/
ajuint embPatGetTypeII (AjPPatComp thys, const AjPStr pattern, ajuint mismatch,
			AjBool protein)
{
    AjBool fclass;
    AjBool compl;
    AjBool dontcare;
    AjBool range;
    ajuint plen;
    ajuint type;
    const char *p;
    const char *q;

    ajStrAssignS(&thys->pattern,pattern);
    if(!embPatClassify(pattern,&thys->pattern,&thys->amino,&thys->carboxyl,
		       &fclass,&compl,&dontcare,&range,protein))
	return 0;

    /* Get real pattern length */
    p = ajStrGetPtr(thys->pattern);
    thys->m = 0;
    while(*p)
    {
	if(*p=='{')
	    while(*p!='}')
		++p;
	else if(*p=='[')
	    while(*p!=']')
		++p;
	++p;
	++thys->m;
    }


    plen = ajStrGetLen(thys->pattern);
    type = 0;

    /*
    **  Select type of search depending on pattern
    */

    if(!range && !dontcare && !fclass && !compl && !mismatch && plen>AJWORD)
    {
	/* Boyer Moore Horspool is the choice for ajlong exact patterns */
	type = 1;
    }
    else if(mismatch && !dontcare && !range && !fclass && !compl &&
	    plen<AJALPHA/2)
    {
	/* Baeza-Yates Perleberg for exact patterns plus mismatches */
	type = 2;
    }
    else if(!range && !dontcare && !fclass && !compl && !mismatch &&
	    plen<=AJWORD)
    {
	/* Shift-OR is the choice for small exact patterns */
	type = 3;
    }
    else if(!range &&
	    (fclass || compl || dontcare) &&
	    !mismatch && thys->m<=AJWORD)
    {
	/*
	 *  Baeza-Yates Gonnet for classes and dontcares.
	 *  No mismatches or ranges. Patterns less than (e.g.) 32
         */
	type = 4;
    }
    else if(!mismatch && (range || thys->m>AJWORD))
    {
        q = ajStrGetPtr(pattern);
	while(*q && *q!='?')
	    ++q;
	if(*q=='?')
	    type=7;
	else
	    type = 5;
    }
    else if(mismatch && !range && (fclass || compl))
    {
	/* Try a Tarhio-Ukkonen-Bleasby         */
	type = 6;
    }
    else if((mismatch && range) || !type)
    {
	/*
        **  No choice left but to do a Bleasby recursive brute force
        */
	type = 7;
    }

    ajDebug("embPatTypeII %d '%S'\n", thys->type, thys->pattern);
    if (!ajStrMatchCaseS(pattern, thys->pattern))
	ajDebug("embPatTypeII cleaned to '%S'\n", thys->pattern);

    thys->type=type;

    return type;
}
