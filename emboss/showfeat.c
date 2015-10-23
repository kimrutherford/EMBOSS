/* @source showfeat application
**
** Show features of a sequence
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




static void showfeat_ShowFeatSeq(AjPFile outfile, const AjPSeq seq, ajint beg,
				 ajint end, const AjPStr matchsource,
				 const AjPStr matchtype, const AjPStr matchtag,
				 const AjPStr matchvalue,
				 const AjPStr sortlist,
				 ajint width, AjBool joinfeat, AjBool collapse,
				 AjBool forward, AjBool reverse,
				 AjBool unknown, AjBool strand,
				 AjBool source, AjBool position,
				 AjBool type, AjBool tags, AjBool values, 
				 AjBool stricttags, const AjPRange annotation);

static void showfeat_WriteFeat(AjPStr line, char strand, ajint fstart,
			       ajint fend, ajint width, ajint beg, ajint end);

static void showfeat_FeatOut(AjPFile outfile, const AjPStr lineout,
			     char strandout,
			     const AjPStr sourceout, const AjPStr posout,
			     const AjPStr typeout,
			     const AjPStr tagsout, ajint width, AjBool strand,
			     AjBool source, AjBool type, AjBool tags,
			     AjBool position);

static ajint showfeat_CompareFeatSource(const void * a, const void * b);

static ajint showfeat_CompareFeatType(const void * a, const void * b);

static ajint showfeat_CompareFeatPos(const void * a, const void * b);

static AjBool showfeat_MatchPatternTags(const AjPFeature feat,
					const AjPStr tpattern,
					const AjPStr vpattern,
					AjBool stricttags,
					AjPStr *tagstmp, AjBool values);
static void showfeat_AddPos(AjPStr *posout, ajint start, ajint end);




/* @prog showfeat *************************************************************
**
** Show features of a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall = NULL;
    AjPFile outfile = NULL;
    AjPSeq seq = NULL;
    AjPStr matchsource = NULL;
    AjPStr matchtype = NULL;
    AjPStr matchtag = NULL;
    AjPStr matchvalue = NULL;
    AjPStr sortlist = NULL;
    AjBool joinfeat;
    AjBool html;
    AjBool id;
    AjBool description;
    AjBool scale;
    ajint width;
    AjBool collapse;
    AjBool forward;
    AjBool reverse;
    AjBool unknown;
    AjBool strand;
    AjBool source;
    AjBool position;
    AjBool type;
    AjBool tags;
    AjBool values;
    AjBool stricttags;
    AjPRange annotation = NULL;
    
    ajint i;
    ajint beg;
    ajint end;
    AjPStr descriptionline = NULL;

    embInit("showfeat", argc, argv);

    seqall      = ajAcdGetSeqall("sequence");
    outfile     = ajAcdGetOutfile("outfile");
    matchsource = ajAcdGetString("sourcematch");
    matchtype   = ajAcdGetString("typematch");
    matchtag    = ajAcdGetString("tagmatch");
    matchvalue  = ajAcdGetString("valuematch");
    sortlist    = ajAcdGetListSingle("sort");
    joinfeat    = ajAcdGetBoolean("joinfeatures");
    html        = ajAcdGetBoolean("html");
    id          = ajAcdGetBoolean("id");
    description = ajAcdGetBoolean("description");
    scale       = ajAcdGetBoolean("scale");
    width       = ajAcdGetInt("width");
    collapse    = ajAcdGetBoolean("collapse");
    forward     = ajAcdGetBoolean("forward");
    reverse     = ajAcdGetBoolean("reverse");
    unknown     = ajAcdGetBoolean("unknown");
    strand      = ajAcdGetBoolean("strand");
    source      = ajAcdGetBoolean("sourceshow");
    position    = ajAcdGetBoolean("position");
    type        = ajAcdGetBoolean("typeshow");
    tags        = ajAcdGetBoolean("tagshow");
    values      = ajAcdGetBoolean("valueshow");
    stricttags  = ajAcdGetBoolean("stricttags");
    annotation  = ajAcdGetRange("annotation"); 
    
    while(ajSeqallNext(seqall, &seq))
    {
	/* get begin and end positions */
	beg = ajSeqGetBegin(seq)-1;
	end = ajSeqGetEnd(seq)-1;

	/* do the ID name and description */
	if(id)
	{
	    if(html)
		ajFmtPrintF(outfile, "<H2>%S</H2>\n",
			    ajSeqGetNameS(seq));
	    else
		ajFmtPrintF(outfile, "%S\n", ajSeqGetNameS(seq));
	}

	if(description)
	{
	    if(html)
		ajFmtPrintF(outfile, "<H3>%S</H3>\n",
			    ajSeqGetDescS(seq));
	    else
	    {
		descriptionline = ajStrNew();
		ajStrAssignS(&descriptionline, ajSeqGetDescS(seq));
		ajStrFmtWrap(&descriptionline, 80);
		ajFmtPrintF(outfile, "%S\n", descriptionline);
		ajStrDel(&descriptionline);
	    }
	}

	/* the simplest way of formatting for HTML is just to PRE it all */
	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	/* show the scale */
	if(scale && width > 3)
	{
	    ajFmtPrintF(outfile, "|");
	    for(i=0; i<width-2; i++)
		ajFmtPrintF(outfile, "=");

	    ajFmtPrintF(outfile, "| ");
	    if(beg != 0 || end+1 != (ajint)ajSeqGetLen(seq))
		ajFmtPrintF(outfile, "%d-%d of %u\n", beg+1, end+1,
			    ajSeqGetLen(seq));
	    else
		ajFmtPrintF(outfile, "%u\n", ajSeqGetLen(seq));
	}


	/* show the features */
	showfeat_ShowFeatSeq(outfile, seq, beg, end, matchsource,
			     matchtype, matchtag, matchvalue,
			     sortlist, width, joinfeat, collapse, forward,
			     reverse, unknown, strand, source,
			     position, type, tags, values, stricttags,
			     annotation);

	/* end the HTML PRE block */
	if(html)
	    ajFmtPrintF(outfile, "</PRE>");

	/* gratuitous blank line after the sequence details */
	ajFmtPrintF(outfile, "\n");

    }
    
    ajFileClose(&outfile);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&matchsource);
    ajStrDel(&matchtype);
    ajStrDel(&matchtag);
    ajStrDel(&matchvalue);
    ajStrDel(&sortlist);
    ajRangeDel(&annotation);
    ajStrDel(&descriptionline);

    embExit();

    return 0;
}




