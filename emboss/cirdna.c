/* @source cirdna application
**
** Draws circular maps of DNA constructs
** @author Copyright (C) Nicolas Tourasse (tourasse@biotek.uio.no),
** Biotechnology Centre of Oslo, Norway.
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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>




static AjBool cirdna_ReadInput(AjPFile infile,
                               float *Start, float *End);
static AjPStr cirdna_ReadGroup(AjPFile infile, ajint maxlabels,
			       float* From, float *To,
			       AjPStr *Name2, char *FromSymbol,
			       char *ToSymbol, AjPStr *Style2,
			       ajint *NumLabels, ajint *NumNames,
			       ajint *Colour);
static float cirdna_TextGroup(float TextHeight, float TextLength,
			      AjPStr const *Name2, ajint NumLabels,
			      const ajint *NumNames,
			      AjPStr const *Style2,
			      const float* From,
			      const float* To,
			      const AjPStr PosTicks);
static float cirdna_TextGroupStr(AjPStr const *Name2, ajint NumLabels,
				 const ajint *NumNames,
				 float TextCoef, AjPStr const *Style2,
				 const float* From,
				 const float* To,
				 const AjPStr PosTicks);
static float cirdna_HeightGroup(float postext,
				float TickHeight, float BlockHeight,
				float RangeHeight,
				AjPStr const *Style2, ajint NumLabels,
				const AjPStr PosTicks,
				const AjPStr PosBlocks, ajint Adjust);
static ajint cirdna_OverlapTextGroup(AjPStr const *Name2,
				     AjPStr const *Style2,
				     ajint NumLabels,
				     const float* From, const float* To,
				     float Start, float End,
				     const AjPStr PosTicks, ajint *Adjust);
static AjBool cirdna_OverlapTickRuler(ajint NumGroups, const ajint *NumLabels,
				     float* const * From,
				      const AjPStr PosTicks,
				      ajint RulerTick);
static void cirdna_DrawGroup(float xDraw, float yDraw, float posblock,
			     float posrange, float postext, float TickHeight,
			     float BlockHeight, float RangeHeight,
			     float RealLength, float Radius, float RadiusMax,
			     const float* From, const float* To,
			     AjPStr const *Name2,
			     const char *FromSymbol, const char *ToSymbol,
			     AjPStr const *Style2, AjBool InterSymbol,
			     AjBool InterTicks, ajint NumLabels,
			     float OriginAngle,
			     const ajint *NumNames, const AjPStr PosTicks,
			     const AjPStr PosBlocks, const ajint *Adjust,
			     ajint InterColour, const ajint *Colour,
			     const AjPStr BlockType);
static float cirdna_TextRuler(float Start, float End, ajint GapSize,
			      float TextLength, float TextHeight,
			      const AjPStr PosTicks, ajint NumGroups,
			      const ajint *NumLabels);
static float cirdna_TextRulerStr(float Start, float End, ajint GapSize,
				 float TextCoef, const AjPStr PosTicks,
				 ajint NumGroups, const ajint *NumLabels);
static float cirdna_HeightRuler(float Start, float End, ajint GapSize,
				float postext, float TickHeight,
				const AjPStr PosTicks, ajint NumGroups,
				const ajint *NumLabels);
static void cirdna_DrawRuler(float xDraw, float yDraw, float Start, float End,
			     float RealLength, float Radius, float TickHeight,
			     float OriginAngle, ajint GapSize,
			     AjBool TickLines, float postext,
			     ajint NumGroups, const ajint *NumLabels,
			     float* const * From,
			     const AjPStr PosTicks, ajint Colour);
static void cirdna_DrawTicks(float xDraw, float yDraw, float RealLength,
			     float Radius, float TickHeight, float From,
			     const AjPStr Name2, float OriginAngle,
			     float postext,
			     const AjPStr PosTicks,
			     ajint NumNames, ajint Adjust,
			     ajint Colour);
static void cirdna_DrawBlocks(float xDraw, float yDraw, float RealLength,
			      float Radius, ajint BlockHeight, float From,
			      float To, const AjPStr Name2,
			      float postext, float OriginAngle,
			      const AjPStr PosBlocks, ajint NumNames,
			      ajint Adjust,
			      ajint Colour, const AjPStr BlockType);
static void cirdna_DrawRanges(float xDraw, float yDraw, float RealLength,
			      float Radius, float RangeHeight, float From,
			      float To, char FromSymbol,
			      char ToSymbol, const AjPStr Name2,
			      float OriginAngle,
			      ajint NumNames, float postext, ajint Adjust,
			      ajint Colour);
static void cirdna_InterBlocks(float xDraw, float yDraw, float RealLength,
			       float Radius, float BlockHeight, float From,
			       float To, float OriginAngle,
			       AjBool InterSymbol, ajint Colour);
static void cirdna_DrawArrowHeadsOncurve(float xDraw, float yDraw,
					 float RealLength, float Height,
					 float Length, float Radius,
					 float Angle, ajint Way);
static void cirdna_DrawBracketsOncurve(float xDraw, float yDraw,
				       float RealLength, float Height,
				       float Length, float Radius,
				       float Angle, ajint Way);
static void cirdna_DrawBarsOncurve(float xDraw, float yDraw,
				       float Height, float Radius,
				       float Angle);
static void cirdna_HorTextPile(float x, float y, float Radius,
			       float StartAngle, float EndAngle,
			       const AjPStr Name2,
			       float postext, ajint NumNames);
static float cirdna_HorTextPileHeight(float postext, ajint NumNames);
static float cirdna_HorTextPileLengthMax(const AjPStr Name2, ajint NumNames);
static float cirdna_ComputeAngle(float RealLength, float Length,
				 float OriginAngle);
static float cirdna_ComputeArc(float RealLength, float Length);



static AjPStr** Style=NULL;
static AjPStr** Name=NULL;
/*static AjPStr Style[MAXGROUPS][MAXLABELS]; */
/*static AjPStr Name[MAXGROUPS][MAXLABELS];*/

static ajint cirdnaMaxinter=0;
static ajint* cirdnaInter=NULL;
/*ajint Inter[MAXLABELS];*/


static float* cirdnaFromText=NULL;
static float* cirdnaToText=NULL;
/*    float FromText[MAXLABELS];*/
/*    float ToText[MAXLABELS];*/




/* @prog cirdna ***************************************************************
**
** Draws circular maps of DNA constructs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPGraph graph;
    ajint i;
    ajint j;
    ajint GapSize;
    ajint* NumLabels;
    ajint** NumNames;
/*    ajint NumNames[MAXGROUPS][MAXLABELS];*/
    ajint NumGroups;
    ajint InterColour;
    ajint** Colour;
    ajint** Adjust;
    ajint* AdjustMax=NULL;
    char** FromSymbol=NULL;
    char** ToSymbol=NULL;
/*    ajint Colour[MAXGROUPS][MAXLABELS];*/
/*    ajint Adjust[MAXGROUPS][MAXLABELS];*/
/*    ajint AdjustMax[MAXGROUPS];*/
/*    char FromSymbol[MAXGROUPS][MAXLABELS];*/
/*    char ToSymbol[MAXGROUPS][MAXLABELS];*/
    float xDraw;
    float yDraw;
    float Radius;
    float RadiusMax;
    float DrawRadius;
    float OriginAngle;
    float** From=NULL;
    float** To=NULL;
/*    float From[MAXGROUPS][MAXLABELS];*/
/*    float To[MAXGROUPS][MAXLABELS];*/
    float TotalHeight;
    float* GroupHeight=NULL;
/*    float GroupHeight[MAXGROUPS];*/
    float RulerHeight;
    float Width;
    float Height;
    float Border;
    float Start;
    float End;
    float DrawLength;
    float RealLength;
    float TickHeight;
    float BlockHeight;
    float RangeHeight;
    float TextLength;
    float TextHeight;
    float GapGroup;
    float posblock;
    float posrange;
    float postext;
    AjPFile infile;
    AjPStr line;
    AjPStr* GroupName=NULL;
