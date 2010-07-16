/* @source recoder
**
** Find restriction sites in a nucleotide sequence and remove
** them whilst maintaining the same translation.
**
**
** This program uses a similar output style to that of
** "silent". Also, the enzyme reading function was taken
** straight out of "silent".
**
** @author Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
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

#define IUBFILE "Ebases.iub"




/* @datastatic PRinfo *******************************************************
**
** recoder internals for RE information
**
** @alias SRinfo
** @alias ORinfo
**
** @attr code [AjPStr] Undocumented
** @attr site [AjPStr] Undocumented
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
    ajint ncuts;
    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    char Padding[4];
} ORinfo;
#define PRinfo ORinfo*




/* @datastatic PMutant *********************************************************
**
** recoder internals for mutation sites
**
** @alias SMutant
** @alias OMutant
**
** @attr code [AjPStr] Undocumented
** @attr site [AjPStr] Undocumented
** @attr match [ajint] Undocumented
** @attr base [ajint] Undocumented
** @attr seqaa [AjPStr] Undocumented
** @attr reaa [AjPStr] Undocumented
** @attr obase [char] Undocumented
** @attr nbase [char] Undocumented
** @attr Padding [char[6]] Padding to alignment boudnary
******************************************************************************/

typedef struct SMutant
{
    AjPStr code;
    AjPStr site;
    ajint match;
    ajint base;
    AjPStr seqaa;
    AjPStr reaa;
    char   obase;
    char   nbase;
    char   Padding[6];
} OMutant;
#define PMutant OMutant*


static AjPTrn recoderTable = NULL;               /* translation table object */


static ajint  recoder_readRE(AjPList *relist, const AjPStr enzymes);
static AjPList recoder_rematch(const AjPStr sstr, AjPList ressite,
			       AjPStr* tailstr,
			       const AjPStr sname, ajint RStotal, ajint radj,
			       AjBool rev, ajint begin, ajint end,
			       AjBool tshow);
static AjPList recoder_checkTrans(const AjPStr seq,const EmbPMatMatch match,
				  const PRinfo rlp, ajint begin, ajint radj,
				  AjBool rev, ajint end, ajint pos,
				  AjBool* empty);
static AjBool recoder_checkPat(const EmbPMatMatch match,
			       const PRinfo rlp, ajint radj, AjBool rev,
			       ajint begin, ajint end);
static ajint recoder_changebase(char pbase, char* tbase);
static void  recoder_mutFree(PMutant* mut);
static ajint recoder_basecompare(const void *a, const void *b);

static void recoder_fmt_seq(const char* title, const AjPStr seq,
			    AjPStr* tailstr,
			    ajint start, AjBool num);
static void recoder_fmt_muts(AjPList muts, AjPFeattable feat, AjBool rev);