/* @funcstatic showfeat_ShowFeatSeq *******************************************
**
** Show the sequence features using clunky ascii graphics
**
** @param [u] outfile [AjPFile] output file
** @param [r] seq [const AjPSeq] sequence
** @param [r] beg [ajint] sequence start position
** @param [r] end [ajint] sequence end position
** @param [r] matchsource [const AjPStr] source pattern to display
** @param [r] matchtype [const AjPStr] type pattern to display
** @param [r] matchtag [const AjPStr] tag pattern to display
** @param [r] matchvalue [const AjPStr] tag's value pattern to display
** @param [r] sortlist [const AjPStr] type of sorting of features to do
** @param [r] width [ajint] width of line of features
** @param [r] joinfeat [AjBool] join multi-location features on one line
** @param [r] collapse [AjBool] show all features on separate lines
** @param [r] forward [AjBool] show forward sense features
** @param [r] reverse [AjBool] show reverse sense features
** @param [r] unknown [AjBool] show unknown sense features
** @param [r] strand [AjBool] show strand of feature
** @param [r] source [AjBool] show source of feature
** @param [r] position [AjBool] show position of feature
** @param [r] type [AjBool] show type of feature
** @param [r] tags [AjBool] show tags and values of feature
** @param [r] values [AjBool] show tag values of feature
** @param [r] stricttags [AjBool] only show those tags that
**                                match the specified patterns
** @param [r] annotation [const AjPRange] annotation range object
** @return [void]
** @@
******************************************************************************/

