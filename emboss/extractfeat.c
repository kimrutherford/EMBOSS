/* @source extractseq application
**
** Extract features from a sequence
**
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




static void extractfeat_FeatSeqExtract(const AjPSeq seq, AjPSeqout seqout,
				       AjPFeattable featab, ajint before,
				       ajint after, AjBool join,
				       AjBool featinname,
				       const AjPStr describe);

static void extractfeat_GetFeatseq(const AjPSeq seq, const AjPFeature gf,
				   AjPStr *gfstr, AjBool sense);

static void extractfeat_WriteOut(AjPSeqout seqout, AjPStr *featstr,
				 AjBool compall, AjBool sense, ajint firstpos,
				 ajint lastpos,
				 ajint before, ajint after,
				 const AjPSeq seq,
				 AjBool remote, const AjPStr type,
				 AjBool featinname,
				 const AjPStr describestr);

static void extractfeat_BeforeAfter(const AjPSeq seq, AjPStr * featstr,
				    ajint firstpos, ajint lastpos,
				    ajint before, ajint after,
				    AjBool sense);

static void extractfeat_GetRegionPad(const AjPSeq seq, AjPStr *featstr, ajint
				     start, ajint end, AjBool sense,
				     AjBool beginning);

static void extractfeat_FeatureFilter(AjPFeattable featab,
				      const AjPStr source, const AjPStr type,
				      ajint sense,
				      float minscore,
				      float maxscore, const AjPStr tag,
				      const AjPStr value);

static AjBool extractfeat_MatchFeature(const AjPFeature gf,
				       const AjPStr source, const AjPStr type,
				       ajint sense,
				       float minscore,
				       float maxscore, const AjPStr tag,
				       const AjPStr value, AjBool *tagsmatch);

static AjBool extractfeat_MatchPatternTags(const AjPFeature feat,
					   const AjPStr tpattern,
					   const AjPStr vpattern);

static AjBool extractfeat_MatchPatternDescribe(const AjPFeature feat, 
					       const AjPStr describe,
					       AjPStr *strout);




/* @prog extractfeat **********************************************************
**
** Extract features from a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    ajint before;
    ajint after;
    AjBool join;
    AjPFeattable featab;
    AjBool featinname;
    AjPStr describe;

    /* feature filter criteria */
    AjPStr source = NULL;
    AjPStr type   = NULL;
    ajint sense;
    float minscore;
    float maxscore;
    AjPStr tag  = NULL;
    AjPStr value = NULL;

    embInit("extractfeat", argc, argv);
    
    seqall     = ajAcdGetSeqall("sequence");
    seqout     = ajAcdGetSeqout("outseq");
    before     = ajAcdGetInt("before");
    after      = ajAcdGetInt("after");
    join       = ajAcdGetBoolean("join");
    featinname = ajAcdGetBoolean("featinname");
    describe   = ajAcdGetString("describe");
    
    /* feature filter criteria */
    source   = ajAcdGetString("source");
    type     = ajAcdGetString("type");
    sense    = ajAcdGetInt("sense");
    minscore = ajAcdGetFloat("minscore");
    maxscore = ajAcdGetFloat("maxscore");
    tag      = ajAcdGetString("tag");
    value    = ajAcdGetString("value");
    
    while(ajSeqallNext(seqall, &seq))
    {
	/* get the feature table of the sequence */
	featab = ajSeqGetFeatCopy(seq);

        /* delete features in the table that don't match our criteria */
        extractfeat_FeatureFilter(featab, source, type, sense,
				  minscore, maxscore, tag, value);

        /* extract the features */
        extractfeat_FeatSeqExtract(seq, seqout, featab, before,
				   after, join, featinname, describe);

        ajFeattableDel(&featab);

    }
    
    ajSeqoutClose(seqout);
    
    ajSeqoutDel(&seqout);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    ajStrDel(&describe);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&tag);
    ajStrDel(&value);

    embExit();

    return 0;
}




