/* @Source embshow.c
**
** General routines for sequence display.
** Copyright (c) 2000 Gary Williams
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
#include <ctype.h>




/*
**
** TO ADD A NEW DISPLAY TYPE:
**
**
**
** Example: complement of the sequence
**
** Create a new type in the descriptor object types 'enum ShowEValtype'.
** eg: SH_COMP
** Use this to refer to this type in later routines. (eg: by embShowAddComp)
**
** Create a structure to hold information about what options for this type
** you can have.
** eg: typedef struct EmbSShowComp { ... }
**
** Create a function called by the user to set up the new type as the next
** thing to be displayed in the list of things.
** eg: embShowAddComp
**
** Create the routine to actually output a line's length of whatever is
** being displayed from position 'pos' of the sequence - the output is
** added to the end of a list of AjPStr.
** Everything to be printed should be ajListPushAppend'd on to the 'lines' list.
** These strings need not be complete lines - you can push many strings of
** partial lines if you prefer.
** End the lines to be output by pushing a string ending with a '\n'.
** As many lines as you wish may be pushed onto the lines list.
** eg: showFillComp
**
** Add a case statement to showFillLines to call the showFill* routine.
** eg:
**      case SH_COMP:
**      showFillComp(this, lines, info, pos);
**      break;
*/




static void    showFillRE(const EmbPShow thys, AjPList lines,
			  EmbPShowRE info, ajuint pos,ajuint last);
static void    showFillREflat(const EmbPShow thys,
			      AjPList lines, const EmbPShowRE info,
			      ajuint pos,ajuint last);
static void    showFillREupright(const EmbPShow thys, AjPList lines,
				 EmbPShowRE info, ajuint pos,ajuint last);
static ajint   showFillREuprightSort(const void* a, const void* b);
static void    showOverPrint(AjPStr *target, ajint start, AjPStr insert);
static AjBool  showLineIsClear(AjPStr *line, ajint start, ajint end);
static void    showFillLines(AjPList lines, const EmbPShow thys,
			     ajuint pos, ajuint last);
static void    showPrintLines(AjPFile out, const AjPList lines);
static void    showMargin(const EmbPShow thys, AjPList lines);
static void    showMarginNumber(const EmbPShow thys,
				AjPList lines, ajint number);
static void    showPad(AjPList lines, ajint number);
static void    showInsertHTML(AjPStr *target, ajuint pos, const AjPStr insert);

static void    showFillSeq(const EmbPShow thys,
			   AjPList lines, const EmbPShowSeq info,
			   ajuint pos,ajuint last);
static void    showFillBlank(const EmbPShow thys,
			     AjPList lines, const EmbPShowBlank info,
			     ajuint pos,ajuint last);
static void    showFillTicks(const EmbPShow thys,
			     AjPList lines, const EmbPShowTicks info,
			     ajuint pos,ajuint last);
static void    showFillTicknum(const EmbPShow thys, AjPList lines,
			       const EmbPShowTicknum info,
			       ajuint pos,ajuint last);
static void    showFillComp(const EmbPShow thys,
			    AjPList lines, const EmbPShowComp info,
			    ajuint pos,ajuint last);
static void    showFillTran(const EmbPShow thys,
			    AjPList lines, EmbPShowTran info,
			    ajuint pos,ajuint last);
static void    showFillFT(const EmbPShow thys,
			  AjPList lines, const EmbPShowFT info,
			  ajuint pos,ajuint last);
static void    showFillNote(const EmbPShow thys,
			    AjPList lines, const EmbPShowNote info,
			    ajuint pos,ajuint last);

static void showDelSeq(EmbPShowSeq* pinfo);
static void showDelBlank(EmbPShowBlank* pinfo);
static void showDelTicks(EmbPShowTicks* pinfo);
static void showDelTicknum(EmbPShowTicknum* pinfo);
static void showDelComp(EmbPShowComp* pinfo);
static void showDelTran(EmbPShowTran* pinfo);
static void showDelRE(EmbPShowRE* pinfo);
static void showDelFT(EmbPShowFT* pinfo);
static void showDelNote(EmbPShowNote* pinfo);
static void showAddTags(AjPStr *tagsout, const AjPFeature feat, AjBool values);



/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Show Sequence Constructors ****************************************
**
** All constructors return a new show sequence object by pointer.
** The target pointer does not need to be initialised to NULL, but it is
** good programming practice to do so anyway.
**
******************************************************************************/




/* @func embShowNew ***********************************************************
**
** Creates a new sequence show object.
**
** @param [r] seq [const AjPSeq] Sequence to describe
** @param [r] begin [ajint] start position in  sequence
** @param [r] end [ajint] end position in  sequence
** @param [r] width [ajint] width of displayed sequence on a line
** @param [r] length [ajint] length of a page in lines (0=no length)
** @param [r] margin [ajint] margin for numbers etc.
** @param [r] html [AjBool] format output for HTML
** @param [r] offset [ajint] number to start display of position numbering at
** @return [EmbPShow] New sequence show object.
** @@
******************************************************************************/

EmbPShow embShowNew(const AjPSeq seq, ajint begin, ajint end, ajint width,
		    ajint length, ajint margin, AjBool html, ajint offset)
{
    EmbPShow pthis;

    AJNEW0(pthis);

    pthis->list = ajListNew();

    /* information about the sequence */
    pthis->seq     = seq;
    pthis->nucleic = ajSeqIsNuc(seq); /* ajTrue = sequence is nucleic */
    pthis->start   = begin;
    pthis->end     = end;

    /* information about the page layout */
    pthis->width  = width;	     /* length of sequence per line */
    pthis->length = length;    	     /* length of a page (0 = indefinite) */
    pthis->margin = margin;	     /* margin for numbering */
    pthis->html   = html;	     /* ajTrue = format for HTML */
    pthis->offset = offset;	     /* number to start displaying with */

    return pthis;
}




/* @funcstatic showInfoNew ****************************************************
**
** Creates a new descriptor structure to be pushed on the list
**
** @param [r] info [void*] descriptor
** @param [r] type [ajint] type of descriptor
** @return [EmbPShowInfo] New sequence show object.
** @@
******************************************************************************/

static EmbPShowInfo showInfoNew(void* info, ajint type)
{
    EmbPShowInfo pthis;

    AJNEW0(pthis);

    pthis->info = info;
    pthis->type = type;

    return pthis;
}




/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Show Sequence Destructors *****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the show sequence object.
**
******************************************************************************/




/* @func embShowDel ***********************************************************
**
** Deletes a show sequence object.
**
** @param [d] pthis [EmbPShow*] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowDel(EmbPShow* pthis)
{
    EmbPShow thys;
    AjIList iter;
    EmbPShowInfo infostruct;
    ajint type;				/* descriptor type */
    void *ptr = NULL;

    ajDebug("embShowDel\n");

    /*****  DO NOT: ajSeqDel(&pthis->seq); *****/

    thys = *pthis;

    /* free the descriptors */
    iter = ajListIterNewread(thys->list);

    while((infostruct = ajListIterGet(iter)) != NULL)
    {

	/* iterate through the descriptors filling out the lines */
	type = infostruct->type;

	switch(type)
	{
	case SH_SEQ:
	    showDelSeq((EmbPShowSeq*) &infostruct->info);
	    break;

	case SH_BLANK:
	    showDelBlank((EmbPShowBlank*) &infostruct->info);
	    break;

	case SH_TICK:
	    showDelTicks((EmbPShowTicks*) &infostruct->info);
	    break;

	case SH_TICKNUM:
	    showDelTicknum((EmbPShowTicknum*) &infostruct->info);
	    break;

	case SH_COMP:
	    showDelComp((EmbPShowComp*) &infostruct->info);
	    break;

	case SH_TRAN:
	    showDelTran((EmbPShowTran*) &infostruct->info);
	    break;

	case SH_RE:
	    showDelRE((EmbPShowRE*) &infostruct->info);
	    break;

	case SH_FT:
	    showDelFT((EmbPShowFT*) &infostruct->info);
	    break;

	case SH_NOTE:
	    showDelNote((EmbPShowNote*) &infostruct->info);
	    break;

	default:
	    ajFatal("Unknown descriptor type found in embShowDel: %d",
		    type);
	}


	AJFREE(infostruct);
    }

    ajListIterDel(&iter);

    /* we have already freed the descriptors, so use ajListFree here */
    while(ajListPop(thys->list,(void **)&ptr));
    ajListFree(&thys->list);

    AJFREE(*pthis);

    return;
}




/* @funcstatic showDelSeq *****************************************************
**
** Deletes a show sequence descriptor object.
**
** @param [d] pinfo [EmbPShowSeq*] Show sequence descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelSeq(EmbPShowSeq* pinfo)
{
    AJFREE(*pinfo);

    return;
}




/* @funcstatic showDelBlank ***************************************************
**
** Deletes a show blank descriptor object.
**
** @param [d] pinfo [EmbPShowBlank*] Show blank descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelBlank(EmbPShowBlank* pinfo)
{
  AJFREE(*pinfo);

  return;
}




/* @funcstatic showDelTicks ***************************************************
**
** Deletes a show ticks descriptor object.
**
** @param [d] pinfo [EmbPShowTicks*] Show ticks descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelTicks(EmbPShowTicks* pinfo)
{
  AJFREE(*pinfo);

  return;
}




/* @funcstatic showDelTicknum *************************************************
**
** Deletes a show tick numbers descriptor object.
**
** @param [d] pinfo [EmbPShowTicknum*] Show tick numbers descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelTicknum(EmbPShowTicknum* pinfo)
{
    AJFREE(*pinfo);

    return;
}




/* @funcstatic showDelComp ****************************************************
**
** Deletes a show complement descriptor object.
**
** @param [d] pinfo [EmbPShowComp*] Show complement descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelComp(EmbPShowComp* pinfo)
{
    AJFREE(*pinfo);

    return;
}




/* @funcstatic showDelTran ****************************************************
**
** Deletes a show translation descriptor object.
**
** @param [d] pinfo [EmbPShowTran*] Show translation descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelTran(EmbPShowTran* pinfo)
{
    EmbPShowTran info;

    info = *pinfo;

    /* AJB: Why was the seqdel commented out? Memory leak without it */
    ajSeqDel(&info->transeq);
    AJFREE(*pinfo);

    return;
}




/* @funcstatic showDelRE ******************************************************
**
** Deletes a show restriction enzyme descriptor object.
**
** @param [d] pinfo [EmbPShowRE*] Show restriction enzyme descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelRE(EmbPShowRE* pinfo)
{
    void *ptr;
    EmbPShowRE info;

    info = *pinfo;

    ajListFree(&info->matches);		/* the nodes are elsewhere */
 
    while(ajListPop(info->sitelist,(void **)&ptr))
	AJFREE(ptr);

    ajListFree(&info->sitelist);

    AJFREE(*pinfo);

    return;
}




