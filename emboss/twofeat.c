/* @source twofeat application
**
** Finds neighbouring pairs of features in sequences
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




/* @datastatic PHit ***********************************************************
**
** twofeat internals
**
** @alias SHit
** @alias OHit
**
** @attr gfA [AjPFeature] Undocumented
** @attr gfB [AjPFeature] Undocumented
** @attr Start [ajint] Undocumented
** @attr End [ajint] Undocumented
** @attr distance [ajint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct SHit
{
    AjPFeature gfA;
    AjPFeature gfB;
    ajint Start;
    ajint End;
    ajint distance;	   
    char  Padding[4];
} OHit;
#define PHit OHit*


 	  

static void twofeat_rippledown(const AjPFeattable tabA,
			       const AjPFeattable tabB,
			       ajint overlapi, ajint minrange,
			       ajint maxrange, ajint
			       rangetypei, ajint sensei,
			       AjPFeattable outtab,
			       AjBool twoout, const AjPStr typeout);
static AjBool twofeat_check_match(AjPFeature gfA, AjPFeature gfB,
				  PHit *detail, ajint overlapi,
				  ajint minrange, ajint maxrange, ajint
				  rangetypei, ajint sensei);
static void twofeat_report_hits(const AjPList hitlist, AjBool twoout,
				const AjPStr typeout, AjPFeattable outtab);
static void twofeat_find_features(const AjPSeq seq, AjPFeattable tab,
				  ajint begin, ajint end, const AjPStr source,
				  const AjPStr type, ajint sense,
				  float minscore,  float maxscore,
				  const AjPStr tag, const AjPStr value);
static AjBool twofeat_MatchFeature(const AjPFeature gf,
				   const AjPStr source, const AjPStr type,
				   ajint sense, float minscore,
				   float maxscore,
				   const AjPStr tag, const AjPStr value,
				   AjBool *tagsmatch);
static AjBool twofeat_MatchPatternTags(const AjPFeature feat,
				       const AjPStr tpattern,
				       const AjPStr vpattern);
static PHit twofeat_HitsNew(void);
static void twofeat_HitsDel(PHit *pthis);
static ajint twofeat_get_overlap_type(const AjPStr overlap);
static ajint twofeat_get_range_type(const AjPStr rangetype);
static ajint twofeat_get_sense_type(const AjPStr sense);
/* static ajint twofeat_get_order_type(const AjPStr order); */




/********* relation criterion types *****************/
enum OVERLAP_TYPE {OV_ANY, OV_OVERLAP, OV_NOTOVER, OV_NOTIN, OV_AIN, OV_BIN};
enum RANGE_TYPE {RA_NEAREST, RA_LEFT, RA_RIGHT, RA_FURTHEST};
enum SENSE_TYPE {SN_ANY, SN_SAME, SN_OPPOSITE};
enum ORDER_TYPE {OR_ANY, OR_AB, OR_BA};