/*    AjPStr GroupName[MAXGROUPS];*/
    AjBool Ruler;
    AjBool InterSymbol;
    AjBool InterTicks;
    AjPStr PosTicks;
    AjBool TickLines;
    AjPStr BlockType;
    AjPStr PosBlocks;
    float charsize;
    float minsize;
    ajint maxgroups;
    ajint maxlabels;

    /* read the ACD file for graphical programs */
    embInit("cirdna", argc, argv);

    /* array size limits */
    maxgroups = ajAcdGetInt("maxgroups");
    maxlabels = ajAcdGetInt("maxlabels");

    /* to draw or not to draw the ruler */
    Ruler = ajAcdGetBoolean("ruler");

    /* get the type of blocks */
    BlockType = ajAcdGetListSingle("blocktype");

    /* get the angle of the molecule's origin */
    OriginAngle = ajAcdGetFloat("originangle");

    /* get the position of the ticks */
    PosTicks = ajAcdGetSelectSingle("posticks");

    /* get the position of the text for blocks */
    PosBlocks = ajAcdGetSelectSingle("posblocks");

    /* to draw or not to draw junctions to link blocks */
    InterSymbol = ajAcdGetBoolean("intersymbol");
    /* get the colour of junctions used to link blocks */
    InterColour = ajAcdGetInt("intercolour");

    /* to draw or not to draw junctions between ticks */
    InterTicks = ajAcdGetBoolean("interticks");

    /* get the size of the intervals between the ruler's ticks */
    GapSize = ajAcdGetInt("gapsize");
    /* to draw or not to draw vertical lines at ruler's ticks */
    TickLines = ajAcdGetBoolean("ticklines");


    /* set the output graphical context */
    graph = ajAcdGetGraph("graphout");

    /* get the input file */
    infile = ajAcdGetInfile("infile");

    /* Allocate memory for the old fixed-length arrays */

    AJCNEW0(Style, maxgroups);
    AJCNEW0(Name, maxgroups);
    AJCNEW0(NumNames, maxgroups);
    AJCNEW0(NumLabels, maxgroups);
    AJCNEW0(Colour, maxgroups);
    AJCNEW0(Adjust, maxgroups);
    AJCNEW0(AdjustMax, maxgroups);
    AJCNEW0(FromSymbol, maxgroups);
    AJCNEW0(ToSymbol, maxgroups);
    AJCNEW0(From, maxgroups);
    AJCNEW0(To, maxgroups);
    AJCNEW0(GroupHeight, maxgroups);
    AJCNEW0(GroupName, maxgroups);

    /* length and height of text */
    /*TextHeight = 10;
    TextLength = TextHeight+25;*/
    TextHeight = 2*ajAcdGetFloat("textheight");
    TextLength = 7*ajAcdGetFloat("textlength");


    /* read the start and end positions */
    if(!cirdna_ReadInput(infile, &Start, &End))
        ajFatal("Error processing input file");

    /* compute the real length of the molecule */
    RealLength = (End - Start) + 1;

    /* height of a tick, a block, and a range */
    TickHeight = 3*ajAcdGetFloat("tickheight");
    if( ajStrMatchCaseC(PosBlocks, "Out") )
	BlockHeight = 3*ajAcdGetFloat("blockheight");
    else
    {
	BlockHeight = (TextHeight+3)*ajAcdGetFloat("blockheight");
	if(BlockHeight<(TextHeight+3) )
	    BlockHeight = (TextHeight+3);
    }
    RangeHeight = 3*ajAcdGetFloat("rangeheight");

    /* set the relative positions of elements of a group */
    posblock = 0;
    posrange = 0;
    GapGroup = 3*ajAcdGetFloat("gapgroup");

    ajGraphAppendTitleS(graph, ajFileGetPrintnameS(infile));

    /* open the window in which the graphics will be drawn */
    ajGraphOpenMm(graph, &Width, &Height);

    Border = (float) 2.0 * (TickHeight+TextLength);
    DrawLength = Height - 2*Border;

    /* coordinates of the circle's center */
    xDraw = Width/(float)2.0;
    yDraw = Height/(float)2.0;

    /* radius of the outermost circle */
    Radius = RadiusMax = DrawLength/(float)2.0;

    /* read the contents of the groups */
    line = ajStrNew();
    ajFileSeek(infile, 0L, 0);
    i = 0;
    while( ajReadlineTrim(infile, &line) )
    {
	if( ajStrPrefixC(line, "group") )
	{
	    if (i == maxgroups)
		ajWarn("Too many groups (maxgroups=%d) in input", maxgroups);
	    if (i < maxgroups)
	    {
		AJCNEW0(Style[i], maxlabels);
		AJCNEW0(Name[i], maxlabels);
		AJCNEW0(NumNames[i], maxlabels);
		AJCNEW0(Colour[i], maxlabels);
		AJCNEW0(Adjust[i], maxlabels);
		AJCNEW0(FromSymbol[i], maxlabels);
		AJCNEW0(ToSymbol[i], maxlabels);
		AJCNEW0(From[i], maxlabels);
		AJCNEW0(To[i], maxlabels);

		GroupName[i] = cirdna_ReadGroup(infile, maxlabels,
						From[i], To[i],
						Name[i],
						FromSymbol[i], ToSymbol[i],
						Style[i], &NumLabels[i],
						NumNames[i], Colour[i]);
		j = NumLabels[i];
		AJCRESIZE(Style[i], j);
		AJCRESIZE(Name[i], j);
		AJCRESIZE(NumNames[i], j);
		AJCRESIZE(Colour[i], j);
		AJCRESIZE(Adjust[i], j);
		AJCRESIZE(FromSymbol[i], j);
		AJCRESIZE(ToSymbol[i], j);
		AJCRESIZE(From[i], j);
		AJCRESIZE(To[i], j);
	    }
	    i++;
	}
    }
    NumGroups = i;

    AJCRESIZE(Style, i);
    AJCRESIZE(Name, i);
    AJCRESIZE(NumNames, i);
    AJCRESIZE(NumLabels, i);
    AJCRESIZE(Colour, i);
    AJCRESIZE(Adjust, i);
    AJCRESIZE(AdjustMax, i);
    AJCRESIZE(FromSymbol, i);
    AJCRESIZE(ToSymbol, i);
    AJCRESIZE(From, i);
    AJCRESIZE(To, i);
    AJCRESIZE(GroupHeight, i);
    AJCRESIZE(GroupName, i);

    /* remove the beginning of the molecule in case it doesn't begin at 1 */
    if (Start != 1)
    {
	for(i=0; i<NumGroups; i++)
	    for(j=0; j<NumLabels[i]; j++)
	    {
		From[i][j] -= (Start-1);
		To[i][j]   -= (Start-1);
	    }
    }

    /* compute the character size that fits all groups, including the ruler */
    minsize = 100.0;
    charsize = cirdna_TextRuler(Start, End, GapSize, TextLength, TextHeight,
				PosTicks, NumGroups, NumLabels);
    if( charsize<minsize ) minsize = charsize;
    ajDebug("Calculated charsize: %f minsize:%f\n", charsize, minsize);
    for(i=0; i<NumGroups; i++)
    {
	charsize = cirdna_TextGroup(TextHeight, TextLength, Name[i],
				    NumLabels[i], NumNames[i],
				    Style[i], From[i], To[i],
				    PosTicks);
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphicsSetDefcharsize(minsize);
    ajDebug("Initial charsize: %f\n", minsize);


    /* find whether horizontal text strings overlap within a group */
    postext = (ajGraphicsCalcTextheight()+1)*ajAcdGetFloat("postext");
    for(i=0; i<NumGroups; i++)
	AdjustMax[i] = cirdna_OverlapTextGroup(Name[i], Style[i], NumLabels[i],
					       From[i], To[i], Start, End,
					       PosTicks, Adjust[i]);


    /* compute the height of the ruler */
    RulerHeight = cirdna_HeightRuler(Start, End, GapSize, postext, TickHeight,
				     PosTicks, NumGroups, NumLabels);
    if(!RulerHeight)
	ajDebug("Err: Ruler height");

    /* compute the height of the groups */
    TotalHeight = 0.0;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = cirdna_HeightGroup(postext,
					    TickHeight, BlockHeight,
					    RangeHeight, Style[i],
					    NumLabels[i],
					    PosTicks, PosBlocks, AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }

    /*
    **  decrease the radius such that the innermost group is not
    **  compressed in the centre of the circle
    */
    DrawRadius = Radius - (TotalHeight/NumGroups);

    /*
    **  if the groups are too big, resize them such that they fit in the
    **  window
    */
    if(TotalHeight<DrawRadius)
	TotalHeight = DrawRadius;
    TickHeight/=(TotalHeight/DrawRadius);
    BlockHeight/=(TotalHeight/DrawRadius);
    RangeHeight/=(TotalHeight/DrawRadius);
    TextHeight/=(TotalHeight/DrawRadius);
    TextLength/=(TotalHeight/DrawRadius);
    postext/=(TotalHeight/DrawRadius);
    posblock/=(TotalHeight/DrawRadius);
    posrange/=(TotalHeight/DrawRadius);
    GapGroup/=(TotalHeight/DrawRadius);

    /*
    **  the groups having been resized, recompute the character size that
    ** fits all groups, including the ruler
    */
    minsize = 100.0;
    charsize = cirdna_TextRulerStr(Start, End, GapSize,
				   (TotalHeight/DrawRadius),
				   PosTicks, NumGroups, NumLabels);
    ajDebug("Resized calculated charsize: %f minsize:%f\n", charsize, minsize);
    if(charsize<minsize)
	minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
	charsize = cirdna_TextGroupStr(Name[i], NumLabels[i], NumNames[i],
				       (TotalHeight/DrawRadius),
				       Style[i], From[i], To[i],
				       PosTicks);
	if(charsize<minsize)
	    minsize = charsize;
    }
    ajGraphicsSetDefcharsize(minsize);
    ajDebug("Resized charsize: %f\n", minsize);


    /* the ruler having been resized, recompute its height */
    RulerHeight = cirdna_HeightRuler(Start, End, GapSize, postext, TickHeight,
				     PosTicks, NumGroups, NumLabels);
    /* the groups having been resized, recompute their height */
    TotalHeight = 0.0;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = cirdna_HeightGroup(postext,
					    TickHeight, BlockHeight,
					    RangeHeight,
					    Style[i], NumLabels[i],
					    PosTicks, PosBlocks,
					    AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }

    /* draw the ruler */
    if(Ruler)
	cirdna_DrawRuler(xDraw, yDraw, Start, End,
			 RealLength, Radius, TickHeight,
			 OriginAngle, GapSize, TickLines,
			 postext, NumGroups, NumLabels,
			 From, PosTicks, 1);

    /* draw the groups */
    for(i=0; i<NumGroups; i++)
    {
	Radius -= (GroupHeight[i]+GapGroup);
	cirdna_DrawGroup(xDraw, yDraw, posblock, posrange, postext, TickHeight,
			 BlockHeight, RangeHeight, RealLength,
			 Radius, RadiusMax, From[i], To[i],
			 Name[i], FromSymbol[i], ToSymbol[i], Style[i],
			 InterSymbol, InterTicks, NumLabels[i],
			 OriginAngle, NumNames[i], PosTicks, PosBlocks,
			 Adjust[i], InterColour, Colour[i], BlockType);
	ajStrDel(&GroupName[i]);
    }


    /* close the input file */
    ajFileClose(&infile);
    ajStrDel(&line);

    ajStrDel(&PosTicks);
    ajStrDel(&PosBlocks);
    ajStrDel(&BlockType);

    /* close the graphical window */
    ajGraphicsClose();
    ajGraphxyDel(&graph);

    for(i=0;i<NumGroups;i++)
    {
	for(j=0;j<NumLabels[i];j++)
	{
	    ajStrDel(&Style[i][j]);
	    ajStrDel(&Name[i][j]);
	}
	ajStrDel(&GroupName[i]);
	AJFREE(NumNames[i]);
	AJFREE(Colour[i]);
	AJFREE(Adjust[i]);
	AJFREE(FromSymbol[i]);
	AJFREE(ToSymbol[i]);
	AJFREE(From[i]);
	AJFREE(To[i]);
	AJFREE(Name[i]);
	AJFREE(Style[i]);
    }
    AJFREE(AdjustMax);
    AJFREE(GroupHeight);
    AJFREE(NumLabels);
    AJFREE(GroupName);
    AJFREE(NumNames);
    AJFREE(Style);
    AJFREE(Name);
    AJFREE(Colour);
    AJFREE(Adjust);
    AJFREE(FromSymbol);
    AJFREE(ToSymbol);
    AJFREE(From);
    AJFREE(To);
    AJFREE(cirdnaInter);
    AJFREE(cirdnaFromText);
    AJFREE(cirdnaToText);

    embExit();

    return 0;
}




