/* @source silent application
**
** Find silent mutation sites for restriction enzyme/SDM experiments
**
** @author Copyright (C) Amy Williams (bmbawi@bmb.leeds.ac.uk)
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
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
******************************************************************************/

#include "emboss.h"
#include <limits.h>




/* @datastatic PRinfo *******************************************************
**
** recoder internals for RE information
**
** @alias SRinfo
** @alias ORinfo
**
** @attr code [AjPStr] structure for silent mutation info
** @attr site [AjPStr] Undocumented
** @attr revsite [AjPStr] Undocumented
** @attr ncuts [ajint] Undocumented
** @attr cut1 [ajint] Undocumented
** @attr cut2 [ajint] Undocumented
** @attr cut3 [ajint] Undocumented
** @attr cut4 [ajint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct SRinfo
{
    AjPStr code;
    AjPStr site;
    AjPStr revsite;
    ajint ncuts;
    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    char Padding[4];
} ORinfo;
#define PRinfo ORinfo*




/* @datastatic PSilent ******************************************************
**
** recoder internals for silent sites
**
** @alias SSilent
** @alias OSilent
**
** @attr code [AjPStr] Undocumented
** @attr site [AjPStr] Undocumented
** @attr match [ajint] Undocumented
** @attr base [ajint] Undocumented
** @attr seqaa [AjPStr] Undocumented
** @attr reaa [AjPStr] Undocumented
** @attr issilent [AjBool] Undocumented
** @attr obase [char] Undocumented
** @attr nbase [char] Undocumented
** @attr Padding [char[2]] Padding to alignment boundary
******************************************************************************/

typedef struct SSilent
{
    AjPStr code;
    AjPStr site;
    ajint match;
    ajint base;
    AjPStr seqaa;
    AjPStr reaa;
    AjBool issilent;
    char   obase;
    char   nbase;
    char Padding[2];
} OSilent;
#define PSilent OSilent*



static void silent_relistdel(AjPList* relist);
static AjPList silent_mismatch(const AjPStr sstr, AjPList ressite,
			       AjPStr* tailstr,
			       const AjPStr sname, ajint RStotal, ajint begin,
			       ajint end,
			       AjBool tshow);
static ajint silent_restr_read(AjPList *relist, const AjPStr enzymes);
static PSilent silent_checktrans(const AjPStr seq,const EmbPMatMatch match,
				const PRinfo rlp, ajint begin,
				AjBool rev, ajint end);
static void silent_fmt_sequence(const char* title, const AjPStr seq,
				AjPStr* tailstr,
				ajint start, AjBool num);
static void silent_fmt_hits(AjPList hits, AjPFeattable feat,
			    AjBool silent);
static void silent_split_hits(AjPList *hits, AjPList *silents,
			      AjPList *nonsilents, AjBool allmut);
static ajint silent_basecompare(const void *a, const void *b);