/* @funcstatic extractfeat_FeatSeqExtract *************************************
**
** Extract features from a sequence
** Build up the complete sequence of a feature, concatenating child features
** to the parent if this is a multiple join and 'join' is set TRUE.
** When writing out, get the amount 'before' and 'after' the feature.
**
** @param [r] seq [const AjPSeq] sequence
** @param [u] seqout [AjPSeqout] output sequence
** @param [u] featab [AjPFeattable] features to extract
** @param [r] before [ajint] region before feature to add to extraction
** @param [r] after [ajint] region after feature to add to extraction
** @param [r] join [AjBool] concatenate 'join()' features
** @param [r] featinname [AjBool] TRUE if want the type to be part of the name
** @param [r] describe [const AjPStr] patterns of tag name for describe line
** @return [void]
** @@
******************************************************************************/

static void extractfeat_FeatSeqExtract(const AjPSeq seq, AjPSeqout seqout,
				       AjPFeattable featab, ajint before,
				       ajint after, AjBool join,
				       AjBool featinname,
				       const AjPStr describe)
{
    AjIList iter = NULL;
    AjPFeature gf = NULL;
    AjBool  single;		/* ajtrue = is not a multiple */
    AjBool  parent;		/* ajtrue = is a parent of a multiple */
    AjBool  child;		/* ajTrue = is a child of a multiple */
    AjBool  compall;		/* ajTrue = reverse comp all of join */
    AjBool  sense;		/* ajTrue = forward sense */
    AjBool  remote;		/* ajTrue = remote ID */
    AjPStr  type = NULL;	/* name of feature */
    AjPStr  featseq = NULL;	/* feature sequence string */
    AjPStr  tmpseq = NULL;	/* temporary sequence string */
    ajint   firstpos;
    ajint   lastpos;	        /* bounds of feature in sequence */
    AjPStr  describeout = NULL;	/* tag names/values to add to descriptions */


    /* For all features... */
    if(featab && ajFeattableSize(featab))
    {
	/* initialise details of a feature */
        featseq = ajStrNew();
        tmpseq  = ajStrNew();
        type    = ajStrNew();
        remote  = ajFalse;
        compall = ajFalse;
        sense   = ajTrue;
        firstpos = 0;
        lastpos  = 0;
        describeout = ajStrNew();


	iter = ajListIterNewread(featab->Features);
	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter) ;

	    /*
	    ** Determine what sort of thing this feature is. Only one of
	    ** these will be true.
	    ** True if this is part of a multiple join and it is not
	    ** the parent
	    */
	    child = ajFalse;

	    /* True if this is part of a multiple join and it is the parent */
	    parent = ajFalse;

	    /* True if this is not part of a multiple join */
	    single = ajFalse;

            if(ajFeatIsMultiple(gf))
	    {
            	if(ajFeatIsChild(gf))
            	    child = ajTrue;
		else
            	    parent = ajTrue;
            }
	    else
            	single = ajTrue;

	    /* 
	    ** If not wish to assembling joins(), then force all features
	    ** to be treated as single 
	    */
	    if(!join)
	    {
	    	child = ajFalse;
	    	parent = ajFalse;
	    	single = ajTrue;
	    }



	    ajDebug("feature %S %d-%d is parent %B, child %B, single %B\n",
		    ajFeatGetType(gf), ajFeatGetStart(gf), ajFeatGetEnd(gf),
		    parent, child, single);

	    /*
	    ** If single or parent, write out any stored previous feature
	    ** sequence
	    */	    
            if(single || parent)
	    {
            	extractfeat_WriteOut(seqout, &featseq, compall, sense,
				     firstpos, lastpos, before, after, seq,
				     remote, type, 
				     featinname, describeout);

                /* reset joined feature information */
                ajStrSetClear(&featseq);
                ajStrSetClear(&tmpseq);
                ajStrSetClear(&type);
		ajStrSetClear(&describeout);
                remote = ajFalse;
                compall = ajFalse;
                sense = ajTrue;
                firstpos = 0;
                lastpos = 0;
            }

	    /*
	    ** Don't process Remote IDs and abort a multiple join if one
	    ** contains a Remote ID
	    */
            if(! ajFeatIsLocal(gf))
                remote = ajTrue;

	    ajDebug("remote=%B\n", remote);
            if(remote)
            	continue;


	    /* if parent, note if have Complemented Join */
            if(parent)
                compall = ajFeatIsCompMult(gf);

	    /*
	    ** Get the sense of the feature
	    ** NB.  if complementing several joined features, then pretend they
	    ** are forward sense until its possible to  reverse-complement
	    ** them all together.
	    */
	    if(!compall && ajFeatGetStrand(gf) == '-')
	        sense = ajFalse;
	    
	    /* get 'type' name of feature */
	    if(single || parent)
	    	ajStrAssignS(&type, ajFeatGetType(gf));
	    
	    /*
	    ** if single or parent, get 'before' + 'after' sequence
	    ** positions
	    */
            if(single || parent)
	    {
                firstpos = ajFeatGetStart(gf)-1;
                lastpos = ajFeatGetEnd(gf)-1;
            }
	    
	    /* if child, update the boundary positions */
            if(child)
	    {
                if(sense)
                    lastpos = ajFeatGetEnd(gf)-1;
		else
		    firstpos = ajFeatGetStart(gf)-1;
            }
	    
            extractfeat_MatchPatternDescribe(gf, describe, &describeout);
	    
	    /* get feature sequence(complement if required) */
            extractfeat_GetFeatseq(seq, gf, &tmpseq, sense);
	    ajDebug("extracted feature = %d bases\n", ajStrGetLen(tmpseq));
	    
	    /*
	    ** if child, append to previous sequence, otherwise
	    ** simply assign
	    */
            if(child)
	    {
            	ajStrAppendS(&featseq, tmpseq);
	        ajDebug("joined feature so far = %d bases\n",
			ajStrGetLen(featseq));
            }
	    else
            	ajStrAssignS(&featseq, tmpseq);
	}
	ajListIterDel(&iter) ;
	
	/*
	** write out any previous sequence(s)
	** - add before + after, complement all
	*/
        extractfeat_WriteOut(seqout, &featseq, compall, sense,
			     firstpos, lastpos, before, after,
			     seq, remote, type, 
			     featinname, describeout);
	
        ajStrDel(&featseq);
        ajStrDel(&tmpseq);
        ajStrDel(&type);
        ajStrDel(&describeout);
    }
    
    return;
}




