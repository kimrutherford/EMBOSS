/* @source showseq application
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




static void showseq_FormatShow(EmbPShow ss,
			       const AjPStr format, const AjPTrn trnTable,
			       const AjPRange translaterange,
			       const AjPRange uppercase,
			       const AjPRange highlight, AjBool threeletter,
			       AjBool numberseq, const AjPFeattable feat,
			       ajint orfminsize, const AjPList restrictlist,
			       AjBool plasmid, AjBool flat,
			       const AjPRange annotation);

static void showseq_read_equiv(AjPFile equfile, AjPTable table);
static void showseq_read_file_of_enzyme_names(AjPStr *enzymes);
static AjBool showseq_MatchFeature(const AjPFeature gf,
				   AjPFeature newgf,
				   const AjPStr matchsource,
				   const AjPStr matchtype,
				   ajint matchsense, float minscore,
				   float maxscore, const AjPStr matchtag,
				   const AjPStr matchvalue, AjBool *tagsmatch, 
				   AjBool stricttags);
static AjBool showseq_MatchPatternTags(const AjPFeature gf,
				       AjPFeature newgf, 
				       const AjPStr tpattern,
				       const AjPStr vpattern,
				       AjBool stricttags);
static void showseq_FeatureFilter(const AjPFeattable featab,
				  AjPFeattable newfeatab,
				  const AjPStr matchsource,
				  const AjPStr matchtype,
				  ajint matchsense, float minscore,
				  float maxscore, const AjPStr matchtag,
				  const AjPStr matchvalue, AjBool stricttags);
static AjPFeature showseq_FeatCopy(const AjPFeature orig);


#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "embossre.equ"
#define EQUGUESS 3500     /* Estimate of number of equivalent names */