/* @prog twofeat **************************************************************
**
** Finds neighbouring pairs of features in sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* input */
    AjPSeqall seqall;

    /* feature a */
    AjPStr asource;
    AjPStr atype;
    AjPStr asense;
    ajint  asensei;
    ajint aminscore;
    ajint amaxscore;
    AjPStr atag;
    AjPStr avalue;

    /* feature b */
    AjPStr bsource;
    AjPStr btype;
    AjPStr bsense;
    ajint  bsensei;
    ajint bminscore;
    ajint bmaxscore;
    AjPStr btag;
    AjPStr bvalue;

    /* relation */ 
    AjPStr overlap;
    ajint minrange;
    ajint maxrange;
    AjPStr rangetype;
    AjPStr sense;
    AjPStr order;
    ajint overlapi;
    ajint rangetypei;
    ajint sensei;

    /* output */
    AjBool twoout;
    AjPStr typeout;
    AjPReport report = NULL;


    AjPSeq seq;
    AjPFeattable tabA = NULL;
    AjPFeattable tabB = NULL;
    AjPFeattable outtab = NULL;

    AjPStr seqname = NULL;
    
    ajint    begin;
    ajint    end;
    float tf;
    

    embInit("twofeat", argc, argv);

    /* input */    
    seqall   = ajAcdGetSeqall("sequence");

    /* feature a */
    asource   = ajAcdGetString("asource");
    atype     = ajAcdGetString("atype");
    asense    = ajAcdGetListSingle("asense");

    tf = ajAcdGetFloat("aminscore");
    aminscore = (ajint) tf;
    tf = ajAcdGetFloat("amaxscore");
    amaxscore = (ajint) tf;
    
    atag      = ajAcdGetString("atag");
    avalue    = ajAcdGetString("avalue");
    
    /* feature b */
    bsource   = ajAcdGetString("bsource");
    btype     = ajAcdGetString("btype");
    bsense    = ajAcdGetListSingle("bsense");

    tf = ajAcdGetFloat("bminscore");
    bminscore = (ajint) tf;
    tf = ajAcdGetFloat("bmaxscore");
    bmaxscore = (ajint) tf;

    btag      = ajAcdGetString("btag");
    bvalue    = ajAcdGetString("bvalue");

    /* relation */
    overlap   = ajAcdGetListSingle("overlap"); 
    minrange  = ajAcdGetInt("minrange");
    maxrange  = ajAcdGetInt("maxrange");
    rangetype = ajAcdGetListSingle("rangetype");
    sense     = ajAcdGetListSingle("sense");
    order     = ajAcdGetListSingle("order");
    
    /* output */
    twoout   = ajAcdGetToggle("twoout");
    typeout  = ajAcdGetString("typeout");
    report   = ajAcdGetReport("outfile");

    /* convert feature sense to integer */
    if(ajStrMatchC(asense, "+"))
    	asensei = +1;
    else if(ajStrMatchC(asense, "+"))
    	asensei = -1;
    else
    	asensei = 0;


    if(ajStrMatchC(bsense, "+"))
    	bsensei = +1;
    else if(ajStrMatchC(bsense, "+"))
    	bsensei = -1;
    else
    	bsensei = 0;


    /* convert relation codes to integer values */
    overlapi = twofeat_get_overlap_type(overlap);
    rangetypei = twofeat_get_range_type(rangetype);
    sensei = twofeat_get_sense_type(sense);
    /* orderi = twofeat_get_order_type(order); Unused */

    seqname = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {

	ajStrAssignC(&seqname, ajSeqGetNameC(seq));
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);

	/* make new feature table for A */
        if(!tabA)
	    tabA = ajFeattableNewSeq(seq);

	/* make new feature table for B */
        if(!tabB)
	    tabB = ajFeattableNewSeq(seq);


	/* go through seq's features adding those that match A to table A */
        twofeat_find_features(seq, tabA, begin, end, asource, atype, asensei,
			      (float)aminscore, (float)amaxscore, atag,
			      avalue);
        
	/* go through seq's features adding those that match B to table B */
        twofeat_find_features(seq, tabB, begin, end, bsource, btype, bsensei,
			      (float)bminscore, (float)bmaxscore, btag,
			      bvalue);


        ajDebug("No of hits in tabA: %d\n", ajFeattableGetSize(tabA));
        ajDebug("No of hits in tabB: %d\n", ajFeattableGetSize(tabB));

	if(ajFeattableGetSize(tabA) && ajFeattableGetSize(tabB))
	{
	    /* initialise the output feature table */
            outtab = ajFeattableNewSeq(seq);


            /*
	    **  find pairs of hits within the required distance and output
	    **  the results
	    */
	    twofeat_rippledown(tabA, tabB, overlapi, minrange, maxrange,
			       rangetypei, sensei, outtab, twoout,
			       typeout);

	    /* write features and tidy up */
	    ajReportWrite(report, outtab, seq);        
	    ajFeattableDel(&outtab);
	}
	

        ajDebug("ajFeattableDel(&tabA)\n");	 
	ajFeattableDel(&tabA);
        ajDebug("ajFeattableDel(&tabB)\n");	 
	ajFeattableDel(&tabB);
    }
    ajReportSetSeqstats(report, seqall);

    ajStrDel(&seqname);
    ajSeqDel(&seq);
    ajSeqallDel(&seqall);

    ajStrDel(&asource);
    ajStrDel(&atype);
    ajStrDel(&asense);
    ajStrDel(&atag);
    ajStrDel(&avalue);

    ajStrDel(&bsource);
    ajStrDel(&btype);
    ajStrDel(&bsense);
    ajStrDel(&btag);
    ajStrDel(&bvalue);

    ajStrDel(&overlap);
    ajStrDel(&rangetype);
    ajStrDel(&sense);
    ajStrDel(&order);
    ajStrDel(&typeout);

    ajReportClose(report);
    ajReportDel(&report);

    embExit();

    return 0;
}