/* @funcstatic extractfeat_GetFeatseq *****************************************
**
** Get the sequence string of a feature (complement if reverse sense)
**
** @param [r] seq [const AjPSeq] input sequence
** @param [r] gf [const AjPFeature] feature
** @param [w] gfstr [AjPStr *] the resulting feature sequence string
** @param [r] sense [AjBool] FALSE if reverse sense, so complement the result
** @return [void]
** @@
******************************************************************************/

static void extractfeat_GetFeatseq(const AjPSeq seq, const AjPFeature gf, 
				   AjPStr *gfstr, AjBool sense)
{
    const AjPStr str = NULL;		/* sequence string */
    AjPStr tmp = NULL;

    str = ajSeqGetSeqS(seq);
    tmp = ajStrNew();

    ajDebug("about to get sequence from %d-%d, len=%d\n",
	    ajFeatGetStart(gf), ajFeatGetEnd(gf),
	    ajFeatGetLength(gf));

    ajStrAssignSubS(&tmp, str, ajFeatGetStart(gf)-1, ajFeatGetEnd(gf)-1);

    /* if feature was in reverse sense, then get reverse complement */
    if(!sense)
    	ajSeqstrReverse(&tmp);

    ajStrAssignS(gfstr, tmp);

    ajStrDel(&tmp);

    return;
}




/* @funcstatic extractfeat_WriteOut *******************************************
**
** Get sequence with 'before' and 'after' additions/truncation
** Write feature sequence to a file
**
** @param [u] seqout [AjPSeqout] output sequence
** @param [u] featstr [AjPStr *] sequence string of feature
** @param [r] compall [AjBool] TRUE if we need to complement a join
** @param [r] sense [AjBool] FALSE if any feature in this was complemented
** @param [r] firstpos [ajint] position of start of feature
** @param [r] lastpos [ajint] position of end of feature
** @param [r] before [ajint] region before feature to get
** @param [r] after [ajint] region after feature to get
** @param [r] seq [const AjPSeq] input sequence
** @param [r] remote [AjBool] TRUE if must abort becuase it includes Remote IDs
** @param [r] type [const AjPStr] type of feature
** @param [r] featinname [AjBool] TRUE if want the type to be part of the name
** @param [r] describestr [const AjPStr] tag names/values for description line
** @return [void]
** @@
******************************************************************************/