/* @funcstatic cirdna_TextRuler ***********************************************
**
** Compute the character size that fits all elements of the ruler
** provided that the height and the length of all strings are at
** most TextHeight and TextLength, respectively
**
** @param [r] Start [float] Undocumented
** @param [r] End [float] Undocumented
** @param [r] GapSize [ajint] Undocumented
** @param [r] TextLength [float] Undocumented
** @param [r] TextHeight [float] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] NumGroups [ajint] Undocumented
** @param [r] NumLabels [const ajint*] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_TextRuler(float Start, float End, ajint GapSize,
			      float TextLength, float TextHeight,
			      const AjPStr PosTicks, ajint NumGroups,
			      const ajint *NumLabels)
{
    ajint i;
    ajint j;
    const AjPStr token;
    AjPStr string;
    float charsize;
    float minsize = 100.0;

    string = ajStrNew();

    ajStrFromInt(&string, (ajint)Start);
    charsize = ajGraphicsCalcCharsize(0, 0, TextLength, TextLength,
                                      ajStrGetPtr(string), TextHeight );
    if(charsize < minsize)
	minsize = charsize;

    for(i=GapSize; i<End; i+=GapSize)
	if(i>Start)
	{
	    ajStrFromInt(&string, i);
	    charsize = ajGraphicsCalcCharsize(0, 0, TextLength, TextLength,
                                              ajStrGetPtr(string), TextHeight );
	    if(charsize < minsize)
		minsize = charsize;
	}

    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	    if(ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") )
	    {
		token = ajStrParseC(Name[i][j], ";");
		charsize = ajGraphicsCalcCharsize( 0, 0, TextLength,
                                                   TextLength,
                                                   ajStrGetPtr(token),
                                                   TextHeight );
		if(charsize < minsize)
		    minsize = charsize;
	    }

    ajStrDel(&string);

    return minsize;
}




/* @funcstatic cirdna_TextRulerStr ********************************************
**
** compute the character size that fits all elements of the ruler provided
** that the height and the length of all strings are multiplied by TextCoef
**
** @param [r] Start [float] Undocumented
** @param [r] End [float] Undocumented
** @param [r] GapSize [ajint] Undocumented
** @param [r] TextCoef [float] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] NumGroups [ajint] Undocumented
** @param [r] NumLabels [const ajint*] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_TextRulerStr(float Start, float End, ajint GapSize,
				 float TextCoef, const AjPStr PosTicks,
				 ajint NumGroups, const ajint *NumLabels)
{
    ajint i;
    ajint  j;
    const AjPStr token;
    AjPStr string;
    float charsize;
    float minsize = 100.0;
    float stringLength;
    float stringHeight;

    string = ajStrNew();

    ajStrFromInt(&string, (ajint)Start);
    stringLength = ajGraphicsCalcTextlengthS(string);
    stringHeight = ajGraphicsCalcTextheight();
    charsize = ajGraphicsCalcCharsize(0, 0, stringLength/TextCoef,
                                      stringLength/TextCoef,
                                      ajStrGetPtr(string),
                                      stringHeight/TextCoef );
    if( charsize < minsize ) minsize = charsize;

    for(i=GapSize; i<End; i+=GapSize)
	if(i>Start)
	{
	    ajStrFromInt(&string, i);
	    stringLength = ajGraphicsCalcTextlengthS(string);
	    stringHeight = ajGraphicsCalcTextheight();
	    charsize = ajGraphicsCalcCharsize(0, 0, stringLength/TextCoef,
                                              stringLength/TextCoef,
                                              ajStrGetPtr(string),
                                              stringHeight/TextCoef);
	    if(charsize < minsize)
		minsize = charsize;
	}

    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	{
	    if( ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") )
	    {
		token = ajStrParseC(Name[i][j], ";");
		stringLength = ajGraphicsCalcTextlengthS(token);
		stringHeight = ajGraphicsCalcTextheight();
		charsize = ajGraphicsCalcCharsize(0, 0, stringLength/TextCoef,
                                                  stringLength/TextCoef,
                                                  ajStrGetPtr(token),
                                                  stringHeight/TextCoef );
		if(charsize < minsize)
		    minsize = charsize;
	    }
	}

    ajStrDel(&string);

    return minsize;
}




/* @funcstatic cirdna_HeightRuler *********************************************
**
** compute the ruler's height
**
** @param [r] Start [float] Undocumented
** @param [r] End [float] Undocumented
** @param [r] GapSize [ajint] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] TickHeight [float] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] NumGroups [ajint] Undocumented
** @param [r] NumLabels [const ajint*] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_HeightRuler(float Start, float End, ajint GapSize,
				float postext, float TickHeight,
				const AjPStr PosTicks, ajint NumGroups,
				const ajint *NumLabels)
{
    ajint i;
    ajint j;
    float RulerHeight;
    float stringLength;
    float maxLength = 0.0;
    const AjPStr token;
    AjPStr string;

    string = ajStrNew();

    RulerHeight = TickHeight+postext;

    ajStrFromInt(&string, (ajint)Start);
    stringLength = ajGraphicsCalcTextlengthS(string);
    if(stringLength>maxLength)
	maxLength = stringLength;

    for(i=GapSize; i<End; i+=GapSize)
	if(i>Start)
	{
	    ajStrFromInt(&string, i);
	    stringLength = ajGraphicsCalcTextlengthS(string);
	    if(stringLength>maxLength)
		maxLength = stringLength;
	}

    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	    if(ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") )
	    {
		token = ajStrParseC(Name[i][j], ";");
		stringLength = ajGraphicsCalcTextlengthS(token);
		if( stringLength>maxLength )
		    maxLength = stringLength;
	    }

    RulerHeight += maxLength;

    ajStrDel(&string);

    return RulerHeight;
}




/* @funcstatic cirdna_DrawRuler ***********************************************
**
** draw a ruler
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] Start [float] Undocumented
** @param [r] End [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] TickHeight [float] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @param [r] GapSize [ajint] Undocumented
** @param [r] TickLines [AjBool] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] NumGroups [ajint] Undocumented
** @param [r] NumLabels [const ajint*] Undocumented
** @param [r] From [float* const *] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] Colour [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_DrawRuler(float xDraw, float yDraw, float Start, float End,
			     float RealLength, float Radius, float TickHeight,
			     float OriginAngle, ajint GapSize,
			     AjBool TickLines, float postext,
			     ajint NumGroups, const ajint *NumLabels,
			     float* const * From,
			     const AjPStr PosTicks, ajint Colour)
{
    ajint i;
    AjPStr string;
    AjPStr posticks;
    float *xy;
    float Angle;

    string   = ajStrNew();
    posticks = ajStrNew();

    ajGraphicsSetFgcolour(Colour);

    ajGraphicsDrawposCircle( xDraw, yDraw, Radius );

    ajStrAssignEmptyC(&posticks, "Out");

    /* set the circle's origin */
    ajStrFromInt(&string, (ajint)Start);
    if(TickLines)
    {
	Angle = cirdna_ComputeAngle(RealLength, 0, OriginAngle);
	xy = ajGraphicsCalcCoord(xDraw, yDraw, Radius, Angle);
	ajGraphicsDrawposLine(xDraw, yDraw, xy[0], xy[1]);
	AJFREE(xy);
    }

    if(!cirdna_OverlapTickRuler(NumGroups, NumLabels, From, PosTicks,
				(ajint)Start) )
	cirdna_DrawTicks(xDraw, yDraw, RealLength, Radius, TickHeight, 0,
			 string, OriginAngle, postext,
			 posticks, 1, 0, Colour);

    /* draw the ruler's ticks */
    for(i=GapSize; i<End; i+=GapSize)
	if(i>Start)
	{
	    ajStrFromInt(&string, i);
	    if( TickLines )
	    {
		Angle = cirdna_ComputeAngle(RealLength, i-Start, OriginAngle);
		xy = ajGraphicsCalcCoord(xDraw, yDraw, Radius, Angle);
		ajGraphicsDrawposLine(xDraw, yDraw, xy[0], xy[1]);
		AJFREE(xy);
	    }
	    if(!cirdna_OverlapTickRuler(NumGroups, NumLabels, From, PosTicks,
					i))
		cirdna_DrawTicks(xDraw, yDraw, RealLength, Radius, TickHeight,
				 i-Start, string, OriginAngle,
				 postext, posticks, 1, 0, Colour);
	}

    ajStrDel(&string);
    ajStrDel(&posticks);

    return;
}