/* @funcstatic showDelFT ******************************************************
**
** Deletes a show feature table descriptor object.
**
** @param [d] pinfo [EmbPShowFT*] Show feature table descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelFT(EmbPShowFT* pinfo)
{
    EmbPShowFT info;

    info = *pinfo;

    ajFeattableDel(&info->feat);   /* cloned pointer in showseq etc.*/
    AJFREE(*pinfo);

    return;
}




/* @funcstatic showDelNote ****************************************************
**
** Deletes a show annotation region descriptor object.
**
** @param [d] pinfo [EmbPShowNote*] Show annotation region descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelNote(EmbPShowNote* pinfo)
{
    AJFREE(*pinfo);

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Show Sequence Assignments *****************************************
**
** These functions add to the show sequence object provided as the
** first argument.
**
******************************************************************************/




/* @func embShowAddSeq ********************************************************
**
** Adds the sequence to be displayed to the list of things to show
** This must be done before the final printing is done as without a sequence
** to hang all the other features and things on, there can be no output to
** show.
**
** @param [u] thys [EmbPShow] Show sequence object
** @param [r] number [AjBool] Number the sequence
** @param [r] threeletter [AjBool] Use three letter protein code
** @param [r] upperrange [const AjPRange] Range of sequence to uppercase
** @param [r] colour [const AjPRange] Range of sequence to colour in HTML
** @return [void]
** @@
******************************************************************************/

void embShowAddSeq(EmbPShow thys, AjBool number, AjBool threeletter,
		   const AjPRange upperrange, const AjPRange colour)
{
    EmbPShowSeq info;

    AJNEW0(info);

    info->number      = number;
    info->threeletter = threeletter; /* use three-letter protein code */
    info->upperrange  = upperrange;  /* Range of sequence to uppercase */
    info->highlight   = colour;	     /* Range to colour in HTML */

    ajListPushAppend(thys->list, showInfoNew(info, SH_SEQ));

    return;
}




/* @func embShowAddBlank ******************************************************
**
** Adds a blank line to the list of things to show.
**
** @param [u] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowAddBlank(EmbPShow thys)
{
    EmbPShowBlank info;

    ajDebug("embShowAddBlank\n");

    AJNEW0(info);

    ajListPushAppend(thys->list, showInfoNew(info, SH_BLANK));

    return;
}




/* @func embShowAddTicks ******************************************************
**
** Adds a ticks line to the list of things to show.
**
** @param [u] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowAddTicks(EmbPShow thys)
{

    EmbPShowTicks info;

    ajDebug("embShowAddTicks\n");

    AJNEW0(info);

    ajListPushAppend(thys->list, showInfoNew(info, SH_TICK));

    return;
}




/* @func embShowAddTicknum ****************************************************
**
** Adds a ticks number line to the list of things to show.
**
** @param [u] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowAddTicknum(EmbPShow thys)
{
    EmbPShowTicknum info;

    ajDebug("embShowAddTicknum\n");

    AJNEW0(info);

    ajListPushAppend(thys->list, showInfoNew(info, SH_TICKNUM));

    return;
}




/* @func embShowAddComp *******************************************************
**
** Adds the sequence complement to be displayed to the list of things to show
**
** @param [u] thys [EmbPShow] Show sequence object
** @param [r] number [AjBool] ajTrue = number the complement
** @return [void]
** @@
******************************************************************************/

void embShowAddComp(EmbPShow thys, AjBool number)
{
    EmbPShowComp info;

    ajDebug("embShowAddComp\n");

    AJNEW0(info);

    info->number = number;

    ajListPushAppend(thys->list, showInfoNew(info, SH_COMP));

    return;
}




/* @func embShowAddTran *******************************************************
**
** Adds the translation to be displayed to the list of things to show
**
** @param [u] thys [EmbPShow] Show sequence object
** @param [r] trnTable [const AjPTrn] Translation table
** @param [r] frame [ajint] Reading frame to translate
** @param [r] threeletter [AjBool] ajTrue for 3 letter code
** @param [r] number [AjBool] ajTrue for numbering
** @param [r] regions [const AjPRange] Sequence range(s)
** @param [r] orfminsize [ajint] Minimum length of ORF to be shown
** @param [r] lcinterorf [AjBool] ajTrue to put inter-orf regions in lowercase
** @param [r] firstorf [AjBool] ajTrue beginning of the seq is a possible ORF
** @param [r] lastorf [AjBool] ajTrue end of the seq is a possible ORF
** @param [r] showframe [AjBool] ajTrue write the frame number
** @return [void]
** @@
******************************************************************************/

void embShowAddTran(EmbPShow thys, const AjPTrn trnTable, ajint frame,
		    AjBool threeletter, AjBool number, const AjPRange regions,
		    ajint orfminsize, AjBool lcinterorf, AjBool firstorf,
		    AjBool lastorf, AjBool showframe)
{
    EmbPShowTran info;

    ajDebug("embShowAddTran\n");

    AJNEW0(info);

    info->trnTable = trnTable;		/* translation table */
    info->frame    = frame;             /* 1,2,3,-1,-2 or -3 = frame to
					   translate */
    info->threeletter = threeletter;    /* ajTrue = display in 3 letter code */
    info->regions     = regions;        /* only translate these regions,
					   NULL = all */
    info->number = number;	        /* ajTrue = number the translation */
    info->orfminsize = orfminsize;      /* minimum size of ORF to display */

    /* these are used by showFillTran */
    info->transeq = NULL;               /* not yet stored the translation
					   here */
    info->tranpos = 0;	                /* store translation position for
					   numbering */
    info->lcinterorf = lcinterorf;      /* ajTrue = put the inter-orf regions
					   in lower case */
    info->firstorf  = firstorf;
    info->lastorf   = lastorf;
    info->showframe = showframe;

    ajListPushAppend(thys->list, showInfoNew(info, SH_TRAN));

    return;
}




/* @func embShowAddRE *********************************************************
**
** Adds the Restriction Enzymes to be displayed to the list of things to show
**
** @param [u] thys [EmbPShow] Show sequence object
** @param [r] sense [ajint] sense to translate (+1 or -1)
** @param [r] restrictlist [const AjPList] restriction enzyme cut site list
** @param [r] plasmid [AjBool] Circular (plasmid) sequence
** @param [r] flat [AjBool] show in flat format with recognition sites
** @return [void]
** @@
******************************************************************************/

void embShowAddRE(EmbPShow thys, ajint sense, const AjPList restrictlist,
		  AjBool plasmid, AjBool flat)
{
    EmbPShowRE info;
    ajDebug("embShowAddRE\n");
    
    AJNEW0(info);
    
    info->sense = sense;		/* 1 or -1 = sense to translate */
    info->flat = flat;		        /* upright or flat display */
    info->hits = ajListGetLength(restrictlist);
    info->matches = ajListNewListref(restrictlist);
    info->sitelist = NULL;	        /* show we have not yet created this
					   list */
    info->plasmid = plasmid;

    ajListPushAppend(thys->list, showInfoNew(info, SH_RE));
    
    return;
}




/* @func embShowAddFT *********************************************************
**
** Adds the Features to be displayed to the list of things to show
**
** @param [u] thys [EmbPShow] Show sequence object
** @param [r] feat [const AjPFeattable] features
** @return [void]
** @@
******************************************************************************/

void embShowAddFT(EmbPShow thys, const AjPFeattable feat)
{
    EmbPShowFT info;

    ajDebug("embShowAddFT\n");

    AJNEW0(info);

    info->feat = ajFeattableCopy(feat); /* store the feature table */
  
    ajListPushAppend(thys->list, showInfoNew(info, SH_FT));

    return;
}




/* @func embShowAddNote *******************************************************
**
** Adds the annotations to be displayed to the list of things to show
**
** @param [u] thys [EmbPShow] Show sequence object
** @param [r] regions [const AjPRange] Sequence range(s)
** @return [void]
** @@
******************************************************************************/

void embShowAddNote(EmbPShow thys, const AjPRange regions)
{
    EmbPShowNote info;
    ajDebug("embShowAddNote\n");

    AJNEW0(info);

    info->regions = regions;		/* regions to note */

    ajListPushAppend(thys->list, showInfoNew(info, SH_NOTE));

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Show Sequence Modifiers *******************************************
**
** These functions modify the behaviour of the last show descriptor on
** the list.
**
**
******************************************************************************/






/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Show Sequence Operators *******************************************
**
** These functions use the contents of a Show object but do not modify it.
**
******************************************************************************/




/* @func embShowPrint *********************************************************
**
** Prints a Show object
**
** @param [u] out [AjPFile] Output file handle
** @param [r] thys [const EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowPrint(AjPFile out, const EmbPShow thys)
{
    AjPList lines;		    /* list of lines to be printed */
    ajuint pos;		            /* current printing position in sequence */
    ajuint start;
    ajuint end;
    AjIList liter;		    /* iterator for lines */
    AjPStr line;
    ajuint count   = 0;		    /* count of newlines in the list */
    ajuint line_no = 0;		    /* line number on page */
    ajuint last = 0;

    ajDebug("embShowPrint\n");

    /* set up the start and end positions to print */
    start = thys->start;
    end = thys->end;
    last = start-1;

    /* run through the whole sequence, line-width by line-width */
    for(pos = start; pos<=end; pos += thys->width)
    {
	last += thys->width;
	if(last >= end)
	    last = end;

	/* make a new list of lines */
	lines=ajListstrNew();
	/* put the sequence and any other descriptors in the lines list */
	showFillLines(lines, thys, pos, last);

	/* throw a formfeed if we would go over the length of the page */
	count = 0;
	liter = ajListIterNewread(lines);
	while((line = ajListIterGet(liter)) != NULL)
	    if(ajStrGetLen(line))
		if(ajStrGetPtr(line)[ajStrGetLen(line)-1] == '\n')
		    count++;
	ajListIterDel(&liter);

	/* thys->length is zero if we have an indefinite page length */
	if(thys->length && (count+line_no > thys->length) &&
	   (count < thys->length))
	{
	    line_no = 0;
	    ajFmtPrintF(out, "%c", '\f'); /* form feed character */
	}

	showPrintLines(out, lines);
	ajListstrFreeData(&lines);
    }

    return;
}




/* @funcstatic showPrintLines *************************************************
**
** Print the lines to the output.
**
** @param [u] out [AjPFile] Output file handle
** @param [r] lines [const AjPList] lines to print
** @return [void]
** @@
******************************************************************************/