static void extractfeat_WriteOut(AjPSeqout seqout, AjPStr *featstr,
				 AjBool compall, AjBool sense, ajint firstpos,
				 ajint lastpos, ajint before, ajint after,
				 const AjPSeq seq, AjBool remote,
				 const AjPStr type,
				 AjBool featinname, const AjPStr describestr)
{
    AjPSeq newseq = NULL;
    AjPStr name   = NULL;	/* new name of the sequence */
    AjPStr value  = NULL;	/* string value of start or end position */
    AjPStr desc   = NULL;	/* sequence description */
    ajint tmp;

    /* see if there is a sequence to be written out */
    if(!ajStrGetLen(*featstr))
    {
        ajDebug("feature not written out because it has length=0 "
		"(probably first time round)\n");
    	return;
    }

    /* see if must abort because there were Remote IDs in the features */
    if(remote)
    {
        ajDebug("feature not written out because it has Remote IDs\n");
        return;
    }

    /* if complementing the whole sequence, swap before and after */
    if(compall)
    {
        tmp    = before;
        before = after;
        after  = tmp;
    }

    ajDebug("feature = %d bases\n", ajStrGetLen(*featstr));

    /* featstr may be edited, so it is a AjPStr* */
    extractfeat_BeforeAfter (seq, featstr, firstpos, lastpos, before,
			     after, sense);

    ajDebug("feature+before/after = %d bases\n", ajStrGetLen(*featstr));

    /*
    ** if join was all in reverse sense, now finally get reverse
    ** complement
    */
    if(compall)
    	ajSeqstrReverse(featstr);

    /* set the extracted sequence */
    newseq = ajSeqNew();
    ajSeqAssignSeqS(newseq, *featstr);

     /* create a nice name for the new sequence */
    name = ajStrNew();
    ajStrAppendS(&name, ajSeqGetNameS(seq));
    ajStrAppendC(&name, "_");
    value = ajStrNew();
    ajStrFromInt(&value, firstpos+1);
    ajStrAppendS(&name, value);
    ajStrAppendC(&name, "_");
    ajStrFromInt(&value, lastpos+1);
    ajStrAppendS(&name, value);

    /* add the type of feature to the name, if required */
    if(featinname)
    {
    	ajStrAppendC(&name, "_");
    	ajStrAppendS(&name, type);
    }

    ajSeqAssignNameS(newseq, name);

    /* set the sequence description with the 'type' added */
    desc = ajStrNew();
    ajStrAppendC(&desc, "[");
    ajStrAppendS(&desc, type);
    ajStrAppendC(&desc, "] ");
    if(ajStrGetLen(describestr))
    	ajStrAppendS(&desc, describestr);

    ajStrAppendS(&desc, ajSeqGetDescS(seq));
    ajSeqAssignDescS(newseq, desc);

    /* set the type */
    if(ajSeqIsNuc(seq))
        ajSeqSetNuc(newseq);
    else
        ajSeqSetProt(newseq);


    /* write the new sequence */
    ajSeqoutWriteSeq(seqout, newseq);


    ajSeqDel(&newseq);
    ajStrDel(&name);
    ajStrDel(&value);
    ajStrDel(&desc);

    return;
}




/* @funcstatic extractfeat_BeforeAfter ****************************************
**
** Extracts regions before and after a feature (complement if necessary)
**
** @param [r] seq [const AjPSeq] input sequence
** @param [u] featstr [AjPStr *] sequence string of feature
** @param [r] firstpos [ajint] position of start of feature
** @param [r] lastpos [ajint] position of end of feature
** @param [r] before [ajint] region before feature to get
** @param [r] after [ajint] region after feature to get
** @param [r] sense [AjBool] FALSE if any feature in this was complemented
** @return [void]
** @@
******************************************************************************/

static void extractfeat_BeforeAfter(const AjPSeq seq, AjPStr * featstr,
				    ajint firstpos, ajint lastpos,
				    ajint before, ajint after,
				    AjBool sense)