static void showfeat_ShowFeatSeq(AjPFile outfile, const AjPSeq seq, ajint beg,
				 ajint end, const AjPStr matchsource,
				 const AjPStr matchtype, const AjPStr matchtag,
				 const AjPStr matchvalue,
				 const AjPStr sortlist,
				 ajint width, AjBool joinfeat, AjBool collapse,
				 AjBool forward, AjBool reverse,
				 AjBool unknown, AjBool strand,
				 AjBool source, AjBool position,
				 AjBool type, AjBool tags, AjBool values, 
				 AjBool stricttags, const AjPRange annotation)
{
    AjIList    iter = NULL ;
    AjIList    itersub = NULL ;
    AjPFeature gf   = NULL ;
    AjPFeature gfsub   = NULL ;
    void **array   = NULL ;
    AjPFeature *fullfeats   = NULL ;
    ajuint *featindex = NULL;
    const AjPFeattable feat;
    AjPList featlist = NULL;
    AjPList fullfeatlist = NULL;
    AjPStr lineout;
    char strandout   = '\0';
    AjPStr sourceout = NULL;
    AjPStr typeout   = NULL;
    AjPStr tagstmp   = NULL;
    AjPStr tagsout   = NULL;
    AjPStr posout    = NULL;

    ajuint  count;	/* annotation range count */
    ajuint  rstart;	/* annotation range start */
    ajuint  rend;	/* annotation range end */
    AjPStr ann_text;	/* annotation range text */

    AjBool gotoutput = ajFalse;		 /* have a line to output */

    AjPStrTok tokens = NULL;
    AjPStr key = NULL;
    AjBool val = ajFalse;

    ajuint i;
    ajuint sizei;
    ajuint sizej;
    ajuint j = 0;

    /* get the feature table of the sequence */
    feat = ajSeqGetFeat(seq);
    if(!feat)
	return;

    lineout = ajStrNew();
    tagstmp = ajStrNewC("");
    tagsout = ajStrNewC("");
    posout  = ajStrNewC("");

    if(ajFeattableGetSize(feat))
    {
        sizei = (ajuint) ajListToarray(feat->Features, (void***) &array);
        fullfeats = (AjPFeature*) array;

        featlist = ajListNew();
        AJCNEW0(featindex, sizei);

        sizej = 0;
        for(i=0;fullfeats[i];i++)
        {
            ajListPushAppend(featlist, fullfeats[i]);
            featindex[sizej++] = i;
        }

	if(!ajStrCmpC(sortlist, "source"))
	    /* sort by: sense, source, type, start */
	    ajListToindex(featlist, featindex, &showfeat_CompareFeatSource);
	else if(!ajStrCmpC(sortlist, "start"))
	    /* sort by: sense, start, type, source */
	    ajListToindex(featlist, featindex, &showfeat_CompareFeatPos);
	else if(!ajStrCmpC(sortlist, "type"))
	    /* type */
	    /* sort by: sense, type, source, start */
	    ajListToindex(featlist, featindex, &showfeat_CompareFeatType);
	/* else - no sort */

	if(!ajStrCmpC(sortlist, "nosort"))
        {
            iter = ajListIterNewread(feat->Features);
        }
        else 
        {
            fullfeatlist = ajListNew();
            j = 0;
            for(i=0;i<sizej;i++) 
            {
                j = featindex[i];
                ajListPushAppend(fullfeatlist, fullfeats[j]);
            }
            iter = ajListIterNewread(fullfeatlist);
        }
        AJFREE(featindex);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

            ajStrSetClear(&tagstmp);
	    /* check that we want to output this match of source, type */
	    if(!showfeat_MatchPatternTags(gf, matchtag, matchvalue,
					  stricttags, &tagstmp, values))
                continue;
            if(!embMiscMatchPatternDelimC(ajFeatGetSource(gf),
                                          matchsource,",;|"))
		continue;

            if(ajStrGetLen(matchtype))
            {
                val = ajFalse;
                tokens = ajStrTokenNewC(matchtype, " \t\n\r,;|");

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
                    continue;
            }
            
/* check that the feature is within the range we wish to display */
	    if(beg+1 > (ajint)ajFeatGetEnd(gf) || 
	       end+1 < (ajint)ajFeatGetStart(gf))
		continue;

            if(ajListGetLength(gf->Subfeatures))
            {
                if(joinfeat)
                {
                    if(gotoutput)
                    {
                        showfeat_FeatOut(outfile,
                                         lineout, strandout, sourceout,
                                         posout, typeout, tagsout,
                                         width, strand,
                                         source, type, tags, position);
                        gotoutput = ajFalse;
                    }

                    ajStrSetClear(&lineout);
                    ajStrAppendCountK(&lineout, ' ', width);
                    ajStrAssignS(&sourceout, ajFeatGetSource(gf));
                    ajStrAssignS(&typeout, ajFeatGetType(gf));
                    strandout = ajFeatGetStrand(gf);
                    ajStrSetClear(&tagsout);
                    ajStrSetClear(&posout);
                }

                itersub = ajListIterNewread(gf->Subfeatures);
                while(!ajListIterDone(itersub))
                {
                    gfsub = ajListIterGet(itersub);
                    /* check that we want to output this sense */
                    if(!forward && ajFeatGetStrand(gfsub) == '+')
                        continue;
                    if(!reverse && ajFeatGetStrand(gfsub) == '-')
                        continue;
                    if(!unknown && ajFeatGetStrand(gfsub) == '\0')
                        continue;
                    /* ignore remote IDs */
                    if(!ajFeatIsLocal(gfsub))
                        continue;

                    if(!joinfeat)
                    {
                        if(gotoutput)
                            showfeat_FeatOut(outfile,
                                             lineout, strandout, sourceout,
                                             posout, typeout, tagsout,
                                             width, strand,
                                             source, type, tags, position);

                        /* reset the strings for the new line */
                        ajStrSetClear(&lineout);
                        ajStrAppendCountK(&lineout, ' ', width);
                        ajStrAssignS(&sourceout, ajFeatGetSource(gfsub));
                        ajStrAssignS(&typeout, ajFeatGetType(gfsub));
                        strandout = ajFeatGetStrand(gfsub);
                        ajStrSetClear(&tagsout);
                        ajStrSetClear(&posout);

                    }

                    /* something to output */
                    gotoutput = ajTrue;

                    /* append current tags to tagsout */
                    ajStrAppendS(&tagsout, tagstmp);
                    ajStrSetClear(&tagstmp);

                    /* add positions to posout */
                    showfeat_AddPos(&posout,
                                    ajFeatGetStart(gfsub),
                                    ajFeatGetEnd(gfsub));

                    /* write the feature on the line */
                    showfeat_WriteFeat(lineout, strandout,
                                       ajFeatGetStart(gfsub),
                                       ajFeatGetEnd(gfsub),
                                       width, beg, end);
                }

                ajListIterDel(&itersub);

                if(gotoutput)
                {
                    showfeat_FeatOut(outfile,
                                     lineout, strandout, sourceout,
                                     posout, typeout, tagsout,
                                     width, strand,
                                     source, type, tags, position);
                    gotoutput = ajFalse;
                }
            }
            else
            {
                /* check that we want to output this sense */
                if(!forward && ajFeatGetStrand(gf) == '+')
                    continue;
                if(!reverse && ajFeatGetStrand(gf) == '-')
                    continue;
                if(!unknown && ajFeatGetStrand(gf) == '\0')
                    continue;

                /* ignore remote IDs */
                if(!ajFeatIsLocal(gf))
                    continue;

		if(!collapse)
                {
                    if(gotoutput)
                        showfeat_FeatOut(outfile,
                                         lineout, strandout, sourceout,
                                         posout, typeout, tagsout,
                                         width, strand,
                                         source, type, tags, position);

                    /* reset the strings for the new line */
                    ajStrSetClear(&lineout);
                    ajStrAppendCountK(&lineout, ' ', width);
                    ajStrAssignS(&sourceout, ajFeatGetSource(gf));
                    ajStrAssignS(&typeout, ajFeatGetType(gf));
                    strandout = ajFeatGetStrand(gf);
                    ajStrSetClear(&tagsout);
                    ajStrSetClear(&posout);

                    /* something to output */
                    gotoutput = ajTrue;
                }

                /* append current tags to tagsout */
                ajStrAppendS(&tagsout, tagstmp);
                ajStrSetClear(&tagstmp);

                /* add positions to posout */
                showfeat_AddPos(&posout, ajFeatGetStart(gf),
                                ajFeatGetEnd(gf));

                /* write the feature on the line */
                showfeat_WriteFeat(lineout, strandout,
                                   ajFeatGetStart(gf),
                                   ajFeatGetEnd(gf),
                                   width, beg, end);
            }
	}

	/* print out any last line */
	if(gotoutput)
	    showfeat_FeatOut(outfile, lineout, strandout, sourceout, posout,
			     typeout, tagsout, width, strand, source, type,
			     tags, position);

        /* 
        ** Now do the annotation range object, if we have one
        */
        if (annotation && ajRangeGetSize(annotation)) {
            ann_text = ajStrNew();
            gotoutput = ajFalse;
            for(count = 0; count < ajRangeGetSize(annotation); count++)
            {
                ajRangeElementGetValues(annotation, count, &rstart, &rend);

	    /* check that the feature is within the range we wish to
               display */
	        if(beg+1 > (ajint) rend || end+1 < (ajint) rstart)
		    continue;

                /* get the annotation text */                 
                ajRangeElementGetText(annotation, count, &ann_text);

           /* don't start a new line if collapse and previous text is
              the same */
                if (!collapse || ajStrCmpCaseS(ann_text, typeout)) {
		    if(gotoutput)
                        showfeat_FeatOut(outfile, lineout, strandout,
					 sourceout, posout,
					 typeout, tagsout, width, strand,
					 source, type, tags, position);

		    /* reset the strings for the new line */
		    ajStrSetClear(&lineout);
		    ajStrAppendCountK(&lineout, ' ', width);
		    ajStrAssignC(&sourceout, "Annotation");
                    ajStrAssignS(&typeout, ann_text);
		    strandout = '\0';
		    ajStrSetClear(&tagsout);
		    ajStrSetClear(&posout);
		    /* something to output */
		    gotoutput = ajTrue;
		}

	        /* add positions to posout */
	        showfeat_AddPos(&posout, rstart, rend);

	        /* write the feature on the line */
	        showfeat_WriteFeat(lineout, strandout, rstart, rend, width,
			       beg, end);
	    }
	    /* print out any last line */
	    if(gotoutput)
	        showfeat_FeatOut(outfile, lineout, strandout,
				 sourceout, posout,
				 typeout, tagsout, width, strand, source, type,
				 tags, position);

            ajStrDel(&ann_text);
        }

	ajListIterDel(&iter) ;

    }

    ajStrDel(&tagstmp);
    ajStrDel(&tagsout);
    ajStrDel(&posout);
    ajStrDel(&lineout);
    ajStrDel(&sourceout);
    ajStrDel(&typeout);
    ajListFree(&featlist);
    ajListFree(&fullfeatlist);
    AJFREE(array);

    return;
}




