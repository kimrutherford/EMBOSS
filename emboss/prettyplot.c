/* @source prettyplot application
**
** Displays and plots sequence alignments and consensus for PLOTTERS.
** @author Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
**
** Replaces program "prettyplot" (EGCG)
**
** options.
**
** -ccolours    Colour residues by their consensus value. (TRUE)
** -cidentity   Colour to display identical matches. (RED)
** -csimilarity Colour to display similar matches.   (GREEN)
** -cother      Colour to display other matches.     (BLACK)
**
** -docolour    Colour residues by table oily, amide, basic etc. (FALSE)
**
** -shade       Colour residues by shades. (BLPW)
**              B-Black L-Brown P-Wheat W-White
**
** -pair        values to represent identical similar related (1.5,1.0,0.5)
**
** -identity    Only match those which are identical in all sequences.
**
** -box         Display prettybox.
**
**
** -consensus   Display the consensus.
**
** -name        Display the sequence names.
**
** -number      Display the residue number at the end of each sequence.
**
** -maxnamelen  Margin size for the sequence name.
**
** -plurality   plurality check value used in consensus. (totweight/2)
**
** -collision   Allow collisions.
**
** -portrait    Display as a portrait (default landscape).
**
** -datafile    The data file holding the matrix comparison table.
**
** -showscore   Obsolete debug variable:
**              Print out the scores for a residue number.
**
** -alternative 3 other checks for collisions.
**
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
#include <limits.h>

#define BOXTOP      0x0001
#define BOXBOT      0x0002
#define BOXLEF      0x0004
#define BOXRIG      0x0008
#define BOXCOLOURED 0x0010

const char **seqcharptr;
ajint **seqcolptr;
ajint **seqboxptr = NULL;
ajint *seqcount = NULL;
ajint charlen;
AjBool shownames;
AjBool shownumbers;
AjPSeqset seqset;
AjPStr *seqnames;
ajint numgaps;
char *constr = NULL;




static ajint prettyplot_calcseqperpage(float yincr,float y,AjBool consensus);
static void  prettyplot_fillinboxes(ajint length, ajint numseq,
				    ajint start, ajint end,
				    ajint seqstart,ajint seqend, 
				    ajint numres, ajint resbreak,
				    AjBool boxit, AjBool boxcol, 
				    AjBool consensus,
				    float ystart, float yincr, 
				    const AjPSeqCvt cvt);




/* @prog prettyplot ***********************************************************
**
** Displays aligned sequences, with colouring and boxing
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint i;
    ajint numseq;
    ajint j = 0;
    ajint numres;
    ajint count;
    ajint k;
    ajint kmax;
    float defheight;
    float currentscale;
    AjPStr shade = NULL;
    AjPFloat pair  = NULL;
    AjPGraph graph = NULL;
    AjPMatrix cmpmatrix = NULL;
    AjPSeqCvt cvt = NULL;
    AjPStr matcodes = NULL;
    AjBool consensus;
    AjBool colourbyconsensus;
    AjBool colourbyresidues;
    AjBool colourbyshade = AJFALSE;
    AjBool boxit;
    AjBool boxcol;
    AjBool portrait;
    AjBool collision;
    ajint identity;
    AjBool listoptions;
    ajint alternative;
    AjPStr altstr = NULL;
    AjPStr sidentity = NULL;
    AjPStr ssimilarity = NULL;
    AjPStr sother = NULL;
    AjPStr sboxcolval = NULL;
    AjPStr options = NULL;
    /*    ajint showscore = 0; */
    ajint iboxcolval = 0;
    ajint cidentity = RED;
    ajint csimilarity = GREEN;
    ajint cother = BLACK;
    float fxp;
    float fyp;
    float yincr;
    float y;
    ajint ixlen;
    ajint iylen;
    ajint ixoff;
    ajint iyoff;
    char res[2] = " ";

    float *score = 0;
    float scoremax = 0;

    float *identical = NULL;
    ajint identicalmaxindex;
    float *matching = NULL;
    ajint matchingmaxindex;

    float *colcheck = NULL;

    ajint **matrix;
    ajint m1 = 0;
    ajint m2 = 0;
    ajint ms = 0;
    ajint highindex = 0;
    ajint myindex;
    ajint *previous = 0;
    AjBool iscons = ajFalse;
    ajint currentstate = 0;
    ajint oldfg = 0;
    float fold = 0.0;
    ajint *colmat = 0;
    ajint *shadecolour = 0;
    /* float identthresh = 1.5; */
    /* float simthresh = 1.0; */
    /* float relthresh = 0.5; */
    float part = 0.0;
    const char *cptr;
    ajint resbreak;
    float fplural;
    float ystart;
    float xmin;
    float xmax;
    float xmid;
    AjPTime ajtime;
    ajint gapcount = 0;
    ajint countforgap = 0;
    ajint boxindex;
    float max;
    ajint matsize;
    ajint seqperpage = 0;
    ajint startseq;
    ajint endseq;
    ajint newILend = 0;
    ajint newILstart;
    void *freeptr;
    ajint itmp;
    
    embInit("prettyplot", argc, argv);

    seqset   = ajAcdGetSeqset("sequences");
    numres   = ajAcdGetInt("residuesperline");
    resbreak = ajAcdGetInt("resbreak");

    ajSeqsetFill(seqset);	/* Pads sequence set with gap characters */
    numseq = ajSeqsetGetSize(seqset);

    graph             = ajAcdGetGraph("graph");
    colourbyconsensus = ajAcdGetBoolean("ccolours");
    colourbyresidues  = ajAcdGetBoolean("docolour");
    shade             = ajAcdGetString("shade");
    pair              = ajAcdGetArray("pair");
    identity          = ajAcdGetInt("identity");
    boxit             = ajAcdGetBoolean("box");

    ajtime = ajTimeNewTodayFmt("daytime");

    ajSeqsetTrim(seqset);
    /* offset = ajSeqsetGetOffset(seqset); Unused */

    ajGraphAppendTitleS(graph, ajSeqsetGetUsa(seqset));

    if(boxit)
    {
	AJCNEW(seqboxptr, numseq);
	for(i=0;i<numseq;i++)
	    AJCNEW(seqboxptr[i], ajSeqsetGetLen(seqset));
    }
    boxcol      = ajAcdGetBoolean("boxcol");
    sboxcolval  = ajAcdGetString("boxuse");

    if(boxcol)
    {
	iboxcolval = ajGraphicsCheckColourS(sboxcolval);
	if(iboxcolval == -1)
	    iboxcolval = GREY;
    }

    consensus = ajAcdGetBoolean("consensus");
    if(consensus)
    {
	AJCNEW(constr, ajSeqsetGetLen(seqset)+1);
	constr[0] = '\0';
    }
    shownames   = ajAcdGetBoolean("name");
    shownumbers = ajAcdGetBoolean("number");
    charlen     = ajAcdGetInt("maxnamelen");
    fplural     = ajAcdGetFloat("plurality");
    portrait    = ajAcdGetBoolean("portrait");
    collision   = ajAcdGetBoolean("collision");
    listoptions = ajAcdGetBoolean("listoptions");
    altstr = ajAcdGetListSingle("alternative");
    cmpmatrix   = ajAcdGetMatrix("matrixfile");

    ajStrToInt(altstr, &alternative);

    matrix = ajMatrixGetMatrix(cmpmatrix);
    cvt = ajMatrixGetCvt(cmpmatrix);
    matsize = ajMatrixGetSize(cmpmatrix);

    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);
    AJCNEW(colcheck,matsize);

    numgaps = numres/resbreak;
    numgaps--;

    if(portrait)
    {
	ajGraphicsSetPortrait(1);
	ystart = (float) 75.0;
    }
    else
	ystart = (float) 75.0;

    /* pair is an array of three non-negative floats */

    /* identthresh = ajFloatGet(pair,0); Unused */
    /* simthresh = ajFloatGet(pair,1); Unused */
    /* relthresh = ajFloatGet(pair,2); Unused */

    /*
    ** shade is a formatted 4-character string. Characters BLPW only.
    ** controlled by a pattern in ACD.
    */

    if(ajStrGetLen(shade))
    {
	AJCNEW(shadecolour,4);
	cptr = ajStrGetPtr(shade);
	for(i=0;i<4;i++){
	    if(cptr[i]== 'B' || cptr[i]== 'b')
		shadecolour[i] = BLACK;
	    else if(cptr[i]== 'L' || cptr[i]== 'l')
		shadecolour[i] = BROWN;
	    else if(cptr[i]== 'P' || cptr[i]== 'p')
		shadecolour[i] = WHEAT;
	    else if(cptr[i]== 'W' || cptr[i]== 'w')
		shadecolour[i] = WHITE;
	}

	colourbyconsensus = colourbyresidues = ajFalse;
	colourbyshade = ajTrue;
    }

