/******************************************************************************
** @source AJAX GRAPH (ajax graphics) functions
** @author Ian Longden
** These functions control all aspects of AJAX graphics.
**
** @version 1.0
** @modified 1988-11-12 pmr First version
** @modified 1999 ajb ANSI
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#define GRAPHMAIN 1

#include "ajax.h"
#include "ajgraph.h"

#include <math.h>
#include <limits.h>
#include <float.h>
#define AZ 28


#include "plplotP.h"


static void     GraphArray(ajuint numofpoints,
			   float *x, float *y);
static void     GraphArrayGaps(ajuint numofpoints,
			       float *x, float *y);
static void     GraphArrayGapsI(ajuint numofpoints, ajint *x, ajint *y);
static void     GraphCharScale(float size);
static void     GraphCheckFlags(ajint flags);
static void     GraphCheckPoints(ajint n, const PLFLT *x, const PLFLT *y);
static void     GraphClose(void);
static void     GraphColourBack(void);
static void     GraphColourFore(void);
static void     GraphDatafileNext(void);
static void     GraphDataDraw(const AjPGraphdata graphdata);
static void     GraphDataPrint(const AjPGraphdata graphdata);
static void     GraphDefCharSize(float size);
static float    GraphDistPts(float xx1, float yy1, float xx2, float yy2);
static void     GraphDrawsetLines( ajuint numoflines,PLFLT *xx1, PLFLT *yy1);
static void     GraphFill(ajuint numofpoints, float *x, float *y);
static void     GraphFillPat(ajint pat);
static void     GraphInit(AjPGraph thys);
static void     GraphLabelTitle(const char *title, const char *subtitle);
static void     GraphLineStyle(ajint style);
static void     GraphNewPlplot(AjPGraph graph);
static void     GraphDraw(const AjPGraph thys);
static void     GraphPrint(const AjPGraph thys);
static void     GraphOpenData(AjPGraph thys, const char *ext);
static void     GraphOpenFile(AjPGraph thys, const char *ext);
static void     GraphOpenSimple(AjPGraph thys, const char *ext);
#ifndef X_DISPLAY_MISSING
static void     GraphOpenXwin(AjPGraph thys, const char *ext);
#endif
static void     GraphPen(ajint pen, ajint red, ajint green, ajint blue);
static void     GraphSetName(const AjPGraph thys,
			     const AjPStr txt, const char *ext);
static void     GraphSetNumSubPage(ajuint numofsubpages);
static void     GraphSetPen(ajint colour);
static void     GraphSetWin(float xmin, float xmax, float ymin, float ymax);
static void     GraphSetWin2(float xmin, float xmax, float ymin, float ymax);
static void     GraphSubPage(ajint page);
static void     GraphSymbols(float *xx1, float *yy1, ajuint numofdots,
			     ajint symbol);
static void     GraphText(float xx1, float yy1, float xx2, float yy2,
			  float just, const char *text);
static void     GraphWind(float xmin, float xmax, float ymin, float ymax);
static void     GraphxyDisplayToDas(AjPGraph thys, AjBool closeit,
				     const char *ext);
static void     GraphxyDisplayToData(AjPGraph thys, AjBool closeit,
				     const char *ext);
static void     GraphxyDisplayToFile(AjPGraph thys, AjBool closeit,
				     const char *ext);
static void     GraphxyDisplayXwin(AjPGraph thys, AjBool closeit,
				   const char *ext);
static void     GraphxyGeneral(AjPGraph thys, AjBool closeit);
static void     GraphdataInit(AjPGraphdata graphdata);
static void     GraphxyNewPlplot(AjPGraph thys);
static void     GraphDrawarcText(PLFLT xcentre, PLFLT ycentre,
                                 PLFLT Radius, PLFLT Angle, PLFLT pos,
                                 const char *Text, PLFLT just);

enum AjEGraphObjectTypes { RECTANGLE, RECTANGLEFILL, TEXT, LINE};

static const char *graphColourName[] =
{
    "BLACK", "RED", "YELLOW", "GREEN",
    "AQUAMARINE", "PINK", "WHEAT",
    "GREY", "BROWN", "BLUE", "BLUEVIOLET",
    "CYAN", "TURQUOISE", "MAGENTA",
    "SALMON", "WHITE"
};

struct graphSData
{
    AjPFile  File;
    AjPStr   FName;
    AjPStr   Ext;
    ajint    Num;
    ajint    Lines;
    float    xmin;
    float    ymin;
    float    xmax;
    float    ymax;
    AjPList  List;
} *graphData=NULL;




/* @filesection ajgraph *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datastatic GraphPType *****************************************************
**
** Graph type definition structure
**
** @alias GraphSType
** @alias GraphOType
**
** @attr Name [const char*] Name used by Ajax
** @attr Device [const char*] Name used by plplot library
** @attr Ext [const char*] File extension for output file if any
** @attr Width [ajint] Default width within the driver, 0 if not a plot
** @attr Height [ajint] Default height within the driver, 0 if not a plot
** @attr Plplot [AjBool] true if using PlPlot data and library calls
** @attr Alias [AjBool] ajTrue if this name is a user alias for another
**                      entry with the same Device name.
** @attr Screen [AjBool] true if plplot writes to the screen
** @attr Onefile [AjBool] true if plplot can write multiple pages to one file
**                        (for example, postscript output)
** @attr Fixedsize [AjBool] true if plplot fixes the plot window size,
**                          ignoring user settings
** @attr Interactive [AjBool] true if plplot can interact with the device
** @attr XYDisplay [(void*)] Function to display an XY graph
** @attr GOpen [(void*)] Function to display a general graph
** @attr Desc [const char*] Description
** @@
******************************************************************************/

typedef struct GraphSType
{
    const char* Name;
    const char* Device;
    const char* Ext;
    ajint Width;
    ajint Height;
    AjBool Plplot;
    AjBool Alias;
    AjBool Screen;
    AjBool Onefile;
    AjBool Fixedsize;
    AjBool Interactive;
    void (*XYDisplay) (AjPGraph thys, AjBool closeit, const char *ext);
    void (*GOpen) (AjPGraph thys, const char *ext);
    const char* Desc;
} GraphOType;
#define GraphPType GraphOType*




/* @funclist graphType ********************************************************
**
** Graph functions to open and display a graph with the PLPLOT
** device drivers
**
******************************************************************************/