/* @funcstatic cirdna_DrawTicks ***********************************************
**
** draw a Tick
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] TickHeight [float] Undocumented
** @param [r] From [float] Undocumented
** @param [r] Name2 [const AjPStr] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] NumNames [ajint] Undocumented
** @param [r] Adjust [ajint] Undocumented
** @param [r] Colour [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_DrawTicks(float xDraw, float yDraw, float RealLength,
			     float Radius, float TickHeight, float From,
			     const AjPStr Name2, float OriginAngle,
			     float postext,
			     const AjPStr PosTicks,
			     ajint NumNames, ajint Adjust,
			     ajint Colour)
{
    float Angle;
    float StartAngle;
    float EndAngle;
    float *xy1;
    float *xy2;
    float *xy3;
    float stringLength;
    float r1Ticks = Radius;
    float r2Ticks = r1Ticks+TickHeight;
    const AjPStr token;
    float mmtolen;

    /* radius is 2pi*radius in mm, RealLength in bases */
    mmtolen = RealLength/(Radius * (float) 2.0 * (float) 3.1416);

    ajGraphicsSetFgcolour(Colour);

    Angle = cirdna_ComputeAngle(RealLength, From, OriginAngle);

    xy1 = ajGraphicsCalcCoord(xDraw, yDraw, r1Ticks, Angle);
    xy2 = ajGraphicsCalcCoord(xDraw, yDraw, r2Ticks, Angle);
    ajGraphicsDrawposLine(xy1[0], xy1[1], xy2[0], xy2[1] );
    AJFREE(xy1);
    AJFREE(xy2);

    if(ajStrMatchCaseC(PosTicks, "In") )
    {
	stringLength = mmtolen * cirdna_HorTextPileLengthMax(Name2, NumNames);
	StartAngle = cirdna_ComputeAngle(RealLength, From+stringLength/2,
					 OriginAngle);
	EndAngle = cirdna_ComputeAngle(RealLength, From-stringLength/2,
				       OriginAngle);
	cirdna_HorTextPile(xDraw, yDraw, r2Ticks+(Adjust*postext), StartAngle,
			   EndAngle, Name2, postext, 1);
    }
    else
    {
	token = ajStrParseC(Name2, ";");
	/*ajStrExchangeCC(&Name2, ";", " ");*/
	stringLength = ajGraphicsCalcTextlengthS(token);
	xy1 = ajGraphicsCalcCoord(xDraw, yDraw, r2Ticks+postext, Angle);
	xy2 = ajGraphicsCalcCoord(xDraw, yDraw, r2Ticks+postext+stringLength,
			     Angle);
	xy3 = ajGraphicsCalcCoord(xDraw, yDraw, r2Ticks+postext-stringLength,
			     Angle);
	if((Angle>=0.0 && Angle<=90.0) || (Angle>=270.0 && Angle<=360.0) ||
	   (Angle>=360.0 && Angle<=450.0) || (Angle>=630.0 && Angle<=720.0) )
	    ajGraphicsDrawposTextAtlineJustify( xy1[0], xy1[1], xy2[0], xy2[1],
                                                ajStrGetPtr(token), 0.0 );
	else
	    ajGraphicsDrawposTextAtlineJustify( xy1[0], xy1[1], xy3[0], xy3[1],
                                                ajStrGetPtr(token), 1.0 );
	AJFREE(xy1);
	AJFREE(xy2);
	AJFREE(xy3);
    }

    return;
}




/* @funcstatic cirdna_DrawBlocks **********************************************
**
** draw a Block
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] BlockHeight [ajint] Undocumented
** @param [r] From [float] Undocumented
** @param [r] To [float] Undocumented
** @param [r] Name2 [const AjPStr] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @param [r] PosBlocks [const AjPStr] Undocumented
** @param [r] NumNames [ajint] Undocumented
** @param [r] Adjust [ajint] Undocumented
** @param [r] Colour [ajint] Undocumented
** @param [r] BlockType [const AjPStr] Undocumented
** @@
******************************************************************************/

static void cirdna_DrawBlocks(float xDraw, float yDraw, float RealLength,
			      float Radius, ajint BlockHeight, float From,
			      float To, const AjPStr Name2, float postext,
			      float OriginAngle, const AjPStr PosBlocks,
			      ajint NumNames, ajint Adjust, ajint Colour,
			      const AjPStr BlockType)
{
    float StartAngle;
    float EndAngle;
    float stringLength;
    float stringHeight;
    float r1Blocks;
    float r2Blocks;

    r1Blocks = Radius+((float)1.0*BlockHeight/(float)2);
    r2Blocks = r1Blocks-BlockHeight;

    StartAngle = cirdna_ComputeAngle(RealLength, From, OriginAngle);
    EndAngle = cirdna_ComputeAngle(RealLength, To, OriginAngle);

    ajGraphicsSetFgcolour(Colour);
    if(ajCharCmpCase(ajStrGetPtr(BlockType), "Open")==0 )
	ajGraphicsDrawarcRect(xDraw, yDraw, r2Blocks,
                              StartAngle, EndAngle, (float)BlockHeight);
    else if( ajCharCmpCase(ajStrGetPtr(BlockType), "Filled")==0 )
	ajGraphicsDrawarcRectFill(xDraw, yDraw, r2Blocks,
                                  StartAngle, EndAngle, (float)BlockHeight);
    else
    {
	ajGraphicsDrawarcRectFill(xDraw, yDraw, r2Blocks,
                                  StartAngle, EndAngle, (float)BlockHeight);
	ajGraphicsSetFgcolour(0);
	ajGraphicsDrawarcRect(xDraw, yDraw, r2Blocks,
                              StartAngle, EndAngle, (float)BlockHeight);
	ajGraphicsSetFgcolour(Colour);
    }

    stringLength = cirdna_HorTextPileLengthMax(Name2, NumNames);
    stringHeight = ajGraphicsCalcTextheight();
    StartAngle   = cirdna_ComputeAngle(RealLength, (To+From)/2+stringLength/2,
				       OriginAngle);
    EndAngle = cirdna_ComputeAngle(RealLength, (To+From)/2-stringLength/2,
				   OriginAngle);

    if(ajStrMatchCaseC(PosBlocks, "Out") )
	cirdna_HorTextPile(xDraw, yDraw, r1Blocks+(Adjust*postext), StartAngle,
			   EndAngle, Name2, postext, 1);
    else
	cirdna_HorTextPile(xDraw, yDraw,
			   (r1Blocks+r2Blocks)/2-(stringHeight/2)-postext,
			   StartAngle, EndAngle, Name2, postext, 1);

    return;
}




