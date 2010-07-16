/* @source martfilters application
**
** Return biomart filters given mart dataset info
**
** @author Copyright (C) Alan Bleasby
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




/* @prog martfilters *********************************************************
**
** Reads and writes (returns) sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqin seqin  = NULL;
    AjPMartquery mq = NULL;
    AjPMartquery pmq = NULL;
    AjPMartFilter filt = NULL;
    
    AjPStr dataset = NULL;
    AjPStr host = NULL;
    AjPStr path = NULL;
    
    AjPFile outf = NULL;
    ajint iport = 0;

    ajuint i;
    AjPStr key1   = NULL;
    AjPStr key2   = NULL;
    const AjPStr value1 = NULL;
    const AjPStr value2 = NULL;
    
    
    embInit("martfilters", argc, argv);


    host = ajAcdGetString("host");
    path = ajAcdGetString("path");
    iport = ajAcdGetInt("port");
    dataset  = ajAcdGetString("dataset");
    
    outf   = ajAcdGetOutfile("outfile");

    mq    = ajMartqueryNew();
    seqin = ajSeqinNew();

    ajMartAttachMartquery(seqin,mq);
    ajMartSetMarthostS(seqin,host);
    ajMartSetMartpathS(seqin,path);
    ajMartSetMartport(seqin,iport);

    /*
    ** 
    ** This section retrieves the filters given a mart dataset name
    ** from a mart host.
    */

    ajMartGetFilters(seqin, dataset);
    ajMartfiltersParse(seqin);
    

    key1  = ajStrNewC("name");
    key2  = ajStrNewC("displayName");
    
    pmq = ajMartGetMartqueryPtr(seqin);
    filt = pmq->Filters;

    for(i=0; i < filt->Nfilters; ++i)
    {
        value1 = ajTableFetch(filt->Filters[i],(void *)key1);
        value2 = ajTableFetch(filt->Filters[i],(void *)key2);
        ajFmtPrintF(outf,"%-40S %S\n",value1,value2);
    }
    

    ajStrDel(&key1);
    ajStrDel(&key2);

    
    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&dataset);
    
    ajMartquerySeqinFree(seqin);

    ajSeqinDel(&seqin);
    ajFileClose(&outf);
    
    embExit();

    return 0;
}
