/* @source martseqs application
**
** Return mart datasets that can return sequences
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




/* @prog martseqs ********************************************************
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

    AjPMartDataset    ds = NULL;
    AjPMartAttribute att = NULL;
    
    AjPStr host = NULL;
    AjPStr path = NULL;
    
    AjPFile outf  = NULL;
    ajint iport = 0;

    ajuint i;
    ajuint j;
    ajuint k;
    AjPStr key1  = NULL;
    AjPStr mart  = NULL;

    AjPStr dataset = NULL;
    AjPStr vschema = NULL;
    
    const AjPStr value1 = NULL;

    
    embInit("martseqs", argc, argv);


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

    key1 = ajStrNew();
    mart = ajStrNew();
    
    dataset = ajStrNew();
    vschema = ajStrNew();
    
    pmq = ajMartGetMartqueryPtr(seqin);
    locs = pmq->Loc;

    
    for(i=0; i < locs->Nurls; ++i)
    {
        ajStrAssignC(&key1,"visible");
        value1 = ajTableFetchS(locs->Urls[i], key1);
        if(ajStrMatchC(value1,"0"))
            continue;

        ajStrAssignC(&key1,"serverVirtualSchema");
        value1 = ajTableFetchS(locs->Urls[i], key1);
        if(!value1)
            ajStrAssignC(&vschema,"default");
        else
            ajStrAssignS(&vschema,value1);

        ajStrAssignC(&key1,"name");
        value1 = ajTableFetchS(locs->Urls[i], key1);
        ajStrAssignS(&mart,value1);

        ajFmtPrintF(outf,"Mart = %S\n",mart);
        
        ajMartGetDatasets(seqin, mart);
        ajMartdatasetParse(seqin);

        ds = pmq->Dataset;

        for(j=0; j < ds->Nsets; ++j)
        {
            ajStrAssignC(&key1,"visible");
            value1 = ajTableFetchS(ds->Sets[j], key1);
            if(ajStrMatchC(value1,"0"))
                continue;
            
            ajStrAssignC(&key1,"name");
            value1 = ajTableFetchS(ds->Sets[j], key1);
            ajStrAssignS(&dataset,value1);
            ajFmtPrintF(outf,"    Dataset = %S\n",dataset);

            ajMartGetAttributesSchema(seqin,dataset,vschema);
            if(!ajMartattributesParse(seqin))
            {
                ajMartGetAttributesRetry(seqin,dataset);

                if(!ajMartattributesParse(seqin))
                {
                    ajWarn("Invalid attributes returned by server");
                    ajMartAttributeDel(&pmq->Atts);
                    pmq->Atts = ajMartAttributeNew();
                    continue;
                }
            }

            att = pmq->Atts;
            for(k=0; k < att->Natts; ++k)
            {
                ajStrAssignC(&key1,"format");
                value1 = ajTableFetchS(att->Attributes[k], key1);
                if(!ajStrMatchC(value1,"fasta"))
                    continue;

                ajStrAssignC(&key1,"tableName");
                value1 = ajTableFetchS(att->Attributes[k], key1);
                if(ajStrGetLen(value1))
                    continue;

                ajStrAssignC(&key1,"columnName");
                value1 = ajTableFetchS(att->Attributes[k], key1);
                if(ajStrGetLen(value1))
                    continue;

                ajStrAssignC(&key1,"name");
                value1 = ajTableFetchS(att->Attributes[k], key1);

                ajFmtPrintF(outf,"        Sequence Attribute = %S\n",value1);
            }

            ajMartAttributeDel(&pmq->Atts);
            pmq->Atts = ajMartAttributeNew();
            
        }
        
        ajMartDatasetDel(&pmq->Dataset);
        pmq->Dataset = ajMartDatasetNew();
    }
    

    ajStrDel(&key1);
    ajStrDel(&mart);
    

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&dataset);
    ajStrDel(&vschema);
    
    ajMartquerySeqinFree(seqin);

    ajSeqinDel(&seqin);
    ajFileClose(&outf);
    
    embExit();

    return 0;
}