/*
** we can colour by consensus or residue but not both
** if we have to choose, use the consensus
*/

    if(colourbyconsensus && colourbyresidues)
	colourbyconsensus = AJFALSE;

    sidentity = ajAcdGetString("cidentity");
    ssimilarity = ajAcdGetString("csimilarity");
    sother = ajAcdGetString("cother");

    if(colourbyconsensus)
    {
	cidentity = ajGraphicsCheckColourS(sidentity);
	if(cidentity == -1)
	    cidentity = RED;

	csimilarity = ajGraphicsCheckColourS(ssimilarity);
	if(csimilarity == -1)
	    csimilarity = GREEN;


	cother = ajGraphicsCheckColourS(sother);
	if(cother == -1)
	    cother = BLACK;

    }
    else if(colourbyresidues)
    {
	matcodes = ajMatrixGetCodes(cmpmatrix);
	if(ajSeqsetIsProt(seqset))
	    colmat = ajGraphicsBasecolourNewProt(matcodes);
	else
	    colmat = ajGraphicsBasecolourNewNuc(matcodes);
    }


    /* output the options used as the subtitle for the bottom of the graph */
    if(listoptions)
    {
	ajStrAssignC(&options,"");
	ajFmtPrintAppS(&options,"-plurality %.1f",fplural);

	if(collision)
	    ajStrAppendC(&options," -collision");
	else
	    ajStrAppendC(&options," -nocollision");

	if(boxit)
	    ajStrAppendC(&options," -box");
	else
	    ajStrAppendC(&options," -nobox");

	if(boxcol)
	    ajStrAppendC(&options," -boxcol");
	else
	    ajStrAppendC(&options," -noboxcol");

	if(colourbyconsensus)
	    ajStrAppendC(&options," -colbyconsensus");
	else if(colourbyresidues)
	    ajStrAppendC(&options," -colbyresidues");
	else if(colourbyshade)
	    ajStrAppendC(&options," -colbyshade");
	else
	    ajStrAppendC(&options," -nocolour");

	if(alternative==2)
	    ajStrAppendC(&options," -alt 2");
	else if(alternative==1)
	    ajStrAppendC(&options," -alt 1");
	else if(alternative==3)
	    ajStrAppendC(&options," -alt 3");
    }


    AJCNEW(seqcolptr, numseq);
    for(i=0;i<numseq;i++)
	AJCNEW(seqcolptr[i], ajSeqsetGetLen(seqset));

    AJCNEW(seqcharptr, numseq);
    AJCNEW(seqnames, numseq);
    AJCNEW(score, numseq);
    AJCNEW(previous, numseq);
    AJCNEW(seqcount, numseq);

    for(i=0;i<numseq;i++)
    {
	ajSeqsetFmtUpper(seqset);
	seqcharptr[i] =  ajSeqsetGetseqSeqC(seqset, i);
	seqnames[i] = 0;
	ajStrAppendS(&seqnames[i],ajSeqsetGetseqNameS(seqset, i));
	ajStrTruncateLen(&seqnames[i],charlen);
	previous[i] = 0;
	seqcount[i] = 0;
    }

    /*
    ** user will pass the number of residues to fit a page
    ** therefore we now need to calculate the size of the chars
    ** based on this and get the new char width.
    ** 'charlen' maximum characters for the name (truncated above)
    */

    ajGraphicsGetCharsize(&defheight,&currentscale);

    xmin = -charlen - (float)2.0;
    xmax = (float)numres+(float)11.0+(float)(numres/resbreak);
    xmid = (xmax + xmin)/(float)2.0;

    ajGraphOpenWin(graph, xmin, xmax,
		   (float)0.0, ystart+(float)1.0);
 
    ajGraphGetParamsPage(graph, &fxp,&fyp,&ixlen,&iylen,&ixoff,&iyoff);

    if(portrait)
    {
        itmp = ixlen;
        ixlen = iylen;
        iylen = itmp;
    }

    ajGraphicsGetCharsize(&defheight,&currentscale);

    ajGraphicsSetCharscale(((float)ixlen/((float)(numres+charlen+1)*
                                          (currentscale * (float) 1.5)))/
                                           currentscale);