static void showPrintLines(AjPFile out, const AjPList lines)
{
    AjIList liter;			/* iterator for lines */
    AjPStr str;

    ajDebug("showPrintLines\n");

    /* iterate through the lines and print them */
    liter = ajListIterNewread(lines);
    while((str = ajListIterGet(liter)) != NULL)
	ajFmtPrintF(out, "%S", str);

    ajListIterDel(&liter);

    return;
}




/* ==================================================================== */
/* ======================== Assignments ================================*/
/* ==================================================================== */


/* @section Show Fill Assignments *********************************************
**
** These functions fill out the sequence and features lines according to the
** descriptor data.
**
******************************************************************************/




/* @funcstatic showFillLines **************************************************
**
** Calls the descriptor routines to fill the lines.
**
** @param [u] lines [AjPList] Lines list
** @param [r] thys [const EmbPShow] Show sequence object
** @param [r] pos [ajuint] position in sequence so far while printing
** @param [r] last [ajuint] final position in sequence so far while printing
** @return [void]
** @@
******************************************************************************/

static void showFillLines(AjPList lines, const EmbPShow thys,
			  ajuint pos, ajuint last)
{
    EmbPShowInfo infostruct;		/* structure of type and descriptor */
    ajint type;				/* descriptor type */
    void * info;			/* descriptor */

    AjIList diter;			/* iterator for descriptors */

    ajDebug("showFillLines\n");

    /* iterate through the descriptors filling out the lines */
    diter = ajListIterNewread(thys->list);

    while((infostruct = ajListIterGet(diter)) != NULL)
    {
	type = infostruct->type;
	info = infostruct->info;

	switch(type)
	{
	case SH_SEQ:
	    showFillSeq(thys, lines, info, pos, last);
	    break;

	case SH_BLANK:
	    showFillBlank(thys, lines, info, pos, last);
	    break;

	case SH_TICK:
	    showFillTicks(thys, lines, info, pos, last);
	    break;

	case SH_TICKNUM:
	    showFillTicknum(thys, lines, info, pos, last);
	    break;

	case SH_COMP:
	    showFillComp(thys, lines, info, pos, last);
	    break;

	case SH_TRAN:
	    showFillTran(thys, lines, info, pos, last);
	    break;

	case SH_RE:
	    showFillRE(thys, lines, info, pos, last);
	    break;

	case SH_FT:
	    showFillFT(thys, lines, info, pos, last);
	    break;

	case SH_NOTE:
	    showFillNote(thys, lines, info, pos, last);
	    break;

	default:
	    ajFatal("Unknown descriptor type found in "
			   "showFillLines: %d",type);
	}
    }
    ajListIterDel(&diter);

    return;
}




/* @funcstatic showMargin *****************************************************
**
** Add a blank margin to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @return [void]
** @@
******************************************************************************/

static void showMargin(const EmbPShow thys, AjPList lines)
{
    AjPStr marginfmt;

    marginfmt = ajStrNewRes(10);

    /* variable width margin */
    if(thys->margin)
    {
	ajFmtPrintS(&marginfmt, "%%-%ds ", thys->margin-1);
	ajListstrPushAppend(lines, ajFmtStr(ajStrGetPtr(marginfmt), ""));
    }
    ajStrDel(&marginfmt);

    return;
}




/* @funcstatic showMarginNumber ***********************************************
** Add a margin containing a number to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] number [ajint] number to display
** @return [void]
** @@
******************************************************************************/

static void showMarginNumber(const EmbPShow thys, AjPList lines, ajint number)
{
    AjPStr marginfmt;

    marginfmt = ajStrNewRes(10);

    /* variable width margin containing a number */
    if(thys->margin)
    {
	ajFmtPrintS(&marginfmt, "%%%dd ", thys->margin-1);
	ajListstrPushAppend(lines, ajFmtStr(ajStrGetPtr(marginfmt), number));
    }
    ajStrDel(&marginfmt);

    return;
}




/* @funcstatic showPad ********************************************************
**
** Add a set of space characters to the lines list to pad out an output line
**
** @param [u] lines [AjPList] list of lines to add to
** @param [r] number [ajint] number of space characters to output
** @return [void]
** @@
******************************************************************************/

static void showPad(AjPList lines, ajint number)
{
    AjPStr marginfmt;

    marginfmt=ajStrNewRes(10);

    /* variable width pad of spaces */
    if(number>0)
    {
	ajFmtPrintS(&marginfmt, "%%-%ds", number);
	ajListstrPushAppend(lines, ajFmtStr(ajStrGetPtr(marginfmt), ""));
    }
    ajStrDel(&marginfmt);

    return;
}




/* @func embShowUpperRange ****************************************************
**
** Uppercase a string from a sequence with a range
** I.e ranges of a sequence are to be uppercased.
** We have a small region of the original sequence in a string.
** We want to uppercase any bits of the string that are in the ranges.
**
** @param [u] line [AjPStr *] line to uppercase if it is in the ranges
** @param [r] upperrange [const AjPRange] range of original sequence
**                                        to uppercase
** @param [r] pos [ajuint] position in sequence that line starts at
** @return [void]
** @@
******************************************************************************/

void embShowUpperRange(AjPStr * line, const AjPRange upperrange, ajuint pos)
{
    ajint nr;
    ajint i;
    ajuint j;
    ajuint start;			/* start of next range */
    ajuint end;				/* end of next range */
    ajint value;     /* code for type of overlap of range with line */
    char *p;	      /* ptr to start of range in line to uppercase */

    nr = ajRangeNumber(upperrange);

    for(i=0; i<nr; i++)
    {
	/* for each range in AjPRange upperrange */
	ajRangeValues(upperrange, i, &start, &end);

	/* get type of overlap */
	value = ajRangeOverlapSingle(start, end, pos, ajStrGetLen(*line));


	ajDebug("embShowUpperRange %d %u..%u pos:%u len:%u value:%d\n",
		i, start, end, pos, ajStrGetLen(*line), value);
	/* complete overlap */
	if(value == 2)
	{
	    ajStrFmtUpper(line);
	    return;
	}
	else if(value)
	{
	    /* partial overlap */
	    start--;       /* change start,end from human-readable position*/
	    end--;

	    if(start < pos)
		start = pos;

	    ajDebug("make uppercase start:%u end:%u pos:%u '%S'\n",
		    start, end, pos, *line);
	    p = ajStrGetuniquePtr(line)+start-pos;
	    for(j=start; *p && j<=end; j++, p++)
	    {
		ajDebug("uppercase test pos:%u j:%u pos-j:%u\n",
			pos, j, pos-j);
		if(j-pos < ajStrGetLen(*line))
		{
		    ajDebug("uppercase char %u '%c'\n", pos-j, *p);
		    *p = toupper((ajint) *p);
		}
	    }
	    ajDebug("made uppercase start:%u pos:%u end:%u '%S'\n",
		    start, end, pos, *line);
	}
    }

    return;
}




/* @func embShowColourRange ***************************************************
**
** colour a string from a sequence with a range
** I.e ranges of a sequence are to be coloured in HTML.
** We have a small region of the original sequence in a string.
** We want to colour any bits of the string that are in the ranges.
**
** @param [u] line [AjPStr *] line to colour if it is in the ranges
** @param [r] colour [const AjPRange] range of original sequence to colour
** @param [r] pos [ajuint] position in sequence that line starts at
** @return [void]
** @@
******************************************************************************/

void embShowColourRange(AjPStr * line, const AjPRange colour, ajuint pos)
{
    ajint nr;
    ajint i;
    ajuint start;			/* start of next range */
    ajuint end;				/* end of next range */
    ajint istart;
    ajint iend;
    ajint value;                        /* code for type of overlap of
					   range with line */
    AjPStr html = NULL;
    AjPStr col = NULL;

    nr = ajRangeNumber(colour);

    for(i=0; i<nr; i++)
    {
	/* for each range in AjPRange colour */
	ajRangeValues(colour, i, &start, &end);

	/* get type of overlap */
	value = ajRangeOverlapSingle(start, end, pos, ajStrGetLen(*line));

	/* partial or complete overlap */
	if(value)
	{
	    istart = start - pos - 1;
	    iend = end - pos - 1;

	    if(istart < 0)
		istart = 0;

	    if(iend > (ajint)ajStrGetLen(*line)-1)
		iend = ajStrGetLen(*line)-1;

	    /* start */
	    ajStrAssignC(&html, "<font color=");
	    ajRangeText(colour, i, &col);

	    if(ajStrGetLen(col))
		ajStrAppendS(&html, col);
	    else
	    {
		/* no colour, use 'red' as default */
		ajStrAppendC(&html, "red");
	    }

	    ajStrAppendC(&html, ">");
	    showInsertHTML(line, istart, html);

	    /* end */
	    ajStrAssignC(&html, "</font>");

	    /* end+1 because want the tag after this position */
	    showInsertHTML(line, iend+1, html);
	}
    }
    ajStrDel(&col);
    ajStrDel(&html);

    return;
}




/* @funcstatic showInsertHTML *************************************************
**
** Insert a string at a position in another string
** The position ignores any inserted HTML tags (anything between '<>')
** For example, insert "*" at position 3 of "<html tag>0123"
** gives "<html tag>012*3"
**
** If the insert position is past the end of the string, it inserts at the end.
**
** @param [u] target [AjPStr *] HTMLised string to insert into
** @param [r] pos [ajuint] position (ignoring HTML tags) to insert at
** @param [r] insert [const AjPStr] string to insert
** @return [void]
** @@
******************************************************************************/

static void showInsertHTML(AjPStr *target, ajuint pos, const AjPStr insert)
{
    ajuint i;
    ajuint j;
    AjBool tag = ajFalse;

    /* find the required position, not including tags */
    for(i=0, j=0; j<pos && i<ajStrGetLen(*target); i++)
	if(tag == ajFalse)
	{
	    if(ajStrGetPtr(*target)[i] == '<')
		tag = ajTrue;
	    else
		j++;		    /* count the non-tag characters */
	}
	else
	{
	    if(ajStrGetPtr(*target)[i] == '>')
		tag = ajFalse;
	}

    /* we may have some tags at this position which still need to be skippped */
    while (ajStrGetPtr(*target)[i] == '<') 
    {
    	while (ajStrGetPtr(*target)[i] != '>') 
    	    i++;
        i++;
    }
    
   
    ajStrInsertS(target, i, insert);

    return;
}