/* @funcstatic cirdna_DrawRanges **********************************************
**
** draw a Range
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] RangeHeight [float] Undocumented
** @param [r] From [float] Undocumented
** @param [r] To [float] Undocumented
** @param [r] FromSymbol [char] Undocumented
** @param [r] ToSymbol [char] Undocumented
** @param [r] Name2 [const AjPStr] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @param [r] NumNames [ajint] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] Adjust [ajint] Undocumented
** @param [r] Colour [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_DrawRanges(float xDraw, float yDraw, float RealLength,
			      float Radius, float RangeHeight, float From,
			      float To, char FromSymbol, char ToSymbol,
			      const AjPStr Name2, float OriginAngle,
			      ajint NumNames, float postext, ajint Adjust,
			      ajint Colour)
{
    float StartAngle;
    float EndAngle;
    float stringLength;
    float rRanges = Radius;
    float rupper;
    float BoundaryLength;
    float mmtolen;

    /* radius is 2pi*radius in mm, RealLength in bases */
    mmtolen = RealLength/(Radius * (float) 2.0 * (float) 3.1416);

    rupper = rRanges+((float)1.0*RangeHeight/(float)2);

    ajGraphicsSetFgcolour(Colour);

    StartAngle = cirdna_ComputeAngle(RealLength, From, OriginAngle);
    EndAngle   = cirdna_ComputeAngle(RealLength, To, OriginAngle);
    ajGraphicsDrawarcArc(xDraw, yDraw, rRanges, StartAngle, EndAngle);

    if( RangeHeight>(From-To)/3 )
	BoundaryLength = (From-To)/3;
    else
	BoundaryLength = RangeHeight;

    if(FromSymbol=='<')
	cirdna_DrawArrowHeadsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				     BoundaryLength, rRanges, StartAngle,
				     +1);
    if(FromSymbol=='>')
	cirdna_DrawArrowHeadsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				     BoundaryLength, rRanges, StartAngle,
				     -1);
    if(FromSymbol=='[')
	cirdna_DrawBracketsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				   BoundaryLength, rRanges, StartAngle,
				   +1);
    if(FromSymbol==']')
	cirdna_DrawBracketsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				   BoundaryLength, rRanges, StartAngle,
				   -1);
    if(FromSymbol=='|')
	cirdna_DrawBarsOncurve(xDraw, yDraw, RangeHeight,
				   rRanges, StartAngle);

    if(ToSymbol=='<')
	cirdna_DrawArrowHeadsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				     BoundaryLength, rRanges, EndAngle,
				     +1);
    if(ToSymbol=='>')
	cirdna_DrawArrowHeadsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				     BoundaryLength, rRanges, EndAngle,
				     -1);
    if(ToSymbol=='[')
	cirdna_DrawBracketsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				   BoundaryLength, rRanges, EndAngle,
				   +1);
    if(ToSymbol==']')
	cirdna_DrawBracketsOncurve(xDraw, yDraw, RealLength, RangeHeight,
				   BoundaryLength, rRanges, EndAngle,
				   -1);
    if(ToSymbol=='|')
	cirdna_DrawBarsOncurve(xDraw, yDraw, RangeHeight,
			       rRanges, EndAngle);

    stringLength = mmtolen * cirdna_HorTextPileLengthMax(Name2, NumNames);
    StartAngle   = cirdna_ComputeAngle(RealLength, (To+From)/2+stringLength/2,
				       OriginAngle);
    EndAngle = cirdna_ComputeAngle(RealLength, (To+From)/2-stringLength/2,
				   OriginAngle);
    cirdna_HorTextPile(xDraw, yDraw, rupper+(Adjust*postext), StartAngle,
		       EndAngle, Name2, postext, 1);

    return;
}




/* @funcstatic cirdna_InterBlocks *********************************************
**
** draw an InterBlock
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] BlockHeight [float] Undocumented
** @param [r] From [float] Undocumented
** @param [r] To [float] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @param [r] InterSymbol [AjBool] Undocumented
** @param [r] Colour [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_InterBlocks(float xDraw, float yDraw, float RealLength,
			       float Radius, float BlockHeight, float From,
			       float To, float OriginAngle,
			       AjBool InterSymbol, ajint Colour)
{
    float StartAngle;
    float  EndAngle;
    float r1Inter;
    float r2Inter;


    r1Inter = Radius+((float)1.0*BlockHeight/(float)2);
    r2Inter = r1Inter-BlockHeight;

    ajGraphicsSetFgcolour(Colour);

    StartAngle = cirdna_ComputeAngle(RealLength, To, OriginAngle);
    EndAngle   = cirdna_ComputeAngle(RealLength, From, OriginAngle);

    if(InterSymbol)
	ajGraphicsDrawarcArc(xDraw, yDraw, (r1Inter+r2Inter)/2, StartAngle,
                             EndAngle);

    return;
}




/* @funcstatic cirdna_ComputeAngle ********************************************
**
** compute the angle at the Length position
**
** @param [r] RealLength [float] Undocumented
** @param [r] Length [float] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_ComputeAngle(float RealLength, float Length,
				 float OriginAngle)
{
    float i;
    float j;

    i = Length/RealLength;
    j = i * 360 - OriginAngle;

    return 360-j;
}




/* @funcstatic cirdna_ComputeArc ********************************************
**
** compute the angle covering the length as a fraction of the real length
**
** @param [r] RealLength [float] Undocumented
** @param [r] Length [float] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_ComputeArc(float RealLength, float Length)
{
    float i;
    float j;

    i = Length/RealLength;
    j = i * 360;

    return j;
}




/* @funcstatic cirdna_DrawArrowHeadsOncurve ***********************************
**
** draw arrowheads on a curve
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Height [float] Undocumented
** @param [r] Length [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] Angle [float] Undocumented
** @param [r] Way [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_DrawArrowHeadsOncurve(float xDraw, float yDraw,
					 float RealLength, float Height,
					 float Length, float Radius,
					 float Angle, ajint Way)
{
    float *xy1;
    float *xy2;
    float pos;
    float middle;


    middle = (float)1.0*Height/(float)2;

    pos = cirdna_ComputeArc(RealLength, Length);
    if(pos < 1.0)
        pos = 1.0;

    if(Way==1)
    {
	xy1 = ajGraphicsCalcCoord(xDraw, yDraw, Radius, Angle);
	xy2 = ajGraphicsCalcCoord(xDraw, yDraw, Radius+middle, Angle+pos);
	ajGraphicsDrawposLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	AJFREE(xy2);
	xy2 = ajGraphicsCalcCoord(xDraw, yDraw, Radius-middle, Angle+pos);
	ajGraphicsDrawposLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	AJFREE(xy1);
	AJFREE(xy2);
    }

    if(Way==-1)
    {
	xy1 = ajGraphicsCalcCoord(xDraw, yDraw, Radius+middle, Angle-pos);
	xy2 = ajGraphicsCalcCoord(xDraw, yDraw, Radius, Angle);
	ajGraphicsDrawposLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	AJFREE(xy1);
	xy1 = ajGraphicsCalcCoord(xDraw, yDraw, Radius-middle, Angle-pos);
	ajGraphicsDrawposLine( xy1[0], xy1[1], xy2[0], xy2[1] );
	AJFREE(xy1);
	AJFREE(xy2);
    }

    return;
}




/* @funcstatic cirdna_DrawBracketsOncurve *************************************
**
** draw brackets on a curve
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Height [float] Undocumented
** @param [r] Length [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] Angle [float] Undocumented
** @param [r] Way [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_DrawBracketsOncurve(float xDraw, float yDraw,
				       float RealLength, float Height,
				       float Length, float Radius,
				       float Angle, ajint Way)
{
    float *xy1;
    float *xy2;
    float pos;
    float middle;

    middle = (float)1.0*Height/(float)2;

    pos = cirdna_ComputeArc(RealLength, Length);
    if(pos < 1.0)
        pos = 1.0;

    if(Way==1)
    {
	xy1 = ajGraphicsCalcCoord(xDraw, yDraw, Radius+middle, Angle);
	xy2 = ajGraphicsCalcCoord(xDraw, yDraw, Radius-middle, Angle);
	ajGraphicsDrawposLine(xy1[0], xy1[1], xy2[0], xy2[1]);
	AJFREE(xy1);
	AJFREE(xy2);

	ajGraphicsDrawarcArc(xDraw, yDraw, Radius+middle, Angle-pos, Angle);
	ajGraphicsDrawarcArc(xDraw, yDraw, Radius-middle, Angle-pos, Angle);
    }

    if(Way==-1)
    {
	xy1 = ajGraphicsCalcCoord(xDraw, yDraw, Radius+middle, Angle);
	xy2 = ajGraphicsCalcCoord(xDraw, yDraw, Radius-middle, Angle);
	ajGraphicsDrawposLine(xy1[0], xy1[1], xy2[0], xy2[1]);
	AJFREE(xy1);
	AJFREE(xy2);
	ajGraphicsDrawarcArc(xDraw, yDraw, Radius+middle, Angle, Angle+pos);
	ajGraphicsDrawarcArc(xDraw, yDraw, Radius-middle, Angle, Angle+pos);
    }

    return;
}




/* @funcstatic cirdna_DrawBarsOncurve *****************************************
**
** draw bars on a curve
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] Height [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] Angle [float] Undocumented
** @return [void]
** @@
******************************************************************************/

