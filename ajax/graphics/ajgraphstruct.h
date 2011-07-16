#ifndef ajgraphstruct_h
#define ajgraphstruct_h

#define MAX_STRING 180




/* @data AjPGraphobj *******************************************************
**
** AJAX data structure for graph objects, contained as a substructure
** in AjPGraphdata
**
** @attr type [ajuint] Object type in AjEGraphObjectTypes
** @attr colour [ajuint] See AjEGraphColour for plplot colours
** @attr text [AjPStr] Text to plot
** @attr xx1 [float] x start
** @attr xx2 [float] x end
** @attr yy1 [float] y start
** @attr yy2 [float] y end
** @attr next [struct AjSGraphobj*] link to next object in the list
** @attr scale [float] scale for text (0.0 to use the default)
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSGraphobj {
  ajuint type;
  ajuint colour;
  AjPStr text;
  float xx1;
  float xx2;
  float yy1;
  float yy2;
  struct AjSGraphobj *next;
  float scale;
  char Padding[4];
} AjOGraphobj;
#define AjPGraphobj AjOGraphobj*




/* @data AjPGraphdata ******************************************************
**
** Graph data object. Substructure of AjPGraph.
**
** @attr x [float*] x coordinates
** @attr y [float*] y coordinates
** @attr Dataobj [AjPGraphobj] First graph object - links to rest
** @attr title [AjPStr] Plot title
** @attr subtitle [AjPStr] Plot subtitle
** @attr xaxis [AjPStr] Plot x axis title
** @attr yaxis [AjPStr] Plot y axis title
** @attr gtype [AjPStr] Graph type: 2D, Tick etc
** @attr xcalc [AjBool] if x calculated then delete after
** @attr ycalc [AjBool] as with x. So we do not delete data if it was
**                      passed as a ptr
** @attr truescale [AjBool] if true xy scale has been set
** @attr numofpoints [ajuint] Number of points in x and y
** @attr numofobjects [ajuint] Number of graph objects starting at Obj
** @attr minX [float] Lowest x value
** @attr maxX [float] Highest x value
** @attr minY [float] Lowest y value
** @attr maxY [float] Highest y value
** @attr tminX [float] First x value to plot
** @attr tmaxX [float] Last x value to plot
** @attr tminY [float] First y value to plot
** @attr tmaxY [float] Last y value to plot
** @attr colour [ajint] See AjEGraphColour for plplot colours
** @attr lineType [ajint] Line type for plplot
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSGraphdata {
  float *x;
  float *y;
  AjPGraphobj Dataobj;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  AjPStr gtype;
  AjBool xcalc;
  AjBool ycalc;
  AjBool truescale;
  ajuint numofpoints;
  ajuint numofobjects;
  float minX;
  float maxX;
  float minY;
  float maxY;
  float tminX;
  float tmaxX;
  float tminY;
  float tmaxY;
  ajint colour;
  ajint lineType;
  char Padding[4];
} AjOGraphdata;
#define AjPGraphdata AjOGraphdata*




/* @data AjPGraph *************************************************************
**
** Graph object.
**
** @attr displaytype [ajuint] Displaytype index to graphType
** @attr numsets [ajuint] Number of sets in a multiple graph
** @attr numofgraphs [ajuint] Number of graphs in graphs
** @attr numofobjects [ajuint] Number of objects in Mainobj
** @attr numofgraphsmax [ajuint] Maximum number of graphs expected
** @attr flags [ajuint] over rides the EmbGraphData flags
** @attr minX [float] Lowest x value for all graphs
** @attr maxX [float] Highest x value for all graphs
** @attr minY [float] Lowest y value for all graphs
** @attr maxY [float] Highest y value for all graphs
** @attr xstart [float] First x value to plot
** @attr xend [float] Last x value to plot
** @attr ystart [float] First y value to plot
** @attr yend [float] Last y value to plot
** @attr ready [AjBool] Set by plplot device init
** @attr minmaxcalc [AjBool] Set true when (xy)start/end are set
** @attr windowset [AjBool] Set true when (xy) window coordinates are set
** @attr isdata [AjBool] True for a data type graph
** @attr desc [AjPStr] Plot description, used to set default title
** @attr title [AjPStr] Plot title
** @attr subtitle [AjPStr] Plot subtitle
** @attr xaxis [AjPStr] Plot x axis title
** @attr yaxis [AjPStr] Plot y axis title
** @attr outputfile [AjPStr] Output filename
** @attr graphs [AjPGraphdata*] XY Data to plot for Graph(s)
** @attr Mainobj [AjPGraphobj] Objects to plot for single graph
** @@
******************************************************************************/

typedef struct AjSGraph {

  ajuint displaytype;
  ajuint numsets;
  ajuint numofgraphs;
  ajuint numofobjects;
  ajuint numofgraphsmax;
  ajuint flags;
  float minX;
  float maxX;
  float minY;
  float maxY;
  float xstart;
  float xend;
  float ystart;
  float yend;
  AjBool ready;
  AjBool minmaxcalc;
  AjBool windowset;
  AjBool isdata;
  AjPStr desc;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  AjPStr outputfile;
  AjPGraphdata *graphs;
  AjPGraphobj Mainobj;
} AjOGraph;
#define AjPGraph AjOGraph*

enum AjEGraphColours {BLACK, RED, YELLOW, GREEN, AQUAMARINE,
		      PINK, WHEAT, GREY, BROWN, BLUE, BLUEVIOLET,
		      CYAN, TURQUOISE, MAGENTA, SALMON, WHITE};


#define MAXCOL 15

#endif /* ajgraphstruct_h */