/* @funcstatic twofeat_rippledown ********************************************
**
** Go down the two lists of matches looking for hits within the required
** maximum distance.
** 
** Foreach feature in tabA
**   look at each feature in tabB 
**   if all the match criteria are valid
**     add the matching pair to the list of hits
** 
** Add any required hits from the hit-list to the output feature table
** 
**
** @param [r] tabA [const AjPFeattable] table A of features to compare to tabB
** @param [r] tabB [const AjPFeattable] table B of features to compare to tabA
** @param [r] overlapi [ajint] types of overlap allowed
** @param [r] minrange [ajint] min distance allowed
** @param [r] maxrange [ajint] max distance allowed
** @param [r] rangetypei [ajint] where to measure the distance from
** @param [r] sensei [ajint] sense relationships allowed
** @param [u] outtab [AjPFeattable] output feature table
** @param [r] twoout [AjBool] True=write both features, else make a single one
** @param [r] typeout [const AjPStr] if a single feature, this is its type name
** @return [void]
** @@
******************************************************************************/

static void twofeat_rippledown(const AjPFeattable tabA,
			       const AjPFeattable tabB,
			       ajint overlapi, ajint minrange,
			       ajint maxrange, ajint rangetypei,
			       ajint sensei,
			       AjPFeattable outtab, AjBool twoout,
			       const AjPStr typeout)
{


    AjIList    iterA = NULL;
    AjPFeature gfA   = NULL;

    AjIList    iterB = NULL;
    AjPFeature gfB   = NULL;

    AjPList hitlist;
    PHit detail = NULL;


    hitlist = ajListNew(); 
    
    if(ajFeattableGetSize(tabA))
    {
        /* For all features in tabA ... */
    	iterA = ajListIterNewread(tabA->Features);
    	while(!ajListIterDone(iterA))
    	{
    	    gfA = ajListIterGet(iterA);
            ajDebug("In rippledown gfA=%S %d..%d\n",
		    ajFeatGetType(gfA), ajFeatGetStart(gfA),
		    ajFeatGetEnd(gfA));

            /* For all features in tabB ... */
            if(ajFeattableGetSize(tabB)) 
            {
   	        iterB = ajListIterNewread(tabB->Features);
                while(!ajListIterDone(iterB))
                {
                    gfB = ajListIterGet(iterB);
                    ajDebug("In rippledown gfB=%S %d..%d\n",
			    ajFeatGetType(gfB), ajFeatGetStart(gfB),
			    ajFeatGetEnd(gfB));

		    /*
		    ** check for overlap, minrange, maxrange, rangetype,
		    ** sense, order
		    */
                    if(twofeat_check_match(gfA, gfB, &detail, overlapi,
					   minrange, maxrange, rangetypei,
					   sensei))
                        /* push details on hitlist */
                        ajListPush(hitlist, detail);
                }
                ajListIterDel(&iterB);
            }
	}
	ajListIterDel(&iterA);
    }

    /* Put hits in outtab */
    twofeat_report_hits(hitlist, twoout, typeout, outtab);


    ajListFree(&hitlist);

    return;
}




/* @funcstatic twofeat_report_hits ***********************************
**
** Outputs the pairs of hits to the output feature table
**
** @param [r] hitlist [const AjPList] list of matches (PHit) 
** @param [r] twoout [AjBool] True if want pairs of features output
** @param [r] typeout [const AjPStr]  name of type if want single features
** @param [u] outtab [AjPFeattable] output feature table
** @return [void]
** @@
******************************************************************************/