static GraphOType graphType[] =
{
    /*Name         Device     Extension Width Height
     PlPlot   Alias    Screen   Onefile  Fixedsize Interactive
     DisplayFunction       OpenFunction
     Description*/
    {"ps",         "ps",      ".ps",   720,  540,
     AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Postscript"},
    {"postscript", "ps",      ".ps",    720,  540,
     AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Postscript"},

#ifndef WIN32
    {"hpgl",       "lj_hpgl", ".hpgl",  11000,7200,
     AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "HPGL general"},

    {"hp7470",     "hp7470",  ".hpgl",  10300,7650,
     AJTRUE,  AJFALSE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "HPGL 7470 pen plotter"},

    {"hp7580",     "hp7580",  ".hpgl",  9000, 5580,
     AJTRUE,  AJFALSE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "HPGL 7580 pen plotter"},
#endif

    {"meta",       "plmeta",  ".meta",  0,    0,
     AJTRUE,  AJFALSE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Metafile"},

    {"cps",        "psc",     ".ps",    720,  540,
     AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Colour postscript"},

    {"colourps",   "psc",     ".ps",    720,  540,
     AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Colour postscript"},

#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is  available */
    {"x11",        "xwin",    "null",   1440, 900,
     AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenXwin,
     "X11 in new window"},

    {"xwindows",   "xwin",    "null",   1440, 900,
     AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenXwin,
     "X11 in new window"},
#endif

    {"tek",        "tek4107t","null",   1024, 780,
     AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "Tektronix screen graphics"},

    {"tek4107t",   "tek4107t","null",   1024, 780,
     AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "Tektronix model 4107 screen graphics"},

    {"tekt",       "tekt",    "null",   1024, 780,
     AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "Tektronix screen graphics"},

    {"tektronics", "tekt",    "null",   1024, 780,
     AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "Tektronix screen graphics"},

#endif

    {"none",       "null",    "null",  0,    0,
     AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "No output"},

    {"null",       "null",    "null",  0,    0,
     AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "No output"},

    {"text",       "null",    "null",  0,    0,
     AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayXwin,   GraphOpenSimple,
     "Text file"},
/* data file output */
  
    {"data",       "data",    ".dat",  960,  768,
     AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
     GraphxyDisplayToData, GraphOpenData,
     "Data file for Staden package"},

    {"das",        "das",     ".das",  0,    0,
     AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,
     GraphxyDisplayToDas,  NULL,
     "Data file for DAS server"},

#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is available */
    {"xterm",      "xterm",   "null",   1024, 780,
     AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,   GraphOpenXwin,
     "Xterm screen graphics"},
#endif
#endif

#ifdef PLD_png          /* if png/gd/zlib libraries available for png driver */
    {"png",        "png",     ".png",   800,  600,
     AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "PNG graphics files"},
#ifndef WIN32
    {"gif",        "gif",     ".gif",   640,  480,
     AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "GIF graphics files"},
#endif
#endif

    {"pdf",        "pdf",     ".pdf",  1683, 1190,
     AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Adobe PDF"},

    {"svg",        "svg",     ".svg",   720,  540, /* see plplot/svg.c */
     AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "Scalable vector graphics"},

#ifdef WIN32
    {"win3",        "win3",     "null", 720,  540,
     AJTRUE, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,  AJTRUE,
     GraphxyDisplayXwin,     GraphOpenXwin,
     "Windows screen graphics"},
#endif

#ifdef PLD_pdf          /* if png/gd/zlib libraries available for png driver */
    {"pdf",        "pdf",     ".pdf",   800,  600,
     AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "PDF graphics files"},
#endif

    {"svg",        "svg",     ".svg",   800,  600,
     AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE,  AJFALSE,
     GraphxyDisplayToFile, GraphOpenFile,
     "SVG graphics files"},
    
    {NULL, NULL, NULL, 0, 0,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE,
     NULL, NULL,
     NULL}
};


#ifndef NO_PLOT




static ajuint currentfgcolour = BLACK; /* Change with ajGraphicsSetFgcolour */
static AjBool currentbgwhite = AJTRUE; /* By default white background */
static PLFLT  currentcharsize   = 4.0; /* ajGraphicsSetCharSize to change */
static PLFLT  currentcharscale  = 1.0; /* ajGraphicsSetCharscale to change */

/* AJB: This variable is set quite a lot, but never used */
#if 0
static PLFLT  currentcharht  = 4.0;    /* ajGraphicsSetCharscale to change */
#endif

static PLINT  currentlinestyle = 1;    /* ajGraphicsSetLinestyle to change*/
static PLINT  currentfillpattern = 1;  /* ajGraphicsSetFillpat to change*/

static AjPStr graphBasename = NULL;
static AjPStr graphExtension = NULL;

/******************************************************************************
plplot calls should only be made once from now on so all calls to plplot
should be made via ajGraph calls. So why bother? This is to make it easier
to change to another graph package as all the plplot calls should only be
listed once hence only a few routines should need to be changed?? Well in
theory anyway.
******************************************************************************/

/******************************************************************************
PLPLOT CALLS *START*
******************************************************************************/




/* @datasection [none] Graphics internals **************************************
**
** Function is for controlling the graphics internals without using
** an AjPGraph object
**
** @nam2rule Graphics
*/




/* @section modifiers **********************************************************
**
** Controlling the internals
**
** @fdata [none]
** @fcategory modify
**
** @nam3rule Close Close a plot
** @nam4rule CloseWin Close a window based plot
** @nam3rule Plenv Defines a PLPLOT environment
** @nam3rule Reset Reset internal state
** @nam3rule Set Set internal state
**
** @nam4rule Bgcolour Background colour
** @nam4rule Charscale Character scale factor
** @nam4rule Charsize Character height
** @nam4rule Defcharsize Character default height
** @nam4rule Device Set plplot device name from graph object
** @nam4rule Fgcolour Foreground plotting colour
** @nam4rule Filename BaseName and extension
** @nam4rule Fillpat Fill pattern
** @nam4rule Labels Plot titles and axis labels
** @nam4rule Linestyle Line style
** @nam4rule Filename File name from graph object
** @nam4rule Pagesize Page dimensions
** @nam4rule Penwidth Pen width
** @nam4rule Portrait Portrait orientation
** @nam4rule Rlabel Right axis label
**
** @nam5rule Black Set colour to black
** @nam5rule White Set colour to white
**
** @suffix C Character data
** @suffix S String object data
**
** @argrule Plenv xmin [float] X axis start
** @argrule Plenv xmax [float] X axis end
** @argrule Plenv ymin [float] Y axis start
** @argrule Plenv ymax [float] Y axis end
** @argrule Plenv flags [ajint] flag bit settings
** @argrule SetFgcolour colour [ajint] Foreground plplot colour code
** @argrule Charscale scale [float] Character relative scale factor
** @argrule Charsize size [float] Character height in plot units
** @argrule Defcharsize size [float] Default character height in plot units
** @argrule Device graph [const AjPGraph] Graph object with device defined
** @argrule Filename graph [const AjPGraph] Graph object with name defined
** @argrule Fillpat patstyle [ajint] Pattern style plplot code
** @argrule Linestyle linestyle [ajint] Line style plplot code
** @argrule Pagesize plotwidth  [ajuint] Page width (x-axis) in pixels or mm
** @argrule Pagesize plotheight [ajuint] Page height (y-axis) in pixels or mm
** @argrule Penwidth penwidth [float] Pen width in plot units
** @argrule Portrait set [AjBool] True to set orientation, false for landscape
** @argrule LabelsC x [const char*] X axis label
** @argrule LabelsS strx [const AjPStr] X axis label
** @argrule LabelsC y [const char*] Y axis label
** @argrule LabelsS stry [const AjPStr] Y axis label
** @argrule LabelsC title [const char*] Plot title
** @argrule LabelsS strtitle [const AjPStr] Plot title
** @argrule LabelsC subtitle [const char*] Plot subtitle
** @argrule LabelsS strsubtitle [const AjPStr] Plot subtitle
** @argrule RlabelC txt [const char*] C character string
** @argrule RlabelS str [const AjPStr] String object
**
** @valrule * [void]
** @valrule *Charscale [float] previous character scale factor
** @valrule *Charsize [float] previous character height in plot units
** @valrule *Defcharsize [float] previous character height in plot units
** @valrule *SetFgcolour [ajint] previous foreground colour code
** @valrule *Fillpat [ajint] previous pattern fill plplot code
** @valrule *Linestyle [ajint] previous line style plplot code
**
******************************************************************************/




/* @func ajGraphicsClose *******************************************************
**
** Close current Plot.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsClose(void)
{
    ajDebug("ajGraphicsClose\n");
    GraphClose();

    return;
}




/* @obsolete ajGraphClose
** @rename ajGraphicsClose
*/
__deprecated void ajGraphClose(void)
{
    GraphClose();

    return;
}




/* @func ajGraphicsCloseWin ****************************************************
**
** Close current window Plot
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsCloseWin(void)
{
    ajDebug("ajGraphicsCloseWin\n");
    GraphClose();

    return;
}




/* @obsolete ajGraphCloseWin
** @rename ajGraphicsClose
*/
__deprecated void ajGraphCloseWin(void)
{
    GraphClose();

    return;
}




/* @func ajGraphicsPlenv *******************************************************
**
** Defines a plot environment. i.e. tells plplot whether the graph is boxed,
** whether it has tick marks, whether it has labels etc. These should already
** be set in the flags.
**
** @param [r] xmin [float] X axis start
** @param [r] xmax [float] X axis end
** @param [r] ymin [float] Y axis start
** @param [r] ymax [float] Y axis end
** @param [r] flags [ajint] flag bit settings
** @return [void]
** @@
******************************************************************************/

void ajGraphicsPlenv(float xmin, float xmax, float ymin, float ymax,
                     ajint flags)
{
    char xopt[15] = " ";
    char yopt[15] = " ";
    ajuint i = 0;
    ajuint j = 0;

    /* ignore 'a' axis at zero value */
    /* ignore 'd' label plot as date/time */
    /* ignore 'f' fixed point labels */
    /* ignore 'h' minor interval grid */
    /* ignore 'l' logarithmic axis */

    ajDebug("ajGraphicsPlenv(%.3f, %.3f, %.3f, %.3f, flags:%x)\n",
	     xmin, xmax, ymin, ymax, flags);
    GraphSubPage(0);
    GraphSetWin2(xmin,xmax,ymin,ymax);
    
    if(flags & AJGRAPH_X_BOTTOM)
	xopt[i++] = 'b';
    
    if(flags & AJGRAPH_Y_LEFT)
	yopt[j++] = 'b';
    
    if(flags & AJGRAPH_X_TOP)
	xopt[i++] = 'c';
    
    if(flags & AJGRAPH_Y_RIGHT)
	yopt[j++] = 'c';
    
    if(flags & AJGRAPH_X_TICK)
    {
	xopt[i++] = 't';		/* do ticks */
	xopt[i++] = 's';		/* do subticks */

	if(flags & AJGRAPH_X_INVERT_TICK)
	    xopt[i++] = 'i';

	if(flags & AJGRAPH_X_NUMLABEL_ABOVE)
	    xopt[i++] = 'm';
	else
	    xopt[i++] = 'n';		/* write numeric labels */
    }
    
    if(flags & AJGRAPH_Y_TICK)
    {
	yopt[j++] = 't';
	yopt[j++] = 's';

	if(flags & AJGRAPH_Y_INVERT_TICK)
	    yopt[j++] = 'i';

	if(flags & AJGRAPH_Y_NUMLABEL_LEFT)
	    yopt[j++] = 'm';
	else
	    yopt[j++] = 'n';
    }
    
    if(flags & AJGRAPH_X_GRID)
	xopt[i++] = 'g';
    
    if(flags & AJGRAPH_Y_GRID)
	yopt[j++] = 'g';

    /* ignore minor interval grid option 'h' */

    xopt[i] = '\0';
    yopt[j] = '\0';

    ajDebug("=g= plbox('%s', 0.0, 0, '%s', 0.0, 0) "
	     "[xopt, 0.0, 0, yopt, 0.0, 0]\n", xopt, yopt);
    plbox(xopt, 0.0, 0, yopt, 0.0, 0);
    
    return;
}




/* @obsolete ajGraphPlenv
** @rename ajGraphicsPlenv
*/
__deprecated void ajGraphPlenv(float xmin, float xmax, float ymin, float ymax,
		  ajint flags)
{
    ajGraphicsPlenv(xmin, xmax, ymin, ymax, flags);
    return;
}




/* @func ajGraphicsResetBgcolour ***********************************************
**
** Reset the background colour to either black or white depending on the
** current settings.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsResetBgcolour(void)
{
    GraphColourBack();
    return;
}




/* @obsolete ajGraphColourBack
** @rename ajGraphicsResetBgcolour
*/

__deprecated void ajGraphColourBack(void)
{
    GraphColourBack();
    return;
}




/* @func ajGraphicsResetFgcolour ***********************************************
**
** Reset the foreground plotting colour using current stored colour.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsResetFgcolour(void)
{
    GraphColourFore();
    return;
}




/* @obsolete ajGraphColourFore
** @rename ajGraphicsResetFgcolour
*/

__deprecated void ajGraphColourFore(void)
{
    GraphColourFore();
    return;
}




/* @func ajGraphicsSetBgcolourBlack ********************************************
**
** Set the background colour to black
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetBgcolourBlack(void)
{
    ajDebug("ajGraphicsSetBgcolourBlack currentbgwhite: %B\n", currentbgwhite);

    if(currentbgwhite)
    {
	currentbgwhite = ajFalse;

	GraphPen(0, 0, 0, 0);
	GraphPen(MAXCOL, 255, 255, 255);

	graphColourName[0] = "BLACK";
	graphColourName[MAXCOL] = "WHITE";

	ajDebug("ajGraphicsSetBgcolourBlack pen 0 '%s' pen %d '%s'\n",
		graphColourName[0], MAXCOL, graphColourName[MAXCOL]);
    }

    return;
}




/* @obsolete ajGraphSetBackBlack
** @rename ajGraphicsSetBgcolourBlack
*/

__deprecated void ajGraphSetBackBlack(void)
{
    ajGraphicsSetBgcolourBlack();
    return;
}




/* @func ajGraphicsSetBgcolourWhite ********************************************
**
** Set the background colour to white (the default colour)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetBgcolourWhite(void)
{
    ajDebug("ajGraphicsSetBgcolourWhite currentbgwhite: %B\n", currentbgwhite);

    if(!currentbgwhite)
    {
	currentbgwhite = ajTrue;

	GraphPen(0, 255, 255, 255);
	GraphPen(MAXCOL, 0, 0, 0);

	graphColourName[0] = "WHITE";
	graphColourName[MAXCOL] = "BLACK";

	ajDebug("ajGraphicsSetBgcolourWhite pen 0 '%s' pen %d '%s'\n",
		graphColourName[0], MAXCOL, graphColourName[MAXCOL]);
    }

    return;
}




/* @obsolete ajGraphSetBackWhite
** @rename ajGraphicsSetBgcolourkWhite
*/

__deprecated void ajGraphSetBackWhite(void)
{
    ajGraphicsSetBgcolourWhite();
    return;
}




/* @func ajGraphicsSetCharscale ************************************************
**
** Set the character scale factor
**
** @param [r] scale [float]  character scale.
** @return [float] the previous character scale factor.
** @@
******************************************************************************/

float ajGraphicsSetCharscale(float scale)
{
    float oldscale;

    oldscale = currentcharscale;

    if(scale)
    {
	currentcharscale = scale;
	GraphCharScale(currentcharscale);
    }

    return oldscale;
}




/* @obsolete ajGraphSetCharScale
** @rename ajGraphicsSetCharscale
*/

__deprecated float ajGraphSetCharScale(float scale)
{
    return ajGraphicsSetCharscale(scale);
}




/* @func ajGraphicsSetCharsize *************************************************
**
** Set the character size factor
**
** @param [r] size [float]  character size.
** @return [float] the previous character size factor.
** @@
******************************************************************************/

float ajGraphicsSetCharsize(float size)
{
    float oldsize;

    oldsize = currentcharsize;

    if(size)
    {
	currentcharsize = size;
#if 0
	currentcharht = size;
#endif
        currentcharscale = 1.0;
	plschr(currentcharsize, currentcharscale);
    }

    return oldsize;
}




/* @obsolete ajGraphSetCharSize
** @rename ajGraphicsSetCharsize
*/

__deprecated float ajGraphSetCharSize(float size)
{
    return ajGraphicsSetCharsize(size);
}




/* @func ajGraphicsSetDefcharsize **********************************************
**
** Set the default character size in mm.
**
** @param [r] size [float]  character size in mm.
** @return [float] the previous character size in mm.
** @@
******************************************************************************/

float ajGraphicsSetDefcharsize(float size)
{
    float oldsize;
    float oldscale;

    ajGraphicsGetCharsize(&oldsize, &oldscale);
    GraphDefCharSize((PLFLT)size);

    return oldsize;
}




/* @obsolete ajGraphSetDefCharSize
** @rename ajGraphicsSetDefcharsize
*/

__deprecated float ajGraphSetDefCharSize(float size)
{
    return ajGraphicsSetDefcharsize(size);
}




/* @func ajGraphicsSetDevice **************************************************
**
** Set internal graph device to that selected for a graph object
**
** @param [r] graph [const AjPGraph] Graph object with display type set
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetDevice(const AjPGraph graph)
{
    ajDebug("=g= plsdev ('%s') [graphType[%d].Device] ready: %B\n",
            graphType[graph->displaytype].Device,
            graph->displaytype, graph->ready);

    if(!graph->ready)
        plsdev(graphType[graph->displaytype].Device);

    return;
}




/* @obsolete ajGraphSetDevice
   @rename ajGraphicsSetDevice
*/

__deprecated void ajGraphSetDevice(const AjPGraph thys)
{
    ajGraphicsSetDevice(thys);

    return;
}




/* @func ajGraphicsSetFgcolour ************************************************
**
** Set the foreground plotting colour
**
** @param [r] colour [ajint]  colour to set drawing to.
** @return [ajint] the previous colour.
** @@
******************************************************************************/

ajint ajGraphicsSetFgcolour(ajint colour)
{
    ajint oldcolour;
    ajint col;

    col = colour;

    /* in case of looping through colours */
    /* start at the beginning once past end */
    while(col > MAXCOL)
	col -= (MAXCOL+1);

    if(col < 0)
	ajDebug("ajGraphicsSetFgcolour give up and use "
                "currentfgcolour %d '%s'\n",
		 currentfgcolour, graphColourName[currentfgcolour]);
    if(col < 0)
	return currentfgcolour;

    oldcolour = currentfgcolour;
    currentfgcolour = col;

    if(currentbgwhite)          /* swap black and white foreground */
    {
	if(col == 0)
	    col = MAXCOL;
	else if(col == MAXCOL)
	    col = 0;
    }

    ajDebug("ajGraphicsSetFgcolour (%d '%s') currentbgwhite: %B, "
	    "oldcolour: %d '%s'\n",
	    colour, graphColourName[colour], currentbgwhite,
	    oldcolour, graphColourName[oldcolour]);

    GraphColourFore();

    return oldcolour;
}




/* @obsolete ajGraphSetFore
** @rename ajGraphicsSetFgcolour
*/

__deprecated ajint ajGraphSetFore(ajint colour)
{

    return ajGraphicsSetFgcolour(colour);
}




/* @funcstatic GraphDefCharSize ***********************************************
**
** Set the default char size in mm.
**
** @param [r] size [float] Character size in mm. See PLPLOT.
** @return [void]
** @@
******************************************************************************/

static void GraphDefCharSize(float size)
{
    currentcharscale = 1.0;
    currentcharsize = size;
#if 0
    currentcharht = size;
#endif
    plschr(currentcharsize, currentcharscale);

    return;
}




/* @funcstatic GraphSetName ***************************************************
**
** Set BaseName and extension.
**
** @param [r] thys [const AjPGraph] Graph object.
** @param [r] txt [const AjPStr] base name for files
** @param [r] ext [const char*] extension for files
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetName(const AjPGraph thys,
			 const AjPStr txt, const char *ext)
{
    if(!thys->ready)
    {
        ajDebug("=g= plxsfnam ('%S', '%s')\n", txt, ext);
        plxsfnam(ajStrGetPtr(txt), ext);
        ajStrAssignS(&graphBasename, txt);
        ajStrAssignC(&graphExtension, ext);
        if(ajStrGetCharFirst(graphExtension) == '.')
            ajStrCutStart(&graphExtension, 1);
    }

    return;
}




/* @func ajGraphicsSetFilename *************************************************
**
** set BaseName and extension.
**
** @param [r] graph [const AjPGraph] Graph object.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetFilename(const AjPGraph graph)
{
    if(!ajCharMatchCaseC(graphType[graph->displaytype].Ext, "null"))
        GraphSetName(graph, graph->outputfile,
                     graphType[graph->displaytype].Ext);

    return;
}




/* @obsolete ajGraphSetName
** @rename ajGraphicsSetFilename
*/
__deprecated void ajGraphSetName(const AjPGraph thys)
{
    ajGraphicsSetFilename(thys);
    return;
}




/* @func ajGraphicsSetFillpat *************************************************
**
** Set the Fill Pattern type.
**
** @param [r] patstyle [ajint]  line style to set drawing to.
** @return [ajint] the previous line style.
** @@
******************************************************************************/

ajint ajGraphicsSetFillpat(ajint patstyle)
{
    ajint oldstyle;

    if(patstyle < 0 || patstyle > 8)
	patstyle = 1;

    oldstyle = currentfillpattern;
    currentfillpattern = patstyle;
    GraphFillPat((PLINT)patstyle);

    return oldstyle;
}




/* @obsolete ajGraphSetFillPat
** @rename ajGraphicsSetFillpat
*/
__deprecated ajint ajGraphSetFillPat(ajint style)
{
    return ajGraphicsSetFillpat(style);
}




/* @func ajGraphicsSetLabelsC **************************************************
**
** Label current Plot.
**
** @param [r] x [const char*]        text for x axis labelling.
** @param [r] y [const char*]        text for y axis labelling.
** @param [r] title [const char*]    text for title of plot.
** @param [r] subtitle [const char*] text for subtitle of plot.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetLabelsC(const char *x, const char *y,
                          const char *title, const char *subtitle)
{
    float fold;

    ajDebug("=g= plmtex('t', 2.5, 0.5, 0.5, '%s') [title]\n", title);
    plmtex("t", (PLFLT) 2.5, (PLFLT) 0.5, (PLFLT) 0.5, title);
    ajDebug("=g= plmtex('b', 3.2, 0.5, 0.5, '%s') [x-title]\n", x);
    plmtex("b", (PLFLT) 3.2, (PLFLT) 0.5, (PLFLT) 0.5, x);
    ajDebug("=g= plmtex('l', 5.0, 0.5, 0.5, '%s') [y-title]\n", y);
    plmtex("l", (PLFLT) 5.0, (PLFLT) 0.5, (PLFLT) 0.5, y);

    fold = ajGraphicsSetCharscale(0.0);
    ajGraphicsSetCharscale(fold/(float)2.0);

    ajDebug("=g= plmtex('t', 1.5, 0.5, 0.5, '%s') [subtitle]\n", subtitle);
    plmtex("t", (PLFLT) 1.5, (PLFLT) 0.5, (PLFLT) 0.5, subtitle);

    fold = ajGraphicsSetCharscale(fold);

    /*  pllab(x,y,title);*/

    return;
}




/* @func ajGraphicsSetLabelsS **************************************************
**
** Label current Plot.
**
** @param [r] strx [const AjPStr]        text for x axis labelling.
** @param [r] stry [const AjPStr]        text for y axis labelling.
** @param [r] strtitle [const AjPStr]    text for title of plot.
** @param [r] strsubtitle [const AjPStr] text for subtitle of plot.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetLabelsS(const AjPStr strx, const AjPStr stry,
                          const AjPStr strtitle, const AjPStr strsubtitle)
{
    float fold;

    ajDebug("=g= plmtex('t', 2.5, 0.5, 0.5, '%S') [title]\n", strtitle);
    plmtex("t", (PLFLT) 2.5, (PLFLT) 0.5, (PLFLT) 0.5, MAJSTRGETPTR(strtitle));
    ajDebug("=g= plmtex('b', 3.2, 0.5, 0.5, '%S') [x-title]\n", strx);
    plmtex("b", (PLFLT) 3.2, (PLFLT) 0.5, (PLFLT) 0.5,  MAJSTRGETPTR(strx));
    ajDebug("=g= plmtex('l', 5.0, 0.5, 0.5, '%S') [y-title]\n", stry);
    plmtex("l", (PLFLT) 5.0, (PLFLT) 0.5, (PLFLT) 0.5,  MAJSTRGETPTR(stry));

    fold = ajGraphicsSetCharscale(0.0);
    ajGraphicsSetCharscale(fold/(float)2.0);

    ajDebug("=g= plmtex('t', 1.5, 0.5, 0.5, '%s') [subtitle]\n",
            MAJSTRGETPTR(strsubtitle));
    plmtex("t", (PLFLT) 1.5, (PLFLT) 0.5, (PLFLT) 0.5,
           MAJSTRGETPTR(strsubtitle));

    fold = ajGraphicsSetCharscale(fold);

    /*  pllab(x,y,strtitle);*/

    return;
}




/* @obsolete ajGraphLabel
** @rename ajGraphicsSetLabelsC
*/

__deprecated void ajGraphLabel(const char *x, const char *y,
                               const char *title, const char *subtitle)
{
    ajGraphicsSetLabelsC(x, y, title, subtitle);
    return;
}




/* @func ajGraphicsSetLinestyle ***********************************************
**
** Set the line style.
**
** @param [r] linestyle [ajint]  line style to set drawing to.
** @return [ajint] the previous line style.
** @@
******************************************************************************/

ajint ajGraphicsSetLinestyle(ajint linestyle)
{
    ajint oldstyle;

    if(linestyle < 1 || linestyle > 8)
	linestyle = 1;

    oldstyle = currentlinestyle;
    currentlinestyle = linestyle;
    GraphLineStyle((PLINT)linestyle);

    return oldstyle;
}




/* @obsolete ajGraphSetLineStyle
** @rename ajGraphicsSetLinestyle
*/
__deprecated ajint ajGraphSetLineStyle(ajint style)
{
    return ajGraphicsSetLinestyle(style);
}




/* @func ajGraphicsSetPagesize ************************************************
**
** Set the width and height of the plot page.
**
** Ignored by most plplot drivers, but will be used by PNG
**
** @param [r] plotwidth [ajuint] Page width in pixels or mm
** @param [r] plotheight [ajuint] Page height in pixels or mm
** @return [void]
**
******************************************************************************/

void ajGraphicsSetPagesize(ajuint plotwidth, ajuint plotheight)
{
    plspage(0.0, 0.0, plotwidth, plotheight, 0, 0);

    return;
}




/* @obsolete ajGraphSetPage
** @rename ajGraphicsSetPagesize
*/
__deprecated void ajGraphSetPage(ajuint width, ajuint height)
{
    ajGraphicsSetPagesize(width, height);
    return;
}




/* @func ajGraphicsSetPenwidth ************************************************
**
** Set the current pen width.
**
** @param [r] penwidth [float] width for the pen.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsSetPenwidth(float penwidth)
{
    ajDebug("=g= plwid(%.2f) [width]\n", penwidth);
    plwid((PLINT)penwidth);

    return;
}




/* @obsolete ajGraphSetPenWidth
** @rename ajGraphicsSetPenwidth
*/
__deprecated void ajGraphSetPenWidth(float penwidth)
{
    ajGraphicsSetPenwidth(penwidth);
    return;
}




/* @func ajGraphicsSetPortrait *************************************************
**
** Set graph orientation
**
** @param [r] set [AjBool] portrait orientation if true, else landscape
** @return [void]
**@@
******************************************************************************/

void ajGraphicsSetPortrait(AjBool set)
{
    ajDebug("=g= plsori(%d) [ori]\n", set);

    if(set)
        plsori(1);
    else
        plsori(0);

    return;
}




/* @obsolete ajGraphSetOri
** @rename ajGraphicsSetPortrait
*/

__deprecated void ajGraphSetOri(ajint ori)
{
    ajGraphicsSetPortrait(ori);
    return;
}




/* @func ajGraphicsSetRlabelC *************************************************
**
** Label the right hand y axis.
**
** @param [r] txt [const char*] text for label of right y axis.
** @return [void]
** @@
******************************************************************************/
void ajGraphicsSetRlabelC(const char *txt)
{
    ajDebug("=g= plmtex('r', 2.0, 0.5, 0.5, '%s') [ajGraphicsDrawRlabelC]\n",
            txt);
    plmtex("r",2.0,0.5,0.5,txt);

    return;
}




/* @func ajGraphicsSetRlabelS *************************************************
**
** Label the right hand y axis.
**
** @param [r] str [const AjPStr] text for label of right y axis.
** @return [void]
** @@
******************************************************************************/
void ajGraphicsSetRlabelS(const AjPStr str)
{
    ajDebug("=g= plmtex('r', 2.0, 0.5, 0.5, '%S') [ajGraphicsDrawRlabelS]\n",
            str);
    plmtex("r",2.0,0.5,0.5,MAJSTRGETPTR(str));

    return;
}




/* @obsolete ajGraphLabelYRight
** @rename ajGraphicsSetRlabelC
*/

__deprecated void ajGraphLabelYRight(const char *text)
{
    ajGraphicsSetRlabelC(text);
    return;
}




/* @funcstatic GraphColourFore ************************************************
**
** Set the foreground plotting colour using current stored colour.
** @return [void]
** @@
******************************************************************************/

static void GraphColourFore(void)
{
    ajint colour;

    colour = currentfgcolour;

    if(currentbgwhite)	 /* OKAY!! when we swap backgrounds we also */
    {
	if(colour == 0)	/* swap pens. User does not know this so switch */
	    colour = MAXCOL;		/* for them */
	else if(colour == MAXCOL)
	    colour = 0;
    }

    GraphSetPen(colour);
    ajDebug("GraphColourFore currentbgwhite: %B '%s' "
	    "currentfgcolour: %d (%s) => %d (%s)\n",
	    currentbgwhite, graphColourName[0],
	    currentfgcolour, graphColourName[currentfgcolour],
	    colour, graphColourName[colour]);

    return;
}




/* @funcstatic GraphTextLength ************************************************
**
** Compute the length of a string in millimetres.
**
** @param [u] txt [const char*] text string
** @return [float] The length of the string in user coordinates
** @@
******************************************************************************/

static float GraphTextLength(const char *txt)
{
    return plstrl(txt);
}




/* @funcstatic GraphFillPat ***************************************************
**
** Set the pattern to fill with.
**
** @param [r] pat [ajint] Pattern code. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphFillPat(ajint pat)
{
    ajDebug("=g= plpsty(%d) [pattern style]\n", pat);
    plpsty((PLINT)pat);

    return;
}




/* @funcstatic GraphLineStyle *************************************************
**
** Set the Line style. i.e. dots dashes unbroken.
**
** @param [r] style [ajint] Line style. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphLineStyle(ajint style)
{
    ajDebug("=g= pllsty(%d) [line style]\n", style);
    pllsty((PLINT)style);

    return;
}




/* @funcstatic GraphCharScale *************************************************
**
** Set the char size.
**
** @param [r] scale [float] Character scale factor. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphCharScale(float scale)
{
    ajDebug("=g= plschr(0.0, %.2f) [0.0 charscale]\n", scale);
    plschr(0.0,(PLFLT)scale);		/* use current size */
#if 0
    if(scale)
        currentcharht = currentcharsize*scale;
#endif

    return;
}




/* @funcstatic GraphLabelTitle ************************************************
**
** Label current Plot with title and subtitle
**
** @param [r] title [const char*]    text for title of plot.
** @param [r] subtitle [const char*] text for subtitle of plot.
** @return [void]
** @@
******************************************************************************/

static void GraphLabelTitle(const char *title, const char *subtitle)
{
    float fold;
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(title);
    if(ajStrGetLen(tmpstr) > 55)
    {
	ajStrKeepRange(&tmpstr, 0, 55);
	ajStrAppendC(&tmpstr, "...");
    }

    ajDebug("=g= plmtex('t', 2.5, 0.5, 0.5, '%S') [title]\n", tmpstr);

    plvsta();                   /* set standard viewport for standard titles */

    if(ajStrGetLen(tmpstr)) {
	fold = ajGraphicsSetCharscale(1.0);
	ajDebug("GraphLabelTitle title fold: %.3f size: %.3f scale %.3f\n",
		fold, currentcharsize, currentcharscale);
	plmtex("t", (PLFLT) 4.0, (PLFLT) 0.5, (PLFLT) 0.5,
	       ajStrGetPtr(tmpstr));
	fold = ajGraphicsSetCharscale(fold);
    }

    ajStrAssignC(&tmpstr, subtitle);

    if(ajStrGetLen(tmpstr) > 95)
    {
	ajStrKeepRange(&tmpstr, 0, 95);
	ajStrAppendC(&tmpstr, "...");
    }

    ajDebug("=g= plmtex('t', 1.5, 0.5, 0.5, '%S') [subtitle]\n", tmpstr);

    if(ajStrGetLen(tmpstr))
    {
	fold = ajGraphicsSetCharscale(1.0);
	ajGraphicsSetCharscale(0.5);
	ajDebug("GraphLabelTitle subtitle fold: %.3f size: %.3f scale %.3f\n",
		fold, currentcharsize, currentcharscale);
	plmtex("t", (PLFLT) 5.0, (PLFLT) 0.5, (PLFLT) 0.5,
	       ajStrGetPtr(tmpstr));
	fold = ajGraphicsSetCharscale(fold);
    }

    if (graphData)
    {
        ajFmtPrintF(graphData->File,"##Maintitle %s\n",title);
        graphData->Lines++;

        if(subtitle)
        {
            ajFmtPrintF(graphData->File,"##Subtitle %s\n",subtitle);
            graphData->Lines++;
        }
    }


    /*  pllab(x,y,title);*/

    ajStrDel(&tmpstr);
    return;
}




/* @funcstatic GraphSetPen ****************************************************
**
** Set the pen to the colour specified.
**
** @param [r] colour [ajint] Pen colour (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetPen(ajint colour)
{
    ajDebug("=g= plcol(%d '%s') [colour]\n", colour, graphColourName[colour]);
    plcol((PLINT)colour);

    return;
}




/* @funcstatic GraphSetNumSubPage *********************************************
**
** Sets the number of sub pages for a page.
**
** @param [r] numofsubpages [ajuint] Number of subpages
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetNumSubPage(ajuint numofsubpages)
{
    ajDebug("=g= plssub (1, %d) [numofsubpages]\n", numofsubpages);
    plssub(1,numofsubpages);

    return;
}




/* @section Base colours *******************************************************
**
** Generating arrays of valid colour codes
**
** @fdata [none]
** @fcategory new
**
** @nam3rule Basecolour Base colour code array
** @nam4rule New Constructor
** @nam5rule Nuc Colours for nucleotide base codes
** @nam5rule Prot Colours for protein residue codes
** @argrule New codes [const AjPStr] Residue codes for each numbered position
**
** @valrule * [ajint*] Array of colours (see PLPLOT)
**
******************************************************************************/




/* @func ajGraphicsBasecolourNewNuc ********************************************
**
** Initialise a base colours array for a string of nucleotide
** sequence characters
**
** @param [r] codes [const AjPStr] Residue codes for each numbered position
** @return [ajint*] Array of colours (see PLPLOT)
** @@
******************************************************************************/

ajint* ajGraphicsBasecolourNewNuc(const AjPStr codes)
{
    ajint *ret;
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    ajint colours[] =
        {
	GREEN,YELLOW,BLUE,YELLOW,YELLOW,YELLOW,
	BLACK,YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,
	YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,
	YELLOW,RED,RED,YELLOW,YELLOW,YELLOW,
	YELLOW,YELLOW,YELLOW
    };
    const char* cp;
    const char* cq;
    ajint i;
    ajint j;

    ret = (ajint *) AJALLOC0(sizeof(ajint)*AZ);	/* BLACK is zero */

    ajDebug("ajGraphGetBaseColourProt '%S'\n", codes);

    cp = ajStrGetPtr(codes);
    i = 1;

    while(*cp)
    {
	cq = strchr(alphabet, *cp);

	if(cq)
	{
	    j = cq - alphabet;
	    ret[i] = colours[j];
	    ajDebug("ColourProt %d: '%c' %d -> %d %s\n",
		    i, *cp, j, colours[j], graphColourName[colours[j]]);
	}

	i++;
	cp++;
    }

    return ret;
}




/* @obsolete ajGraphGetBaseColourNuc
** @rename ajGraphicsBasecolourNewNuc
*/

__deprecated ajint* ajGraphGetBaseColourNuc(const AjPStr codes)
{
    return ajGraphicsBasecolourNewNuc(codes);
}




/* @func ajGraphicsBasecolourNewProt *******************************************
**
** Initialize a base colours array for a string of protein sequence characters
** according to the following colour scheme designed by Toby Gibson (EMBL
** Heidelberg) to show the chemical properties important in sequence
** and structure alignment and to be clear to those with common forms
** of colour blindness.
**
** DE: Red (Acidic)
** HKR: Blue (Basic)
** NQ: Green (Amide)
** AILV: Black (Hydrophobic Small)
** FWY: Wheat (Hydrophobic Large)
** ST: Cyan (Hydroxyl group)
** CM: Yellow (Sulphur chemistry)
** G: Grey (helix breaker I)
** P: Violet (helix breaker II)
**
** @param [r] codes [const AjPStr] Residue codes for each numbered position
** @return [ajint*] Array of colours (see PLPLOT)
** @@
******************************************************************************/

ajint* ajGraphicsBasecolourNewProt(const AjPStr codes)
{
    ajint *ret;
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    ajint colours[] =
        {
	BLACK,BLACK,YELLOW,RED,RED,WHEAT, /* A-F */
	GREY,BLUE,BLACK,BLACK,BLUE,BLACK, /* G-L */
	YELLOW,GREEN,BLACK,BLUEVIOLET,GREEN,BLUE, /* M-R */
	CYAN,CYAN,BLACK,BLACK,WHEAT,BLACK, /* S-X */
	WHEAT,BLACK,BLACK,BLACK};	/* Y-Z plus 2 more */
    const char* cp;
    const char* cq;
    ajint i;
    ajint j;

    ret = (ajint *) AJALLOC0(sizeof(ajint)*AZ);	/* BLACK is zero */

    ajDebug("ajGraphGetBaseColourProt '%S'\n", codes);

    cp = ajStrGetPtr(codes);
    i = 1;

    while(*cp)
    {
	cq = strchr(alphabet, *cp);

	if(cq)
	{
	    j = cq - alphabet;
	    ret[i] = colours[j];
	    ajDebug("ColourProt %d: '%c' %d -> %d %s\n",
		    i, *cp, j, colours[j], graphColourName[colours[j]]);
	}

	i++;
	cp++;
    }

    return ret;
}




/* @obsolete ajGraphGetBaseColourProt
** @rename ajGraphicsBasecolourNewProt
*/

__deprecated ajint* ajGraphGetBaseColourProt(const AjPStr codes)
{
    return ajGraphicsBasecolourNewProt(codes);
}




/* @obsolete ajGraphGetBaseColour
** @remove Use ajGraphicsBasecolourNewNuc or ajGraphicsBasecolourNewProt
*/

__deprecated ajint* ajGraphGetBaseColour(void)
{
    ajint *ret;

    ret = (ajint *) AJALLOC0(sizeof(ajint)*AZ);

    ret[0] = BLACK;			/* A */
    ret[1] = BLACK;			/* B */
    ret[2] = YELLOW;			/* C */
    ret[3] = RED;			/* D */
    ret[4] = RED;			/* E */
    ret[5] = WHEAT;			/* F */
    ret[6] = GREY;			/* G */
    ret[7] = BLUE;			/* H */
    ret[8] = BLACK;			/* I */
    ret[9] = BLACK;			/* J */
    ret[10] = BLUE;			/* K */
    ret[11] = BLACK;			/* L */
    ret[12] = YELLOW;			/* M */
    ret[13] = GREEN;			/* N */
    ret[14] = BLACK;			/* O */
    ret[15] = BLUEVIOLET;		/* P */
    ret[16] = GREEN;			/* Q */
    ret[17] = BLUE;			/* R */
    ret[18] = CYAN;			/* S */
    ret[19] = CYAN;			/* T */
    ret[20] = BLACK;			/* U */
    ret[21] = BLACK;			/* V */
    ret[22] = WHEAT;			/* W */
    ret[23] = BLACK;			/* X */
    ret[24] = WHEAT;			/* Y */
    ret[25] = BLACK;			/* Z */
    ret[26] = BLACK;			/* ? */
    ret[27] = BLACK;			/* ?*/

    return ret;
}




/* @section Queries ***********************************************************
**
** Querying and reporting internal values
**
** @fdata [none]
** @fcategory use
**
** @nam3rule Check Validate an external value
** @nam3rule Get Return an internal value
**
** @nam4rule Charsize Character size in plot units
** @nam4rule CheckColour Find if the colour is on the list
** @nam4rule Fgcolour Undocumented
** @nam4rule Params Get parameters
** @nam5rule ParamsPage Get the output page parameters
** @suffix C Character data
** @suffix S String object data
** 
**
** @argrule Charsize defheight [float*] default character height
** @argrule Charsize currentscale [float*] current (scaled) character height
** @argrule ParamsPage xp [float*] where to store the x pixels/inch
** @argrule ParamsPage yp [float*] where to store the y pixels/inch
** @argrule ParamsPage xleng [ajint*] where to store the x length
** @argrule ParamsPage yleng [ajint*] where to the y length
** @argrule ParamsPage xoff [ajint*] where to store the x offset
** @argrule ParamsPage yoff [ajint*] where to store the y offset
** @argrule C txt [const char*] colour name.
** @argrule S str [const AjPStr] colour name.
**
** @valrule * [void]
** @valrule *Check [ajint] the colour number if found else -1.
** @valrule *Fgcolour [ajint] the foreground colour number
**
******************************************************************************/




/* @func ajGraphicsCheckColourC ************************************************
**
** Find if the colour is on the list
**
** @param [r] txt [const char*] colour name.
** @return [ajint] the colour number if found else -1.
** @@
******************************************************************************/

ajint ajGraphicsCheckColourC(const char* txt)
{
    ajint i;

    for(i=0;i<=MAXCOL;i++)
	if(ajCharMatchCaseC(txt,graphColourName[i]))
	    return i;

    return -1;
}




/* @func ajGraphicsCheckColourS ************************************************
**
** Find if the colour is on the list
**
** @param [r] str [const AjPStr]  colour name.
** @return [ajint] the colour number if found else -1.
** @@
******************************************************************************/

ajint ajGraphicsCheckColourS(const AjPStr str)
{
    ajint i;

    for(i=0;i<=MAXCOL;i++)
	if(ajStrMatchCaseC(str,graphColourName[i]))
	    return i;

    return -1;
}




/* @obsolete ajGraphCheckColour
** @rename ajGraphicsCheckColourS
*/

__deprecated ajint ajGraphCheckColour(const AjPStr colour)
{
    return ajGraphicsCheckColourS(colour);
}




/* @func ajGraphicsGetCharsize *************************************************
**
** Get the char size.
**
** @param [u] defheight [float *] where to store the default character height
** @param [u] currentscale [float *] where to store the current (scaled)
**                                    character height
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsGetCharsize(float *defheight, float *currentscale)
{
    if(graphData)
    {
	*defheight = currentcharsize;
	*currentscale = currentcharscale;
	ajDebug("=g= plgchr(&f &f) [defht:%f, scale:%f] copy\n",
		*defheight, *currentscale);
    }
    else
    {
	plgchr(defheight,currentscale);
	ajDebug("=g= plgchr(&f &f) [defht:%f, scale:%f] plplot\n",
		*defheight, *currentscale);
    }

    return;
}




/* @func ajGraphicsGetFgcolour ************************************************
**
** Return current foreground colour
**
** @return [ajint] colour.
** @@
******************************************************************************/

ajint ajGraphicsGetFgcolour(void)
{
    return currentfgcolour;
}




/* @obsolete ajGraphGetColour
** @rename ajGraphicsGetFgcolour
*/

__deprecated ajint ajGraphGetColour(void)
{
    return ajGraphicsGetFgcolour();
}




/* @func ajGraphicsGetParamsPage ***********************************************
**
** Get the output page parameters for plplot internal use. These can be set by
** the user and used by some device drivers.
**
** For graph data type, sets to zero as these are not applicable.
**
** @param [u] xp [float *] where to store the x pixels/inch
** @param [u] yp [float *] where to store the y pixels/inch
** @param [u] xleng [ajint *] where to store the x length
** @param [u] yleng [ajint *] where to the y length
** @param [u] xoff [ajint *] where to store the x offset
** @param [u] yoff [ajint *] where to store the y offset
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsGetParamsPage(float *xp,float *yp, ajint *xleng, ajint *yleng,
                             ajint *xoff, ajint *yoff)
{
    ajDebug("=g= plgpage(&f &f) [&xp, &yp, &xleng, &yleng, &xoff, &yoff]\n");

    if(graphData)
    {
	*xp=0.0;
	*yp=0.0;
	*xleng=0;
	*yleng=0;
	*xoff=0;
	*yoff=0;
    }
    else
    {
	plgpage(xp,yp,xleng,yleng,xoff,yoff);
	ajDebug("returns xp:%.3f yp:%.3f xleng:%d yleng:%d xoff:%d yoff:%d\n",
		 *xp, *yp, *xleng, *yleng, *xoff, *yoff);
    }

    return;
}




/* @obsolete ajGraphGetOut
** @rename ajGraphicsGetParamsPage
*/

__deprecated void ajGraphGetOut(float *xp,float *yp, ajint *xleng, ajint *yleng,
		   ajint *xoff, ajint *yoff)

{

    ajGraphicsGetParamsPage(xp, yp, xleng, yleng, xoff, yoff);
    return;
}




/* @obsolete ajGraphGetCharSize
** @rename ajGraphicsGetCharsize
*/

__deprecated void ajGraphGetCharSize(float *defheight, float *currentheight)
{
    ajGraphicsGetCharsize(defheight, currentheight);
    return;
}




/* @section Drawing ***********************************************************
**
** Drawing lines and objects
**
** @fdata [none]
** @fcategory use
**
** @nam3rule Draw Draw at a point
** @nam3rule Drawarc Draw on an arc
** @nam3rule Drawbars Draw bar chart
** @nam3rule Drawpos Draw at a defined xy coordinate
** @nam3rule Drawset Draw at a set of defined xy coordinates
**
** @nam4rule Arc Draw an arc
** @nam4rule Bars Error bars
** @nam4rule Box Box
** @nam4rule Circle Circle
** @nam4rule Dia Diamond
** @nam4rule Dots Dots
** @nam4rule Line Line
** @nam4rule Lines LInes
** @nam4rule Poly Polygon
** @nam4rule Rect Rectangle
** @nam4rule Rlabel Right axis label
** @nam4rule Symbols Symbols
** @nam4rule Text Text
** @nam4rule Tri Triangle
** @nam4rule Horiz Horizontal direction
** @nam4rule Vert Vertical direction
** @nam5rule Atend Placed with end at fixed point
** @nam5rule Atline Along a straight line
** @nam5rule Atmid Centred at a point
** @nam5rule Atstart Placed with start at fixed point
** @suffix Fill Fill in the plot area
** @suffix Justify Justify text according to a plplot justify value
**
** @argrule Drawarc xcentre [PLFLT] Centre x coordinate
** @argrule Drawarc ycentre [PLFLT] Centre y coordinate
** @argrule Drawarc radius [PLFLT] Radius
** @argrule Drawarc startangle [PLFLT] Start angle
** @argrule Drawarc endangle [PLFLT] End angle
** @argrule DrawarcRect height [PLFLT] Height of the rectangle
**                                     in user coordinates.
**
** @argrule Drawbars num [ajuint] Number of dots
** @argrule DrawbarsHoriz y [CONST PLFLT*] y start positions
** @argrule DrawbarsHoriz xmin [CONST PLFLT*] x minimum values
** @argrule DrawbarsHoriz xmax [CONST PLFLT*] x maximum values
** @argrule DrawbarsVert x [CONST PLFLT*] x start positions
** @argrule DrawbarsVert ymin [CONST PLFLT*] y minimum values
** @argrule DrawbarsVert ymax [CONST PLFLT*] y maximum values
**
** @argrule Drawpos x [PLFLT] x start position
** @argrule Drawpos y [PLFLT] y start position
**
** @argrule Drawset num [ajuint] Number of dots
** @argrule Drawset xx [CONST PLFLT*] Centre x coordinates
** @argrule Drawset yy [CONST PLFLT*] Centre y coordinates
**
** @argrule DrawposBox size [PLFLT] Size of sides starting at x,y
** @argrule DrawposCircle radius [float] radius of the circle.
** @argrule DrawposDia size [PLFLT] Size of sides starting at x,y
** @argrule DrawposLine x2 [PLFLT] x end position
** @argrule DrawposLine y2 [PLFLT] y end position
** @argrule DrawposRect x2 [PLFLT] x1 coordinate
** @argrule DrawposRect y2 [PLFLT] y1 coordinate
** @argrule DrawposTri x2 [PLFLT] x2 coordinate
** @argrule DrawposTri y2 [PLFLT] y2 coordinate
** @argrule DrawposTri x3 [PLFLT] x3 coordinate
** @argrule DrawposTri y3 [PLFLT] y3 coordinate
** @argrule DrawsetLines xx2 [CONST PLFLT*] Centre x2 coordinates
** @argrule DrawsetLines yy2 [CONST PLFLT*] Centre y2 coordinates
** @argrule DrawsetSymbols symbol [ajuint] Symbol code.
** @argrule TextAtline x2 [PLFLT] x2 coordinate
** @argrule TextAtline y2 [PLFLT] y2 coordinate
** @argrule Text txt [const char*] The text to be displayed.
** @argrule Justify just [PLFLT] justification of the string.
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphicsDrawarcArc *************************************************
**
** Draw a portion of a circle (an arc).
**
** @param  [r] xcentre [PLFLT] x coord for centre.
** @param  [r] ycentre [PLFLT] y coord for centre.
** @param  [r] radius  [PLFLT] radius of the circle.
** @param  [r] startangle [PLFLT] angle of the start of the arc.
** @param  [r] endangle [PLFLT] angle of the end of the arc.
** @return [void]
** @@
**
** FIXME: Due to x and y not the same length this produces an oval!!
**       This will have to do for now. But I am aware that the code
**       is slow and not quite right.
******************************************************************************/

void ajGraphicsDrawarcArc(PLFLT xcentre, PLFLT ycentre, PLFLT radius,
                          PLFLT startangle, PLFLT endangle)
{
    PLFLT angle;
    ajuint i;
    PLFLT x[361];
    PLFLT y[361];
    ajuint numofpoints;
    PLFLT stopangle = endangle;

    if(endangle < startangle)
        stopangle += 360.0;

    if((stopangle-startangle)>=360.0)
        stopangle = startangle + (float) 360.0;

    x[0] = xcentre + ( radius*(float)cos(ajCvtDegToRad(startangle)) );
    y[0] = ycentre + ( radius*(float)sin(ajCvtDegToRad(startangle)) );

    for(i=1, angle=startangle+1; angle<stopangle; angle++, i++)
    {
	x[i] = xcentre + ( radius*(float)cos(ajCvtDegToRad(angle)) );
	y[i] = ycentre + ( radius*(float)sin(ajCvtDegToRad(angle)) );
    }

    x[i] = xcentre + ( radius*(float)cos(ajCvtDegToRad(stopangle)) );
    y[i] = ycentre + ( radius*(float)sin(ajCvtDegToRad(stopangle)) );
    numofpoints = i+1;

    GraphDrawsetLines(numofpoints, x, y);

    return;
}




/* @obsolete ajGraphPartCircle
** @rename ajGraphicsDrawarcArc
*/

__deprecated void ajGraphPartCircle(PLFLT xcentre, PLFLT ycentre, PLFLT Radius,
		       PLFLT StartAngle, PLFLT EndAngle)
{
    
    ajGraphicsDrawarcArc(xcentre, ycentre, Radius, StartAngle, EndAngle);
    return;
}





/* @func ajGraphicsDrawarcRect ************************************************
**
** Draw a rectangle along a curve with the current pen colour/style.
**
** @param  [r] xcentre [PLFLT] x coord for centre.
** @param  [r] ycentre [PLFLT] y coord for centre.
** @param  [r] radius  [PLFLT] radius of the circle.
** @param  [r] startangle [PLFLT] angle of the start of the rectangle.
** @param  [r] endangle [PLFLT] angle of the end of the rectangle.
** @param  [r] height [PLFLT] Height of the rectangle in user coordinates.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawarcRect(PLFLT xcentre, PLFLT ycentre, PLFLT radius,
                           PLFLT startangle, PLFLT endangle,
                           PLFLT height)
{
    PLFLT *xyy1;
    PLFLT *xyy2;
    PLFLT r1Blocks;
    PLFLT r2Blocks;

    r1Blocks = radius;
    r2Blocks = r1Blocks+height;

    ajGraphicsDrawarcArc(xcentre, ycentre, r1Blocks, startangle, endangle);
    ajGraphicsDrawarcArc(xcentre, ycentre, r2Blocks, startangle, endangle);

    xyy1 = ajGraphicsCalcCoord(xcentre, ycentre, r1Blocks, startangle);
    xyy2 = ajGraphicsCalcCoord(xcentre, ycentre, r2Blocks, startangle);
    ajGraphicsDrawposLine( xyy1[0], xyy1[1], xyy2[0], xyy2[1] );
    AJFREE(xyy1);
    AJFREE(xyy2);
    xyy1 = ajGraphicsCalcCoord(xcentre, ycentre, r1Blocks, endangle);
    xyy2 = ajGraphicsCalcCoord(xcentre, ycentre, r2Blocks, endangle);
    ajGraphicsDrawposLine( xyy1[0], xyy1[1], xyy2[0], xyy2[1] );
    AJFREE(xyy1);
    AJFREE(xyy2);

    return;
}




/* @obsolete ajGraphRectangleOnCurve
** @replace ajGraphicsDrawarcRect (1,2,3,4,5,6/1,2,3,5,6,4)
*/

__deprecated void ajGraphRectangleOnCurve(PLFLT xcentre, PLFLT ycentre,
                                          PLFLT Radius, PLFLT BoxHeight,
                                          PLFLT StartAngle, PLFLT EndAngle)
{

    ajGraphicsDrawarcRect(xcentre, ycentre, Radius,
                          StartAngle, EndAngle, BoxHeight);
    return;
}




/* @func ajGraphicsDrawarcRectFill ********************************************
**
** Draw a rectangle along a curve and fill it with the current pen
** colour/style.
**
** @param  [r] xcentre [PLFLT] x coord for centre.
** @param  [r] ycentre [PLFLT] y coord for centre.
** @param  [r] radius  [PLFLT] radius of the circle.
** @param  [r] startangle [PLFLT] angle of the start of the rectangle.
** @param  [r] endangle [PLFLT] angle of the end of the rectangle.
** @param  [r] height [PLFLT] Height of the rectangle in user coordinates.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawarcRectFill(PLFLT xcentre, PLFLT ycentre, PLFLT radius,
                               PLFLT startangle, PLFLT endangle,
                               PLFLT height)
{
    PLFLT angle;
    ajuint i;
    PLFLT x[4];
    PLFLT y[4];
    ajuint numofpoints;
    PLFLT r1Blocks;
    PLFLT r2Blocks;
    PLFLT stopangle = endangle;

    if(endangle < startangle)
        stopangle += 360.0;

    if((stopangle-startangle)>=360.0)
        stopangle = startangle + (float) 360.0;

    r1Blocks = radius;
    r2Blocks = r1Blocks+height;

    x[0]=xcentre + ( r1Blocks*(float)cos(ajCvtDegToRad(startangle)) );
    y[0]=ycentre + ( r1Blocks*(float)sin(ajCvtDegToRad(startangle)) );
    x[1]=xcentre + ( r2Blocks*(float)cos(ajCvtDegToRad(startangle)) );
    y[1]=ycentre + ( r2Blocks*(float)sin(ajCvtDegToRad(startangle)) );

    for(i=0, angle=startangle+1; angle<stopangle; angle++, i++)
    {
	x[2]=xcentre + ( r2Blocks*(float)cos(ajCvtDegToRad(angle)) );
	y[2]=ycentre + ( r2Blocks*(float)sin(ajCvtDegToRad(angle)) );
	x[3]=xcentre + ( r1Blocks*(float)cos(ajCvtDegToRad(angle)) );
	y[3]=ycentre + ( r1Blocks*(float)sin(ajCvtDegToRad(angle)) );
	numofpoints = 4;
	ajGraphicsDrawsetPolyFill(numofpoints, x, y);
        x[0] = x[3];
        y[0] = y[3];
        x[1] = x[2];
        y[1] = y[2];
    }

    x[2]=xcentre + ( r2Blocks*(float)cos(ajCvtDegToRad(stopangle)) );
    y[2]=ycentre + ( r2Blocks*(float)sin(ajCvtDegToRad(stopangle)) );
    x[3]=xcentre + ( r1Blocks*(float)cos(ajCvtDegToRad(stopangle)) );
    y[3]=ycentre + ( r1Blocks*(float)sin(ajCvtDegToRad(stopangle)) );
    numofpoints = 4;
    ajGraphicsDrawsetPolyFill(numofpoints, x, y);

    return;
}




/* @obsolete ajGraphFillRectangleOnCurve
** @replace ajGraphicsDrawarcRectFill (1,2,3,4,5,6/1,2,3,5,6,4)
*/

__deprecated void ajGraphFillRectangleOnCurve(PLFLT xcentre, PLFLT ycentre,
                                              PLFLT Radius,
                                              PLFLT BoxHeight, PLFLT StartAngle,
                                              PLFLT EndAngle)
{

    ajGraphicsDrawarcRectFill(xcentre, ycentre, Radius,
                              StartAngle, EndAngle, BoxHeight);
    return;
}




/* @func ajGraphicsDrawarcTextJustify *****************************************
**
** Draw text along a curve (i.e., an arc of a circle).  The text is
** written character by character, forwards or backwards depending on
** the angle.
**
** @param  [r] xcentre [PLFLT] x coord for centre.
** @param  [r] ycentre [PLFLT] y coord for centre.
** @param  [r] radius  [PLFLT] radius of the circle.
** @param  [r] startangle [PLFLT] angle of the start of the arc (in deg).
** @param  [r] endangle [PLFLT] angle of the end of the arc (in deg).
** @param  [r] txt [const char*] The text to be displayed.
** @param  [r] just [PLFLT] justification of the string.
**                          (0=left,1=right,0.5=middle etc)
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawarcTextJustify(PLFLT xcentre, PLFLT ycentre, PLFLT radius,
                                  PLFLT startangle, PLFLT endangle,
                                  const char *txt, PLFLT just)
{
    ajint numchar;
    PLFLT pos;

    numchar = strlen(txt);
    pos = (endangle-startangle)/numchar;

    if( ((startangle>180.0 && startangle<=360.0) &&
	 (endangle>180.0 && endangle<=360.0)) ||
       ((startangle>540.0 && startangle<=720.0)
	&& (endangle>540.0 && endangle<=720.0)) )
	GraphDrawarcText(xcentre, ycentre, radius, startangle,
                         +1*pos, txt, just);
    else GraphDrawarcText(xcentre, ycentre, radius, endangle,
                          -1*pos, txt, just);

    return;
}




/* @obsolete ajGraphDrawTextOnCurve
** @rename ajGraphicsDrawarcTextJustify
*/

__deprecated void ajGraphDrawTextOnCurve(PLFLT xcentre, PLFLT ycentre,
                                         PLFLT Radius, PLFLT StartAngle,
                                         PLFLT EndAngle,
                                         const char *Text, PLFLT just)

{
    ajGraphicsDrawarcTextJustify(xcentre, ycentre, Radius,
                                 StartAngle, EndAngle, Text, just);
    return;
}




/* @func ajGraphicsDrawbarsHoriz ***********************************************
**
** Draw Horizontal Error Bars.
**
** @param [r] num [ajuint] number of error bars to be drawn.
** @param [r] y [CONST PLFLT*] y positions to draw at.
** @param [r] xmin [CONST PLFLT*] x positions to start at.
** @param [r] xmax [CONST PLFLT*] x positions to end at.
** @return [void]
**
** @@
******************************************************************************/
void ajGraphicsDrawbarsHoriz(ajuint num,
                             PLFLT *y, PLFLT *xmin, PLFLT *xmax)
{
    if(num)
	ajDebug("=g= plerrx(%d %.2f..%.2f, %.2f, %.2f) "
		 "[num,y..y,xmin,xmax]\n",
		 num, y[0], y[num-1], xmin, xmax);
    else
	ajDebug("=g= plerrx(%d <>..<>, %.2f, %.2f) [num,y..y,xmin,xmax]\n",
		 num, xmin, xmax);

    plerrx(num,y,xmin,xmax);

    return;
}




/* @obsolete ajGraphHoriBars
** @rename ajGraphicsDrawbarsHoriz
*/

__deprecated void ajGraphHoriBars(ajuint numofpoints,
                                  PLFLT *y, PLFLT *xmin, PLFLT *xmax)
{
    ajGraphicsDrawbarsHoriz(numofpoints,y, xmin, xmax);
    return;
}




/* @func ajGraphicsDrawbarsVert ************************************************
**
** Draw vertical Error Bars.
**
** @param [r] num [ajuint] number of error bars to be drawn.
** @param [r] x [CONST PLFLT*] x positions to draw at.
** @param [r] ymin [CONST PLFLT*] y positions to start at.
** @param [r] ymax [CONST PLFLT*] y positions to end at.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawbarsVert(ajuint num,
                            PLFLT *x, PLFLT *ymin, PLFLT *ymax)
{
    if(num)
	ajDebug("=g= plerry(%d %.2f..%.2f, %.2f, %.2f) "
		 "[num,x..x,ymin,ymax]\n",
		 num, x[0], x[num-1], ymin, ymax);
    else
	ajDebug("=g= plerry(%d <>..<>, %.2f, %.2f) [num,x..x,ymin,ymax]\n",
		 num, ymin, ymax);

    plerry(num,x,ymin,ymax);

    return;
}




/* @obsolete ajGraphVertBars
** @rename ajGraphicsDrawbarsVert
*/

__deprecated void ajGraphVertBars(ajuint numofpoints,
                                  PLFLT *x, PLFLT *ymin, PLFLT *ymax)
{
    ajGraphicsDrawbarsVert(numofpoints, x, ymin, ymax);
    return;
}




/* @func ajGraphicsDrawposBox ************************************************
**
** Draw a box to the plotter device at point xx0,yy0 size big.
**
** @param [r] x [PLFLT] x position to draw the box.
** @param [r] y [PLFLT] y position to draw the box.
** @param [r] size [PLFLT]  how big to draw the box.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposBox(PLFLT x, PLFLT y,PLFLT size)
{
    PLFLT xa[5];
    PLFLT ya[5];

    if(graphData)
    {
	ajFmtPrintF(graphData->File,"Rectangle x1 %f y1 %f x2 %f"
		    " y2 %f colour %d\n",
		    x, y, x+size, y+size, currentfgcolour);
	graphData->Lines++;
    }
    else
    {
	xa[0] = x;
	ya[0] = y;
	xa[1] = x;
	ya[1] = y + size;
	xa[2] = x + size;
	ya[2] = y + size;
	xa[3] = x + size;
	ya[3] = y;
	xa[4] = x;
	ya[4] = y;
	GraphArray(5, xa, ya);
    }

    return;
}




/* @obsolete ajGraphBox
** @rename ajGraphicsDrawposBox
*/

__deprecated void ajGraphBox(PLFLT xx0, PLFLT yy0,PLFLT size)
{
    ajGraphicsDrawposBox(xx0, yy0, size);
    return;
}




/* @func ajGraphicsDrawposBoxFill **********************************************
**
** Draw a box to the plotter device at point xx0,yy0 size big and
** fill it in.
**
** @param [r] x [PLFLT] x position to draw the box.
** @param [r] y [PLFLT] y position to draw the box.
** @param [r] size [PLFLT]  how big to draw the box.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposBoxFill(PLFLT x, PLFLT y, PLFLT size)
{
    PLFLT xa[4];
    PLFLT ya[4];

    if(graphData)
    {
	ajFmtPrintF(graphData->File,"Shaded Rectangle x1 %f y1 %f x2 %f"
		    " y2 %f colour %d\n",
		    x, y, x+size, y+size, currentfgcolour);
	graphData->Lines++;
    }
    else
    {
	xa[0] = x;
	ya[0] = y;
	xa[1] = x;
	ya[1] = y + size;
	xa[2] = x + size;
	ya[2] = y + size;
	xa[3] = x + size;
	ya[3] = y;
	GraphFill(4, xa, ya);
    }

    return;
}




/* @obsolete ajGraphBoxFill
** @rename ajGraphicsDrawposBoxFill
*/

__deprecated void ajGraphBoxFill(PLFLT xx0, PLFLT yy0, PLFLT size)
{
    ajGraphicsDrawposBoxFill(xx0, yy0, size);
    return;
}




/* @func ajGraphicsDrawposCircle **********************************************
**
** Draw a circle.
**
** @param  [r] x [PLFLT] x coord for centre.
** @param  [r] y [PLFLT] y coord for centre.
** @param  [r] radius  [float] radius of the circle.
** @return [void]
** @@
**
** FIXME: Due to x and y not the same length this produces an oval!!
**       This will have to do for now. But I (il@sanger) am aware that the code
**       is slow and not quite right.
******************************************************************************/

void ajGraphicsDrawposCircle(PLFLT x, PLFLT y, float radius)
{
    PLFLT xa[361];
    PLFLT ya[361];
    
    ajint i;

    /* graphData : uses GraphArray */

    for(i=0;i<360;i++)
    {
	xa[i] = x + (radius * (float)cos(ajCvtDegToRad((float)i)));
	ya[i] = y + (radius * (float)sin(ajCvtDegToRad((float)i)));
    }

    xa[360] = xa[0];
    ya[360] = ya[0];

    GraphArray(361, xa,ya);

    return;
}




/* @obsolete ajGraphCircle
** @rename ajGraphicsDrawposCircle
*/

__deprecated void ajGraphCircle(PLFLT xcentre, PLFLT ycentre, float radius)
{
    ajGraphicsDrawposCircle(xcentre, ycentre, radius);
    return;
}




/* @func ajGraphicsDrawposDia **************************************************
**
** Draw a diamond to the plotter device at point xx0,yy0 size big.
**
** @param [r] x [PLFLT] x position to draw the diamond.
** @param [r] y [PLFLT] y position to draw the diamond.
** @param [r] size [PLFLT]  how big to draw the diamond.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposDia(PLFLT x, PLFLT y, PLFLT size)
{
    PLFLT xa[5];
    PLFLT ya[5];
    PLFLT incr;

    incr = size*(float)0.5;

    /* graphData : uses GraphArray */

    xa[0] = x;
    ya[0] = y + incr;
    xa[1] = x + incr;
    ya[1] = y + size;
    xa[2] = x + size;
    ya[2] = y + incr;
    xa[3] = x + incr;
    ya[3] = y;
    xa[4] = x;
    ya[4] = y;
    GraphArray(5, xa, ya);

    return;
}




/* @obsolete ajGraphDia
** @rename ajGraphicsDrawposDia
*/

__deprecated void ajGraphDia(PLFLT xx0, PLFLT yy0, PLFLT size)
{
    ajGraphicsDrawposDia(xx0, yy0, size);
    return;
}




/* @func ajGraphicsDrawposDiaFill **********************************************
**
** Draw a diamond to the plotter device at point xx0,yy0 size big and
** fill it in.
**
** @param [r] x [PLFLT] x position to draw the diamond.
** @param [r] y [PLFLT] y position to draw the diamond.
** @param [r] size [PLFLT]  how big to draw the diamond.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposDiaFill(PLFLT x, PLFLT y, PLFLT size)
{
    PLFLT xa[4];
    PLFLT ya[4];
    PLFLT incr;

    incr = size*(float)0.5;

    xa[0] = x;
    ya[0] = y + incr;
    xa[1] = x + incr;
    ya[1] = y + size;
    xa[2] = x + size;
    ya[2] = y + incr;
    xa[3] = x + incr;
    ya[3] = y;

    GraphFill(4, xa, ya);

    return;
}




/* @obsolete ajGraphDiaFill
** @rename ajGraphicsDrawposDiaFill
*/

__deprecated void ajGraphDiaFill(PLFLT xx0, PLFLT yy0, PLFLT size)
{
    ajGraphicsDrawposDiaFill(xx0, yy0, size);
    return;
}




/* @func ajGraphicsDrawposLine *************************************************
**
** Draw line between 2 points.
**
** @param [r] x [PLFLT]  x start position.
** @param [r] y [PLFLT]  y start position.
** @param [r] x2 [PLFLT]  x end position.
** @param [r] y2 [PLFLT]  y end position.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposLine(PLFLT x, PLFLT y, PLFLT x2, PLFLT y2)
{
    if(graphData)
    {
	ajFmtPrintF(graphData->File,
		    "Line x1 %f y1 %f x2 %f y2 %f colour %d\n",
		    x, y, x2, y2, currentfgcolour);
	graphData->Lines++;
    }
    else
    {
	/*ajDebug("=g= pljoin(%.2f, %.2f, %.2f, %.2f) [xy xy]\n",
		 x, y, x2, y2);*/
	pljoin(x,y,x2,y2);
    }

    return;
}