/* @funcstatic showFillSeq ****************************************************
**
** Add this line's worth of sequence to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowSeq] data on how to display the sequence data
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillSeq(const EmbPShow thys,
			AjPList lines, const EmbPShowSeq info,
			ajuint pos, ajuint last)
{
    AjPStr line;

    AjPStr line1;	     /* used to make the three-letter codes */
    AjPStr line2;
    AjPStr line3;
    const char *p;
    const char *p3;
    ajuint count;
    ajuint width;

    ajDebug("showFillSeq\n");

    line = ajStrNewRes(81);
    width = last - pos + 1;

    /* variable width margin at left with optional number in it */
    if(info->number)
	showMarginNumber(thys, lines, pos+thys->offset);
    else
	showMargin(thys, lines);

    /* get the bit of the sequence to display */
    ajStrAppendSubS(&line, ajSeqGetSeqS(thys->seq), pos, last);

    /* change to three-letter code */
    if(!thys->nucleic && info->threeletter)
    {

	line1=ajStrNewRes(81);
	line2=ajStrNewRes(81);
	line3=ajStrNewRes(81);
	for(count=0, p=ajStrGetPtr(line);
	    count < ajStrGetLen(line);
	    count++, p++)
	    if(*p == '*')
	    {
		ajStrAppendC(&line1, "*");
		ajStrAppendC(&line2, "*");
		ajStrAppendC(&line3, "*");
	    }
	    else if(!isalpha((ajint)*p))
	    {
		ajStrAppendC(&line1, "?");
		ajStrAppendC(&line2, "?");
		ajStrAppendC(&line3, "?");
	    }
	    else
	    {
		p3 = embPropCharToThree(*p);
		ajStrAppendK(&line1, *p3);
		ajStrAppendK(&line2, *(p3+1));
		ajStrAppendK(&line3, *(p3+2));
	    }

	ajListstrPushAppend(lines, line1);
	ajStrDel(&line);

    }
    else
    {
	/*
	** nucleic or single-letter code 
	** do uppercase ranges
	*/
	if(ajRangeOverlaps(info->upperrange, pos, width))
	    embShowUpperRange(&line, info->upperrange, pos);

	/* do colour ranges if we are displaying HTML*/
	if(thys->html && ajRangeOverlaps(info->highlight, pos, width))
	    embShowColourRange(&line, info->highlight, pos);

	ajListstrPushAppend(lines, line);
    }

    /* optional number at right */
    if(info->number)
    {
	/*
	** if the sequence has ended we might have to fill out the end
	** with blanks
	*/
	if(last >= ajSeqGetLen(thys->seq))
	{
	    showPad(lines, thys->width - ajSeqGetLen(thys->seq) + pos);
	    ajListstrPushAppend(lines,
			     ajFmtStr(" %d",
				      ajSeqGetLen(thys->seq)+thys->offset-1));
	}
	else
	    ajListstrPushAppend(lines,
			     ajFmtStr(" %d",
				      pos+width+thys->offset-1));
    }

    /* end the output line */
    ajListstrPushAppend(lines, ajFmtStr("\n"));


    /* if three-letter code, add the other 3-letter output lines */
    if(!thys->nucleic && info->threeletter)
    {
	showMargin(thys, lines);
	ajListstrPushAppend(lines, line2);
	ajListstrPushAppend(lines, ajFmtStr("\n"));
	showMargin(thys, lines);
	ajListstrPushAppend(lines, line3);
	ajListstrPushAppend(lines, ajFmtStr("\n"));
    }

    return;
}




/* @funcstatic showFillBlank **************************************************
**
** Add a blank line to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowBlank] data on how to display the
**                                       sequence data
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillBlank(const EmbPShow thys,
			  AjPList lines, const EmbPShowBlank info,
			  ajuint pos,ajuint last)
{
    AjPStr line;

    ajDebug("showFillBlank\n");

    (void) thys;			/* make it used */
    (void) info;			/* make it used */
    (void) pos;				/* make it used */
    (void) last;			/* make it used */

    line = ajStrNewRes(2);

    ajStrAssignC(&line, "\n");
    ajListstrPushAppend(lines, line);

    return;
}




/* @funcstatic showFillTicks **************************************************
**
** Add a tick line to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowTicks] data on how to display the
**                                      sequence data
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillTicks(const EmbPShow thys,
			  AjPList lines, const EmbPShowTicks info,
			  ajuint pos,ajuint last)
{
    AjPStr line;
    ajuint i;
    ajuint offset;
    ajuint width;

    ajDebug("showFillTicks\n");

    (void) info;			/* make it used */

    line   = ajStrNewRes(81);
    offset = thys->offset;
    width  = last - pos + 1;


    /* make the ticks line */
    for(i=pos+offset; i<pos+offset+width; i++)
	if(!(i % 10))
	    ajStrAppendC(&line, "|");
	else if(!(i % 5))
	    ajStrAppendC(&line, ":");
	else
	    ajStrAppendC(&line, "-");

    showMargin(thys, lines);
    ajListstrPushAppend(lines, line);

    /* end the output ticks line */
    ajListstrPushAppend(lines, ajFmtStr("\n"));

    return;
}




/* @funcstatic showFillTicknum ************************************************
**
** Add a tick line numbers to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowTicknum] data on how to display
**                 the sequence data
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillTicknum(const EmbPShow thys,
			    AjPList lines, const EmbPShowTicknum info,
			    ajuint pos, ajuint last)
{
    AjPStr line;
    ajuint i;
    ajuint offset;
    ajuint width;
    ajuint pad;

    ajDebug("showFillTicknum\n");

    (void) info;			/* make it used */

    line   = ajStrNewRes(81);
    offset = thys->offset;
    width  = last - pos + 1;

    showMargin(thys, lines);
    pad = 9 - ((pos+offset-1) % 10);
    if(pad)
	showPad(lines, pad);

    for(i=pos + offset + pad; i < pos+offset+width; i+=10)
	ajFmtPrintAppS(&line, "%-10d", i);

    ajListstrPushAppend(lines, line);

    /* end the output line */
    ajListstrPushAppend(lines, ajFmtStr("\n"));

    return;
}




/* @funcstatic showFillComp ***************************************************
**
** Add thys line's worth of sequence complement to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowComp] data on how to display the
**                                     sequence data
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillComp(const EmbPShow thys,
			 AjPList lines, const EmbPShowComp info,
			 ajuint pos, ajuint last)
{
    AjPStr line;
    ajuint width;

    ajDebug("showFillComp\n");

    line = ajStrNewRes(81);
    width = last - pos + 1;

    /*
    ** do a quick check that we have a nucleic sequence
    ** - else just ignore this
    */
    if(!thys->nucleic)
	return;

    /* variable width margin at left with optional number in it */
    if(info->number)
	showMarginNumber(thys, lines, pos+thys->offset);
    else
	showMargin(thys, lines);


    /* get the sequence at this position */
    ajStrAppendSubS(&line, ajSeqGetSeqS(thys->seq), pos, pos+width-1);

    /* get the complement */
    ajSeqstrComplement(&line);
    ajListstrPushAppend(lines, line);

    /* optional number at right */
    if(info->number)
    {
	/*
	** if the sequence has ended we might have to fill
	** out the end with blanks
	*/
	if(last >= ajSeqGetLen(thys->seq))
	{
	    showPad(lines, thys->width - ajSeqGetLen(thys->seq) + pos);
	    ajListstrPushAppend(lines,
			     ajFmtStr(" %d",
				      ajSeqGetLen(thys->seq)+thys->offset-1));
	}
	else
	    ajListstrPushAppend(lines, ajFmtStr(" %d",
					     pos+thys->width+thys->offset-1));
    }

    /* end the output line */
    ajListstrPushAppend(lines, ajFmtStr("\n"));

    return;
}




