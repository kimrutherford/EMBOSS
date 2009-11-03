/******************************************************************************
** @source embpatlist.c
**
** Functions for patternlist handling.
**
** @author Copyright (C) 2004 Henrikki Almusa, Medicel Oy
** @version 0.9
** @modified Jun 13 Beta version
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
#include "emboss.h"

/* @func embPatlistSeqSearch **************************************************
**
** The main search function of patterns. It compiles the patterns and searches
** with them. If the pattern fails to compile, it is removed from list.
**
** @param [w] ftable [AjPFeattable] Table of found features
** @param [r] seq [const AjPSeq] Sequence to search
** @param [u] plist [AjPPatlistSeq] List of patterns to search with
** @param [r] reverse [AjBool] Search reverese sequence as well
** @return [void]
** @@
******************************************************************************/
void embPatlistSeqSearch (AjPFeattable ftable, const AjPSeq seq,
			  AjPPatlistSeq plist, AjBool reverse)
{
    AjPPatternSeq patseq = NULL;
    AjPPatComp compPat;

    ajDebug ("embPatlistSearchListSeq: Searching '%S' for %d patterns\n",
	     ajSeqGetNameS(seq), ajPatlistSeqGetSize(plist));
    while (ajPatlistSeqGetNext(plist,&patseq))
    {
        compPat = ajPatternSeqGetCompiled(patseq);
	if (!compPat && !embPatternSeqCompile(patseq))
        {
            ajPatlistSeqRemoveCurrent(plist);
            continue;
        }
        embPatternSeqSearch(ftable,seq,patseq,reverse);
        ajDebug("end loop\n");
    }

    ajPatlistSeqRewind(plist);
    return;
}

/* @func embPatlistRegexSearch ************************************************
**
** The main search function of patterns. It compiles the patterns and searches
** with them. If the pattern fails to compile, it is removed from list.
**
** @param [w] ftable [AjPFeattable] Table of found features
** @param [r] seq [const AjPSeq] Sequence to search
** @param [u] plist [AjPPatlistRegex] List of patterns to search with
** @param [r] reverse [AjBool] Search reverese sequence as well
** @return [void]
** @@
******************************************************************************/
void embPatlistRegexSearch (AjPFeattable ftable, const AjPSeq seq,
			    AjPPatlistRegex plist, AjBool reverse)
{
    AjPPatternRegex patreg = NULL;
    AjPRegexp compPat;
    AjPStr tmp = NULL;

    ajStrAssignS(&tmp,ajSeqGetNameS(seq));
    ajDebug ("embPatlistSearchSequence: Searching '%S' for patterns\n",tmp);
    while (ajPatlistRegexGetNext(plist,&patreg))
    {
        compPat = ajPatternRegexGetCompiled(patreg);
	if (!compPat)
        {
            ajPatlistRegexRemoveCurrent(plist);
            continue;
        }
        embPatternRegexSearch(ftable,seq,patreg,reverse);
        ajDebug("end loop\n");
    }
    // ajDebug ("embPatlistSearchListRegex: Done search '%S'\n",tmp);

    ajPatlistRegexRewind(plist);

    ajStrDel(&tmp);

    return;
}

/* @func embPatternRegexSearch ************************************************
**
** The search function for a single regular expression pattern.
**
** @param [w] ftable [AjPFeattable] Table of found features
** @param [r] seq [const AjPSeq] Sequence to search
** @param [r] pat [const AjPPatternRegex] Pattern to search with
** @param [r] reverse [AjBool] Sequence has been reversed
** @return [void]
** @@
******************************************************************************/
void embPatternRegexSearch (AjPFeattable ftable, const AjPSeq seq,
			    const AjPPatternRegex pat, AjBool reverse)
{
    ajint pos=0;
    ajint off;
    ajint len;
    AjPFeature sf    = NULL;
    /*AjPSeq revseq    = NULL;*/
    AjPStr substr    = NULL;
    AjPStr seqstr    = NULL;
    AjPStr tmpstr = NULL;
    AjPStr tmp       = ajStrNew();
    AjPRegexp patexp = ajPatternRegexGetCompiled(pat);
    ajint adj;

    adj = ajSeqGetEnd(seq);
    pos = ajSeqGetBegin(seq);

/* this code is to reverse the sequence */
/*
    if (reverse)
    {
        revseq = ajSeqNewS (seq);
        ajSeqReverseForce(revseq);
        ajStrAssignSubS(&seqstr, ajSeqGetSeqS(revseq), pos-1, adj-1);
    }
*/
    ajStrAssignSubS(&seqstr, ajSeqGetSeqS(seq), pos-1, adj-1);

    ajStrFmtUpper(&seqstr);

    while(ajStrGetLen(seqstr) && ajRegExec(patexp, seqstr))
    {
	off = ajRegOffset(patexp);
	len = ajRegLenI(patexp, 0);

	if(off || len)
	{
	    ajRegSubI(patexp, 0, &substr);
	    ajRegPost(patexp, &tmp);
	    ajStrAssignS(&seqstr, tmp);
	    pos += off;
            if (reverse)
                sf = ajFeatNewIIRev (ftable, adj - pos - len + 2,
				     adj - pos + 1);
            else
	        sf = ajFeatNewII (ftable,pos,pos+len-1);
	    ajFmtPrintS (&tmpstr,"*pat %S",
			 ajPatternRegexGetName(pat));
	    ajFeatTagAdd (sf,NULL,tmpstr);
	    pos += len;
	}
	else
	{
	    pos++;
	    ajStrCutStart(&seqstr, 1);
	}
    }
/*
    if (reverse)
        ajSeqDel(&revseq);
*/

    ajStrDel(&tmpstr);
    ajStrDel(&tmp);
    ajStrDel(&substr);
    ajStrDel(&seqstr);

    return;
}

