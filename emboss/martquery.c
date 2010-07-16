/* @source martquery application
**
** Run a simple mart query
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




/* @prog martquery ***********************************************************
**
** Reads and writes (returns) sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqin seqin  = NULL;
    AjPMartquery mq = NULL;

    AjPStr dataset = NULL;
    AjPStr host    = NULL;
    AjPStr path    = NULL;
    AjPStr atts    = NULL;
    AjPStr filts   = NULL;

    AjPMartqinfo qinfo = NULL;

    AjPFile outf = NULL;
    ajint iport = 0;

    AjPStr line   = NULL;
    
    
    embInit("martquery", argc, argv);


    host    = ajAcdGetString("host");
    path    = ajAcdGetString("path");
    iport   = ajAcdGetInt("port");
    dataset = ajAcdGetString("dataset");
    atts    = ajAcdGetString("attributes");
    filts   = ajAcdGetString("filters");
    
    outf   = ajAcdGetOutfile("outfile");

    mq    = ajMartqueryNew();
    seqin = ajSeqinNew();
    line  = ajStrNew();

    ajMartAttachMartquery(seqin,mq);
    ajMartSetMarthostS(seqin,host);
    ajMartSetMartpathS(seqin,path);
    ajMartSetMartport(seqin,iport);

    /*
    ** 
    ** This section performs a query given a mart dataset name, some
    ** attributes and optional filters
    */


    qinfo = ajMartQinfoNew(1);


    ajMartParseParameters(qinfo,atts,filts,0);

    ajMartSetQuerySchemaC(qinfo,"default");
    ajMartSetQueryVersionC(qinfo,"");
    ajMartSetQueryFormatC(qinfo,"TSV");
    ajMartSetQueryCount(qinfo,ajFalse);
    ajMartSetQueryHeader(qinfo,ajFalse);
    ajMartSetQueryUnique(qinfo,ajFalse);
    ajMartSetQueryStamp(qinfo,ajTrue);
    ajMartSetQueryVerify(qinfo,ajTrue);


    ajMartSetQueryDatasetName(qinfo,dataset,0);
    ajMartSetQueryDatasetInterfaceC(qinfo,"default",0);
    
    ajMartCheckQinfo(seqin,qinfo);

    ajMartMakeQueryXml(qinfo,seqin);


    if(!ajMartSendQuery(seqin))
        ajWarn("Query Failed");
    else
        while(ajBuffreadLine(seqin->Filebuff,&line))
            ajFmtPrintF(outf,"%S",line);
        
    
    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&dataset);
    ajStrDel(&line);
    ajStrDel(&atts);
    ajStrDel(&filts);
    
    ajMartquerySeqinFree(seqin);

    ajSeqinDel(&seqin);
    ajFileClose(&outf);
    
    embExit();

    return 0;
}
