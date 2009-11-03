#ifdef __cplusplus
extern "C"
{
#endif

/* @source ajgraph.h
**
** General Plot/Printing routines.
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


#ifndef ajgraph_h
#define ajgraph_h

#include "plplot.h"
#include "ajgraphstruct.h"
#include "ajdefine.h"
#include "ajstr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ajGraphDrawLine ajGraphLine
#define ajGraphDrawTextOnLine ajGraphTextLine

#define GRAPH_XGAP      60
#define GRAPH_YGAP      60
#define GRAPH_TITLE_GAP 60

#define AJGRAPH_X_BOTTOM   0x0001  /* print xaxis bottom line */
#define AJGRAPH_Y_LEFT     0x0002  /* print yaxis left line */
#define AJGRAPH_X_TOP      0x0004  /* print xaxis top line */
#define AJGRAPH_Y_RIGHT    0x0008  /* printf y axis on the right*/
#define AJGRAPH_X_TICK     0x0010  /* add tick marks for x axis (bottom) */
#define AJGRAPH_Y_TICK     0x0020  /* add tick marks dor y axis (left) */
#define AJGRAPH_X_LABEL    0x0040  /* add x axis label */
#define AJGRAPH_Y_LABEL    0x0080  /* add y axis label */
#define AJGRAPH_TITLE      0x0100  /* add title */
#define AJGRAPH_JOINPOINTS 0x0200  /* join the point data by a line a line */
#define AJGRAPH_OVERLAP    0x0400  /* write plots on top of each other */
#define AJGRAPH_Y_NUMLABEL_LEFT   0x0800
#define AJGRAPH_Y_INVERT_TICK     0x1000
#define AJGRAPH_Y_GRID            0x2000
#define AJGRAPH_X_NUMLABEL_ABOVE  0x4000
#define AJGRAPH_X_INVERT_TICK     0x8000
#define AJGRAPH_X_GRID            0x10000
#define AJGRAPH_CIRCLEPOINTS      0x20000
#define AJGRAPH_SUBTITLE          0x40000
#define AJGRAPH_GAPS              0x80000

#define GRAPH_XY (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_X_TOP + \
		  AJGRAPH_Y_RIGHT + AJGRAPH_X_TICK + AJGRAPH_Y_TICK + \
		  AJGRAPH_Y_LABEL + AJGRAPH_JOINPOINTS + AJGRAPH_X_LABEL + \
		  AJGRAPH_TITLE + AJGRAPH_SUBTITLE + AJGRAPH_OVERLAP)

#define GRAPH_XY_MAIN (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT + \
		       AJGRAPH_X_TOP + AJGRAPH_Y_TICK + AJGRAPH_X_LABEL + \
		       AJGRAPH_Y_LABEL + AJGRAPH_JOINPOINTS + AJGRAPH_TITLE + \
		       AJGRAPH_SUBTITLE + AJGRAPH_OVERLAP)



/*
** Prototype definitions
*/

void          ajGraphBox (PLFLT xx0, PLFLT yy0,PLFLT size);
void          ajGraphBoxFill (PLFLT xx0, PLFLT yy0, PLFLT size);
ajint         ajGraphCheckColour (const AjPStr colour);
void          ajGraphCircle (PLFLT xcentre, PLFLT ycentre, float radius);
void          ajGraphClose (void);
void          ajGraphCloseWin (void);
void          ajGraphColourBack (void);
void          ajGraphColourFore (void);
void          ajGraphPlpDataAddLine (AjPGraphPlpData graphs,
				     float xx1, float yy1,
				     float xx2, float yy2, ajint colour);
void          ajGraphPlpDataAddRect (AjPGraphPlpData graphs,
				     float xx1, float yy1,
				     float xx2, float yy2,
				     ajint colour, ajint fill);
void          ajGraphPlpDataAddText (AjPGraphPlpData graphs,
				     float xx1, float yy1,
				     ajint colour, const char *text);
void          ajGraphPlpDataAddTextScale (AjPGraphPlpData graphs,
					  float xx1, float yy1,
					  ajint colour, float scale,
					  const char *text);
void          ajGraphPlpDataSetMaxMin (AjPGraphPlpData graphdata, float xmin,
				      float xmax, float ymin, float ymax);
void          ajGraphArrayMaxMin(const float *array, ajint npoints, float *min,
				  float *max);

void          ajGraphPlpDataSetMaxima(AjPGraphPlpData graphdata, float xmin,
				     float xmax, float ymin, float ymax);