/* @obsolete ajGraphLine
** @rename ajGraphicsDrawposLine
*/

__deprecated void ajGraphLine(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2)
{
    ajGraphicsDrawposLine(xx1, yy1, xx2, yy2);
    return;
}




/* @func ajGraphicsDrawposRect *************************************************
**
** Draw a rectangle with the current pen colour/style.
**
** @param [r] x [PLFLT] xx0 coord.
** @param [r] y [PLFLT] yy0 coord.
** @param [r] x2 [PLFLT] xx1 coord.
** @param [r] y2 [PLFLT] yy1 coord.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposRect(PLFLT x, PLFLT y,PLFLT x2, PLFLT y2)
{
    PLFLT xa[5];
    PLFLT ya[5];

    if(graphData)
    {
	ajFmtPrintF(graphData->File,"Rectangle x1 %f y1 %f x2 %f"
		    " y2 %f colour %d\n",
		    x, y, x2, y2, currentfgcolour);
	graphData->Lines++;
    }
    else
    {
	xa[0] = x;
	ya[0] = y;
	xa[1] = x;
	ya[1] = y2;
	xa[2] = x2;
	ya[2] = y2;
	xa[3] = x2;
	ya[3] = y;
	xa[4] = x;
	ya[4] = y;
	GraphArray(5, xa, ya);
    }

    return;
}




/* @obsolete ajGraphRect
** @rename ajGraphicsDrawposRect
*/

__deprecated void ajGraphRect(PLFLT xx0, PLFLT yy0,PLFLT xx1, PLFLT yy1)
{
    ajGraphicsDrawposRect(xx0, yy0, xx1, yy1);
    return;
}




/* @func ajGraphicsDrawposRectFill *********************************************
**
** Draw a rectangle and fill it with the current pen colour/style.
**
** @param [r] x [PLFLT] xx0 coord.
** @param [r] y [PLFLT] yy0 coord.
** @param [r] x2 [PLFLT] xx1 coord.
** @param [r] y2 [PLFLT] yy1 coord.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposRectFill(PLFLT x, PLFLT y, PLFLT x2, PLFLT y2)
{
    PLFLT xa[4];
    PLFLT ya[4];

    if(graphData)
    {
	ajFmtPrintF(graphData->File,"Shaded Rectangle x1 %f y1 %f x2 %f"
		    " y2 %f colour %d\n",
		    x, y, x2, y2, currentfgcolour);
	graphData->Lines++;
    }
    else
    {
	xa[0] = x;
	ya[0] = y;
	xa[1] = x;
	ya[1] = y2;
	xa[2] = x2;
	ya[2] = y2;
	xa[3] = x2;
	ya[3] = y;
	GraphFill(4, xa, ya);
    }

    return;
}




/* @obsolete ajGraphRectFill
** @rename ajGraphicsDrawposRectFill
*/

__deprecated void ajGraphRectFill(PLFLT xx0, PLFLT yy0, PLFLT xx1, PLFLT yy1)
{
    ajGraphicsDrawposRectFill(xx0, yy0, xx1, yy1);
    return;
}




/* @func ajGraphicsDrawposTextAtend ********************************************
**
** Draw text ending at position (xx1,yy1)
**
** @param [r] x [PLFLT] xx1 coord.
** @param [r] y [PLFLT] yy1 coord.
** @param [r] txt [const char*] The text to be displayed.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposTextAtend(PLFLT x, PLFLT y, const char *txt)
{
    if(graphData)
    {
	ajFmtPrintF(graphData->File,
		    "Text3 x1 %f y1 %f colour %d size %f %s\n",
		    x,y,
		    currentfgcolour, currentcharscale,txt);
	graphData->Lines++;
    }
    else
	GraphText(x,y,1.0,0.0,1.0, txt);

    return;
}




/* @obsolete ajGraphTextEnd
** @rename ajGraphicsDrawposTextAtend
*/

__deprecated void ajGraphTextEnd(PLFLT x, PLFLT y, const char *text)
{
    ajGraphicsDrawposTextAtend(x,y,text);
    return;
}




/* @func ajGraphicsDrawposTextAtlineJustify ************************************
**
** Draw text along a line.
**
** @param [r] x [PLFLT] x1 coord.
** @param [r] y [PLFLT] y1 coord.
** @param [r] x2 [PLFLT] x2 coord.
** @param [r] y2 [PLFLT] y2 coord.
** @param [r] txt [const char*] The text to be displayed.
** @param [r] just [PLFLT] justification of the string.
**                         (0=left,1=right,0.5=middle etc)
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposTextAtlineJustify(PLFLT x, PLFLT y, PLFLT x2, PLFLT y2,
                                        const char *txt, PLFLT just)
{
    if(graphData)
    {
	ajFmtPrintF(graphData->File,
		    "Textline x1 %f y1 %f x2 %f y2 %f colour %d size %f %s\n",
		    x,y,x2, y2,
		    currentfgcolour, currentcharscale,txt);
	graphData->Lines++;
    }
    else
	GraphText(x,y,x2-x,y2-y,just, txt);

    return;
}




/* @obsolete ajGraphTextLine
** @rename ajGraphicsDrawposTextAtlineJustify
*/

__deprecated void ajGraphTextLine(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
		     const char *text, PLFLT just)
{
    ajGraphicsDrawposTextAtlineJustify(xx1, yy1, xx2, yy2, text, just);
    return;
}




/* @func ajGraphicsDrawposTextAtmid ********************************************
**
** Draw text with Mid point of text at (xx1,yy1).
**
** For graph data type, writes directly.
**
** @param [r] x [PLFLT] xx1 coord.
** @param [r] y [PLFLT] yy1 coord.
** @param [r] txt [const char*] The text to be displayed.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposTextAtmid(PLFLT x, PLFLT y, const char *txt)
{
    if(graphData)
    {
	ajFmtPrintF(graphData->File,
		    "Text2 x1 %f y1 %f colour %d size %f %s\n",
		    x, y,
		    currentfgcolour, currentcharscale, txt);
	graphData->Lines++;
    }
    else
	GraphText(x,y,1.0,0.0,0.5, txt);

    return;
}




/* @obsolete ajGraphTextMid
** @rename ajGraphicsDrawposTextAtmid
*/

__deprecated void ajGraphTextMid(PLFLT xx1, PLFLT yy1, const char *text)
{
    ajGraphicsDrawposTextAtmid(xx1,yy1,text);
    return;
}




/* @func ajGraphicsDrawposTextAtstart ******************************************
**
** Draw text starting at position (xx1,yy1)
**
** @param [r] x [PLFLT] xx1 coordinate
** @param [r] y [PLFLT] yy1 coordinate
** @param [r] txt [const char*] The text to be displayed.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposTextAtstart(PLFLT x, PLFLT y, const char *txt)
{
    if(graphData)
    {
	ajFmtPrintF(graphData->File,
		    "Text1 x1 %f y1 %f colour %d size %f %s\n",
		    x,y,
		    currentfgcolour, currentcharscale,txt);
	graphData->Lines++;
    }
    else
	GraphText(x,y,1.0,0.0,0.0, txt);

    return;
}




/* @obsolete ajGraphTextStart
** @rename ajGraphicsDrawposTextAtstart
*/

__deprecated void ajGraphTextStart(PLFLT xx1, PLFLT yy1, const char *text)
{
    ajGraphicsDrawposTextAtstart(xx1,yy1,text);
    return;
}




/* @func ajGraphicsDrawposTextJustify ******************************************
**
** Draw text, positioning with respect to (xx1,yy1) by justified as
** defined by just.
**
** @param [r] x [PLFLT] xx1 coord.
** @param [r] y [PLFLT] yy1 coord.
** @param [r] txt [const char*] The text to be displayed.
** @param [r] just [PLFLT] justification of the string.
**                         (0=left,1=right,0.5=middle etc)
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawposTextJustify(PLFLT x, PLFLT y, const char *txt,
                                  PLFLT just)
{
    if(graphData)
    {
	ajFmtPrintF(graphData->File,
		    "Text1 x1 %f y1 %f colour %d size %f %s\n",
		    x,y,currentfgcolour, currentcharscale,txt);
	graphData->Lines++;
    }
    else
	GraphText(x,y,1.0,0.0,just, txt);

    return;
}




/* @obsolete ajGraphText
** @rename ajGraphicsDrawposTextJustify
*/

__deprecated void ajGraphText(PLFLT xx1, PLFLT yy1,
                              const char *text, PLFLT just)
{
    ajGraphicsDrawposTextJustify(xx1,yy1,text,just);
    return;
}




/* @func ajGraphicsDrawposTri **************************************************
**
** Draw a Triangle.
**
** @param [r] x [PLFLT] xx1 coord of point 1.
** @param [r] y [PLFLT] yy1 coord of point 1.
** @param [r] x2 [PLFLT] xx2 coord of point 2.
** @param [r] y2 [PLFLT] yy2 coord of point 2.
** @param [r] x3 [PLFLT] x3 coord of point 3.
** @param [r] y3 [PLFLT] y3 coord of point 3.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposTri(PLFLT x, PLFLT y, PLFLT x2, PLFLT y2,
                          PLFLT x3, PLFLT y3)
{
    PLFLT xa[4];
    PLFLT ya[4];
    
    xa[0]=x; xa[1]=x2; xa[2]=x3; xa[3]=x;
    ya[0]=y; ya[1]=y2; ya[2]=y3; ya[3]=y;

    ajGraphicsDrawsetPoly(4, xa, ya);

    return;
}




/* @obsolete ajGraphTri
** @rename ajGraphicsDrawposTri
*/

__deprecated void ajGraphTri(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
		PLFLT x3, PLFLT y3)
{
    ajGraphicsDrawposTri(xx1,yy1,xx2,yy2,x3,y3);
    return;
}




/* @func ajGraphicsDrawposTriFill **********************************************
**
** Draw a Triangle and fill it in.
**
** @param [r] x [PLFLT] xx1 coord of point 1.
** @param [r] y [PLFLT] yy1 coord of point 1.
** @param [r] x2 [PLFLT] xx2 coord of point 2.
** @param [r] y2 [PLFLT] yy2 coord of point 2.
** @param [r] x3 [PLFLT] x3 coord of point 3.
** @param [r] y3 [PLFLT] y3 coord of point 3.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawposTriFill(PLFLT x, PLFLT y, PLFLT x2, PLFLT y2,
                              PLFLT x3, PLFLT y3)
{
    PLFLT xa[3];
    PLFLT ya[3];
    
    xa[0]=x; xa[1]=x2; xa[2]=x3;
    ya[0]=y; ya[1]=y2; ya[2]=y3;

    ajGraphicsDrawsetPolyFill(3, xa, ya);

    return;
}




/* @obsolete ajGraphTriFill
** @rename ajGraphicsDrawposTriFill
*/

__deprecated void ajGraphTriFill(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
                                 PLFLT x3, PLFLT y3)
{
    ajGraphicsDrawposTriFill(xx1,yy1,xx2,yy2,x3,y3);
    return;
}




/* @func ajGraphicsDrawsetDots *************************************************
**
** Draw a set of dots.
**
** @param [r] num [ajuint] The number of dots to be drawn.
** @param [r] xx [CONST PLFLT*] xx1 coord.
** @param [r] yy [CONST PLFLT*] yy1 coord.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawsetDots(ajuint num, PLFLT *xx, PLFLT *yy)
{
    GraphSymbols(xx, yy, num, 17);

    /* see xx06c in examples for codes e.g. 17 is a dot*/
    return;
}




/* @obsolete ajGraphDots
** @replace ajGraphicsDrawsetDots (1,2,3/3,1,2)
*/

__deprecated void ajGraphDots(PLFLT *xx1, PLFLT *yy1, ajuint numofdots)
{
    ajGraphicsDrawsetDots(numofdots,xx1,yy1);
    return;
}




/* @funcstatic GraphDrawsetLines **********************************************
**
** Draw a number of lines. The points are stored in two array.
**
** @param [r] num [ajuint] number of joined points to plot.
** @param [r] xx [CONST PLFLT*] pointer to x coordinates.
** @param [r] yy [CONST PLFLT*] pointer to y coordinates.
** @return [void]
** @@
******************************************************************************/

static void GraphDrawsetLines(ajuint num, PLFLT *xx, PLFLT *yy)
{
    ajuint i;
    PLFLT xx0;
    PLFLT yy0;
    PLFLT *xx1 = xx;
    PLFLT *yy1 = yy;

    /* graphdata : calls ajGraphicsDrawposLine */

    for(i=1; i<num; i++)
    {
	xx0 = *xx1++;
	yy0 = *yy1++;
	ajGraphicsDrawposLine(xx0,yy0,*xx1,*yy1);
    }

    return;
}




/* @func ajGraphicsDrawsetLines ************************************************
**
** Draw a set of lines.
**
** @param [r] num [ajuint] The number of lines to be drawn.
** @param [u] xx [CONST PLFLT*] x start coordinates
** @param [u] yy [CONST PLFLT*] y start coordinates
** @param [u] xx2 [CONST PLFLT*] x end coordinates
** @param [u] yy2 [CONST PLFLT*] y end coordinates
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawsetLines(ajuint num, PLFLT *xx, PLFLT *yy,
                            PLFLT *xx2, PLFLT *yy2)
{
    ajuint i = 0;
    PLFLT *xa = xx;
    PLFLT *ya = yy;
    PLFLT *xb = xx2;
    PLFLT *yb = yy2;

    /* graphdata : calls ajGraphLine */

    ajDebug("=g= ajGraphicsDrawsetLines(... %d) [xx1,xx2,yy1,yy2,numoflines]\n",
	    num);

    for(i=0; i<num; i++)
	ajGraphicsDrawposLine(*xa++,*ya++,*xb++,*yb++);

    return;
}




/* @obsolete ajGraphLines
** @replace ajGraphicsDrawsetLines (1,2,3,4,5/5,1,2,3,4)
*/

__deprecated void ajGraphLines(PLFLT *xx1, PLFLT *yy1, PLFLT *xx2, PLFLT *yy2,
                               ajuint numoflines)

{
    ajGraphicsDrawsetLines(numoflines, xx1, yy1, xx2, yy2);
    return;
}




/* @func ajGraphicsDrawsetPoly *************************************************
**
** Draw a polygon.
**
** @param [r] num [ajuint] number of points
** @param [r] xx [CONST PLFLT *] x coord of points
** @param [r] yy [CONST PLFLT *] y coord of points
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawsetPoly(ajuint num, PLFLT *xx, PLFLT *yy)
{
    /* graphData : uses GraphArray */

    GraphArray(num, xx, yy);

    return;
}




/* @obsolete ajGraphPoly
** @rename ajGraphicsDrawsetPoly
*/

__deprecated void ajGraphPoly(ajuint n, PLFLT *x, PLFLT *y)
{
    ajGraphicsDrawsetPoly(n, x, y);
    return;
}




/* @func ajGraphicsDrawsetPolyFill *********************************************
**
** Draw a polygon and fill it in.
**
** @param [r] num [ajuint] number of points
** @param [r] xx [CONST PLFLT *] x coord of points
** @param [r] yy [CONST PLFLT *] y coord of points
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDrawsetPolyFill(ajuint num, PLFLT *xx, PLFLT *yy)
{
    GraphFill(num, xx, yy);

    return;
}




/* @obsolete ajGraphPolyFill
** @rename ajGraphicsDrawsetPolyFill
*/

__deprecated void ajGraphPolyFill(ajuint n, PLFLT *x, PLFLT *y)
{
    ajGraphicsDrawsetPolyFill(n, x, y);
    return;
}




/* @func ajGraphicsDrawsetSymbols **********************************************
**
** Draw a set of dots.
**
** @param [r] num [ajuint] Number of coordinates in xx1 and xx2.
** @param [r] xx [CONST PLFLT*] xx1 coord.
** @param [r] yy [CONST PLFLT*] yy1 coord.
** @param [r] symbol [ajuint] Symbol code.
** @return [void]
**
** @@
******************************************************************************/

void ajGraphicsDrawsetSymbols(ajuint num, PLFLT *xx, PLFLT *yy,
                              ajuint symbol)
{
    GraphSymbols(xx, yy, num, symbol);

    /* see xx06c in examples for codes e.g. 17 is a dot*/
    return;
}




/* @obsolete ajGraphSymbols
** @rename ajGraphicsDrawsetSymbols
*/

__deprecated void ajGraphSymbols( ajuint numofdots, PLFLT *xx1,PLFLT *yy1,
                                  ajuint symbol)
{
    ajGraphicsDrawsetSymbols(numofdots,xx1,yy1,symbol);
    return;
}




/* @section Calculations ******************************************************
**
** Calculations using plot internals
**
** @fdata [none]
** @fcategory cast
**
** @nam3rule Calc Calculation
** @nam4rule Charsize Computes the character size (in mm)
** @nam4rule Coord Compute the coordinates of a point on a circle
**                 knowing the angle
** @nam4rule Distance Compute the distance between 2 points in user coordinates
** @nam4rule Range Get the max and min of the data points you wish to display.
** @nam4rule Textheight Get text height (in mm)
** @nam4rule Textlength Get text length (in mm)
** @suffix C Character data
** @suffix S String object data
**
** @argrule Charsize xx1 [PLFLT] xx1 coord.
** @argrule Charsize yy1 [PLFLT] yy1 coord.
** @argrule Charsize xx2 [PLFLT] xx2 coord.
** @argrule Charsize yy2 [PLFLT] yy2 coord.
** @argrule Charsize text [const char*] The text to be displayed.
** @argrule Charsize TextHeight [PLFLT] The height of the text (in user coord).
** @argrule Coord x [PLFLT] x coord for centre.
** @argrule Coord y [PLFLT] y coord for centre.
** @argrule Coord radius [PLFLT] Radius of the circle.
** @argrule Coord angle [PLFLT] angle at which the point is.
** @argrule Distance xx1 [PLFLT] x coord of point 1
** @argrule Distance yy1 [PLFLT] y coord of point 1
** @argrule Distance xx2 [PLFLT] x coord of point 2
** @argrule Distance yy2 [PLFLT] y coord of point 2
** @argrule Range array [const float*] array
** @argrule Range npoints [ajuint] Number of data points
** @argrule Range min [float*] min. value in array
** @argrule Range max [float*] max. value in array
** @argrule [r] C txt [const char*] Text
** @argrule [r] S str [const AjPStr] Text
**
** @valrule * [void]
** @valrule *Charsize [PLFLT] The character size (in mm) that fits the specified
**                 height and length.
** @valrule *Coord [PLFLT*] The x and y coordinates of the point
** @valrule *Distance [PLFLT] Distance between 2 points in user coordinates
** @valrule *Textheight [PLFLT] Text height (in mm)
** @valrule *Textlength [PLFLT] Text length (in mm)
**
******************************************************************************/