static void cirdna_DrawBarsOncurve(float xDraw, float yDraw,
				   float Height, float Radius,
				   float Angle)
{
    float *xy1;
    float *xy2;
    float middle;
    
    middle = (float)1.0*Height/(float)2;

    xy1 = ajGraphicsCalcCoord(xDraw, yDraw, Radius+middle, Angle);
    xy2 = ajGraphicsCalcCoord(xDraw, yDraw, Radius-middle, Angle);
    ajGraphicsDrawposLine( xy1[0], xy1[1], xy2[0], xy2[1] );
    AJFREE(xy1);
    AJFREE(xy2);

    return;
}




/* @funcstatic cirdna_HorTextPile *********************************************
**
** write a pile of horizontal text strings
**
** @param [r] x [float] Undocumented
** @param [r] y [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] StartAngle [float] Undocumented
** @param [r] EndAngle [float] Undocumented
** @param [r] Name2 [const AjPStr] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] NumNames [ajint] Undocumented
** @@
******************************************************************************/

static void cirdna_HorTextPile(float x, float y, float Radius,
			       float StartAngle, float EndAngle,
			       const AjPStr Name2, float postext,
			       ajint NumNames)
{
    float rupper;
    float stringHeight;
    float totalHeight;
    const AjPStr token;
    ajint i;

    totalHeight = Radius+postext;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrParseC(Name2, ";");
	else
	    token = ajStrParseC(NULL, ";");
	stringHeight = ajGraphicsCalcTextheight();
	rupper = totalHeight+stringHeight;
	if(token && ajStrGetPtr(token))
	    ajGraphicsDrawarcTextJustify(x, y,
                                         (totalHeight+rupper)/2, StartAngle,
                                         EndAngle, ajStrGetPtr(token), 0.5);
	totalHeight+=(stringHeight+postext);
    }

    return;
}




/* @funcstatic cirdna_HorTextPileHeight ***************************************
**
** compute the height of a pile of horizontal text strings
**
** @param [r] postext [float] Undocumented
** @param [r] NumNames [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_HorTextPileHeight(float postext, ajint NumNames)
{
    float stringHeight;
    float totalHeight;
    ajint i;

    totalHeight = 0.0;
    for(i=0; i<NumNames; i++)
    {
	stringHeight = ajGraphicsCalcTextheight();
	totalHeight += (stringHeight+postext);
    }

    return totalHeight;
}




/* @funcstatic cirdna_HorTextPileLengthMax ************************************
**
** compute the maximum length of a pile of horizontal text strings
** (this is the length of the longest string)
**
** @param [r] Name2 [const AjPStr] Undocumented
** @param [r] NumNames [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float cirdna_HorTextPileLengthMax(const AjPStr Name2, ajint NumNames)
{
    float stringLength;
    float maxLength;
    ajint i;
    const AjPStr token;

    maxLength = 0.0;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrParseC(Name2, ";");
	else
	    token = ajStrParseC(NULL, ";");
	stringLength = ajGraphicsCalcTextlengthS(token);

	if(stringLength>maxLength)
	    maxLength = stringLength;
    }

    return maxLength;
}




/* @funcstatic cirdna_ReadInput ***********************************************
**
** read the beginning of the input file
**
** @param [u] infile [AjPFile] Undocumented
** @param [w] Start [float*] Undocumented
** @param [w] End [float*] Undocumented
** @return [AjBool] True if input file is valid
** @@
******************************************************************************/

static AjBool cirdna_ReadInput(AjPFile infile,
                               float *Start, float *End)
{
    AjPStr line;

    AjBool foundstart = ajFalse;
    AjBool foundend   = ajFalse;
    
    line = ajStrNew();
    while(ajReadlineTrim(infile, &line))
    {
	/* read the start and end positions */
	if(ajStrPrefixC(line, "Start"))
        {
            foundstart = ajTrue;
	    if(sscanf(ajStrGetPtr(line), "%*s%f", Start) != 1)
                return ajFalse;
        }

	if(ajStrPrefixC(line, "End"))
        {
            foundend = ajTrue;
	    if(sscanf(ajStrGetPtr(line), "%*s%f", End) != 1)
            {
                ajStrDel(&line);
                return ajFalse;
            }
        }
    }

    ajStrDel(&line);

    if(!foundstart || !foundend)
    {
        ajWarn("Missing Start and/or End line(s) in input file");
        return ajFalse;
    }
    
    
    return ajTrue;;
}




/* @funcstatic cirdna_ReadGroup ***********************************************
**
** read a group
**
** @param [u] infile [AjPFile] Undocumented
** @param [r] maxlabels [ajint] Undocumented
** @param [w] From [float*] From position array
** @param [w] To [float*] To position array
** @param [w] Name2 [AjPStr*] Array of sequence names
** @param [w] FromSymbol [char*] Undocumented
** @param [w] ToSymbol [char*] Undocumented
** @param [w] Style2 [AjPStr *] Undocumented
** @param [w] NumLabels [ajint*] Undocumented
** @param [w] NumNames [ajint*] Array of ajints
** @param [w] Colour [ajint*] Array of colour codes
** @return [AjPStr] Undocumented
******************************************************************************/


static AjPStr cirdna_ReadGroup(AjPFile infile, ajint maxlabels,
			       float* From, float* To,
			       AjPStr *Name2, char *FromSymbol,
			       char *ToSymbol, AjPStr *Style2,
			       ajint *NumLabels, ajint *NumNames,
			       ajint *Colour)
{
    ajint i;
    ajint j;
    AjPStr GroupName;
    AjPStr line;
    const AjPStr token;
    char *style;
    ajlong pos;

    line      = ajStrNew();
    GroupName = ajStrNew();

    style = (char *)AJALLOC( 10*sizeof(char) );

    /* read the group's name */
    pos = ajFileResetPos(infile);
    while(ajReadlineTrim(infile, &GroupName))
    {
	token = ajStrParseC(GroupName, " \n\t\r\f");
	if(ajStrGetLen(token)!=0)
	{
	    if(ajStrMatchCaseC(GroupName, "label") ||
	       ajStrMatchCaseC(GroupName, "endgroup"))
		ajStrAssignC(&GroupName, " ");

	    if(ajStrGetLen(GroupName)>20)
		ajStrCutRange( &GroupName, 20, ajStrGetLen(GroupName)-1 );
	    break;
	}
    }

    i = 0;
    ajFileSeek(infile, pos, 0);
    while(ajReadlineTrim(infile, &line))
    {
	token = ajStrParseC(line, " \n\t\r\f");
	if(ajStrGetLen(token)!=0)
	{
	    if(ajStrPrefixC(line, "endgroup"))
		break;
	    else
	    {
		/* read the group's label(s) */
		if(ajStrPrefixC(line, "label"))
		{
		    while(ajReadlineTrim(infile, &line))
		    {
			if (i == maxlabels)
			    ajWarn("Too many labels (maxlabels=%d) in input",
				   maxlabels);
			token = ajStrParseC(line, " \n\t\r\f");
			if(ajStrGetLen(token)!=0)
			{
			    if (i < maxlabels)
			    {
				FromSymbol[i] = '<';
				ToSymbol[i] = '>';
				sscanf( ajStrGetPtr(line), "%s", style );
				if(ajCharMatchCaseC(style, "Tick"))
				    sscanf( ajStrGetPtr(line), "%*s %f %d %*c",
					   &From[i], &Colour[i] );
				if(ajCharMatchCaseC(style, "Block"))
				    sscanf( ajStrGetPtr(line), "%*s %f %f %d %*c",
					   &To[i], &From[i], &Colour[i] );
				if(ajCharMatchCaseC(style, "Range"))
				    sscanf( ajStrGetPtr(line),
					   "%*s %f %f %c %c %d %*c",
					   &To[i], &From[i],
					   &FromSymbol[i], &ToSymbol[i],
					   &Colour[i] );
				ajStrAssignC(&Style2[i], style);
			    }
			    break;
			}
		    }

		    j = 0;

		    /* read the label's name(s) */
		    while(ajReadlineTrim(infile, &line))
		    {
			token = ajStrParseC(line, " \n\t\r\f");
			if(ajStrGetLen(token)!=0)
			{
			    if(ajStrPrefixC(line, "endlabel"))
				break;
			    else
			    {
				if (i < maxlabels)
				{
				    ajStrAppendS(&Name2[i], line);
				    ajStrAppendC(&Name2[i], ";");
				    j++;
				}
			    }
			}
		    }
		    if (i < maxlabels)
			NumNames[i] = j;
		    i++;
		}
	    }
	}
    }
    if (i < maxlabels)
	*NumLabels = i;
    else
	*NumLabels = maxlabels;

    AJFREE(style);
    ajStrDel(&line);

    return GroupName;
}




