/* @source dbxreport application
**
** Statistics for a dbx database
**
** @author Copyright (C) 2010 Peter Rice (pmr@ebi.ac.uk)
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




/* @datastatic DbxreportPData *************************************************
**
** Index file names
**
** @alias DbxreportSData
** @alias DbxreportOData
**
** @attr cache [AjPBtcache] Index cache
** @attr refs [ajulong*] Reference count
** @attr nroot [ajuint] Number of root pages
** @attr nnroot [ajuint] Number of numeric root pages
** @attr ninternal [ajuint] Number of internal node pages
** @attr nleaf [ajuint] Number of leaf node pages
** @attr nbucket [ajuint] Number of bucket pages
** @attr noverflow [ajuint] Number of overflow pages
** @attr npribucket [ajuint] Number of primary key bucket pages
** @attr nsecbucket [ajuint] Number of secondary key bucket pages
** @attr nnumbucket [ajuint] Number of numbucket pages
** @attr nunknown [ajuint] Number of unknown type pages
** @attr nkeys [ajuint] Number of keys
** @attr nover [ajuint] Number of node overflow pages
** @attr nbkeys [ajuint] Number of bucket key pages
** @attr nbdups [ajuint]  Number of bucket duplicate pages
** @attr nbxtra [ajuint]   Number of bucket extra pages
** @attr nbover [ajuint] Number of bucket overflow pages
** @attr npkeys [ajuint]  Number of primary keys
** @attr npover [ajuint]  Number of primary key overflow pages
** @attr nskeys [ajuint]  Number of secondary keys
** @attr nsover [ajuint]  Number of secondary key overflow pages
** @attr nnkeys [ajuint]  Number of numeric keys
** @attr nndups [ajuint]  Number of numeric key duplicates
** @attr nnover [ajuint]  Number of numeric key overflow pages
** @attr nlkeys [ajuint] Number of leaf keys
** @attr nlover [ajuint] Number of leaf overflow pages
** @attr nunused [ajuint] Number of unused pages
** @attr freespace [ajulong] Free space total for all pages
** @attr pagecount [ajulong] Page count
** @attr totsize   [ajulong] Total index size
** @@
******************************************************************************/

typedef struct DbxreportSData
{
    AjPBtcache cache;
    ajulong* refs;
    ajuint nroot;
    ajuint nnroot;
    ajuint ninternal;
    ajuint nleaf;
    ajuint nbucket;
    ajuint noverflow;
    ajuint npribucket;
    ajuint nsecbucket;
    ajuint nnumbucket;
    ajuint nunknown;
    ajuint nkeys;
    ajuint nover;
    ajuint nbkeys;
    ajuint nbdups;
    ajuint nbxtra;
    ajuint nbover;
    ajuint npkeys;
    ajuint npover;
    ajuint nskeys;
    ajuint nsover;
    ajuint nnkeys;
    ajuint nndups;
    ajuint nnover;
    ajuint nlkeys;
    ajuint nlover;
    ajuint nunused;
    ajulong freespace;
    ajulong pagecount;
    ajulong totsize;
} DbxreportOData;
#define DbxreportPData DbxreportOData*




/* @datastatic DbxreportNames *************************************************
**
** Index file names
**
** @alias DbxreportSNames
** @alias DbxreportONames
**
** @attr name [const char*] Field name
** @attr iname [const char*] Field index name
** @attr isid [AjBool] Field is an identifier (false for keywords)
** @@
******************************************************************************/

typedef struct DbxreportSNames
{
    const char* name;
    const char* iname;
    AjBool isid;
} DbxreportONames;
#define DbxreportPNames DbxreportONames*