/* @funcstatic showfeat_WriteFeat *********************************************
**
** Show the sequence features using clunky ascii graphics
**
** @param [w] line [AjPStr] Line of ASCII graphics to write in
** @param [r] strand [char] strand
** @param [r] fstart [ajint] start of feature
** @param [r] fend [ajint] end of feature
** @param [r] width [ajint] width of line of features
** @param [r] beg [ajint] sequence start position
** @param [r] end [ajint] sequence end position
** @return [void]
** @@
******************************************************************************/

static void showfeat_WriteFeat(AjPStr line, char strand, ajint fstart,
			       ajint fend, ajint width, ajint beg, ajint end)
{
    ajint i;
    ajint len;
    ajint pos1;
    ajint pos2;
    float tf;
    
    len  = end-beg+1;
    tf = ((float)(fstart-beg)/(float)len)*width-(float)1;
    pos1 = (ajint) tf;
    tf = ((float)(fend-beg)/(float)len)*width-(float)1;
    pos2 = (ajint) tf;

    /* write the '-'s */
    for(i=pos1; i<pos2; i++)
	if(i >= 0 && i < width)
	    /* don't overwrite any characters except space */
	    if(ajStrGetCharPos(line,i) == ' ')
		ajStrPasteCountK(&line, i, '-', 1);

    /* write the end characters */
    if(pos1 >= 0 && pos1 < width)
    {
	if(strand == '+')
	    ajStrPasteCountK(&line, pos1, '|', 1);
	else if(strand == '-')
	    ajStrPasteCountK(&line, pos1, '<', 1);
	else
	    ajStrPasteCountK(&line, pos1, '|', 1);
    }

    if(pos2 >= 0 && pos2 < width)
    {
	if(strand == '+')
	    ajStrPasteCountK(&line, pos2, '>', 1);
	else if(strand == '-')
	    ajStrPasteCountK(&line, pos2, '|', 1);
	else
	    ajStrPasteCountK(&line, pos2, '|', 1);

    }

    return;
}