/* @func ajGraphicsCalcCharsize ***********************************************
**
** Computes the character size (in mm) needed to write a text string
** with specified height and length (in user coord). The length of the
** string is the distance between (xx1,yy1) and (xx2,yy2); its height is
** TextHeight.  If the default size is too large, characters are
** shrunk. If it is too small, characters are enlarged.
**
** @param [r] xx1 [PLFLT] xx1 coord.
** @param [r] yy1 [PLFLT] yy1 coord.
** @param [r] xx2 [PLFLT] xx2 coord.
** @param [r] yy2 [PLFLT] yy2 coord.
** @param [r] text [const char*] The text to be displayed.
** @param [r] TextHeight [PLFLT] The height of the text (in user coord).
** @return [PLFLT] The character size (in mm) that fits the specified
**                 height and length.
**
** @@
******************************************************************************/

PLFLT ajGraphicsCalcCharsize(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
			   const char *text,
			   PLFLT TextHeight)
{
    PLFLT origStringHeight;
    PLFLT origStringLength;
    PLFLT distpts;
    PLFLT origcharsize;
    PLFLT origcharht;
    PLFLT newcharsize;
    PLFLT wx1 = 0.0;
    PLFLT wx2 = 0.0;
    PLFLT wy1 = 0.0;
    PLFLT wy2 = 0.0;
    PLINT win = 0;

    plcalc_world(xx1, yy1, &wx1, &wy1, &win);
    plcalc_world(xx2, yy2, &wx2, &wy2, &win);

    /* adjust character height */

    ajGraphicsGetCharsize(&origcharsize,&origcharht);

    origStringHeight = currentcharsize;

    newcharsize = origcharsize * TextHeight/origStringHeight;
    ajGraphicsSetCharsize(newcharsize);

    /* Now we have to check the text fits the available space */

    /* adjust character width */
    distpts = ajGraphicsCalcDistance(xx1, yy1, xx2, yy2);
    origStringLength = ajGraphicsCalcTextlengthC(text);

    if(origStringLength > distpts)
        newcharsize = newcharsize * distpts/origStringLength;

    ajDebug("ajGraphicsCalcCharsize returns charsize %f\n", newcharsize);
    ajGraphicsSetCharsize(origcharsize);

    return newcharsize;
}




/* @obsolete ajGraphFitTextAtline
** @rename ajGraphicsCalcCharsize
*/
__deprecated PLFLT ajGraphFitTextAtline(PLFLT xx1, PLFLT yy1,
                                        PLFLT xx2, PLFLT yy2,
                                        const char *text,
                                        PLFLT TextHeight)
{
    return ajGraphicsCalcCharsize(xx1,yy1,xx2,yy2,text,TextHeight);
}




/* @func ajGraphicsCalcCoord ***************************************************
**
** compute the coordinates of a point on a circle knowing the angle.
**
** @param  [r] x [PLFLT] x coord for centre.
** @param  [r] y [PLFLT] y coord for centre.
** @param  [r] radius  [PLFLT] Radius of the circle.
** @param  [r] angle [PLFLT] angle at which the point is.
** @return [PLFLT*] The x and y coordinates of the point.
** @@
******************************************************************************/

PLFLT* ajGraphicsCalcCoord(PLFLT x, PLFLT y, PLFLT radius, PLFLT angle)
{
    PLFLT *xy;

    xy = (float *)AJALLOC( 2*sizeof(float) );
    xy[0] = x + ( radius*(float)cos(ajCvtDegToRad(angle)) );
    xy[1] = y + ( radius*(float)sin(ajCvtDegToRad(angle)) );

    return xy;
}




/* @obsolete ajComputeCoord
** @rename ajGraphicsCalcCoord
*/
__deprecated PLFLT* ajComputeCoord(PLFLT xcentre, PLFLT ycentre,
                                   PLFLT Radius, PLFLT Angle)
{
    return ajGraphicsCalcCoord(xcentre, ycentre, Radius, Angle);
}




/* @func ajGraphicsCalcDistance ***********************************************
**
** Compute the distance between 2 points in user coordinates.
**
** @param [r] xx1 [PLFLT] x coord of point 1
** @param [r] yy1 [PLFLT] y coord of point 1
** @param [r] xx2 [PLFLT] x coord of point 2
** @param [r] yy2 [PLFLT] y coord of point 2
**
** @return [PLFLT] The distance between the 2 points in user coordinates.
** @@
******************************************************************************/

PLFLT ajGraphicsCalcDistance(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2)
{
    return GraphDistPts(xx1, yy1, xx2, yy2);
}




/* @obsolete ajGraphDistPts
** @rename ajGraphicsCalcDistance
*/
__deprecated PLFLT ajGraphDistPts(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2)
{
    return GraphDistPts(xx1, yy1, xx2, yy2);
}




/* @func ajGraphicsCalcRange **************************************************
**
** Get the max and min of the data points you wish to display.
**
** @param [r] array [const float*] array
** @param [r] npoints [ajuint] Number of data points
** @param [w] min [float*]  min.
** @param [w] max [float*]  max.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsCalcRange(const float *array,
			ajuint npoints, float *min, float *max)
{
    ajuint i;

    *min = FLT_MAX;
    *max = FLT_MIN;

    for(i=0;i<npoints;++i)
    {
	*min = (*min < array[i]) ? *min : array[i];
	*max = (*max > array[i]) ? *max : array[i];
    }

    if(*min == *max)
    {
        *min *= (float) 0.9;
        *max *= (float) 1.1;
    }

    return;
}




/* @obsolete ajGraphArrayMaxMin
** @rename ajGraphicsCalcRange
*/
__deprecated void ajGraphArrayMaxMin(const float *array,
                                     ajint npoints, float *min, float *max)
{
    ajGraphicsCalcRange(array, npoints, min, max);
    return;
}




/* @func ajGraphicsCalcTextheight *********************************************
**
** Compute the height of a character in millimetres.
**
** @return [PLFLT] The height of the character in user coordinates.
** @@
******************************************************************************/

PLFLT ajGraphicsCalcTextheight(void)
{
    return currentcharsize;
}




/* @obsolete ajGraphTextHeight
** @rename ajGraphicsCalcTextheight
*/
__deprecated PLFLT ajGraphTextHeight(PLFLT xx1, PLFLT yy1,
                                     PLFLT xx2, PLFLT yy2)
{
    (void) xx1;
    (void) yy1;
    (void) xx2;
    (void) yy2;
    return currentcharsize;
}




/* @func ajGraphicsCalcTextlengthC *********************************************
**
** Compute the length of a string in millimetres.
**
** @param [r] txt [const char*] Text
**
** @return [PLFLT] The length of the string in user coordinates.
** @@
******************************************************************************/

PLFLT ajGraphicsCalcTextlengthC(const char *txt)
{
    return GraphTextLength(txt);
}




/* @func ajGraphicsCalcTextlengthS *********************************************
**
** Compute the length of a string in millimetres.
**
** @param [r] str [const AjPStr] Text
**
** @return [PLFLT] The length of the string in user coordinates.
** @@
******************************************************************************/

PLFLT ajGraphicsCalcTextlengthS(const AjPStr str)
{
    return GraphTextLength(MAJSTRGETPTR(str));
}




/* @obsolete ajGraphTextLength
** @rename ajGraphicsCalcTextlengthC
*/
__deprecated PLFLT ajGraphTextLength(PLFLT xx1, PLFLT yy1, PLFLT xx2, PLFLT yy2,
                                     const char *text)
{
    (void) xx1;
    (void) yy1;
    (void) xx2;
    (void) yy2;
    return GraphTextLength(text);
}




/* @section unused ************************************************************
**
** @fdata [none]
**
** @nam3rule Unused Contains dummy calls to unused functions to keep
**                 compilers happy
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajGraphicsUnused ******************************************************
**
** Unused functions to avoid compiler warnings
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsUnused(void)
{
    float f = 0.0;
    ajint i = 0;
    AjPGraph thys = NULL;
    AjPGraphdata graphdata = NULL;

    GraphPrint(thys);
    GraphDataPrint(graphdata);
    GraphCheckPoints(0, &f, &f);
    GraphCheckFlags(0);
    GraphArrayGapsI(0, &i, &i);

    return;
}




/* @section exit **************************************************************
**
** Functions called on exit
**
** @fdata [none]
**
** @nam3rule Exit Cleanup of internals when program exits
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajGraphicsExit ********************************************************
**
** Cleans up graphics internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsExit(void)
{
    ajStrDel(&graphBasename);
    ajStrDel(&graphExtension);

    return;
}




/* @datasection [AjPGraph] Graph object ****************************************
**
** Function is for manipulating an AjPGraph general graph object
**
** @nam2rule Graph
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Construct a new graph object to be populated by other functions
**
** @fdata [AjPGraph]
** @fcategory new
**
** @nam3rule New
**
** @valrule * [AjPGraph] New graph object
**
******************************************************************************/




/* @func ajGraphNew ***********************************************************
**
** Create a structure to hold a general graph.
**
** @return [AjPGraph] multiple graph structure.
** @@
******************************************************************************/

AjPGraph ajGraphNew(void)
{
    AjPGraph graph;

    AJNEW0(graph);
    ajDebug("ajGraphNew - need to call ajGraphSet\n");

    return graph;
}




/* @section Opening ***********************************************************
**
** Opens a plot page
**
** @fdata [AjPGraph]
** @fcategory modify
**
** @nam3rule Init Initialise a graph object
** @nam4rule InitSeq Initialises using default values based on a sequence.
** @nam3rule Open Open a plot
** @nam3rule Newpage Start a new plot page
** @nam4rule OpenFlags Open a plot with PLPLOT flag bits
** @nam4rule OpenMm Open a window whose view is defined by x and y's
**                  size in millimetres**
** @nam4rule OpenPlotset Open a plot window with a set number of plots
** @nam4rule OpenWin Open a window whose view is defined by x and y's
**                   min and max.
** @argrule * thys [AjPGraph] Graph object
** @argrule Newpage resetdefaults [AjBool] reset page setting if true
** @argrule OpenWin xmin [float] minimum x value.(user coordinates)
** @argrule OpenWin xmax [float] maximum x value.
** @argrule OpenWin ymin [float] minimum y value.
** @argrule OpenWin ymax [float] maximum y value.
** @argrule OpenFlags xmin [PLFLT] minimum x value.(user coordinates)
** @argrule OpenFlags xmax [PLFLT] maximum x value.
** @argrule OpenFlags ymin [PLFLT] minimum y value.
** @argrule OpenFlags ymax [PLFLT] maximum y value.
** @argrule OpenFlags flags [ajint] flag bit settings
** @argrule OpenPlotset numofsets [ajuint] number of plots in set.
** @argrule Mm xmm [float*] x length in millimetres
** @argrule Mm ymm [float*] y length in millimetres
** @argrule ajGraphInitSeq seq [const AjPSeq] Sequence
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphInitSeq *******************************************************
**
** Initialises a graph using default values based on a sequence.
**
** Existing titles and other data are unchanged
**
** @param [u] thys [AjPGraph] Graph
** @param [r] seq [const AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajGraphInitSeq(AjPGraph thys, const AjPSeq seq)
{
    if(!ajStrGetLen(thys->title))
        ajFmtPrintS(&thys->title, "%S of %S",
                    ajUtilGetProgram(), ajSeqGetNameS(seq));

    ajGraphxySetXrangeII(thys, 1, ajSeqGetLen(seq));

    return;
}




/* @func ajGraphNewpage *******************************************************
**
** Clear Screen if (X) or new page if plotter/postscript. Also pass a boolean
** to state whether you want the current pen colour character sizes,
** plot title and subtitle, etc to be reset or stay the same for the next page.
**
** @param [w] thys [AjPGraph] Graph object
** @param [r] resetdefaults [AjBool] reset page setting?
** @return [void]
** @@
******************************************************************************/

void ajGraphNewpage(AjPGraph thys, AjBool resetdefaults)
{
    ajint old;
    ajint cold;
    float fold;


    ajDebug("ajGraphNewPage reset:%B\n", resetdefaults);

    if(graphData)
    {
	GraphDatafileNext();
	return;
    }

    GraphSubPage(0);

    if(resetdefaults)
    {
	ajGraphicsSetFgcolour(BLACK);
	ajGraphicsSetCharscale(1.0);
	ajGraphicsSetLinestyle(0);
    }
    else
    {
	/* pladv resets every thing so need */
	/* to get the old copies */
	cold = ajGraphicsSetFgcolour(BLACK);
	fold = ajGraphicsSetCharscale(0.0);
	old = ajGraphicsSetLinestyle(0);
	ajGraphTrace(thys);

	GraphLabelTitle(((thys->flags & AJGRAPH_TITLE) ?
                         ajStrGetPtr(thys->title) : " "),
                        ((thys->flags & AJGRAPH_SUBTITLE) ?
                         ajStrGetPtr(thys->subtitle) : " "));

        if(thys->windowset)
            GraphSetWin(thys->xstart, thys->xend,
                        thys->ystart, thys->yend);
        else
            GraphSetWin(0.0, 480.0,
                        0.0, 640.0);

	/* then set it again */
	ajGraphicsSetFgcolour(cold);
	ajGraphicsSetCharscale(fold);
	ajGraphicsSetLinestyle(old);
    }

    return;
}




/* @obsolete ajGraphNewPage
** @rename ajGraphNewpage
*/
__deprecated void ajGraphNewPage(AjPGraph thys, AjBool resetdefaults)
{
    ajGraphNewpage(thys, resetdefaults);
    return;
}




/* @func ajGraphOpenFlags *****************************************************
**
** Open a window whose view is defined by the x's and y's min and max
** values.
**
** @param [w] thys [AjPGraph] Graph object
** @param [r] xmin [PLFLT] minimum x value.(user coordinates)
** @param [r] xmax [PLFLT] maximum x value.
** @param [r] ymin [PLFLT] minimum y value.
** @param [r] ymax [PLFLT] maximum y value.
** @param [r] flags [ajint] flag bit settings
**
** @return [void]
** @@
******************************************************************************/

void ajGraphOpenFlags(AjPGraph thys, PLFLT xmin, PLFLT xmax,
                      PLFLT ymin, PLFLT ymax, ajint flags)
{
    AjPTime ajtime;

    ajGraphicsSetDevice(thys);

    /* Calling funclist graphType() */
    if(thys)
        graphType[thys->displaytype].GOpen(thys,
                                       graphType[thys->displaytype].Ext);

    if( ajStrGetLen(thys->title) <=1)
    {
        ajtime = ajTimeNewToday();
        ajStrAppendC(&thys->title,
                     ajFmtString("%S (%D)",ajUtilGetProgram(),ajtime));
        ajTimeDel(&ajtime);
    }

    GraphSetName(thys, thys->outputfile,
                 graphType[thys->displaytype].Ext);
    thys->xstart = xmin;
    thys->xend = xmax;
    thys->ystart = ymin;
    thys->yend = ymax;

    GraphColourBack();
    GraphInit(thys);
    GraphColourFore();
    ajGraphicsPlenv(xmin, xmax , ymin, ymax , flags);

    return;
}




/* @obsolete ajGraphOpen
** @rename ajGraphOpenFlags
*/
__deprecated void ajGraphOpen(AjPGraph thys, PLFLT xmin, PLFLT xmax,
                              PLFLT ymin, PLFLT ymax, ajint flags)
{
    ajGraphOpenFlags(thys, xmin, xmax, ymin, ymax, flags);
    return;
}




/* @func ajGraphOpenMm ******************************************************
**
** Open a window whose view is defined by x and y's size in millimetres
** to allow easy scaling of characters and strings whose sizes are always
** in millimetres.
**
** @param [w] thys [AjPGraph] Graph object
** @param [w] xmm [float*] x length in millimetres
** @param [w] ymm [float*] y length in millimetres
**
** @return [void]
** @@
******************************************************************************/

void ajGraphOpenMm(AjPGraph thys, float * xmm, float * ymm)
{
    PLFLT mmxmin;
    PLFLT mmxmax;
    PLFLT mmymin;
    PLFLT mmymax;

    AjPTime ajtime;

    ajGraphicsSetDevice(thys);

    if (thys)
    {
	/* Calling funclist graphType() */
	graphType[thys->displaytype].GOpen(thys,
			     graphType[thys->displaytype].Ext);

	if(!ajStrGetLen(thys->title))
	{
	    ajFmtPrintAppS(&thys->title,"%S",
			   ajUtilGetProgram());
	}

	if(!ajStrGetLen(thys->subtitle))
	{
	    ajtime = ajTimeNewTodayFmt("report");
	    ajFmtPrintAppS(&thys->subtitle,"%D",
			   ajtime);
	    ajTimeDel(&ajtime);
	}
    }
    GraphColourBack();
    GraphInit(thys);
    GraphColourFore();
    GraphSubPage(0);

    if(graphData)
    {
	*xmm= (float) 255.9;
	*ymm= (float) 191.9;
    }
    else
    {
        plgspa(&mmxmin, &mmxmax, &mmymin, &mmymax);
        *xmm = mmxmax - mmxmin;
        *ymm = mmymax - mmymin;
    }

    GraphLabelTitle(((thys->flags & AJGRAPH_TITLE) ?
                     ajStrGetPtr(thys->title) : ""),
                    ((thys->flags & AJGRAPH_SUBTITLE) ?
                     ajStrGetPtr(thys->subtitle) : ""));
    thys->xstart = 0.0;
    thys->xend = *xmm;
    thys->ystart = 0.0;
    thys->yend = *ymm;
    thys->windowset = ajTrue;

    GraphSetWin(thys->xstart, thys->xend,
                thys->ystart, thys->yend);

    return;
}




/* @func ajGraphOpenPlotset ***************************************************
**
** Open a window with a set of plots
**
** @param [w] thys [AjPGraph] Graph object.
** @param [r] numofsets [ajuint] number of plots in set.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphOpenPlotset(AjPGraph thys, ajuint numofsets)
{
    ajGraphicsSetDevice(thys);
    GraphSetNumSubPage(numofsets);
    GraphColourBack();
    GraphInit(thys);
    GraphColourFore();

    return;
}




/* @obsolete ajGraphOpenPlot
** @rename ajGraphOpenPlotset
*/
__deprecated void ajGraphOpenPlot(AjPGraph thys, ajuint numofsets)
{
    ajGraphOpenPlotset(thys, numofsets);
    return;
}




/* @func ajGraphOpenWin *******************************************************
**
** Open a window whose view is defined by x and y's min and max.
**
** @param [w] thys [AjPGraph] Graph object
** @param [r] xmin [float] minimum x value.(user coordinates)
** @param [r] xmax [float] maximum x value.
** @param [r] ymin [float] minimum y value.
** @param [r] ymax [float] maximum y value.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphOpenWin(AjPGraph thys, float xmin, float xmax,
		    float ymin, float ymax)
{
    AjPTime ajtime;

    ajGraphicsSetDevice(thys);

    if (thys)
    {
	/* Calling funclist graphType() */
	graphType[thys->displaytype].GOpen(thys,
			     graphType[thys->displaytype].Ext);

	if(!ajStrGetLen(thys->title))
	{
	    ajFmtPrintAppS(&thys->title,"%S",
			   ajUtilGetProgram());
	}

	if(!ajStrGetLen(thys->subtitle))
	{
	    ajtime = ajTimeNewTodayFmt("report");
	    ajFmtPrintAppS(&thys->subtitle,"%D",
			   ajtime);
	    ajTimeDel(&ajtime);
	}
    }
    GraphColourBack();
    GraphInit(thys);
    GraphColourFore();
    GraphSubPage(0);

    GraphLabelTitle(((thys->flags & AJGRAPH_TITLE) ?
                     ajStrGetPtr(thys->title) : ""),
                    ((thys->flags & AJGRAPH_SUBTITLE) ?
                     ajStrGetPtr(thys->subtitle) : ""));
    thys->xstart = xmin;
    thys->xend = xmax;
    thys->ystart = ymin;
    thys->yend = ymax;
    thys->windowset = ajTrue;

    GraphSetWin(xmin, xmax, ymin, ymax);

    return;
}




/* @section Modifiers *********************************************************
**
** Modifying a general graph object
**
** @fdata [AjPGraph]
** @fcategory modify
**
** @nam3rule Append Add to end of an internal string
** @nam3rule Clear  Clear and reset all internal values
** @nam3rule Set Set internal value(s)
** @nam3rule Show Set flag to show element of graph
**
** @nam4rule Desc Plot description
** @nam4rule Devicetype Plplot device by name
** @nam4rule Flag Set or unset a plplot flag
** @nam4rule Multi Set graph to store multiple graphs
** @nam4rule Outdir Output directory
** @nam4rule Outfile Output filename
** @nam4rule Subtitle Graph subtitle
** @nam4rule Title Graph title
** @nam4rule Xlabel Title for the X axis
** @nam4rule Ylabel Title for the Y axis
** @suffix C C character string data
** @suffix S String object data
**
** @argrule * thys [AjPGraph] Graph object
** @argrule Multi numsets [ajuint] Maximum number of graphs that can be stored
** @argrule Show set [AjBool] True to display, false to hide
** @argrule Devicetype devicetype [const AjPStr] AJAX device name
** @argrule Flag flag [ajint] Plplot flag (one bit set)
** @argrule Flag set [AjBool] Set the flag if true, unset it if false
** @argrule C txt [const char*] C character string
** @argrule S str [const AjPStr] String object
**
** @valrule * [void]
** @valrule *Devicetype [AjBool] True on success
*
******************************************************************************/




/* @func ajGraphAppendTitleC **************************************************
**
** Set an empty graph title from the description or program name and extra text
** 'of ...' supplied by the caller.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] txt [const char*]  Title.
** @return [void]
** @@
******************************************************************************/

void ajGraphAppendTitleC(AjPGraph thys, const char* txt)
{
    ajDebug("ajGraphAppendTitleC '%s'\n", txt);

    if (!thys)
	return;

    if(ajStrGetLen(thys->title))
	return;

    if(ajStrGetLen(thys->desc))
	ajFmtPrintS(&thys->title, "%S of %s",
                    thys->desc, txt);
    else
	ajFmtPrintS(&thys->title, "%S of %s",
                    ajUtilGetProgram(), txt);

    return;
}




/* @func ajGraphAppendTitleS **************************************************
**
** Set an empty graph title from the description or program name and extra text
** 'of ...' supplied by the caller.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] str [const AjPStr]  Title.
** @return [void]
** @@
******************************************************************************/

void ajGraphAppendTitleS(AjPGraph thys, const AjPStr str)
{
    ajDebug("ajGraphAppendTitleS '%S'\n", str);

    if (!thys)
	return;

    if(ajStrGetLen(thys->title))
	return;

    if(ajStrGetLen(thys->desc))
	ajFmtPrintS(&thys->title, "%S of %S",
                    thys->desc, str);
    else
	ajFmtPrintS(&thys->title, "%S of %S",
                    ajUtilGetProgram(), str);

    return;
}




/* @obsolete ajGraphSetTitlePlus
** @rename ajGraphAppendTitle
*/
__deprecated void ajGraphSetTitlePlus(AjPGraph thys, const AjPStr title)
{
    ajGraphAppendTitleS(thys, title);
    return;
}




/* @func ajGraphClear **************************************************
**
**  Delete all the drawable objects connected to the graph object.
**
** @param [u] thys [AjPGraph] Graph object
**
** @return [void]
** @@
******************************************************************************/

void ajGraphClear(AjPGraph thys)
{
    AjPGraphobj Obj;
    AjPGraphobj anoth;

    if(!thys->Mainobj)
	return;
    else
    {
	Obj = thys->Mainobj;

	while(Obj)
	{
	    anoth = Obj->next;
	    ajStrDel(&Obj->text);
	    AJFREE(Obj);
	    Obj = anoth;
	}
    }

    thys->Mainobj = NULL;
    thys->numofobjects = 0;

    return;
}




/* @func ajGraphSetDescC *******************************************************
**
** Set the graph description.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] txt [const char*]  Title.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphSetDescC(AjPGraph thys, const char* txt)
{
    ajDebug("ajGraphSetDescC '%s'\n", txt);

    ajStrAssignC(&thys->desc,txt);

    return;
}




/* @func ajGraphSetDescS *******************************************************
**
** Set the graph description.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] str [const AjPStr]  Title.
**
** @return [void]
** @@
******************************************************************************/

void ajGraphSetDescS(AjPGraph thys, const AjPStr str)
{
    ajDebug("ajGraphSetDescS '%S'\n", str);

    ajStrAssignS(&thys->desc,str);

    return;
}




/* @obsolete ajGraphSetDesc
** @rename ajGraphSetDescS
*/

__deprecated void ajGraphSetDesc(AjPGraph thys, const AjPStr title)
{
    ajGraphSetDescS(thys, title);

    return;
}




/* @func ajGraphSetDevicetype *************************************************
**
** Initialize options for a general graph object
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] devicetype [const AjPStr] Graph type
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajGraphSetDevicetype(AjPGraph thys, const AjPStr devicetype)
{
    ajint i;
    ajint j=-1;
    ajint k;
    AjPStr aliases = NULL;

    ajDebug("ajGraphSetDevicetype '%S'\n", devicetype);

    for(i=0;graphType[i].Name;i++)
    {
        if(!graphType[i].GOpen)
            continue;

        if(ajStrMatchCaseC(devicetype, graphType[i].Name))
	{
	    j = i;
	    break;
	}

	if(ajCharPrefixCaseS(graphType[i].Name, devicetype))
	{
	    if(j < 0)
		j = i;
	    else
	    {
		for(k=0;graphType[k].Name;k++)
		{
		    if(ajCharPrefixCaseS(graphType[k].Name, devicetype))
		    {
			if(ajStrGetLen(aliases))
			    ajStrAppendC(&aliases, ", ");
			ajStrAppendC(&aliases, graphType[k].Name);
		    }
		}
		    
		ajErr("Ambiguous graph device name '%S' (%S)",
		       devicetype, aliases);
		ajStrDel(&aliases);

		return ajFalse;
	    }
	}
    }

    if(j<0)
	return ajFalse;

    GraphNewPlplot(thys);

    thys->displaytype = j;

    ajDebug("ajGraphSetDevicetype plplot type '%S' displaytype %d '%s'\n",
	    devicetype, j, graphType[j].Name);

    return ajTrue;
}




/* @obsolete ajGraphSet
** @rename ajGraphSetDevicetype
*/
__deprecated AjBool ajGraphSet(AjPGraph thys, const AjPStr type)
{
    return ajGraphSetDevicetype(thys, type);
}




/* @func ajGraphSetFlag *****************************************************
**
** Set the flags for the graph to add or subtract "flag" depending on set
**
** @param [u] thys [AjPGraph] graph to have flags altered.
** @param [r] flag   [ajint]  flag to be set.
** @param [r] set [AjBool]    whether to set the flag or remove it.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetFlag(AjPGraph thys, ajint flag, AjBool set)
{
    if (!thys)
	return;

    if(thys->flags & flag)
    {
	if(!set)
                thys->flags &= ~flag;
    }
    else
    {
	if(set)
	    thys->flags |= flag;
    }

    return;
}




/* @func ajGraphSetMulti ******************************************************
**
** Create a structure to hold a number of graphs.
**
** @param [w] thys [AjPGraph] Graph structure to store info in.
** @param [r] numsets [ajuint] maximum number of graphs that can be stored.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetMulti(AjPGraph thys, ajuint numsets)
{
    if (!thys)
	return;

    if (thys->graphs)
        AJFREE(thys->graphs);

    AJCNEW0(thys->graphs,numsets);

    ajDebug("ajGraphSetMulti numsets: %d\n", numsets);

    thys->numofgraphs    = 0;
    thys->numofgraphsmax = numsets;
    thys->minmaxcalc     = 0;
    thys->flags          = GRAPH_XY;

    return;
}




/* @func ajGraphSetOutdirS **************************************************
**
** Set the directory of the output file. Only used later if the device
** plotter is capable of writing to a file (e.g. postscript or png)
**
** @param [u] thys [AjPGraph] Graph structure to update file name
** @param [r] str [const AjPStr] Name of the file.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetOutdirS(AjPGraph thys, const AjPStr str)
{
    if(!ajStrGetLen(str))
	return;

    if (!thys)
	return;

    if(ajStrGetCharLast(str) != '/')
	ajStrInsertC(&thys->outputfile, 0, "/");
    ajStrInsertS(&thys->outputfile, 0, str);

    return;
}




/* @obsolete ajGraphSetDir
** @rename ajGraphSetOutdirS
*/

__deprecated void ajGraphSetDir(AjPGraph thys, const AjPStr txt)
{
    ajGraphSetOutdirS(thys, txt);
    return;
}




/* @func ajGraphSetOutfileC ***************************************************
**
** Set the name of the output file. Only used later if the device
** plotter is capable of postscript output. ps and cps.
**
** @param [u] thys [AjPGraph] Graph structure to write file name to.
** @param [r] txt [const char*] Name of the file.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetOutfileC(AjPGraph thys, const char* txt)
{
    if(!strlen(txt))
	return;

    if (!thys)
	return;

    ajStrAssignC(&thys->outputfile, txt);

    return;
}




/* @obsolete ajGraphSetOutC
** @rename ajGraphSetOutfileC
*/
__deprecated void ajGraphSetOutC(AjPGraph thys, const char* txt)
{
    ajGraphSetOutfileC(thys, txt);
    return;
}




/* @func ajGraphSetOutfileS ***************************************************
**
** Set the name of the output file. Only used later if the device
** plotter is capable of postscript output. ps and cps.
**
** @param [u] thys [AjPGraph] Graph structure to write file name to.
** @param [r] str [const AjPStr] Name of the file.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetOutfileS(AjPGraph thys, const AjPStr str)
{
    if(!ajStrGetLen(str))
	return;

    if (!thys)
	return;

    ajStrAssignS(&thys->outputfile, str);

    return;
}




/* @obsolete ajGraphSetOut
** @rename ajGraphSetOutfileS
*/
__deprecated void ajGraphSetOut(AjPGraph thys, const AjPStr txt)
{

    ajGraphSetOutfileS(thys, txt);
    return;
}




/* @func ajGraphSetSubtitleC **************************************************
**
** Set the graph subtitle.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] txt [const char*]  Subtitle.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetSubtitleC(AjPGraph thys, const char* txt)
{
    ajStrAssignC(&thys->subtitle,txt);

    return;
}




/* @obsolete ajGraphSetSubTitleC
** @rename ajGraphSetSubtitleC
*/

__deprecated void ajGraphSetSubTitleC(AjPGraph thys, const char* title)
{
    ajGraphSetSubtitleC(thys, title);
    return;
}




/* @func ajGraphSetSubtitleS ***************************************************
**
**  Set the title for the Y axis.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] str [const AjPStr] Subtitle
** @return [void]
** @@
******************************************************************************/

void ajGraphSetSubtitleS(AjPGraph thys, const AjPStr str)
{
    ajStrAssignS(&thys->subtitle,str);

    return;
}




/* @obsolete ajGraphSetSubTitle
** @rename ajGraphSetSubtitleS
*/

__deprecated void ajGraphSetSubTitle(AjPGraph thys, const AjPStr title)
{
    ajGraphSetSubtitleS(thys, title);
    return;
}




/* @func ajGraphSetTitleC ***************************************************
**
** Set the graph Title.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] txt [const char*]  title.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetTitleC(AjPGraph thys, const char* txt)
{
    ajDebug("ajGraphSetTitleC '%s'\n", txt);

    ajStrAssignC(&thys->title,txt);

    return;
}




/* @func ajGraphSetTitleS ******************************************************
**
** Set the graph title.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] str [const AjPStr]  Title.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetTitleS(AjPGraph thys, const AjPStr str)
{
    ajDebug("ajGraphSetTitleS '%S'\n", str);

    ajStrAssignS(&thys->title,str);

    return;
}




/* @obsolete ajGraphSetTitle
** @rename ajGraphSetTitleS
*/