{
    ajint start;
    ajint end;
    ajint featlen;
    ajint len;



    /*
     ** There is now:
     ** A complete set of joined features in featstr.
     ** 'sense' is FALSE if any feature was reverse sense (ignore 'compall'
     ** in this routine).
     ** 'firstpos' and 'lastpos' set to the first and last positions
     ** of the feature
     */


    /*
    ** get start and end positions to truncate featstr at or to
    ** extract from seq
    */

    /* do negative values of before/after */
    featlen = ajStrGetLen(*featstr)-1;
    start = 0;
    end = featlen;

    if(before < 0)			/* negative, so before end */
    	start = end + before+1;

    if(after < 0)			/* negative, so after start */
    	end = start - after-1;

    if(end < start)
    {
        ajWarn("Extraction region end less than start for %S [%d-%d]",
	       ajSeqGetNameS(seq), firstpos+1, lastpos+1);
        return;
    }


     /*
     ** truncate the featstr
     ** if start or end are past start/end of featstr, use 0 or featlen
     */
    if(before < 0 || after < 0)
    {
        ajDebug("truncating featstr to %d-%d\n",
		start < 0 ? 0 : start, end>featlen ? featlen : end);
        ajDebug("featstr len=%d bases\n", ajStrGetLen(*featstr));
    	ajStrKeepRange(featstr, start<0 ? 0 : start, end>featlen ? featlen :end);
        ajDebug("featstr len=%d bases\n", ajStrGetLen(*featstr));
    }


    /*
    ** add surrounding sequence if past 0/featlen when before/after
    ** are negative
    */
    if(start < 0)
    {
        ajDebug("start < 0\n");
        len = -start;
        if(sense)
            extractfeat_GetRegionPad(seq, featstr, firstpos-len,
				     firstpos-1, sense, ajTrue);
	else
            extractfeat_GetRegionPad(seq, featstr, lastpos+1,
				     lastpos+len, sense, ajTrue);
    }

    if(end > featlen)
    {
	/* NB use the original length of featstr */
        ajDebug("end > featlen\n");
        len = end-featlen;
        ajDebug("len=%d, end=%d\n", len, end);

        if(sense)
            extractfeat_GetRegionPad(seq, featstr, lastpos+1,
				     lastpos+len, sense, ajFalse);
	else
            extractfeat_GetRegionPad(seq, featstr, firstpos-len,
				     firstpos-1, sense, ajFalse);
    }


    /* add surrounding sequence if have positive values of before/after */
    if(before > 0)
    {
        ajDebug("before > 0\n");
        if(sense)
	{
            ajDebug("get Before firstpos=%d\n", firstpos);
            extractfeat_GetRegionPad(seq, featstr, firstpos-before,
				     firstpos-1, sense, ajTrue);
	}
	else
	{
	    ajDebug("get Before (reverse sense) lastpos=%d\n", lastpos);
            extractfeat_GetRegionPad(seq, featstr, lastpos+1, lastpos+before,
				     sense, ajTrue);
	}
    }

    if(after > 0)
    {
        ajDebug("after > 0\n");
        if(sense)
	{
	    ajDebug("get After lastpos=%d\n", lastpos);
            extractfeat_GetRegionPad(seq, featstr, lastpos+1, lastpos+after,
				     sense, ajFalse);
	}
	else
	{
	    ajDebug("get After (reverse sense) firstpos=%d\n", firstpos);
            extractfeat_GetRegionPad(seq, featstr, firstpos-after,
				     firstpos-1, sense, ajFalse);
	}
    }

    return;
}




/* @funcstatic extractfeat_GetRegionPad ***************************************
**
** Gets a subsequence string and pads with N or X if it is off the end
**
** @param [r] seq [const AjPSeq] Sequence to extract from
** @param [w] featstr [AjPStr *] sequence string of feature
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @param [r] sense [AjBool] FALSE if reverse sense
** @param [r] beginning [AjBool] TRUE if prepend, FALSE if want to append
** @return [void]
** @@
******************************************************************************/

