/* @source seqxrefget application
**
** Return a sequence
**
** @author Copyright (C) Peter Rice
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




/* @prog seqxrefget ***********************************************************
**
** Reads sequences and returns commands to fetch their cross-references
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPFile outf;
    AjPSeq seq = NULL;
    AjPList xrefs = NULL;
    ajuint nrefs;
    AjPSeqXref xref = NULL;
    AjBool *dbtypes = NULL;
    AjPTable typesTable = NULL;
    AjPStr dbname = NULL;
    AjPStr dbvalue = NULL;
    ajuint itype = 0;
    AjPResource drcat = NULL;
    AjIList iter;
    AjPResquery resqry = NULL;

    embInit("seqxrefget", argc, argv);

    outf = ajAcdGetOutfile("outfile");
    seqall = ajAcdGetSeqall("sequence");

    xrefs = ajListNew();
    typesTable = ajTablestrNew(250);

    while(ajSeqallNext(seqall, &seq))
    {
        nrefs = ajSeqGetXrefs(seq, xrefs);
        ajSeqxreflistSort(xrefs);

        ajFmtPrintF(outf, "#%S: %u\n", ajSeqGetUsaS(seq), nrefs);

        while(ajListPop(xrefs, (void**)&xref))
        {
            if(ajStrPrefixCaseS(xref->Id, xref->Db) &&
               ajStrGetCharPos(xref->Id,
                               ajStrGetLen(xref->Db)) == ':')
                ajStrCutStart(&xref->Id,
                              1+ajStrGetLen(xref->Db));

            if(ajStrPrefixCaseS(xref->Secid, xref->Db) &&
               ajStrGetCharPos(xref->Secid,
                               ajStrGetLen(xref->Db)) == ':')
                ajStrCutStart(&xref->Secid,
                              1+ajStrGetLen(xref->Db));

            if(ajStrPrefixCaseS(xref->Terid, xref->Db) &&
               ajStrGetCharPos(xref->Terid,
                               ajStrGetLen(xref->Db)) == ':')
                ajStrCutStart(&xref->Terid,
                              1+ajStrGetLen(xref->Db));

            if(ajStrPrefixCaseS(xref->Quatid, xref->Db) &&
               ajStrGetCharPos(xref->Quatid,
                               ajStrGetLen(xref->Db)) == ':')
                ajStrCutStart(&xref->Quatid,
                              1+ajStrGetLen(xref->Db));

            dbtypes = ajTableFetchmodS(typesTable, xref->Db);
            if(!dbtypes)
            {
                AJCNEW0(dbtypes, AJDATATYPE_MAX);
                ajStrAssignS(&dbname, xref->Db);
                if(ajNamAliasDatabase(&dbname))
                {
                    itype = 0;
                    ajNamDbGetAttrC(dbname, "type", &dbvalue);
                    if(ajNamDbGetType(dbname, &itype))
                        dbtypes[itype] = ajTrue;
                }
                else 
                {
                    drcat = ajResourceNewDrcat(xref->Db);
                    if(drcat)
                    {
                        iter = ajListIterNewread(drcat->Query);
                        while(!ajListIterDone(iter)) 
                        {
                            resqry = ajListIterGet(iter);
                            if(ajSeqinformatTerm(resqry->FormatTerm))
                                dbtypes[AJDATATYPE_SEQUENCE] = ajTrue;
                            else if(ajFeattabinformatTerm(resqry->FormatTerm))
                                dbtypes[AJDATATYPE_FEATURES] = ajTrue;
                            else if(ajOboinformatTerm(resqry->FormatTerm))
                                dbtypes[AJDATATYPE_OBO] = ajTrue;
                            else if(ajTextinformatTerm(resqry->FormatTerm))
                                dbtypes[AJDATATYPE_TEXT] = ajTrue;
                            else
                                dbtypes[AJDATATYPE_URL] = ajTrue;
                        }
                        ajListIterDel(&iter);
                    }
                    
                    ajResourceDel(&drcat);
                }

                ajTablePut(typesTable, ajStrNewS(xref->Db), dbtypes);
            }
            
            if(dbtypes[AJDATATYPE_SEQUENCE])
                ajFmtPrintF(outf, "entret %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_FEATURES])
                ajFmtPrintF(outf, "feattext %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_OBO])
                ajFmtPrintF(outf, "ontotext %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_RESOURCE])
                ajFmtPrintF(outf, "drtext %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_ASSEMBLY])
                ajFmtPrintF(outf, "assemtext %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_TAXON])
                ajFmtPrintF(outf, "taxtext %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_TEXT])
                ajFmtPrintF(outf, "textget %S:%S\n", xref->Db, xref->Id);
            else if (dbtypes[AJDATATYPE_URL])
                ajFmtPrintF(outf, "urlget %S:%S\n", xref->Db, xref->Id);
            else
                ajFmtPrintF(outf, "# no route to %S:%S\n", xref->Db, xref->Id);
            ajSeqxrefDel(&xref);
        }
    }

    ajListFree(&xrefs);
    ajSeqDel(&seq);
    
    ajFileClose(&outf);
    ajSeqallDel(&seqall);
    ajTableDelValdel(&typesTable, ajMemFree);

    ajStrDel(&dbvalue);
    ajStrDel(&dbname);

    embExit();

    return 0;
}