/* @prog recoder **************************************************************
**
** Remove restriction sites but maintain the same translation
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq   = NULL;
    AjPReport report = NULL;
    AjPFeattable feat=NULL;

    AjPStr sstr  = NULL;
    const AjPStr sname = NULL;	     /* seq name */
    AjPStr revcomp = NULL;	     /* rev complement of seq */
    AjPStr enzymes = NULL;           /* string for RE selection */

    ajint RStotal;
    ajint begin;                     /* specified start by user */
    ajint end;                       /* specified end by user */
    ajint radj;
    ajint start;
    AjBool sshow;
    AjBool tshow;

    AjPList relist = NULL;
    AjPList muts;
    AjPList nmuts;
    PRinfo re;
    AjPStr tailstr = NULL;

    embInit("recoder", argc, argv);

    seq     = ajAcdGetSeq("sequence");          /* sequence to investigate */
    enzymes = ajAcdGetString("enzymes");   /* enzyme list             */
    sshow   = ajAcdGetBoolean("sshow");       /* display seq             */
    tshow   = ajAcdGetBoolean("tshow");       /* display translated seq  */
    report = ajAcdGetReport ("outfile");     /* report filename         */


    RStotal=recoder_readRE(&relist,enzymes);      /* read in RE info */

    ajDebug("readRE read %d listlen:%d\n", RStotal, ajListGetLength(relist));

    begin = ajSeqGetBegin(seq);              /* seq start posn, or 1     */
    end   = ajSeqGetEnd(seq);                /* seq end posn, or seq len */
    radj  = begin+end+1;                  /* posn adjustment for compl seq */

    /* --begin and --end to convert counting from 0-N, not 1-N */
    ajStrAssignSubC(&sstr,ajSeqGetSeqC(seq),--begin,--end);
    ajStrFmtUpper(&sstr);

    sname = ajSeqGetNameS(seq);

    ajStrAssignC(&revcomp,ajStrGetPtr(sstr));	 /* copying seq into revcomp */
    ajSeqstrReverse(&revcomp);		 /* getting rev complement   */
    start = begin+1;

    feat = ajFeattableNewDna(ajSeqGetNameS(seq));

    if(sshow)
    {
        recoder_fmt_seq("SEQUENCE",
			sstr,&tailstr,start,ajTrue);
	recoder_fmt_seq("REVERSE COMPLEMENT SEQUENCE",
			revcomp,&tailstr,start,ajTrue);
    }

    /******* get de-restriction site *******/
    /******* forward strand          *******/
    muts = recoder_rematch(sstr,relist,&tailstr,sname,RStotal,
			   radj,ajFalse,begin,end,tshow);


    ajReportSetHeaderC(report,
		       "KEY:\n"
		       "Dir: Direction (Rev for reverse complement)\n"
		       "EnzymeName: Enzyme name\n"
		       "RS-Pattern: Restriction enzyme recognition site "
		       "pattern\n"
		       "Base-Posn: Position of base to be mutated\n"
		       "AAs: Amino acid. Original sequence(.)After mutation\n"
		       "Mutation: The base mutation to perform\n\n"
		       "Creating silent mutations");
    recoder_fmt_muts(muts,feat, ajFalse);


    /******* reverse strand ****************/
    nmuts = recoder_rematch(revcomp,relist,&tailstr,sname,RStotal,
			    radj,ajTrue,begin,end,tshow);

    recoder_fmt_muts(nmuts,feat, ajTrue);

    ajReportSetTailS(report, tailstr);
    ajReportSetStatistics(report, 1, ajSeqGetLenTrimmed(seq));
    (void) ajReportWrite (report,feat,seq);
    ajFeattableDel(&feat);

    while(ajListPop(relist,(void **)&re))
    {
	ajStrDel(&re->code);
	ajStrDel(&re->site);
	AJFREE(re);
    }
    ajListFree(&relist);

    ajSeqDel(&seq);
    ajStrDel(&revcomp);
    ajStrDel(&enzymes);
    ajStrDel(&sstr);
    ajStrDel(&tailstr);

    ajListFree(&muts);
    ajListFree(&nmuts);

    ajReportClose(report);
    ajReportDel(&report);
    ajTrnDel(&recoderTable);

    embExit();

    return 0;
}




/* @funcstatic recoder_rematch ************************************************
**
** Looks for RE matches and test new bases to find those that
** give the same translation.
**
** @param [r] sstr [const AjPStr] Search sequence as a string
** @param [u] relist [AjPList] Regular expression list
** @param [w] tailstr [AjPStr*] Report tail as a string
** @param [r] sname [const AjPStr] Sequence name
** @param [r] RStotal [ajint] Restriction sites
** @param [r] radj [ajint] Position adjustment for reversed sequence
** @param [r] rev [AjBool] Reverse sequence
** @param [r] begin [ajint] Start position
** @param [r] end [ajint] End position
** @param [r] tshow [AjBool] Show translation
** @return [AjPList] Mutant list of those RS sites removed but
**                   maintaining same translation.
******************************************************************************/