static void extractfeat_GetRegionPad(const AjPSeq seq, AjPStr *featstr,
				     ajint start, ajint end, AjBool sense,
				     AjBool beginning)
{
    ajint tmp;
    ajint pad;

    AjPStr result;

    ajDebug("In extractfeat_GetRegionPad start=%d, end=%d\n", start, end);

    result = ajStrNew();


    if(start > end)
    	return;

    if(start < 0)
    {
        pad = -start;
        if(ajSeqIsNuc(seq))
            ajStrAppendCountK(&result, 'N', pad);
        else
            ajStrAppendCountK(&result, 'X', pad);
        start = 0;
    }

    if(end > (ajint) ajSeqGetLen(seq)-1)
    	tmp = ajSeqGetLen(seq)-1;
    else
    	tmp = end;

    if(start <= (ajint) ajSeqGetLen(seq) && tmp >= 0)
    {
        ajDebug("Get subsequence %d-%d\n", start, tmp);
        ajStrAppendSubS(&result, ajSeqGetSeqS(seq), start, tmp);
        ajDebug("result=%S\n", result);
    }

    if(end > (ajint) ajSeqGetLen(seq)-1)
    {
        pad = end - ajSeqGetLen(seq)+1;
        if(ajSeqIsNuc(seq))
            ajStrAppendCountK(&result, 'N', pad);
        else
            ajStrAppendCountK(&result, 'X', pad);
        ajDebug("result=%S\n", result);
    }


    /* if feature was in reverse sense, then get reverse complement */
    if(!sense)
    {
	ajDebug("get reverse sense of subsequence\n");
    	ajSeqstrReverse(&result);
	ajDebug("result=%S\n", result);
    }

    if(beginning)
    {
	ajDebug("Prepend to featstr: %S\n", result);
        ajStrInsertS(featstr, 0, result);
    }
    else
    {
	ajDebug("Append to featstr: %S\n", result);
    	ajStrAppendS(featstr, result);
    }
    ajDebug("featstr=%S\n", *featstr);


    ajStrDel(&result);

    return;
}




/* @funcstatic extractfeat_FeatureFilter **************************************
**
** Removes unwanted features from a feature table
**
** @param [u] featab [AjPFeattable] Feature table to filter
** @param [r] source [const AjPStr] Required Source pattern
** @param [r] type [const AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value$
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [const AjPStr] Required Tag pattern
** @param [r] value [const AjPStr] Required Value pattern
** @return [void]
** @@
******************************************************************************/

static void extractfeat_FeatureFilter(AjPFeattable featab,
				      const AjPStr source, const AjPStr type,
				      ajint sense,
				      float minscore, float maxscore,
				      const AjPStr tag, const AjPStr value)
{
    AjIList iter = NULL;
    AjPFeature gf = NULL;

    /*
     ** 'tagsmatch' is set true if a parent of a join()
     ** has matching tag/values.
     ** Remember the value of a the pattern match to the tags
     ** of the parent of a join() because the children of the join()
     ** don't contain the tags information in their gf's
     */
    AjBool tagsmatch = ajFalse;


    /* foreach feature in the feature table */
    if(featab)
    {
	iter = ajListIterNew(featab->Features);
	while(!ajListIterDone(iter))
	{
	    gf = (AjPFeature)ajListIterGet(iter);
	    if(!extractfeat_MatchFeature(gf, source, type, sense,
					 minscore, maxscore, tag, value,
					 &tagsmatch))
	    {
		/* no match, so delete feature from feature table */
		ajFeatDel(&gf);
		ajListIterRemove(iter);
	    }
	}
	ajListIterDel(&iter);
   }

    return;
}