__deprecated void ajGraphSetTitle(AjPGraph thys, const AjPStr title)
{
    ajGraphSetTitleS(thys, title);
    return;
}




/* @func ajGraphSetXlabelC ****************************************************
**
** Set the title for the X axis for multiple plot on one graph.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] txt [const char*] title for the x axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetXlabelC(AjPGraph thys, const char* txt)
{
    ajStrAssignC(&thys->xaxis,txt);

    return;
}




/* @obsolete ajGraphSetXTitleC
** @rename ajGraphSetXlabelC
*/

__deprecated void ajGraphSetXTitleC(AjPGraph thys, const char* title)
{
    ajGraphSetXlabelC(thys, title);
    return;
}




/* @func ajGraphSetXlabelS *****************************************************
**
** Set the title for the X axis for multiple plots on one graph.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] str [const AjPStr] title for the x axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetXlabelS(AjPGraph thys, const AjPStr str)
{
    ajStrAssignS(&thys->xaxis,str);

    return;
}




/* @obsolete ajGraphSetXTitle
** @rename ajGraphSetXlabelS
*/

__deprecated void ajGraphSetXTitle(AjPGraph thys, const AjPStr title)
{
    ajGraphSetXlabelS(thys, title);
    return;
}




/* @func ajGraphSetYlabelC ****************************************************
**
** Set the title for the Y axis for multiple plots on one graph.
**
** @param [u] thys  [AjPGraph] Graph structure to store info in.
** @param [r] txt [const char*] title for the y axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetYlabelC(AjPGraph thys, const char* txt)
{
    ajStrAssignC(&thys->yaxis, txt);

    return;
}




/* @obsolete ajGraphSetYTitleC
** @rename ajGraphSetYlabelC
*/

__deprecated void ajGraphSetYTitleC(AjPGraph thys, const char* title)
{
    ajGraphSetYlabelC(thys, title);
    return;
}




/* @func ajGraphSetYlabelS *****************************************************
**
** Set the title for the Y axis for multiple plots on one graph.
**
** @param [u] thys  [AjPGraph] Graph structure to store info in.
** @param [r] str [const AjPStr] title for the y axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetYlabelS(AjPGraph thys, const AjPStr str)
{
    ajStrAssignS(&thys->yaxis, str);

    return;
}




/* @obsolete ajGraphSetYTitle
** @rename ajGraphSetYlabelS
*/

__deprecated void ajGraphSetYTitle(AjPGraph thys, const AjPStr title)
{
    ajGraphSetYlabelS(thys, title);
    return;
}




/* @func ajGraphShowSubtitle **************************************************
**
** Set whether the graph is to display the subtitle.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphShowSubtitle(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_SUBTITLE, set);

    return;
}




/* @obsolete ajGraphSetSubTitleDo
** @rename ajGraphShowSubtitle
*/

__deprecated void ajGraphSetSubTitleDo(AjPGraph thys, AjBool set)
{
    ajGraphShowSubtitle(thys, set);
    return;
}




/* @func ajGraphShowTitle *****************************************************
**
** Set whether the graph is to display the title.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphShowTitle(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_TITLE, set);

    return;
}




/* @obsolete ajGraphSetTitleDo
** @rename ajGraphShowTitle
*/

__deprecated void ajGraphSetTitleDo(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_TITLE, set);

    return;
}




/* @section Queries ***********************************************************
**
** Querying and reporting internal values
**
** @fdata [AjPGraph]
** @fcategory use
**
** @nam3rule Get Return an internal value
** @nam4rule Params Get parameters
** @nam5rule ParamsPage Get the output page parameters
** @nam4rule Subtitle Get subtitle
** @nam4rule Title    Get title
** @nam4rule Xlabel   Get x-axis label
** @nam4rule Ylabel   Get y-axis label
** @nam3rule Is       Test for presence of a property
** @nam4rule IsData   Test for graph being a data file rather than a plot
** @suffix C Return a C character string
** @suffix S return a string object
**
** @argrule * thys [const AjPGraph] Graph object
** @argrule ParamsPage xp [float*] where to store the x position
** @argrule ParamsPage yp [float*] where to store the y position
** @argrule ParamsPage xleng [ajint*] where to store the x length
** @argrule ParamsPage yleng [ajint*] where to the y length
** @argrule ParamsPage xoff [ajint*] where to store the x offset
** @argrule ParamsPage yoff [ajint*] where to store the y offset
**
** @valrule * [void]
** @valrule *C [const char*] C character string
** @valrule *S [const AjPStr] String object
** @valrule *Is [AjBool] True if the property is found
**
******************************************************************************/




/* @func ajGraphGetParamsPage **************************************************
**
** Get the output page parameters stored in a graph object
**
** For graph data type, sets to zero as these are not applicable.
**
** @param [r] thys [const AjPGraph] Graph object.
** @param [u] xp [float *] where to store the x position
** @param [u] yp [float *] where to store the y position
** @param [u] xleng [ajint *] where to store the x length
** @param [u] yleng [ajint *] where to the y length
** @param [u] xoff [ajint *] where to store the x offset
** @param [u] yoff [ajint *] where to store the y offset
**
** @return [void]
** @@
******************************************************************************/

void ajGraphGetParamsPage(const AjPGraph thys,
                          float *xp,float *yp, ajint *xleng, ajint *yleng,
                          ajint *xoff, ajint *yoff)
{
    ajDebug("=g= plgpage(&f &f) [&xp, &yp, &xleng, &yleng, &xoff, &yoff]\n");

    if(graphData)
    {
	*xp=0.0;
	*yp=0.0;
	*xleng=graphType[thys->displaytype].Width;
	*yleng=graphType[thys->displaytype].Height;
	*xoff=0;
	*yoff=0;
    }
    else
    {
	plgpage(xp,yp,xleng,yleng,xoff,yoff);
        if(!*xleng)
            *xleng = graphType[thys->displaytype].Width;
        if(!*yleng)
            *yleng = graphType[thys->displaytype].Height;
	ajDebug("returns xp:%.3f yp:%.3f xleng:%d yleng:%d xoff:%d yoff:%d\n",
		 *xp, *yp, *xleng, *yleng, *xoff, *yoff);
    }

    return;
}




/* @func ajGraphGetSubtitleC **************************************************
**
** Return plot subtitle
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const char*] Subtitle
** @@
******************************************************************************/

const char* ajGraphGetSubtitleC(const AjPGraph thys)
{
    if (thys->subtitle)
	    return ajStrGetPtr(thys->subtitle);

    return "";
}




/* @obsolete ajGraphGetSubTitleC
** @rename ajGraphGetSubtitleC
*/
__deprecated const char* ajGraphGetSubTitleC(const AjPGraph thys)
{
    return ajGraphGetSubtitleC(thys);
}




/* @func ajGraphGetSubtitleS **************************************************
**
** Return plot subtitle
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const AjPStr] Subtitle
** @@
******************************************************************************/

const AjPStr ajGraphGetSubtitleS(const AjPGraph thys)
{
    return thys->subtitle;

}




/* @obsolete ajGraphGetSubTitle
** @rename ajGraphGetSubtitleS
*/
__deprecated const AjPStr ajGraphGetSubTitle(const AjPGraph thys)
{
    return ajGraphGetSubtitleS(thys);
}




/* @func ajGraphGetTitleC *****************************************************
**
** Return plot title
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const char*] Title
** @@
******************************************************************************/

const char* ajGraphGetTitleC(const AjPGraph thys)
{
    if(thys->title)
        return ajStrGetPtr(thys->title);

    return "";
}




/* @func ajGraphGetTitleS *****************************************************
**
** Return plot title
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const AjPStr] Title
** @@
******************************************************************************/

const AjPStr ajGraphGetTitleS(const AjPGraph thys)
{
    return thys->title;
}




/* @obsolete ajGraphGetTitle
** @rename ajGraphGetTitleS
*/
__deprecated const AjPStr ajGraphGetTitle(const AjPGraph thys)
{
    return thys->title;
}




/* @func ajGraphGetXlabelC ****************************************************
**
** Return plot x-axis label
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const char*] Title of x-axis
** @@
******************************************************************************/

const char* ajGraphGetXlabelC(const AjPGraph thys)
{
    if (thys->xaxis)
        return ajStrGetPtr(thys->xaxis);

    return "";
}




/* @obsolete ajGraphGetXTitleC
** @rename ajGraphGetXlabelC
*/
__deprecated const char* ajGraphGetXTitleC(const AjPGraph thys)
{
    return ajGraphGetXlabelC(thys);
}




/* @func ajGraphGetXlabelS *****************************************************
**
** Return plot x-axis label
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const AjPStr] Title of x-axis
** @@
******************************************************************************/

const AjPStr ajGraphGetXlabelS(const AjPGraph thys)
{
    return thys->xaxis;
}




/* @obsolete ajGraphGetXTitle
** @rename ajGraphGetXlabelS
*/
__deprecated const AjPStr ajGraphGetXTitle(const AjPGraph thys)
{
    return thys->xaxis;
}




/* @func ajGraphGetYlabelC ************************************************
**
** Return plot y-axis label
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const char*] Title of y-axis
** @@
******************************************************************************/

const char* ajGraphGetYlabelC(const AjPGraph thys)
{
    if (thys->yaxis)
        return ajStrGetPtr(thys->yaxis);

    return "";
}




/* @obsolete ajGraphGetYTitleC
** @rename ajGraphGetYlabelC
*/
__deprecated const char* ajGraphGetYTitleC(const AjPGraph thys)
{
    return  ajGraphGetYlabelC(thys);
}





/* @func ajGraphGetYlabelS *****************************************************
**
** Return plot y-axis label
**
** @param [r] thys [const AjPGraph] Graph object.
** @return [const AjPStr] Title of y-axis
** @@
******************************************************************************/

const AjPStr ajGraphGetYlabelS(const AjPGraph thys)
{
    return thys->yaxis;
}




/* @obsolete ajGraphGetYTitle
** @rename ajGraphGetYlabelS
*/
__deprecated const AjPStr ajGraphGetYTitle(const AjPGraph thys)
{
    return thys->yaxis;
}





/* @func ajGraphIsData **************************************************
**
** returns ajTrue if the graph is creating a data file
**
** @param [r] thys [const AjPGraph] Graph object
** @return [AjBool] ajTrue if the graph is of type data
** @@
******************************************************************************/

AjBool ajGraphIsData(const AjPGraph thys)
{
    return thys->isdata;
}




/* @section Plotting **********************************************************
**
** Plotting drawable objects
**
** @fdata [AjPGraph]
** @fcategory use
**
** @nam3rule Add Add an object to a graph
** @nam4rule Line Line
** @nam4rule Rect Rectangle
** @nam4rule Text Text
** @nam5rule TextScale Scaled text
** @suffix C Character string input
** @suffix S String object data
**
** @argrule * thys [AjPGraph] Graph object
** @argrule Add x [float] x start position
** @argrule Add y [float] y start position
** @argrule AddLine x2 [float] x end position
** @argrule AddLine y2 [float] y end position
** @argrule AddRect x2 [float] x end position
** @argrule AddRect y2 [float] y end position
** @argrule Add colour [ajint] Colour code (see PLPLOT)
** @argrule AddRect fill [ajint] Fill code (see PLPLOT)
** @argrule Scale scale [float] Character scale (0.0 to use the default)
** @argrule C txt [const char*] Text string
** @argrule S str [const AjPStr] Text string
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphAddLine *******************************************************
**
** Add a line to be drawn when the graph is plotted.
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] x2 [float] End x position
** @param [r] y2 [float] End y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphAddLine(AjPGraph thys, float x, float y,
                    float x2, float y2, ajint colour)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	AJNEW(thys->Mainobj);
	Obj = thys->Mainobj;
    }
    else
    {
	Obj = thys->Mainobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++thys->numofobjects;

    Obj->type = LINE;
    Obj->text = 0;
    Obj->xx1 = x;
    Obj->xx2 = x2;
    Obj->yy1 = y;
    Obj->yy2 = y2;
    Obj->colour = colour;
    Obj->next = 0;

    return;
}




/* @func ajGraphAddRect ****************************************************
**
** Add a Rectangle to be drawn when the graph is plotted, fill states whether
** the rectangle should be filled in.
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] x [float] Start x coordinate
** @param [r] y [float] Start y coordinate
** @param [r] x2 [float] End x coordinate
** @param [r] y2 [float] End y coordinate
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] fill [ajint] Fill code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphAddRect(AjPGraph thys, float x, float y,
		       float x2, float y2, ajint colour, ajint fill)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	AJNEW(thys->Mainobj);
	Obj = thys->Mainobj;
    }
    else
    {
	Obj = thys->Mainobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++thys->numofobjects;


    if(fill)
	Obj->type = RECTANGLEFILL;
    else
	Obj->type = RECTANGLE;

    Obj->text = 0;
    Obj->xx1 = x;
    Obj->xx2 = x2;
    Obj->yy1 = y;
    Obj->yy2 = y2;
    Obj->colour = colour;
    Obj->next = 0;

    return;
}




/* @func ajGraphAddTextC ******************************************************
**
** Add text to be drawn when the graph is plotted.
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] txt [const char*] Text
**
** @return [void]
** @@
******************************************************************************/

void ajGraphAddTextC(AjPGraph thys, float x, float y,
		       ajint colour, const char *txt)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	AJNEW(thys->Mainobj);
	Obj = thys->Mainobj;
    }
    else
    {
	Obj = thys->Mainobj;
	while(Obj->next)
	    Obj = Obj->next;
	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++thys->numofobjects;


    Obj->type = TEXT;
    Obj->text = ajStrNewC(txt);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = 0.0;
    Obj->next = 0;

    return;
}




/* @obsolete ajGraphAddText
** @rename ajGraphAddTextC
*/
__deprecated void ajGraphAddText(AjPGraph thys, float x, float y,
		       ajint colour, const char *txt)
{
    ajGraphAddTextC(thys, x, y, colour, txt);
    return;
}




/* @func ajGraphAddTextS ******************************************************
**
** Add text to be drawn when the graph is plotted.
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] str [const AjPStr] Text
**
** @return [void]
** @@
******************************************************************************/

void ajGraphAddTextS(AjPGraph thys, float x, float y,
                     ajint colour, const AjPStr str)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	AJNEW(thys->Mainobj);
	Obj = thys->Mainobj;
    }
    else
    {
	Obj = thys->Mainobj;
	while(Obj->next)
	    Obj = Obj->next;
	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++thys->numofobjects;


    Obj->type = TEXT;
    Obj->text = ajStrNewS(str);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = 0.0;
    Obj->next = 0;

    return;
}




/* @func ajGraphAddTextScaleC *************************************************
**
** Add text to be drawn when the graph is plotted.
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] scale [float] Character scale (0.0 to use the default)
** @param [r] txt [const char*] Text
**
** @return [void]
** @@
******************************************************************************/

void ajGraphAddTextScaleC(AjPGraph thys, float x, float y,
                          ajint colour, float scale, const char *txt)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	AJNEW(thys->Mainobj);
	Obj = thys->Mainobj;
    }
    else
    {
	Obj = thys->Mainobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++thys->numofobjects;


    Obj->type = TEXT;
    Obj->text = ajStrNewC(txt);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = scale;
    Obj->next = 0;

    return;
}




/* @obsolete ajGraphAddTextScale
** @rename ajGraphAddTextScaleC
*/
__deprecated void ajGraphAddTextScale(AjPGraph thys, float xx1, float yy1,
                                      ajint colour, float scale,
                                      const char *txt)
{

    ajGraphAddTextScaleC(thys, xx1, yy1, colour, scale, txt);
    return;
}




/* @func ajGraphAddTextScaleS *************************************************
**
** Add text to be drawn when the graph is plotted.
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] scale [float] Character scale (0.0 to use the default)
** @param [r] str [const AjPStr] Text
**
** @return [void]
** @@
******************************************************************************/

void ajGraphAddTextScaleS(AjPGraph thys, float x, float y,
                          ajint colour, float scale, const AjPStr str)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	AJNEW(thys->Mainobj);
	Obj = thys->Mainobj;
    }
    else
    {
	Obj = thys->Mainobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++thys->numofobjects;


    Obj->type = TEXT;
    Obj->text = ajStrNewS(str);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = scale;
    Obj->next = 0;

    return;
}




/* @funcstatic GraphDraw ***************************************************
**
** Display the drawable objects connected to this graph.
**
** @param [r] thys [const AjPGraph] Graph object
**
** @return [void]
** @@
******************************************************************************/

static void GraphDraw(const AjPGraph thys)
{
    AjPGraphobj Obj;
    ajint temp;
    float cold = 1.0;

    /* graphdata : calls ajGraphLine etc. */

    if(!thys->Mainobj)
	return;
    else
    {
	Obj = thys->Mainobj;

	while(Obj)
	{
	    if(Obj->type == RECTANGLE)
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposRect(Obj->xx1, Obj->yy1,Obj->xx2,
                                      Obj->yy2);
		ajGraphicsSetFgcolour(temp);
	    }
	    else if(Obj->type == RECTANGLEFILL)
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposRectFill(Obj->xx1, Obj->yy1,Obj->xx2,
                                          Obj->yy2);
		ajGraphicsSetFgcolour(temp);
	    }
	    else if(Obj->type == TEXT )
	    {
		if(Obj->scale)
		    cold = ajGraphicsSetCharscale(Obj->scale);
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposTextAtstart(Obj->xx1, Obj->yy1,
                                             ajStrGetPtr(Obj->text));
		ajGraphicsSetFgcolour(temp);
		if(Obj->scale)
		    ajGraphicsSetCharscale(cold);
	    }
	    else if(Obj->type == LINE)
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposLine(Obj->xx1, Obj->yy1,Obj->xx2,
                                      Obj->yy2);
		ajGraphicsSetFgcolour(temp);
	    }
	    else
		ajUserDumpC("UNDEFINED OBJECT TYPE USED");

	    Obj = Obj->next;
	}
    }

    return;
}




/* @section Debugging **********************************************************
**
** Tracing the internal values of a graph object
**
** @fdata [AjPGraph]
** @fcategory use
**
** @nam3rule Trace Trace internals
**
** @argrule * thys [const AjPGraph] Graph object
**
** @valrule * [void]
**
******************************************************************************/




/* @funcstatic GraphPrint **************************************************
**
** Print all the drawable objects in readable form.
**
** @param [r] thys [const AjPGraph] Graph object
**
** @return [void]
** @@
******************************************************************************/

static void GraphPrint(const AjPGraph thys)
{
    AjPGraphobj Obj;

    if(!thys->Mainobj)
    {
	ajUserDumpC("No Objects");

	return;
    }
    else			   /* cycle through till NULL found */
    {
	Obj = thys->Mainobj;

	while(Obj)
	{
	    if(Obj->type == RECTANGLE )
		ajUser("type = RECTANGLE, %f %f %f %f col= %d",
		       Obj->xx1, Obj->yy1,
		       Obj->xx2,Obj->yy2,Obj->colour);
	    else if(Obj->type == RECTANGLEFILL )
		ajUser("type = RECTANGLEFILL, %f %f %f %f col= %d",
		       Obj->xx1, Obj->yy1,
		       Obj->xx2,Obj->yy2,Obj->colour);
	    else if(Obj->type == TEXT)
		ajUser("type = TEXT, %f %f col= %d %S",
		       Obj->xx1, Obj->yy1,Obj->colour,
		       Obj->text);
	    else if(Obj->type == LINE )
		ajUser("type = LINE, %f %f %f %f col= %d",
		       Obj->xx1, Obj->yy1,
		       Obj->xx2,Obj->yy2,Obj->colour);

	    Obj = Obj->next;
	}
    }

    return;
}




/* @func ajGraphTrace *********************************************************
**
** Writes debug messages to trace the contents of a graph object.
**
** @param [r] thys [const AjPGraph] Graph object
** @return [void]
** @@
******************************************************************************/

void ajGraphTrace(const AjPGraph thys)
{
    ajDebug("Graph trace\n");

    ajDebug("\n(a) True booleans\n");

    ajDebug("minmaxcalc %B\n", thys->minmaxcalc);


    ajDebug("\n(b) Strings with values\n");

    ajDebug("Desc '%S'\n", thys->desc);
    ajDebug("Title '%S'\n", thys->title);
    ajDebug("Subtitle '%S'\n", thys->subtitle);
    ajDebug("Xaxis '%S'\n", thys->xaxis);
    ajDebug("Yaxis '%S'\n", thys->yaxis);
    ajDebug("outputfile '%S'\n", thys->outputfile);

    ajDebug("\n(c) Other values\n");
    ajDebug("flags %x\n", thys->flags);
    ajDebug("numofgraphs %d\n", thys->numofgraphs);
    ajDebug("numofgraphsmax %d\n", thys->numofgraphsmax);
    ajDebug("minX   %7.3f maxX   %7.3f\n",
	    thys->minX,
	    thys->maxX);
    ajDebug("minY   %7.3f maxY   %7.3f\n",
	    thys->minY,
	    thys->maxY);
    ajDebug("xstart %7.3f xend   %7.3f\n",
	    thys->xstart,
	    thys->xend);
    ajDebug("ystart %7.3f yend   %7.3f\n",
	    thys->ystart,
	    thys->yend);
    ajDebug("displaytype %d '%s'\n", thys->displaytype,
	    graphType[thys->displaytype].Device);

    return;
}




/* @section Graph data management ********************************************
**
** Functions to manage the graphdata objects associated with a graph object
**
** @fdata [AjPGraph]
** @fcategory modify
**
** @nam3rule Data
** @nam4rule Add Add a graph data object
** @nam4rule Replace Replace graph data object
** @suffix I Index number of data for multiple graphs
**
** @argrule * thys [AjPGraph] multiple graph structure.
** @argrule * graphdata [AjPGraphdata] graph to be added.
** @argrule I num [ajuint] number within multiple graph.
**
** @valrule * [AjBool]
**
******************************************************************************/




/* @func ajGraphDataAdd ****************************************************
**
** Add another graph structure to the multiple graph structure.
**
** The graphdata now belongs to the graph - do not delete it while
** the graph is using it.
**
** @param [u] thys [AjPGraph] multiple graph structure.
** @param [u] graphdata [AjPGraphdata] graph to be added.
** @return [AjBool] True if graph added successfully
** @@
******************************************************************************/

AjBool ajGraphDataAdd(AjPGraph thys, AjPGraphdata graphdata)
{
    if(thys->numofgraphs)
    {
        ajDebug("ajGraphDataAdd multi \n");
    }

    if(thys->numofgraphs < thys->numofgraphsmax)
    {
        (thys->graphs)[thys->numofgraphs++] = graphdata;
        return ajTrue;
    }

    ajErr("Too many multiple graphs - expected %d graphs",
          thys->numofgraphsmax);

    return ajFalse;
}




/* @func ajGraphDataReplace ************************************************
**
** Replace graph structure into the multiple graph structure.
**
** The graphdata now belongs to the graph - do not delete it while
** the graph is using it.
**
** @param [u] thys [AjPGraph] multiple graph structure.
** @param [u] graphdata [AjPGraphdata] graph to be added.
** @return [AjBool] True if graph added successfully
** @@
******************************************************************************/

AjBool ajGraphDataReplace(AjPGraph thys, AjPGraphdata graphdata)
{
    ajGraphdataDel(&(thys->graphs)[0]);
    (thys->graphs)[0] = graphdata;
    thys->numofgraphs=1;

    thys->minmaxcalc = 0;

    return ajTrue;
}




/* @func ajGraphDataReplaceI ************************************************
**
** Replace one of the graph structures in the multiple graph structure.
**
** The graphdata now belongs to the graph - do not delete it while
** the graph is using it.
**
** @param [u] thys [AjPGraph] multiple graph structure.
** @param [u] graphdata [AjPGraphdata] graph to be added.
** @param [r] num [ajuint] number within multiple graph.
** @return [AjBool] True if graph added successfully
** @@
******************************************************************************/

AjBool ajGraphDataReplaceI(AjPGraph thys, AjPGraphdata graphdata, ajuint num)
{
    if(thys->numofgraphs > num) {
        ajGraphdataDel(&(thys->graphs)[num]);
        (thys->graphs)[num] = graphdata;
    }
    else if(thys->numofgraphs < thys->numofgraphsmax)
    {
        (thys->graphs)[thys->numofgraphs++] = graphdata;
    }
    else
    {
        return ajFalse;
    }

    thys->minmaxcalc = 0;

    return ajTrue;
}




/* @datasection [AjPGraph] Graph object ****************************************
**
** Function is for manipulating an AjPGraph object for an XY graph
**
** @nam2rule Graphxy
*/




/* @section Constructors ******************************************************
**
** Construct a new graph object to be populated by other functions
**
** @fdata [AjPGraph]
** @fcategory new
**
** @nam3rule New
** @suffix I Number of graphs
**
** @argrule I numsets [ajuint] maximum number of graphs that can be stored
** @valrule * [AjPGraph] New graph object
**
******************************************************************************/




/* @func ajGraphxyNewI ********************************************************
**
** Create a structure to hold a number of graphs.
**
** @param [r] numsets [ajuint] maximum number of graphs that can be stored.
** @return [AjPGraph] multiple graph structure.
** @@
******************************************************************************/

AjPGraph ajGraphxyNewI(ajuint numsets)
{
    AjPGraph ret;

    AJNEW0(ret);
    ret->numsets = numsets;

    ajDebug("ajGraphxyNewI numsets: %d\n", numsets);

    return ret;
}




/* @section Destructors *******************************************************
**
** Destructors for AjGraph XY graph data
**
** @fdata [AjPGraph]
** @fcategory delete
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPGraph*] Graph structure to store info in.
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphxyDel *********************************************************
**
** Destructor for a graph object
**
** @param [w] pthis [AjPGraph*] Graph structure to store info in.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDel(AjPGraph* pthis)
{
    AjPGraphdata graphdata;
    AjPGraph thys;
    ajuint i;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    thys = *pthis;

    ajDebug("ajGraphxyDel numofgraphs:%d\n",
            thys->numofgraphs);

    for(i = 0 ; i < thys->numofgraphs ; i++)
    {
        graphdata = (thys->graphs)[i];

        if (graphdata)
        {
            ajDebug("ajGraphxyDel graphs[%d] xcalc:%B ycalc:%B x:%x y:%x\n",
                    i, graphdata->xcalc, graphdata->ycalc,
                    graphdata->x, graphdata->y);

            if(!graphdata->xcalc)
                AJFREE(graphdata->x);
            if(!graphdata->ycalc)
                AJFREE(graphdata->y);
            if(!graphdata->gtype)
                ajStrDel(&graphdata->gtype);
            ajGraphdataDel(&graphdata);
        }
    }

    ajStrDel(&thys->desc);
    ajStrDel(&thys->title);
    ajStrDel(&thys->subtitle);
    ajStrDel(&thys->xaxis);
    ajStrDel(&thys->yaxis);
    ajStrDel(&thys->outputfile);

    ajGraphClear(thys);

    AJFREE(thys->graphs);

    AJFREE(thys);

    *pthis = NULL;

    return;
}




/* @section Display ***********************************************************
**
** Functions to display or write the graph
**
** @fdata [AjPGraph]
** @fcategory modify
**
** @nam3rule Display Display the graph
**
** @argrule Display thys [AjPGraph] Multiple graph pointer.
** @argrule Display closeit [AjBool] Whether to close graph at the end.
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphxyDisplay *****************************************************
**
** A general routine for drawing graphs.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool]   Whether to close graph at the end.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDisplay(AjPGraph thys, AjBool closeit)
{
    /* Calling funclist graphType() */
    (graphType[thys->displaytype].XYDisplay)
	(thys, closeit, graphType[thys->displaytype].Ext);

    return;
}




/* @section Modifiers *********************************************************
**
** Set or modify internal values of an XY graph object
**
** @fdata [AjPGraph]
** @fcategory modify
**
** @nam3rule Set Set internals
** @nam3rule Setflag Set internal boolean flag
** @nam3rule Show Set whether to display an xy graph element
** @nam4rule Devicetype Plplot device type from a defined xy graph object
** @nam4rule Gaps Set whether graph will allow gaps
** @nam4rule Overlay Set whether graph will overlay multiple graphs
** @nam4rule Points Options for display of points on the graph
** @nam5rule PointsCircle Display of points as circles
** @nam5rule PointsJoin Display of points joined by lines
** @nam4rule Minmax Set min and max values
** @nam4rule Ranges Use min and max values from data
** @nam4rule Raxis Set the graph is to display a right hand Y axis.
** @nam4rule Uaxis Set the graph is to display an upper X axis.
** @nam4rule Unum Set the graph is to number an upper X axis.
** @nam4rule Ynum Set the graph is to number the Y axis.
** @nam4rule Xaxis Set the graph is to display an X axis.
** @nam4rule Yaxis Set the graph is to display a Y axis.
** @nam4rule Xlabel Set whether the graph is to label the x axis.
** @nam4rule Ylabel Set whether the graph is to label the y axis.
** @nam4rule Xtick Set whether the graph is to tick mark the x axis.
** @nam4rule Ytick Set whether the graph is to tick mark the y axis.
** @nam4rule Xend Set the end position for X in the graph.
** @nam4rule Yend Set the end position for Y in the graph.
** @nam4rule Xgrid Whether the graph is to grid the tick marks on the x axis
** @nam4rule Ygrid Whether the graph is to grid the tick marks on the y axis
** @nam4rule Xinvert whether the graph is to display the tick marks inside
**                   the plot on the x axis.
** @nam4rule Yinvert whether the graph is to display the tick marks inside
**                   the plot on the x axis.
** @nam4rule Xrange Set the X axis range
** @nam4rule Yrange Set the Y axis range
** @nam4rule Xstart Set the start position for X in the graph.
** @nam4rule Ystart Set the start position for Y in the graph.
** @suffix F floating point value
** @suffix I Integer value(s)
**
** @argrule * thys [AjPGraph] Graph object
** @argrule Devicetype devicetype [const AjPStr] Device type
** @argrule Setflag set [AjBool] True to set the flag, false to clear
** @argrule Show set [AjBool] True to set the flag, false to clear
** @argrule Minmax xmin [float] X minimum value
** @argrule Minmax xmax [float] X maximum value
** @argrule Minmax ymin [float] Y minimum value
** @argrule Minmax ymax [float] Y maximum value
** @argrule F val [float] Graph position in user units
** @argrule I start [ajint] start position
** @argrule I end [ajint] end position
**
** @valrule * [void]
** @valrule *Devicetype [AjBool] True on success
**
******************************************************************************/




/* @func ajGraphxySetDevicetype ***********************************************
**
** Initialize options for a graphxy object
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] devicetype [const AjPStr] Device type
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajGraphxySetDevicetype(AjPGraph thys, const AjPStr devicetype)
{
    ajint i;
    ajint j=-1;
    ajint k;
    AjPStr aliases = NULL;

    ajDebug("ajGraphxySetDevicetype '%S'\n", devicetype);

    for(i=0;graphType[i].Name;i++)
    {
        if(!graphType[i].XYDisplay)
            continue;

        if(ajStrMatchCaseC(devicetype, graphType[i].Name))
	{
	    j = i;
	    break;
	}

	if(ajCharPrefixCaseS(graphType[i].Name, devicetype))
	{
	    if(j < 0)
		j = i;
	    else
	    {
		for(k=0;graphType[k].Name;k++)
		{
		    if(ajCharPrefixCaseS(graphType[k].Name, devicetype))
		    {
			if(ajStrGetLen(aliases))
			    ajStrAppendC(&aliases, ", ");
			ajStrAppendC(&aliases, graphType[k].Name);
		    }
		}
		    
		ajErr("Ambiguous graph device name '%S' (%S)",
		       devicetype, aliases);
		ajStrDel(&aliases);

		return ajFalse;
	    }
	}
    }

    if(j<0)
	return ajFalse;

    GraphxyNewPlplot(thys);

    /* Calling funclist graphType() */

    thys->displaytype = j;

    ajDebug("ajGraphxySetDevicetype type '%S' displaytype %d '%s'\n",
	    devicetype, j, graphType[j].Name);

    return ajTrue;

}




