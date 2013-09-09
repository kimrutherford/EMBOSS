/*
** notes
** =====
**
** for solo fragments, need to set lengths as we sort.
** Need similar array of cutsites (fa), reverse(fr), site# (fs) index (fi)
** and match (fm)
**
** need to copy the list properly ... otherwise sorting for solofrags
** changes the positions.
*/

/* @source restrict application
 **
 ** Reports restriction enzyme cleavage sites
 ** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
 ** @@
 **
 ** The author wishes to thank Helge Horch for important
 ** discussions and suggestions in the production of this program
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
 *****************************************************************************/

#include "emboss.h"
#include <string.h>
#include <limits.h>

#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "REBASE/embossre.equ"


#define EQUGUESS 3500	  /* Estimate of number of equivalent names */
#define ENZGUESS 5000     /* Estimate of withrefm entries           */


static void restrict_reportHits(AjPReport report, const AjPSeq seq,
				AjPFeattable TabRpt, AjPList l,
				ajuint hits, ajint begin, ajint end,
				AjBool ambiguity, ajint mincut, ajint maxcut,
				AjBool plasmid, AjBool blunt, AjBool sticky,
				ajint sitelen, AjBool limit,
				const AjPTable table,
				AjBool alpha, AjBool frags,
				AjBool ifrag);
static void restrict_read_equiv(AjPFile equfile, AjPTable table,
				AjBool commercial);
static void restrict_read_file_of_enzyme_names(AjPStr *enzymes);
static ajint restrict_enzcompare(const void *a, const void *b);
static void restrict_namecheck(const AjPStr enzymes, AjPFile enzfile);




/* @prog restrict *************************************************************
**
** Finds restriction enzyme cleavage sites
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq     = NULL;
    AjPStr enzymes = NULL;
    AjPReport report    = NULL;
    AjPFeattable TabRpt = NULL;
    ajint begin;
    ajint end;
    ajint min;
    ajint max;
    ajint sitelen;
    AjBool alpha;
    AjBool single;
    AjBool blunt;
    AjBool sticky;
    AjBool ambiguity;
    AjBool plasmid;
    AjBool commercial;
    AjBool limit;
    AjBool frags;
    AjBool methyl;
    
    AjPFile enzfile = NULL;
    AjPFile equfile = NULL;
    AjPFile methfile = NULL;
    
    AjPStr name = NULL;

    AjPTable table = NULL;

    ajint hits;


    AjPList l;
    AjBool  ifrag;

    
    embInit("restrict", argc, argv);

    seqall     = ajAcdGetSeqall("sequence");
    enzymes    = ajAcdGetString("enzymes");
    report     = ajAcdGetReport("outfile");
    min        = ajAcdGetInt("min");
    max        = ajAcdGetInt("max");
    sitelen    = ajAcdGetInt("sitelen");
    blunt      = ajAcdGetBoolean("blunt");
    sticky     = ajAcdGetBoolean("sticky");
    single     = ajAcdGetBoolean("single");
    alpha      = ajAcdGetBoolean("alphabetic");
    ambiguity  = ajAcdGetBoolean("ambiguity");
    plasmid    = ajAcdGetBoolean("plasmid");
    commercial = ajAcdGetBoolean("commercial");
    limit      = ajAcdGetBoolean("limit");
    frags      = ajAcdGetBoolean("fragments");
    methyl     = ajAcdGetBoolean("methylation");
    enzfile    = ajAcdGetDatafile("datafile");
    methfile   = ajAcdGetDatafile("mfile");
    ifrag      = ajAcdGetBoolean("solofragment");
    
    if (!blunt  && !sticky)
	ajFatal("Blunt/Sticky end cutters shouldn't both be disabled.");


    if(single)
	max = min = 1;

    table = ajTablestrNew(EQUGUESS);


    /* read the local file of enzymes names */
    restrict_read_file_of_enzyme_names(&enzymes);

    if(!enzfile)
    {
	enzfile = ajDatafileNewInNameC(ENZDATA);
	if(!enzfile)
	    ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");
    }


    if(enzymes)
        if(!ajStrMatchCaseC(enzymes,"all"))
            restrict_namecheck(enzymes,enzfile);

    
    if(limit)
    {
	equfile = ajDatafileNewInNameC(EQUDATA);
	if(!equfile)
	    limit = ajFalse;
	else
	{
	    restrict_read_equiv(equfile,table,commercial);
	    ajFileClose(&equfile);
	}
    }



    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);
	ajFileSeek(enzfile,0L,0);

	TabRpt = ajFeattableNewSeq(seq);

	if(frags)
	  ajReportSetTailC(report, "Fragment lengths:\n");
	else
	  ajReportSetTailC(report, "");

	l = ajListNew();
	hits = embPatRestrictMatch(seq,begin,end,enzfile,methfile,enzymes,
                                   sitelen,plasmid,ambiguity,min,max,blunt,
                                   sticky,commercial,methyl,l);

	ajDebug("Restrict found %d hits\n", hits);

	if(hits)
	{
	    name = ajStrNewC(ajSeqGetNameC(seq));
	    restrict_reportHits(report, seq, TabRpt,
				l,hits,begin,end,
				ambiguity,min,max,
				plasmid,blunt,sticky,sitelen,
				limit,table,
				alpha,frags,ifrag);
	    ajStrDel(&name);
	}
	else
	    ajReportSetTailC(report,"");
	
	ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);

	ajListFree(&l);
    }
    ajReportSetSeqstats(report, seqall);


    ajListFree(&l);
    ajSeqDel(&seq);
    ajFileClose(&enzfile);
    ajFileClose(&methfile);

    ajReportClose(report);
    ajReportDel(&report);

    ajSeqallDel(&seqall);
    ajStrDel(&enzymes);

    ajTablestrFree(&table);

    embExit();

    return 0;
}