/* @funcstatic extractfeat_MatchFeature ***************************************
**
** Test if a feature matches a set of criteria
**
** @param [r] gf [const AjPFeature] Feature to test
** @param [r] source [const AjPStr] Required Source pattern
** @param [r] type [const AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value)
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [const AjPStr] Required Tag pattern
** @param [r] value [const AjPStr] Required Value pattern
** @param [u] tagsmatch [AjBool *] true if a join has matching tag/values
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool extractfeat_MatchFeature(const AjPFeature gf,
				       const AjPStr source,
				       const AjPStr type, ajint sense,
				       float minscore,
				       float maxscore, const AjPStr tag,
				       const AjPStr value, AjBool *tagsmatch)
{
    AjBool scoreok;

    /* if maxscore < minscore, then don't test the scores */
    scoreok = (minscore < maxscore);


     /*
     ** is this a child of a join() ?
     ** if it is a child, then we use the previous result of MatchPatternTags
     */
    if(!ajFeatIsMultiple(gf) || !ajFeatIsChild(gf))
        *tagsmatch = extractfeat_MatchPatternTags(gf, tag, value);

    /* ignore remote IDs */

    /*
    ** No - don't reject remote IDs here.  We want to include all features
    ** in this routine so that we can reject a whole join later on if any one
    ** member of the join is remote. 
    ** if(!ajFeatIsLocal(gf))
    ** return ajFalse;
    */

    /* check source, type, sense, score, tags, values */
    /* Special values indication that we match anything:
    **      for strings, '*'
    **      for sense, 0
    **      for score, maxscore <= minscore
    */

    ajDebug("extractfeat_MatchFeature\n");

    ajDebug("embMiscMatchPattern(ajFeatGetSource(gf), source) %B\n",
	    embMiscMatchPattern(ajFeatGetSource(gf), source));
    ajDebug("embMiscMatchPattern(ajFeatGetType(gf), type) %B\n",
	    embMiscMatchPattern(ajFeatGetType(gf), type));
    ajDebug("ajFeatGetStrand(gf) '%x' sense %d\n", ajFeatGetStrand(gf), sense);
    ajDebug("scoreok: %B ajFeatGetScore(gf): %f minscore:%f maxscore:%f\n",
	    scoreok, ajFeatGetScore(gf), minscore, maxscore);
    if(!embMiscMatchPattern(ajFeatGetSource(gf), source) ||
       !embMiscMatchPattern(ajFeatGetType(gf), type) ||
       (ajFeatGetStrand(gf) == '+' && sense == -1) ||
       (ajFeatGetStrand(gf) == '-' && sense == +1) ||
       (scoreok && ajFeatGetScore(gf) < minscore) ||
       (scoreok && ajFeatGetScore(gf) > maxscore) ||
       !*tagsmatch)
    {
	ajDebug("return ajFalse\n");
	return ajFalse;
    }
    ajDebug("return ajTrue\n");

    return ajTrue;
}




/* @funcstatic extractfeat_MatchPatternTags ***********************************
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

static AjBool extractfeat_MatchPatternTags(const AjPFeature feat,
					   const AjPStr tpattern,
					   const AjPStr vpattern)
{
    AjIList titer;                      /* iterator for feat */
    AjPStr tagnam = NULL;        /* tag structure */
    AjPStr tagval = NULL;        /* tag structure */
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
        tval = embMiscMatchPattern(tagnam, tpattern);

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
            vval = embMiscMatchPattern(tagval, vpattern);

        if(tval && vval)
	{
            val = ajTrue;
            break;
        }
    }
    ajListIterDel(&titer);

    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return val;
}




/* @funcstatic extractfeat_MatchPatternDescribe *******************************
**
** Checks all of the tag names of a feature against a set of patterns.
** If one or more matches are found, then it writes the matching tags
** with their values, (if any), to a returned string ready to be
** added to the sequence's Description line.
**
** @param [r] feat [const AjPFeature] Feature to process
** @param [r] describe [const AjPStr] tags patterns to search with
** @param [w] strout [AjPStr*] returned string to add to Description line
**
** @return [AjBool] ajTrue if any matching tags were found
** @@
******************************************************************************/

static AjBool extractfeat_MatchPatternDescribe(const AjPFeature feat, 
					       const AjPStr describe,
					       AjPStr *strout)
{
    AjIList titer;                      /* iterator for feat */
    AjPStr tagnam = NULL;        /* tag structure */
    AjPStr tagval = NULL;        /* tag structure */
    AjBool val = ajFalse;               /* returned value */


    /* iterate through the tags and test for match to patterns */
    titer = ajFeatTagIter(feat);

    while(ajFeatTagval(titer, &tagnam, &tagval))
    {
        if(embMiscMatchPattern(tagnam, describe))
	{
            /* There's a match, so write to strout in a pretty format */
            if(!val)
            	ajStrAssignC(strout, "(");
            else
            	ajStrAppendC(strout, ", ");
            val = ajTrue;
            ajStrAppendS(strout, tagnam);

            if(ajStrGetLen(tagval))
	    {
            	ajStrAppendC(strout, "=\"");
            	ajStrAppendS(strout, tagval);
            	ajStrAppendC(strout, "\"");
            }
        }	
    }
    ajListIterDel(&titer);

    if(val)
        ajStrAppendC(strout, ") ");


    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return val;
}