/* @funcstatic showFillTran ***************************************************
**
** Add this line's worth of sequence translation to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [u] info [EmbPShowTran] data on how to display the
**                          sequence data
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillTran(const EmbPShow thys,
			 AjPList lines, EmbPShowTran info,
			 ajuint pos, ajuint last)
{

    AjPStr line;
    AjPSeq tran   = NULL;
    AjPSeq seq    = NULL; /* local copy of sequence for translating ranges */
    AjPStr temp = NULL;
    AjPStr sajb =NULL;	  /* peptide expanded to 3-let code or by 2 spaces */
    AjPStr transeq =NULL; /* sequence copy for editing */
    ajint frame;
    ajuint framepad = 0; /* no. of spaces to pad to the correct frame pos */
    ajuint linepos;
    ajuint startpos;	  /* number at start of line */
    ajuint endpos;	  /* number at end of line */
    ajuint i;
    ajint j;
    ajint orflast;

    ajDebug("showFillTran\n");

    line = ajStrNewRes(81);

    /*
    **  do a quick check that we have a nucleic sequence - else just
    **  ignore this
    */
    if(!thys->nucleic)
	return;


    /* if the translation has not yet been done, do it now - once only */
    if(!info->transeq)
    {
	/* translate a set of ranges ... */
	if(info->regions && ajRangeNumber(info->regions))
	{
	    framepad = 0;
	    seq = ajSeqNewSeq(thys->seq);
	    ajRangeSeqExtract(info->regions, seq);
	    tran = ajTrnSeqOrig(info->trnTable, seq, 1);
	    ajSeqDel(&seq);

	    /* expand to fill line or change to three-letter code */
	    if(info->threeletter)
	    {
		sajb = embPropProt1to3(tran,framepad);
		ajSeqAssignSeqS(tran, sajb);
	    }
	    else
	    {
		/* pad with 2 spaces after every residue */
		sajb = embPropProtGaps(tran,framepad);
		ajSeqAssignSeqS(tran,sajb);
	    }
	    ajStrDel(&sajb);

	    /*
	    **  now put in spaces to align the translation to the
	    **  sequence ranges
	    */
	    ajRangeSeqStuff(info->regions, tran);
	    ajStrSetClear(&temp);
	}
	else
	{

	    /* ... or just translate in the required frame */

	    /*
	    **  change frames -1 to -3 to frames 4 to 6 for translation
	    **  of complement (NB that really should say just 'complement', not
	    **  'reverse-complement' as we will be putting the resulting
	    **  reversed peptide under the forward nucleic sequence.)
	    */
	    frame = info->frame;
	    if(frame < 0)
		frame = 3-frame;

	    /* do the translation */
	    tran = ajTrnSeqOrig(info->trnTable, thys->seq, frame);

	    /* shift the translation to the correct frame */
	    if(frame == 1 || frame == 5)
		framepad = 0;
	    else if(frame == 2 || frame == 6)
		framepad = 1;
	    else if(frame == 3 || frame == 4)
		framepad = 2;

	    /* convert inter-ORF regions to '-'s or put it in lower case*/
	    orflast = -1;

	    /*
	       for(i=0; i<ajSeqGetLen(tran); i++)
	       if(ajStrGetPtr(ajSeqGetSeqS(tran))[i] == '*')
	       {
		   if(i-orflast < info->orfminsize+1)
		   {
		       if(info->lcinterorf)
			   for(j=orflast+1; j<i; j++)
			       ajStrGetPtr(ajSeqGetSeqS(tran))[j] =
			       tolower((ajint) ajStrGetPtr(ajSeqGetSeqS(tran))[j]);
		       else
			   for(j=orflast+1; j<i; j++)
			       ajStrGetPtr(ajSeqGetSeqS(tran))[j] = '-';
		   }
		   orflast = i;
	       }
	     */

	    /* Thomas version */
	    if(frame < 4)
	    {
		transeq = ajSeqGetSeqCopyS(tran);
		for(i=0; i<ajStrGetLen(transeq); i++)
		    if(ajStrGetCharPos(transeq,i) == '*')
		    {
			if(i-orflast < info->orfminsize+1)
			{
			    if(!(info->firstorf && orflast == -1))
			    {
				j = orflast+1;
				if(info->lcinterorf)
				    ajStrFmtLowerSub(&transeq,j,i-1);
				else
				    ajStrPasteCountK(&transeq,j,'-',i-j);
			    }
			}
			orflast = i;
		    }

		/* put the last ORF in lower case or convert it to -'s */
		if(i == ajStrGetLen(transeq) && !(info->lastorf)  
		   && i-orflast < info->orfminsize+1)
		{
		    j = orflast+1;
		    if(info->lcinterorf)
			ajStrFmtLowerSub(&transeq,j,i-1);
		    else
			ajStrPasteCountK(&transeq,j,'-',i-j);
		}
		ajSeqAssignSeqS(tran, transeq);
		ajStrDel(&transeq);
	    }
	    else /* frame 4,5,6 */
	    {
		transeq = ajSeqGetSeqCopyS(tran);
		for(i=0; i<ajStrGetLen(transeq); i++)
		    if(ajStrGetCharPos(transeq,i) == '*')
		    {
			if(i-orflast < info->orfminsize+1) 
			    if(!(info->lastorf && orflast == -1))
			    {
				j = orflast+1;
				if(info->lcinterorf)
				    ajStrFmtLowerSub(&transeq,j,i-1);
				else
				    ajStrPasteCountK(&transeq,j,'-',i-j);
			    }
			orflast = i;
		    } 

		/* put the first ORF in lower case or convert it to -'s */
		if(i == ajSeqGetLen(tran) && !(info->firstorf) 
		   && i-orflast < info->orfminsize+1)
		{
		    j = orflast+1;
		    if(info->lcinterorf)
			ajStrFmtLowerSub(&transeq,j,i-1);
		    else
			ajStrPasteCountK(&transeq,j,'-',i-j);
		}
		ajSeqAssignSeqS(tran, transeq);
		ajStrDel(&transeq);
	    }
	    
	    /* expand to fill line or change to three-letter code */
	    if(info->threeletter)
	    {
		sajb = embPropProt1to3(tran,framepad);
		ajSeqAssignSeqS(tran,sajb );
	    }
	    else
	    {
		sajb = embPropProtGaps(tran,framepad);

		/* pad with 2 spaces after every residue */
		ajSeqAssignSeqS(tran,sajb );
	    }
	    ajStrDel(&sajb);
	}

	/* store the resulting translation in our descriptor structure */
	info->transeq = tran;
    }


    /* get the sequence at this position */
    ajStrAppendSubS(&line, ajSeqGetSeqS(info->transeq),
		    pos, last);

    /* get the number of the starting and ending amino-acid on this line */
    startpos = info->tranpos;
    endpos = info->tranpos;
    for(linepos=0; linepos<ajStrGetLen(line); linepos++)
	/*
	**  only count the starting letter of 3-letter codes and don't
	**  count *'s
	*/
	if(ajStrGetPtr(line)[linepos] >= 'A' &&
	   ajStrGetPtr(line)[linepos] <= 'Z')
	    info->tranpos++;

    /* less than width in the line? Add blanks to pad it out */
    for(;linepos < thys->width; linepos++)
	ajStrAppendC(&line, " ");


    /* if at least one residue, count it at start */
    if(info->tranpos != endpos)
	startpos++;


    /* variable width margin at left with optional number in it */
    if(info->number)
	showMarginNumber(thys, lines, startpos);
    else
	showMargin(thys, lines);


    /* put the translation line on the output list */
    ajListstrPushAppend(lines, line);

    /* optional number at right */
    if(info->number)
	ajListstrPushAppend(lines, ajFmtStr(" %d", info->tranpos));

    if(info->showframe)
    {
	frame = info->frame;
	if(frame < 0)
	    frame = 3 - frame;
	ajListstrPushAppend(lines, ajFmtStr("%4s%d", "F", frame));
    }

    /* end the output line */
    ajListstrPushAppend(lines, ajFmtStr("\n"));

    return;
}




/* @funcstatic showFillRE *****************************************************
**
** Add this line's worth of Restriction Enzyme cut sites to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [u] info [EmbPShowRE] data on how to display the RE cut sites
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillRE(const EmbPShow thys,
		       AjPList lines, EmbPShowRE info,
		       ajuint pos, ajuint last)
{
    ajDebug("showFillRE\n");

    /*
    **  do a quick check that we have a nucleic sequence - else just ignore
    **  this
    */
    if(!thys->nucleic)
	return;

    if(info->flat)
	showFillREflat(thys, lines, info, pos, last);
    else
	showFillREupright(thys, lines, info, pos, last);

    return;
}




/* @funcstatic showFillREupright **********************************************
**
** Add this line's worth of Restriction Enzyme cut sites to the lines list
** Display in upright sit-up-and-beg format
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [u] info [EmbPShowRE] data on how to display the RE cut sites
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillREupright(const EmbPShow thys,
			      AjPList lines, EmbPShowRE info,
			      ajuint pos, ajuint last)
{
    AjPStr line    = NULL;
    AjPStr newline = NULL;
    AjPStr baseline;			/* line holding first set of ticks */
    AjPList linelist;			/* list of lines to fill */
    ajuint cut;				/* the sites to display */
    AjIList liter;			/* iterator for linelist */
    AjBool freespace;			/* flag: found free space to print */
    EmbPMatMatch m = NULL;		/* restriction enz match structure */
    AjIList miter;			/* iterator for matches list */
    EmbPShowREsite sitenode;		/* site node structure */
    AjIList siter;			/* iterator for sites list */
    EmbPShowREsite s = NULL;		/* site node structure */
    AjPStr tick = NULL;			/* tick "|" string */
    ajint ln;
    AjPStr sajb = NULL;

    linelist = ajListstrNew();
    baseline = ajStrNew();

    /* if not yet produced a sorted list of cut sites, do it now */
    if(info->sitelist == NULL)
    {
	info->sitelist = ajListNew();

	miter = ajListIterNewread(info->matches);
	while((m = ajListIterGet(miter)) != NULL)
	{
	    /* store the first cut site in this sense */
	    if(info->sense == 1)	/* forward sense */
		cut = m->cut1;
	    else			/* reverse sense */
		cut = m->cut2;

	    cut--;

	    AJNEW0(sitenode);
	    sitenode->pos = cut;
	    sitenode->name = m->cod;
	    ajListPushAppend(info->sitelist, sitenode);

	    /* now store the potential second cut site on this sense */
	    if(info->sense == 1)	/* forward sense */
		cut = m->cut3;
	    else			/* reverse sense */
		cut = m->cut4;

	    if(cut)
	    {
		cut--;
		AJNEW0(sitenode);
		sitenode->pos  = cut;
		sitenode->name = m->cod;
		ajListPushAppend(info->sitelist, sitenode);
	    }
	}

	ajListIterDel(&miter);
	ajListSort(info->sitelist, showFillREuprightSort);
    }

    ajStrAssignC(&tick, "|");		/* a useful string */

    /* iterate through the site list */
    siter = ajListIterNewread(info->sitelist);
    while((s = ajListIterGet(siter)) != NULL)
    {
	cut = s->pos;

	/* ignore this match if nothing is to be displayed on this line */
	if(cut >= pos && cut <= last)
	{

	    /* convert to position in the line */
	    cut = cut-pos;

	    /* put tick in base line */
	    showOverPrint(&baseline, cut, tick);

	    /* work up list of lines */
	    freespace = ajFalse;

	    /*
	    **  potentially updating the nodes of linelist, so
	    **  don't just iterate, use ajListstrPop and ajListstrPushApp
	    **  to pop off the bottom and then push the altered node back on
	    **  the top of the list
	    */
	    for(ln = ajListstrGetLength(linelist); ln>0; ln--)
	    {
		ajListstrPop(linelist, &line);
		/*
		**  if not yet written the name in this set of
		**  iterations, see if if it can be done so now
		*/ 
		if(!freespace)
		{
		    /* if name space is clear, write name and break */
		    if(showLineIsClear(&line, cut, cut+ajStrGetLen(s->name)))
		    {
			showOverPrint(&line, cut, s->name);
		
			/* flag to show we have written name */
			freespace = ajTrue;
		    }
		    else
			/*
			**  if cut site position character is space, change
			**  it to a tick
			*/
			if(*(ajStrGetPtr(line)+cut) == ' ')
			    showOverPrint(&line, cut, tick);
		}

		ajListstrPushAppend(linelist, line);
		/* end 'iteration' through lines */
	    }


	    /* if name was not written, append a new line and write name */
	    if(!freespace)
	    {
		newline=ajStrNew();
		showOverPrint(&newline, cut, s->name);
		ajListstrPushAppend(linelist, newline);
	    }
	}
    }
    ajListIterDel(&siter);


    /* convert base line ticks to forward or reverse slashes */
    if(info->sense == 1)		/* forward sense */
	ajStrExchangeSetCC(&baseline, "|", "\\");
    else				/* reverse sense */
	ajStrExchangeSetCC(&baseline, "|", "/");


    /* put base line at start of lines list */
    ajListstrPush(linelist, baseline);

    /*
    **  reverse the order of the lines if we are in the forward sense as
    **  then we get the ticks pointing downwards :-)
    */
    if(info->sense == 1) ajListstrReverse(linelist);

    /* iterate through the lines and print them */
    liter = ajListIterNewread(linelist);
    while((line = ajListIterGet(liter)) != NULL)
    {
	/* output to the lines list */
	/* variable width margin at left */
	/* with optional number in it */
	showMargin(thys, lines);
	/* put the translation line */
	/* on the output list */
	ajListstrPushAppend(lines, line);
	/* end the output line */
	ajListstrPushAppend(lines, ajFmtStr("\n"));
    }
    ajListIterDel(&liter);

    while(ajListstrPop(linelist,&sajb));

    ajListstrFree(&linelist);
    ajStrDel(&tick);

    return;
}