/* @prog silent ***************************************************************
**
** Silent mutation restriction enzyme scan
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq    = NULL;
    AjPReport report = NULL;
    AjPFeattable feat=NULL;
    AjPStr sstr   = NULL;

    const AjPStr sname   = NULL;
    ajint RStotal;
    AjPStr enzymes = NULL;                /* string for RE selection */

    AjPList relist = NULL;
    ajint begin;
    ajint end;
    ajint start;
    AjBool sshow;
    AjBool tshow;
    AjBool allmut;

    AjPList results1 = NULL;              /* for forward strand */
    AjPList silenthits;
    AjPList otherhits;
    AjPStr tailstr = NULL;



    embInit("silent", argc, argv);

    seq     = ajAcdGetSeq("sequence");
    enzymes = ajAcdGetString("enzymes");
    sshow   = ajAcdGetBoolean("sshow");
    tshow   = ajAcdGetBoolean("tshow");
    allmut  = ajAcdGetBoolean("allmut");
    report = ajAcdGetReport ("outfile");

    silenthits  = ajListNew();
    otherhits  = ajListNew();

    /*calling function to read in RE info*/
    RStotal = silent_restr_read(&relist,enzymes);

    begin = ajSeqGetBegin(seq);             /* returns the seq start posn, or 1
                                            if no start has been set */
    end   = ajSeqGetEnd(seq);               /* returns the seq end posn, or seq
                                            length if no end has been set */

    ajStrAssignSubC(&sstr,ajSeqGetSeqC(seq),--begin,--end);
    ajStrFmtUpper(&sstr);

    sname = ajSeqGetNameS(seq);
    start  = begin+1;

    feat = ajFeattableNewDna(ajSeqGetNameS(seq));

    if(sshow)
    {
        silent_fmt_sequence("SEQUENCE", sstr,&tailstr,start,ajTrue);
    }

    results1 = silent_mismatch(sstr,relist,&tailstr,sname,RStotal,begin,
			       end,tshow);

    silent_split_hits(&results1,&silenthits,&otherhits,allmut);

    ajReportSetHeaderC(report,
		       "KEY:\n"
		       "EnzymeName: Enzyme name\n"
		       "RS-Pattern: Restriction enzyme recognition site "
		       "pattern\n"
		       "Base-Posn: Position of base to be mutated\n"
		       "AAs: Amino acid. Original sequence(.)After mutation\n"
		       "Silent: Yes for unchanged amino acid\n"
		       "Mutation: The base mutation to perform\n\n"
		       "Creating silent and non-silent mutations\n");

    silent_fmt_hits(silenthits,feat, ajTrue);
    if(allmut)
        silent_fmt_hits(otherhits,feat, ajTrue);
        

    ajReportSetStatistics(report, 1, ajSeqGetLenTrimmed(seq));
    ajReportSetTailS(report, tailstr);
    (void) ajReportWrite (report,feat,seq);
    ajFeattableDel(&feat);

    ajStrDel(&enzymes);

    ajListFree(&results1);
    ajListFree(&silenthits);
    ajListFree(&otherhits);

    ajReportClose(report);
    ajReportDel(&report);
    ajSeqDel(&seq);
    ajStrDel(&sstr);

    silent_relistdel(&relist);
    ajStrDel(&tailstr);

    embExit();

    return 0;
}




/* @funcstatic silent_relistdel ***********************************************
**
** Undocumented.
**
** @param [d] relist [AjPList*] restriction enzymes
** @return [void]
******************************************************************************/

static void silent_relistdel(AjPList* relist)
{
    PRinfo rlp = NULL;
    
    while(ajListPop(*relist,(void **)&rlp))
    {
	ajStrDel(&rlp->code);
	ajStrDel(&rlp->site);
	ajStrDel(&rlp->revsite);
	AJFREE(rlp);
    }
    ajListFree(relist);
}




/* @funcstatic silent_mismatch ************************************************
**
** Undocumented.
**
** @param [r] sstr [const AjPStr] sequence
** @param [u] relist [AjPList] restriction enzymes
** @param [w] tailstr [AjPStr*] Report tail as a string
** @param [r] sname [const AjPStr] sequence names
** @param [r] RStotal [ajint] number of REs
** @param [r] begin [ajint] start position
** @param [r] end [ajint] end position
** @param [r] tshow [AjBool] show translated sequence
** @return [AjPList] hit list
** @@
******************************************************************************/