/* @func embPatternSeqSearch **************************************************
**
** The search function for a single sequence pattern.
**
** @param [w] ftable [AjPFeattable] Table of found features
** @param [r] seq [const AjPSeq] Sequence to search
** @param [r] pat [const AjPPatternSeq] Pattern to search with
** @param [r] reverse [AjBool] Search reverese sequence as well
** @return [void]  
** @@
******************************************************************************/
void embPatternSeqSearch (AjPFeattable ftable, const AjPSeq seq,
			  const AjPPatternSeq pat, AjBool reverse)
{
    const void *tidy;
    ajuint hits;
    ajuint i;
    AjPPatComp pattern;
    EmbPMatMatch m = NULL;
    AjPFeature sf  = NULL;
    AjPSeq revseq  = NULL;
    AjPList list   = ajListNew();
    AjPStr seqstr  = ajStrNew();
    AjPStr seqname = ajStrNew();
    AjPStr tmp     = ajStrNew();
    ajint adj;
    ajint begin;

    begin = ajSeqGetBegin(seq);
    adj = ajSeqGetEnd(seq);

    ajStrAssignS(&seqname,ajSeqGetNameS(seq));
    pattern = ajPatternSeqGetCompiled(pat);
    if (reverse)
    {
        revseq = ajSeqNewSeq(seq);
        ajSeqReverseForce(revseq);
        ajStrAssignSubS(&seqstr, ajSeqGetSeqS(revseq),begin-1,adj-1);
    }
    else
        ajStrAssignSubS(&seqstr, ajSeqGetSeqS(seq),begin-1,adj-1);

    ajStrFmtUpper(&seqstr);
    ajDebug("embPatternSeqSearch '%S' protein: %B reverse: %B\n",
	    pattern->pattern, pat->Protein, reverse);
    embPatFuzzSearchII(pattern,begin,seqname,seqstr,list,
                       ajPatternSeqGetMismatch(pat),&hits,&tidy);

    ajDebug ("embPatternSeqSearch: found %d hits\n",hits);
    if(!reverse)
	ajListReverse(list);

    for(i=0;i<hits;++i)
    {
        ajListPop(list,(void **)&m);
        if (reverse)
            sf = ajFeatNewIIRev(ftable, adj - m->start - m->len + 2,
				adj - m->start + 1);
        else
            sf = ajFeatNewII(ftable, m->start, m->start + m->len - 1);

	ajFeatSetScore(sf, (float) (m->len - m->mm));
	ajFeatSetStrand(sf, reverse);

        ajFmtPrintS(&tmp, "*pat %S", ajPatternSeqGetName(pat));
        ajFeatTagAdd(sf,NULL,tmp);
        if(m->mm)
        {
            ajFmtPrintS(&tmp, "*mismatch %d", m->mm);
            ajFeatTagAdd(sf, NULL, tmp);
        }

        embMatMatchDel(&m);
    }

    if (reverse)
        ajSeqDel(&revseq);

    ajStrDel(&seqname);
    ajStrDel(&seqstr);
    ajStrDel(&tmp);
    ajListFree(&list);

    return;
}


/* @func embPatternSeqCompile *************************************************
**
** Adds compiled pattern into AjPPattern.
**
** @param [w] pat [AjPPatternSeq] Pattern for compiling
** @return [AjBool] True, if compilation succeeded
** @@
******************************************************************************/
AjBool embPatternSeqCompile (AjPPatternSeq pat)
{
    AjPPatComp embpat;
    AjBool embType;
    AjPStr pattern = NULL;

    ajStrAssignS(&pattern,ajPatternSeqGetPattern(pat));
    ajStrFmtUpper(&pattern);
    ajDebug("embPatlistSeqCompile: name %S, pattern %S\n",
            ajPatternSeqGetName(pat),pattern);

    embpat = ajPatCompNew();
    if (ajPatternSeqGetProtein(pat))
	embType=ajTrue;
    else
	embType=ajFalse;
    if (!embPatGetTypeII(embpat,pattern,
			 ajPatternSeqGetMismatch(pat),embType))
    {
	ajDebug("embPatlistSeqCompile: Illegal pattern %S: '%S'\n",
		ajPatternSeqGetName(pat),ajPatternSeqGetPattern(pat));
	ajPatCompDel(&embpat);
	ajStrDel(&pattern);
	return ajFalse;
    }
    embPatCompileII(embpat,ajPatternSeqGetMismatch(pat));
    ajPatternSeqSetCompiled(pat,embpat);
    ajStrDel(&pattern);

    return ajTrue;
}