/* @funcstatic cirdna_TextGroup ***********************************************
**
**  compute the character size that fits all elements of a group
**  provided that the height and the length of all strings are at most
**  TextHeight and TextLength, respectively
**
** @param [r] TextHeight [float] Undocumented
** @param [r] TextLength [float] Undocumented
** @param [r] Name2 [AjPStr const *] Undocumented
** @param [r] NumLabels [ajint] Undocumented
** @param [r] NumNames [const ajint*] Undocumented
** @param [r] Style2 [AjPStr const *] Undocumented
** @param [r] From [const float*] Undocumented
** @param [r] To [const float*] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @return [float] Undocumented
******************************************************************************/
static float cirdna_TextGroup(float TextHeight, float TextLength,
			      AjPStr const *Name2, ajint NumLabels,
			      const ajint *NumNames,
			      AjPStr const *Style2,
			      const float* From,
			      const float* To,
			      const AjPStr PosTicks)
{
    ajint i;
    ajint j;
    float charsize;
    float  minsize = 100.0;
    const AjPStr token;

    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumNames[i]; j++)
	{
	    if(!(ajStrMatchCaseC(Style2[i], "Tick") &&
		  ajStrMatchCaseC(PosTicks, "Out")))
	    {
		if(j==0)
		    token = ajStrParseC(Name2[i], ";");
		else
		    token = ajStrParseC(NULL, ";");
		if(ajStrMatchCaseC(Style2[i], "Block") &&
		   ((From[i]-To[i])<TextLength))
		    charsize = ajGraphicsCalcCharsize(0, 0, From[i]-To[i],
                                                      From[i]-To[i],
                                                      ajStrGetPtr(token),
                                                      TextHeight);
		else
		    charsize = ajGraphicsCalcCharsize(0, 0, TextLength,
                                                      TextLength,
                                                      ajStrGetPtr(token),
                                                      TextHeight);
		if(charsize < minsize)
		    minsize = charsize;
	    }
	}
    }

    return minsize;
}




/* @funcstatic cirdna_TextGroupStr ********************************************
**
**  compute the character size that fits all elements of a group provided that
**  the height and the length of all strings are multiplied by TextCoef
**
** @param [r] Name2 [AjPStr const *] Undocumented
** @param [r] NumLabels [ajint] Undocumented
** @param [r] NumNames [const ajint*] Undocumented
** @param [r] TextCoef [float] Undocumented
** @param [r] Style2 [AjPStr const *] Undocumented
** @param [r] From [const float*] Undocumented
** @param [r] To [const float*] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @return [float] Undocumented
******************************************************************************/
static float cirdna_TextGroupStr(AjPStr const *Name2, ajint NumLabels,
				 const ajint *NumNames,
				 float TextCoef, AjPStr const *Style2,
				 const float* From, const float* To,
				 const AjPStr PosTicks)
{
    ajint i;
    ajint j;
    float charsize;
    float minsize = 100.0;
    float stringLength;
    float stringHeight;
    const AjPStr token;

    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumNames[i]; j++)
	{
	    if(!(ajStrMatchCaseC(Style2[i], "Tick") &&
		  ajStrMatchCaseC(PosTicks, "Out")))
	    {
		if(j==0)
		    token = ajStrParseC(Name2[i], ";");
		else
		    token = ajStrParseC(NULL, ";");
		stringLength = ajGraphicsCalcTextlengthS(token);
		stringHeight = ajGraphicsCalcTextheight();
		if(ajStrMatchCaseC(Style2[i], "Block") &&
		   ((From[i]-To[i])<stringLength))
		    charsize = ajGraphicsCalcCharsize(0, 0,
                                                      (From[i]-To[i])/TextCoef,
                                                      (From[i]-To[i])/TextCoef,
                                                      ajStrGetPtr(token),
                                                      stringHeight/TextCoef);
		else
		    charsize = ajGraphicsCalcCharsize(0, 0,
                                                      stringLength/TextCoef,
                                                      stringLength/TextCoef,
                                                      ajStrGetPtr(token),
                                                      stringHeight/TextCoef);
		if(charsize < minsize)
		    minsize = charsize;
	    }
	}
    }

    return minsize;
}




/* @funcstatic cirdna_HeightGroup *********************************************
**
** compute the height of a group depending on what's in it
**
** @param [r] postext [float] Undocumented
** @param [r] TickHeight [float] Undocumented
** @param [r] BlockHeight [float] Undocumented
** @param [r] RangeHeight [float] Undocumented
** @param [r] Style2 [AjPStr const *] Undocumented
** @param [r] NumLabels [ajint] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] PosBlocks [const AjPStr] Undocumented
** @param [r] Adjust [ajint] Undocumented
** @return [float] Undocumented
******************************************************************************/

static float cirdna_HeightGroup(float postext,
				float TickHeight, float BlockHeight,
				float RangeHeight,
				AjPStr const *Style2, ajint NumLabels,
				const AjPStr PosTicks,
				const AjPStr PosBlocks, ajint Adjust)
{
    ajint i;
    float GroupHeight;
    float uheight;
    float umaxheight = 0.0;
    float lheight;
    float lmaxheight = 0.0;


    for(i=0; i<NumLabels; i++)
    {
	if(ajStrMatchCaseC(Style2[i], "Tick") &&
	   ajStrMatchCaseC(PosTicks, "In"))
	{
	    uheight = TickHeight;
	    lheight = 0.0;
	    uheight+=cirdna_HorTextPileHeight(postext, 1);

	    if(uheight > umaxheight)
		umaxheight = uheight;

	    if(lheight > lmaxheight)
		lmaxheight = lheight;
	}

	if(ajStrMatchCaseC(Style2[i], "Block"))
	{
	    uheight = (float)1.0*BlockHeight/(float)2;
	    lheight = (float)1.0*BlockHeight/(float)2;

	    if(ajStrMatchCaseC(PosBlocks, "Out"))
		uheight+=cirdna_HorTextPileHeight(postext, 1);

	    if(uheight > umaxheight)
		umaxheight = uheight;

	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}

	if(ajStrMatchCaseC(Style2[i], "Range"))
	{
	    uheight = (float)1.0*RangeHeight/2;
	    lheight = (float)1.0*RangeHeight/2;
	    uheight+=cirdna_HorTextPileHeight(postext, 1);

	    if(uheight > umaxheight)
		umaxheight = uheight;

	    if(lheight > lmaxheight)
		lmaxheight = lheight;
	}
    }

    GroupHeight = umaxheight+lmaxheight+(Adjust*postext);

    return GroupHeight;
}




/* @funcstatic cirdna_OverlapTextGroup ****************************************
**
** find whether horizontal text strings overlap within a group
**
** @param [r] Name2 [AjPStr const *] Undocumented
** @param [r] Style2 [AjPStr const *] Undocumented
** @param [r] NumLabels [ajint] Undocumented
** @param [r] From [const float*] Undocumented
** @param [r] To [const float*] Undocumented
** @param [r] Start [float] Undocumented
** @param [r] End [float] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [w] Adjust [ajint*] Array elements written
** @return [ajint] Undocumented
******************************************************************************/