/* @obsolete ajGraphxySet
** @rename ajGraphxySetDevicetype
*/
__deprecated AjBool ajGraphxySet(AjPGraph thys, const AjPStr type)
{
    return ajGraphxySetDevicetype(thys, type);
}




/* @func ajGraphxySetMinmax ***************************************************
**
** Set the max and min of the data points for all graphs.
**
** @param [u] thys [AjPGraph] multiple graph structure.
** @param [r] xmin [float]  x min.
** @param [r] xmax [float]  x max.
** @param [r] ymin [float]  y min.
** @param [r] ymax [float]  y max.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetMinmax(AjPGraph thys,float xmin,float xmax,
			float ymin,float ymax)
{
    AjPGraphdata graphdata;
    ajuint i;

    if (!thys)
	return;

    thys->minX = xmin;
    thys->minY = ymin;
    thys->maxX = xmax;
    thys->maxY = ymax;

    for(i = 0 ; i < thys->numofgraphs ; i++)
    {
	graphdata = (thys->graphs)[i];
	graphdata->minX = xmin;
	graphdata->minY = ymin;
	graphdata->maxX = xmax;
	graphdata->maxY = ymax;
    }

    return;
}




/* @obsolete ajGraphxySetMaxMin
** @rename ajGraphxySetMinmax
*/
__deprecated void ajGraphxySetMaxMin(AjPGraph thys,float xmin,float xmax,
                                     float ymin,float ymax)
{
    ajGraphxySetMinmax(thys, xmin, xmax, ymin, ymax);
    return;
}




/* @func ajGraphxySetRanges ***************************************************
**
** Calculate the max and min of the data points and store them.
**
** See ajGraphxySetMaxmin for a function that defines explicit ranges
**
** @param [u] thys [AjPGraph] multiple graph structure.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetRanges(AjPGraph thys)
{
    AjPGraphdata graphdata = NULL;
    ajuint i;
    ajuint j;

    for(i = 0 ; i < thys->numofgraphs ; i++)
    {
	graphdata = (thys->graphs)[i];

	if(graphdata->minX == graphdata->maxX ||
	   graphdata->minY == graphdata->maxY)
	{
	    graphdata->minX = graphdata->minY =  64000;
	    graphdata->maxX = graphdata->maxY =  -64000;

	    for( j = 0 ; j < graphdata->numofpoints; j++)
	    {
		if(graphdata->maxX < (graphdata->x)[j])
		    graphdata->maxX = (graphdata->x)[j];
		if(graphdata->maxY < (graphdata->y)[j])
		    graphdata->maxY = (graphdata->y)[j];
		if(graphdata->minX > (graphdata->x)[j])
		    graphdata->minX = (graphdata->x)[j];
		if(graphdata->minY > (graphdata->y)[j])
		    graphdata->minY = (graphdata->y)[j];
	    }
	}

	if (thys->minX > graphdata->minX)
	    thys->minX = graphdata->minX;

	if (thys->minY > graphdata->minY)
	    thys->minY = graphdata->minY;

	if (thys->maxX < graphdata->maxX)
	    thys->maxX = graphdata->maxX;

	if (thys->maxY < graphdata->maxY)
	    thys->maxY = graphdata->maxY;
    }

    if(!thys->minmaxcalc)
    {
	thys->xstart     = thys->minX;
	thys->xend       = thys->maxX;
	thys->ystart     = thys->minY;
	thys->yend       = thys->maxY;
	thys->minmaxcalc = ajTrue;
    }

    return;
}




/* @obsolete ajGraphxyCheckMaxMin
** @rename ajGraphxySetRanges
*/
__deprecated void ajGraphxyCheckMaxMin(AjPGraph thys)
{
    ajGraphxySetRanges(thys);
    return;
}




/* @func ajGraphxySetXendF *****************************************************
**
** Set the end position for X in the graph.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] val  [float]  The end value for x graph coord
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXendF(AjPGraph thys, float val)
{
    if (thys)
    {
	thys->xend = val;
    }

    return;
}




/* @obsolete ajGraphxySetXEnd
** @rename ajGraphxySetXendF
*/

__deprecated void ajGraphxySetXEnd(AjPGraph thys, float val)
{
    ajGraphxySetXendF(thys, val);
    return;
}




/* @func ajGraphxySetXrangeII *************************************************
**
** Sets the X axis range with integers
**
** @param [u] thys [AjPGraph] Graph
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXrangeII(AjPGraph thys, ajint start, ajint end)
{
    if (thys)
    {
	thys->xstart = (float) start;
	thys->xend = (float) end;
    }

    return;
}




/* @obsolete ajGraphxySetXRangeII
** @rename ajGraphxySetXrangeII
*/

__deprecated void ajGraphxySetXRangeII(AjPGraph thys, ajint start, ajint end)
{
    ajGraphxySetXrangeII(thys, start, end);

    return;
}




/* @func ajGraphxySetXstartF ***************************************************
**
** Set the start position for X in the graph.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] val  [float] The start value for x graph coord.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXstartF(AjPGraph thys, float val)
{
    if (thys)
    {
	thys->xstart = val;
    }

    return;
}




/* @obsolete ajGraphxySetXStart
** @rename ajGraphxySetXstartF
*/

__deprecated void ajGraphxySetXStart(AjPGraph thys, float val)
{
    ajGraphxySetXstartF(thys, val);
    return;
}




/* @func ajGraphxySetYendF *****************************************************
**
** Set the end position for Y in the graph.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] val  [float] The start value for y graph coord.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetYendF(AjPGraph thys, float val)
{
    if (thys)
    {
	thys->yend = val;
    }

    return;
}




/* @obsolete ajGraphxySetYEnd
** @rename ajGraphxySetYendF
*/

__deprecated void ajGraphxySetYEnd(AjPGraph thys, float val)
{
    ajGraphxySetYendF(thys, val);
    return;
}




/* @func ajGraphxySetYrangeII *************************************************
**
** Sets the Y axis range with integers
**
** @param [u] thys [AjPGraph] Graph
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetYrangeII(AjPGraph thys, ajint start, ajint end)
{
    if (thys)
    {
	thys->ystart = (float) start;
	thys->yend = (float) end;
    }

    return;
}




/* @obsolete ajGraphxySetYRangeII
** @rename ajGraphxySetYrangeII
*/

__deprecated void ajGraphxySetYRangeII(AjPGraph thys, ajint start, ajint end)

{
    ajGraphxySetYrangeII(thys, start, end);
    return;
}




/* @func ajGraphxySetYstartF ***************************************************
**
** Set the start position for Y in the graph.
**
** @param [u] thys [AjPGraph] Graph structure to store info in.
** @param [r] val  [float] The end value for y graph coord.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetYstartF(AjPGraph thys, float val)
{
    if (thys)
    {
	thys->ystart = val;
    }

    return;
}




/* @obsolete ajGraphxySetYStart
** @rename ajGraphxySetYstartF
*/

__deprecated void ajGraphxySetYStart(AjPGraph thys, float val)
{
    ajGraphxySetYstartF(thys, val);
    return;
}




/* @func ajGraphxySetflagGaps *************************************************
**
** Set whether the graphs should enable gaps.
**
** @param [u] thys [AjPGraph] Multiple graph object
** @param [r] set [AjBool] if true allow gaps else do not.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetflagGaps(AjPGraph thys, AjBool set)
{
    if (!thys)
	return;

    if(thys->flags & AJGRAPH_GAPS)
    {
	if(!set)
	    thys->flags -= AJGRAPH_GAPS;
    }
    else
    {
	if(set)
	    thys->flags += AJGRAPH_GAPS;
    }

    return;
}




/* @obsolete ajGraphxySetGaps
** @rename ajGraphxySetflagGaps
*/
__deprecated void ajGraphxySetGaps(AjPGraph thys, AjBool set)
{
    ajGraphxySetflagGaps(thys, set);
    return;
}




/* @func ajGraphxySetflagOverlay **********************************************
**
** Set whether the graphs should lay on top of each other.
**
** @param [u] thys [AjPGraph] Multiple graph object
** @param [r] set [AjBool] if true overlap else do not.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetflagOverlay(AjPGraph thys, AjBool set)
{
    if (!thys)
	return;

    if(thys->flags & AJGRAPH_OVERLAP)
    {
	if(!set)
	    thys->flags -= AJGRAPH_OVERLAP;
    }
    else
    {
	if(set)
	    thys->flags += AJGRAPH_OVERLAP;
    }

    return;
}




/* @obsolete ajGraphxySetOverLap
** @rename ajGraphxySetflagOverlay
*/
__deprecated void ajGraphxySetOverLap(AjPGraph thys, AjBool overlap)
{
    ajGraphxySetflagOverlay(thys, overlap);
    return;
}




/* @func ajGraphxyShowPointsCircle *********************************************
**
** Set the graph to draw circles at the points.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowPointsCircle(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_CIRCLEPOINTS, set);

    return;
}




/* @obsolete ajGraphxySetCirclePoints
** @rename ajGraphxyShowPointsCircle
*/
__deprecated void ajGraphxySetCirclePoints(AjPGraph thys, AjBool set)
{
    ajGraphxyShowPointsCircle(thys, set);
    return;
}




/* @func ajGraphxyShowPointsJoin ***********************************************
**
** Set the graph to draw lines between the points.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowPointsJoin(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_JOINPOINTS, set);

    return;
}




/* @obsolete ajGraphxySetJoinPoints
** @rename ajGraphxyShowPointsJoin
*/
__deprecated void ajGraphxySetJoinPoints(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_JOINPOINTS, set);

    return;
}




/* @func ajGraphxyShowRaxis ****************************************************
**
** Set the graph is to display a right hand Y axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowRaxis(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_RIGHT, set);

    return;
}




/* @obsolete ajGraphxySetYRight
** @rename ajGraphxyShowRaxis
*/

__deprecated void ajGraphxySetYRight(AjPGraph thys, AjBool set)
{
    ajGraphxyShowRaxis(thys, set);
    return;
}




/* @func ajGraphxyShowUaxis ****************************************************
**
** Set whether the graph is to display the left X axis at the top.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowUaxis(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_TOP, set);

    return;
}




/* @obsolete ajGraphxySetXTop
** @rename ajGraphxyShowUaxis
*/

__deprecated void ajGraphxySetXTop(AjPGraph thys, AjBool set)
{
    ajGraphxyShowUaxis(thys,set);
    return;
}




/* @func ajGraphxyShowUnum *****************************************************
**
** Set whether the graph is to display the labels on the upper x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowUnum(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_NUMLABEL_ABOVE, set);

    return;
}




/* @obsolete ajGraphxySetXLabelTop
** @rename ajGraphxyShowUnum
*/

__deprecated void ajGraphxySetXLabelTop(AjPGraph thys, AjBool set)
{
    ajGraphxyShowUnum(thys, set);
    return;
}




/* @func ajGraphxyShowXaxis ****************************************************
**
** Set whether the graph is to display a bottom x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowXaxis(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_BOTTOM, set);

    return;
}




/* @obsolete ajGraphxySetXBottom
** @rename ajGraphxyShowXaxis
*/

__deprecated void ajGraphxySetXBottom(AjPGraph thys, AjBool set)
{
    ajGraphxyShowXaxis(thys,set);
    return;
}




/* @func ajGraphxyShowXgrid ****************************************************
**
** Set whether the graph is to grid the tick marks on the x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowXgrid(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_GRID, set);

    return;
}




/* @obsolete ajGraphxySetXGrid
** @rename ajGraphxyShowXgrid
*/

__deprecated void ajGraphxySetXGrid(AjPGraph thys, AjBool set)
{
    ajGraphxyShowXgrid(thys, set);
    return;
}




/* @func ajGraphxyShowXinvert **************************************************
**
** Set whether the graph is to display the tick marks inside the plot on
** the x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowXinvert(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_INVERT_TICK, set);

    return;
}




/* @obsolete ajGraphxySetXInvTicks
** @rename ajGraphxyShowXinvert
*/

__deprecated void ajGraphxySetXInvTicks(AjPGraph thys, AjBool set)
{
    ajGraphxyShowXinvert(thys, set);
    return;
}




/* @func ajGraphxyShowXlabel ***************************************************
**
** Set whether the graph is to label the x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowXlabel(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_LABEL, set);

    return;
}




/* @obsolete ajGraphxySetXLabel
** @rename  ajGraphxyShowXlabel
*/

__deprecated void ajGraphxySetXLabel(AjPGraph thys, AjBool set)
{
    ajGraphxyShowXlabel(thys, set);
    return;
}




/* @func ajGraphxyShowXtick *************************************************
**
** Set whether the graph is to display tick marks on the x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowXtick(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_X_TICK, set);

    return;
}




/* @obsolete ajGraphxySetXTick
** @rename ajGraphxyShowXtick
*/

__deprecated void ajGraphxySetXTick(AjPGraph thys, AjBool set)
{
    ajGraphxyShowXtick(thys, set);
    return;
}




/* @func ajGraphxyShowYaxis ****************************************************
**
** Set whether the graph is to display the left Y axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowYaxis(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_LEFT, set);

    return;
}




/* @obsolete ajGraphxySetYLeft
** @rename ajGraphxyShowYaxis
*/
__deprecated void ajGraphxySetYLeft(AjPGraph thys, AjBool set)
{
    ajGraphxyShowYaxis(thys, set);
    return;
}




/* @func ajGraphxyShowYgrid ****************************************************
**
** Set whether the graph is to grid the tick marks on the x axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowYgrid(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_GRID, set);

    return;
}




/* @obsolete ajGraphxySetYGrid
** @rename ajGraphxyShowYgrid
*/

__deprecated void ajGraphxySetYGrid(AjPGraph thys, AjBool set)
{
    ajGraphxyShowYgrid(thys, set);
    return;
}




/* @func ajGraphxyShowYinvert **************************************************
**
** Set whether the graph is to display the tick marks inside the plot
** on the y axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowYinvert(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_INVERT_TICK, set);

    return;
}




/* @obsolete ajGraphxySetYInvTicks
** @rename ajGraphxyShowYinvert
*/

__deprecated void ajGraphxySetYInvTicks(AjPGraph thys, AjBool set)
{
    ajGraphxyShowYinvert(thys,set);
    return;
}




/* @func ajGraphxyShowYlabel **************************************************
**
** Set whether the graph is to label the y axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowYlabel(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_LABEL, set);

    return;
}




/* @obsolete ajGraphxySetYLabel
** @rename ajGraphxyShowYlabel
*/

__deprecated void ajGraphxySetYLabel(AjPGraph thys, AjBool set)
{
    ajGraphxyShowYlabel(thys, set);
    return;
}




/* @func ajGraphxyShowYnum ****************************************************
**
** Set whether the graph is to display the numbers on the left Y axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowYnum(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_NUMLABEL_LEFT, set);

    return;
}




/* @obsolete ajGraphxySetYLabelLeft
** @rename ajGraphxyShowYnum
*/

__deprecated void ajGraphxySetYLabelLeft(AjPGraph thys, AjBool set)
{
    ajGraphxyShowYnum(thys, set);
    return;
}




/* @func ajGraphxyShowYtick ****************************************************
**
** Set the graph is to display tick marks on the y axis.
**
** @param [u] thys [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyShowYtick(AjPGraph thys, AjBool set)
{
    ajGraphSetFlag(thys, AJGRAPH_Y_TICK, set);

    return;
}




/* @obsolete ajGraphxySetYTick
** @rename ajGrapxyhShowYtick
*/

__deprecated void ajGraphxySetYTick(AjPGraph thys, AjBool set)
{

    ajGraphxyShowYtick(thys, set);
    return;
}




/* @datasection [AjPGraphdata] Graph data object ********************************
**
** Function is for manipulating an AjPGraphdata object
**
** @nam2rule Graphdata
*/




/* @section Constructors *******************************************************
**
** Constructors for AjGraphdata plot data
**
** @fdata [AjPGraphdata]
** @fcategory new
**
** @nam3rule New Constructor
** @suffix I Number of points
** @argrule I numofpoints [ajuint] Number of points
**
** @valrule * [AjPGraphdata] New graphdata object
**
******************************************************************************/




/* @func ajGraphdataNew ****************************************************
**
** Creates a new empty graph
**
** @return [AjPGraphdata] New empty graph
** @@
******************************************************************************/

AjPGraphdata ajGraphdataNew(void)
{
    static AjPGraphdata graphdata;

    AJNEW0(graphdata);
    graphdata->numofpoints = 0;

    return graphdata;
}




/* @func ajGraphdataNewI ***************************************************
**
** Create and initialise the data structure for the graph with a defined
** number of data points.
**
** @param [r] numofpoints [ajuint] Number of points
** @return [AjPGraphdata] Pointer to new graph structure.
** @@
******************************************************************************/

AjPGraphdata ajGraphdataNewI(ajuint numofpoints)
{
    AjPGraphdata graphdata;

    AJNEW0(graphdata);

    GraphdataInit(graphdata);

    graphdata->numofpoints = numofpoints;

    graphdata->xcalc = ajFalse;
    AJCNEW0(graphdata->x, numofpoints);

    graphdata->ycalc = ajFalse;
    AJCNEW0(graphdata->y, numofpoints);

    return graphdata;
}




/* @section Destructors *******************************************************
**
** Destructors for AjGraphdata plot data
**
** @fdata [AjPGraphdata]
** @fcategory delete
**
** @nam3rule Del Destructor
**
** @argrule Del pthys [AjPGraphdata*] Graph data object
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphdataDel ****************************************************
**
** Destructor for a graph data object
**
** @param [d] pthys [AjPGraphdata*] Graph data object
** @return [void]
** @@
******************************************************************************/

void ajGraphdataDel(AjPGraphdata *pthys)
{
    AjPGraphobj here = NULL;
    AjPGraphobj p    = NULL;
    AjPGraphdata thys;

    thys = *pthys;

    if (!thys)
	return;

    ajDebug("ajGraphdataDel objects:%d\n", thys->numofobjects);

    here = p = thys->Dataobj;

    while(p)
    {
	p = here->next;
	ajStrDel(&here->text);
	AJFREE(here);
	here = p;
    }

    thys->Dataobj = NULL;

    ajStrDel(&thys->title);
    ajStrDel(&thys->subtitle);
    ajStrDel(&thys->xaxis);
    ajStrDel(&thys->yaxis);
    ajStrDel(&thys->gtype);
    AJFREE(thys->x);
    AJFREE(thys->y);
    thys->numofobjects = 0;

    AJFREE(*pthys);

    return;
}




/* @section Modifiers **********************************************************
**
** Set or modify the internals of a graph data object
**
** @fdata [AjPGraphdata]
** @fcategory modify
**
** @nam3rule Set Set internal values
** @nam3rule Setflag Set internal boolean flag
** @nam3rule Show Set whether to display an xy graph element
** @nam4rule Colour colour [ajint] Colour code (see PLPLOT)
** @nam4rule Linetype Set the line type for this graph.
** @nam4rule Minmax Set min and max values
** @nam4rule Subtitle Graph subtitle
** @nam4rule Title Graph title
** @nam4rule Truescale Set the scale max and min of the data points
** @nam4rule Type Graph data output type
** @nam4rule Xlabel Set the title for the X axis.
** @nam4rule Ylabel Set the title for the Y axis.
** @suffix C C character string data
** @suffix S String object data
**
** @argrule * graphdata [AjPGraphdata] Graph data object
** @argrule Show set [AjBool] True to set the flag, false to clear
** @argrule Colour colour [ajint] Colour code (see PLPLOT)
** @argrule Linetype type [ajint] Set the line type.
** @argrule Minmax xmin [float] X minimum value
** @argrule Minmax xmax [float] X maximum value
** @argrule Minmax ymin [float] Y minimum value
** @argrule Minmax ymax [float] Y maximum value
** @argrule Truescale xmin [float] X minimum value
** @argrule Truescale xmax [float] X maximum value
** @argrule Truescale ymin [float] Y minimum value
** @argrule Truescale ymax [float] Y maximum value
** @argrule C txt [const char*] C character string
** @argrule S str [const AjPStr] String object
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphdataSetColour **********************************************
**
** Set the colour for the plot on one graph.
**
** @param [u] graphdata [AjPGraphdata] Graph structure to store info in.
** @param [r] colour [ajint] colour for this plot.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetColour(AjPGraphdata graphdata, ajint colour)
{
    graphdata->colour = colour;

    return;
}




/* @func ajGraphdataSetLinetype ********************************************
**
** Set the line type for this graph.
**
** @param [u] graphdata [AjPGraphdata] Graph structure to store info in.
** @param [r] type [ajint] Set the line type.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetLinetype(AjPGraphdata graphdata, ajint type)
{
    graphdata->lineType = type;

    return;
}




/* @obsolete ajGraphdataSetLineType
** @rename ajGraphdataSetLinetype
*/
__deprecated void ajGraphdataSetLineType(AjPGraphdata graphdata, ajint type)
{
    ajGraphdataSetLinetype(graphdata, type);
    return;
}




/* @func ajGraphdataSetMinmax **********************************************
**
** Set the max and min of the data points you wish to display.
**
** @param [u] graphdata [AjPGraphdata] multiple graph structure.
** @param [r] xmin [float]  x min.
** @param [r] xmax [float]  x max.
** @param [r] ymin [float]  y min.
** @param [r] ymax [float]  y max.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetMinmax(AjPGraphdata graphdata, float xmin, float xmax,
                          float ymin, float ymax)
{
    graphdata->minX = xmin;
    graphdata->minY = ymin;
    graphdata->maxX = xmax;
    graphdata->maxY = ymax;

    return;
}




/* @obsolete ajGraphdataSetMaxMin
** @rename ajGraphdataSetMinmax
*/
__deprecated void ajGraphdataSetMaxMin(AjPGraphdata graphdata,
                                       float xmin, float xmax,
                                       float ymin, float ymax)
{
    ajGraphdataSetMinmax(graphdata, xmin, xmax, ymin, ymax);
    return;
}




/* @func ajGraphdataSetSubtitleC *******************************************
**
**  Set the subtitle.
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] txt [const char*] Sub Title
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetSubtitleC(AjPGraphdata graphdata, const char *txt)
{
    ajStrAssignEmptyC(&graphdata->subtitle,txt);

    return;
}




/* @obsolete ajGraphdataSetSubTitleC
** @rename ajGraphdataSetSubtitleC
*/
__deprecated void ajGraphdataSetSubTitleC(AjPGraphdata graphdata,
                                          const char *title)
{
    ajStrAssignEmptyC(&graphdata->subtitle,title);

    return;
}




/* @func ajGraphdataSetSubtitleS ********************************************
**
**  Set the Subtitle.
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] str [const AjPStr] Sub Title
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetSubtitleS(AjPGraphdata graphdata, const AjPStr str)
{
    ajStrAssignEmptyS(&graphdata->subtitle,str);

    return;
}




/* @obsolete ajGraphdataSetSubTitle
** @rename ajGraphdataSetSubtitleS
*/
__deprecated void ajGraphdataSetSubTitle(AjPGraphdata graphdata,
                                         const AjPStr title)
{
    ajGraphdataSetSubtitleS(graphdata, title);
    return;
}




/* @func ajGraphdataSetTitleC **********************************************
**
**  Set the title.
**
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] txt [const char*] Title
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetTitleC(AjPGraphdata graphdata, const char *txt)
{
    ajStrAssignEmptyC(&graphdata->title,txt);

    return;
}




/* @func ajGraphdataSetTitleS ***********************************************
**
**  Set the title.
**
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] str [const AjPStr] Title
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetTitleS(AjPGraphdata graphdata, const AjPStr str)
{
    ajStrAssignEmptyS(&graphdata->title,str);

    return;
}




/* @obsolete ajGraphdataSetTitle
** @rename ajGraphdataSetTitleS
*/
__deprecated void ajGraphdataSetTitle(AjPGraphdata graphdata,
                                      const AjPStr title)
{
    ajGraphdataSetTitleS(graphdata, title);
    return;
}




/* @func ajGraphdataSetTruescale **********************************************
**
** Set the scale max and min of the data points you wish to display.
**
** @param [u] graphdata [AjPGraphdata] multiple graph structure.
** @param [r] xmin [float]  true x min.
** @param [r] xmax [float]  true x max.
** @param [r] ymin [float]  true y min.
** @param [r] ymax [float]  true y max.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetTruescale(AjPGraphdata graphdata, float xmin, float xmax,
			    float ymin, float ymax)
{
    graphdata->tminX = xmin;
    graphdata->tminY = ymin;
    graphdata->tmaxX = xmax;
    graphdata->tmaxY = ymax;

    return;
}




/* @obsolete ajGraphdataSetMaxima
** @rename ajGraphdataSetTruescale
*/
__deprecated void ajGraphdataSetMaxima(AjPGraphdata graphdata,
                                       float xmin, float xmax,
                                       float ymin, float ymax)
{
    ajGraphdataSetTruescale(graphdata, xmin, xmax, ymin, ymax);
    return;
}




/* @func ajGraphdataSetTypeC ***********************************************
**
** Set the type of the graph for data output.
**
** @param [u] graphdata [AjPGraphdata] multiple graph structure.
** @param [r] txt [const char*]  Type e.g. "2D Plot", "Histogram".
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetTypeC(AjPGraphdata graphdata,const  char* txt)
{
    ajStrAssignC(&graphdata->gtype,txt);

    return;
}




/* @func ajGraphdataSetTypeS ***********************************************
**
** Set the type of the graph for data output.
**
** @param [u] graphdata [AjPGraphdata] multiple graph structure.
** @param [r] str [const AjPStr]  Type e.g. "2D Plot", "Histogram".
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetTypeS(AjPGraphdata graphdata,const AjPStr str)
{
    ajStrAssignS(&graphdata->gtype,str);

    return;
}




/* @func ajGraphdataSetXlabelC *********************************************
**
** Set the title for the X axis.
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] txt[const char*] title for the X axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetXlabelC(AjPGraphdata graphdata, const char* txt)
{
    ajStrAssignEmptyC(&graphdata->xaxis,txt);

    return;
}




/* @obsolete ajGraphdataSetXTitleC
** @rename ajGraphdataSetXlabelC
*/
__deprecated void ajGraphdataSetXTitleC(AjPGraphdata graphdata,
                                        const char* title)
{
    ajGraphdataSetXlabelC(graphdata, title);
    return;
}




/* @func ajGraphdataSetXlabelS **********************************************
**
** Set the title for the X axis.
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] str [const AjPStr] title for the X axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetXlabelS(AjPGraphdata graphdata, const AjPStr str)
{
    ajStrAssignEmptyS(&graphdata->xaxis,str);

    return;
}




/* @obsolete ajGraphdataSetXTitle
** @rename ajGraphdataSetXlabelS
*/
__deprecated void ajGraphdataSetXTitle(AjPGraphdata graphdata,
                                       const AjPStr title)
{
    ajStrAssignEmptyS(&graphdata->xaxis,title);

    return;
}




/* @func ajGraphdataSetYlabelC *********************************************
**
** Set the title for the Y axis.
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] txt [const char*] title for the Y axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetYlabelC(AjPGraphdata graphdata, const char* txt)
{
    ajStrAssignEmptyC(&graphdata->yaxis,txt);

    return;
}




/* @obsolete ajGraphdataSetYTitleC
** @rename ajGraphdataSetYlabelC
*/
__deprecated void ajGraphdataSetYTitleC(AjPGraphdata graphdata,
                                        const char* title)
{
    ajGraphdataSetYlabelC(graphdata, title);
    return;
}




/* @func ajGraphdataSetYlabelS **********************************************
**
** Set the title for the Y axis.
**
** @param [u] graphdata  [AjPGraphdata] Graph structure to store info in.
** @param [r] str [const AjPStr] title for the Y axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphdataSetYlabelS(AjPGraphdata graphdata, const AjPStr str)
{
    ajStrAssignEmptyS(&graphdata->yaxis, str);

    return;
}




/* @obsolete ajGraphdataSetYTitle
** @rename ajGraphdataSetYlabelS
*/
__deprecated void ajGraphdataSetYTitle(AjPGraphdata graphdata,
                                       const AjPStr title)
{
    ajStrAssignEmptyS(&graphdata->yaxis,title);

    return;
}




/* @funcstatic GraphInit ******************************************************
**
** calls plinit.
**
** @param [w] thys [AjPGraph] Graph object.
** @return [void]
** @@
******************************************************************************/

static void GraphInit(AjPGraph thys)
{
    float fold;

    if (thys)
    {
	if(!thys->ready)
	{
	    ajDebug("=g= plinit ()\n");
	    plinit();
	}

	thys->ready = ajTrue;
    }

    fold = ajGraphicsSetCharscale(0.0);
    fold = ajGraphicsSetCharscale(fold);

    return;
}




/* @funcstatic GraphSubPage ***************************************************
**
** start new graph page.
**
** @param [r] page [ajint] Page number
**
** @return [void]
** @@
******************************************************************************/

static void GraphSubPage(ajint page)
{
    ajDebug("=g= pladv (%d) [page]\n", page);
    pladv(page);

    return;
}




/* @funcstatic GraphWind ******************************************************
**
** Calls plwind.
**
** @param [r] xmin [float] Minimum x axis value
** @param [r] xmax [float] Maximum x axis value
** @param [r] ymin [float] Minimum y axis value
** @param [r] ymax [float] Maximum y axis value
**
** @return [void]
** @@
******************************************************************************/

static void GraphWind(float xmin, float xmax, float ymin, float ymax)
{
    ajDebug("=g= plwind (%.2f, %.2f, %.2f, %.2f) [xmin/max ymin/max]\n",
	     xmin, xmax, ymin, ymax);
    plwind(xmin, xmax, ymin, ymax);

    return;
}




/* @funcstatic GraphSetWin ****************************************************
**
** Creates a window using the ranges.
**
** @param [r] xmin [float] Minimum x axis value
** @param [r] xmax [float] Maximum x axis value
** @param [r] ymin [float] Minimum y axis value
** @param [r] ymax [float] Maximum y axis value
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetWin(float xmin, float xmax, float ymin, float ymax)
{
    ajDebug("=g= plvpor (0.0,1.0,0.0,1.0) [whole screen]\n");
    ajDebug("GraphSetWin(%.3f, %.3f, %.3f, %.3f)\n",
	   xmin, xmax, ymin, ymax);

    if(graphData)
    {
	graphData->xmin = xmin;
	graphData->ymin = ymin;
	graphData->xmax = xmax;
	graphData->ymax = ymax;

	if(graphData->Num == 1)	/* else GraphDatafileNext does it */
	{
	    ajFmtPrintF(graphData->File,
			"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
			graphData->xmin, graphData->ymin,
			graphData->xmax, graphData->ymax);
	    graphData->Lines++;
	    graphData->Lines++;
	}
    }
    else
    {
	/*
	** Use the whole screen. User may add boundaries by
	** modifying xmin, xmax etc.
	*/
        plvpor (0.0,1.0,0.0,1.0);
	GraphWind(xmin, xmax, ymin, ymax);
    }

    return;
}