/* @funcstatic restrict_reportHits ********************************************
**
** Print restriction sites
**
** @param [u] report [AjPReport] report
** @param [r] seq [const AjPSeq] sequence object
** @param [u] TabRpt [AjPFeattable] feature table object to store results
** @param [u] l [AjPList] hits
** @param [r] hits [ajuint] number of hits
** @param [r] begin [ajint] start position
** @param [r] end [ajint] end position
** @param [r] ambiguity [AjBool] allow ambiguities
** @param [r] mincut [ajint] minimum cuts
** @param [r] maxcut [ajint] maximum cuts
** @param [r] plasmid [AjBool] circular
** @param [r] blunt [AjBool] allow blunt cutters
** @param [r] sticky [AjBool] allow sticky cutters
** @param [r] sitelen [ajint] length of cut site
** @param [r] limit [AjBool] limit count
** @param [r] table [const AjPTable] supplier table
** @param [r] alpha [AjBool] alphabetic sort
** @param [r] frags [AjBool] show fragment lengths
** @param [r] ifrag [AjBool] show fragments for individual REs
** @@
******************************************************************************/

static void restrict_reportHits(AjPReport report, const AjPSeq seq,
				AjPFeattable TabRpt, AjPList l,
				ajuint hits, ajint begin, ajint end,
				AjBool ambiguity, ajint mincut, ajint maxcut,
				AjBool plasmid, AjBool blunt, AjBool sticky,
				ajint sitelen, AjBool limit,
				const AjPTable table,
				AjBool alpha, AjBool frags,
				AjBool ifrag)
{
    AjPFeature gf  = NULL;
    EmbPMatMatch m = NULL;
    AjPStr  ps = NULL;
    ajuint  *fa = NULL;
    ajuint  *fx = NULL;
    ajuint *fi = NULL;
    ajuint *fs = NULL;
    AjBool *fr = NULL;
    EmbPMatMatch *fm = NULL;
    ajuint fc = 0;
    ajuint fn = 0;
    ajuint fb = 0;
    ajuint last = 0;

    const AjPStr value   = NULL;
    AjPStr tmpStr  = NULL;
    AjPStr fthit   = NULL;
    AjPStr fragStr = NULL;
    AjPStr codStr  = NULL;
    AjPStr patStr  = NULL;
    
    ajuint i;
    ajuint j;
    ajuint imatch;

    ajuint fraglen = 0;
    
    ajuint c = 0;
    ajuint len;
    ajuint cend;
    
    ajuint  nfrags;

    AjIList iter = NULL;
    AjPList copyl = NULL;

    ps      = ajStrNew();
    fragStr = ajStrNew();
    codStr  = ajStrNew();
    patStr  = ajStrNew();
    
    fn = 0;
    len = ajSeqGetLen(seq);
    
    ajStrAssignC(&fthit, "hit");

    ajFmtPrintAppS(&tmpStr,"Minimum cuts per enzyme: %d\n",mincut);
    ajFmtPrintAppS(&tmpStr,"Maximum cuts per enzyme: %d\n",maxcut);
    ajFmtPrintAppS(&tmpStr,"Minimum length of recognition site: %d\n",
		sitelen);
    if(blunt)
	ajFmtPrintAppS(&tmpStr,"Blunt ends allowed\n");

    if(sticky)
	ajFmtPrintAppS(&tmpStr,"Sticky ends allowed\n");

    if(plasmid)
	ajFmtPrintAppS(&tmpStr,"DNA is circular\n");
    else
	ajFmtPrintAppS(&tmpStr,"DNA is linear\n");

    if(!ambiguity)
	ajFmtPrintAppS(&tmpStr,"No ambiguities allowed\n");
    else
	ajFmtPrintAppS(&tmpStr,"Ambiguities allowed\n");


    if(limit)
    {
        if(alpha)
            ajListSortTwo(l,
                          &embPatRestrictNameCompare,
                          &embPatRestrictStartCompare);

        iter = ajListIterNewread(l);
        while(!ajListIterDone(iter))
        {
            m = ajListIterGet(iter);
	    value = ajTableFetchS(table,m->cod);
	    if(value)
		ajStrAssignS(&m->cod,value);
	}
        ajListIterDel(&iter);
    }
    
    hits = embPatRestrictRestrict(l,hits,!limit,alpha);

    ajReportSetHeaderS(report, tmpStr);



    if(frags || ifrag)
    {
	fa = AJALLOC(hits*2*sizeof(ajuint));
	fr = AJALLOC(hits*2*sizeof(AjBool));
	fi = AJALLOC(hits*2*sizeof(ajuint));
	fs = AJALLOC(hits*2*sizeof(ajuint));
	fx = AJALLOC(hits*2*sizeof(ajuint));
    }
    
    if(frags)
    {
        ajListToarray(l, (void***) &fm);

        fn = 0;
        imatch = 0;

        iter = ajListIterNewread(l);
        while(!ajListIterDone(iter))
        {
            m = ajListIterGet(iter);

            fr[fn] = ajFalse;
            fi[fn] = fn;
            fs[fn] = imatch;
	    fa[fn++] = m->cut1;

            if(m->cut3 || m->cut4)
            {
                if(plasmid || !m->circ34)
                {
                    fr[fn] = ajTrue;
                    fi[fn] = fn;
                    fs[fn] = imatch;
                    fa[fn++] = m->cut3;
                }
            }

            imatch++;
        }

        ajListIterDel(&iter);
    }
    
    
    /* report tail */
    ajStrAssignC(&fragStr, "");


    if(frags)                   /* fragment lengths in table and reported */
    {
	ajSortUintIncI(fa,fi,fn);

	if(!fn || (fn==1 && plasmid))
	    ajFmtPrintAppS(&fragStr,"    %d\n",end-begin+1);
	else
	{
	    last = -1;
	    fb = 0;
	    for(i=0;i<fn;++i)
	    {
		c=fa[fi[i]];
                if(c != last)
                {
		    fi[fb++]=fi[i];
                    last=c;
                }
	    }

	    fn = fb;

	    /* Calc lengths */
	    for(i=0; i < (fn-1); ++i)
            {
                if(i && !ifrag)
                {
                    if(fr[fi[i]])
                        fm[fs[fi[i]]]->len3 = fraglen;
                    else
                        fm[fs[fi[i]]]->len1 = fraglen;
                }
                
		fraglen = fa[fi[i+1]] - fa[fi[i]];
                fx[fc++] = fraglen;

                if(!ifrag)
                {
                    if(fr[fi[i]])
                        fm[fs[fi[i]]]->len4 = fraglen;
                    else
                        fm[fs[fi[i]]]->len2 = fraglen;
                }
            }

            if(i && !ifrag)
                fm[fs[fi[i]]]->len1 = fraglen;

	    if(!plasmid)
	    {
		fraglen = fa[fi[0]] - begin + 1;
                fx[fc++] = fraglen;

                if(!ifrag)
                {
                    if(fr[fi[0]])
                        fm[fs[fi[0]]]->len3 = fraglen;
                    else
                        fm[fs[fi[0]]]->len1 = fraglen;
                }
                
                fraglen = end - fa[fi[fn-1]];
		fx[fc++] = fraglen;

                if(!ifrag)
                {
                    if(fr[fi[fn-1]])
                        fm[fs[fi[fn-1]]]->len4 = fraglen;
                    else
                        fm[fs[fi[fn-1]]]->len2 = fraglen;
                }
	    }
	    else
            {
                fraglen = (fa[fi[0]] - begin + 1) + (end - fa[fi[fn-1]]);
                fx[fc++] = fraglen;

                if(!ifrag)
                {
                    if(fr[fi[0]])
                        fm[fs[fi[0]]]->len3 = fraglen;
                    else
                        fm[fs[fi[0]]]->len1 = fraglen;

                    if(fr[fi[fn-1]])
                        fm[fs[fi[fn-1]]]->len4 = fraglen;
                    else
                        fm[fs[fi[fn-1]]]->len2 = fraglen;
                }
            }

	    ajSortUintDec(fx,fc);

	    for(i=0;i<fc;++i)
		ajFmtPrintAppS(&fragStr,"    %d\n",fx[i]);
	}

        AJFREE(fm);

    }

    if(ifrag)          /* calculate fragment lengths for single digests */
    {
        copyl = ajListNewListref(l);
	ajListSort(copyl, &restrict_enzcompare);
        ajListToarray(copyl, (void***) &fm);

	nfrags = 0;
        imatch = 0;

        iter = ajListIterNewread(copyl);
        while(!ajListIterDone(iter))
	{
	    m = ajListIterGet(iter);

	    if(!plasmid && m->circ12)
            {
                imatch++;
		continue;
            }

	    if(limit)
	    {
		value = ajTableFetchS(table,m->cod);

		if(value)
		    ajStrAssignS(&m->cod,value);
	    }


	    if(!ajStrGetLen(codStr))
	    {
		ajStrAssignS(&patStr,m->pat);
		ajStrAssignS(&codStr,m->cod);
	    }


	    if(ajStrMatchS(codStr,m->cod)) /* same enzyme */
	    {
                fr[nfrags] = ajFalse;
                fi[nfrags] = nfrags;
                fs[nfrags] = imatch;
                fa[nfrags++] = m->cut1;

		if(m->cut3 || m->cut4)
		{
                    if (plasmid || !m->circ34)
                    {
                        fr[nfrags] = ajTrue;
                        fi[nfrags] = nfrags;
                        fs[nfrags] = imatch;
                        fa[nfrags++] = m->cut3;
                    }
                }
	    }
	    else                /* next enzyme - report previous enzyme */
	    {
		ajFmtPrintAppS(&fragStr,"\n%S:\n[%S]",
			       codStr,patStr);
 
		ajStrAssignS(&codStr,m->cod);
		ajStrAssignS(&patStr,m->pat);

		ajSortUintIncI(fa, fi, nfrags);

		last = 0;
		for(j=0;j<nfrags;++j)
		{
		    fraglen = fa[fi[j]] - last;
                    fx[j] = fraglen;

                    if(fr[fi[j]])
                        fm[fs[fi[j]]]->len3 = fraglen;
                    else
                        fm[fs[fi[j]]]->len1 = fraglen;

                    if(j)
                    {
                        if(fr[fi[j-1]])
                            fm[fs[fi[j-1]]]->len4 = fraglen;
                        else
                            fm[fs[fi[j-1]]]->len2 = fraglen;
                    }

		    last  = fa[fi[j]];
                }

		if(!(nfrags && plasmid))
		{
		    if(j)
                    {
                        fraglen = len - fa[fi[j-1]];
                        if(fr[fi[j-1]])
                            fm[fs[fi[j-1]]]->len4 = fraglen;
                        else
                            fm[fs[fi[j-1]]]->len2 = fraglen;
                    }
                    else
                        fraglen = len;
                    fx[j] = fraglen;

                    if(fr[fi[j-1]])
                        fm[fs[fi[j-1]]]->len4 = fraglen;
                    else
                        fm[fs[fi[j-1]]]->len2 = fraglen;

		    ++nfrags;
		}
		else
		{
                    if(nfrags == 1)
                    {
                        fraglen = len;
                        fa[fi[0]] = fraglen;
                        if(fr[fi[0]])
                            fm[fs[fi[0]]]->len3 = fraglen;
                        else
                            fm[fs[fi[0]]]->len1 = fraglen;
                    }
                    else
                    {
                        fraglen = fa[fi[0]] + (len - fa[fi[j-1]]);
                        fa[fi[0]] = fraglen;
                        if(fr[fi[0]])
                            fm[fs[fi[0]]]->len3 = fraglen;
                        else
                            fm[fs[fi[0]]]->len1 = fraglen;
                    }
		}

		ajSortUintInc(fx,nfrags);

		for(j=0;j<nfrags;++j)
		{
		    if(!(j%6))
			ajFmtPrintAppS(&fragStr,"\n");
		    ajFmtPrintAppS(&fragStr,"\t%d",fx[j]);
		}
		ajFmtPrintAppS(&fragStr,"\n");

                /* next enzyme first cut site */

		nfrags = 0;

                fr[nfrags] = ajFalse;
                fi[nfrags] = nfrags;
                fs[nfrags] = imatch;
                fa[nfrags++] = m->cut1;

		if(m->cut3 || m->cut4)
		{
                    if (plasmid || !m->circ34)
                    {
                        fr[nfrags] = ajTrue;
                        fi[nfrags] = nfrags;
                        fs[nfrags] = imatch;
                        fa[nfrags++] = m->cut3;
                    }
                }

	    }

            imatch++;
	}
        ajListIterDel(&iter);

	if(nfrags)              /* report last enzyme */
	{
	    ajFmtPrintAppS(&fragStr,"\n%S:\n[%S]",
			   codStr,patStr);

	    ajSortUintIncI(fa, fi, nfrags);

	    last = 0;
	    for(j=0;j<nfrags;++j)
	    {
		fraglen = fa[fi[j]] - last;
                fx[j] = fraglen;

                if(fr[fi[j]])
                    fm[fs[fi[j]]]->len3 = fraglen;
                else
                    fm[fs[fi[j]]]->len1 = fraglen;

                if(j)
                {
                    if(fr[fi[j-1]])
                        fm[fs[fi[j-1]]]->len4 = fraglen;
                    else
                        fm[fs[fi[j-1]]]->len2 = fraglen;
                }

		last  = fa[fi[j]];
	    }

	    if(!(nfrags && plasmid))
	    {
                if(j)
                {
                    fraglen = len - fa[fi[j-1]];
                    if(fr[fi[j-1]])
                        fm[fs[fi[j-1]]]->len4 = fraglen;
                    else
                        fm[fs[fi[j-1]]]->len2 = fraglen;
                }
                else
                    fraglen = len;
                fx[j] = fraglen;
		++nfrags;
	    }
	    else
	    {
		if(nfrags == 1)
                {
                    fraglen = len;
		    fa[fi[0]] = fraglen;
                    if(fr[fi[0]])
                        fm[fs[fi[0]]]->len3 = fraglen;
                    else
                        fm[fs[fi[0]]]->len1 = fraglen;
                }
		else
                {
                    fraglen = fa[fi[0]] + (len - fa[fi[j-1]]);
		    fa[fi[0]] = fraglen;
                    if(fr[fi[0]])
                        fm[fs[fi[0]]]->len3 = fraglen;
                    else
                        fm[fs[fi[0]]]->len1 = fraglen;
                }
	    }

	    ajSortUintInc(fa,nfrags);

	    for(j=0;j<nfrags;++j)
	    {
		if(!(j%6))
		    ajFmtPrintAppS(&fragStr,"\n");
		ajFmtPrintAppS(&fragStr,"\t%d",fx[j]);
	    }
	    ajFmtPrintAppS(&fragStr,"\n");
	    AJFREE(fa);
	}
        AJFREE(fm);
        ajListFree(&copyl);
    }

    iter = ajListIterNewread(l);
    while(!ajListIterDone(iter))
    {
        m = ajListIterGet(iter);

	if(!plasmid && m->circ12)
	    continue;

	if(m->forward)
	{
	    cend = m->start + ajStrGetLen(m->pat) - 1;
	    gf = ajFeatNewII (TabRpt,
			   m->start, cend);
	}
	else
	{
	    cend = m->start - ajStrGetLen(m->pat) + 1;
	    gf = ajFeatNewIIRev(TabRpt,
			   m->start, cend);
	}

	ajFmtPrintS(&tmpStr, "*enzyme %S", m->cod);
	ajFeatTagAddSS(gf,  NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*site %S", m->pat);
	ajFeatTagAddSS(gf,  NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*5prime %d", m->cut1);
	ajFeatTagAddSS(gf,  NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*3prime %d", m->cut2);
	ajFeatTagAddSS(gf,  NULL, tmpStr);
	if(m->len1)
        {
            ajFmtPrintS(&tmpStr, "*5frag %u", m->len1);
            ajFeatTagAddSS(gf,  NULL, tmpStr);
        }
	if(m->len2)
        {
            ajFmtPrintS(&tmpStr, "*3frag %u", m->len2);
            ajFeatTagAddSS(gf,  NULL, tmpStr);
        }

	if(m->cut3 || m->cut4)
	{
	    if(plasmid || !m->circ34)
	    {
		ajFmtPrintS(&tmpStr, "*5primerev %d", m->cut3);
		ajFeatTagAddSS(gf,  NULL, tmpStr);
		ajFmtPrintS(&tmpStr, "*3primerev %d", m->cut4);
		ajFeatTagAddSS(gf,  NULL, tmpStr);
                if(m->len3)
                {
                    ajFmtPrintS(&tmpStr, "*5fragrev %u", m->len3);
                    ajFeatTagAddSS(gf,  NULL, tmpStr);
                }
                if(m->len4)
                {
                    ajFmtPrintS(&tmpStr, "*3fragrev %u", m->len4);
                    ajFeatTagAddSS(gf,  NULL, tmpStr);
                }
	    }
	}
    }
    ajListIterDel(&iter);

    
    ajReportAppendTailS(report, fragStr);

    for(i=0;i<hits;++i)
    {
	ajListPop(l,(void **)&m);
	embMatMatchDel(&m);
    }
    
    AJFREE(fa);
    AJFREE(fr);
    AJFREE(fs);
    AJFREE(fi);
    AJFREE(fx);

    ajStrDel(&fragStr);
    ajStrDel(&codStr);
    ajStrDel(&patStr);
    ajStrDel(&ps);
    ajStrDel(&tmpStr);
    ajStrDel(&fthit);

    return;
}




/* @funcstatic restrict_read_equiv ********************************************
**
** Read table of equivalents
**
** @param [u] equfile [AjPFile] equivalent name file
** @param [w] table [AjPTable]  equivalent names
** @param [r] commercial [AjBool] only commercial suppliers (for asterisk
**                                removal
** @@
******************************************************************************/


static void restrict_read_equiv(AjPFile equfile, AjPTable table,
				AjBool commercial)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    const char *p;

    line = ajStrNew();

    while(ajReadlineTrim(equfile,&line))
    {
	p = ajStrGetPtr(line);

	if(!*p || *p=='#' || *p=='!')
	    continue;

	p     = ajSysFuncStrtok(p," \t\n");
	key   = ajStrNewC(p);
	p     = ajSysFuncStrtok(NULL," \t\n");
	value = ajStrNewC(p);
	if(!commercial)
	    ajStrTrimEndC(&value,"*");
	ajTablePut(table,(void *)key, (void *)value);
    }

    ajStrDel(&line);

    return;
}




/* @funcstatic restrict_read_file_of_enzyme_names *****************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [u] enzymes [AjPStr*] enzymes to search for or 'all' or '@file'
** @return [void]
** @@
******************************************************************************/

static void restrict_read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file = NULL;
    AjPStr line;
    const char *p = NULL;

    if(ajStrFindC(*enzymes, "@") == 0)
    {
	ajStrTrimC(enzymes, "@");	/* remove the @ */
	file = ajFileNewInNameS(*enzymes);
	if(file == NULL)
	    ajFatal ("Cannot open the file of enzyme names: '%S'", enzymes);

	/* blank off the enzyme file name and replace with the enzyme names */
	ajStrSetClear(enzymes);
	line = ajStrNew();
	while(ajReadlineTrim(file, &line))
	{
	    p = ajStrGetPtr(line);

	    if(!*p || *p == '#' || *p == '!')
		continue;

	    ajStrAppendS(enzymes, line);
	    ajStrAppendC(enzymes, ",");
	}
	ajStrDel(&line);

	ajFileClose(&file);
    }

    return;
}




/* @funcstatic restrict_enzcompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] enzyme1
** @param [r] b [const void*] enzyme2
** @return [ajint] 0 = bases match
** @@
******************************************************************************/


static ajint restrict_enzcompare(const void *a, const void *b)
{
    return strcmp((*(EmbPMatMatch const *)a)->cod->Ptr,
		  (*(EmbPMatMatch const *)b)->cod->Ptr);
}




/* @funcstatic restrict_namecheck *******************************************
**
** Check that all given restriction enzyme names exists
**
** @param [r] enzymes [const AjPStr] enzyme list
** @param [u] enzfile [AjPFile] enzyme file
** @return [void]
** @@
******************************************************************************/

static void restrict_namecheck(const AjPStr enzymes, AjPFile enzfile)
{
    ajuint ne = 0;
    ajuint i;
    
    AjPStr *ea = NULL;
    AjPTable enztable = NULL;
    
    AjPStr key   = NULL;
    AjPStr value = NULL;

    AjPStr oldval = NULL;

    EmbPPatRestrict enz = NULL;
    
    enztable = ajTablestrNew(ENZGUESS);
    enz      = embPatRestrictNew();
    
    ne = ajArrCommaList(enzymes,&ea);
    if(!ne)
        ajFatal("No restriction enzyme names in '%S'",enzymes);

    while(!ajFileIsEof(enzfile))
    {
        if(!embPatRestrictReadEntry(enz,enzfile))
	    continue;

        key = ajStrNew();
        ajStrAssignS(&key,enz->cod);
        ajStrFmtUpper(&key);
        value = ajStrNewC("dummy");
        
        /* check for duplicate names with multiple target sites */
	oldval = ajTablePut(enztable,(void *)key, (void *)value);
        if(oldval)
            ajStrDel(&oldval);
    }
    
 
    for(i=0; i < ne; ++i)
    {
        ajStrRemoveWhite(&ea[i]);
        ajStrFmtUpper(&ea[i]);

        if(!ajTableMatchS(enztable,ea[i]))
            ajFatal("Restriction enzyme %S isn't in the database",ea[i]);
    }
    
    ajFileSeek(enzfile,0L,0);

    ajTablestrFree(&enztable);
    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    if(ne)
	AJFREE(ea);

    embPatRestrictDel(&enz);
    
    return;
}
