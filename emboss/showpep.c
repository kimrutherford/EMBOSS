/* @source showpep application
**
** Display a sequence with translations, features and other bits
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 14 Sept 1999 - GWW - written
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




static void showpep_FormatShow(EmbPShow ss,
			       const AjPStr format,
			       const AjPRange uppercase,
			       const AjPRange highlight,  AjBool threeletter,
			       AjBool numberseq, const AjPFeattable feat,
			       const AjPRange annotation);

static AjBool showpep_MatchFeature(const AjPFeature gf,
				   AjPFeature newgf,
				   const AjPStr matchsource,
				   const AjPStr matchtype, AjBool testscore,
				   float minscore,
				   float maxscore, const AjPStr matchtag,
				   const AjPStr matchvalue, AjBool *tagsmatch, 
				   AjBool stricttags);
static AjBool showpep_MatchPatternTags(const AjPFeature gf,
				       AjPFeature newgf, 
				       const AjPStr tpattern,
				       const AjPStr vpattern,
				       AjBool stricttags);
static void showpep_FeatureFilter(const AjPFeattable featab,
				  AjPFeattable newfeatab,
				  const AjPStr matchsource,
				  const AjPStr matchtype,
				  AjBool testscore, float minscore,
				  float maxscore, const AjPStr matchtag,
				  const AjPStr matchvalue, AjBool stricttags);
static AjPFeature showpep_FeatCopy(const AjPFeature orig);





/* @prog showpep **************************************************************
**
** Display a sequence with features, translation etc
**
******************************************************************************/

