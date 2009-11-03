/* @source treetypedisplay application
**
** Plot wobble base percentage
**
** @author Copyright (C) Ian Longden
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




/* @prog treetypedisplay ******************************************************
**
** Testing graphics
**
******************************************************************************/

int main(int argc, char **argv)
{
    PLFLT x1[9]=
    {
	2,1,2,1,1,2,3,3
    };	/* x1,y1  x2,y2  are lines */
    PLFLT y1[9]=
    {
	4,2,3,1,1,3,2,2
    };
    PLFLT x2[9]=
    {
	2,1,1,0,2,3,4,3
    };
    PLFLT y2[9]=
    {
	3,1,2,0,0,2,2,1
    };
    PLFLT pt1[9]=
    {
	2,1,3,4,1,3,0,2,2
    };	/* pt1,pt1    are pts */
    PLFLT pt2[9]=
    {
	4,2,2,2,1,1,0,0,3
    };
    ajint i;
    char temp[7];
    float xmin = -1.0;
    float xmax = 5.0;
    float ymin = -1.0;
    float ymax = 5.0;
    AjPGraph graph;

    ajGraphInit("treetypedisplay", argc, argv);
    graph = ajAcdGetGraph("graph");

    ajGraphOpenWin(graph, xmin,xmax,ymin,ymax);
    ajGraphLines(x1,y1,x2,y2,8);
    ajGraphDots(pt1,pt2,9);
    i = 0;

    for(i=0;i<8;i++)
    {
	sprintf(temp,"line %d",i);
	ajGraphTextLine(x1[i], y1[i], x2[i], y2[i], temp, 1.0);
    }

    ajGraphCloseWin();
    ajGraphxyDel(&graph);

    embExit();

    return 0;
}