/*    ajGraphicsSetCharscale(((float)ixlen/((float)(numres+charlen)*
                                          (currentscale+(float)1.0)))/
                                          currentscale); */

    ajGraphicsGetCharsize(&defheight,&currentscale);

    yincr = (currentscale + (float)3.0)*(float)0.3;

/*
** If we have titles (now the standard graph title and subtitle and footer)
** leave 7 rows of space for them
*/
    y=ystart-(float)7.0;

    if(ajStrGetLen(options))
    {
	fold = ajGraphicsSetCharscale(1.0);
	ajGraphicsDrawposTextAtmid(xmid,2.0,
                                   ajStrGetPtr(options));
	ajGraphicsSetCharscale(fold);
    }

/* if sequences per page not set then calculate it */

    if(!seqperpage)
    {
	seqperpage = prettyplot_calcseqperpage(yincr,y,consensus);
	if(seqperpage>numseq)
	    seqperpage=numseq;
    }

    count = 0;

/*
** for boxes we need to set a foreground colour for the box lines
** and save the current foreground colour
*/
    if(boxit && boxcol)
	oldfg = ajGraphicsSetFgcolour(iboxcolval);

/*
** step through each residue position
*/

    kmax = ajSeqsetGetLen(seqset) - 1;
    for(k=0; k<= kmax; k++)
    {
	/* reset column score array */
	for(i=0;i<numseq;i++)
	    score[i] = 0.0;

	/* reset matrix character testing arrays */
	for(i=0;i<matsize;i++)
	{
	    identical[i] = 0.0;
	    matching[i] = 0.0;
	    colcheck[i] = 0.0;
	}

	/* generate a score for this residue in each sequence */
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
	    for(j=0;j<numseq;j++)
	    {
		m2 = ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]);
		if(m1 && m2)
		    score[i] += (float)matrix[m1][m2]*
			ajSeqsetGetseqWeight(seqset, j);
	    }
	    if(m1)
		identical[m1] += ajSeqsetGetseqWeight(seqset, i);
	}

	/* find the highest score */
	highindex = -1;
	scoremax  = INT_MIN;
	/*ajDebug("Scores at position %d:\n", k);*/

	for(i=0;i<numseq;i++)
	{
	    /*ajDebug("  seq %d: '%c' %f\n",i,seqcharptr[i][k],score[i]);*/

	    if(score[i] > scoremax)
	    {
		scoremax = score[i];
		highindex = i;
	    }
	}
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);

	    if(!matching[m1])
	    {
		for(j=0;j<numseq;j++)
		{
		    m2 = ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]);
		    if(m1 && m2 && matrix[m1][m2] > 0)
			matching[m1] += ajSeqsetGetseqWeight(seqset, j);
		}
	    }
	}

	/* find highs for matching and identical */
	matchingmaxindex  = 0;
	identicalmaxindex = 0;
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
	    if(identical[m1] > identical[identicalmaxindex])
		identicalmaxindex = m1;
	}
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
	    if(matching[m1] > matching[matchingmaxindex])
		matchingmaxindex = m1;
	    else if(matching[m1] ==  matching[matchingmaxindex])
	    {
		if(identical[m1] > identical[matchingmaxindex])
		    matchingmaxindex= m1;
	    }
	}

	iscons = ajFalse;
	boxindex = -1;
	max = -3;

	ajDebug("k:%2d highindex:%2d matching:%4.2f\n",
		k, highindex,
		matching[ajSeqcvtGetCodeK(cvt, seqcharptr[highindex][k])]);
	if(highindex != -1 &&
	   matching[ajSeqcvtGetCodeK(cvt, seqcharptr[highindex][k])] >= fplural)
	{
	    iscons = ajTrue;
	    boxindex = highindex;
	}
	else
	{
	    for(i=0;i<numseq;i++)
	    {
		m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
		if(matching[m1] > max)
		{
		    max = matching[m1];
		    highindex = i;
		}
		else if(matching[m1] == max)
		{
		    if(identical[m1] >
		       identical[ajSeqcvtGetCodeK(cvt,
                                                  seqcharptr[highindex][k])] )
		    {
			max = matching[m1];
			highindex = i;
		    }
		}
	    }

	    if(matching[ajSeqcvtGetCodeK(cvt,
                                         seqcharptr[highindex][k])] >= fplural)
	    {
		iscons = ajTrue;
		boxindex = highindex;
	    }
	}


	if(iscons)
	{
	    if(!collision)
	    {
		/* check for collisions */
		if(alternative == 1)
		{
		    /* check to see if this is unique for collisions */
		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
			if(identical[m1] >= identical[identicalmaxindex] &&
			   m1 != identicalmaxindex)
			    iscons = ajFalse;
		    }

		    /*ajDebug("after (alt=1) iscons: %B",iscons);*/
		}

		else if(alternative == 2)
		{
		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);

			if((matching[m1] >= matching[matchingmaxindex] &&
			    m1 != matchingmaxindex &&
			    matrix[m1][matchingmaxindex] < 0.1)||
			   (identical[m1] >= identical[matchingmaxindex]
			   && m1 != matchingmaxindex))
			    iscons = ajFalse;
		    }
		}
		else if(alternative == 3)
		{
		    /*
		    ** to do this check one is NOT in consensus to see if
		    ** another score of fplural has been found
		    */
		    ms = ajSeqcvtGetCodeK(cvt, seqcharptr[highindex][k]);

		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
			if(ms != m1 && colcheck[m1] == 0.0)
			    /* NOT in the current consensus */
			    for(j=0;j<numseq;j++)
			    {
				m2 = ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]);
				if( matrix[ms][m2] < 0.1)
				{
				    /* NOT in the current consensus */
				    if( matrix[m1][m2] > 0.1)
					colcheck[m1] +=
                                            ajSeqsetGetseqWeight(seqset,
                                                                 j);
				}
			    }
		    }

		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
			/* if any other matches then we have a collision */
			if(colcheck[m1] >= fplural)
			    iscons = ajFalse;
		    }

		    /*ajDebug("after alt=2 iscons: %B", iscons);*/
		}
		else
		{
		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
			if((matching[m1] >= matching[matchingmaxindex] &&
			    m1 != matchingmaxindex &&
			    matrix[m1][matchingmaxindex] < 0.1))
			    iscons = ajFalse;
			if(identical[m1] >= identical[matchingmaxindex] &&
			   m1 != matchingmaxindex &&
			   matrix[m1][matchingmaxindex] > 0.1)
			    iscons = ajFalse;
		    }

		    if(!iscons)
		    {	/* matches failed try identicals */
			if(identical[identicalmaxindex] >= fplural)
			{
			    iscons = ajTrue;
			    /*
			    ** if nothing has an equal or higher match that
			    ** does not match highest then false
			    */
			    for(i=0;i<numseq;i++)
			    {
				m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[i][k]);
				if(identical[m1] >=
				   identical[identicalmaxindex] &&
				   m1 != identicalmaxindex)
				    iscons = ajFalse;
				else if(matching[m1] >=
					matching[identicalmaxindex] &&
					matrix[m1][matchingmaxindex] <= 0.0)
				    iscons = ajFalse;
				else if(m1 == identicalmaxindex)
				    j = i;
			    }

			    if(iscons)
				highindex = j;
			}
		    }

		}
	    }

	    if(identity)
	    {
		j = 0;
		for(i=0;i<numseq;i++)
		    if(seqcharptr[highindex][k] == seqcharptr[i][k])
			j++;

		if(j<identity)
		    iscons = ajFalse;
	    }
	}

	/*
	** Done a full line of residues
	** Boxes have been defined up to this point
	*/
	if(count >= numres )
	{
	    /* check y position for next set */
	    y=y-(yincr*((float)numseq+(float)2.0+((float)consensus*(float)2)));
	    if(y<yincr*((float)numseq+(float)2.0+((float)consensus*(float)2)))
	    {
		/* full page - print it */
		y=ystart-(float)6.0;

		startseq = 0;
		endseq = seqperpage;
		newILstart = newILend;
		newILend = k;
		while(startseq < numseq)
		{
		    /* AJB */
		    /*if(startseq != 0)
		    	ajGraphNewpage(graph, AJFALSE);*/

		    /*ajDebug("Inner loop: startseq: %d numseq: %d endseq: %d\n",
			    startseq, numseq, endseq);*/
		    if(endseq>numseq)
			endseq=numseq;
		    prettyplot_fillinboxes(numseq,ajSeqsetGetLen(seqset),
					   startseq,endseq,
					   newILstart,newILend,
					   numres,resbreak,
					   boxit,boxcol,consensus,
					   ystart,yincr,cvt);
		    startseq = endseq;
		    endseq += seqperpage;
		    ajGraphNewpage(graph, AJFALSE);
		}
	    }

	    count = 0;
	    gapcount = 0;
	}

	count++;
	countforgap++;

	for(j=0;j<numseq;j++)
	{
	    /* START OF BOXES */

	    if(boxit)
	    {
		seqboxptr[j][k] = 0;
		if(boxindex!=-1)
		{
		    myindex = boxindex;
		    if(matrix[ajSeqcvtGetCodeK(cvt, seqcharptr[j][k])]
		       [ajSeqcvtGetCodeK(cvt, seqcharptr[myindex][k])] > 0)
			part = 1.0;
		    else
		    {
			if(identical[ajSeqcvtGetCodeK(cvt,
                                                      seqcharptr[j][k])] >=
			   fplural)
			    part = 1.0;
			else
			    part = 0.0;
		    }

		    if(previous[j] != part)
			/* draw vertical line */
			seqboxptr[j][k] |= BOXLEF;

		    if(j==0)
		    {
			/* special case for horizontal line */
			if(part)
			{
			    currentstate = 1;
			    /* draw hori line */
			    seqboxptr[j][k] |= BOXTOP;
			}
			else
			    currentstate = 0;
		    }
		    else
		    {
			/* j != 0  Normal case for horizontal line */
			if(part != currentstate)
			{
			    /*draw hori line */
			    seqboxptr[j][k] |= BOXTOP;
			    currentstate = (ajint) part;
			}
		    }

		    if(j== numseq-1 && currentstate)
			/* draw horiline at bottom */
			seqboxptr[j][k] |= BOXBOT;

		    previous[j] = (ajint) part;
		}
		else
		{
		    part = 0;
		    if(previous[j])
		    {
			/* draw vertical line */
			seqboxptr[j][k] |= BOXLEF;
		    }
		    previous[j] = 0;
		}

		if(count == numres || k == kmax || countforgap >= resbreak )
		{			/* last one on the row or a break*/
		    if(previous[j])
		    {
			/* draw vertical line */
			seqboxptr[j][k] |= BOXRIG;
		    }
		    previous[j] = 0;
		}

	    } /* end box */

	    if(boxit && boxcol)
		if(boxindex != -1)
		{
		    myindex = boxindex;
		    if(matrix[ajSeqcvtGetCodeK(cvt, seqcharptr[j][k])]
		       [ajSeqcvtGetCodeK(cvt, seqcharptr[myindex][k])] > 0
		       || identical[ajSeqcvtGetCodeK(cvt, seqcharptr[j][k])] >=
                       fplural )

			seqboxptr[j][k] |= BOXCOLOURED;
		}

	    /* END OF BOXES */




	    if(ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]))
		res[0] = seqcharptr[j][k];
	    else
		res[0] = '-';

	    if(colourbyconsensus)
	    {
		part = (float) matrix[ajSeqcvtGetCodeK(cvt, seqcharptr[j][k])]
		    [ajSeqcvtGetCodeK(cvt, seqcharptr[highindex][k])];
		if(iscons && seqcharptr[highindex][k] == seqcharptr[j][k])
		    seqcolptr[j][k] = cidentity;
		else if(part > 0.0)
		    seqcolptr[j][k] = csimilarity;
		else
		    seqcolptr[j][k] = cother;
	    }
	    else if(colourbyresidues)
		seqcolptr[j][k] = colmat[ajSeqcvtGetCodeK(cvt,
                                                          seqcharptr[j][k])];
	    else if(iscons && colourbyshade)
	    {
		part = (float) matrix[ajSeqcvtGetCodeK(cvt, seqcharptr[j][k])]
		    [ajSeqcvtGetCodeK(cvt, seqcharptr[highindex][k])];
		if(part >= 1.5)
		    seqcolptr[j][k] = shadecolour[0];
		else if(part >= 1.0)
		    seqcolptr[j][k] = shadecolour[1];
		else if(part >= 0.5)
		    seqcolptr[j][k] = shadecolour[2];
		else
		    seqcolptr[j][k] = shadecolour[3];
	    }
	    else if(colourbyshade)
		seqcolptr[j][k] = shadecolour[3];
	    else
		seqcolptr[j][k] = BLACK;
	}

	if(consensus)
	{
	    if(iscons)
		res[0] = seqcharptr[highindex][k];
	    else
		res[0] = '-';
	    strcat(constr,res);
	}

	if(countforgap >= resbreak)
	{
	    gapcount++;
	    countforgap=0;
	}
    }


    startseq = 0;
    endseq=seqperpage;
    newILstart = newILend;
    newILend = k;
    while(startseq < numseq)
    {
	if(startseq)
	    ajGraphNewpage(graph, AJFALSE);

	/*ajDebug("Final loop: startseq: %d numseq: %d endseq: %d\n",
		startseq, numseq, endseq);*/
	if(endseq>numseq)
	    endseq = numseq;
	prettyplot_fillinboxes(numseq,ajSeqsetGetLen(seqset),
			       startseq,endseq,
			       newILstart,newILend,
			       numres,resbreak,
			       boxit,boxcol,consensus,
			       ystart,yincr,cvt);
	startseq = endseq;
	endseq += seqperpage;
    }


    ajGraphicsGetCharsize(&defheight,&currentscale);

    if(boxit && boxcol)
	oldfg = ajGraphicsSetFgcolour(oldfg);

    ajGraphicsCloseWin();
    ajGraphxyDel(&graph);

    ajStrDel(&sidentity);
    ajStrDel(&ssimilarity);
    ajStrDel(&sother);
    ajStrDel(&options);
    ajStrDel(&altstr);

    ajStrDel(&matcodes);

    for(i=0;i<numseq;i++)
    {
	ajStrDel(&seqnames[i]);
	AJFREE(seqcolptr[i]);
	if(seqboxptr)
            AJFREE(seqboxptr[i]);
    }
    AJFREE(seqcolptr);
    AJFREE(seqboxptr);

    AJFREE(seqnames);
    AJFREE(score);
    AJFREE(previous);
    AJFREE(seqcount);

    AJFREE(colmat);
    AJFREE(shadecolour);

    freeptr = (void *) seqcharptr;
    AJFREE(freeptr);

    AJFREE(identical);
    AJFREE(matching);
    AJFREE(colcheck);

    ajSeqsetDel(&seqset);
    ajMatrixDel(&cmpmatrix);
    ajStrDel(&shade);
    ajStrDel(&sboxcolval);
    ajStrDel(&sidentity);
    ajStrDel(&ssimilarity);
    ajStrDel(&sother);
    ajFloatDel(&pair);
    ajTimeDel(&ajtime);
    AJFREE(constr);

    embExit();

    return 0;
}