static void twofeat_report_hits(const AjPList hitlist, AjBool twoout,
				const AjPStr typeout, AjPFeattable outtab)
{
    char strand;
    ajint frame = 0;
    float score = 0.0;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    AjPFeature feature;
    static AjPStr tmp = NULL;
    ajint start;
    ajint end;

    AjIList iter = NULL;
    PHit detail  = NULL;

    source = ajStrNew();
    type   = ajStrNew();
    	
    ajStrAssignC(&source,"twofeat");
    ajStrAssignS(&type, typeout);


    iter = ajListIterNewread(hitlist);
    while(!ajListIterDone(iter))
    {
        detail = ajListIterGet(iter);

        if(twoout)
	{
            ajFeattableAdd(outtab, ajFeatNewFeat(detail->gfA));
            ajFeattableAdd(outtab, ajFeatNewFeat(detail->gfB));
    
        }
	else
	{
            start = detail->Start;
            end = detail->End;

            /* if both features are -ve then output this, else +ve */
            if((detail->gfA)->Strand == '-' &&
                (detail->gfB)->Strand == '-')
                strand = '-';
	    else
                strand = '+';

            feature = ajFeatNew(outtab, source, type, start, end,
				score, strand, frame);

            ajFmtPrintS(&tmp, "*startA %d", ajFeatGetStart(detail->gfA));
            ajFeatTagAddSS(feature, NULL, tmp);

            ajFmtPrintS(&tmp, "*endA %d", ajFeatGetEnd(detail->gfA));
            ajFeatTagAddSS(feature, NULL, tmp);

            ajFmtPrintS(&tmp, "*startB %d", ajFeatGetStart(detail->gfB));
            ajFeatTagAddSS(feature, NULL, tmp);

            ajFmtPrintS(&tmp, "*endB %d", ajFeatGetEnd(detail->gfB));
            ajFeatTagAddSS(feature, NULL, tmp);

	    ajStrDel(&tmp);
	}

        /* delete hit */
        twofeat_HitsDel(&detail);
    }
    ajListIterDel(&iter);

    ajStrDel(&source);
    ajStrDel(&type);
    
    return;
}




/* @funcstatic twofeat_find_features ***********************************
**
** Finds seq features matching the required criteria and puts them in tab
**
** @param [r] seq [const AjPSeq] the sequence
** @param [u] tab [AjPFeattable] Feature table to populate
** @param [r] begin [ajint] start position in sequence
** @param [r] end [ajint] end position in sequence
** @param [r] source [const AjPStr] source criterion
** @param [r] type [const AjPStr] type criterion
** @param [r] sense [ajint] sense criterion
** @param [r] minscore [float] min score criterion
** @param [r] maxscore [float] max score criterion
** @param [r] tag [const AjPStr] tag criterion
** @param [r] value [const AjPStr] tag value criterion
** @return [void] 
** @@
******************************************************************************/

static void twofeat_find_features(const AjPSeq seq, AjPFeattable tab,
				  ajint begin, ajint end, const AjPStr source,
				  const  AjPStr type, ajint sense,
				  float minscore, float maxscore,
				  const AjPStr tag, const AjPStr value)
{

    /* get feature table of sequence */
    const AjPFeattable seqtab = ajSeqGetFeat(seq);

    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    AjPFeature gfcopy = NULL;
    AjBool     tagsmatch;
    
   
    tagsmatch = ajFalse;

    /* For all features... */
    if(ajFeattableGetSize(seqtab)) 
    {
    	iter = ajListIterNewread(seqtab->Features);
    	while(!ajListIterDone(iter))
    	{
    	    gf = ajListIterGet(iter);

            /* is this feature local and in the sequence range? */
            if(! ajFeatIsLocalRange(gf, begin, end))
    	        continue;

    	    /* do criterion match */
            if(twofeat_MatchFeature(gf, source, type, sense, minscore,
				    maxscore, tag, value, &tagsmatch))
	    {
                ajDebug("Found feature source=%S type=%S %d..%d\n", 
                	ajFeatGetSource(gf), ajFeatGetType(gf),
			ajFeatGetStart(gf), ajFeatGetEnd(gf));
		/*
		** could explicitly make a new feature like this, but it
		** is probably safer to let ajFeatNewFeat do it automatically
		**  gfcopy = ajFeatNew(tab, gf->Source, gf->Type, gf->Start,
		** gf->End, gf->Score, gf->Strand, gf->Frame);
		*/
		gfcopy = ajFeatNewFeat(gf);
		ajFeattableAdd(tab, gfcopy);

		/* ajFeatTrace(gfcopy); */
	    }
	}
	ajListIterDel(&iter);
    }

    ajDebug("(In twofeat_find_features) No of hits in tab: %d\n",
	    ajFeattableGetSize(tab));
    
    return;
}