/* @prog showseq **************************************************************
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
    AjPStr tablename;
    ajint table = 0;
    AjPRange translaterange;
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
    ajint orfminsize;
    AjBool flat;
    AjPTrn trnTable;
    AjPRange annotation;

    /* holds ACD or constructed format for output */
    AjPStr format;
    ajint i;

    /* stuff lifted from Alan's 'restrict.c' */
    AjPStr enzymes = NULL;
    ajint mincuts;
    ajint maxcuts;
    ajint sitelen;
    AjBool single;
    AjBool blunt;
    AjBool sticky;
    AjBool ambiguity;
    AjBool plasmid;
    AjBool commercial;
    AjBool limit;
    AjPFile enzfile  = NULL;
    AjPFile equfile  = NULL;
    AjPTable retable = NULL;
    ajint hits;
    AjPList restrictlist = NULL;

    /* feature filter criteria */
    AjPStr matchsource = NULL;
    AjPStr matchtype   = NULL;
    ajint matchsense;
    float minscore;
    float maxscore;
    AjPStr matchtag   = NULL;
    AjPStr matchvalue = NULL;


    embInit("showseq", argc, argv);

    seqall         = ajAcdGetSeqall("sequence");
    outfile        = ajAcdGetOutfile("outfile");
    formatname     = ajAcdGetListSingle("format");
    thinglist      = ajAcdGetList("things");
    tablename      = ajAcdGetListSingle("table");
    translaterange = ajAcdGetRange("translate");
    uppercase      = ajAcdGetRange("uppercase");
    highlight      = ajAcdGetRange("highlight");
    annotation     = ajAcdGetRange("annotation");
    threeletter    = ajAcdGetBool("threeletter");
    numberseq      = ajAcdGetBool("number");
    width          = ajAcdGetInt("width");
    length         = ajAcdGetInt("length");
    margin         = ajAcdGetInt("margin");
    nameseq        = ajAcdGetBool("name");
    description    = ajAcdGetBool("description");
    offset         = ajAcdGetInt("offset");
    html           = ajAcdGetBool("html");
    orfminsize     = ajAcdGetInt("orfminsize");
    flat           = ajAcdGetBool("flatreformat");

    /*  restriction enzyme stuff */
    mincuts    = ajAcdGetInt("mincuts");
    maxcuts    = ajAcdGetInt("maxcuts");
    sitelen    = ajAcdGetInt("sitelen");
    single     = ajAcdGetBool("single");
    blunt      = ajAcdGetBool("blunt");
    sticky     = ajAcdGetBool("sticky");
    ambiguity  = ajAcdGetBool("ambiguity");
    plasmid    = ajAcdGetBool("plasmid");
    commercial = ajAcdGetBool("commercial");
    limit      = ajAcdGetBool("limit");
    enzymes    = ajAcdGetString("enzymes");

    /* feature filter criteria */
    matchsource = ajAcdGetString("matchsource");
    matchtype   = ajAcdGetString("matchtype");
    matchsense  = ajAcdGetInt("matchsense");
    minscore    = ajAcdGetFloat("minscore");
    maxscore    = ajAcdGetFloat("maxscore");
    matchtag    = ajAcdGetString("matchtag");
    matchvalue  = ajAcdGetString("matchvalue");
    stricttags  = ajAcdGetBool("stricttags");


    format = ajStrNew();
    
    /* read the local file of enzymes names */
    showseq_read_file_of_enzyme_names(&enzymes);
    
    /* check that the translate range is ordered */
    if(!ajRangeOrdered(translaterange))
	ajFatal("Translation ranges are not in ascending, "
		"non-overlapping order.");
    
    
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
	ajStrAssignC(&format, "B N T S B 1 A F ");
    else if(ajStrMatchC(formatname, "5"))
	ajStrAssignC(&format, "B N T S B 1 2 3 A F ");
    else if(ajStrMatchC(formatname, "6"))
	ajStrAssignC(&format, "B N T S B 1 2 3 T -3 -2 -1 A F ");
    else if(ajStrMatchC(formatname, "7"))
	ajStrAssignC(&format, "B R S N T C -R B 1 2 3 T -3 -2 -1 A ");
    else if(ajStrMatchC(formatname, "8"))
	ajStrAssignC(&format, "B 1 2 3 N T R S T C -R T -3 -2 -1 A F ");
    else
	ajFatal("Invalid format type: %S", formatname);
    
    
    /* make the format upper case */
    ajStrFmtUpper(&format);
    
    /* create the translation table */
    trnTable = ajTrnNewI(table);
    
    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqGetBegin(seq)-1;
	end   = ajSeqGetEnd(seq)-1;

	restrictlist = ajListNew();

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
        showseq_FeatureFilter(feattab, newfeattab, matchsource, matchtype,
			      matchsense, minscore, maxscore, matchtag,
			      matchvalue, stricttags);


	/*
	**  most of this is lifted from the program 'restrict.c' by Alan
	**  Bleasby
	*/
	if(ajStrFindC(format, "R") != -1)
	{
	    if(single)
		maxcuts = mincuts = 1;
	    retable = ajTablestrNewLen(EQUGUESS);
	    ajFileDataNewC(ENZDATA, &enzfile);

	    if(!enzfile)
		ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");

	    if(limit)
	    {
		ajFileDataNewC(EQUDATA,&equfile);
		if(!equfile)
		    limit = ajFalse;
		else
		{
		    showseq_read_equiv(equfile, retable);
		    ajFileClose(&equfile);
		}
	    }

	    ajFileSeek(enzfile, 0L, 0);
	    hits = embPatRestrictMatch(seq, 1, ajSeqGetLen(seq),
				       enzfile, enzymes,
				       sitelen, plasmid, ambiguity, mincuts,
				       maxcuts, blunt, sticky, commercial,
				       restrictlist);
	    if(hits)
	    {
		/* this bit is lifted from printHits */
		embPatRestrictRestrict(restrictlist, hits, !limit,
				       ajFalse);
		if(limit)
		    embPatRestrictPreferred(restrictlist,retable);
	    }


	    ajFileClose(&enzfile);
	}



	/* make the Show Object */
	ss = embShowNew(seq, begin, end, width, length, margin, html, offset);

	/* get the number of the genetic code used */
	ajStrToInt(tablename, &table);

	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	showseq_FormatShow(ss, format, trnTable, translaterange,
			   uppercase, highlight, threeletter,
			   numberseq, newfeattab, orfminsize,
			   restrictlist, plasmid, flat, annotation);

	embShowPrint(outfile, ss);

	embShowDel(&ss);

	ajListFree(&restrictlist);
	ajFeattableDel(&newfeattab);
	ajFeattableDel(&feattab);

	/* add a newline at the end of the sequence */
	ajFmtPrintF(outfile, "\n");

	if(html)
	    ajFmtPrintF(outfile, "<PRE>");
    }
    

    ajStrDel(&format);
    ajFileClose(&outfile);
    ajTrnDel(&trnTable);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDelarray(&thinglist);
    ajStrDel(&formatname);
    ajStrDel(&tablename);
    ajStrDel(&enzymes);
    ajStrDel(&matchsource);
    ajStrDel(&matchtype);
    ajStrDel(&matchtag);
    ajStrDel(&matchvalue);
    ajRangeDel(&translaterange);
    ajRangeDel(&uppercase);
    ajRangeDel(&highlight);
    ajRangeDel(&annotation);

    embExit();

    return 0;
}