/* @funcstatic showfeat_FeatOut ***********************************************
**
** Show the sequence features  and source, type, etc.
** We guarantee to not have trailing whitespace at the end of a line.
**
** @param [w] outfile [AjPFile] output file
** @param [r] lineout [const AjPStr] ASCII graphics line
** @param [r] strandout [char] strand of feature
** @param [r] sourceout [const AjPStr] source of feature
** @param [r] posout [const AjPStr] positions of feature
** @param [r] typeout [const AjPStr] type of feature
** @param [r] tagsout [const AjPStr] tags string
** @param [r] width [ajint] width of graphics lines
** @param [r] strand [AjBool] show strand of feature
** @param [r] source [AjBool] show source of feature
** @param [r] type [AjBool] show type of feature
** @param [r] tags [AjBool] show tags and values of feature
** @param [r] position [AjBool] show positions of feature
** @return [void]
** @@
******************************************************************************/

static void showfeat_FeatOut(AjPFile outfile,
			     const AjPStr lineout, char strandout,
			     const AjPStr sourceout, const AjPStr posout,
			     const AjPStr typeout,
			     const AjPStr tagsout, ajint width, AjBool strand,
			     AjBool source, AjBool type, AjBool tags,
			     AjBool position)
{
    /* don't display the graphics lines if they have a width less than 4 */
    if(width > 3)
    {
	ajFmtPrintF(outfile, "%S", lineout);

	if(strand || source || type || tags || position)
	    ajFmtPrintF(outfile, " ");
    }


    if(strand)
    {
	if(strandout == '+')
	    ajFmtPrintF(outfile, "+");
	else if(strandout == '-')
	    ajFmtPrintF(outfile, "-");
	else
	    ajFmtPrintF(outfile, "0");

	if(source || type || tags || position)
	    ajFmtPrintF(outfile, " ");
    }

    if(source)
    {
	ajFmtPrintF(outfile, "%S", sourceout);

	if(type || tags || position)
	    ajFmtPrintF(outfile, " ");
    }


    if(position)
    {
	ajFmtPrintF(outfile, "%S", posout);

	if(type || tags)
	    ajFmtPrintF(outfile, " ");
    }

    if(type)
    {
	ajFmtPrintF(outfile, "%S", typeout);

	if(tags)
	    ajFmtPrintF(outfile, " ");
    }


    if(tags)
	ajFmtPrintF(outfile, "%S", tagsout);


    ajFmtPrintF(outfile, "\n");

    return;
}