static ajint cirdna_OverlapTextGroup(AjPStr const *Name2, AjPStr const *Style2,
				     ajint NumLabels,
				     const float* From, const float* To,
				     float Start, float End,
				     const AjPStr PosTicks, ajint *Adjust)
{
    ajint i;
    ajint j;
    ajint AdjustMax;
    const AjPStr token;
    static ajint maxnumlabels=0;
    float llim;
    float ulim;
    float stringLength;

    if (NumLabels > maxnumlabels)
    {
	maxnumlabels = NumLabels;
	AJCRESIZE(cirdnaFromText, maxnumlabels);
	AJCRESIZE(cirdnaToText, maxnumlabels);
    }
    /* compute the length of the horizontal strings */
    for(i=0; i<NumLabels; i++)
    {
	if(ajStrMatchCaseC(Style2[i], "Tick") &&
	   ajStrMatchCaseC(PosTicks, "In"))
	{
	    token = ajStrParseC(Name2[i], ";");
	    stringLength = ajGraphicsCalcTextlengthS(token);
	    ulim = From[i]+stringLength/2;
	    if(ulim>(End-Start-1))
		ulim = End-Start-1-ulim;
	    llim = From[i]-stringLength/2;
	    if((ulim<0.0) || (llim<0.0))
	    {
		cirdnaFromText[i] = llim;
		cirdnaToText[i] = ulim;
	    }
	    else
	    {
		cirdnaFromText[i] = ulim;
		cirdnaToText[i] = llim;
	    }
	}
	else if(ajStrMatchCaseC(Style2[i], "Block"))
	{
	    token = ajStrParseC(Name2[i], ";");
	    stringLength = ajGraphicsCalcTextlengthS(token);
	    ulim = (To[i]+From[i])/2+stringLength/2;
	    if(ulim>(End-Start-1))
		ulim = End-Start-1-ulim;
	    llim = (To[i]+From[i])/2-stringLength/2;
	    if((ulim<0.0) || (llim<0.0))
	    {
		cirdnaFromText[i] = llim;
		cirdnaToText[i] = ulim;
	    }
	    else
	    {
		cirdnaFromText[i] = ulim;
		cirdnaToText[i] = llim;
	    }
	}
	else if(ajStrMatchCaseC(Style2[i], "Range"))
	{
	    token = ajStrParseC(Name2[i], ";");
	    stringLength = ajGraphicsCalcTextlengthS(token);
	    ulim = (To[i]+From[i])/2+stringLength/2;
	    if(ulim>(End-Start-1))
		ulim = End-Start-1-ulim;
	    llim = (To[i]+From[i])/2-stringLength/2;
	    if((ulim<0.0) || (llim<0.0))
	    {
		cirdnaFromText[i] = llim;
		cirdnaToText[i] = ulim;
	    }
	    else
	    {
		cirdnaFromText[i] = ulim;
		cirdnaToText[i] = llim;
	    }
	}
	else
	    cirdnaFromText[i] =  cirdnaToText[i] = 0.0;
    }

    /*
    **  if some strings overlap, the position of the overlapping strings
    **  is moved upwards by Adjust
    */
    for(i=0; i<NumLabels; i++)
	Adjust[i] = 0;

    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumLabels; j++)
	{
	    if((i!=j) && (Adjust[i]==Adjust[j]))
	    {
		if(j>i)
		{
		    if(cirdnaFromText[i]<0.0)
		    {
			ulim = End-Start-1+cirdnaFromText[i];
			if((cirdnaToText[j]<=ulim) &&
			   (cirdnaFromText[j]>=ulim))
			    Adjust[j] = Adjust[i]+1;
		    }
		    if((cirdnaToText[j]<=cirdnaFromText[i]) &&
		       (cirdnaFromText[j]>=cirdnaFromText[i]))
			Adjust[j] = Adjust[i]+1;
		}

		if(i>j)
		{
		    if(cirdnaFromText[j]<0.0)
		    {
			ulim = End-Start-1+cirdnaFromText[j];
			if((cirdnaToText[i]<=ulim) &&
			   (cirdnaFromText[i]>=ulim))
			    Adjust[i] = Adjust[j]+1;
		    }

		    if((cirdnaToText[i]<=cirdnaFromText[j]) &&
		       (cirdnaFromText[i]>=cirdnaFromText[j]))
			Adjust[i] = Adjust[j]+1;
		}
	    }
	}
    }

    AdjustMax = 0;
    for(i=0; i<NumLabels; i++)
	if(Adjust[i]>AdjustMax)
	    AdjustMax = Adjust[i];

    return AdjustMax;
}




/* @funcstatic cirdna_OverlapTickRuler ****************************************
**
** find whether group ticks and ruler's ticks overlap
**
** @param [r] NumGroups [ajint] Undocumented
** @param [r] NumLabels [const ajint*] Undocumented
** @param [r] From [float* const *] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] RulerTick [ajint] Undocumented
** @return [AjBool] Undocumented
******************************************************************************/

static AjBool cirdna_OverlapTickRuler(ajint NumGroups, const ajint *NumLabels,
				      float* const * From,
				      const AjPStr PosTicks,
				      ajint RulerTick)
{
    ajint i;
    ajint j;
    ajint overlap = 0;

    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	{
	    if(ajStrMatchCaseC(Style[i][j], "Tick") &&
	       ajStrMatchCaseC(PosTicks, "Out") &&
	       From[i][j]==RulerTick)
	    {
		overlap = 1;
		break;
	    }
	}

    if(overlap==0)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic cirdna_DrawGroup ***********************************************
**
** draw a group
**
** @param [r] xDraw [float] Undocumented
** @param [r] yDraw [float] Undocumented
** @param [r] posblock [float] Undocumented
** @param [r] posrange [float] Undocumented
** @param [r] postext [float] Undocumented
** @param [r] TickHeight [float] Undocumented
** @param [r] BlockHeight [float] Undocumented
** @param [r] RangeHeight [float] Undocumented
** @param [r] RealLength [float] Undocumented
** @param [r] Radius [float] Undocumented
** @param [r] RadiusMax [float] Undocumented
** @param [r] From [const float*] Undocumented
** @param [r] To [const float*] Undocumented
** @param [r] Name2 [AjPStr const *] Undocumented
** @param [r] FromSymbol [const char*] Undocumented
** @param [r] ToSymbol [const char*] Undocumented
** @param [r] Style2 [AjPStr const *] Undocumented
** @param [r] InterSymbol [AjBool] Undocumented
** @param [r] InterTicks [AjBool] Undocumented
** @param [r] NumLabels [ajint] Undocumented
** @param [r] OriginAngle [float] Undocumented
** @param [r] NumNames [const ajint*] Undocumented
** @param [r] PosTicks [const AjPStr] Undocumented
** @param [r] PosBlocks [const AjPStr] Undocumented
** @param [r] Adjust [const ajint*] Undocumented
** @param [r] InterColour [ajint] Undocumented
** @param [r] Colour [const ajint*] Undocumented
** @param [r] BlockType [const AjPStr] Undocumented
** @return [void]
******************************************************************************/

static void cirdna_DrawGroup(float xDraw, float yDraw, float posblock,
			     float posrange, float postext, float TickHeight,
			     float BlockHeight, float RangeHeight,
			     float RealLength, float Radius, float RadiusMax,
			     const float* From, const float* To,
			     AjPStr const *Name2,
			     const char *FromSymbol, const char *ToSymbol, 
			     AjPStr const *Style2,
			     AjBool InterSymbol, AjBool InterTicks,
			     ajint NumLabels,
			     float OriginAngle, const ajint *NumNames,
			     const AjPStr PosTicks, const AjPStr PosBlocks,
			     const ajint *Adjust,
			     ajint InterColour, const ajint *Colour,
			     const AjPStr BlockType)
{
    ajint i;
    ajint j;
    ajint  NumBlocks;

    if (NumLabels > cirdnaMaxinter)
    {
	cirdnaMaxinter = NumLabels;
	AJCRESIZE(cirdnaInter, cirdnaMaxinter);
    }

    /* draw all labels */
    for(j=0, i=0; i<NumLabels; i++)
    {
	if(ajStrMatchCaseC(Style2[i], "Tick"))
	{
	    if(ajStrMatchCaseC(PosTicks, "In"))
	    {
		cirdna_DrawTicks(xDraw, yDraw, RealLength, Radius, TickHeight,
				 From[i], Name2[i], OriginAngle,
				 postext, PosTicks, NumNames[i],
				 Adjust[i], Colour[i]);
		if(InterTicks)
		    ajGraphicsDrawposCircle(xDraw, yDraw, Radius);
	    }
	    else
		cirdna_DrawTicks(xDraw, yDraw, RealLength, RadiusMax,
				 TickHeight, From[i], Name2[i], OriginAngle,
				 postext, PosTicks,
				 NumNames[i], Adjust[i], Colour[i]);
	}

	if( ajStrMatchCaseC(Style2[i], "Block") )
	{
	    cirdna_DrawBlocks(xDraw, yDraw, RealLength, Radius-posblock,
			      (ajint)BlockHeight, From[i], To[i],
			      Name2[i], postext,
			      OriginAngle, PosBlocks, NumNames[i], Adjust[i],
			      Colour[i], BlockType);
	    cirdnaInter[j++] = i;
	}

	if(ajStrMatchCaseC(Style2[i], "Range"))
	    cirdna_DrawRanges(xDraw, yDraw, RealLength, Radius-posrange,
			      RangeHeight, From[i], To[i], FromSymbol[i],
			      ToSymbol[i], Name2[i], OriginAngle, NumNames[i],
			      postext, Adjust[i], Colour[i]);
    }
    NumBlocks = j;

    /* draw all interblocks */
    for(i=0; i<NumBlocks-1; i++)
	cirdna_InterBlocks(xDraw, yDraw, RealLength, Radius-posblock,
			   BlockHeight,
			   From[cirdnaInter[i]], To[cirdnaInter[i+1]],
			   OriginAngle, InterSymbol, InterColour);

    return;
}