static AjPList recoder_rematch(const AjPStr sstr, AjPList relist,
			       AjPStr* tailstr,
			       const AjPStr sname, ajint RStotal, ajint radj,
			       AjBool rev, ajint begin, ajint end,
			       AjBool tshow)
{
    AjPList res;
    AjPList results;
    AjPStr str;                        /* holds RS patterns */
    AjPStr tstr;
    AjPStr pep = NULL;                 /* string to hold protein */

    AjBool dummy=ajFalse;              /* need a bool for ajPatClassify */
    AjBool empty;

    ajint mm;                            /* no. of mismatches */
    ajint pats;                          /* no. of pattern matches */
    ajint patlen;
    ajint pos;
    ajint aw;
    ajint start;

    AjPList patlist = NULL;            /* list for pattern matches of.. */
    EmbPMatMatch match;                /* ..AjMatMatch structures*/
    PRinfo rlp = NULL;

    str   = ajStrNew();
    tstr  = ajStrNew();
    pep   = ajStrNew();
    if(!recoderTable)
	recoderTable = ajTrnNewI(0);      /* 0 for std DNA (see fuzztran) */

    results = ajListNew();
    start = 1;

    if(!rev)                           /* forward strand */
    {
	ajTrnSeqFrameS(recoderTable,sstr,1,&pep); /* frame 1 */
    	if(tshow)
    	{
            recoder_fmt_seq("TRANSLATED SEQUENCE",
			    pep,tailstr,start,ajFalse);
     	}
    }
    else                               /* reverse strand */
    {
	ajStrAssignC(&tstr,ajStrGetPtr(sstr)+(end-begin+1)%3);
	ajTrnSeqFrameS(recoderTable,tstr,1,&pep);
        if(tshow)
	{
	     recoder_fmt_seq("REVERSE TRANSLATED SEQUENCE",
			     pep,tailstr,start,ajFalse);
	}
    }


    for(aw=0;aw<RStotal;aw++)          /* read in RE patterns */
    {
	                               /* pop off first RE */
	(void) ajListPop(relist,(void **)&rlp);
        /* ignore unknown cut sites & zero cutters */
       	if(*ajStrGetPtr(rlp->site)=='?'||!rlp->ncuts)
        {
       	     ajListPushAppend(relist,(void*)rlp);
       	     continue;
        }
        ajStrFmtUpper(&rlp->site);          /* RS to upper case */
        ajStrAssignS(&str,rlp->site);          /* str holds RS pat */

	patlist = ajListNew();             /* list for matches */

	/* convert our RS pattern to a reg expression    */
	/* "0" means DNA & so does any ambig conversions */

        if(!embPatClassify(str,&str,&dummy,&dummy,&dummy,
                    &dummy,&dummy,&dummy,0)) continue;

	/* find pattern matches in seq with NO mismatches */
	mm = 0;
	pats = embPatBruteForce(sstr,str,ajFalse,ajFalse,
				patlist,begin+1,mm,sname);

	ajDebug("Pattern '%S' pats: %d list: %u\n",
		str, pats, ajListGetLength(patlist));
	if(pats)
        {
            while(ajListPop(patlist,(void**)&match))
            {
              patlen = match->len;

              if(recoder_checkPat(match,rlp,radj,
				  rev,begin,end))
              {
                  for(pos=0;pos<patlen;pos++)
                  {
                    res = recoder_checkTrans(sstr,match,rlp,begin,
					     radj,rev,end,pos,&empty);
                    if(empty)
                        ajListFree(&res);
                    else
                        ajListPushlist(results,&res);
                  }
              }
              embMatMatchDel(&match);
            }
        }

        /* push RE info back to top of the list */
        ajListPushAppend(relist,(void *)rlp);
	ajListFree(&patlist);
    }

    ajStrDel(&str);
    ajStrDel(&tstr);
    ajStrDel(&pep);

    return results;
}




