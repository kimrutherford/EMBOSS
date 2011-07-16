/******************************************************************************
** @source AJAX taxonomy database functions
**
** These functions control all aspects of AJAX taxonomy database access
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Oct 2010 pmr first version
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

#include "ajax.h"
#include "ajtaxdb.h"

#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <errno.h>
#include <signal.h>


/* @datastatic TaxPEmbossQry *************************************************
**
** Btree 'emboss' query structure
**
** @alias TaxSEmbossQry
** @alias TaxOEmbossQry
**
** @attr idcache [AjPBtcache] ID cache
** @attr Caches [AjPList] Caches for each query field
** @attr files [AjPStr*] database filenames
** @attr reffiles [AjPStr*] database reference filenames
** @attr Skip [AjBool*] files numbers to exclude
** @attr libs [AjPFile] Primary (database source) file
** @attr libr [AjPFile] Secondary (database bibliographic source) file
** @attr div [ajuint] division number of currently open database file
** @attr nentries [ajint] number of entries in the filename array(s)
**                        -1 when done
** @attr Samefile [AjBool] true if the same file is passed to
**                         ajFilebuffReopenFile
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TaxSEmbossQry
{
    AjPBtcache idcache;
    AjPList Caches;

    AjPStr *files;
    AjPStr *reffiles;
    AjBool *Skip;

    AjPFile libs;
    AjPFile libr;

    ajuint div;
    ajint nentries;

    AjBool Samefile;
    char Padding[4];
} TaxOEmbossQry;

#define TaxPEmbossQry TaxOEmbossQry*




static AjBool 	  taxEmbossTaxAll(AjPTaxin taxin);
static void       taxEmbossTaxLoadBuff(AjPTaxin taxin);
static AjBool     taxEmbossTaxReadNode(AjPTaxin taxin);
static AjBool     taxEmbossTaxReadRef(AjPTaxin taxin);
static AjBool	  taxEmbossOpenCache(AjPQuery qry, const char *ext,
				     AjPBtcache *cache);
static AjBool     taxEmbossQryClose(AjPQuery qry);
static AjBool     taxEmbossQryEntry(AjPQuery qry);
static AjBool     taxEmbossQryNext(AjPQuery qry);
static AjBool     taxEmbossQryOpen(AjPQuery qry);
static AjBool     taxEmbossQryQuery(AjPQuery qry);
static AjBool     taxEmbossQryReuse(AjPQuery qry);






static AjBool     taxAccessEmbossTax(AjPTaxin taxin);




/* @funclist taxAccess ******************************************************
**
** Functions to access each database or taxonomy access method
**
******************************************************************************/

static AjOTaxAccess taxAccess[] =
{
  /* Name      AccessFunction   FreeFunction
     Qlink    Description
     Alias    Entry    Query    All      Chunk */
    {"embosstax",	 taxAccessEmbossTax, NULL,
     "",      "emboss dbxtax indexed",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE
    },
    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE},
};




/* @section B+tree NCBI Taxonomy Indexing *************************************
**
** These functions manage the EMBOSS B+tree NCBI taxonomy access methods.
**
******************************************************************************/