/* @funcstatic twofeat_MatchFeature *****************************************
**
** Test if a feature matches a set of criteria
**
** @param [r] gf [const AjPFeature] Feature to test
** @param [r] source [const AjPStr] Required Source pattern
** @param [r] type [const AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value$
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [const AjPStr] Required Tag pattern
** @param [r] value [const AjPStr] Required Value pattern
** @param [w] tagsmatch [AjBool*] true if a join has matching tag/values
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool twofeat_MatchFeature(const AjPFeature gf,
				   const AjPStr source, const AjPStr type,
				   ajint sense, float minscore,
				   float maxscore,
				   const AjPStr tag, const AjPStr value,
				   AjBool *tagsmatch)
{
    AjPStrTok tokens = NULL;
    AjPStr key = NULL;
    AjBool val = ajFalse;
    AjBool scoreok;

    scoreok = (minscore < maxscore);

     /* 
     ** is this a child of a join() ? 
     ** if it is a child, then we use the previous result of MatchPatternTags
     */
    if(!ajFeatIsMultiple(gf))
	*tagsmatch = twofeat_MatchPatternTags(gf, tag, value);

    /* ignore remote IDs */
    if(!ajFeatIsLocal(gf))
	return ajFalse;
  
     /* check source, type, sense, score, tags, values
     ** Match anything:
     **      for strings, '*'
     **      for sense, 0
     **      for score, maxscore <= minscore
     */

    if(!embMiscMatchPatternDelimC(ajFeatGetSource(gf), source,",;|") ||
       (ajFeatGetStrand(gf) == '+' && sense == -1) ||
       (ajFeatGetStrand(gf) == '-' && sense == +1) ||
       (scoreok && ajFeatGetScore(gf) < minscore) ||
       (scoreok && ajFeatGetScore(gf) > maxscore) ||
       !*tagsmatch)
	return ajFalse;

    if(ajStrGetLen(type))
    {
        val = ajFalse;
        tokens = ajStrTokenNewC(type, " \t\n\r,;|");

        while (ajStrTokenNextParse(tokens, &key))
        {
            if (ajFeatTypeMatchWildS(gf, key))
            {
                val = ajTrue;
                break;
            }
        }

        ajStrTokenDel( &tokens);
        ajStrDel(&key);
        if(!val)
            return ajFalse;
    }

    return ajTrue;                        
}




/* @funcstatic twofeat_MatchPatternTags **************************************
**
** Checks for a match of the tagpattern and valuepattern to at least one
** tag=value pair
**
** @param [r] feat [const AjPFeature] Feature to process
** @param [r] tpattern [const AjPStr] tags pattern to match with
** @param [r] vpattern [const AjPStr] values pattern to match with
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

static AjBool twofeat_MatchPatternTags(const AjPFeature feat,
				       const AjPStr tpattern,
				       const AjPStr vpattern)
{
    AjIList titer;                      /* iterator for feat */
    static AjPStr tagnam = NULL;        /* tag structure */
    static AjPStr tagval = NULL;        /* tag structure */
    AjBool val = ajFalse;               /* returned value */
    AjBool tval;                        /* tags result */
    AjBool vval;                        /* value result */


    /*
    **  if there are no tags to match, but the patterns are
    **  both '*', then allow this as a match
    */
    if(!ajStrCmpC(tpattern, "*") &&
       !ajStrCmpC(vpattern, "*"))
        return ajTrue;
    
    /* iterate through the tags and test for match to patterns */
    titer = ajFeatTagIter(feat);
    while(ajFeatTagval(titer, &tagnam, &tagval))
    {
        tval = embMiscMatchPatternDelimC(tagnam, tpattern,",;|");
	 /*
	 ** If tag has no value then
	 **   If vpattern is '*' the value pattern is a match
	 ** Else check vpattern
	 */

        if(!ajStrGetLen(tagval))
	{
            if(!ajStrCmpC(vpattern, "*"))
            	vval = ajTrue;
            else
		vval = ajFalse;
        }
	else
            vval = embMiscMatchPatternDelimC(tagval, vpattern,",;|");

        if(tval && vval)
	{
            val = ajTrue;
            break;
        }
    }
    ajListIterDel(&titer);
    
    return val;
}




/* @funcstatic twofeat_HitsNew *****************************************
**
** Creates a new PHit object
**
** @return [PHit] Hit object
** @@
******************************************************************************/

static PHit twofeat_HitsNew(void)
{
    PHit pthis;
    AJNEW0(pthis);
	    
    return pthis;
}




/* @funcstatic twofeat_HitsDel *****************************************
**
** Deletes a PHit object
**
** @param [d] pthis [PHit *] Pointer to object to be deleted
** @return [void]
** @@
******************************************************************************/