/* @funcstatic showFillREuprightSort ******************************************
**
** Sort routine for showFillREupright - produces reverse cut site order
**
** @param [r] a [const void*] First pos
** @param [r] b [const void*] Second pos
** @return [ajint] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
** @@
******************************************************************************/

static ajint showFillREuprightSort(const void* a, const void* b)
{
    ajint res;

    res = (*(EmbPShowREsite const *)b)->pos -
	(*(EmbPShowREsite const *)a)->pos;

    /* if the cut sites are equal, reverse sort by the length of the name */
    if(!res)
	res = ajStrGetLen((*(EmbPShowREsite const *)b)->name) -
	    ajStrGetLen((*(EmbPShowREsite const *)a)->name);

    return res;
}




/* @funcstatic showFillREflat *************************************************
**
** Add this line's worth of Restriction Enzyme cut sites to the lines list
** Display in flat format with recognition sites
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowRE] data on how to display the RE cut sites
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillREflat(const EmbPShow thys,
			   AjPList lines, const EmbPShowRE info,
			   ajuint pos, ajuint last)
{
    AjPStr line  = NULL;
    AjPStr line2 = NULL;
    AjPList linelist = NULL;		/* list of lines to fill */
    ajuint start;
    ajuint end;  			/* start and end position of site */
    ajuint nameend;			/* end position of name */
    ajuint base;		   /* base position of binding site */
    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    AjIList liter;			/* iterator for linelist */
    AjPStr namestr = NULL;		/* name of RE to insert into line */
    AjPStr sitestr = NULL;		/* binding and cut site to insert */
    ajuint i;
    const char *claimchar = "*";        /* char used to stake a claim to */
					/* that position in the string */
    AjBool freespace;			/* flag for found a free space to
					   print in */
    EmbPMatMatch m = NULL;		/* restriction enzyme match struct */
    AjIList miter;			/* iterator for matches list */
    ajuint ln;
    AjPStr sajb = NULL;
    ajuint width;

    width = last - pos + 1;

    linelist = ajListstrNew();

    /* iterate through the list */
    miter = ajListIterNewread(info->matches);
    while((m = ajListIterGet(miter)) != NULL)
    {
	/* get the start and end positions */
	cut1 = m->cut1;
	/* the display points back '<' at cut pos */
	cut2 = m->cut2+1;
	cut3 = m->cut3;
	cut4 = m->cut4;
	if(m->cut4)
	    cut4++;		/* the display points back '<' at cut pos */
	base = m->start;
	start = base;

	ajDebug("showFillRE start:%d sense:%b plasmid:%b circ12:%b 34:%b\n",
		start, info->sense, info->plasmid, m->circ12, m->circ34);
	ajDebug("           cut1:%d 2:%d 3:%d 4:%d\n",
		cut1, cut2, cut3, cut4);

	if(info->sense == 1)
	{				/* forward sense */
	    if(info->plasmid || !m->circ12)
		if(cut1 < (ajint)start)
		    start = cut1;

	    if(info->plasmid || !m->circ34)
		if(cut3 && cut3 < (ajint)start)
		start = cut3;
	}
	else
	{				/* reverse sense */
	    if(info->plasmid || !m->circ12)
		if(cut2 < (ajint)start)
		    start = cut2;

	    if(info->plasmid || !m->circ34)
		if(cut4 && cut4 < (ajint)start)
		    start = cut4;
	}

	end = base + ajStrGetLen(m->pat)-1;
	ajDebug("showFillRE end: %d base: %d patlen: %d pat: '%S'\n",
		end, base, ajStrGetLen(m->pat), m->pat);

	nameend = base + ajStrGetLen(m->cod)-1;

	if(info->sense == 1)
	{				/* forward sense */
	    ajDebug("showFillRE fwd end: %d cut1:%d cut3:%d\n",
		    end, cut1, cut3);
	    if(info->plasmid || !m->circ12)
		if(cut1 > (ajint)end)
		    end = cut1;

	    if(info->plasmid || !m->circ34)
		if(cut3 && cut3 > (ajint)end)
		    end = cut3;
	    ajDebug("showFillRE fwd set end: %d\n",
		    end);
	}
	else
	{				/* reverse sense */
	    ajDebug("showFillRE rev end: %d cut2:%d cut4:%d\n",
		    end, cut2, cut4);
	    if(info->plasmid || !m->circ12)
		if(cut2 > (ajint)end)
		    end = cut2;

	    if(info->plasmid || !m->circ34)
		if(cut4 && cut4 > (ajint)end)
		    end = cut4;
	    ajDebug("showFillRE rev set end: %d\n",
		    end);
	}

	/* convert human-readable sequence positions to string positions */
	start--;
	end--;
	base--;
	nameend--;

	ajDebug("showFillREFlat start:%d end:%d pos:%d width:%d\n",
		start, end, pos, width);
	/* ignore this match if nothing is to be displayed on this line */
	if(start <= last && end >= pos)
	{
	    ajDebug("showFillREFlat site within range\n");

	    /* make a standard name and site string to be chopped up later */

	    /* site string stuff */
	    /* initial string of '.'s */
	    sitestr = ajStrNew();
	    ajStrAppendCountK(&sitestr, '.', end-start+1 );
	    ajDebug("showFillREFlat ajStrAppKI '.' %d\n", end-start+1);
	    /*
	    **  add on any claim characters required to stake a claim to
	    **  positions used by the name
	    */
	    if(nameend > end)
	    {
		ajStrAppendCountK(&sitestr, *claimchar, nameend-end);
		ajDebug("showFillREFlat ajStrAppKI '%c' %d\n",
			*claimchar, nameend-end);
	    }

	    /* cover binding site with '='s */
	    for(i=base-start; i<base-start+ajStrGetLen(m->pat); i++)
		ajStrPasteCountK(&sitestr, i, '=', 1);
	    ajDebug("showFillREFlat ajStrReplaceK '=' %d..%d\n",
		    base-start, i-1);

	    /*
	    **  I tried showing the pattern instead of '='s, but it looks
	    **  awful - GWW 12 Jan 2000
	    **      for(j=0, i=base-start; i<base-start+ajStrGetLen(m->pat);
	    **           j++, i++)
	    **          (ajStrGetPtr(sitestr)+i) = ajStrGetPtr(m->pat)[j];
	    */

	    /* put in cut sites */
	    if(info->sense == 1)
	    {				/* forward sense */
		if(info->plasmid || !m->circ12)
		{
		    ajStrPasteCountK(&sitestr, (cut1-start-1), '>', 1);
		    ajDebug("showFillREFlat ajStrReplaceK1 '>' %d\n",
			    (cut1-start-1));
		}
		if(info->plasmid || !m->circ34)
		{
		    if(cut3)
		    {
			ajStrPasteCountK(&sitestr, (cut3-start-1), '>', 1);
			ajDebug("showFillREFlat ajStrReplaceK3 '>' %d\n",
				(cut3-start-1));
		    }
		}
	    }
	    else
	    {				/* reverse sense */
		if(info->plasmid || !m->circ12)
		{
		    ajStrPasteCountK(&sitestr, (cut2-start-1), '<', 1);
		    ajDebug("showFillREFlat ajStrReplaceK2 '<' %d\n",
			    (cut2-start-1));
		}
		if(info->plasmid || !m->circ34)
		{
		    if(cut4)
		    {
			ajStrPasteCountK(&sitestr, (cut4-start-1), '<', 1);
			ajDebug("showFillREFlat ajStrReplaceK4 '<' %d\n",
				(cut4-start-1));
		    }
		}
	    }


	    /* name string stuff */
	    /* initial string of claimchar's */
	    namestr = ajStrNew();
	    ajStrAppendCountK(&namestr, *claimchar, end-start+1 );
	    ajDebug("showFillREFlat ajStrAppKI name '%c' %d\n",
		    *claimchar, end-start+1 );
	    if(nameend > end)
	    {
		ajStrAppendCountK(&namestr, *claimchar, nameend-end);
		ajDebug("showFillREFlat ajStrAppKI nameend '%c' %d\n",
			*claimchar, nameend-end );
	    }

	    /* insert the name in the namestr */
	    ajStrPasteS(&namestr, (base-start), m->cod);

	    /* now chop up the name and site strings to fit in the line */

	    /* is the feature completely within the line */
	    if(start >= pos && end <= last)
	    {
		/*
		 *  add on an extra couple of claim chars to make a space
		 *  between adjacent matches
		 */
		ajStrAppendC(&sitestr, claimchar);
		ajStrAppendC(&sitestr, claimchar);

	    }
	    else if(start < pos && end <= last)
	    {
		/* starts before the line. cut off the start */
		ajStrKeepRange(&sitestr, pos-start, ajStrGetLen(sitestr)-1);

		/*
		**  add on an extra couple of claim chars to make a space
		**  between adjacent matches
		*/
		ajStrAppendC(&sitestr, claimchar);
		ajStrAppendC(&sitestr, claimchar);

		/*
		**  if the base position is not displayed, move the name to
		**  the start
		*/
		if(base < pos)
		{
		    ajStrAssignS(&namestr, m->cod);
		    ajStrAppendC(&namestr, claimchar);
		    ajStrAppendC(&namestr, claimchar);

		    /*
		    **  add claim characters to end of namestring if
		    **  sitestring is longer, and vice versa
		    */
		    if(ajStrGetLen(namestr) < ajStrGetLen(sitestr))
			ajStrAppendCountK(&namestr, *claimchar,
					  ajStrGetLen(sitestr)-ajStrGetLen(namestr));
		    if(ajStrGetLen(namestr) > ajStrGetLen(sitestr))
			ajStrAppendCountK(&sitestr, *claimchar,
					  ajStrGetLen(namestr)-ajStrGetLen(sitestr));
		}
		else
		    /*
		    **  cut off the start of the name string to make it line
		    **  up with the sitestr
		    */
		    ajStrKeepRange(&namestr, pos-start, ajStrGetLen(namestr)-1);

		/* make it display from the start of the line */
		start = pos;


	    }
	    else if(start >= pos && end > last)
	    {
		/* ends after the line. cut off the end */
		ajStrKeepRange(&sitestr, 0, last - start);
		/*
		**  if the base position is not displayed, move the name
		**  to the start
		*/
		if(base > last)
		{
		    ajStrAssignS(&namestr, m->cod);
		    ajStrAppendC(&namestr, claimchar);
		    ajStrAppendC(&namestr, claimchar);

		    /*
		    **  add claim characters to end of namestring if
		    **  sitestring is longer, and vice versa
		    */
		    if(ajStrGetLen(namestr) < ajStrGetLen(sitestr))
			ajStrAppendCountK(&namestr, *claimchar,
					  ajStrGetLen(sitestr)-ajStrGetLen(namestr));
		    if(ajStrGetLen(namestr) > ajStrGetLen(sitestr))
			ajStrAppendCountK(&sitestr, *claimchar,
					  ajStrGetLen(namestr)-ajStrGetLen(sitestr));
		}
		/* make it display to the end of the line */
		end = last;


	    }
	    else if(start < pos && end > last)
	    {
		/* completely overlaps the line! cut off the start and end */
		ajStrKeepRange(&sitestr, pos-start, last-start);
		/*
		**  if the base position is not displayed, move the name to
		**  the start
		*/
		if(base < pos)
		{
		    ajStrAssignS(&namestr, m->cod);
		    ajStrAppendC(&namestr, claimchar);
		    ajStrAppendC(&namestr, claimchar);

		    /*
		    **  add claim characters to end of namestring if
		    **  sitestring is longer, and vice versa
		    */
		    if(ajStrGetLen(namestr) < ajStrGetLen(sitestr))
			ajStrAppendCountK(&namestr, *claimchar,
					  ajStrGetLen(sitestr)-ajStrGetLen(namestr));
		    if(ajStrGetLen(namestr) > ajStrGetLen(sitestr))
			ajStrAppendCountK(&sitestr, *claimchar,
					  ajStrGetLen(namestr)-ajStrGetLen(sitestr));
		}
		else
		{
		    /*
		    **  cut off the start of the name string to make it line
		    **  up with the sitestr
		    */
		    ajStrKeepRange(&namestr, pos-start,
				    /* ...or should this be , */
				    /* last-start); */
				    ajStrGetLen(namestr)-1);
		}
		/* make it display from the start of the line */
		start = pos;
		end   = last;
	    }
	    else
	    {
		ajDebug("Shouldn't get to here!");
		continue;
	    }

	    /* work up list of lines */
	    freespace = ajFalse;

	    /*
	    **  potentially updating the nodes of linelist, so
	    **  don't just iterate, use ajListstrPop and ajListstrPushApp
	    **  to pop off the bottom and then push the altered node back on
	    **  the top of the list
	    */
	    for(ln = ajListstrGetLength(linelist); ln>0; ln--)
	    {
		ajListstrPop(linelist, &line); /* get the site line */
		ajListstrPop(linelist, &line2); /* get the name line */
		/*
		**  if not yet written the name in this set of
		**  iterations, see if can be done now
		*/
		if(!freespace)
		{
		    if(showLineIsClear(&line, start-pos, end-pos))
		    {
			showOverPrint(&line, start-pos, sitestr);
			showOverPrint(&line2, start-pos, namestr);
			/* flag to show name written */
			freespace = ajTrue;
		    }
		}

		ajListstrPushAppend(linelist, line);
		ajListstrPushAppend(linelist, line2);
		/* end 'iteration' through lines */
	    }

	    /*
	    **  if didn't find a clear region to print in, append two new
	    **  strings and print in them
	    */
	    if(!freespace)
	    {
		line=ajStrNew();

		/* fill with spaces */
		ajStrAppendCountK(&line, ' ', width);
		line2=ajStrNew();

		/* fill with spaces */
		ajStrAppendCountK(&line2, ' ', width);

		showOverPrint(&line, start-pos, sitestr);
		showOverPrint(&line2, start-pos, namestr);
		ajListstrPushAppend(linelist, line);
		ajListstrPushAppend(linelist, line2);
	    }

	    ajStrDel(&namestr);
	    ajStrDel(&sitestr);
	}
    }
    ajListIterDel(&miter);


    /*
    **  reverse the order of the lines if we are in the forward sense as then
    **  we get the most densely populated lines at the bottom closest to the
    **  sequence (and we get the names above the cut-sites)
    */
    if(info->sense == 1)
	ajListstrReverse(linelist);

    /* iterate through the lines and print them */
    liter = ajListIterNewread(linelist);
    while((line = ajListIterGet(liter)) != NULL)
    {
	/*
	**  convert claim characters in the line to spaces as these were
	**  used to stake a claim to the space
	*/
	ajStrExchangeSetCC(&line, claimchar, " ");

	/*
	**  remove trailing spaces - these can be very long in namestr when
	**  the cut and recognition sites are widely separated and so many
	**  claimchars have been appended
	*/
	ajStrTrimEndC(&line, " ");

	showMargin(thys, lines);
	ajListstrPushAppend(lines, line);
	ajListstrPushAppend(lines, ajFmtStr("\n"));
    }
    ajListIterDel(&liter);


    while(ajListstrPop(linelist,&sajb));
    ajListstrFree(&linelist);

    return;
}