/* @funcstatic prettyplot_calcseqperpage **************************************
**
** Calculate the number of sequences per page.
**
** @param [r] yincr [float] Undocumented
** @param [r] y [float] Undocumented
** @param [r] consensus [AjBool] If true, include a consensus sequence
** @return [ajint] Number of sequences that fit on one page
** @@
******************************************************************************/


static ajint prettyplot_calcseqperpage(float yincr,float y,AjBool consensus)
{
    float yep = 1.0;
    ajint numallowed = 1;

    while(yep>0.0)
    {
	yep = y-(yincr*((float)numallowed+(float)2.0+((float)consensus*
						      (float)2)));
	numallowed++;
    }

    return numallowed-1;
}




/* @funcstatic prettyplot_fillinboxes *****************************************
**
** Plot the page
**
** @param [r] numseq [ajint] Number of sequences in alignment
** @param [r] length [ajint] Alignment length
** @param [r] seqstart [ajint] First sequence to plot
** @param [r] seqend [ajint] Last sequence to plot
** @param [r] start [ajint] First residue
** @param [r] end [ajint] Last residue
** @param [r] numres [ajint] Number of residues per line
** @param [r] resbreak [ajint] Number of residues per block
** @param [r] boxit [AjBool] If true, display box outline
** @param [r] boxcol [AjBool] If true, colour the background in each box
** @param [r] consensus [AjBool] If true, include consensus sequence
**                               on last line
** @param [r] ystart [float] Vertical start. 1.0 is the top.
** @param [r] yincr [float] Vertical increment to next row
** @param [r] cvt [const AjPSeqCvt] Conversion table from residue code
**                                  to matrix position
** @return [void] 
** @@
******************************************************************************/