/* @funcstatic showfeat_CompareFeatSource *************************************
**
** Compare two feature node to sort as: sense, source, type, start
**
** @param [r] a [const void *] First node
** @param [r] b [const void *] Second node
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showfeat_CompareFeatSource(const void * a, const void * b)
{
    ajint val;

    const AjPFeature c;
    const AjPFeature d;


    c = *(AjPFeature const *)a;
    d = *(AjPFeature const *)b;

    /* sort by strand */
    if(ajFeatGetStrand(c) == ajFeatGetStrand(d))
    {
	/* stands are the same, sort by source */
	val = ajStrCmpCaseS(ajFeatGetSource(c), ajFeatGetSource(d));

	if(val != 0)
	    return val;


	/* source is the same, sort by type */
	val = ajStrCmpCaseS(ajFeatGetType(c), ajFeatGetType(d));
	if(val != 0)
	    return val;


	/* type is the same, sort by start */
	return (ajFeatGetStart(c) - ajFeatGetStart(d));
    }
    else if(ajFeatGetStrand(c) == '+')
	return -1;
    else if(ajFeatGetStrand(c) == '\0' && ajFeatGetStrand(d) == '-')
	return -1;

    return 1;
}




/* @funcstatic showfeat_CompareFeatType ***************************************
**
** Compare two feature node to sort as: sense, type, source, start
**
** @param [r] a [const void *] First node
** @param [r] b [const void *] Second node
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showfeat_CompareFeatType(const void * a, const void * b)
{
    ajint val;

    const AjPFeature c;
    const AjPFeature d;

    c = *(AjPFeature const *)a;
    d = *(AjPFeature const *)b;

    /* sort by strand */
    if(ajFeatGetStrand(c) == ajFeatGetStrand(d))
    {
	/* stands are the same, sort by type */
	val = ajStrCmpCaseS(ajFeatGetType(c), ajFeatGetType(d));
	if(val != 0)
	    return val;

	/* type is the same, sort by source */
	val = ajStrCmpCaseS(ajFeatGetSource(c), ajFeatGetSource(d));
	if(val != 0)
	    return val;

	/* source is the same, sort by start */
	return (ajFeatGetStart(c) - ajFeatGetStart(d));
    }
    else if(ajFeatGetStrand(c) == '+')
	return -1;
    else if(ajFeatGetStrand(c) == '\0' && ajFeatGetStrand(d) == '-')
	return -1;

    return 1;
}




