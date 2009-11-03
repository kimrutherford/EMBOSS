/* @source dbxflat application
**
** Index flatfile databases
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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





static AjBool dbxflat_ParseEmbl(EmbPBtreeEntry entry, AjPFile inf);
static AjBool dbxflat_ParseGenbank(EmbPBtreeEntry entry, AjPFile inf);

static AjBool dbxflat_NextEntry(EmbPBtreeEntry entry, AjPFile inf);

int global = 0;



/* @datastatic DbxflatPParser *************************************************
**
** Parser definition structure
**
** @alias DbxflatSParser
** @alias DbxflatOParser
**
** @attr Name [const char*] Parser name
** @attr Parser [(AjBool*)] Parser function
** @@
******************************************************************************/

typedef struct DbxflatSParser
{
    const char* Name;
    AjBool (*Parser) (EmbPBtreeEntry entry, AjPFile inf);
} DbxflatOParser;
#define DbxflatPParser DbxflatOParser*




static DbxflatOParser parser[] =
{
    {"EMBL",   dbxflat_ParseEmbl},
    {"SWISS",  dbxflat_ParseEmbl},
    {"GB",     dbxflat_ParseGenbank},
    {"REFSEQ", dbxflat_ParseGenbank},
    {NULL,     NULL}
};





/* @prog dbxflat **************************************************************
**
** Index a flat file database
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPBtreeEntry entry = NULL;
    
    AjPStr dbname   = NULL;
    AjPStr dbrs     = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr dbtype = NULL;

    AjPStr *fieldarray = NULL;
    
    ajint nfields;
    ajint nfiles;

    AjPStr tmpstr = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile inf = NULL;

    AjPStr word = NULL;
    
    AjPBtId  idobj  = NULL;
    AjPBtPri priobj = NULL;
    AjPBtHybrid hyb = NULL;
    

    embInit("dbxflat", argc, argv);

    dbtype     = ajAcdGetListSingle("idformat");
    fieldarray = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    dbrs       = ajAcdGetString("dbresource");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");

    entry = embBtreeEntryNew();
    tmpstr = ajStrNew();
    
    idobj   = ajBtreeIdNew();
    priobj  = ajBtreePriNew();
    hyb     = ajBtreeHybNew();
    

    nfields = embBtreeSetFields(entry,fieldarray);
    embBtreeSetDbInfo(entry,dbname,dbrs,datestr,release,dbtype,directory,
		      indexdir);

    embBtreeGetRsInfo(entry);

    nfiles = embBtreeGetFiles(entry,directory,filename,exclude);
    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);



    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	printf("Processing file %s\n",MAJSTRGETPTR(tmpstr));
	if(!(inf=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	

	while(dbxflat_NextEntry(entry,inf))
	{
	    if(entry->do_id)
	    {
		ajStrFmtLower(&entry->id);
		ajStrAssignS(&hyb->key1,entry->id);
		hyb->dbno = i;
		hyb->offset = entry->fpos;
		hyb->dups = 0;
		ajBtreeHybInsertId(entry->idcache,hyb);
	    }

	    if(entry->do_accession)
	    {
                while(ajListPop(entry->ac,(void **)&word))
                {
		    ajStrFmtLower(&word);
                    ajStrAssignS(&hyb->key1,word);
                    hyb->dbno = i;
		    hyb->offset = entry->fpos;
		    hyb->dups = 0;
		    ajBtreeHybInsertId(entry->accache,hyb);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_sv)
	    {
                while(ajListPop(entry->sv,(void **)&word))
                {
		    ajStrFmtLower(&word);
                    ajStrAssignS(&hyb->key1,word);
                    hyb->dbno = i;
		    hyb->offset = entry->fpos;
		    hyb->dups = 0;
		    ajBtreeHybInsertId(entry->svcache,hyb);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_keyword)
	    {
                while(ajListPop(entry->kw,(void **)&word))
                {
		    ajStrFmtLower(&word);
		    ajStrAssignS(&priobj->id,entry->id);
                    ajStrAssignS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(entry->kwcache, priobj);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_description)
	    {
                while(ajListPop(entry->de,(void **)&word))
                {
		    ajStrFmtLower(&word);
		    ajStrAssignS(&priobj->id,entry->id);
                    ajStrAssignS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(entry->decache, priobj);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_taxonomy)
	    {
                while(ajListPop(entry->tx,(void **)&word))
                {
		    ajStrFmtLower(&word);
		    ajStrAssignS(&priobj->id,entry->id);
                    ajStrAssignS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(entry->txcache, priobj);
		    ajStrDel(&word);
                }
	    }
	}
	
	ajFileClose(&inf);
    }
    


    embBtreeDumpParameters(entry);
    embBtreeCloseCaches(entry);
    

    embBtreeEntryDel(&entry);
    ajStrDel(&tmpstr);
    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&dbname);
    ajStrDel(&dbrs);
    ajStrDel(&release);
    ajStrDel(&datestr);
    ajStrDel(&directory);
    ajStrDel(&indexdir);
    ajStrDel(&dbtype);    

    nfields = 0;
    while(fieldarray[nfields])
	ajStrDel(&fieldarray[nfields++]);
    AJFREE(fieldarray);


    ajBtreeIdDel(&idobj);
    ajBtreePriDel(&priobj);
    ajBtreeHybDel(&hyb);


    embExit();

    return 0;
}




/* @funcstatic dbxflat_ParseEmbl **********************************************
**
** Parse the ID, accession from an EMBL entry.
**
** Reads to the end of the entry and then returns.
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseEmbl(EmbPBtreeEntry entry, AjPFile inf)
{
    AjPStr line = NULL;
    ajlong pos  = 0L;
    
    line = ajStrNewC("");
    
    while(!ajStrPrefixC(line,"//"))
    {
	pos = ajFileTell(inf);
	
	if(!ajFileReadLine(inf,&line))
	{
	    ajStrDel(&line);
	    return ajFalse;
	}
	
	if(ajStrPrefixC(line,"ID"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(line,"%*S%S",&entry->id);
	    ajStrTrimEndC(&entry->id, ";");
/*
	    ++global;
	    printf("%d. %s\n",global,ajStrGetPtr(entry->id));
*/
	    if(entry->do_sv)
		embBtreeEmblSV(line,entry->sv);
	}


	if(entry->do_sv)
	    if(ajStrPrefixC(line,"SV") ||
	       ajStrPrefixC(line,"IV"))	/* emblcds database format */
		embBtreeEmblAC(line,entry->sv);

	if(entry->do_accession)
	    if(ajStrPrefixC(line,"AC") ||
	       ajStrPrefixC(line,"PA"))	/* emblcds database format */
		embBtreeEmblAC(line,entry->ac);
	
	if(entry->do_keyword)
	    if(ajStrPrefixC(line,"KW"))
		embBtreeEmblKW(line,entry->kw,entry->kwlen);

	if(entry->do_description)
	    if(ajStrPrefixC(line,"DE"))
		embBtreeEmblDE(line,entry->de,entry->delen);

	if(entry->do_taxonomy)
	    if(ajStrPrefixC(line,"OC") || ajStrPrefixC(line,"OS"))
		embBtreeEmblTX(line,entry->tx,entry->txlen);
    }
    

    ajStrDel(&line);
    
    return ajTrue;
}