void          ajGraphPlpDataSetTypeC(AjPGraphPlpData graphdata,
				     const char *type);

void          ajGraphPlpDataTrace (const AjPGraphPlpData thys);
void          ajGraphClear(AjPGraph thys);
void          ajGraphDia (PLFLT xx0, PLFLT yy0, PLFLT size);
void          ajGraphDiaFill (PLFLT xx0, PLFLT yy0, PLFLT size);
void          ajGraphDots (PLFLT *xx1,PLFLT *yy1, ajint numofdots);
void          ajGraphDumpDevices (void);
ajint*        ajGraphGetBaseColour (void);
ajint*        ajGraphGetBaseColourNuc(const AjPStr codes);
ajint*        ajGraphGetBaseColourProt(const AjPStr codes);
void          ajGraphGetCharSize (float *defheight, float *currentheight);
ajint         ajGraphGetColour(void);
void          ajGraphGetOut (float *xp,float *yp, ajint *xleng,
			     ajint *yleng, ajint *xoff, ajint *yoff);
const AjPStr  ajGraphGetSubTitle(const AjPGraph thys);
const char*   ajGraphGetSubTitleC(const AjPGraph thys);
const AjPStr  ajGraphGetTitle(const AjPGraph thys);
const char*   ajGraphGetTitleC(const AjPGraph thys);
const AjPStr  ajGraphGetXTitle(const AjPGraph thys);
const char*   ajGraphGetXTitleC(const AjPGraph thys);
const AjPStr  ajGraphGetYTitle(const AjPGraph thys);
const char*   ajGraphGetYTitleC(const AjPGraph thys);

void          ajGraphHoriBars (ajint numofpoints, PLFLT *y,
				   PLFLT *xmin, PLFLT *xmax);
ajint         ajGraphInfo(AjPList* files);
void          ajGraphInit (const char *pgm, ajint argc, char * const argv[]);
void          ajGraphInitP (const char *pgm, ajint argc,char * const argv[],
			    const char *package);
void          ajGraphInitSeq (AjPGraph thys, const AjPSeq seq);
AjBool        ajGraphIsData(const AjPGraph thys);
void          ajGraphLabel (const char *x, const char *y,
			    const char *title, const char *subtitle);
void          ajGraphLabelYRight (const char *text);
void          ajGraphLine (PLFLT xx1,PLFLT yy1,PLFLT xx2,PLFLT yy2);
void          ajGraphLines (PLFLT *xx1,PLFLT *yy1,PLFLT *xx2,PLFLT *yy2,
			       ajint numoflines);
AjPGraph      ajGraphNew (void);
void          ajGraphNewPage (AjPGraph thys, AjBool resetdefaults);
void          ajGraphAddLine (AjPGraph thys, float xx1, float yy1,
			      float xx2, float yy2, ajint colour);
void          ajGraphAddRect (AjPGraph thys, float xx1, float yy1,
			      float xx2, float yy2, ajint colour,
			      ajint fill);
void          ajGraphAddText (AjPGraph thys, float xx1, float yy1,
				 ajint colour, const char *text);
void          ajGraphAddTextScale (AjPGraph thys, float xx1, float yy1,
				   ajint colour, float scale,
				   const char *text);
void          ajGraphPlpDataDel(AjPGraphPlpData *thys);
void          ajGraphOpen (AjPGraph thys, PLFLT xmin, PLFLT xmax,
			   PLFLT ymin, PLFLT ymax, ajint flags);
void          ajGraphOpenPlot (AjPGraph thys, ajint numofsets);
void          ajGraphOpenWin  (AjPGraph thys, float xmin, float xmax,
			       float ymin, float ymax);
void          ajGraphPlenv (float xmin, float xmax, float ymin, float ymax,
			    ajint flags);