/* @funcstatic taxAccessEmbossTax ********************************************
**
** Reads taxon(s) from an NCBI taxonomy database, using B+tree index
** files. Returns with the file pointer set to the position in the
** buffered taxonomy file.
**
** @param [u] taxin [AjPTaxin] Taxon input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool taxAccessEmbossTax(AjPTaxin taxin)
{
    AjBool retval = ajFalse;

    AjPQuery qry;
    TaxPEmbossQry qryd = NULL;

    
    qry = taxin->Input->Query;
    qryd = qry->QryData;
    ajDebug("taxAccessEmbossTax type %d\n", qry->QueryType);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(qry->QueryType == AJQUERY_ALL)
	return taxEmbossTaxAll(taxin);


    if(!qry->QryData)
    {
	if(!taxEmbossQryOpen(qry))
            return ajFalse;

	qryd = qry->QryData;
	taxin->Input->Single = ajTrue;
	ajFilebuffDel(&taxin->Input->Filebuff);
	taxin->Input->Filebuff = ajFilebuffNewNofile();
	
	if(qry->QueryType == AJQUERY_ENTRY)
	{
	    if(!taxEmbossQryEntry(qry))
		ajDebug("embosstax B+tree Entry failed\n");
	}

	if(qry->QueryType == AJQUERY_QUERY)
	{
	    if(!taxEmbossQryQuery(qry))
		ajDebug("embosstax B+tree Query failed\n");
	}
    }
    else
    {
        if(!taxEmbossQryReuse(qry))
        {
            taxEmbossQryClose(qry);
            return ajFalse;
        }
	ajFilebuffClear(taxin->Input->Filebuff, -1);
    }

    if(ajListGetLength(qry->ResultsList))
    {
	retval = taxEmbossQryNext(qry);

	if(retval)
        {
	    taxEmbossTaxLoadBuff(taxin);
            ajStrAssignS(&taxin->Input->Db, qry->DbName);
        }
    }

    if(!ajListGetLength(qry->ResultsList)) /* could be emptied by code above */
    {
	taxEmbossQryClose(qry);
	ajFileClose(&qryd->libs);
	ajFileClose(&qryd->libr);
    }

    return retval;
}




/* @funcstatic taxEmbossTaxAll ***********************************************
**
** Opens the first or next taxonomy file for further reading
**
** @param [u] taxin [AjPTaxin] Taxon input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool taxEmbossTaxAll(AjPTaxin taxin)
{
    AjPQuery qry;
    TaxPEmbossQry qryd;
    static ajint i   = 0;
    AjPStr name      = NULL;
    AjBool ok        = ajFalse;
/*
    AjPStrTok handle = NULL;
    AjPStr wildname  = NULL;
    AjBool found     = ajFalse;
*/
    
    qry = taxin->Input->Query;
    qryd = qry->QryData;

    ajDebug("taxEmbossTaxAll\n");


    if(!qry->QryData)
    {
	ajDebug("taxEmbossTaxAll initialising\n");

	qry->QryData = AJNEW0(qryd);
	qryd = qry->QryData;
	i = -1;
	ajBtreeReadEntriesS(qry->DbAlias,qry->IndexDir,
                            qry->Directory,
                            &qryd->files,&qryd->reffiles);

	taxin->Input->Single = ajTrue;
    }

    qryd = qry->QryData;
    ajFilebuffDel(&taxin->Input->Filebuff);
    taxin->Input->Filebuff = ajFilebuffNewNofile();

    if(!qryd->libs)
    {
	while(!ok && qryd->files[++i])
	{
	    ajStrAssignS(&name,qryd->files[i]);
	    if(ajFilenameTestInclude(name, qry->Exclude, qry->Filename))
		ok = ajTrue;
	}

	ajStrDel(&name);

/*	if(qry->Exclude)
	{
	    ok = ajFalse;
	    wildname = ajStrNew();
	    name     = ajStrNew();
	    while(!ok)
	    {
		ajStrAssignS(&name,qryd->files[i]);
		ajFilenameTrimPath(&name);
		handle = ajStrTokenNewC(qry->Exclude," \n");
		found = ajFalse;
		while(ajStrTokenNextParseC(&handle," \n",&wildname))
		    if(ajStrMatchWildS(name,wildname))
		    {
			found = ajTrue;
			break;
		    }
		ajStrTokenDel(&handle);
		if(!found)
		    ok = ajTrue;
		else
		{
		    ++i;
		    if(!qryd->files[i])
			ok = ajTrue;
		}
	    }

	    ajStrDel(&wildname);
	    ajStrDel(&name);
	}
*/

	if(!qryd->files[i])
	{
	    ajDebug("taxEmbossTaxAll finished\n");
	    i=0;

	    while(qryd->files[i])
	    {
		ajStrDel(&qryd->files[i]);
		ajStrDel(&qryd->reffiles[i]);
		++i;
	    }

	    AJFREE(qryd->files);
	    AJFREE(qryd->reffiles);
	    
	    AJFREE(qry->QryData);
	    qry->QryData = NULL;

	    return ajFalse;
	}


	qryd->libs = ajFileNewInNameS(qryd->files[i]);

	if(!qryd->libs)
	{
	    ajDebug("taxEmbossTaxAll: cannot open nodes file\n");

	    return ajFalse;
	}


	qryd->libr = ajFileNewInNameS(qryd->reffiles[i]);

	if(!qryd->libr)
	{
	    ajDebug("taxEmbossTaxAll: cannot open names file\n");

	    return ajFalse;
	}
    }

    taxEmbossTaxLoadBuff(taxin);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic taxEmbossTaxLoadBuff ******************************************
