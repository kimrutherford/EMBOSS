#ifdef __cplusplus
extern "C"
{
#endif

/* @source ajhist.h
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

/* temporary include file for graphics to define structure(s) */

#ifndef ajhist_h
#define ajhist_h

#include "ajgraph.h"
#include "ajdefine.h"
#include "ajstr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HIST_ONTOP      0
#define HIST_SIDEBYSIDE 1
#define HIST_SEPARATE   2

#define GRAPH_HIST (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT + \
AJGRAPH_Y_INVERT_TICK + AJGRAPH_X_INVERT_TICK + AJGRAPH_Y_TICK + \
AJGRAPH_X_TICK + AJGRAPH_X_LABEL + AJGRAPH_Y_LABEL + AJGRAPH_TITLE )

#ifndef WIN32
extern ajint aj_hist_mark;
#else
#ifdef AJAXDLL_EXPORTS
__declspec(dllexport) ajint aj_hist_mark;
#else
__declspec(dllimport) ajint aj_hist_mark;
#endif
#endif




/* @data AjPHistData **********************************************************
**
** AJAX data structure for histogram graph data
**
** @attr data [float*] y coordinates
** @attr deletedata [AjBool] if true, delete data
** @attr colour [ajint] colour code
** @attr title [AjPStr] graph title
** @attr xaxis [AjPStr] x axis title
** @attr yaxis [AjPStr] y axis title
** @attr pattern [ajint] pattern code
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSHistData {
  float *data;
  AjBool deletedata;
  ajint colour;
  AjPStr title;
  AjPStr xaxis;
  AjPStr yaxis;
  ajint pattern;
  char Padding[4];
} AjOHistData;
#define AjPHistData AjOHistData*

/* @data AjPHist **************************************************************
**
** AJAX data structure for histogram graph objects
**
**
** @attr numofsets [ajint] number of current sets
** @attr numofsetsmax [ajint] maximum number of sets
** @attr numofdatapoints [ajint] numbr of data points
** @attr xmin [float] Lowest x value
** @attr xmax [float] Highest x value
** @attr displaytype [ajint] Display type index
** @attr bins [ajint] Number of histogram bins
** @attr BaW [AjBool] Black and white rendering if ajTrue
** @attr title [AjPStr] Plot title
** @attr xaxis [AjPStr] x axis title
** @attr yaxisleft [AjPStr] y axis title
** @attr yaxisright [AjPStr] second y axis title for right side
** @attr graph [AjPGraph] Graph
** @attr hists [AjPHistData*] Histogram bin data
** @@
******************************************************************************/

typedef struct AjSHist {
  ajint numofsets;
  ajint numofsetsmax;
  ajint numofdatapoints;
  float xmin;
  float xmax;
  ajint displaytype;
  ajint bins;
  AjBool BaW;
  AjPStr title;
  AjPStr xaxis;
  AjPStr yaxisleft;
  AjPStr yaxisright;
  AjPGraph graph;
  AjPHistData *hists;
} AjOHist;
#define AjPHist AjOHist*




/*
** Prototype definitions
*/

void    ajHistClose (void);
void    ajHistCopyData (AjPHist hist, ajint index, const PLFLT *data);
void    ajHistDelete (AjPHist* hist);
void    ajHistDisplay (const AjPHist hist);
AjPHist ajHistNew (ajint numofsets, ajint numofpoints);
AjPHist ajHistNewG (ajint numofsets, ajint numofpoints, AjPGraph graph);
void    ajHistSetBlackandWhite (AjPHist hist, AjBool set);
void    ajHistSetColour(AjPHist hist, ajint index, ajint colour);
void    ajHistSetMultiTitle  (AjPHist hist, ajint index, const AjPStr title);
void    ajHistSetMultiTitleC (AjPHist hist, ajint index, const char *title);
void    ajHistSetMultiXTitle  (AjPHist hist, ajint index, const AjPStr title);
void    ajHistSetMultiXTitleC (AjPHist hist, ajint index, const char *title);
void    ajHistSetMultiYTitle  (AjPHist hist, ajint index, const AjPStr title);
void    ajHistSetMultiYTitleC (AjPHist hist, ajint index, const char *title);
void    ajHistSetPattern (AjPHist hist, ajint index, ajint style);
void    ajHistSetPtrToData (AjPHist hist, ajint index, PLFLT *data);
void    ajHistSetTitleC (AjPHist hist, const char* strng);
void    ajHistSetXAxisC (AjPHist hist, const char* strng);
void    ajHistSetYAxisLeftC (AjPHist hist,const  char* strng);
void    ajHistSetYAxisRightC (AjPHist hist, const char* strng);

/*
** End of prototype definitions
*/

#endif /* ajhist_h */

#ifdef __cplusplus
}
#endif