/* @funcstatic recoder_readRE *************************************************
**
** Read in RE information from REBASE file.
**
** @param [w] relist [AjPList*] returns restriction site information as a list
** @param [r] enzymes [const AjPStr] Selected enzymes to read
** @return [ajint] Number of restriction sites in list
**
******************************************************************************/
static ajint recoder_readRE(AjPList *relist, const AjPStr enzymes)
{
    EmbPPatRestrict rptr = NULL;	/* store RE info */
    AjPFile fin = NULL;			/* file pointer to RE file data */
    AjPStr refilename = NULL;		/* .. & string for the filename */
    register ajint RStotal = 0;		/* counts no of RE */
    PRinfo rinfo = NULL;
    AjBool isall = ajFalse;
    ajint ne = 0;
    ajint i;
    AjPStr *ea = NULL;

    refilename = ajStrNewC("REBASE/embossre.enz");

    rptr = embPatRestrictNew();         /* allocate a restrict struc */

    *relist = ajListNew();              /* list the RS code and info */

    fin = ajDatafileNewInNameS(refilename);
    if(!fin)
	ajFatal("Aborting...restriction file not found");

    if(!enzymes)                         /* parse user-selected enzyme list */
	isall = ajTrue;
    else
    {
	ne=ajArrCommaList(enzymes,&ea);  /* no. of RE's specified */
                                         /* ea points to enzyme list */
        for(i=0;i<ne;++i)
	    ajStrRemoveWhite(&ea[i]);     /* remove all whitespace */

        if(ajStrMatchCaseC(ea[0],"all"))
            isall = ajTrue;
        else
            isall = ajFalse;
    }
    ajDebug("recoder_readRE enzymes: '%S' isall:%B\n", enzymes, isall);


    /* read RE data into AjPRestrict obj */

    while(!ajFileIsEof(fin))
    {
        if(!embPatRestrictReadEntry(rptr,fin))
	    continue;

     	if(!isall)           /* only select enzymes on command line */
	{
	     for(i=0;i<ne;++i)
		if(ajStrMatchCaseS(ea[i],rptr->cod))
			break;

	     if(i==ne)
		continue;
        }


        AJNEW(rinfo);
        rinfo->code  = ajStrNewC(ajStrGetPtr(rptr->cod));
  	rinfo->site  = ajStrNewC(ajStrGetPtr(rptr->pat));
        rinfo->ncuts = rptr->ncuts;
        rinfo->cut1  = rptr->cut1;
        rinfo->cut2  = rptr->cut2;
        rinfo->cut3  = rptr->cut3;
        rinfo->cut4  = rptr->cut4;
	ajListPush(*relist,(void *)rinfo);
	RStotal++;
	ajDebug("Creating RStotal:%d listsize:%u '%S' '%S'\n",
		RStotal, ajListGetLength(*relist), rptr->cod, rptr->pat);
    }

    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    AJFREE(ea);
    embPatRestrictDel(&rptr);
    ajFileClose(&fin);
    ajStrDel(&refilename);

    return RStotal;
}




/* @funcstatic recoder_checkPat ***********************************************
**
** Checks whether the RS pattern falls within the sequence string
**
** @param [r] match [const EmbPMatMatch] Match data
** @param [r] rlp [const PRinfo] Restriction site info
** @param [r] radj [ajint] Adjustment for reversed sequence
** @param [r] rev [AjBool] Reverse sequence
** @param [r] begin [ajint] Start position
** @param [r] end [ajint] End position
** @return [AjBool] ajTrue if the pattern is found
**
******************************************************************************/
static AjBool recoder_checkPat(const EmbPMatMatch match,
			       const PRinfo rlp, ajint radj, AjBool rev,
			       ajint begin, ajint end)
{
    ajint mpos;

    ajint min = INT_MAX;             /* reverse sense intentional! */
    ajint max = -INT_MAX;


    mpos  = match->start;         /* start posn of match in seq */

    if(rlp->ncuts==4)             /* test if cut site is within seq */
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
        return ajFalse;
    }

    if(!rev)                      /* forward strand */
    {
        if(mpos+min<0||mpos+max>end+1)
             return ajFalse;     /* cut not in seq */
    }
    else                         /* reverse strand */
    {
        if(radj-mpos-1-min>end+1||radj-mpos-1-max<begin)
             return ajFalse;     /* cut not in seq */
    }

    return ajTrue;
}




/* @funcstatic recoder_checkTrans *********************************************
**
** Identify mutations at a site in the RS pattern that result in the
** same translation.
**
** @param [r] dna [const AjPStr] Sequence as a string
** @param [r] match [const EmbPMatMatch] Match data
** @param [r] rlp [const PRinfo] Restriction site info
** @param [r] begin [ajint] Start position
** @param [r] radj [ajint] Adjustment for reversed sequence
** @param [r] rev [AjBool] Reverse sequence
** @param [r] end [ajint] End position
** @param [r] pos [ajint] Base position in site
** @param [w] empty [AjBool*] ajTrue if the list is empty
** @return [AjPList] List of de-restricted sites which maintain same
**                   translation.
**
******************************************************************************/