static AjPList silent_mismatch(const AjPStr sstr, AjPList relist,
			       AjPStr* tailstr,
			       const AjPStr sname, ajint RStotal,
                               ajint begin, ajint end,
			       AjBool tshow)
{
    PSilent res;
    AjPList results;
    AjPStr str;                          /*holds RS patterns*/
    AjPStr tstr;
    AjBool dummy = ajFalse;              /*need a bool for ajPatClassify*/
    ajint mm;                            /*number of mismatches*/
    ajint hits;                          /*number of pattern matches found*/
    ajint i;
    ajint aw;
    ajint start;
    AjPList patlist = NULL;            /*a list for pattern matches.
                                             a list of ..*/
    EmbPMatMatch match;                /*..AjMatMatch structures*/
    PRinfo rlp = NULL;
    AjPStr pep   = NULL;               /*string to hold protein*/
    AjPTrn table = NULL;               /*object to hold translation table*/

    AjBool rev = ajFalse;

    str   = ajStrNew();
    tstr  = ajStrNew();
    pep   = ajStrNew();
    table = ajTrnNewI(0);                /*0 stands for standard DNA-
                                         see fuzztran.acd*/
    results = ajListNew();
    start = 1;

    ajTrnSeqFrameS(table,sstr,1,&pep); /*1 stands for frame number*/
    if(tshow)
    {
        silent_fmt_sequence("TRANSLATED SEQUENCE",
                            pep,tailstr,start,ajFalse);
    }

    for(aw=0;aw<RStotal;aw++)          /* loop to read in restriction
					  enzyme patterns */
    {
	/* Pop off the first RE */
	ajListPop(relist,(void **)&rlp);
        /* ignore unknown cut sites and zero cutters */
       	if(*ajStrGetPtr(rlp->site)=='?'||!rlp->ncuts)
        {
	    ajListPushAppend(relist,(void*)rlp);
	    continue;
	}
	ajStrFmtUpper(&rlp->site);   /* convert all RS to upper case */
	ajStrFmtUpper(&rlp->revsite);
	ajStrAssignS(&str,rlp->site);      /* str now holds RS patterns */
        rev = ajFalse;

        while (str)
        {
            patlist = ajListNew();
            if(!embPatClassify(str,&str,
                               &dummy,&dummy,&dummy,&dummy,&dummy,&dummy,0))
                continue;

            mm = 0;
            hits=embPatBruteForce(sstr,str,ajFalse,ajFalse,patlist,begin+1,mm,
                                  sname);

            if(hits)
                while(ajListPop(patlist,(void**)&match))
                    embMatMatchDel(&match);
            else
            {
                mm = 1;
                hits = embPatBruteForce(sstr,str,ajFalse,ajFalse,patlist,
                                        begin+1,mm,sname);

                if(hits)
                    for(i=0;i<hits;++i)
                    {
                        ajListPop(patlist,(void**)&match);
                        res = silent_checktrans(sstr,match,rlp,begin,rev,end);
                        if (res)
                            ajListPushAppend(results,(void *)res);
                        embMatMatchDel(&match);
                    }
            }

            ajListFree(&patlist);

            if(!rev && !ajStrMatchS(rlp->site, rlp->revsite))
            {
                ajStrAssignS(&str,rlp->revsite);
                rev = ajTrue;
            }
            else
                ajStrDel(&str);
        }

        ajListPushAppend(relist,(void *)rlp);
    }
    
    ajStrDel(&str);
    ajStrDel(&tstr);
    ajStrDel(&pep);
    ajTrnDel(&table);
    return (results);
}




/* @funcstatic silent_restr_read **********************************************
**
** Read restriction enzyme data
**
** @param [w] relist [AjPList*] restriction enzyme data
** @param [r] enzymes [const AjPStr] restriction enzyme data to fetch
** @return [ajint] number of enzymes read
** @@
******************************************************************************/

static ajint silent_restr_read(AjPList *relist,const AjPStr enzymes)
{
    EmbPPatRestrict rptr = NULL;
    AjPFile fin = NULL;

    AjPStr refilename = NULL;
    register ajint RStotal = 0;
    PRinfo rinfo = NULL;
    AjBool isall = ajFalse;
    ajint ne = 0;
    ajint i;
    AjPStr *ea = NULL;

    refilename = ajStrNewC("REBASE/embossre.enz");
    rptr       = embPatRestrictNew();
    *relist    = ajListNew();

    fin = ajDatafileNewInNameS(refilename);
    if(!fin)
	ajFatal("Aborting...restriction file '%S' not found", refilename);

    /* Parse the user-selected enzyme list */
    if(!enzymes)
	isall = ajTrue;
    else
    {
	ne = ajArrCommaList(enzymes,&ea);
        for(i=0;i<ne;++i)
	    ajStrRemoveWhite(&ea[i]);

        if(ajStrMatchCaseC(ea[0],"all"))
            isall = ajTrue;
        else
            isall = ajFalse;
    }

    while(!ajFileIsEof(fin))
    {
        if(!embPatRestrictReadEntry(rptr,fin))
	    continue;

     	if(!isall)
	{
		for(i=0;i<ne;++i)
		if(ajStrMatchCaseS(ea[i],rptr->cod))
			break;
	    	if(i==ne)
			continue;
        }

        AJNEW(rinfo);
        /* reading in RE info into rinfo from EmbPPatRestrict structure */
        rinfo->code  = ajStrNewS(rptr->cod);
	rinfo->site  = ajStrNewS(rptr->pat);
	rinfo->revsite  = ajStrNewS(rptr->pat);
        ajSeqstrReverse(&rinfo->revsite);
        rinfo->ncuts = rptr->ncuts;
        rinfo->cut1  = rptr->cut1;
        rinfo->cut2  = rptr->cut2;
        rinfo->cut3  = rptr->cut3;
        rinfo->cut4  = rptr->cut4;
	ajListPush(*relist,(void *)rinfo);
	RStotal++;
    }

    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    AJFREE(ea);

    embPatRestrictDel(&rptr);
    ajFileClose(&fin);
    ajStrDel(&refilename);

    return RStotal;
}