int main(int argc, char **argv)
{

    ajint begin;
    ajint end;
    AjPSeqall seqall;
    AjPSeq seq;
    EmbPShow ss;
    AjPFile outfile;
    AjPStr formatname;
    AjPStr *thinglist;
    AjPRange uppercase;
    AjPRange highlight;
    AjBool threeletter;
    AjBool numberseq;
    AjBool nameseq;
    ajint width;
    ajint length;
    ajint margin;
    AjBool description;
    ajint offset;
    AjBool html;
    AjBool stricttags;

    AjPStr descriptionline;
    AjPFeattable feattab;
    AjPFeattable newfeattab;/* feature table copy, unwanted features removed */
    AjPRange annotation;

    /* holds ACD or constructed format for output */
    AjPStr format;
    ajint i;

    /* feature filter criteria */
    AjPStr matchsource = NULL;
    AjPStr matchtype   = NULL;
    float minscore;
    float maxscore;
    AjPStr matchtag   = NULL;
    AjPStr matchvalue = NULL;
    AjBool testscore = AJFALSE;


    embInit("showpep", argc, argv);

    seqall         = ajAcdGetSeqall("sequence");
    outfile        = ajAcdGetOutfile("outfile");
    formatname     = ajAcdGetListSingle("format");
    thinglist      = ajAcdGetList("things");
    uppercase      = ajAcdGetRange("uppercase");
    highlight      = ajAcdGetRange("highlight");
    annotation     = ajAcdGetRange("annotation");
    threeletter    = ajAcdGetBoolean("threeletter");
    numberseq      = ajAcdGetBoolean("number");
    width          = ajAcdGetInt("width");
    length         = ajAcdGetInt("length");
    margin         = ajAcdGetInt("margin");
    nameseq        = ajAcdGetBoolean("name");
    description    = ajAcdGetBoolean("description");
    offset         = ajAcdGetInt("offset");
    html           = ajAcdGetBoolean("html");

    /* feature filter criteria */
    matchsource = ajAcdGetString("sourcematch");
    matchtype   = ajAcdGetString("typematch");
    minscore    = ajAcdGetFloat("minscore");
    maxscore    = ajAcdGetFloat("maxscore");
    matchtag    = ajAcdGetString("tagmatch");
    matchvalue  = ajAcdGetString("valuematch");
    stricttags  = ajAcdGetBoolean("stricttags");

    testscore = (minscore || maxscore);
    if(minscore && !maxscore)
        if(minscore > maxscore)
            maxscore = minscore;
    if(!minscore && maxscore)
        if(minscore > maxscore)
            minscore = maxscore;

    format = ajStrNew();
    
    /* get the format to use */
    if(ajStrMatchC(formatname, "0"))
	for(i=0; thinglist[i]; i++)
	{
	    ajStrAppendS(&format, thinglist[i]);
	    ajStrAppendC(&format, " ");
	}
    else if(ajStrMatchC(formatname, "1"))
	ajStrAssignC(&format, "S A ");
    else if(ajStrMatchC(formatname, "2"))
	ajStrAssignC(&format, "B N T S A F ");
    else if(ajStrMatchC(formatname, "3"))
	ajStrAssignC(&format, "B N T S A ");
    else if(ajStrMatchC(formatname, "4"))
	ajStrAssignC(&format, "B N T S T C T A F ");
    else
	ajFatal("Invalid format type: %S", formatname);
    
    
    /* make the format upper case */
    ajStrFmtUpper(&format);
    
    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqGetBegin(seq)-1;
	end   = ajSeqGetEnd(seq)-1;

	/* do the name and description */
	if(nameseq)
	{
	    if(html)
		ajFmtPrintF(outfile, "<H2>%S</H2>\n",
			    ajSeqGetNameS(seq));
	    else
		ajFmtPrintF(outfile, "%S\n", ajSeqGetNameS(seq));
	}

	if(description)
	{
	    /*
	    **  wrap the description line at the width of the sequence
	    **  plus margin
	    */
	    if(html)
		ajFmtPrintF(outfile, "<H3>%S</H3>\n",
			    ajSeqGetDescS(seq));
	    else
	    {
		descriptionline = ajStrNew();
		ajStrAssignS(&descriptionline, ajSeqGetDescS(seq));
		ajStrFmtWrap(&descriptionline, width+margin);
		ajFmtPrintF(outfile, "%S\n", descriptionline);
		ajStrDel(&descriptionline);
	    }
	}


	/* get the feature table of the sequence */
	feattab = ajSeqGetFeatCopy(seq);

	/* new feature table to hold the filetered features */
        newfeattab = ajFeattableNew(NULL);
	ajDebug("created newfeattab %x\n", newfeattab);

        /* only copy features in the table that match our criteria */
        showpep_FeatureFilter(feattab, newfeattab, matchsource, matchtype,
			      testscore, minscore, maxscore, matchtag,
			      matchvalue, stricttags);


	/* make the Show Object */
	ss = embShowNew(seq, begin, end, width, length, margin, html, offset);

	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	showpep_FormatShow(ss, format,
			   uppercase, highlight, threeletter,
			   numberseq, newfeattab,
                           annotation);

	embShowPrint(outfile, ss);

	embShowDel(&ss);

	ajFeattableDel(&newfeattab);
	ajFeattableDel(&feattab);

	/* add a newline at the end of the sequence */
	ajFmtPrintF(outfile, "\n");

	if(html)
	    ajFmtPrintF(outfile, "</PRE>\n");
    }
    

    ajStrDel(&format);
    ajFileClose(&outfile);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDelarray(&thinglist);
    ajStrDel(&formatname);
    ajStrDel(&matchsource);
    ajStrDel(&matchtype);
    ajStrDel(&matchtag);
    ajStrDel(&matchvalue);
    ajRangeDel(&uppercase);
    ajRangeDel(&highlight);
    ajRangeDel(&annotation);

    embExit();

    return 0;
}




/* @funcstatic showpep_FormatShow *********************************************
**
** Set up the EmbPShow object, according to the required format
**
** @param [u] ss [EmbPShow] Show sequence object
** @param [r] format [const AjPStr] format codes for the required
**                       things to display
** @param [r] uppercase [const AjPRange] ranges to uppercase
** @param [r] highlight [const AjPRange] ranges to colour in HTML
** @param [r] threeletter [AjBool] use 3-letter code
** @param [r] numberseq [AjBool] put numbers on sequences
** @param [r] feat [const AjPFeattable] sequence's feature table
**                                 NULL after - pointer stored internally
** @param [r] annotation [const AjPRange] ranges to annotate
** @return [void]
** @@
******************************************************************************/