static AjPList recoder_checkTrans(const AjPStr dna, const EmbPMatMatch match,
				  const PRinfo rlp, ajint begin, ajint radj,
				  AjBool rev, ajint end, ajint pos,
				  AjBool* empty)
{
    char *pseq;
    char *pseqm;
    const char *prs;
    char *s;

    PMutant  tresult;
    AjPList res;

    ajint mpos;
    ajint framep;
    ajint i;
    AjPStr s1 = NULL;
    AjPStr s2 = NULL;
    char base;
    char pbase;
    char tbase[4];

    ajint  rmpos;
    ajint  x;
    ajint  nb;

    AjPStr seq = NULL;

    seq = ajStrNewS(dna);

    *empty = ajTrue;
    mpos  = match->start;         /* start posn of match in seq */
    rmpos = radj-mpos-match->len; /* start posn of match in rev seq */

    pseq  = ajStrGetuniquePtr(&seq);        /* pointer to start of seq */
    pseqm = pseq+mpos-(begin+1);  /* pointer to start of match in seq */
    prs   = ajStrGetPtr(rlp->site);  /* pointer to start of RS pattern */

    framep=(end-begin+1)%3;       /* where frame starts on reverse strand */

    base  = pseqm[pos];           /* store orig seq base */
    pbase = prs[pos];

                                  /* use IUB codes to get other bases */
    nb = recoder_changebase(pbase,&tbase[0]);

    x = mpos+pos-(begin+1);

    if(!rev)                      /* forward strand */
      s = pseq+x-x%3;
    else                          /* reverse strand */
      s = pseq+x-(x-framep)%3;

    if(!recoderTable)
	recoderTable = ajTrnNewI(0);
    s1 = ajStrNewK(ajTrnCodonC(recoderTable,s));

    res=ajListNew();


    for(i=0;i<nb;i++)             /* try out other bases */
    {
      pseq[x] = tbase[i];
      s2 = ajStrNewK(ajTrnCodonC(recoderTable,s));

      if(ajStrMatchS(s1,s2))
      {
	  /* if same translation */
          AJNEW(tresult);
          tresult->obase = base;
          tresult->nbase = tbase[i];
          tresult->code  = ajStrNewC(ajStrGetPtr(rlp->code));
          tresult->site  = ajStrNewC(ajStrGetPtr(rlp->site));
          tresult->seqaa = ajStrNewC(ajStrGetPtr(s1));
          tresult->reaa  = ajStrNewC(ajStrGetPtr(s2));

          if(!rev)
          {
       	    tresult->match = mpos;
            tresult->base  = mpos+pos;
          }
          else
          {
            tresult->match = rmpos;
            tresult->base  = rmpos+match->len-1-pos;
          }
          ajListPushAppend(res,(void *)tresult);
          *empty = ajFalse;
      }
      ajStrDel(&s2);
    }

    pseq[x] = base;          /* resubstitute orig base */

    ajStrDel(&s1);
    ajStrDel(&seq);

    return res;
}




/* @funcstatic recoder_changebase *********************************************
**
** Use IUB code to return alternative nucleotides to that provided
** which result in the same translation.
**
** @param [r] pbase [char] Base
** @param [w] tbase [char*] C string with alternative bases
** @return [ajint] number of bases stored in tbase
**
******************************************************************************/

static ajint recoder_changebase(char pbase, char* tbase)
{
    ajint setBase[] =
    {
	1,1,1,1
    };
    AjIStr splits = NULL;
    const AjPStr bt = NULL;
    char bs;
    ajint i;
    ajint nb;

    bt = ajBaseGetCodes((ajint)pbase);
    splits = ajStrIterNew(bt);

    while(!ajStrIterDone(splits))
    {
      bs = ajStrIterGetK(splits);

      if( ajBaseAlphaToBin(bs) & ajBaseAlphaToBin('G') )
	  setBase[0] = 0;
      if( ajBaseAlphaToBin(bs) & ajBaseAlphaToBin('A') )
	  setBase[1] = 0;
      if( ajBaseAlphaToBin(bs) & ajBaseAlphaToBin('T') )
	  setBase[2] = 0;
      if( ajBaseAlphaToBin(bs) & ajBaseAlphaToBin('C') )
	  setBase[3] = 0;

      ajStrIterNext(splits);
    }

    ajStrIterDel(&splits);

    nb = 0;
    for(i=0;i<4;i++)
    {
      if( setBase[i] == 1 )
      {
        if(i==0)
           tbase[nb] = 'G';
        else if(i==1)
           tbase[nb] = 'A';
        else if(i==2)
           tbase[nb] = 'T';
        else if(i==3)
           tbase[nb] = 'C';
        nb++;
      }
    }

    return nb;
}