**
** Copies text data to a buffered file, and taxon data for an
** AjPTaxin internal data structure for reading later
**
** @param [u] taxin [AjPTaxin] Taxon input object
** @return [void]
** @@
******************************************************************************/

static void taxEmbossTaxLoadBuff(AjPTaxin taxin)
{
    AjPQuery qry;
    TaxPEmbossQry qryd;

    qry  = taxin->Input->Query;
    qryd = qry->QryData;

    if(!qry->QryData)
	ajFatal("taxEmbossTaxLoadBuff Query Data not initialised");
    /* read the taxon record */
    taxEmbossTaxReadNode(taxin);

    /* copy all the ref data */

    taxEmbossTaxReadRef(taxin);

    /* ajFilebuffTraceFull(taxin->Input->Filebuff, 9999, 100); */

    if(!qryd->libr)
	ajFileClose(&qryd->libs);

    return;
}




/* @funcstatic taxEmbossTaxReadRef *******************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] taxin [AjPTaxin] Taxon input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool taxEmbossTaxReadRef(AjPTaxin taxin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    TaxPEmbossQry qryd;
    const char *p = NULL;
    ajuint taxid = 0;
    ajuint firstid = 0;

    qry  = taxin->Input->Query;
    qryd = qry->QryData;

    ajDebug("taxEmbossTaxReadRef pos: %Ld\n", ajFileResetPos(qryd->libr));
    while(ajReadline(qryd->libr, &line))
    {
        taxid = 0;
        p = ajStrGetPtr(line);
        while(isdigit(*p))
            taxid = 10*taxid + (*p++ - '0');
        if(!firstid)
            firstid = taxid;
        if(taxid != firstid)
            break;
        ajFilebuffLoadS(taxin->Input->Filebuff, line);
    }

    /* at end of file */

    ajFileClose(&qryd->libr);
    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic taxEmbossTaxReadNode *******************************************
**
** Copies taxon data with a reformatted taxon to the input file.
**
** @param [u] taxin [AjPTaxin] Taxon input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool taxEmbossTaxReadNode(AjPTaxin taxin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    TaxPEmbossQry qryd;

    qry  = taxin->Input->Query;
    qryd = qry->QryData;

    ajDebug("taxEmbossTaxReadNode pos: %Ld\n", ajFileResetPos(qryd->libs));

    if(!ajReadline(qryd->libs, &line))	/* end of file */
	return ajFalse;

    ajFilebuffLoadS(taxin->Input->Filebuff, line);

    ajStrDel(&line);

    return ajTrue;
}




/* @section B+tree Database Indexing *****************************************
**
** These functions manage the B+tree index access methods.
**
******************************************************************************/




/* @funcstatic taxEmbossQryReuse *********************************************
**
** Tests whether the B+tree index query data can be reused or it's finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if reusable,
**                  ajFalse if finished.
** @@
******************************************************************************/