/* @funcstatic GraphSetWin2 ***************************************************
**
** Creates a window using the ranges.
**
** @param [r] xmin [float] Minimum x axis value
** @param [r] xmax [float] Maximum x axis value
** @param [r] ymin [float] Minimum y axis value
** @param [r] ymax [float] Maximum y axis value
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetWin2(float xmin, float xmax, float ymin, float ymax)
{
    ajDebug("=g= plvsta()\n");
    plvsta();
    /* use the whole screen */
    GraphWind(xmin, xmax, ymin, ymax);

    return;
}




/* @funcstatic GraphArray *****************************************************
**
** Draw lines from the array of x and y values.
**
** @param [r] numofpoints [ajuint] Number of data points
** @param [r] x [CONST float*] Array of x axis values
** @param [r] y [CONST float*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphArray(ajuint numofpoints, float *x, float *y)
{
    ajuint i;
    float xlast;
    float ylast;

    if(numofpoints)
	ajDebug("=g= plline( %d, %.2f .. %.2f, %.2f .. %.2f) "
		"[num x..x y..y]\n",
		numofpoints, x[0], x[numofpoints-1], y[0], y[numofpoints-1] );
    else
	ajDebug("=g= plline( %d, <> .. <>, <> .. <>) [num x..x y..y]\n",
		numofpoints );

    if(graphData)
    {
	if(numofpoints < 2)
	    return;

	xlast = *x;
	ylast = *y;

	for(i=1; i<numofpoints; i++)
	{
	    ajGraphicsDrawposLine(xlast, ylast, x[i], y[i]);
	    xlast = x[i];
	    ylast = y[i];
	}
    }
    else
	plline(numofpoints, x,y);

    return;
}




/* @funcstatic GraphArrayGaps *************************************************
**
** Draw lines for an array of floats with gaps.
** Gaps are declared by having values of FLT_MIN.
**
** @param [r] numofpoints [ajuint] Number of data points
** @param [r] x [CONST float*] Array of x axis values
** @param [r] y [CONST float*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphArrayGaps(ajuint numofpoints, float *x, float *y)
{
    ajuint i;
    float *xx1,*xx2;
    float *yy1,*yy2;

    xx1 = xx2 = x;
    yy1 = yy2 = y;
    xx2++;
    yy2++;

    for(i=1;i<numofpoints;i++)
    {
	if(*xx2 != FLT_MIN && *xx1 != FLT_MIN &&
	   *yy2 != FLT_MIN && *yy1 != FLT_MIN)
	{
	    /*ajDebug("=g= pljoin(%.2f, %.2f, %.2f, %.2f) [ xy xy]\n",
		    *xx1, *yy1, *xx2, *yy2);*/
            pljoin(*xx1,*yy1,*xx2,*yy2);
	}

	xx1++; yy1++;
	xx2++; yy2++;
    }

    return;
}




/* @funcstatic GraphArrayGapsI ************************************************
**
** Draw lines for an array of integers with gaps.
** Gaps are declared by having values of INT_MIN.
**
** @param [r] numofpoints [ajuint] Number of data points
** @param [r] x [CONST ajint*] Array of x axis values
** @param [r] y [CONST ajint*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphArrayGapsI(ajuint numofpoints, ajint *x, ajint *y)
{
    ajuint i;
    ajint *xx1,*xx2;
    ajint *yy1,*yy2;

    xx1 = xx2 = x;
    yy1 = yy2 = y;
    xx2++;
    yy2++;

    for(i=0;i<numofpoints-1;i++)
    {
	if(*xx2 != INT_MIN && *xx1 != INT_MIN &&
	   *yy2 != INT_MIN && *yy1 != INT_MIN)
	{
	    /*ajDebug("=g= pljoin(%.2f, %.2f, %.2f, %.2f) "
		    "[ xy xy] [ajint xy xy]\n",
		    (float)*xx1, (float)*xx2, (float)*yy1, (float)*yy2);*/
	    pljoin((float)*xx1,(float)*yy1,(float)*xx2,(float)*yy2);
	}

	xx1++; yy1++;
	xx2++; yy2++;
    }

    return;
}




/* @funcstatic GraphFill ******************************************************
**
** Polygon fill a set of points.
**
** @param [r] numofpoints [ajuint] Number of data points
** @param [r] x [CONST float*] Array of x axis values
** @param [r] y [CONST float*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphFill(ajuint numofpoints, float *x, float *y)
{
    if(numofpoints)
	ajDebug("=g= plfill( %d, %.2f .. %.2f, %.2f .. %.2f) "
		"[num x..x y..y]\n",
		numofpoints, x[0], x[numofpoints-1], y[0], y[numofpoints-1] );
    else
	ajDebug("=g= plfill( %d, <> .. <>, <> .. <>) [num x..x y..y]\n",
		numofpoints );

    plfill(numofpoints, x, y);

    return;
}




/* @funcstatic GraphPen *******************************************************
**
** Change the actual colour of a pen.
**
** @param [r] pen [ajint] Pen colour number
** @param [r] red [ajint] Red value (see PLPLOT)
** @param [r] green [ajint] Green value (see PLPLOT)
** @param [r] blue [ajint] Blue value (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

static void GraphPen(ajint pen, ajint red, ajint green, ajint blue)
{
    ajDebug("=g= plscol0(%d, %d, %d, %d) [pen RGB]\n",
	     pen, red, green, blue);
    plscol0(pen,red,green,blue);

    return;
}




/* @funcstatic GraphClose *****************************************************
**
** Close the graph with the plplot plend command.
**
** @return [void]
** @@
******************************************************************************/

static void GraphClose(void)
{
    AjPList files = NULL;
    AjPStr tmpStr = NULL;

    ajDebug("GraphClose\n");

    if(graphData)
	files = graphData->List;
    else
	ajGraphicsGetOutfiles(&files);

    while(ajListstrGetLength(files))
    {
	ajListstrPop(files, &tmpStr);
	ajDebug("GraphInfo file '%S'\n", tmpStr);
	ajFmtPrint("Created %S\n", tmpStr);
	ajStrDel(&tmpStr);
    }

    if(graphData)
    {
	ajDebug("GraphClose deleting graphData '%F' '%S'.'%S'\n",
		graphData->File, graphData->FName, graphData->Ext);
	ajListstrFree(&graphData->List);
	ajFileClose(&graphData->File);
	ajStrDel(&graphData->FName);
	ajStrDel(&graphData->Ext);
	AJFREE(graphData);
	ajDebug("=g= plend()\n");
 	plend();
    }
    else
    {
	ajListstrFree(&files);

	ajDebug("=g= plend()\n");
	plend();
    }

    return;
}




/* @section Plotting **********************************************************
**
** Plotting drawable objects
**
** @fdata [AjPGraphdata]
** @fcategory use
**
** @nam3rule Add Add an element to the plot data
** @nam3rule Addpos Add an element to the plot data at an (x,y) position
** @nam3rule Calc Calculate data points
** @nam4rule Line Add a line to be drawn when the graph is plotted.
** @nam4rule Rect Add a Rectangle to be drawn when the graph is plotted,
** @nam4rule Text Add Text to be drawn when the graph is plotted.
** @nam5rule TextScale Scaled text
** @nam4rule AddX Adds data points defined in two floating point arrays
** @nam5rule AddXY Adds (x,y) data points defined in two floating point arrays
**
** @nam4rule CalcX data points defined by an x-axis start and increment
** @nam5rule CalcXY data points defined by an x-axis start and increment
**                  and a floating point array of y-axis values
** @suffix C Character data
** @suffix S String data
**
** @argrule * graphdata [AjPGraphdata] Graph data object
** @argrule Addpos x [float] Start x position
** @argrule Addpos y [float] Start y position
** @argrule Line x2 [float] End x position
** @argrule Line y2 [float] End y position
** @argrule Rect x2 [float] End x position
** @argrule Rect y2 [float] End y position
** @argrule Addpos colour [ajint] Colour code (see PLPLOT)
** @argrule Rect fill [ajint] Fill code (see PLPLOT)
** @argrule Scale scale [float] Character scale (0.0 to use the default)
** @argrule C txt [const char*] Character text
** @argrule S str [const AjPStr] Character text
** @argrule AddXY xx [const float*] X coordinates
** @argrule AddXY yy [const float*] Y coordinates
** @argrule CalcXY numofpoints [ajuint] Number of points in array
** @argrule CalcXY start [float] Start position
** @argrule CalcXY incr [float] Increment
** @argrule CalcXY y [const float*] Y coordinates
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphdataAddXY ***********************************************
**
** Adds (x,y) data points defined in two floating point arrays
**
** @param [u] graphdata [AjPGraphdata] Graph object
** @param [r] xx [const float*] X coordinates
** @param [r] yy [const float*] Y coordinates
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddXY(AjPGraphdata graphdata,
			 const float *xx,const float *yy)
{
    ajuint i;

    for(i=0;i<graphdata->numofpoints; i++)
    {
	graphdata->x[i] = xx[i];
	graphdata->y[i] = yy[i];
    }

    return;
}




/* @obsolete ajGraphdataSetXY
** @rename ajGraphdataAddXY
*/
__deprecated void ajGraphdataSetXY(AjPGraphdata graphdata,
                                   const float *x,const float *y)
{
    ajGraphdataAddXY(graphdata, x, y);
    return;
}




/* @func ajGraphdataAddposLine ************************************************
**
** Add a line to be drawn when the graph is plotted.
**
** @param [u] graphdata [AjPGraphdata] Graph data object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] x2 [float] End x position
** @param [r] y2 [float] End y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddposLine(AjPGraphdata graphdata, float x, float y,
			   float x2, float y2, ajint colour)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	AJNEW((graphdata->Dataobj));
	Obj = graphdata->Dataobj;
    }
    else
    {
	Obj = graphdata->Dataobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++graphdata->numofobjects;

    Obj->type = LINE;
    Obj->text = 0;
    Obj->xx1 = x;
    Obj->xx2 = x2;
    Obj->yy1 = y;
    Obj->yy2 = y2;
    Obj->colour = colour;
    Obj->next = 0;

    return;
}




/* @obsolete ajGraphdataAddLine
** @rename ajGraphdataAddposLine
*/
__deprecated void ajGraphdataAddLine(AjPGraphdata graphdata, float x, float y,
			   float x2, float y2, ajint colour)
{
    ajGraphdataAddposLine(graphdata, x, y, x2, y2, colour);
    return;
}




/* @func ajGraphdataAddposRect ************************************************
**
** Add a Rectangle to be drawn when the graph is plotted, fill states whether
** the rectangle should be filled in.
**
** @param [u] graphdata [AjPGraphdata] Graph data object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] x2 [float] End x position
** @param [r] y2 [float] End y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] fill [ajint] Fill code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddposRect(AjPGraphdata graphdata,
			   float x, float y,
			   float x2, float y2,
			   ajint colour, ajint fill)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	AJNEW((graphdata->Dataobj));
	Obj = graphdata->Dataobj;
    }
    else
    {
	Obj = graphdata->Dataobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++graphdata->numofobjects;


    if(fill)
	Obj->type = RECTANGLEFILL;
    else
	Obj->type = RECTANGLE;

    Obj->text = 0;
    Obj->xx1 = x;
    Obj->xx2 = x2;
    Obj->yy1 = y;
    Obj->yy2 = y2;
    Obj->colour = colour;
    Obj->next = 0;

    return;
}




/* @obsolete ajGraphdataAddRect
** @rename ajGraphdataAddposRect
*/
__deprecated void ajGraphdataAddRect(AjPGraphdata graphdata,
			   float x, float y,
			   float x2, float y2,
			   ajint colour, ajint fill)
{
    ajGraphdataAddposRect(graphdata, x, y, x2, y2, colour, fill);
    return;
}




/* @func ajGraphdataAddposTextC ***********************************************
**
** Add Text to be drawn when the graph is plotted.
**
** @param [u] graphdata [AjPGraphdata] Graph data object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] txt [const char*] Text to add
1**
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddposTextC(AjPGraphdata graphdata, float x, float y,
                            ajint colour, const char *txt)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	AJNEW((graphdata->Dataobj));
	Obj = graphdata->Dataobj;
    }
    else			   /* cycle through till NULL found */
    {
	Obj = graphdata->Dataobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++graphdata->numofobjects;


    Obj->type = TEXT;
    Obj->text = 0;
    ajStrAssignEmptyC(&Obj->text,txt);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = 0.0;
    Obj->next = 0;

    return;
}




/* @obsolete ajGraphdataAddText
** @rename ajGraphdataAddposTextC
*/

__deprecated void ajGraphdataAddText(AjPGraphdata graphdata, float x, float y,
			   ajint colour, const char *txt)
{
    ajGraphdataAddposTextC(graphdata, x, y, colour, txt);
    return;
}




/* @func ajGraphdataAddposTextS ***********************************************
**
** Add Text to be drawn when the graph is plotted.
**
** @param [u] graphdata [AjPGraphdata] Graph data object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] str [const AjPStr] Text to add
**
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddposTextS(AjPGraphdata graphdata, float x, float y,
                         ajint colour, const AjPStr str)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	AJNEW((graphdata->Dataobj));
	Obj = graphdata->Dataobj;
    }
    else			   /* cycle through till NULL found */
    {
	Obj = graphdata->Dataobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++graphdata->numofobjects;


    Obj->type = TEXT;
    Obj->text = 0;
    ajStrAssignEmptyS(&Obj->text,str);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = 0.0;
    Obj->next = 0;

    return;
}




/* @func ajGraphdataAddposTextScaleC ******************************************
**
** Add Text to be drawn when the graph is plotted.
**
** @param [u] graphdata [AjPGraphdata] Graph data object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] scale [float] Character scale (0.0 to use the default)
** @param [r] txt [const char*] Text to add
**
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddposTextScaleC(AjPGraphdata graphdata,
                                 float x, float y,
                                 ajint colour, float scale, const char *txt)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	AJNEW((graphdata->Dataobj));
	Obj = graphdata->Dataobj;
    }
    else			   /* cycle through till NULL found */
    {
	Obj = graphdata->Dataobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++graphdata->numofobjects;


    Obj->type = TEXT;
    Obj->text = 0;
    ajStrAssignEmptyC(&Obj->text,txt);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = scale;
    Obj->next = 0;

    return;
}




/* @obsolete ajGraphdataAddTextScale
** @rename ajGraphdataAddposTextScaleC
*/
__deprecated void ajGraphdataAddTextScale(AjPGraphdata graphdata,
				float xx1, float yy1,
				ajint colour, float scale, const char *txt)
{
    ajGraphdataAddposTextScaleC(graphdata, xx1, yy1, colour, scale, txt);
    return;
}




/* @func ajGraphdataAddposTextScaleS ******************************************
**
** Add Text to be drawn when the graph is plotted.
**
** @param [u] graphdata [AjPGraphdata] Graph data object
** @param [r] x [float] Start x position
** @param [r] y [float] Start y position
** @param [r] colour [ajint] Colour code (see PLPLOT)
** @param [r] scale [float] Character scale (0.0 to use the default)
** @param [r] str [const AjPStr] Text to add
**
** @return [void]
** @@
******************************************************************************/

void ajGraphdataAddposTextScaleS(AjPGraphdata graphdata,
                                 float x, float y,
                                 ajint colour, float scale, const AjPStr str)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	AJNEW((graphdata->Dataobj));
	Obj = graphdata->Dataobj;
    }
    else			   /* cycle through till NULL found */
    {
	Obj = graphdata->Dataobj;

	while(Obj->next)
	    Obj = Obj->next;

	AJNEW(Obj->next);
	Obj = Obj->next;
    }

    ++graphdata->numofobjects;


    Obj->type = TEXT;
    Obj->text = 0;
    ajStrAssignEmptyS(&Obj->text,str);
    Obj->xx1 = x;
    Obj->xx2 = 0.0;
    Obj->yy1 = y;
    Obj->yy2 = 0.0;
    Obj->colour = colour;
    Obj->scale = scale;
    Obj->next = 0;

    return;
}




/* @func ajGraphdataCalcXY **********************************************
**
** Adds (x,y) data points defined by an x-axis start and increment
** and a floating point array of y-axis values.
**
** @param [u] graphdata [AjPGraphdata] Graph object
** @param [r] numofpoints [ajuint] Number of points in array
** @param [r] start [float] Start position
** @param [r] incr [float] Increment
** @param [r] y [const float*] Y coordinates
** @return [void]
** @@
******************************************************************************/

void ajGraphdataCalcXY(AjPGraphdata graphdata, ajuint numofpoints,
                       float start, float incr, const float* y)
{
    ajuint i;
    PLFLT x = 0.0;

    if(!graphdata->xcalc)
	AJFREE(graphdata->x);

    if(!graphdata->ycalc)
	AJFREE(graphdata->y);

    graphdata->xcalc = ajFalse;	 /* i.e. OK to delete at the end as it */
    graphdata->ycalc = ajFalse;	 /* is our own copy */

    AJCNEW(graphdata->x, numofpoints);
    AJCNEW(graphdata->y, numofpoints);

    for(i=0;i<numofpoints; i++)
    {
	graphdata->x[i] = (start+x);
	graphdata->y[i] = y[i];
	x += incr;
    }

    graphdata->numofpoints = numofpoints;

    return;
}




/* @funcstatic GraphdataInit ***********************************************
**
** Initialise the contents of a graph object
**
** @param [w] graphdata  [AjPGraphdata] Graph structure to store info in.
** @return [void]
** @@
******************************************************************************/

static void GraphdataInit(AjPGraphdata graphdata)
{
    ajDebug("GraphdataInit title: %x subtitle: %x xaxis: %x yaxis: %x\n",
	    graphdata->title, graphdata->subtitle,
	    graphdata->xaxis, graphdata->yaxis);

    graphdata->x = NULL;
    graphdata->y = NULL;
    graphdata->xcalc = ajTrue;
    graphdata->ycalc = ajTrue;
    ajStrAssignEmptyC(&graphdata->title,"");
    ajStrAssignEmptyC(&graphdata->subtitle,"");
    ajStrAssignEmptyC(&graphdata->xaxis,"");
    ajStrAssignEmptyC(&graphdata->yaxis,"");
    ajStrAssignEmptyC(&graphdata->gtype,"");
    graphdata->minX = graphdata->maxX = 0;
    graphdata->minY = graphdata->maxY = 0;
    graphdata->lineType = 1;
    graphdata->colour = BLACK;
    graphdata->Dataobj = 0;

    return;
}




/* @section Debugging **********************************************************
**
** Tracing the internal values of a graph data object
**
** @fdata [AjPGraphdata]
** @fcategory use
**
** @nam3rule Trace
**
** @argrule * thys [const AjPGraphdata] Graphdata object
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphdataTrace **************************************************
**
** Writes debug messages to trace the contents of a graphdata object.
**
** @param [r] thys [const AjPGraphdata] Graphdata object
** @return [void]
** @@
******************************************************************************/

void ajGraphdataTrace(const AjPGraphdata thys)
{
    ajint i = 0;
    AjPGraphobj obj;

    ajDebug("GraphData trace\n");
    ajDebug("\n(a) True booleans\n");

    if(thys->xcalc)
	ajDebug("xcalc %B\n", thys->xcalc);

    if(thys->ycalc)
	ajDebug("ycalc %B\n", thys->ycalc);

    ajDebug("\n(b) Strings with values\n");

    ajDebug("Title '%S'\n", thys->title);
    ajDebug("Subtitle '%S'\n", thys->subtitle);
    ajDebug("Xaxis '%S'\n", thys->xaxis);
    ajDebug("Yaxis '%S'\n", thys->yaxis);
    ajDebug("gtype '%S'\n", thys->gtype);

    ajDebug("\n(c) Other values\n");
    ajDebug("numofpoints %d\n", thys->numofpoints);
    ajDebug("numofobjects %d\n", thys->numofobjects);
    ajDebug("minX   %7.3f maxX   %7.3f\n", thys->minX, thys->maxX);
    ajDebug("minY   %7.3f maxY   %7.3f\n", thys->minY, thys->maxY);
    ajDebug("tminX   %7.3f tmaxX   %7.3f\n", thys->tminX, thys->tmaxX);
    ajDebug("tminY   %7.3f tmaxY   %7.3f\n", thys->tminY, thys->tmaxY);
    ajDebug("colour %d\n", thys->colour);
    ajDebug("lineType %d\n", thys->lineType);

    ajDebug("obj list: %x\n", thys->Dataobj);

    if(thys->Dataobj)
    {
	obj=thys->Dataobj;

	while(obj->next)
	{
	    i++;
	    obj = obj->next;
	}
    }

    ajDebug("obj list length: %d/%d\n",
	     i, thys->numofobjects);
 
    return;
}




/******************************************************************************
PLPLOT CALLS *END*
******************************************************************************/




/* @datasection [none] Reporting internals ************************************
**
** Reporting internal values for general graphs
**
** @nam2rule Graphics
**
******************************************************************************/




/* @section Reporting internals ***********************************************
**
** @fdata [none]
** @fcategory output
**
** @nam3rule Dump Dump to standard error
** @nam3rule Get Get internal information
** @nam3rule List Save as a list object
** @nam3rule Print to an output file
** @nam4rule PrintType Print available device types
** @nam4rule Devices List of valid device names using AJAX names
** @nam4rule GetOutfiles Get output file names
**
** @argrule Outfiles files [AjPList*] List of graph files
** @argrule ListDevices list [AjPList] List to write device names to.
** @argrule Print outf [AjPFile] Output file
** @argrule Type full [AjBool] Full report (usually ajFalse)
**
** @valrule * [void]
** @valrule *Outfiles [ajuint]
**
******************************************************************************/




/* @func ajGraphicsDumpDevices *************************************************
**
** Dump device options for a graph object
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsDumpDevices(void)
{
    ajint i;
    ajint j;
    AjPStr aliases = NULL;

    ajUserDumpC("Devices allowed (with alternative names) are:-");

    for(i=0;graphType[i].Name;i++)
    {
	if(!graphType[i].GOpen)
            continue;

	if(!graphType[i].Alias)
	{
	    ajStrAssignClear(&aliases);

	    for(j=0;graphType[j].Name;j++)
	    {
		if(graphType[j].Alias &&
		   ajCharMatchC(graphType[i].Device, graphType[j].Device))
		{
		    if(ajStrGetLen(aliases))
			ajStrAppendC(&aliases, ", ");
		    ajStrAppendC(&aliases, graphType[j].Name);
		}
	    }

	    if(ajStrGetLen(aliases))
		ajUser("%s (%S)",graphType[i].Name, aliases);
	    else
		ajUserDumpC(graphType[i].Name);
	}
    }

    ajStrDel(&aliases);

    return;
}




/* @func ajGraphicsGetOutfiles *************************************************
**
** Information on files created for graph output
**
** @param [w] files [AjPList*] List of graph files
** @return [ajuint] Number of files (will match length of list)
** @@
******************************************************************************/

ajuint ajGraphicsGetOutfiles(AjPList* files)
{
    ajint i= -1;
    ajint j;
    AjPStr tmpStr = NULL;

    PLINT family = 0;
    PLINT filenum = 0;
    PLINT bmax = 0;
    char filename[1024];

    plgfam(&family, &filenum, &bmax);

    if(filenum)
        i = filenum;
    else
        i = -1;

    ajDebug("ajGraphicsGetOutfiles i:%d\n", i);

    *files = ajListstrNew();

    ajDebug("ajGraphicsGetOutfiles new list\n");

    if(!i)
	return 0;

    if(i < 0)				/* single filename - or none */
    {
        plgfnam(filename);
        if(!*filename)
            return 0;
	ajStrAssignC(&tmpStr, filename);
	ajListstrPushAppend(*files, tmpStr);
	tmpStr=NULL;
	return 1;
    }

    for(j=1; j<=i; j++) /* family: tmp has format, i is no. of files */
    {
	ajDebug("ajGraphicsGetOutfiles printing file %d\n", j);
	ajFmtPrintS(&tmpStr, "%S.%d.%S", graphBasename, j, graphExtension);
	ajDebug("ajGraphicsGetOutfiles storing file '%S'\n", tmpStr);
	ajListstrPushAppend(*files, tmpStr);
	tmpStr=NULL;
    }

    return i;
}




/* @obsolete ajGraphInfo
** @rename ajGraphicsGetOutfiles
*/

__deprecated ajint ajGraphInfo(AjPList* files)
{
    return ajGraphicsGetOutfiles(files);
}




/* @func ajGraphicsListDevices *************************************************
**
** Store device names for a graph object in a list
**
** @param [w] list [AjPList] List to write device names to.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsListDevices (AjPList list)
{
    ajint i;
    AjPStr devicename;

    for(i=0;graphType[i].Name;i++)
    {
	if(!graphType[i].GOpen)
            continue;

	if(!graphType[i].Alias)
	{
	    devicename = ajStrNewC(graphType[i].Name);
	    ajListstrPushAppend(list, devicename);
	}
    }

    return;
}




/* @obsolete ajGraphListDevices
** @rename ajGraphicsListDevices
*/
__deprecated void ajGraphListDevices (AjPList list)
{
    ajGraphicsListDevices(list);
    return;
}




/* @func ajGraphicsPrintType **************************************************
**
** Print graph types
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/
void ajGraphicsPrintType(AjPFile outf, AjBool full)
{
    GraphPType gt;
    ajint i;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Graphics Devices\n");
    ajFmtPrintF(outf, "# Name     Name\n");
    ajFmtPrintF(outf, "# Alias    Alias name\n");
    ajFmtPrintF(outf, "# Device   Device name\n");
    ajFmtPrintF(outf, "# Extension Filename extension (null if no file "
                "created)\n");
    ajFmtPrintF(outf, "# Description\n");
    ajFmtPrintF(outf, "# Name       Alias Device     Extension Description\n");
    ajFmtPrintF(outf, "GraphType {\n");

    for(i=0; graphType[i].Name; i++)
    {
	gt = &graphType[i];

	if(full || !gt->Alias)
        {
	    ajFmtPrintF(outf, "  %-10s", gt->Name);
	    ajFmtPrintF(outf, " %5B", gt->Alias);
	    ajFmtPrintF(outf, " %-10s", gt->Device);
	    ajFmtPrintF(outf, " %-10s", gt->Ext);
	    ajFmtPrintF(outf, " \"%s\"", gt->Desc);
	    ajFmtPrintF(outf, "\n");
	}
    }

    ajFmtPrintF(outf, "}\n");

    return;
}




/* @obsolete ajGraphPrintType
** @rename ajGraphicsPrintType
*/
__deprecated void ajGraphPrintType(AjPFile outf, AjBool full)
{
    ajGraphicsPrintType(outf, full);
    return;
}




/* @datasection [none] Reporting internals ************************************
**
** Reporting internal values for sequence-based XY graphs
**
** @nam2rule Graphicsxy
**
******************************************************************************/




/* @section Reporting internals ***********************************************
**
** @fdata [none]
** @fcategory output
**
** @nam3rule Dump Dump to standard error
** @nam3rule List Save as a list object
** @nam4rule Devices List of valid device names using AJAX names
**
** @argrule ListDevices list [AjPList] List to write device names to.
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajGraphicsxyDumpDevices ***********************************************
**
** Dump device options for a graph object
**
** @return [void]
** @@
******************************************************************************/

void ajGraphicsxyDumpDevices(void)
{
    ajint i;
    ajint j;
    AjPStr aliases = NULL;

    ajUserDumpC("Devices allowed (with alternative names) are:-");

    for(i=0;graphType[i].Name;i++)
    {
	if(!graphType[i].XYDisplay)
            continue;

	if(!graphType[i].Alias)
	{
	    ajStrAssignClear(&aliases);

	    for(j=0;graphType[j].Name;j++)
	    {
		if(graphType[j].Alias &&
		   ajCharMatchC(graphType[i].Device, graphType[j].Device))
		{
		    if(ajStrGetLen(aliases))
			ajStrAppendC(&aliases, ", ");
		    ajStrAppendC(&aliases, graphType[j].Name);
		}
	    }

	    if(ajStrGetLen(aliases))
		ajUser("%s (%S)",graphType[i].Name, aliases);
	    else
		ajUserDumpC(graphType[i].Name);
	}
    }

    ajStrDel(&aliases);

    return;
}




/* @obsolete ajGraphxyDumpDevices
** @rename ajGraphicsxyDumpDevices
*/
__deprecated void ajGraphxyDumpDevices(void)
{
    ajGraphicsxyDumpDevices();
    return;
}




/* @func ajGraphicsxyListDevices **********************************************
**
** Store device names for a graph object in a list
**
** @param [w] list [AjPList] List to write device names to.
** @return [void]
** @@
******************************************************************************/

void ajGraphicsxyListDevices (AjPList list)
{
    ajint i;
    AjPStr devicename;

    for(i=0;graphType[i].Name;i++)
    {
	if(!graphType[i].XYDisplay)
            continue;

	if(!graphType[i].Alias)
	{
	    devicename = ajStrNewC(graphType[i].Name);
	    ajListstrPushAppend(list, devicename);
	}
    }

    return;
}




/* @obsolete ajGraphxyListDevices
** @rename ajGraphicsxyListDevices
*/
__deprecated void ajGraphxyListDevices (AjPList list)
{
    ajGraphicsxyListDevices(list);
    return;
}




/* @funcstatic GraphCheckFlags ************************************************
**
** Prints the flags defined by bits in the input integer value.
**
** @param [r] flags [ajint] flag bits
** @return [void]
** @@
******************************************************************************/

static void GraphCheckFlags(ajint flags)
{
    ajFmtPrint("flag = %d\n",flags);
    if(flags & AJGRAPH_X_BOTTOM)
	ajFmtPrint("AJGRAPH_X_BOTTOM \n");

    if(flags & AJGRAPH_Y_LEFT)
	ajFmtPrint("AJGRAPH_Y_LEFT \n");

    if(flags & AJGRAPH_X_TOP)
	ajFmtPrint("AJGRAPH_X_TOP \n");

    if(flags & AJGRAPH_Y_RIGHT)
	ajFmtPrint(" AJGRAPH_Y_RIGHT\n");

    if(flags & AJGRAPH_X_TICK)
	ajFmtPrint("AJGRAPH_X_TICK \n");

    if(flags & AJGRAPH_Y_TICK)
	ajFmtPrint("AJGRAPH_Y_TICK \n");

    if(flags & AJGRAPH_X_LABEL)
	ajFmtPrint("AJGRAPH_X_LABEL \n");

    if(flags & AJGRAPH_Y_LABEL)
	ajFmtPrint("AJGRAPH_Y_LABEL \n");

    if(flags & AJGRAPH_TITLE)
	ajFmtPrint("AJGRAPH_TITLE \n");

    if(flags & AJGRAPH_JOINPOINTS)
	ajFmtPrint("AJGRAPH_JOINPOINTS \n");

    if(flags & AJGRAPH_OVERLAP)
	ajFmtPrint("AJGRAPH_OVERLAP \n");

    if(flags & AJGRAPH_Y_NUMLABEL_LEFT)
	ajFmtPrint("AJGRAPH_Y_NUMLABEL_LEFT");

    if(flags & AJGRAPH_Y_INVERT_TICK)
	ajFmtPrint("AJGRAPH_Y_INVERT_TICK");

    if(flags & AJGRAPH_Y_GRID)
	ajFmtPrint("AJGRAPH_Y_GRID");

    if(flags & AJGRAPH_X_NUMLABEL_ABOVE)
	ajFmtPrint("AJGRAPH_X_NUMLABEL_ABOVE");

    if(flags & AJGRAPH_X_INVERT_TICK)
	ajFmtPrint("AJGRAPH_X_INVERT_TICK");

    if(flags & AJGRAPH_X_GRID)
	ajFmtPrint("AJGRAPH_X_GRID");
    
    return;
}