/* @funcstatic silent_checktrans **********************************************
**
** Determine whether silent mutation exists
**
** @param [r] seq [const AjPStr] sequence
** @param [r] match [const EmbPMatMatch] pattern match
** @param [r] rlp [const PRinfo] RE information
** @param [r] begin [ajint] start position
** @param [r] rev [AjBool] do complement
** @param [r] end [ajint] end position
** @return [PSilent] silent mutation object or NULL if not found
** @@
******************************************************************************/

static PSilent silent_checktrans(const AjPStr seq,const EmbPMatMatch match,
				const PRinfo rlp, ajint begin,
				AjBool rev, ajint end)
{
    PSilent ret;
    const char *p = NULL;
    const char *q = NULL;
    const char *s = NULL;
    char *t;
    const char *u;
    ajint matchpos;

    ajint count;
    AjPTrn table = NULL;
    AjPStr s1 = NULL;
    AjPStr s2 = NULL;
    char c;
    char rc;
    ajint  min = INT_MAX;          /* Reverse sense intentional! */
    ajint  max = -INT_MAX;
    ajint fpos;
    ajint x;
    AjPStr tstr = NULL;

    matchpos = match->start;
    fpos = matchpos;

    tstr = ajStrNewS(seq);
    t = ajStrGetuniquePtr(&tstr);

    p = t+fpos-(begin+1);

    if(!rev)
        u = q = ajStrGetPtr(rlp->site);
    else
        u = q = ajStrGetPtr(rlp->revsite);

    /* Test here for whether cut site is within sequence substring */
    if(rlp->ncuts==4)
    {
	min = AJMIN(rlp->cut1,rlp->cut2);
	max = AJMAX(rlp->cut3,rlp->cut4);
    }
    else if(rlp->ncuts==2)
    {
	min = AJMIN(rlp->cut1,rlp->cut2);
	max = AJMAX(rlp->cut1,rlp->cut2);
    }
    else
    {
        ajWarn("Possibly corrupt RE file");
	ajStrDel(&tstr);
        return NULL;
    }

    if(matchpos+min<0||matchpos+max>end+1)
    {
        /*Cut site not in sequence range*/
        ajStrDel(&tstr);
        return NULL;
    }

    count=0;
    while(ajBaseAlphaToBin(*q++) & ajBaseAlphaToBin(*p++))
          ++count;

    /* Changed base postion */
    x = fpos+count-(begin+1);

    c  = t[x];
    rc = u[count];

    s = t+x-x%3;

    table = ajTrnNewI(0);

    /* translates codon pointed to by s (original seq) */
    s1 = ajStrNewK(ajTrnCodonC(table,s));

    t[x] = rc;

    /*  translates codon pointed to by s (mutated base from RS pattern */
    s2 = ajStrNewK(ajTrnCodonC(table,s));

    t[x] = c;  /* changes mutated base in seq back to original base */

    AJNEW(ret);
    ret->obase = c;
    ret->nbase = rc;
    ret->code  = ajStrNewS(rlp->code);

    if(!rev)
        ret->site  = ajStrNewS(rlp->site);
    else
        ret->site  = ajStrNewS(rlp->revsite);

    ret->seqaa = ajStrNewS(s1);
    ret->reaa  = ajStrNewS(s2);

    if(ajStrMatchS(s1,s2))
	ret->issilent = ajTrue;
    else
	ret->issilent = ajFalse;

    ret->match = matchpos;
    ret->base  = matchpos+count;

    ajStrDel(&tstr);
    ajStrDel(&s1);
    ajStrDel(&s2);
    ajTrnDel(&table);

    return ret;
}




/* @funcstatic silent_fmt_sequence ********************************************
**
** Output sequence information
**
** @param [r] title [const char*] Title (header) string
** @param [r] seq [const AjPStr] sequence
** @param [w] tailstr [AjPStr*] Generated footer string
** @param [r] start [ajint] start position
** @param [r] num [AjBool] Adjust numbering
** @@
******************************************************************************/