void          ajGraphPoly (ajint n, PLFLT *x, PLFLT *y);
void          ajGraphPolyFill (ajint n, PLFLT *x, PLFLT *y);
void          ajGraphPrintType(AjPFile outf, AjBool full);
void          ajGraphRect (PLFLT xx0, PLFLT yy0,PLFLT xx1, PLFLT yy1) ;
void          ajGraphRectFill (PLFLT xx0, PLFLT yy0, PLFLT xx1, PLFLT yy1) ;
AjBool        ajGraphSet (AjPGraph thys, const AjPStr type);
void          ajGraphSetBackBlack (void);
void          ajGraphSetBackWhite (void);
float         ajGraphSetCharScale (float scale);
float         ajGraphSetCharSize(float size);
void          ajGraphSetDesc(AjPGraph thys, const AjPStr title);
void          ajGraphSetDevice(const AjPGraph thys);
void          ajGraphSetDir(AjPGraph thys, const AjPStr txt);
ajint         ajGraphSetFillPat (ajint style);
ajint         ajGraphSetFore (ajint colour);
ajint         ajGraphSetLineStyle (ajint style);
void          ajGraphSetMulti (AjPGraph thys, ajint numsets);
void          ajGraphSetName (const AjPGraph thys);
void          ajGraphSetOri(ajint ori);
void          ajGraphSetPenWidth(float width);
void          ajGraphSymbols( ajint numofdots, PLFLT *xx1,PLFLT *yy1,
			      ajint symbol);
void          ajGraphText (PLFLT xx1, PLFLT yy1, const char *text, PLFLT just);
void          ajGraphTextEnd (PLFLT xx1, PLFLT yy1, const char *text);
void          ajGraphTextMid (PLFLT xx1, PLFLT yy1, const char *text);
void          ajGraphTextLine (PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
			       const char *text, PLFLT just);
void          ajGraphTextStart (PLFLT xx1, PLFLT yy1, const char *text);
void          ajGraphTrace (const AjPGraph thys);
void          ajGraphTri (PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
			  PLFLT x3, PLFLT y3);
void          ajGraphTriFill (PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
			      PLFLT x3, PLFLT y3);
void          ajGraphUnused(void);
void          ajGraphVertBars (ajint numofpoints, PLFLT *x,
			       PLFLT *ymin, PLFLT *ymax);
void          ajGraphPlpDataCalcXY (AjPGraphPlpData graphdata,
				    ajint numofpoints,
				    float start, float incr,
				    const float* y);
void          ajGraphPlpDataSetXY (AjPGraphPlpData graphdata,
				   const float *x, const float *y);
ajint         ajGraphDataAdd (AjPGraph thys, AjPGraphPlpData graphdata);
ajint         ajGraphDataReplace (AjPGraph thys, AjPGraphPlpData graphdata);
ajint         ajGraphDataReplaceI (AjPGraph thys, AjPGraphPlpData graphdata,
				   ajint num);
AjPGraphPlpData  ajGraphPlpDataNew (void);
AjPGraphPlpData  ajGraphPlpDataNewI (ajint numsets);
void          ajGraphPlpDataSetSubTitle  (AjPGraphPlpData graphdata,
					  const AjPStr title);
void          ajGraphPlpDataSetSubTitleC (AjPGraphPlpData graphdata,
					  const char *title);
void          ajGraphPlpDataSetTitle  (AjPGraphPlpData graphdata,
				       const AjPStr title);
void          ajGraphPlpDataSetTitleC (AjPGraphPlpData graphdata,
				       const char *title);
void          ajGraphSetTitlePlus(AjPGraph thys, const AjPStr title);
void          ajGraphPlpDataSetXTitle  (AjPGraphPlpData graphdata,
				       const AjPStr title);
void          ajGraphPlpDataSetXTitleC (AjPGraphPlpData graphdata,
				       const char *title);
void          ajGraphPlpDataSetYTitle  (AjPGraphPlpData graphdata,
				       const AjPStr title);
void          ajGraphPlpDataSetYTitleC (AjPGraphPlpData graphdata,
				       const char *title);
void          ajGraphxyDel (AjPGraph* pmult);
void          ajGraphxyDisplay (AjPGraph thys, AjBool closeit );
AjPGraph      ajGraphxyNewI (ajint numofpoints);
void          ajGraphxyCheckMaxMin (AjPGraph thys);
AjBool        ajGraphxySet (AjPGraph thys, const AjPStr type);
void          ajGraphxySetCirclePoints (AjPGraph thys, AjBool set);
void          ajGraphPlpDataSetColour (AjPGraphPlpData graphdata,
				       ajint colour);
void          ajGraphSetFlag (AjPGraph thys, ajint flag, AjBool istrue);
void          ajGraphxySetJoinPoints (AjPGraph thys, AjBool set);
void          ajGraphPlpDataSetLineType (AjPGraphPlpData graphdata,
					 ajint type);
void          ajGraphxySetMaxMin(AjPGraph thys,float xmin,float xmax,
				 float ymin,float ymax);