static void twofeat_HitsDel(PHit *pthis)
{
    if(! pthis)
	return;

    if(! *pthis)
	return;
	         
    AJFREE(*pthis);
             
    *pthis = NULL;

    return;
}




/* @funcstatic twofeat_check_match *********************************
**
** check for overlap, minrange, maxrange, rangetype, sense, order
** put the results in a PHit object
**
** @param [u] gfA [AjPFeature] Feature A - stored for adding new tags
** @param [u] gfB [AjPFeature] Feature B - stored for adding new tags
** @param [w] detail [PHit *] Returned details of match 
** @param [r] overlapi [ajint] types of overlap allowed
** @param [r] minrange [ajint] min distance allowed
** @param [r] maxrange [ajint] max distance allowed
** @param [r] rangetypei [ajint] where to measure the distance from
** @param [r] sensei [ajint] sense relationships allowed
** @return [AjBool] True if got a match
** @@
******************************************************************************/

static AjBool twofeat_check_match(AjPFeature gfA, AjPFeature gfB,
				  PHit *detail, ajint overlapi,
				  ajint minrange, ajint maxrange, ajint
				  rangetypei, ajint sensei)
{
    ajint distance = 0;
    ajint sA;
    ajint eA;
    ajint sB;
    ajint eB;				/* start and end positions */
    ajint ss;
    ajint ee;
    ajint se;
    ajint es;		  /* distances from start and end positions */
    ajint tmp1;
    ajint tmp2;

    sA = ajFeatGetStart(gfA);
    eA = ajFeatGetEnd(gfA);
    sB = ajFeatGetStart(gfB);
    eB = ajFeatGetEnd(gfB);

    *detail = NULL;

    ajDebug("Next gfA=%S %d..%d\n",
	    ajFeatGetType(gfA), ajFeatGetStart(gfA), ajFeatGetEnd(gfA));
    ajDebug("Next gfB=%S %d..%d\n",
	    ajFeatGetType(gfB), ajFeatGetStart(gfB), ajFeatGetEnd(gfB));

    /* get distances and absolute distances from each end */
    ss = abs(sA - sB);
    ee = abs(eA - eB);
    se = abs(sA - eB);
    es = abs(eA - sB);

    switch(rangetypei)
    {
    case RA_NEAREST:
        tmp1 = (ss < ee) ? ss : ee;
        tmp2 = (se < es) ? se : es;
        distance = (tmp1 < tmp2) ? tmp1 : tmp2;
        break;
 
    case RA_LEFT:
        distance = ss;
        break;

    case RA_RIGHT:
        distance = ee;
        break;

    case RA_FURTHEST:
        tmp1 = (ss > ee) ? ss : ee;
        tmp2 = (se > es) ? se : es;
        distance = (tmp1 > tmp2) ? tmp1 : tmp2;
        break;

    default:
        ajFatal("Unknown range type: %d", rangetypei);
    }

    ajDebug("Distance: %d\n", distance);

    /* check distance criteria */
    if(minrange < maxrange)
    {
	/* ignore distance if these are the same */
        ajDebug("Distance < %d: %B\n", minrange, distance < minrange);
        ajDebug("Distance > %d: %B\n", maxrange, distance > maxrange);
        if(distance < minrange)
	    return ajFalse;

        if(distance > maxrange)
	    return ajFalse;
    }
    ajDebug("Distance OK\n");
    
    /* check overlap criteria */
    switch(overlapi)
    {
    case OV_ANY:
	break;

    case OV_OVERLAP:			/* want sA in B or sB in A */
        if((sA<sB || sA>eB) && (sB<sA || sB>eA))
	    return ajFalse;
	break;

    case OV_NOTOVER:		  	/* want eA before B or sA after B */
        if((sA>=sB && sA<=eB) || (sB>=sA && sB<=eA))
	    return ajFalse;
	break;

    case OV_NOTIN:  /* want sA or eA out of B and sB or eB out of A */
        if((sA<=sB && eA>=eB) || (sB<=sA && eB>=eA))
	    return ajFalse;
	break;

    case OV_AIN:			/* want A in B */
        if(sA<sB || eA>eB)
	    return ajFalse;
	break;

    case OV_BIN:			/* want B in A */
        if(sB<sA || eB>eA)
	    return ajFalse;
	break;
   
    default:
        ajFatal("Unknown overlap type: %d", overlapi);
    }
    ajDebug("Overlap OK\n");

    /* check sense criteria */
    switch(sensei)
    {
    case SN_ANY:
	break;

    case SN_SAME:	
        if(ajFeatGetStrand(gfA) != ajFeatGetStrand(gfB))
	    return ajFalse;
	break;

    case SN_OPPOSITE:	
        if(ajFeatGetStrand(gfA) == ajFeatGetStrand(gfB)) return ajFalse;

    default:
        ajFatal("Unknown sense type: %d", sensei);
    }
    ajDebug("Sense OK\n");

    /* check order criteria */
    /* measure the order from the mid-point of each */
    switch(sensei)
    {
    case SN_ANY:
	break;

    case OR_AB:
        tmp1 = (eA-sA)/2;
        tmp2 = (eB-sB)/2;
        if(tmp1>tmp2)
	    return ajFalse;
	break;

    case OR_BA:
        tmp1 = (eA-sA)/2;
        tmp2 = (eB-sB)/2;
        if(tmp1<tmp2)
	    return ajFalse;
	break;

    default:
        ajFatal("Unknown sense type: %d", sensei);
    }
    ajDebug("Order OK\n");
    
    /*
    ** reject any match found to the same feature (same position,
    ** sense and type)
    */
    if(ss == 0 &&
       ee == 0 &&
       ajFeatGetStrand(gfA) == ajFeatGetStrand(gfB) &&
       ajStrMatchS(ajFeatGetType(gfA), ajFeatGetType(gfB)))
    {
        ajDebug("Found match of feature to itself\n");
        return ajFalse;    
    }

    /* if we have a hit, make a PHit object */
    *detail = twofeat_HitsNew();
    (*detail)->gfA = gfA;
    (*detail)->gfB = gfB;
    (*detail)->distance = distance;
    (*detail)->Start = (sA < sB) ? sA : sB;
    (*detail)->End = (eA > eB) ? eA : eB;

    ajDebug("Hit found\n");

    return ajTrue;

}