/* @funcstatic GraphCheckPoints ***********************************************
**
** Prints a list of data points from two floating point arrays.
**
** @param [r] n [ajint] Number of points to print
** @param [r] x [const PLFLT*] X coordinates
** @param [r] y [const PLFLT*] Y coordinates
** @return [void]
** @@
******************************************************************************/

static void GraphCheckPoints(ajint n, const PLFLT *x, const PLFLT *y)
{
    ajint i;

    for(i=0;i<n;i++)
	ajFmtPrint("%d %f %f\n",i,x[i],y[i]);

    return;
}




/* @funcstatic GraphOpenFile **************************************************
**
** A general routine for setting BaseName and extension in plplot.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphOpenFile(AjPGraph thys, const char *ext)
{
    ajDebug("GraphOpenFile '%S'\n", thys->outputfile);

    GraphSetName(thys, thys->outputfile,ext);

    return;
}




/* @funcstatic GraphOpenData **************************************************
**
** A general routine for setting BaseName and extension in plplot
** for -graph data output.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphOpenData(AjPGraph thys, const char *ext)
{
    ajDebug("GraphOpenData '%S' '%s\n", thys->outputfile, ext);

    if(!graphData)
    {
        AJNEW0(graphData);
        graphData->List = ajListstrNew();
    }

    GraphSetName(thys, thys->outputfile,ext);
    thys->isdata = ajTrue;

    ajStrAssignS(&graphData->FName, thys->outputfile);
    ajStrAssignC(&graphData->Ext, ext);

    GraphDatafileNext();

    return;
}




/* @funcstatic GraphOpenSimple ************************************************
**
** A general routine for drawing graphs. Does nothing.
**
** This is called when initialised. Graph files are opened when
** each graph is written.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphOpenSimple(AjPGraph thys, const char *ext )
{
    (void) thys;			/* make it used */
    (void) ext;				/* make it used */

    return;
}




#ifndef X_DISPLAY_MISSING
/* @funcstatic GraphOpenXwin **************************************************
**
** A general routine for drawing graphs to an xwin. Only creates the window.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphOpenXwin(AjPGraph thys, const char *ext )
{
    char* plargv[2] = {NULL,NULL};
    ajint plargc = 2;
    char argname[] = "-plwindow";

    (void) thys;			/* make it used */
    (void) ext;				/* make it used */

    plargv[0] = argname;
    plargv[1] = MAJSTRGETPTR(ajUtilGetProgram());
    ajDebug("=g= plparseopts('%S', '%s') ajUtilGetProgram\n",
            plargv[0], plargv[1]);

    plparseopts(&plargc, plargv,
                PL_PARSE_NOPROGRAM | PL_PARSE_QUIET |
                PL_PARSE_NODELETE | PL_PARSE_SKIP);

/*
    GraphLabelTitle(((thys->flags & AJGRAPH_TITLE) ?
		     ajStrGetPtr(thys->title) : " "),
		    ((thys->lags & AJGRAPH_SUBTITLE) ?
		     ajStrGetPtr(thys->subtitle) : " "));
	
*/

    return;
}
#endif




/* @funcstatic GraphxyDisplayToFile *******************************************
**
** A general routine for drawing graphs to a file.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphxyDisplayToFile(AjPGraph thys, AjBool closeit,
				 const char *ext)
{
    	ajDebug("GraphxyDisplayToFile '%S'\n", thys->outputfile);

	GraphSetName(thys, thys->outputfile,ext);
	ajGraphxySetRanges(thys);
	GraphxyGeneral(thys, closeit);

    return;
}




/* @funcstatic GraphxyDisplayToDas ********************************************
**
** A general routine for drawing graphs to DAS output file as points.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphxyDisplayToDas(AjPGraph thys, AjBool closeit,
                                const char *ext)
{
    AjPFile outf    = NULL;
    AjPGraphdata graphdata  = NULL;
    AjPStr temp;
    AjPTime ajtime;
    ajuint i,j;
    float minxa = 64000.;
    float minya = 64000.;
    float maxxa = -64000.;
    float maxya = -64000.;
    ajint nfeat = 0;
    ajint istart;
    ajint iend;

    /*
    ** Things to do:
    **
    ** Test this output
    ** Check how it handles multiple sequences
    ** Try to catch the sequence information
    **
    ** Do we want end=start or end=start+1
    **
    */

     ajDebug("GraphxyDisplayToDas '%S' '%s' %d graphs\n",
	     thys->outputfile, ext, thys->numofgraphs);
        
    if(!graphData)
    {
	AJNEW0(graphData);
	graphData->List = ajListstrNew();
    }

    /* Calculate maxima and minima */
    for(i=0;i<thys->numofgraphs;i++)
    {
	graphdata = (thys->graphs)[i];
	minxa = (minxa<graphdata->minX) ? minxa : graphdata->minX;
	minya = (minya<graphdata->minY) ? minya : graphdata->minY;
	maxxa = (maxxa>graphdata->maxX) ? maxxa : graphdata->maxX;
	maxya = (maxya>graphdata->maxY) ? maxya : graphdata->maxY;
    }
    
    ajFmtPrintF(outf,"<DASGFF>\n");
    ajFmtPrintF(outf,"  <GFF version=\"1.0\" href=\"url\">\n");
    ajFmtPrintF(outf,"<!DOCTYPE DASGFF SYSTEM \"http://www.biodas.org/dtd/dasgff.dtd\">\n");
    ajFmtPrintF(outf,"<DASGFF>\n");
    ajFmtPrintF(outf,"  <GFF version=\"1.0\" href=\"url\">\n");

    ajFmtPrintF(outf,"<!-- Title %S -->\n", thys->title);
    ajFmtPrintF(outf,"<!-- Graphs %d -->\n",thys->numofgraphs);
    ajFmtPrintF(outf,"<!-- Number %d -->\n",i+1);
    ajFmtPrintF(outf,"<!-- Points %d -->\n",graphdata->numofpoints);
	
    for(i=0;i<thys->numofgraphs;i++)
    {
	graphdata = (thys->graphs)[i];
	
	/* open a file for dumping the data points */
	temp = ajFmtStr("%S%d%s",thys->outputfile,i+1,ext);
	outf = ajFileNewOutNameS(temp);
	ajListstrPushAppend(graphData->List, temp);
	if(!outf)
	{
	    ajErr("Could not open graph file %S\n",temp);
	    return;
	}
	else
	    ajDebug("Writing graph %d data to %S\n",i+1,temp);

	if( ajStrGetLen(thys->title) <=1)
	{
	    ajFmtPrintAppS(&thys->title,"%S",
			   ajUtilGetProgram());
	}
	if( ajStrGetLen(thys->subtitle) <=1)
	{
	    ajtime = ajTimeNewTodayFmt("report");
	    ajFmtPrintAppS(&thys->subtitle,"%D",
			   ajtime);
	    ajTimeDel(&ajtime);
	}

        istart = (ajint) graphdata->minX;
        if(istart < 1)
            istart = 1;
        iend = (ajint) graphdata->maxX;
        if(iend < istart)
            iend = istart;
	ajFmtPrintF(outf,"    <SEGMENT id=\"%s\" start=\"%d\" stop=\"%d\"\n",
                    "graphid",
                    istart,
                    iend);
        ajFmtPrintF(outf,
                    "                version=\"%s\">\n",
                    "0.0");

	
	/* Dump out the data points */
	for(j=0;j<graphdata->numofpoints;j++)
        {
            nfeat++;
	    ajFmtPrintF(outf,"      <FEATURE id=\"%s.%d\">\n",
                        "featid", nfeat);
            ajFmtPrintF(outf,"        <START>%d</START>\n",
                        (ajint) graphdata->x[j]);
            ajFmtPrintF(outf,"        <END>%d</END>\n",
                        (ajint) graphdata->x[j]);
            ajFmtPrintF(outf,"        <SCORE>%f</SCORE>\n",
                        graphdata->y[j]);
	    ajFmtPrintF(outf,"      </FEATURE>\n");
	}
        
	
	ajFmtPrintF(outf,"    <SEGMENT>\n");
    }
    
    ajFmtPrintF(outf,
                "  </GFF>\n");
    ajFmtPrintF(outf,
                "</DASGFF>\n");

    ajFileClose(&outf);
    if(closeit)
	GraphClose();

    return;
}




/* @funcstatic GraphxyDisplayToData *******************************************
**
** A general routine for drawing graphs to file as points.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphxyDisplayToData(AjPGraph thys, AjBool closeit,
				 const char *ext)
{
    AjPFile outf    = NULL;
    AjPGraphdata graphdata  = NULL;
    AjPGraphobj ptr = NULL;
    AjPStr temp;
    AjPTime ajtime;
    ajuint i,j;
    float minxa = 64000.;
    float minya = 64000.;
    float maxxa = -64000.;
    float maxya = -64000.;
    ajint type  = 0;
    
    ajDebug("GraphxyDisplayToData '%S' '%s' %d graphs\n",
	     thys->outputfile, ext, thys->numofgraphs);
        
    if(!graphData)
    {
	AJNEW0(graphData);
	graphData->List = ajListstrNew();
    }

    /* Calculate maxima and minima */
    for(i=0;i<thys->numofgraphs;i++)
    {
	graphdata = (thys->graphs)[i];
	minxa = (minxa<graphdata->minX) ? minxa : graphdata->minX;
	minya = (minya<graphdata->minY) ? minya : graphdata->minY;
	maxxa = (maxxa>graphdata->maxX) ? maxxa : graphdata->maxX;
	maxya = (maxya>graphdata->maxY) ? maxya : graphdata->maxY;
    }
    
    for(i=0;i<thys->numofgraphs;i++)
    {
	graphdata = (thys->graphs)[i];
	
	/* open a file for dumping the data points */
	temp = ajFmtStr("%S%d%s",thys->outputfile,i+1,ext);
	outf = ajFileNewOutNameS(temp);
	ajListstrPushAppend(graphData->List, temp);

	if(!outf)
	{
	    ajErr("Could not open graph file %S\n",temp);

	    return;
	}
	else
	    ajDebug("Writing graph %d data to %S\n",i+1,temp);

	if( ajStrGetLen(thys->title) <=1)
	{
	    ajFmtPrintAppS(&thys->title,"%S",
			   ajUtilGetProgram());
	}

	if( ajStrGetLen(thys->subtitle) <=1)
	{
	    ajtime = ajTimeNewTodayFmt("report");
	    ajFmtPrintAppS(&thys->subtitle,"%D",
			   ajtime);
	    ajTimeDel(&ajtime);
	}

	ajFmtPrintF(outf,"##%S\n",graphdata->gtype);
	ajFmtPrintF(outf,"##Title %S\n",thys->title);
	ajFmtPrintF(outf,"##Graphs %d\n",thys->numofgraphs);
	ajFmtPrintF(outf,"##Number %d\n",i+1);
	ajFmtPrintF(outf,"##Points %d\n",graphdata->numofpoints);
	
	
	ajFmtPrintF(outf,"##XminA %f XmaxA %f YminA %f YmaxA %f\n",
			   minxa,maxxa,minya,maxya);
	ajFmtPrintF(outf,"##Xmin %f Xmax %f Ymin %f Ymax %f\n",
		    graphdata->tminX,graphdata->tmaxX,
		    graphdata->tminY,graphdata->tmaxY);
	ajFmtPrintF(outf,"##ScaleXmin %f ScaleXmax %f ScaleYmin %f "
			   "ScaleYmax %f\n",
		    graphdata->minX,graphdata->maxX,
		    graphdata->minY,graphdata->maxY);
	
	ajFmtPrintF(outf,"##Maintitle %S\n",graphdata->title);
	
	if(thys->numofgraphs == 1)
	{
	    ajFmtPrintF(outf,"##Xtitle %S\n",thys->xaxis);
	    ajFmtPrintF(outf,"##Ytitle %S\n",thys->yaxis);
	}
	else
	{
	    if(graphdata->subtitle)
	        ajFmtPrintF(outf,"##Subtitle %S\n",graphdata->subtitle);
	    ajFmtPrintF(outf,"##Xtitle %S\n",graphdata->xaxis);
	    ajFmtPrintF(outf,"##Ytitle %S\n",graphdata->yaxis);
	}
	
	/* Dump out the data points */
	for(j=0;j<graphdata->numofpoints;j++)
	    ajFmtPrintF(outf,"%f\t%f\n",graphdata->x[j],graphdata->y[j]);
	
	
	/* Now for the Data graph objects */
	ajFmtPrintF(outf,"##DataObjects\n##Number %d\n",
		    graphdata->numofobjects);
	
	if(graphdata->numofobjects)
	{
	    ptr = graphdata->Dataobj;

	    for(j=0;j<graphdata->numofobjects;++j)
	    {
		type = ptr->type;

		if(type==LINE || type==RECTANGLE || type==RECTANGLEFILL)
		{
		    if(type==LINE)
			ajFmtPrintF(outf,"Line ");
		    else if(type==RECTANGLE)
			ajFmtPrintF(outf,"Rectangle ");
		    else
			ajFmtPrintF(outf,"Filled Rectangle ");
		    ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d\n",
				ptr->xx1,ptr->yy1,
				ptr->xx2,ptr->yy2,ptr->colour);
		}
		else if(type==TEXT)
		{
		    ajFmtPrintF(outf,"Textline ");
		    ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d "
				"size 1.0 %S\n",
				ptr->xx1,ptr->yy1,ptr->xx2,
				ptr->yy2,ptr->colour,
				ptr->text);
		}

		ptr = ptr->next;
	    }
	}
	
	
	/* Now for the Graph graph objects */
	ajFmtPrintF(outf,"##GraphObjects\n##Number %d\n",
		    thys->numofobjects);
	
	if(thys->numofobjects)
	{
	    ptr = thys->Mainobj;

	    for(j=0;j<thys->numofobjects;++j)
	    {
		type = ptr->type;

		if(type==LINE || type==RECTANGLE || type==RECTANGLEFILL)
		{
		    if(type==LINE)
			ajFmtPrintF(outf,"Line ");
		    else if(type==RECTANGLE)
			ajFmtPrintF(outf,"Rectangle ");
		    else
			ajFmtPrintF(outf,"Filled Rectangle ");

		    ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d\n",
				ptr->xx1,ptr->yy1,
				ptr->xx2,ptr->yy2,ptr->colour);
		}
		else if(type==TEXT)
		{
		    ajFmtPrintF(outf,"Textline ");
		    ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d "
				"size 1.0 %S\n",
				ptr->xx1,ptr->yy1,
				ptr->xx2,ptr->yy2,ptr->colour,
				ptr->text);
		}

		ptr = ptr->next;
	    }
	}
	
	
	ajFileClose(&outf);
    }
    
    if(closeit)
	GraphClose();

    return;
}




/* @funcstatic GraphxyDisplayXwin *********************************************
**
** A general routine for drawing graphs to an xwin.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [const char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphxyDisplayXwin(AjPGraph thys, AjBool closeit, const char *ext )
{
    (void) ext;				/* make it used */

    ajGraphxySetRanges(thys);
    GraphxyGeneral(thys, closeit);

    return;
}




/* @funcstatic GraphNewPlplot *************************************************
**
** Populate a plplot structure to hold a general graph.
**
** @param [w] graph [AjPGraph] multiple graph structure.
** @return [void]
** @@
******************************************************************************/

static void GraphNewPlplot(AjPGraph graph)
{
    AJCNEW0(graph->graphs,1);

    ajDebug("GraphNewPlplot\n");

    graph->numofgraphs    = 0;
    graph->numofgraphsmax = 1;
    graph->flags          = GRAPH_XY;
    graph->minmaxcalc     = 0;
    ajFmtPrintS(&graph->outputfile,"%S", ajUtilGetProgram());

    return;
}




/* @funcstatic GraphxyNewPlplot ***********************************************
**
** Create a structure to hold a number of graphs.
**
** @param [w] thys [AjPGraph] Empty graph object
** @return [void]
** @@
******************************************************************************/

static void GraphxyNewPlplot(AjPGraph thys)
{
    AJCNEW0(thys->graphs,thys->numsets);

    ajDebug("GraphxyNewPlplot numsets: %d\n", thys->numsets);

    thys->numofgraphs       = 0;
    thys->numofgraphsmax    = thys->numsets;
    thys->minX = thys->minY = 64000;
    thys->maxX = thys->maxY = -64000;
    thys->flags             = GRAPH_XY;
    thys->minmaxcalc          = 0;
    thys->xstart = thys->xend = 0;
    thys->ystart = thys->yend = 0;
    thys->Mainobj             = NULL;
    ajFmtPrintS(&thys->outputfile,"%S", ajUtilGetProgram());

    return;
}




/* @funcstatic GraphxyGeneral *************************************************
**
** A general routine for drawing graphs.
**
** @param [u] thys [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close at end if true.
** @return [void]
** @@
******************************************************************************/

static void GraphxyGeneral(AjPGraph thys, AjBool closeit)
{
    AjPGraphdata graphdata;
    ajuint i;
    ajint old;
    ajint old2;
    AjPTime ajtime;

    ajGraphicsSetDevice(thys);
    
    ajDebug("GraphxyGeneral flags %x\n", thys->flags);
    ajDebug("... title '%S'\n", thys->title);
    
    if(thys->flags & AJGRAPH_OVERLAP)
    {
	ajDebug("... AJGRAPH_OVERLAP\n");
	GraphColourBack();
	GraphInit(thys);
	GraphColourFore();
	/*    GraphSubPage(0);         Done in ajGraphicsplenv*/
	
	if(thys->numofgraphs)
	{
	    graphdata = (thys->graphs)[0];
	    thys->xstart = graphdata->minX;
	    thys->xend   = graphdata->maxX;
	    thys->ystart = graphdata->minY;
	    thys->yend   = graphdata->maxY;
	}

	ajGraphicsPlenv(thys->xstart, thys->xend,
                        thys->ystart, thys->yend,
                        thys->flags);
	
	if((thys->flags & AJGRAPH_TITLE) &&
	   ajStrGetLen(thys->title) <=1)
	{
	    ajtime = ajTimeNewToday();
	    ajStrAppendC(&thys->title,
			     ajFmtString("%S (%D)",
					 ajUtilGetProgram(),
					 ajtime));
	    ajTimeDel(&ajtime);
	}

	ajGraphicsSetLabelsS(((thys->flags & AJGRAPH_X_LABEL) ?
                              thys->xaxis : NULL),
                             ((thys->flags & AJGRAPH_Y_LABEL) ?
                              thys->yaxis : NULL),
                             ((thys->flags & AJGRAPH_TITLE) ?
                              thys->title : NULL),
                             (thys->flags & AJGRAPH_SUBTITLE) ?
                             thys->subtitle : NULL);
	
	for(i=0;i<thys->numofgraphs;i++)
	{
	    graphdata = (thys->graphs)[i];

	    /* Draw the line through the data */
	    old = ajGraphicsSetFgcolour(graphdata->colour);

	    if(thys->flags & AJGRAPH_CIRCLEPOINTS)
		ajGraphicsDrawsetSymbols(graphdata->numofpoints,
                                         (graphdata->x), (graphdata->y), 9);

	    old2 = ajGraphicsSetLinestyle(graphdata->lineType);

	    if(thys->flags & AJGRAPH_JOINPOINTS)
	    {
		if(thys->flags & AJGRAPH_GAPS)
		    GraphArrayGaps(graphdata->numofpoints,
				   (graphdata->x),
				   (graphdata->y));
		else
		    GraphArray(graphdata->numofpoints,
			       (graphdata->x), (graphdata->y));
	    }

	    ajGraphicsSetLinestyle(old2);
	    ajGraphicsSetFgcolour(old);
	}

	GraphDraw(thys);	
    }
    else
    {
	ajDebug("... else not AJGRAPH_OVERLAP\n");
	GraphSetNumSubPage(thys->numofgraphs);
	GraphColourBack();
	GraphInit(thys);
	GraphColourFore();

	/* first do the main plot title */
	if((thys->flags & AJGRAPH_TITLE) &&
	   ajStrGetLen(thys->title) <=1)
	{
	    ajtime = ajTimeNewToday();
	    ajStrAppendC(&thys->title,
			     ajFmtString("%S (%D)",
					 ajUtilGetProgram(),
					 ajtime));
	    ajTimeDel(&ajtime);
	}

	for(i=0;i<thys->numofgraphs;i++)
	{
	    graphdata = (thys->graphs)[i];
	    ajGraphicsPlenv(graphdata->minX, graphdata->maxX,
                            graphdata->minY, graphdata->maxY,
                            thys->flags);

	    if(!ajStrGetLen(graphdata->title) &&
	       (thys->flags & AJGRAPH_TITLE))
		ajStrAssignS(&graphdata->title, thys->title);

	    if(!ajStrGetLen(graphdata->subtitle) &&
	       (thys->flags & AJGRAPH_SUBTITLE))
		ajStrAssignS(&graphdata->subtitle, thys->subtitle);

	    ajGraphicsSetLabelsS(((thys->flags & AJGRAPH_X_LABEL) ?
                                  graphdata->xaxis : NULL),
                                 ((thys->flags & AJGRAPH_Y_LABEL) ?
                                  graphdata->yaxis : NULL),
                                 ((thys->flags & AJGRAPH_TITLE) ?
                                  graphdata->title : NULL),
                                 (thys->flags & AJGRAPH_SUBTITLE) ?
                                 graphdata->subtitle : NULL);
	    old = ajGraphicsSetFgcolour(graphdata->colour);

	    if(thys->flags & AJGRAPH_CIRCLEPOINTS)
		ajGraphicsDrawsetSymbols(graphdata->numofpoints,
                                         (graphdata->x), (graphdata->y), 9);
	    
	    /* Draw the line through the data */
	    /*    old = ajGraphicsSetFgcolour(i+2);*/
	    old2 = ajGraphicsSetLinestyle(graphdata->lineType);

	    if(thys->flags & AJGRAPH_JOINPOINTS)
	    {
		if(thys->flags & AJGRAPH_GAPS)
		    GraphArrayGaps(graphdata->numofpoints,
				   (graphdata->x), (graphdata->y));
		else
		    GraphArray(graphdata->numofpoints,
			       (graphdata->x), (graphdata->y));
	    }

	    ajGraphicsSetLinestyle(old2);
	    ajGraphicsSetFgcolour(old);
	    GraphDataDraw(graphdata);
	}	
    }

    if(closeit)
	GraphClose();

    return;
}




/* @funcstatic GraphDataPrint **********************************************
**
** Print all the drawable objects in readable form.
**
** @param [r] graphdata [const AjPGraphdata] Graph data object
**
** @return [void]
** @@
******************************************************************************/

static void GraphDataPrint(const AjPGraphdata graphdata)
{
    AjPGraphobj Obj;

    if(!graphdata->Dataobj)
    {
	ajUserDumpC("No Objects");

	return;
    }
    else
    {
	Obj = graphdata->Dataobj;

	while(Obj)
	{
	    if(Obj->type == RECTANGLE )
		ajUser("type = RECTANGLE, %f %f %f %f col= %d",
		       Obj->xx1,Obj->yy1,
		       Obj->xx2,Obj->yy2,Obj->colour);
	    else if(Obj->type == RECTANGLEFILL )
		ajUser("type = RECTANGLEFILL, %f %f %f %f col= %d",
		       Obj->xx1, Obj->yy1,
		       Obj->xx2,Obj->yy2,Obj->colour);
	    else if(Obj->type == TEXT)
		ajUser("type = TEXT, %f %f col= %d %S",Obj->xx1,
		       Obj->yy1,Obj->colour,
		       Obj->text);
	    else if(Obj->type == LINE )
		ajUser("type = LINE, %f %f %f %f col= %d",Obj->xx1,
		       Obj->yy1,
		       Obj->xx2,Obj->yy2,Obj->colour);

	    Obj = Obj->next;
	}
    }

    return;
}




/* @funcstatic GraphDataDraw ***********************************************
**
** Display the drawable objects connected to this graph.
**
** @param [r] graphdata [const AjPGraphdata] Graph data object
**
** @return [void]
** @@
******************************************************************************/

static void GraphDataDraw(const AjPGraphdata graphdata)
{
    AjPGraphobj Obj;
    ajint temp;

    /* graphdata : calls ajGraphLine */

    if(!graphdata->Dataobj)
	return;
    else
    {
	Obj = graphdata->Dataobj;
	while(Obj)
	{
	    if(Obj->type == RECTANGLE)
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposRect(Obj->xx1, Obj->yy1,
                                Obj->xx2,Obj->yy2);
		ajGraphicsSetFgcolour(temp);
	    }
	    else if(Obj->type == RECTANGLEFILL)
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposRectFill(Obj->xx1, Obj->yy1,
				Obj->xx2,Obj->yy2);
		ajGraphicsSetFgcolour(temp);
	    }
	    else if(Obj->type == TEXT )
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposTextAtstart(Obj->xx1, Obj->yy1,
                                             ajStrGetPtr(Obj->text));
		ajGraphicsSetFgcolour(temp);
	    }
	    else if(Obj->type == LINE)
	    {
		temp = ajGraphicsSetFgcolour(Obj->colour);
		ajGraphicsDrawposLine(Obj->xx1, Obj->yy1,
			    Obj->xx2,Obj->yy2);
		ajGraphicsSetFgcolour(temp);
	    }
	    else
		ajUserDumpC("UNDEFINED OBJECT TYPE USED");

	    Obj = Obj->next;
	}
    }

    return;
}




/* @funcstatic GraphDistPts ***************************************************
**
** Compute the distance between 2 points in user coordinates.
**
** @param [r] xx1 [float] x coord of point 1
** @param [r] yy1 [float] y coord of point 1
** @param [r] xx2 [float] x coord of point 2
** @param [r] yy2 [float] y coord of point 2
**
** @return [float] The distance between the 2 points in user coordinates
** @@
******************************************************************************/

static float GraphDistPts(float xx1, float yy1, float xx2, float yy2)
{
    PLFLT diag;

    diag = (PLFLT) sqrt( (xx2-xx1) * (xx2-xx1) + (yy2-yy1) * (yy2-yy1) );

    return diag;
}




/* @funcstatic GraphDrawarcText ***********************************************
**
** Draw text along a curve (i.e., an arc of a circle).
** The text is written character by character.
**
** @param  [r] xcentre [PLFLT] x coord for centre.
** @param  [r] ycentre [PLFLT] y coord for centre.
** @param  [r] Radius  [PLFLT] Radius of the circle.
** @param  [r] Angle [PLFLT] angle at which a particular character
**                           will be written (in deg).
** @param  [r] pos [PLFLT] index for incrementing the angle for the
**                         next character in the text.
** @param  [r] txt [const char*] The text to be displayed.
** @param  [r] just [PLFLT] justification of the string.
**                          (0=left,1=right,0.5=middle etc)
** @return [void]
** @@
******************************************************************************/

static void GraphDrawarcText(PLFLT xcentre, PLFLT ycentre,
                             PLFLT Radius, PLFLT Angle, PLFLT pos,
                             const char *txt, PLFLT just)
{
    ajuint i;
    ajuint numchar;
    PLFLT *xy1;
    PLFLT *xy2;
    char textchar[2];

    numchar = strlen(txt);

    textchar[1] = '\0';
    for(i=0; i<numchar; i++)
    {
	xy1 = ajGraphicsCalcCoord(xcentre, ycentre, Radius, Angle+pos*(PLFLT)i+
			     (PLFLT)0.5*pos);
	xy2 = ajGraphicsCalcCoord(xcentre, ycentre, Radius, Angle+pos*(PLFLT)i+
			     (PLFLT)1.5*pos);
	textchar[0] = txt[i];
	ajGraphicsDrawposTextAtlineJustify(xy1[0], xy1[1],
                                           xy2[0], xy2[1],
                                           textchar, just);
	AJFREE(xy1);
	AJFREE(xy2);
    }

    return;
}




/* @funcstatic GraphSymbols ***************************************************
**
** Draw a symbol from the font list.
**
** @param [r] xx1 [CONST float*] Array of x axis values
** @param [r] yy1 [CONST float*] Array of y axis values
** @param [r] numofdots [ajuint] Number of data points
** @param [r] symbol [ajint] Symbol number (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

static void GraphSymbols(float *xx1, float *yy1, ajuint numofdots,
			 ajint symbol)
{
    ajDebug("=g= plpoin(%d, %.2f .. %.2f, %.2f .. %.2f, %d) "
	     "[size, x..x y..y sym ]\n",
	     numofdots,xx1[0], xx1[numofdots-1], yy1[0], yy1[numofdots-1],
            symbol);
    plpoin(numofdots,xx1,yy1, symbol);

    return;
}




/* @funcstatic GraphText ******************************************************
**
** Display text.
**
** @param [r] xx1 [float] Start of text box on x axis
** @param [r] yy1 [float] Start of text box on y axis
** @param [r] xx2 [float] End of text box on x axis
** @param [r] yy2 [float] End of text box on y axis
** @param [r] just [float] Justification (see PLPLOT)
** @param [r] txt [const char*] Text
**
** @return [void]
** @@
******************************************************************************/

static void GraphText(float xx1, float yy1, float xx2, float yy2,
		      float just,const char *txt)
{
    ajDebug("=g= plptex(%.2f, %.2f, %.2f, %.2f, %.2f, '%s') "
	    "[xy xy just text]\n",
	    xx1, yy1, xx2, yy2, just, txt);
    plptex(xx1,yy1,xx2,yy2,just, txt);

    return;
}




/* @funcstatic GraphDatafileNext **********************************************
**
** Opens the first or next graphData file
**
** return [void]
** @@
******************************************************************************/

static void GraphDatafileNext(void)
{
    AjPStr tempstr;

    ajDebug("GraphDatafileNext lines %d (%.3f, %.3f, %.3f, %.3f)\n",
	     graphData->Lines,
	     graphData->xmin, graphData->xmax,
	     graphData->ymin, graphData->ymax);

    if(graphData)
    {
	tempstr = NULL;
	ajFileClose(&graphData->File);
	graphData->Lines = 0;
	tempstr = ajFmtStr("%S%d%S",
			   graphData->FName,++graphData->Num,graphData->Ext);
	graphData->File = ajFileNewOutNameS(tempstr);

	if(!graphData->File)
	{
	    ajErr("Could not open graph file %S\n",tempstr);
	    return;
	}

	ajDebug("Writing graph %d data to %S\n",graphData->Num,tempstr);

	if(graphData->Num > 1)		/* ajGraphOpenWin will do the first */
	{
	    ajFmtPrintF(graphData->File,
			"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
			graphData->xmin, graphData->ymin,
			graphData->xmax, graphData->ymax);
	    graphData->Lines++;
	    graphData->Lines++;
	}

	ajListstrPushAppend(graphData->List, tempstr);
    }

    return;
}




/* @funcstatic GraphColourBack *************************************************
**
** Set the background colour to either black or white depending on the
** current settings.
**
** @return [void]
** @@
******************************************************************************/

static void GraphColourBack(void)
{
    ajDebug("GraphColourBack currentbgwhite: %B\n", currentbgwhite);

    if(currentbgwhite)
    {
	GraphPen(0, 255, 255, 255);
	GraphPen(MAXCOL, 0, 0, 0);

	graphColourName[0] = "WHITE";
	graphColourName[MAXCOL] = "BLACK";

	ajDebug("GraphColourBack pens 0 '%s' and pen %d '%s'\n",
		graphColourName[0], MAXCOL, graphColourName[MAXCOL]);
    }

    return;
}


#endif