static AjBool taxEmbossQryReuse(AjPQuery qry)
{
    TaxPEmbossQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
	return ajFalse;


    if(!qry->ResultsList)
    {
	ajDebug("taxEmbossQryReuse: query data all finished\n");
	AJFREE(qry->QryData);
	qryd = NULL;

	return ajFalse;
    }
    else
    {
	ajDebug("taxEmbossQryReuse: reusing data from previous call %x "
                "listsize:%u\n",
		qry->QryData, ajListGetLength(qry->ResultsList));
	/*ajListTrace(qry->ResultsList);*/
    }


    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic taxEmbossQryOpen **********************************************
**
** Open caches (etc) for B+tree search
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool taxEmbossQryOpen(AjPQuery qry)
{
    TaxPEmbossQry qryd;
    ajint i;
    AjPStr name     = NULL;
    AjIList iter = NULL;
    AjPQueryField field = NULL;
    AjPBtcache cache = NULL;

    qry->QryData = AJNEW0(qryd);
    qryd = qry->QryData;
    qryd->div = -1;
    qryd->nentries = -1;
    
    qryd->Caches = ajListNew();

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database '%S'\n", qry->DbName);
	ajErr("no indexdir defined for database '%S'", qry->DbName);
	return ajFalse;
    }

    if(!taxEmbossOpenCache(qry,"id",&qryd->idcache))
        return ajFalse;

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        
        ajStrFmtLower(&field->Wildquery);
        if(!taxEmbossOpenCache(qry, MAJSTRGETPTR(field->Field), &cache))
            return ajFalse;
        ajListPushAppend(qryd->Caches, cache);
        cache = NULL;
    }
    ajListIterDel(&iter);


    ajDebug("directory '%S'fields: %u hasacc:%B\n",
	    qry->IndexDir, ajListGetLength(qry->QueryFields), qry->HasAcc);


    if(ajStrGetLen(qry->Exclude) && qryd->nentries >= 0)
    {
	AJCNEW0(qryd->Skip,qryd->nentries);
	name     = ajStrNew();
	
	for(i=0; i < qryd->nentries; ++i)
	{
	    ajStrAssignS(&name,qryd->files[i]);

	    if(!ajFilenameTestInclude(name, qry->Exclude, qry->Filename))
		qryd->Skip[i] = ajTrue;
	}

	ajStrDel(&name);
    }

    return ajTrue;
}