static void silent_fmt_sequence(const char* title,
				const AjPStr seq, AjPStr* tailstr, ajint start,
				AjBool num)
{
    const char *p;
    ajint m;
    ajint i;
    ajint tlen;

    ajFmtPrintAppS(tailstr,"%s:\n",title);
    if(num)
    {
    	p = ajStrGetPtr(seq);
    	ajFmtPrintAppS(tailstr,"%-7d",start);
    	tlen = ajStrGetLen(seq);
    	for(i=0; i<tlen ; i++)
    	{
		ajFmtPrintAppS(tailstr,"%c",p[i]);
        	m=i+1;
        	if(m%10==0)
		    ajFmtPrintAppS(tailstr," ");
		if(m%60==0 && m<tlen)
		    ajFmtPrintAppS(tailstr,"\n%-7d",(start+m+1));
    	}
    }
    else
    {
	p = ajStrGetPtr(seq);
        tlen = ajStrGetLen(seq);
        for(i=0; i<tlen ; i++)
        {
                ajFmtPrintAppS(tailstr,"%c",p[i]);
                m=i+1;
                if(m%10==0)
		    ajFmtPrintAppS(tailstr," ");
                if(m%60==0 && m<tlen)
		    ajFmtPrintAppS(tailstr,"\n");
        }
    }

    ajFmtPrintAppS(tailstr,"\n\n");

    return;
}




/* @funcstatic silent_fmt_hits ************************************************
**
** Output silent mutation information
**
** @param [u] hits [AjPList] results
** @param [u] feat [AjPFeattable] Feature table object
** @param [r] silent [AjBool] Silent mutation
** @@
******************************************************************************/

static void silent_fmt_hits(AjPList hits, AjPFeattable feat,
			    AjBool silent)
{
    PSilent res;
    AjPFeature sf = NULL;
    AjPStr tmpFeatStr = NULL;

    ajListSort(hits,silent_basecompare);

    while(ajListPop(hits,(void **)&res))
    {
        sf = ajFeatNewII(feat,
                         res->match, res->match+ajStrGetLen(res->site)-1);

	if (silent)
	{
	    ajFmtPrintS(&tmpFeatStr, "*silent %B", res->issilent);
	    ajFeatTagAddSS(sf, NULL, tmpFeatStr);
	}
	ajFmtPrintS(&tmpFeatStr, "*enzyme %S", res->code);
	ajFeatTagAddSS(sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*rspattern %S", res->site);
	ajFeatTagAddSS(sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*baseposn %d", res->base);
	ajFeatTagAddSS(sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*aa %S.%S", res->seqaa, res->reaa);
	ajFeatTagAddSS(sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*mutation %c->%c", res->obase,res->nbase);
	ajFeatTagAddSS(sf, NULL, tmpFeatStr);
	
       ajStrDel(&res->code);
       ajStrDel(&res->site);
       ajStrDel(&res->seqaa);
       ajStrDel(&res->reaa);
       AJFREE(res);
    }

    ajStrDel(&tmpFeatStr);
    return;
}




/* @funcstatic silent_split_hits **********************************************
**
** Divide hitlist into separate silent/non-silent lists
**
** @param [u] hits [AjPList*] hitlist
** @param [u] silents [AjPList*] silent hits
** @param [u] nonsilents [AjPList*] non-silent hits
** @param [r] allmut [AjBool] do non-silents
** @@
******************************************************************************/

static void silent_split_hits(AjPList *hits, AjPList *silents,
			      AjPList *nonsilents, AjBool allmut)
{
    PSilent res;

    while(ajListPop(*hits,(void **)&res))
    {
	if(res->issilent)
	{
           	ajListPushAppend(*silents,(void *)res);
	   	continue;
	}
	if(allmut)
           	ajListPushAppend(*nonsilents,(void *)res);
	else
	{
	    ajStrDel(&res->code);
	    ajStrDel(&res->site);
	    ajStrDel(&res->seqaa);
	    ajStrDel(&res->reaa);
	    AJFREE(res);
	}
    }

    return;
}




/* @funcstatic silent_basecompare *********************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint silent_basecompare(const void *a, const void *b)
{
    return((*(PSilent const *)a)->base)-((*(PSilent const *)b)->base);
}