/* @funcstatic showseq_FormatShow *********************************************
**
** Set up the EmbPShow object, according to the required format
**
** @param [u] ss [EmbPShow] Show sequence object
** @param [r] format [const AjPStr] format codes for the required
**                       things to display
** @param [r] trnTable [const AjPTrn] genetic code translation table
** @param [r] translaterange [const AjPRange] ranges to translate
** @param [r] uppercase [const AjPRange] ranges to uppercase
** @param [r] highlight [const AjPRange] ranges to colour in HTML
** @param [r] threeletter [AjBool] use 3-letter code
** @param [r] numberseq [AjBool] put numbers on sequences
** @param [r] feat [const AjPFeattable] sequence's feature table
**                                 NULL after - pointer stored internally
** @param [r] orfminsize [ajint] minimum size of ORFs to display
**                              (0 for no ORFs)
** @param [r] restrictlist [const AjPList] restriction enzyme site list
**                            (or NULL) NULL after - pointer stored internally
** @param [r] plasmid [AjBool] Circular (plasmid) sequence
** @param [r] flat [AjBool] show restriction sites in flat format
** @param [r] annotation [const AjPRange] ranges to annotate
** @return [void]
** @@
******************************************************************************/

static void showseq_FormatShow(EmbPShow ss,
			       const AjPStr format, const AjPTrn trnTable,
			       const AjPRange translaterange,
			       const AjPRange uppercase,
			       const AjPRange highlight,  AjBool threeletter,
			       AjBool numberseq, const AjPFeattable feat,
			       ajint orfminsize, const AjPList restrictlist,
			       AjBool plasmid, AjBool flat,
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
	else if(!ajStrCmpC(code, "1"))
	    embShowAddTran(ss, trnTable, 1, threeletter, numberseq,
			   translaterange, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if(!ajStrCmpC(code, "2"))
	    embShowAddTran(ss, trnTable, 2, threeletter, numberseq,
			   NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if(!ajStrCmpC(code, "3"))
	    embShowAddTran(ss, trnTable, 3, threeletter, numberseq,
			   NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if(!ajStrCmpC(code, "-1"))
	    embShowAddTran(ss, trnTable, -1, threeletter, numberseq,
			   NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if(!ajStrCmpC(code, "-2"))
	    embShowAddTran(ss, trnTable, -2, threeletter, numberseq,
			   NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if(!ajStrCmpC(code, "-3"))
	    embShowAddTran(ss, trnTable, -3, threeletter, numberseq,
			   NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if(!ajStrCmpC(code, "T"))
	    embShowAddTicks(ss);
	else if(!ajStrCmpC(code, "N"))
	    embShowAddTicknum(ss);
	else if(!ajStrCmpC(code, "C"))
	    embShowAddComp(ss, numberseq);
	else if(!ajStrCmpC(code, "F"))
	    embShowAddFT(ss, feat);
	else if(!ajStrCmpC(code, "R"))
	    embShowAddRE(ss, 1, restrictlist, plasmid, flat);
	else if(!ajStrCmpC(code, "-R"))
	    embShowAddRE(ss, -1, restrictlist, plasmid, flat);
	else if(!ajStrCmpC(code, "A"))
	    embShowAddNote(ss, annotation);
	else
	    ajFatal("Formatting code not recognised: '%S'", code);
    }

    ajStrDel(&code);
    ajStrTokenDel(&tok);

    return;
}




/* @funcstatic showseq_read_equiv *********************************************
**
** reads the equivalents file.
**
** @param [u] equfile [AjPFile] file to read
** @param [w] table [AjPTable] table to write to.
** @return [void]
** @@
******************************************************************************/

static void showseq_read_equiv(AjPFile equfile, AjPTable table)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    const char *p;

    line = ajStrNew();

    while(ajFileReadLine(equfile,&line))
    {
        p = ajStrGetPtr(line);

        if(!*p || *p=='#' || *p=='!')
            continue;

        p = ajSysFuncStrtok(p," \t\n");
        key = ajStrNewC(p);
        p = ajSysFuncStrtok(NULL," \t\n");
        value = ajStrNewC(p);
        ajTablePut(table,(void *)key, (void *)value);
    }

    return;
}




/* @funcstatic showseq_read_file_of_enzyme_names ******************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [u] enzymes [AjPStr*] names of enzymes to search for or 'all'
**                              or '@file'
** @return [void]
** @@
******************************************************************************/

static void showseq_read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file = NULL;
    AjPStr line;
    const char *p = NULL;

    if(ajStrFindC(*enzymes, "@") == 0)
    {
	ajStrTrimC(enzymes, "@");	/* remove the @ */
	file = ajFileNewIn(*enzymes);
	if(file == NULL)
	    ajFatal("Cannot open the file of enzyme names: '%S'", enzymes);

	/* blank off the enzyme file name and replace with the enzyme names */
	ajStrSetClear(enzymes);
	line = ajStrNew();
	while(ajFileReadLine(file, &line))
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




/* @funcstatic showseq_FeatureFilter ******************************************
**
** Removes unwanted features from a feature table
**
** @param [r] featab [const AjPFeattable] Feature table to filter
** @param [u] newfeatab [AjPFeattable] Retured table of filtered features
** @param [r] matchsource [const AjPStr] Required Source pattern
** @param [r] matchtype [const AjPStr] Required Type pattern
** @param [r] matchsense [ajint] Required Sense pattern +1,0,-1
**                               (or other value$
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] matchtag [const AjPStr] Required Tag pattern
** @param [r] matchvalue [const AjPStr] Required Value pattern
** @param [r] stricttags [AjBool] If false then only display tag/values
**                                that match the criteria
** @return [void]
** @@
******************************************************************************/

static void showseq_FeatureFilter(const AjPFeattable featab,
				  AjPFeattable newfeatab,
				  const AjPStr matchsource,
				  const AjPStr matchtype,
				  ajint matchsense, float minscore,
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
            newgf = showseq_FeatCopy(gf); /* copy of gf that we can add */
	    /* required tags to */
	    if(showseq_MatchFeature(gf, newgf, matchsource, matchtype,
				    matchsense,
				    minscore, maxscore, matchtag,
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




/* @funcstatic showseq_MatchFeature *******************************************
**
** Test if a feature matches a set of criteria
**
** @param [r] gf [const AjPFeature] Feature to test
** @param [u] newgf [AjPFeature] Copy of feature with filtered
**                                    Tag/Value list
** @param [r] source [const AjPStr] Required Source pattern
** @param [r] type [const AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value$
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

static AjBool showseq_MatchFeature(const AjPFeature gf, AjPFeature newgf,
				   const AjPStr source, const AjPStr type,
				   ajint sense,
				   float minscore, float maxscore,
				   const AjPStr tag, const AjPStr value,
				   AjBool *tagsmatch, AjBool stricttags)
{
    AjBool scoreok;

    scoreok = (minscore < maxscore);

     /*
     ** is this a child of a join() ?
     ** if it is a child, then we use the previous result of MatchPatternTags
     */
    if(!ajFeatIsMultiple(gf) || !ajFeatIsChild(gf))
	*tagsmatch = showseq_MatchPatternTags(gf, newgf, tag, value,
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
    if(!embMiscMatchPattern(ajFeatGetSource(gf), source) ||
       !embMiscMatchPattern(ajFeatGetType(gf), type) ||
       (ajFeatGetStrand(gf) == '+' && sense == -1) ||
       (ajFeatGetStrand(gf) == '-' && sense == +1) ||
       (scoreok && ajFeatGetScore(gf) < minscore) ||
       (scoreok && ajFeatGetScore(gf) > maxscore) ||
       !*tagsmatch)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic showseq_MatchPatternTags ***************************************
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

static AjBool showseq_MatchPatternTags(const AjPFeature gf,
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
            /*
            ** The value can be one or more words and the vpattern could
            ** be the whole phrase, so test not only each word in vpattern
	    ** against the value, but also test to see if there is a match
	    ** of the whole of vpattern without spitting it up into words.
            */
            vval = (ajStrMatchS(tagval, vpattern) ||
		    embMiscMatchPattern(tagval, vpattern));


        if(tval && vval)
	{
            val = ajTrue;
	    /*
	    ** if strict tags then only want to see the tags
	    ** that match the criteria
	    */
	    if(stricttags)
	    	ajFeatTagAdd(newgf, tagnam, tagval);
        }

        /* if not strict tags then need to see all the tags */
        if(!stricttags)
            ajFeatTagAdd(newgf, tagnam, tagval);
    }
    ajListIterDel(&titer);

    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return val;
}




/* @funcstatic showseq_FeatCopy ***********************************************
**
** Makes a copy of a feature, except for the Tags list which is added later.
** This is mostly copied from the original in ajFeatCopy,
** but without the Tags stuff.
**
** @param [r]   orig  [const AjPFeature]  Original feature
** @return [AjPFeature] New copy of  feature
** @@
******************************************************************************/

static AjPFeature showseq_FeatCopy(const AjPFeature orig) 
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