/* @funcstatic dbxflat_ParseGenbank *******************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [w] entry [EmbPBtreeEntry] entry
** @param [u] inf [AjPFile] Input file
**
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxflat_ParseGenbank(EmbPBtreeEntry entry, AjPFile inf)
{
    AjPStr line = NULL;
    ajlong pos  = 0L;
    AjBool ret = ajTrue;
    
    AjPStr sumline = NULL;
    
    line = ajStrNewC("");
    sumline = ajStrNew();

    
    while(!ajStrPrefixC(line,"//") && ret)
    {
	if(ajStrPrefixC(line,"LOCUS"))
	{
	    entry->fpos = pos;
	    ajFmtScanS(line,"%*S%S",&entry->id);
	}
	
	if(entry->do_sv)
	    if(ajStrPrefixC(line,"VERSION"))
		embBtreeGenBankAC(line,entry->sv);
	
	if(entry->do_accession)
	    if(ajStrPrefixC(line,"ACCESSION"))
		embBtreeGenBankAC(line,entry->ac);
	
	if(entry->do_keyword)
	    if(ajStrPrefixC(line,"KEYWORDS"))
	    {
		ajStrAssignS(&sumline,line);
		ret = ajFileReadLine(inf,&line);
		while(ret && *MAJSTRGETPTR(line)==' ')
		{
		    ajStrAppendS(&sumline,line);
		    ret = ajFileReadLine(inf,&line);
		}
		ajStrRemoveWhiteExcess(&sumline);
		embBtreeGenBankKW(sumline,entry->kw,entry->kwlen);
		continue;
	    }

	if(entry->do_description)
	    if(ajStrPrefixC(line,"DEFINITION"))
	    {
		ajStrAssignS(&sumline,line);
		ret = ajFileReadLine(inf,&line);
		while(ret && *MAJSTRGETPTR(line)==' ')
		{
		    ajStrAppendS(&sumline,line);
		    ret = ajFileReadLine(inf,&line);
		}
		ajStrRemoveWhiteExcess(&sumline);
		embBtreeGenBankDE(sumline,entry->de,entry->delen);
		continue;
	    }
	

	if(entry->do_taxonomy)
	    if(ajStrPrefixC(line,"SOURCE"))
	    {
		ret = ajFileReadLine(inf,&line);
		ajStrAppendC(&line,";");
		while(ret && *MAJSTRGETPTR(line)==' ')
		{
		    ajStrAppendS(&sumline,line);
		    ret = ajFileReadLine(inf,&line);
		}
		ajStrRemoveWhiteExcess(&sumline);
		embBtreeGenBankTX(sumline,entry->tx,entry->txlen);
		continue;
	    }
	

	pos = ajFileTell(inf);

	if(!ajFileReadLine(inf,&line))
	    ret = ajFalse;
    }

    ajStrDel(&line);
    ajStrDel(&sumline);
    
    return ret;
}




/* @funcstatic dbxflat_NextEntry ********************************************
**
** Parse the next entry from a flatfile
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxflat_NextEntry(EmbPBtreeEntry entry, AjPFile inf)
{
    static AjBool init = AJFALSE;
    static ajint  nparser = -1;
    ajint i;
    
    if(!init)
    {
	entry->fpos = 0L;
	for(i=0; parser[i].Name && nparser == -1; ++i)
	    if(ajStrMatchC(entry->dbtype, parser[i].Name))
		nparser = i;
	if(nparser == -1)
	    ajFatal("Database format (%S) unknown",entry->dbtype);
	init = ajTrue;
    }
    

    if(!parser[nparser].Parser(entry,inf))
	return ajFalse;
    


    return ajTrue;
}
