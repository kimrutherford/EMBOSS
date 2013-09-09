/* @source union application
**
** Read a list of sequences, combine them into one sequence and write it
**
** @author Copyright (C) Peter Rice
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

static ajulong union_GetOverlap (const AjPSeq first,
                                 const AjPSeq second);

static void union_CopyFeatures (const AjPFeattable old_feattable,
                                AjPFeattable new_feattable,
                                ajulong offset,
                                const AjPFeature source_feature);




/* @prog union *************************************************************
**
** Reads sequence fragments and builds one sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq    = NULL;
    AjPSeq uniseq = NULL;
    AjPStr unistr = NULL;
    AjBool first = ajTrue;
    AjBool feature;
    AjBool source;
    AjBool findoverlap;
    AjPFeattable new_feattable = NULL;
    ajulong offset = 0;
    AjPSeq prev_seq = NULL;
    AjPFile overlap_file = NULL;

    AjPFeattable old_feattable = NULL;
    AjPFeature source_feature = NULL;

    AjPStr source_str = ajStrNew();
    AjPStr type_str = ajStrNew();
    char strand='+';
    ajint frame=0;
    float score = 0.0;
    ajulong overlap_base_count = 0;

    embInit ("union", argc, argv);

    feature = ajAcdGetBoolean("feature");
    source  = ajAcdGetBoolean("source");
    /* if true, search for overlaps */
    findoverlap = ajAcdGetBoolean("findoverlap");
    /* file to write overlap base counts to */
    overlap_file = ajAcdGetOutfile ("overlapfile");

    seqout = ajAcdGetSeqout("outseq");
    seqall = ajAcdGetSeqall("sequence");

    while(ajSeqallNext(seqall, &seq))
    {

	ajStrAssignC(&source_str, "union");
	ajStrAssignC(&type_str, "source");

	if(first)
	{
	    uniseq = ajSeqNewSeq(seq);

	    if(feature)
	    {
		new_feattable = ajFeattableNewSeq(seq);
		uniseq->Fttable = new_feattable;
	    }
	}

	ajSeqTrim(seq);

	if(findoverlap)
	{
	    if(!first)
		overlap_base_count = union_GetOverlap (prev_seq, seq);

	    if(overlap_file)
		ajFmtPrintF (overlap_file, "%Ld\n", overlap_base_count);
	}

	if(feature)
	{
	    old_feattable = ajSeqGetFeatCopy(seq);
	    source_feature = NULL;

	    if(source)
	    {
		source_feature = ajFeatNew(new_feattable, source_str, type_str,
					   ajStrGetLen(unistr) -
					   (ajint)overlap_base_count + 1,
					   ajStrGetLen(unistr) +
					   ajSeqGetLen(seq) -
					   (ajint)overlap_base_count,
					   score, strand, frame);
		ajFeatTagAddCC(source_feature, "origid",
			       ajStrGetPtr(seq->Name));
	    }

	    if(old_feattable)
		union_CopyFeatures(old_feattable, new_feattable,
				   offset - overlap_base_count,
				   source_feature);

	    offset += ajSeqGetLen(seq) - overlap_base_count;
	}

	ajStrAppendSubS(&unistr, ajSeqGetSeqS(seq), (ajint) overlap_base_count,
			ajSeqGetLen(seq) - 1);

	if(!first)
	    ajSeqDel (&prev_seq);

	first = ajFalse;

	prev_seq = ajSeqNewSeq(seq);

	ajStrDel(&source_str);
	ajStrDel(&type_str);
    }

    ajSeqAssignSeqS(uniseq, unistr);

    ajSeqoutWriteSeq(seqout, uniseq);
    ajSeqoutClose(seqout);

    if (overlap_file)
	ajFileClose (&overlap_file);


    ajSeqallDel(&seqall);
    ajSeqoutDel(&seqout);
    ajSeqDel(&seq);
    ajSeqDel(&uniseq);
    ajSeqDel(&prev_seq);

    ajStrDel(&unistr);
    ajFeattableDel(&old_feattable);

    embExit();

    return 0;
}




/* @funcstatic union_GetOverlap ***********************************************
**
** returns the number of bases by which two sequences overlap
**
** @param [r] first_seq [const AjPSeq] Undocumented
** @param [r] second_seq [const AjPSeq] Undocumented
** @return [ajulong] Undocumented
** @@
** FIXME - replace with an EMBOSS library call
******************************************************************************/

static ajulong union_GetOverlap (const AjPSeq first_seq,
				 const AjPSeq second_seq)
{
    const AjPStr first_seq_str = ajSeqGetSeqS(first_seq);
    const AjPStr second_seq_str = ajSeqGetSeqS(second_seq);

    ajint i = ajSeqGetLen(first_seq);

    const char * first_str = ajStrGetPtr(first_seq_str);
    const char * second_str = ajStrGetPtr(second_seq_str);

    if (i > (ajint)ajStrGetLen(second_seq_str))
	i = ajStrGetLen(second_seq_str);

    for (; i >= 0 ; --i)
	if(memcmp(&first_str[ajStrGetLen(first_seq_str)-i],second_str,i) == 0)
	    return i;

    return 0;
}




/* @funcstatic union_CopyFeatures  ********************************************
**
** Copy features with an offset
**
** @param [r] old_feattable [const AjPFeattable] input feature table
** @param [w] new_feattable [AjPFeattable] output feature table
** @param [r] offset [ajulong] amount to offset all feature locations by
** @param [r] source_feature [const AjPFeature] Source feature to update
** @return [void]
** @@
******************************************************************************/

static void union_CopyFeatures (const AjPFeattable old_feattable,
                                AjPFeattable new_feattable,
                                ajulong offset,
                                const AjPFeature source_feature)
{
    AjPStr outseq_name;
    AjPFeature gf = NULL;
    const AjPStr type = NULL;
    AjPFeature copy = NULL;
    const AjPStr source_feature_type = NULL;

    AjIList iter;

    if(ajFeattableGetSize(old_feattable))
    {
        outseq_name = ajStrNew();

        iter = ajListIterNewread(old_feattable->Features);

        while(!ajListIterDone(iter))
        {
	    gf = ajListIterGet(iter);
	    type = ajFeatGetType(gf);

	    copy = ajFeatNewFeat(gf);
    
	    /* FIXME */
            if(copy->Start)
                copy->Start += (ajuint) offset;
            if(copy->End)
                copy->End += (ajuint) offset;
            if(copy->Start2)
                copy->Start2 += (ajuint) offset;
            if(copy->End2)
                copy->End2 += (ajuint) offset;

	    if (source_feature != NULL)
	    {
	        source_feature_type = ajFeatGetType(source_feature);

	        if (ajStrMatchS(type, source_feature_type) &&
		    ajFeatGetStart(copy) == ajFeatGetStart (source_feature) &&
		    ajFeatGetEnd(copy) == ajFeatGetEnd (source_feature))
	        {

		    if (ajFeatGetTagC(gf,"origid",1,&outseq_name))
		    {
		        /* don't duplicate src features if got already */
		        continue;
		    }
	        }
	    }
    
	    ajFeattableAddNew(new_feattable, copy);
	}
    }

    return;
}