void          ajGraphSetOut (AjPGraph thys, const AjPStr txt);
void          ajGraphSetOutC (AjPGraph thys, const char *txt);
void          ajGraphxySetOverLap (AjPGraph thys, AjBool overlap);
void          ajGraphSetPage(ajuint width, ajuint height);
void          ajGraphSetSubTitleDo (AjPGraph thys, AjBool set);
void          ajGraphSetTitleDo (AjPGraph thys, AjBool set);
void          ajGraphxySetXBottom (AjPGraph thys, AjBool set);
void          ajGraphxySetXLabelTop (AjPGraph thys, AjBool set);
void          ajGraphxySetXTick (AjPGraph thys, AjBool set);
void          ajGraphxySetXTop (AjPGraph thys, AjBool set);
void          ajGraphxySetXEnd (AjPGraph thys, float val);
void          ajGraphxySetXGrid (AjPGraph thys, AjBool set);
void          ajGraphxySetXInvTicks (AjPGraph thys, AjBool set);
void          ajGraphxySetXLabel (AjPGraph thys, AjBool set);
void          ajGraphxySetXRangeII (AjPGraph thys, ajint start, ajint end);
void          ajGraphxySetXStart (AjPGraph thys, float val);
void          ajGraphxySetYLabelLeft (AjPGraph thys, AjBool set);
void          ajGraphxySetYLeft (AjPGraph thys, AjBool set);
void          ajGraphxySetYRight (AjPGraph thys, AjBool set);
void          ajGraphxySetYTick (AjPGraph thys, AjBool set);
void          ajGraphxySetYEnd (AjPGraph thys, float val);
void          ajGraphxySetYGrid (AjPGraph thys, AjBool set);
void          ajGraphxySetYInvTicks (AjPGraph thys, AjBool set);
void          ajGraphxySetYLabel (AjPGraph thys, AjBool set);
void          ajGraphxySetYRangeII (AjPGraph thys, ajint start, ajint end);
void          ajGraphxySetYStart (AjPGraph thys, float val);
void          ajGraphSetSubTitle  (AjPGraph thys, const AjPStr title);
void          ajGraphSetSubTitleC (AjPGraph thys, const char *title);
void          ajGraphSetTitle     (AjPGraph thys, const AjPStr title);
void          ajGraphSetTitleC    (AjPGraph thys, const char *title);
void          ajGraphSetXTitle  (AjPGraph thys, const AjPStr title);
void          ajGraphSetXTitleC (AjPGraph thys, const char *title);
void          ajGraphSetYTitle  (AjPGraph thys, const AjPStr title);
void          ajGraphSetYTitleC (AjPGraph thys, const char *title);
void          ajGraphxySetGaps(AjPGraph thys, AjBool overlap);
void          ajGraphPartCircle(PLFLT xcentre, PLFLT ycentre, PLFLT Radius,
				PLFLT StartAngle, PLFLT EndAngle);
PLFLT         *ajComputeCoord(PLFLT xcentre, PLFLT ycentre, PLFLT Radius,
			      PLFLT Angle);
void          ajGraphDrawTextOnCurve(PLFLT xcentre, PLFLT ycentre,
				     PLFLT Radius, PLFLT StartAngle,
				     PLFLT EndAngle,
				     const char *Text, PLFLT just);
void          ajGraphRectangleOnCurve(PLFLT xcentre, PLFLT ycentre,
				      PLFLT Radius, PLFLT BoxHeight,
				      PLFLT StartAngle, PLFLT EndAngle);
PLFLT         ajGraphTextLength(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
				const char *text);
PLFLT         ajGraphTextHeight(PLFLT xx1, PLFLT xx2, PLFLT yy1, PLFLT yy2);
PLFLT         ajGraphDistPts(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2);
PLFLT          ajGraphFitTextOnLine(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
				    const char *text, PLFLT TextHeight);
float         ajGraphSetDefCharSize (float size);

void          ajGraphFillRectangleOnCurve(PLFLT xcentre, PLFLT ycentre,
					  PLFLT Radius, PLFLT BoxHeight,
					  PLFLT StartAngle, PLFLT EndAngle);

/*
** End of prototype definitions
*/


/*
**  These prototypes do not have any corresponding .c code so cannot
**  go into the block above, otherwise the VC compilation will fail 
*/
void          ajGraphTraceInt (FILE* outf);

/* End of prototypes without C code */



#endif /* ajgraph_h */



#ifdef __cplusplus
}
#endif