/* @funcstatic showfeat_CompareFeatPos ****************************************
**
** Compare two feature node to sort as: sense, start, type, source
**
** @param [r] a [const void *] First node
** @param [r] b [const void *] Second node
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showfeat_CompareFeatPos(const void * a, const void * b)
{
    ajint val;

    const AjPFeature c;
    const AjPFeature d;

    c = *(AjPFeature const *)a;
    d = *(AjPFeature const *)b;

    /* sort by strand */
    if(ajFeatGetStrand(c) == ajFeatGetStrand(d))
    {
	/* strands are the same, sort by start */
	val = ajFeatGetStart(c) - ajFeatGetStart(d);
	if(val != 0)
	    return val;

	/* starts are the same, sort by type */
	val = ajStrCmpCaseS(ajFeatGetType(c), ajFeatGetType(d));
	if(val != 0)
	    return val;

	/* type is the same, sort by source */
	val = ajStrCmpCaseS(ajFeatGetSource(c), ajFeatGetSource(d));
	return val;
    }
    else if(ajFeatGetStrand(c) == '+')
	return -1;
    else if(ajFeatGetStrand(c) == '\0' && ajFeatGetStrand(d) == '-')
	return -1;

    return 1;
}




/* @funcstatic showfeat_MatchPatternTags **************************************
**
** Checks for a match of the tagpattern and valuepattern to at least one
** tag=value pair and returns the tag/value pairs ready for display in tagsout
**
** @param [r] feat [const AjPFeature] Feature to process
** @param [r] tpattern [const AjPStr] tags pattern to match with
** @param [r] vpattern [const AjPStr] values pattern to match with
** @param [r] stricttags [AjBool] remove any tag-value pairs that
**                                don't match the patterns
** @param [w] tagstmp [AjPStr *] tags out string
** @param [r] values [AjBool] display values of tags
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

static AjBool showfeat_MatchPatternTags(const AjPFeature feat,
					const AjPStr tpattern,
					const AjPStr vpattern,
					AjBool stricttags,
					AjPStr *tagstmp, AjBool values)
{
    AjIList titer;                      /* iterator for feat */
    AjPStr tagnam = NULL;        /* tag structure */
    AjPStr tagval = NULL;        /* tag structure */
    AjBool val = ajFalse;               /* returned value */
    AjBool tval;                        /* tags result */
    AjBool vval;                        /* value result */

     /*
     **  Even if there are no tags to match, but the patterns are
     **  both '*', then allow this as a match.
     **  There are no tags to add to tagstmp, so just return now.
     */
    if(ajListGetLength(feat->Tags) == 0 && 
       !ajStrCmpC(tpattern, "*") && 
       !ajStrCmpC(vpattern, "*")) 
        val = ajTrue;

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
	{
             /*
	     ** The value can be one or more words and the vpattern could
	     ** be the whole phrase, so test not only
	     ** each word in vpattern against the
	     ** value, but also test to see if there is a
	     ** match of the whole of vpattern
	     ** without spitting it up into words. 
	     */
            vval = (ajStrMatchS(tagval, vpattern) ||
		    embMiscMatchPatternDelimC(tagval, vpattern,",;|"));
	}

        if(tval && vval)
	{
            val = ajTrue;
             /* 
	     ** Got a match, add it to the tagstmp string
	     ** If explicitly asked for a translation tag,
	     ** then we get it appended.
	     ** A match to tpattern='*' is not explicitly
	     ** asking for a translation tag.
	     ** So display if tpattern != '*' or tagnam != "translation"
	     */
	    if(ajStrCmpC(tpattern, "*") || ajStrCmpC(tagnam, "translation"))
	    {
		if(values == ajTrue)
		    ajFmtPrintAppS(tagstmp, " %S=\"%S\"", tagnam, tagval);
		else
		    ajFmtPrintAppS(tagstmp, " %S", tagnam);
	    }
        }
	else
	{
             /* 
	     ** Not got a match, add it to the tagstmp string
	     ** anyway if not 'stricttags'
	     */
            if(!stricttags)
	    {
                 /*
		 **  Don't display the translation tag - it is far too long
		 */
	        if(ajStrCmpC(tagnam, "translation"))
	        {
	            if(values == ajTrue)
		        ajFmtPrintAppS(tagstmp, " %S=\"%S\"", tagnam, tagval);
	            else
		        ajFmtPrintAppS(tagstmp, " %S", tagnam);
	        }
            }	
        }
    }
    ajListIterDel(&titer);

     /*
     ** If no match, then clear the tagstmp string
     */
    if(!val)
        ajStrSetClear(tagstmp);

    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return val;
}




/* @funcstatic showfeat_AddPos ************************************************
**
** writes the positions to the positions string
**
** @param [w] posout [AjPStr *] position string
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
**
** @return [void]
** @@
******************************************************************************/

static void showfeat_AddPos(AjPStr *posout, ajint start, ajint end)
{

    /* if the string already has positions in, separate them with commas */
    if(ajStrGetLen(*posout) > 0)
	ajFmtPrintAppS(posout, ",");

    ajFmtPrintAppS(posout, "%d-%d", start, end);

    return;
}