static void showpep_FormatShow(EmbPShow ss,
			       const AjPStr format,
			       const AjPRange uppercase,
			       const AjPRange highlight, AjBool threeletter,
			       AjBool numberseq, const AjPFeattable feat,
			       const AjPRange annotation)
{
    AjPStrTok tok;
    char white[] = " \t\n\r";
    char whiteplus[] = " \t,.!@#$%^&*()_+|~`\\={}[]:;\"'<>,.?/";
    AjPStr code = NULL;

    /* start token to parse format */
    tok = ajStrTokenNewC(format,  white);
    while(ajStrTokenNextParseC(&tok, whiteplus, &code))
    {
	ajStrFmtUpper(&code);

	if(!ajStrCmpC(code, "S"))
	    embShowAddSeq(ss, numberseq, threeletter, uppercase,
			  highlight);
	else if(!ajStrCmpC(code, "B"))
	    embShowAddBlank(ss);
	else if(!ajStrCmpC(code, "T"))
	    embShowAddTicks(ss);
	else if(!ajStrCmpC(code, "N"))
	    embShowAddTicknum(ss);
	else if(!ajStrCmpC(code, "C"))
	    embShowAddComp(ss, numberseq);
	else if(!ajStrCmpC(code, "F"))
	    embShowAddFT(ss, feat);
	else if(!ajStrCmpC(code, "A"))
	    embShowAddNote(ss, annotation);
	else
	    ajFatal("Formatting code not recognised: '%S'", code);
    }

    ajStrDel(&code);
    ajStrTokenDel(&tok);

    return;
}




/* @funcstatic showpep_FeatureFilter ******************************************
**
** Removes unwanted features from a feature table
**
** @param [r] featab [const AjPFeattable] Feature table to filter
** @param [u] newfeatab [AjPFeattable] Retured table of filtered features
** @param [r] matchsource [const AjPStr] Required Source pattern
** @param [r] matchtype [const AjPStr] Required Type pattern
** @param [r] testscore [AjBool] Filter by score values
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] matchtag [const AjPStr] Required Tag pattern
** @param [r] matchvalue [const AjPStr] Required Value pattern
** @param [r] stricttags [AjBool] If false then only display tag/values
**                                that match the criteria
** @return [void]
** @@
******************************************************************************/

static void showpep_FeatureFilter(const AjPFeattable featab,
				  AjPFeattable newfeatab,
				  const AjPStr matchsource,
				  const AjPStr matchtype, AjBool testscore,
				  float minscore,
				  float maxscore, const AjPStr matchtag,
				  const AjPStr matchvalue, AjBool stricttags)
{

    AjIList iter = NULL;
    AjPFeature gf = NULL;
    AjPFeature newgf = NULL;
    AjBool tagsmatch;

    tagsmatch = ajFalse;

    /* foreach feature in the feature table */
    if(featab)
    {
	iter = ajListIterNewread(featab->Features);
	while(!ajListIterDone(iter))
	{
	    gf = (AjPFeature)ajListIterGet(iter);
            newgf = showpep_FeatCopy(gf); /* copy of gf that we can add */
	    /* required tags to */
	    if(showpep_MatchFeature(gf, newgf, matchsource, matchtype,
				    testscore, minscore, maxscore, matchtag,
				    matchvalue, &tagsmatch, stricttags))
	    	 /* 
		 ** There's a match, so add the copy of gf
		 ** to the new feature table
		 */
                ajFeattableAdd(newfeatab, newgf);
	    else
	    	ajFeatDel(&newgf);
	}
	ajListIterDel(&iter);
    }

    return;
}




