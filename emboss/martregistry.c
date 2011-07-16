/* @source martregistry application
**
** Return biomart registry information
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




/* @prog martregistry ********************************************************
**
** Reads and writes (returns) sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqin seqin  = NULL;
    AjPMartquery mq = NULL;
    AjPMartquery pmq = NULL;
    AjPMartLoc locs = NULL;
    
    AjPStr host = NULL;
    AjPStr path = NULL;
    
    AjPFile outf  = NULL;
    ajint iport = 0;

    ajuint i;
    AjPStr key1   = NULL;
    AjPStr key2   = NULL;
    const AjPStr value1 = NULL;
    const AjPStr value2 = NULL;
    
    embInit("martregistry", argc, argv);


    host = ajAcdGetString("host");
    path = ajAcdGetString("path");
    iport = ajAcdGetInt("port");
    
    outf   = ajAcdGetOutfile("outfile");

    mq    = ajMartqueryNew();
    seqin = ajSeqinNew();

    ajMartAttachMartquery(seqin,mq);
    ajMartSetReghostS(seqin,host);
    ajMartSetRegpathS(seqin,path);
    ajMartSetRegport(seqin,iport);

    ajMartGetRegistry(seqin);
    ajMartregistryParse(seqin);

    key1  = ajStrNewC("name");
    key2  = ajStrNewC("displayName");
    
    pmq = ajMartGetMartqueryPtr(seqin);
    locs = pmq->Loc;


    for(i=0; i < locs->Nurls; ++i)
    {
        value1 = ajTableFetchS(locs->Urls[i], key1);
        value2 = ajTableFetchS(locs->Urls[i], key2);
        ajFmtPrintF(outf,"%-40S %S\n",value1,value2);
    }
    

    ajStrDel(&key1);
    ajStrDel(&key2);
    

    ajStrDel(&host);
    ajStrDel(&path);
    
    ajMartquerySeqinFree(seqin);

    ajSeqinDel(&seqin);
    ajFileClose(&outf);
    
    embExit();

    return 0;
}