/* @funcstatic showFillFT *****************************************************
**
** Add this line's worth of features to the lines list
** NB. the 'source' feature is always ignored
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowFT] data on how to display the features
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/

static void showFillFT(const EmbPShow thys,
		       AjPList lines, const EmbPShowFT info,
		       ajuint pos, ajuint last)
{


    AjIList iter  = NULL;
    AjPFeature gf = NULL;

    AjPStr line      = NULL;
    AjPStr line2     = NULL;
    AjPList linelist = NULL;	/* list of lines to fill */
    ajuint start;
    ajuint end;
    ajuint namestart;
    ajuint nameend;		/* start and end position of namestr */
    AjIList liter;		/* iterator for linelist */
    AjPStr namestr = NULL;	/* name of feature to insert into line */
    AjPStr linestr = NULL;	/* line graphics to insert */
    const char *claimchar = "*";   /* char used to stake a claim to */
				   /* that position in the string */
    AjBool freespace;		/* flag for found a free space to
				   print in */
    ajuint ln;
    AjPStr sajb = NULL;

    ajuint width;

    width = last - pos + 1;

    ajDebug("showFillFT\n");
    linelist = ajListstrNew();

    /*
    **  if feat is NULL then there are no features associated with this
    **  sequence
    */
    if(!info->feat)
	return;


    /* reminder of the AjSFeature structure for handy reference
    **
    **
    **  AjEFeatClass      Class ;
    **  AjPFeattable      Owner ;
    **  AjPFeatVocFeat    Source ;
    **  AjPFeatVocFeat    Type ;
    **  ajint             Start ;
    **  ajint             End;
    **  ajint             Start2;
    **  ajint             End2;
    **  AjPStr            Score ;
    **  AjPList           Tags ;  a.k.a. the [group] field tag-values of GFF2
    **  AjPStr            Comment ;
    **  AjEFeatStrand     Strand ;
    **  AjEFeatFrame      Frame ;
    **  AjPStr            desc ;
    **  ajint             Flags;
    **
    */


    /* iterate through the features */
    if(info->feat->Features)
    {
	iter = ajListIterNewread(info->feat->Features) ;
	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter) ;

            /* ignore remote IDs */
            if(!ajFeatIsLocal(gf))
		continue;

            /* don't output the 'source' feature */
	    if(!ajStrCmpC(ajFeatGetType(gf), "source"))
		continue;

	    /*
	    ** check that the feature is within the line to display (NB.
	    ** Working in human coordinates here: 1 to SeqLength,
	    ** not 0 to SeqLength-1)
	    */
	    if(pos >= ajFeatGetEnd(gf) ||
	       last <= ajFeatGetStart(gf))
		continue;

	    /* prepare name string */
	    namestr = ajStrNew();
	    ajStrAssignS(&namestr,  ajFeatGetType(gf));

	    /* add tags to namestr*/
	    showAddTags(&namestr, gf, ajTrue);

	    /*
	    **  note the start and end positions of the name and line
	    **  graphics
	    */
	    start = (ajFeatGetStart(gf)-1<pos) ?
		pos : ajFeatGetStart(gf)-1;
	    end   = (ajFeatGetEnd(gf)-1>last) ?
		last : ajFeatGetEnd(gf)-1;

	    /* print the name starting with the line */
	    namestart = start;
	    nameend   =  start + ajStrGetLen(namestr)-1;

	    /* shift long namestr back if longer than the line when printed */
	    if(nameend > last+thys->margin)
	    {
		if(ajStrGetLen(namestr) > end-pos+1)
		{
		    namestart = pos;
		    nameend = pos + ajStrGetLen(namestr) -1;
		    /*
		    **  it is shifted back to the start of the display line
		    **  is it still longer than the line? truncate it
		    */
		    if(nameend > thys->width-1+thys->margin)
		    {
			ajStrTruncateLen(&namestr, thys->width-1+thys->margin);
			nameend = pos+thys->width-1+thys->margin;
		    }
		}
		else
		{
		    namestart = end - ajStrGetLen(namestr)+1;
		    nameend = namestart + ajStrGetLen(namestr)-1;
		}
	    }

	    /*
	    **  add on any claim characters required to stake a claim to
	    **  positions used by the line graphics
	    */
	    if(end > nameend)
	    {
		ajStrAppendCountK(&namestr, *claimchar, end-nameend);
		nameend = end;
	    }

	    /*
	    **  add on a couple more claim characters to space out the
	    **  features
	    */
	    ajStrAppendCountK(&namestr, *claimchar, 2);
	    nameend += 2;

	    /* prepare line string */
	    /* initial string of '='s */
	    linestr = ajStrNew();
	    ajStrAppendCountK(&linestr, '=', end-start+1 );

	    /* put in end position characters */
	    if(ajFeatGetStart(gf)-1>=pos)
		ajStrPasteCountK(&linestr,0, '|', 1);
	    if(ajFeatGetEnd(gf)-1<=last)
		ajStrPasteCountK(&linestr, (end-start), '|', 1);


	    /* work up list of lines */
	    freespace = ajFalse;

	    /*
	    **  iterate through list of existing lines to find no overlap
	    **  with existing lines we will be potentially updating the
	    **  nodes of linelist, so don't just iterate, use ajListstrPop
	    **  and ajListstrPushApp to pop off the bottom and then push
	    **  the altered node back on the top of the list
	    */
	    for(ln = ajListstrGetLength(linelist); ln>0; ln--)
	    {
		/* get the linestr line */
		ajListstrPop(linelist, &line);
		/* get the namestr line */
		ajListstrPop(linelist, &line2);

		/*
		**  if not yet written the name in this set of
		**  iterations, see if it can be done now
		*/
		if(!freespace)
		{
		    /* if name space is clear, write namestr and sitestr */
		    if(showLineIsClear(&line2, start-pos, end-pos) &&
			showLineIsClear(&line2, namestart-pos, nameend-pos))
		    {
			showOverPrint(&line, start-pos, linestr);
			showOverPrint(&line2, namestart-pos, namestr);
			/* flag to show name written */
			freespace = ajTrue;
		    }
		}

		ajListstrPushAppend(linelist, line);
		ajListstrPushAppend(linelist, line2);
		/* end 'iteration' through lines */
	    }

	    /*
	    **  if didn't find a clear region to print in, append two new
	    **  strings and print in them
	    */
	    if(!freespace)
	    {
		line=ajStrNew();
		/* fill with spaces */
		ajStrAppendCountK(&line, ' ', width);
		line2=ajStrNew();
		/* fill with spaces */
		ajStrAppendCountK(&line2, ' ', width);

		showOverPrint(&line, start-pos, linestr);
		showOverPrint(&line2, namestart-pos, namestr);
		ajListstrPushAppend(linelist, line);
		ajListstrPushAppend(linelist, line2);
	    }

	    ajStrDel(&namestr);
	    ajStrDel(&linestr);
	}
	ajListIterDel(&iter);
    }

    /* iterate through the lines and print them */
    liter = ajListIterNewread(linelist);
    while((line = ajListIterGet(liter)) != NULL)
    {
	/*
	** convert claim characters in the line to spaces as these were
	** used to stake a claim to the space
	*/
	ajStrExchangeSetCC(&line, claimchar, " ");

	/* remove trailing spaces - these can be very long */
	ajStrTrimEndC(&line, " ");

	/*
	** output to the lines list
	** variable width margin at left
	*/
	showMargin(thys, lines);

	/*
	** with optional number in it
	** put the translation line on the output list
	*/
	ajListstrPushAppend(lines, line);

	/* end output line */
	ajListstrPushAppend(lines, ajFmtStr("\n"));
    }
    ajListIterDel(&liter);


    while(ajListstrPop(linelist,&sajb));
    ajListstrFree(&linelist);

    return;
}