/* @funcstatic recoder_fmt_seq ************************************************
**
** Write sequence to the output file.
**
** @param [r] title [const char*] Title for sequence report
** @param [r] seq [const AjPStr] Sequence as a string
** @param [w] tailstr [AjPStr*] Report tail as a string
** @param [r] start [ajint] Start position
** @param [r] num [AjBool] Numbered sequence
** @return [void]
**
******************************************************************************/

static void recoder_fmt_seq(const char* title, const AjPStr seq,
			    AjPStr* tailstr,
			    ajint start, AjBool num)
{
    const char *p;
    ajint m;
    ajint i;
    ajint tlen;

    ajFmtPrintAppS(tailstr,"%s:\n",title);
    if(num)
    {
    	p=ajStrGetPtr(seq);
    	ajFmtPrintAppS(tailstr,"%-7d",start);
    	tlen=ajStrGetLen(seq);
    	for(i=0; i<tlen ; i++)
    	{
	    ajFmtPrintAppS(tailstr,"%c",p[i]);
	    m=i+1;
	    if(m%10==0)
		ajFmtPrintAppS(tailstr," ");
	    if(m%60==0 && m<tlen)
		ajFmtPrintAppS(tailstr,"\n%-7d",(start+m));
    	}
    }
    else
    {
	p=ajStrGetPtr(seq);
        tlen=ajStrGetLen(seq);
        for(i=0; i<tlen ; i++)
        {
	    ajFmtPrintAppS(tailstr,"%c",p[i]);
	    m=i+1;
	    if(m%10==0)
		ajFmtPrintAppS(tailstr," ");
	    if(m%60==0)
		ajFmtPrintAppS(tailstr,"\n");
        }
    }

    ajFmtPrintAppS(tailstr,"\n\n");

    return;
}




/* @funcstatic recoder_fmt_muts ***********************************************
**
** Write de-restricted sites to feature table
**
** @param [u] muts [AjPList] List of derestricted sites
** @param [u] feat [AjPFeattable] Feature table object
** @param [r] rev [AjBool] Reverse direction
** @return [void]
******************************************************************************/

static void recoder_fmt_muts(AjPList muts, AjPFeattable feat, AjBool rev)
{
    PMutant res;
    AjPFeature sf = NULL;
    AjPStr tmpFeatStr = NULL;

    ajListSort(muts,recoder_basecompare);

    while(ajListPop(muts,(void **)&res))
    {
	if (rev)
	{
	    sf = ajFeatNewIIRev(feat,
				res->match, res->match+ajStrGetLen(res->site)-1);
	    ajFmtPrintS(&tmpFeatStr, "*dir Rev");
	    ajFeatTagAdd (sf, NULL, tmpFeatStr);
	}
	else
	{
	    sf = ajFeatNewII(feat,
			     res->match, res->match+ajStrGetLen(res->site)-1);
	}
	ajFmtPrintS(&tmpFeatStr, "*enzyme %S", res->code);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*rspattern %S", res->site);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*baseposn %d", res->base);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*aa %S.%S", res->seqaa, res->reaa);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*mutation %c->%c", res->obase,res->nbase);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	
	recoder_mutFree(&res);
    }

    ajStrDel(&tmpFeatStr);
    return;
}




/* @funcstatic recoder_basecompare ********************************************
**
** Compare 2 base positions in the nucleotide sequence
**
** @param [r] a [const void *] First base
** @param [r] b [const void *] Second base
** @return [ajint] Comparison result
******************************************************************************/

static ajint recoder_basecompare(const void *a, const void *b)
{
    return((*(PMutant const *)a)->base)-((*(PMutant const *)b)->base);
}




/* @funcstatic recoder_mutFree ************************************************
**
** Free allocated memory for mutant structure
**
** @param [d] mut [PMutant*] Mutant structure to be deleted
** @return [void]
******************************************************************************/

static void recoder_mutFree(PMutant* mut)
{
    ajStrDel(&(*mut)->code);
    ajStrDel(&(*mut)->site);
    ajStrDel(&(*mut)->seqaa);
    ajStrDel(&(*mut)->reaa);
    AJFREE(*mut);

    return;
}