/* @funcstatic taxEmbossOpenCache ********************************************
**
** Create primary B+tree index cache
**
** @param [u] qry [AjPQuery] Query data
** @param [r] ext [const char*] Index file extension
** @param [w] cache [AjPBtcache*] cache
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool taxEmbossOpenCache(AjPQuery qry, const char *ext,
                                 AjPBtcache *cache)
{
    TaxPEmbossQry qryd;
    AjPStr indexextname = NULL;

    AjPBtpage page   = NULL;

    qryd = qry->QryData;

    indexextname = ajStrNewS(ajBtreeFieldGetExtensionC(ext));
    
    *cache = ajBtreeCacheNewReadS(qry->DbAlias,indexextname,
                                  qry->IndexDir);
    ajStrDel(&indexextname);

    if(!*cache)
    {
	qryd->nentries = -1;

	return ajFalse;
    }
    
    if(qryd->nentries == -1)
	qryd->nentries = ajBtreeReadEntriesS(qry->DbAlias,
                                             qry->IndexDir,
                                             qry->Directory,
                                             &qryd->files,
                                             &qryd->reffiles);

    page = ajBtreeCacheRead(*cache,0L);
    page->dirty = BT_LOCK;

    return ajTrue;
}





/* @funcstatic taxEmbossQryEntry *********************************************
**
** Queries for a single entry in a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool taxEmbossQryEntry(AjPQuery qry)
{
    AjPBtId entry  = NULL;
    TaxPEmbossQry qryd;
    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    
    qryd = qry->QryData;

    ajDebug("taxEmbossQryEntry fields: %u hasacc:%B qrylist:%u\n",
	    ajListGetLength(qry->QueryFields), qry->HasAcc,
            ajListGetLength(qry->ResultsList));

    fdlist = ajQueryGetallFields(qry);
    cachelist = qryd->Caches;

    iter= ajListIterNewread(fdlist);
    icache = ajListIterNewread(cachelist);
    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        cache = ajListIterGet(icache);

        ajDebug("qry type:%d field '%S' wild '%S'\n",
                fd->Link, fd->Field, fd->Wildquery);

        if((fd->Link == AJQLINK_ELSE) && ajListGetLength(qry->ResultsList))
                continue;

        if(ajBtreeCacheIsSecondary(cache))
        {

        }
        else
        {
	    entry = ajBtreeIdFromKey(cache,fd->Wildquery);
            ajDebug("id '%S' entry: %p\n", fd->Wildquery, entry);
	    if(entry)
	    {
                ajDebug("entry id: '%S' dups: %u offset: %Ld\n",
                        entry->id, entry->dups, entry->offset);
		if(!entry->dups)
		    ajListPushAppend(qry->ResultsList,(void *)entry);
		else
                {
		    ajBtreeHybLeafList(cache,entry->offset,
				       entry->id,qry->ResultsList);
                    ajBtreeIdDel(&entry);
                }
	    }
        }
    }
  
    ajListIterDel(&iter);
    ajListIterDel(&icache);

    ajDebug("  qrylist:%u\n", ajListGetLength(qry->ResultsList));

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic taxEmbossQryNext **********************************************
**
** Processes the next query for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool taxEmbossQryNext(AjPQuery qry)
{
    AjPBtId entry;
    TaxPEmbossQry qryd;
    void* item;
    AjBool ok = ajFalse;


    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    /*ajListTrace(qry->ResultsList);*/


    if(!qryd->Skip)
    {
	ajListPop(qry->ResultsList, &item);
	entry = (AjPBtId) item;
    }
    else
    {
	ok = ajFalse;

	while(!ok)
	{
	    ajListPop(qry->ResultsList, &item);
	    entry = (AjPBtId) item;

	    if(!qryd->Skip[entry->dbno])
		ok = ajTrue;
	    else
	    {
		ajBtreeIdDel(&entry);

		if(!ajListGetLength(qry->ResultsList))
		    return ajFalse;
	    }
	}
    }

    qryd->Samefile = ajTrue;

    if(entry->dbno != qryd->div)
    {
	qryd->Samefile = ajFalse;
	qryd->div = entry->dbno;
	ajFileClose(&qryd->libs);

	if(qryd->reffiles)
	    ajFileClose(&qryd->libr);
    }

    if(!qryd->libs)
    {
	qryd->libs = ajFileNewInNameS(qryd->files[entry->dbno]);

	if(!qryd->libs)
	{
	    ajBtreeIdDel(&entry);

	    return ajFalse;
	}
    }
	
    if(qryd->reffiles && !qryd->libr)
    {
	ajFileClose(&qryd->libr);
	qryd->libr = ajFileNewInNameS(qryd->reffiles[entry->dbno]);

	if(!qryd->libr)
	{
	    ajBtreeIdDel(&entry);

	    return ajFalse;
	}
    }
    
    
    ajFileSeek(qryd->libs, (ajlong) entry->offset, 0);
    if(qryd->reffiles)
      ajFileSeek(qryd->libr, (ajlong) entry->refoffset, 0);

    ajBtreeIdDel(&entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic taxEmbossQryClose *********************************************
**
** Closes query data for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool taxEmbossQryClose(AjPQuery qry)
{
    TaxPEmbossQry qryd;
    void* item;
    ajint i;

    if(!qry)
	return ajFalse;

    if(!qry->QryData)
	return ajFalse;

    qryd = qry->QryData;

    ajDebug("taxEmbossQryClose clean up qryd %x size:%u\n",
            qryd, ajListGetLength(qry->ResultsList));

    while(ajListGetLength(qryd->Caches))
    {
        ajListPop(qryd->Caches, &item);
        ajBtreeCacheDel((AjPBtcache*) &item);
    }
    ajListFree(&qryd->Caches);
    ajBtreeCacheDel(&qryd->idcache);

    if(qryd->Skip)
    {
	AJFREE(qryd->Skip);
	qryd->Skip = NULL;
    }

    if(qryd->files)
    {
	i = 0;

	while(qryd->files[i])
	{
	    ajStrDel(&qryd->files[i]);

	    if(qryd->reffiles)
		ajStrDel(&qryd->reffiles[i]);

	    ++i;
	}

	AJFREE(qryd->files);
    }

    if(qryd->reffiles)
	AJFREE(qryd->reffiles);

    qryd->files = NULL;
    qryd->reffiles = NULL;


    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic taxEmbossQryQuery *********************************************
**
** Queries for one or more entries in an EMBOSS B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool taxEmbossQryQuery(AjPQuery qry)
{
    TaxPEmbossQry qryd;
    AjPBtId   id   = NULL;

    AjPList  tlist = NULL;
    AjPStr   kwid  = NULL;
    ajulong treeblock = 0L;
    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    qryd = qry->QryData;

    cachelist = qryd->Caches;

    fdlist = ajQueryGetallFields(qry);

    iter = ajListIterNewread(fdlist);
    icache = ajListIterNewread(cachelist);
    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        cache = ajListIterGet(icache);

        ajDebug("field '%S' query: '%S'\n", fd->Field, fd->Wildquery);

        /* is this a primary or secondary key (check the cache)? */

        if(ajBtreeCacheIsSecondary(cache))
        {
            if(!qry->Wild)
            {
                if(ajBtreePriFindKeywordLen(cache, fd->Wildquery,
                                            &treeblock))
                {
                    tlist = ajBtreeSecLeafList(cache, treeblock);
                        ajDebug("ajBtreeSecLeafList(%Ld)  results:%u\n",
                                treeblock, ajListGetLength(tlist));

                    while(ajListPop(tlist,(void **)&kwid))
                    {
                        ajStrFmtLower(&kwid);
                        id = ajBtreeIdFromKey(qryd->idcache, kwid);
                        ajDebug("ajBtreeIdFromKey id:%x\n",
                                id);

                        if(id)
                        {
                            if(!id->dups)
                                ajListPushAppend(qry->ResultsList,(void *)id);
                            else
                            {
                                ajBtreeHybLeafList(qryd->idcache,id->offset,
                                                   id->id,qry->ResultsList);
                                ajBtreeIdDel(&id);
                            }
                        }

                        ajStrDel(&kwid);
                    }

                    ajListFree(&tlist);
                }

                ajListIterDel(&iter);
                ajListIterDel(&icache);
                return ajTrue;
            }
            else
            {
                ajBtreeListFromKeywordW(cache,fd->Wildquery,
                                        qryd->idcache, qry->ResultsList);
                ajListIterDel(&iter);
                ajListIterDel(&icache);
                return ajTrue;
            }
        }
        else
        {
            ajBtreeListFromKeyW(cache,fd->Wildquery,qry->ResultsList);
            ajDebug("ajBtreeListFromKeyW results:%u\n",
                    ajListGetLength(qry->ResultsList));

            if(ajListGetLength(qry->ResultsList))
            {
                ajListIterDel(&iter);
                ajListIterDel(&icache);
                return ajTrue;
            }
        }
    }

    ajListIterDel(&iter);
    ajListIterDel(&icache);

    return ajFalse;
}




/* @func ajTaxdbInit ********************************************************
**
** Initialise taxonomy database internals
**
** @return [void]
******************************************************************************/

void ajTaxdbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajTaxaccessGetDb();

    while(taxAccess[i].Name)
    {
        ajCallTableRegister(table, taxAccess[i].Name,
                            (void*) &taxAccess[i]);
	i++;
    }

    return;
}




/* @func ajTaxdbPrintAccess ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajTaxdbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Taxonomy access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; taxAccess[i].Name; i++)
	if(full || !taxAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			taxAccess[i].Name,  taxAccess[i].Alias,
			taxAccess[i].Entry, taxAccess[i].Query,
			taxAccess[i].All,   taxAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTaxdbExit ********************************************************
**
** Cleans up taxonomy database processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajTaxdbExit(void)
{
    return;
}