/* @prog dbxreport ************************************************************
**
** Statistics for a dbx index
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr   dbname = NULL;
    AjPStr   idir   = NULL;
    AjPStr   fields = NULL;
    AjPFile  outf   = NULL;

    AjBool full;
    
    ajuint i;
    AjBool ok = ajTrue;

    AjPStr  *field  = NULL;
    ajuint nindex;
    AjPBtpage page = NULL;
    DbxreportPData dbxdata = NULL;
    AjBool compressed;
    ajulong pagecount;
    ajulong totsize;
    ajulong ipage;
    ajulong pageoffset = 0L;
    const char* nodetype;
    ajuint nkeys;
    ajuint ndups;
    ajuint nextra;
    ajuint nover;
    ajuint freespace;
    ajuint totlen = 0;
    ajulong nfound;
    ajulong nfoundall;
    AjPList fieldlist = NULL;
    AjPStr tmpstr = NULL;
    AjPStrTok handle = NULL;
    ajulong *pagepos = NULL;
    ajulong *pageindex = NULL;
    AjPTable newpostable;
    ajuint pagesize;
    const AjPStr fieldext = NULL;

    embInit("dbxreport", argc, argv);
    
    dbname = ajAcdGetString("dbname");
    idir   = ajAcdGetDirectoryName("indexdir");
    fields = ajAcdGetString("fields");
    outf   = ajAcdGetOutfile("outfile");
    full   = ajAcdGetBoolean("fullreport");
    
    fieldlist = ajListstrNew();
    handle = ajStrTokenNewC(fields, ",");
    while(ajStrTokenNextParse(&handle, &tmpstr))
        ajListPushAppend(fieldlist, ajStrNewS(tmpstr));
    ajStrDel(&tmpstr);
    ajStrTokenDel(&handle);

    nindex = ajListstrToarray(fieldlist, &field);
    AJCNEW0(dbxdata, nindex);

    ajListFree(&fieldlist);

    if(!ajStrGetLen(idir))
    {
/* if not forced by the user, find the index directory for the database */
        if(!ajNamDbGetIndexdir(dbname, &idir))
            ajDie("Database '%S' has no indexdirectory defined", dbname);
    }

    for(i=0; i<nindex;i++)
    {
        fieldext = ajBtreeFieldGetExtensionS(field[i]);
        dbxdata[i].cache = ajBtreeCacheNewReadS(dbname,fieldext,idir);
        if(!dbxdata[i].cache)
        {
            ajErr("No '%S' index found\n", field[i]);
            ok = ajFalse;
        }
    }

    if(!ok)
        embExitBad();

    for(i=0; i<nindex;i++)
    {
        fieldext = ajBtreeFieldGetExtensionS(field[i]);
        ajFmtPrintF(outf,
                    "Index '%S' (.%S)\n",
                    field[i], fieldext);
        ajFmtPrintF(outf,
                    " compressed: %4B (Compressed)\n",
                    dbxdata[i].cache->compressed);
        ajFmtPrintF(outf,
                    "  cachesize: %4u (Size of cache)\n",
                    dbxdata[i].cache->cachesize);
        ajFmtPrintF(outf,
                    "   pagesize: %4u (Page size)\n",
                    dbxdata[i].cache->pagesize);
        ajFmtPrintF(outf,
                    "  pagecount: %4Lu (Page count)\n",
                    dbxdata[i].cache->pagecount);
        ajFmtPrintF(outf,
                    "      order: %4u (Tree order)\n",
                    dbxdata[i].cache->order);
        ajFmtPrintF(outf,
                    "     sorder: %4u (Order of secondary tree)\n",
                    dbxdata[i].cache->sorder);
        ajFmtPrintF(outf,
                    "      level: %4u (Level of tree)\n",
                    dbxdata[i].cache->level);
        ajFmtPrintF(outf,
                    "     slevel: %4u (Level of secondary tree)\n",
                    dbxdata[i].cache->slevel);
        ajFmtPrintF(outf,
                    "       fill: %4u (Entries per bucket)\n",
                    dbxdata[i].cache->nperbucket);
        ajFmtPrintF(outf,
                    "      sfill: %4u (Entries per secondary bucket)\n",
                    dbxdata[i].cache->snperbucket);
        ajFmtPrintF(outf,
                    "    kwlimit: %4u (Max key size)\n",
                    dbxdata[i].cache->kwlimit);
        ajFmtPrintF(outf,
                    "      count: %4Lu (Unique entries in index)\n",
                    dbxdata[i].cache->countunique);
        ajFmtPrintF(outf,
                    "  fullcount: %4Lu (Total entries in index)\n",
                    dbxdata[i].cache->countall);

        pagecount = dbxdata[i].cache->pagecount;
        totsize = ajBtreeGetTotsize(dbxdata[i].cache);
        pagesize = dbxdata[i].cache->pagesize;
        compressed = dbxdata[i].cache->compressed;
        dbxdata[i].pagecount = pagecount;
        dbxdata[i].totsize = totsize;

        AJCNEW0(dbxdata[i].refs,pagecount);
        AJCNEW0(pagepos,pagecount);
        AJCNEW0(pageindex,pagecount);

        newpostable = ajTableulongNewConst((ajuint)pagecount);
        ++dbxdata[i].refs[0];

        pageoffset = 0L;
        for(ipage=0L; ipage < pagecount; ipage++)
        {
            pagepos[ipage] = pageoffset;
            pageindex[ipage] = ipage;
            page = ajBtreeCacheRead(dbxdata[i].cache, pagepos[ipage]);
            nodetype = ajBtreePageGetTypename(page);
            ajTablePut(newpostable, &pagepos[ipage],
                       &pageindex[ipage]);
            if(compressed)
                pageoffset += ajBtreePageGetSize(page);
            else
                pageoffset += pagesize;
        }

        if(!ajBtreeCacheIsSecondary(dbxdata[i].cache))
        {
            ajFmtPrintF(outf,
                        "Identifier index '%S' index found with %Lu pages\n",
                        field[i], pagecount);
            for(ipage=0L; ipage < pagecount; ipage++)
            {
                page = ajBtreeCacheRead(dbxdata[i].cache, pagepos[ipage]);
                nodetype = ajBtreePageGetTypename(page);
                GBT_TOTLEN(page->buf,&totlen);
                switch((int) *nodetype)
                {
                    case 'f':
                        dbxdata[i].nunused++;
                        ajFmtPrintF(outf,
                                    "Freed page type for page %Lu at %Lu",
                                    ipage, ipage * pagesize);
                        break;
                    case 'r':
                        if(totlen)
                            dbxdata[i].nroot++;
                        else
                            dbxdata[i].nnroot++;
                        ajBtreeStatNode(dbxdata[i].cache, page, full,
                                        &nkeys, &nover, &freespace,
                                        dbxdata[i].refs, newpostable);
                        dbxdata[i].nkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nover += nover;
                        break;
                    case 'i':
                        dbxdata[i].ninternal++;
                        ajBtreeStatNode(dbxdata[i].cache, page, full,
                                        &nkeys, &nover, &freespace,
                                        dbxdata[i].refs, newpostable);
                        dbxdata[i].nkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nover += nover;
                        break;
                    case 'l':
                        dbxdata[i].nleaf++;
                        ajBtreeStatNode(dbxdata[i].cache, page, full,
                                        &nkeys, &nover, &freespace,
                                        dbxdata[i].refs, newpostable);
                        dbxdata[i].nlkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nlover += nover;
                        break;
                    case 'b':
                        dbxdata[i].nbucket++;
                        ajBtreeStatBucket(dbxdata[i].cache, page, full,
                                          &nkeys, &ndups, &nextra,
                                          &nover, &freespace,
                                          dbxdata[i].refs, newpostable);
                        dbxdata[i].nbkeys += nkeys;
                        dbxdata[i].nbdups += ndups;
                        dbxdata[i].nbxtra += nextra;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nbover += nover;
                        break;
                    case 'o':
                        dbxdata[i].noverflow++;
                        ajFmtPrintF(outf,
                                    "Overflow page type for page %Lu at %Lu",
                                    ipage, ipage * pagesize);
                        break;
                    case 'p':
                        dbxdata[i].npribucket++;
                        ajBtreeStatPribucket(dbxdata[i].cache, page, full,
                                             &nkeys, &nover, &freespace,
                                             dbxdata[i].refs, newpostable);
                        dbxdata[i].npkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].npover += nover;
                        break;
                    case 's':
                        dbxdata[i].nsecbucket++;
                        ajBtreeStatSecbucket(dbxdata[i].cache, page, full,
                                             &nkeys, &nover, &freespace);
                        dbxdata[i].nskeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nsover += nover;
                        break;
                    case 'n':
                        dbxdata[i].nnumbucket++;
                        ajBtreeStatNumbucket(dbxdata[i].cache, page, full,
                                             &nkeys, &nover, &freespace);
                        dbxdata[i].nnkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nnover += nover;
                        break;
                    case 'u':
                    default:
                        dbxdata[i].nunknown++;
                        ajFmtPrintF(outf,
                                    "Unknown page type for page %Lu at %Lu\n",
                                    ipage, ipage * pagesize);
                        break;
                }
            }
        }
        else
        {
            ajFmtPrintF(outf,
                        "Secondary index '%S' index found with %Lu pages\n",
                        field[i], pagecount);
            for(ipage=0L; ipage < pagecount; ipage++)
            {
                page = ajBtreeCacheRead(dbxdata[i].cache, pagepos[ipage]);
                nodetype = ajBtreePageGetTypename(page);

                switch((int) *nodetype)
                {
                    case 'f':
                        dbxdata[i].nunused++;
                        ajFmtPrintF(outf,
                                    "Freed page type for page %Lu at %Lu",
                                    ipage, ipage * pagesize);
                        break;
                    case 'r':
                        if(totlen)
                            dbxdata[i].nroot++;
                        else
                            dbxdata[i].nnroot++;
                        ajBtreeStatNode(dbxdata[i].cache, page, full,
                                        &nkeys, &nover, &freespace,
                                        dbxdata[i].refs, newpostable);
                        dbxdata[i].nkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nover += nover;
                        break;
                    case 'i':
                        dbxdata[i].ninternal++;
                        ajBtreeStatNode(dbxdata[i].cache, page, full,
                                        &nkeys, &nover, &freespace,
                                        dbxdata[i].refs, newpostable);
                        dbxdata[i].nkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nover += nover;
                        break;
                    case 'l':
                        dbxdata[i].nleaf++;
                        ajBtreeStatNode(dbxdata[i].cache, page, full,
                                        &nkeys, &nover, &freespace,
                                        dbxdata[i].refs, newpostable);
                        dbxdata[i].nlkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nlover += nover;
                        break;
                    case 'b':
                        dbxdata[i].nbucket++;
                        ajBtreeStatBucket(dbxdata[i].cache, page, full,
                                          &nkeys, &ndups, &nextra,
                                          &nover, &freespace,
                                          dbxdata[i].refs, newpostable);
                        dbxdata[i].nbkeys += nkeys;
                        dbxdata[i].nbdups += ndups;
                        dbxdata[i].nbxtra += nextra;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nbover += nover;
                        break;
                    case 'o':
                        dbxdata[i].noverflow++;
                        ajFmtPrintF(outf,
                                    "Overflow page type for page %Lu at %Lu",
                                    ipage, ipage * pagesize);
                        break;
                    case 'p':
                        dbxdata[i].npribucket++;
                        ajBtreeStatPribucket(dbxdata[i].cache, page, full,
                                             &nkeys, &nover, &freespace,
                                             dbxdata[i].refs, newpostable);
                        dbxdata[i].npkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].npover += nover;
                        break;
                    case 's':
                        dbxdata[i].nsecbucket++;
                        ajBtreeStatSecbucket(dbxdata[i].cache, page, full,
                                             &nkeys, &nover, &freespace);
                        dbxdata[i].nskeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nsover += nover;
                        break;
                    case 'n':
                        dbxdata[i].nnumbucket++;
                        ajBtreeStatNumbucket(dbxdata[i].cache, page, full,
                                             &nkeys, &nover, &freespace);
                        dbxdata[i].nnkeys += nkeys;
                        dbxdata[i].freespace += (ajulong) freespace;
                        dbxdata[i].nnover += nover;
                        break;
                    case 'u':
                    default:
                        dbxdata[i].nunknown++;
                        ajFmtPrintF(outf,
                                    "Unknown page type for page %Lu",
                                    ipage * pagesize);
                        break;
                }
            }
        }
        for(ipage=0L; ipage < pagecount; ipage++)
        {
            if(!dbxdata[i].refs[ipage]) 
            {
                page = ajBtreeCacheRead(dbxdata[i].cache, pagepos[ipage]);
                nodetype = ajBtreePageGetTypename(page);
                ajFmtPrintF(outf, "Unused page %Lu at %Lu type '%s'\n",
                            ipage, pagepos[ipage], nodetype);
            }
        }
        
        if(dbxdata[i].nroot)
            ajFmtPrintF(outf, "      COUNT: %u\n", dbxdata[i].cache->countunique);
        if(dbxdata[i].nroot)
            ajFmtPrintF(outf, "   COUNTALL: %u\n", dbxdata[i].cache->countall);
        if(dbxdata[i].nroot)
            ajFmtPrintF(outf, "       root: %u\n", dbxdata[i].nroot);
        if(dbxdata[i].nnroot)
            ajFmtPrintF(outf, "    numroot: %u\n", dbxdata[i].nnroot);
        if(dbxdata[i].ninternal)
            ajFmtPrintF(outf, "   internal: %u\n", dbxdata[i].ninternal);
        if(dbxdata[i].nleaf)
            ajFmtPrintF(outf, "       leaf: %u\n", dbxdata[i].nleaf);
        if(dbxdata[i].nbucket)
            ajFmtPrintF(outf, "     bucket: %u\n", dbxdata[i].nbucket);
        if(dbxdata[i].noverflow)
            ajFmtPrintF(outf, "   overflow: %u\n", dbxdata[i].noverflow);
        if(dbxdata[i].nnumbucket)
            ajFmtPrintF(outf, "  numbucket: %u\n", dbxdata[i].nnumbucket);
        if(dbxdata[i].npribucket)
            ajFmtPrintF(outf, "  pribucket: %u\n", dbxdata[i].npribucket);
        if(dbxdata[i].nsecbucket)
            ajFmtPrintF(outf, "  secbucket: %u\n", dbxdata[i].nsecbucket);
        if(dbxdata[i].nunknown)
            ajFmtPrintF(outf, "    unknown: %u\n", dbxdata[i].nunknown);
        if(dbxdata[i].nkeys)
            ajFmtPrintF(outf, "...       keys: %u\n", dbxdata[i].nkeys);
        if(dbxdata[i].nlkeys)
            ajFmtPrintF(outf, "...   leafkeys: %u\n", dbxdata[i].nlkeys);
        if(dbxdata[i].nbkeys)
            ajFmtPrintF(outf, "...      bkeys: %u\n", dbxdata[i].nbkeys);
        if(dbxdata[i].nbdups)
            ajFmtPrintF(outf, "...      bdups: %u\n", dbxdata[i].nbdups);
        if(dbxdata[i].nbxtra)
            ajFmtPrintF(outf, "...      bxtra: %u\n", dbxdata[i].nbxtra);
        if(dbxdata[i].npkeys)
            ajFmtPrintF(outf, "...      pkeys: %u\n", dbxdata[i].npkeys);
        if(dbxdata[i].nskeys)
            ajFmtPrintF(outf, "...      skeys: %u\n", dbxdata[i].nskeys);
        if(dbxdata[i].nnkeys)
            ajFmtPrintF(outf, "...      nkeys: %u (%u)\n",
                        dbxdata[i].nnkeys, dbxdata[i].nndups);
        if(dbxdata[i].nover)
            ajFmtPrintF(outf, "...    overflows_used: %u\n", dbxdata[i].nover);
        if(dbxdata[i].nlover)
            ajFmtPrintF(outf, "...leafoverflows_used: %u\n", dbxdata[i].nlover);
        if(dbxdata[i].nbover)
            ajFmtPrintF(outf, "...   boverflows_used: %u\n", dbxdata[i].nbover);
        if(dbxdata[i].npover)
            ajFmtPrintF(outf, "...   poverflows_used: %u\n", dbxdata[i].npover);
        if(dbxdata[i].nsover)
            ajFmtPrintF(outf, "...   soverflows_used: %u\n", dbxdata[i].nsover);
        if(dbxdata[i].nnover)
            ajFmtPrintF(outf, "...   noverflows_used: %u\n", dbxdata[i].nnover);
        if(dbxdata[i].nunused)
            ajFmtPrintF(outf, "... pages_unused: %u\n", dbxdata[i].nunused);
        if(dbxdata[i].totsize)
            ajFmtPrintF(outf, "...   total_size: %Lu\n",
                        dbxdata[i].totsize);
        if(dbxdata[i].freespace)
            ajFmtPrintF(outf, "...    freespace: %Lu, %.1f of uncompressed\n",
                        dbxdata[i].freespace,
                        100.0 * (float)dbxdata[i].freespace/
                        (float)(pagesize*pagecount));
        if(!ajBtreeCacheIsSecondary(dbxdata[i].cache))
        {
            nfound = dbxdata[i].nbkeys;
            nfoundall = nfound + dbxdata[i].nbxtra;
        }
        else
        {
            nfound = dbxdata[i].npkeys;
            nfoundall = dbxdata[i].nskeys;
        }
        ajFmtPrintF(outf, "      FOUND: %Lu\n", nfound);
        ajFmtPrintF(outf, "   FOUNDALL: %Lu\n", nfoundall);

        ajTableFree(&newpostable);
        AJFREE(pagepos);
        AJFREE(pageindex);
   }

    ajStrDel(&dbname);
    ajStrDel(&fields);
    ajStrDel(&idir);

    ajFileClose(&outf);
    for(i=0;i<nindex;i++)
    {
        ajBtreeCacheDel(&dbxdata[i].cache);
        AJFREE(dbxdata[i].refs);
        ajStrDel(&field[i]);
    }
    AJFREE(field);
    AJFREE(dbxdata);

    embExit();
    
    return 0;
}