/* @funcstatic showFillNote ***************************************************
**
** Add this line's worth of user annotation to the lines list
**
** @param [r] thys [const EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [const EmbPShowNote] data on how to display the annotation
** @param [r] pos [ajuint] current printing position in the sequence
** @param [r] last [ajuint] last printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillNote(const EmbPShow thys,
			 AjPList lines, const EmbPShowNote info,
			  ajuint pos, ajuint last)
{

    AjPStr line  = NULL;
    AjPStr line2 = NULL;
    AjPList linelist = NULL;	/* list of lines to fill */
    ajuint start;
    ajuint end;			/* start and end position of linestr */
    ajuint namestart;
    ajuint nameend;		/* start and end position of namestr */
    AjIList liter;		/* iterator for linelist */
    AjPStr namestr = NULL;	/* name of feature to insert into line */
    AjPStr linestr = NULL;	/* line graphics to insert */
    const char *claimchar = "*";   /* char used to stake a claim to */
				   /* that position in the string */
    AjBool freespace;		/* flag for found a free space to print in */
    ajint ln;
    AjPStr sajb = NULL;

    ajuint count;		/* count of annotation region */
    ajuint rstart;
    ajuint rend;			/* region start and end */
    ajuint width;

    width = last - pos + 1;

    ajDebug("showFillFT\n");
    linelist = ajListstrNew();

    /* count through the annotation regions */
    if(info->regions && ajRangeNumber(info->regions))
    {
	for(count = 0; count < ajRangeNumber(info->regions); count++)
	{
            ajRangeValues(info->regions, count, &rstart, &rend);

	    /*
	    ** check that the region is within the line to display
	    */
	    if(pos >= rend || last <= rstart)
		continue;

	    /* get annotation string */
            ajRangeText(info->regions, count, &namestr);

	    /*
	    **  note the start and end positions of the name and line
	    **  graphics
	    */
	    start = (rstart-1<pos) ? pos : rstart-1;
	    end   = (rend-1>last) ? last :
		rend-1;

	    /* print the name starting with the line */
	    namestart = start;
	    nameend   = start + ajStrGetLen(namestr)-1;

	    /* shift long namestr back if longer than the line when printed */
	    if(nameend >last+thys->margin)
	    {
		if(ajStrGetLen(namestr) > end-pos+1)
		{
		    namestart = pos;
		    nameend = pos + ajStrGetLen(namestr) -1;
		    /*
		    **  it is shifted back to the start of the display line
		    **  is it still longer than the line? truncate it
		    */
		    if(nameend > width-1+thys->margin)
		    {
			ajStrTruncateLen(&namestr, width-1+thys->margin);
			nameend = last+thys->margin;
		    }
		}
		else
		{
		    namestart = end - ajStrGetLen(namestr)+1;
		    nameend = namestart + ajStrGetLen(namestr)-1;
		}
	    }

	    /*
	    **  add on any claim characters required to stake a claim to
	    **  positions used by the line graphics
	    */
	    if(end > nameend)
	    {
		ajStrAppendCountK(&namestr, *claimchar, end-nameend);
		nameend = end;
	    }

	    /*
	    **  add on a couple more claim characters to space out the
	    **  features
	    */
	    ajStrAppendCountK(&namestr, *claimchar, 2);
	    nameend += 2;

	    /*
	    ** prepare line string
	    ** initial string of '-'s
	    */
	    linestr = ajStrNew();
	    ajStrAppendCountK(&linestr, '-', end-start+1 );

	    /* put in end position characters */
	    if(rstart-1>=pos)
		ajStrPasteCountK(&linestr, 0, '|', 1);
	    if(rend-1<=last)
		ajStrPasteCountK(&linestr, (end-start), '|', 1);

	    /* work up list of lines */
	    freespace = ajFalse;

	    /*
	    **  iterate through list of existing lines to find no overlap
	    **  with existing lines we will be potentially updating the
	    **  nodes of linelist, so don't just iterate, use ajListstrPop
	    **  and ajListstrPushApp to pop off the bottom and then push
	    **  the altered node back on the top of the list
	    */
	    for(ln = ajListstrGetLength(linelist); ln>0; ln--)
	    {
		/* linestr line */
		ajListstrPop(linelist, &line);
		/* namestr line */
		ajListstrPop(linelist, &line2);

		/*
		 *  if not yet written the name in this set of
		 *  iterations, see if it can be done now
		 */
		if(!freespace)
		{
		    /* if name space is clear, write namestr and sitestr */
		    if(showLineIsClear(&line2, start-pos, end-pos) &&
			showLineIsClear(&line2, namestart-pos, nameend-pos))
		    {
			showOverPrint(&line, start-pos, linestr);
			showOverPrint(&line2, namestart-pos, namestr);

			/* flag to show name written */
			freespace = ajTrue;
		    }
		}

		ajListstrPushAppend(linelist, line);
		ajListstrPushAppend(linelist, line2);
		/* end 'iteration' through lines */
	    }

	    /*
	    **  if we didn't find a clear region to print in, append two new
	    **  strings and print in them
	    */
	    if(!freespace)
	    {
		line=ajStrNew();
		/* fill with spaces */
		ajStrAppendCountK(&line, ' ', width);
		line2=ajStrNew();

		/* fill with spaces */
		ajStrAppendCountK(&line2, ' ', width);

		showOverPrint(&line, start-pos, linestr);

		showOverPrint(&line2, namestart-pos, namestr);
		ajListstrPushAppend(linelist, line);
		ajListstrPushAppend(linelist, line2);
	    }

	    ajStrDel(&namestr);
	    ajStrDel(&linestr);
	}
    }

    /* iterate through the lines and print them */
    liter = ajListIterNewread(linelist);
    while((line = ajListIterGet(liter)) != NULL)
    {
	/*  convert claim characters in the line to spaces as these were
	**  used to stake a claim to the space
	*/
	ajStrExchangeSetCC(&line, claimchar, " ");

	/* remove trailing spaces - these can be very long */
	ajStrTrimEndC(&line, " ");

	/*
	** output to the lines list 
	** variable width margin at left
	*/
	showMargin(thys, lines);

	/*
	** with optional number in it
	** put the translation line on the output list
	*/
	ajListstrPushAppend(lines, line);

	/* end output line */
	ajListstrPushAppend(lines, ajFmtStr("\n"));
    }
    ajListIterDel(&liter);


    while(ajListstrPop(linelist,&sajb));
    ajListstrFree(&linelist);

    return;
}




/* @funcstatic showOverPrint **************************************************
**
** Overwrite (appending if necessary) a portion of a string with another
**
** @param [u] target [AjPStr *] target string to overwrite
** @param [r] start [ajint] start position in target to begin overwriting at
** @param [u] insert [AjPStr] string to overwrite with
** @return [void]
** @@
******************************************************************************/

static void showOverPrint(AjPStr *target, ajint start, AjPStr insert)
{
    /*
    ** if start position of insert is less than length of target, pad it out
    ** with space characters to get the required length
    */

    if(ajStrGetLen(*target) < start+ajStrGetLen(insert))
	ajStrAppendCountK(target, ' ',
		   start+ajStrGetLen(insert) - ajStrGetLen(*target));

    /* overwrite the remaining characters */
    ajStrPasteS(target, start, insert);
    return;
}




/* @funcstatic showLineIsClear ************************************************
**
** Checks that a section of a string is clear to print in (only has blanks in)
**
** @param [u] line [AjPStr *] target string to check
** @param [r] start [ajint] start position in target to begin checking at
** @param [r] end [ajint] end position in target to check
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool showLineIsClear(AjPStr *line, ajint start, ajint end)
{
    ajint i;
    ajint len;

    len = ajStrGetLen(*line)-1;

    if(len < end)
	ajStrAppendCountK(line, ' ', end-len);

    for(i=start; i<=end; i++)
	if(ajStrGetCharPos(*line,i) != ' ')
	    return ajFalse;

    return ajTrue;
}




/* @funcstatic showAddTags ****************************************************
**
** writes feature tags to the tagsout string
**
** @param [w] tagsout [AjPStr*] tags out string
** @param [r] feat [const AjPFeature] Feature to be processed
** @param [r] values [AjBool] display values of tags (currently ignored)
**
** @return [void]
** @@
******************************************************************************/

static void showAddTags(AjPStr *tagsout, const AjPFeature feat, AjBool values)
{

    AjPStr tagnam = NULL;
    AjPStr tagval = NULL;
    AjIList titer;

    /*
    ** iterate through the tags and test for match to patterns
    */

    (void) values;			/* make it used */

    tagval = ajStrNew();
    tagnam = ajStrNew();

    titer = ajFeatTagIter(feat);

    /* don't display the translation tag - it is far too long */
    while(ajFeatTagval(titer, &tagnam, &tagval))
	if(ajStrCmpC(tagnam, "translation"))
	{
	    if(ajStrGetLen(tagval))
		ajFmtPrintAppS(tagsout, " %S=\"%S\"", tagnam, tagval);
	    else
		ajFmtPrintAppS(tagsout, " %S", tagnam);
	}

    ajListIterDel(&titer);

    ajStrDel(&tagval);
    ajStrDel(&tagnam);

    return;
}