/* @funcstatic showpep_MatchFeature *******************************************
**
** Test if a feature matches a set of criteria
**
** @param [r] gf [const AjPFeature] Feature to test
** @param [u] newgf [AjPFeature] Copy of feature with filtered
**                                    Tag/Value list
** @param [r] source [const AjPStr] Required Source pattern
** @param [r] type [const AjPStr] Required Type pattern
** @param [r] testscore [AjBool] Filter by score values
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [const AjPStr] Required Tag pattern
** @param [r] value [const AjPStr] Required Value pattern
** @param [u] tagsmatch [AjBool *] true if a join has matching tag/values
** @param [r] stricttags [AjBool] If false then only display tag/values
**                                that match the criteria
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool showpep_MatchFeature(const AjPFeature gf, AjPFeature newgf,
				   const AjPStr source, const AjPStr type,
				   AjBool testscore,
                                   float minscore, float maxscore,
				   const AjPStr tag, const AjPStr value,
				   AjBool *tagsmatch, AjBool stricttags)
{
    AjPStrTok tokens = NULL;
    AjPStr key = NULL;
    AjBool val = ajFalse;

    /*
    ** is this a child of a join() ?
    ** if it is a child, then we use the previous result of MatchPatternTags
    */
    if(!ajFeatIsMultiple(gf))
	*tagsmatch = showpep_MatchPatternTags(gf, newgf, tag, value,
					      stricttags);

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
       (testscore && ajFeatGetScore(gf) < minscore) ||
       (testscore && ajFeatGetScore(gf) > maxscore) ||
       !*tagsmatch)
	return ajFalse;

    if(ajStrGetLen(type))
    {
        val = ajFalse;
        tokens = ajStrTokenNewC(type, " \t\n\r,;|");

        while (ajStrTokenNextParse( &tokens, &key))
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




/* @funcstatic showpep_MatchPatternTags ***************************************
**
** Checks for a match of the tagpattern and valuepattern to at least one
** tag=value pair
**
** @param [r] gf [const AjPFeature] Feature to process
** @param [u] newgf [AjPFeature] Copy of feature returned
**                               with filtered Tag/Value list
** @param [r] tpattern [const AjPStr] tags pattern to match with
** @param [r] vpattern [const AjPStr] values pattern to match with
** @param [r] stricttags [AjBool] If false then only display tag/values
**                                that match the criteria
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

static AjBool showpep_MatchPatternTags(const AjPFeature gf,
				       AjPFeature newgf,
				       const AjPStr tpattern,
				       const AjPStr vpattern,
				       AjBool stricttags)
{
    AjIList titer;                      /* iterator for feat */
    AjPStr tagnam = NULL;	        /* tag name from tag structure */
    AjPStr tagval = NULL;       	/* tag value from tag structure */
    AjBool val = ajFalse;               /* returned value */
    AjBool tval;                        /* tags result */
    AjBool vval;                        /* value result */

    /*
    **  If there are no tags to match, but the patterns are
    **  both '*', then allow this as a match
    */
    if(ajListGetLength(gf->Tags) == 0 && 
        !ajStrCmpC(tpattern, "*") &&
        !ajStrCmpC(vpattern, "*"))
        return ajTrue;


    /* iterate through the tags and test for match to patterns */
    titer = ajFeatTagIter(gf);
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
            /*
            ** The value can be one or more words and the vpattern could
            ** be the whole phrase, so test not only each word in vpattern
	    ** against the value, but also test to see if there is a match
	    ** of the whole of vpattern without spitting it up into words.
            */
            vval = (ajStrMatchS(tagval, vpattern) ||
		    embMiscMatchPatternDelimC(tagval, vpattern,",;|"));


        if(tval && vval)
	{
            val = ajTrue;
	    /*
	    ** if strict tags then only want to see the tags
	    ** that match the criteria
	    */
	    if(stricttags)
	    	ajFeatTagAddSS(newgf, tagnam, tagval);
        }

        /* if not strict tags then need to see all the tags */
        if(!stricttags)
            ajFeatTagAddSS(newgf, tagnam, tagval);
    }
    ajListIterDel(&titer);

    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return val;
}




/* @funcstatic showpep_FeatCopy ***********************************************
**
** Makes a copy of a feature, except for the Tags list which is added later.
** This is mostly copied from the original in ajFeatCopy,
** but without the Tags stuff.
**
** @param [r]   orig  [const AjPFeature]  Original feature
** @return [AjPFeature] New copy of  feature
** @@
******************************************************************************/

static AjPFeature showpep_FeatCopy(const AjPFeature orig) 
{
	
    AjPFeature ret;
      
    AJNEW0(ret);

    ret->Tags = ajListNew();

    ajStrAssignS(&ret->Source, orig->Source);
    ajStrAssignS(&ret->Type, orig->Type);
    ajStrAssignS(&ret->Remote, orig->Remote);
    ajStrAssignS(&ret->Label, orig->Label);
                 
    ret->Protein = orig->Protein;     
    ret->Start   = orig->Start;
    ret->End     = orig->End;
    ret->Start2  = orig->Start2;
    ret->End2    = orig->End2;
    ret->Score   = orig->Score;
    ret->Strand  = orig->Strand;
    ret->Frame   = orig->Frame;
    ret->Flags   = orig->Flags;
    ret->Group   = orig->Group;
    ret->Exon    = orig->Exon;
                   
    return ret;
}       