static void prettyplot_fillinboxes(ajint numseq, ajint length, 
				   ajint seqstart, ajint seqend,
				   ajint start, ajint end,
				   ajint numres, ajint resbreak,
				   AjBool boxit, AjBool boxcol, 
				   AjBool consensus,
				   float ystart, float yincr,
				   const AjPSeqCvt cvt)
{
    ajint count = 1;
    ajint gapcount = 0;
    ajint countforgap = 0;
    ajint table[16];
    ajint i;
    ajint j;
    ajint k;
    ajint w;
    ajint oldfg = 0;
    ajint oldcol = 0;
    ajint l;
    float y;
    char res[2]=" ";
    AjPStr strcon = NULL;
    char numberstring[10];
    float defcs = 0.;
    float curcs = 0.;

/*
    ajDebug("fillinboxes numseq: %d length: %d\n",
	    numseq, length);
    ajDebug("fillinboxes start: %d end: %d seqstart: %d seqend; %d\n",
	    start, end, seqstart, seqend);
    ajDebug("fillinboxes numres: %d resbreak: %d\n",
	    numres, resbreak);
    ajDebug("fillinboxes boxit: %B boxcol: %B consensus: %B \n",
	    boxit, boxcol, consensus);
    ajDebug("fillinboxes xmid: %.3f ystart:%.3f yincr: %.3f\n",
	    xmid, ystart, yincr);
*/
    ajStrAppendC(&strcon,"Consensus");
    ajStrTruncateLen(&strcon,charlen);

    if(boxit && boxcol)
    {
	y = ystart-(float)6.0;
	for(k=start; k< end; k++)
	{
	    if(countforgap >= resbreak)
	    {
		gapcount++;
		countforgap = 0;
	    }
	    if(count >= numres+1 )
	    {
		y = y-(yincr*((float)numseq+(float)2.0+((float)consensus*
							(float)2)));
		if(y<yincr*((float)numseq+(float)2.0+((float)consensus*
						      (float)2)))
		{
		    y = ystart-(float)6.0;
		}
		count = 1;
		gapcount = 0;
	    }
	    count++;
	    countforgap++;
	    /* thiscol = ajGraphicsGetColourFore(); Unused */

	    for(j=seqstart,l=0;j<seqend;j++,l++)
		if(seqboxptr[j][k] & BOXCOLOURED)
		{
		    ajGraphicsDrawposRectFill((float)(count+gapcount-(float)1)+
                                              (float)1.0,
                                              y-(yincr*((float)l+(float)0.5)),
                                              (float)(count+gapcount-(float)1),
                                              y-(yincr*((float)l-(float)0.5)));
		}
	}
    }

    oldcol = ajGraphicsSetFgcolour(BLACK);

    /* DO THE BACKGROUND OF THE BOXES FIRST */

    count = 0;
    gapcount = countforgap = 0;
    y = ystart-(float)6.0;

    ajGraphicsGetCharsize(&defcs,&curcs);

    if(shownames)
    {
	for(i=seqstart,l=0;i<seqend;i++,l++)
	    ajGraphicsDrawposTextAtstart((float)-charlen,y-(yincr*l),
                                         ajStrGetPtr(seqnames[i]));

	if(consensus && (numseq==seqend))
	    ajGraphicsDrawposTextAtstart(
                (float)-charlen,
                y-(yincr*((seqend-seqstart)+(float)1)),ajStrGetPtr(strcon));

    }

    for(k=start; k< end; k++)
    {
	if(countforgap >= resbreak)
	{
	    gapcount++;
	    countforgap = 0;
	}

	if(count >= numres )
	{
	    if(shownumbers)
	    {
		for(j=seqstart,l=0;j<seqend;j++,l++)
		{
		    sprintf(numberstring,"%d",seqcount[j]);
		    ajGraphicsDrawposTextAtstart(
                        (float)(count+numgaps)+(float)5.0,
                        y-(yincr*(float)l),numberstring);
		}

		if(consensus && (numseq==seqend))
		{
		    sprintf(numberstring,"%d",k);
		    ajGraphicsDrawposTextAtstart(
                        (float)(count+numgaps)+(float)5.0,
                        y-(yincr*((float)l+(float)1)),
                        numberstring);
		}
	    }

	    y = y-(yincr*((float)numseq+(float)2.0+((float)consensus*
						    (float)2)));
	    if(y<yincr*((float)numseq+(float)2.0+((float)consensus*(float)2)))
	    {
		y = ystart-(float)6.0;
	    }

	    count = 0;
	    gapcount = 0;
	    if(shownames)
	    {
		for(i=seqstart,l=0;i<seqend;i++,l++)
		    ajGraphicsDrawposTextAtstart((float)-charlen,y-(yincr*l),
                                                 ajStrGetPtr(seqnames[i]));

		if(consensus &&(numseq==seqend))
		    ajGraphicsDrawposTextAtstart(
                        (float)-charlen,
                        y-(yincr*((seqend-seqstart)+1)),
                        ajStrGetPtr(strcon));
	    }
	}
	count++;
	countforgap++;

	if(boxit)
	{
	    for(j=seqstart,l=0; j< seqend; j++,l++)
	    {
		if(ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]))
		    seqcount[j]++;
		if(seqboxptr[j][k] & BOXLEF)
		    ajGraphicsDrawposLine((float)(count+gapcount),y-
                                          (yincr*((float)l+(float)0.5)),
                                          (float)(count+gapcount),
                                          y-(yincr*((float)l-(float)0.5)));

		if(seqboxptr[j][k] & BOXTOP)
		    ajGraphicsDrawposLine((float)(count+gapcount),y-
                                          (yincr*((float)l-(float)0.5)),
                                          (float)(count+gapcount)+(float)1.0,
                                          y-(yincr*((float)l-(float)0.5)));

		if(seqboxptr[j][k] & BOXBOT)
		    ajGraphicsDrawposLine((float)(count+gapcount),y-
                                          (yincr*((float)l+(float)0.5)),
                                          (float)(count+gapcount)+(float)1.0,
                                          y-(yincr*((float)l+(float)0.5)));

		if(seqboxptr[j][k] & BOXRIG)
		    ajGraphicsDrawposLine((float)(count+gapcount)+(float)1.0,y-
                                          (yincr*((float)l+(float)0.5)),
                                          (float)(count+gapcount)+(float)1.0,
                                          y-(yincr*((float)l-(float)0.5)));
	    }
	}
	else if(shownumbers)	/* usually set in the boxit block */
	{
	  for(j=seqstart,l=0; j< seqend; j++,l++)
	  {
	    if(ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]))
	      seqcount[j]++;
	  }
	}
	if(consensus && (numseq==seqend))
	{
	    res[0] = constr[k];

	    /* start -> mid */
            ajGraphicsDrawposTextAtmid((float) 0.5 + (float)(count+gapcount),
                                       y-(yincr*((seqend-seqstart)+1)),res);
	}
    }

    if(shownumbers)
    {
	for(j=seqstart,l=0;j<seqend;j++,l++)
	{
	    sprintf(numberstring,"%d",seqcount[j]);
	    ajGraphicsDrawposTextAtstart((float)(count+numgaps)+
                                         (float)5.0,y-(yincr*l),
                                         numberstring);
	}

	if(consensus && (numseq==seqend))
	{
	    sprintf(numberstring,"%d",k);
	    ajGraphicsDrawposTextAtstart((float)(count+numgaps)+(float)5.0,
                                         y-(yincr*(l+(float)1)),
                                         numberstring);
	}
    }


    ajStrDel(&strcon);

    for(i=0;i<16;i++)
	table[i] = -1;
    for(i=0;i<numseq;i++)
	for(k=0; k< length; k++)
	    table[seqcolptr[i][k]] = 1;

    /* now display again but once for each colour */

    /*    for(w=0;w<15;w++)*/
    for(w=0;w<16;w++)
    {
	/* not 16 as we can ignore white on plotters*/
	if(table[w] > 0)
	{
	    oldfg = ajGraphicsSetFgcolour(w);
	    count = 0;
	    gapcount = countforgap = 0;

	    y = ystart-(float)6.0;

	    for(k=start; k< end; k++)
	    {
		if(countforgap >= resbreak)
		{
		    gapcount++;
		    countforgap=0;
		}

		if(count >= numres )
		{
		    y=y-(yincr*((float)(seqend-seqstart)+(float)2.0+
				((float)consensus*(float)2)));
		    count = 0;
		    gapcount = 0;
		}
		count++;
		countforgap++;

		for(j=seqstart,l=0; j< seqend; j++,l++){
		    if(seqcolptr[j][k] == w)
		    {
			if(ajSeqcvtGetCodeK(cvt, seqcharptr[j][k]))
			    res[0] = seqcharptr[j][k];
			else
			    res[0] = '-';
	    /* start -> mid */
			ajGraphicsDrawposTextAtmid((float) 0.5 +
                                                   (float) (count+gapcount),
                                                   y-(yincr*l),res);
		    }
		}
	    }
	}
	oldfg = ajGraphicsSetFgcolour(oldfg);
    }

    oldfg = ajGraphicsSetFgcolour(oldcol);
    start = end;

    return;
}