/* @funcstatic twofeat_get_overlap_type *********************************
**
** converts the overlap code to an integer
** 
**
** @param [r] overlap [const AjPStr] Overlap code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_overlap_type(const AjPStr overlap)
{

    if(ajStrMatchC(overlap, "A"))
	return OV_ANY;

    if(ajStrMatchC(overlap, "O"))
	return OV_OVERLAP;

    if(ajStrMatchC(overlap, "NO"))
	return OV_NOTOVER;

    if(ajStrMatchC(overlap, "NW"))
	return OV_NOTIN;

    if(ajStrMatchC(overlap, "AW"))
	return OV_AIN;

    if(ajStrMatchC(overlap, "BW")) return OV_BIN;

    ajFatal("Unknown Overlap code: %S", overlap);
    return -1;
}




/* @funcstatic twofeat_get_range_type *********************************
**
** converts the range code to an integer
** 
**
** @param [r] rangetype [const AjPStr] Range code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_range_type(const AjPStr rangetype)
{
    if(ajStrMatchC(rangetype, "N"))
	return RA_NEAREST;

    if(ajStrMatchC(rangetype, "L"))
	return RA_LEFT;

    if(ajStrMatchC(rangetype, "R"))
	return RA_RIGHT;

    if(ajStrMatchC(rangetype, "F"))
	return RA_FURTHEST;

    ajFatal("Unknown rangetype code: %S", rangetype);

    return -1;
}




/* @funcstatic twofeat_get_sense_type *********************************
**
** converts the sense code to an integer
** 
**
** @param [r] sense [const AjPStr] sense code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_sense_type(const AjPStr sense)
{
    if(ajStrMatchC(sense, "A"))
	return SN_ANY;

    if(ajStrMatchC(sense, "S"))
	return SN_SAME;

    if(ajStrMatchC(sense, "O"))
	return SN_OPPOSITE;

    ajFatal("Unknown sense code: %S", sense);

    return -1;
}




#if 0
/* @funcstatic twofeat_get_order_type *********************************
**
** converts the order code to an integer
** 
**
** @param [r] order [const AjPStr] order code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_order_type(const AjPStr order)
{
    if(ajStrMatchC(order, "A"))
	return OR_ANY;

    if(ajStrMatchC(order, "AB"))
	return OR_AB;

    if(ajStrMatchC(order, "BA"))
	return OR_BA;

    ajFatal("Unknown order code: %S", order);

    return -1;
}
#endif
