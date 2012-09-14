/* @source ajtaxdb ************************************************************
**
** AJAX taxonomy database functions
**
** These functions control all aspects of AJAX taxonomy database access
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.21 $
** @modified Oct 2010 pmr first version
** @modified $Date: 2012/07/14 14:52:39 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/


#include "ajlib.h"

#include "ajtaxdb.h"
#include "ajtaxread.h"
#include "ajindex.h"
#include "ajnam.h"
#include "ajquery.h"
#include "ajfileio.h"
#include "ajcall.h"


#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>


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
** @attr reffiles [AjPStr**] database reference filenames
** @attr Skip [AjBool*] files numbers to exclude
** @attr libs [AjPFile] Primary (database source) file
** @attr libr [AjPFile] Secondary (database bibliographic source) file
** @attr div [ajuint] division number of currently open database file
** @attr refcount [ajuint] number of reference file(s) per entry
** @attr nentries [ajint] number of entries in the filename array(s)
**                        -1 when done
** @attr Samefile [AjBool] true if the same file is passed to
**                         ajFilebuffReopenFile
** @@
******************************************************************************/

typedef struct TaxSEmbossQry
{
    AjPBtcache idcache;
    AjPList Caches;

    AjPStr *files;
    AjPStr **reffiles;
    AjBool *Skip;

    AjPFile libs;
    AjPFile libr;

    ajuint div;
    ajuint refcount;
    ajint nentries;

    AjBool Samefile;
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




/* @funclist taxAccess ********************************************************
**
** Functions to access each database or taxonomy access method
**
******************************************************************************/

static AjOTaxAccess taxAccess[] =
{
    /*  Name     AccessFunction   FreeFunction
        Qlink    Description
        Alias    Entry    Query    All      Chunk   Padding */
    {
        "embosstax", &taxAccessEmbossTax, NULL,
        "",      "emboss dbxtax indexed",
        AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
        NULL, NULL, NULL,
        NULL, NULL,
        AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    },
};




/* @section B+tree NCBI Taxonomy Indexing *************************************
**
** These functions manage the EMBOSS B+tree NCBI taxonomy access methods.
**
******************************************************************************/




/* @funcstatic taxAccessEmbossTax *********************************************
**
** Reads taxon(s) from an NCBI taxonomy database, using B+tree index
** files. Returns with the file pointer set to the position in the
** buffered taxonomy file.
**
** @param [u] taxin [AjPTaxin] Taxon input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
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




/* @funcstatic taxEmbossTaxAll ************************************************
**
** Opens the first or next taxonomy file for further reading
**
** @param [u] taxin [AjPTaxin] Taxon input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxEmbossTaxAll(AjPTaxin taxin)
{
    AjPQuery qry;
    TaxPEmbossQry qryd;
    static ajint i   = 0;
    ajuint iref;
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
                            &qryd->files,
                            &qryd->reffiles,
                            &qryd->refcount);

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

                if(qryd->reffiles)
                {
                    for(iref=0; iref < qryd->refcount; iref++)
                        ajStrDel(&qryd->reffiles[i][iref]);
                }

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


	qryd->libr = ajFileNewInNameS(qryd->reffiles[i][0]);

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




/* @funcstatic taxEmbossTaxLoadBuff *******************************************
**
** Copies text data to a buffered file, and taxon data for an
** AjPTaxin internal data structure for reading later
**
** @param [u] taxin [AjPTaxin] Taxon input object
** @return [void]
**
** @release 6.4.0
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




/* @funcstatic taxEmbossTaxReadRef ********************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] taxin [AjPTaxin] Taxon input object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
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
        while(isdigit((int)*p))
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
**
** @release 6.4.0
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




/* @funcstatic taxEmbossQryReuse **********************************************
**
** Tests whether the B+tree index query data can be reused or it's finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if reusable,
**                  ajFalse if finished.
**
** @release 6.4.0
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
                "listsize:%Lu\n",
		qry->QryData, ajListGetLength(qry->ResultsList));
	/*ajListTrace(qry->ResultsList);*/
    }


    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic taxEmbossQryOpen ***********************************************
**
** Open caches (etc) for B+tree search
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
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


    ajDebug("directory '%S'fields: %Lu hasacc:%B\n",
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




/* @funcstatic taxEmbossOpenCache *********************************************
**
** Create primary B+tree index cache
**
** @param [u] qry [AjPQuery] Query data
** @param [r] ext [const char*] Index file extension
** @param [w] cache [AjPBtcache*] cache
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxEmbossOpenCache(AjPQuery qry, const char *ext,
                                 AjPBtcache *cache)
{
    TaxPEmbossQry qryd;
    AjPStr indexextname = NULL;

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
                                             &qryd->reffiles,
                                             &qryd->refcount);

    return ajTrue;
}





/* @funcstatic taxEmbossQryEntry **********************************************
**
** Queries for a single entry in a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxEmbossQryEntry(AjPQuery qry)
{
    TaxPEmbossQry qryd;
    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    
    qryd = qry->QryData;

    ajDebug("taxEmbossQryEntry fields: %Lu hasacc:%B qrylist:%Lu\n",
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

        if(!ajBtreeCacheIsSecondary(cache))
        {
	    ajBtreeIdentFetchHitref(cache,fd->Wildquery,
                                    qry->ResultsList);
        }
    }
  
    ajListIterDel(&iter);
    ajListIterDel(&icache);

    ajDebug("  qrylist:%Lu\n", ajListGetLength(qry->ResultsList));

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic taxEmbossQryNext ***********************************************
**
** Processes the next query for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxEmbossQryNext(AjPQuery qry)
{
    AjPBtHitref entry;
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
	entry = (AjPBtHitref) item;
    }
    else
    {
	ok = ajFalse;

	while(!ok)
	{
	    ajListPop(qry->ResultsList, &item);
	    entry = (AjPBtHitref) item;

	    if(!qryd->Skip[entry->dbno])
		ok = ajTrue;
	    else
	    {
		ajBtreeHitrefDel(&entry);

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
	    ajBtreeHitrefDel(&entry);

	    return ajFalse;
	}
    }
	
    if(qryd->reffiles && !qryd->libr)
    {
	ajFileClose(&qryd->libr);
	qryd->libr = ajFileNewInNameS(qryd->reffiles[entry->dbno][0]);

	if(!qryd->libr)
	{
	    ajBtreeHitrefDel(&entry);

	    return ajFalse;
	}
    }
    
    
    ajFileSeek(qryd->libs, (ajlong) entry->offset, 0);
    if(qryd->reffiles)
      ajFileSeek(qryd->libr, (ajlong) entry->refoffset, 0);

    ajBtreeHitrefDel(&entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic taxEmbossQryClose **********************************************
**
** Closes query data for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxEmbossQryClose(AjPQuery qry)
{
    TaxPEmbossQry qryd;
    void* item;
    ajint i;
    ajuint iref;

    if(!qry)
	return ajFalse;

    if(!qry->QryData)
	return ajFalse;

    qryd = qry->QryData;

    ajDebug("taxEmbossQryClose clean up qryd %x size:%Lu\n",
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
            {
                for(iref=0; iref < qryd->refcount; iref++)
                    ajStrDel(&qryd->reffiles[i][iref]);
            }

	    ++i;
	}

	AJFREE(qryd->files);
    }

    if(qryd->reffiles)
    {
        for(iref=0; iref < qryd->refcount; iref++)
            AJFREE(qryd->reffiles[iref]);

	AJFREE(qryd->reffiles);
    }

    qryd->files = NULL;
    qryd->reffiles = NULL;


    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic taxEmbossQryQuery **********************************************
**
** Queries for one or more entries in an EMBOSS B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxEmbossQryQuery(AjPQuery qry)
{
    TaxPEmbossQry qryd;

    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    AjPBtHitref newhit;
    AjPBtHitref *allhits = NULL;
    AjPTable newtable = NULL;

    ajuint i;
    ajulong lasthits = 0UL;
    ajulong fdhits = 0UL;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    qryd = qry->QryData;

    cachelist = qryd->Caches;

    ajTableSetDestroy(qry->ResultsTable, NULL, &ajBtreeHitrefDelVoid);
    ajTableSettypeUser(qry->ResultsTable,
                       &ajBtreeHitrefCmp, &ajBtreeHitrefHash);

    fdlist = ajQueryGetallFields(qry);

    ajDebug("taxEmbossQryQuery wild: %B list:%Lu fields:%Lu\n",
            qry->Wild, ajListGetLength(qry->ResultsList),
            ajListGetLength(fdlist));

    iter = ajListIterNewread(fdlist);
    icache = ajListIterNewread(cachelist);

    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        cache = ajListIterGet(icache);

        ajDebug("field '%S' query: '%S'\n", fd->Field, fd->Wildquery);

        if((fd->Link == AJQLINK_ELSE) && (lasthits > 0))
        {
            continue;
        }

        /* is this a primary or secondary key (check the cache)? */

        if(ajBtreeCacheIsSecondary(cache))
        {
            if(!qry->Wild)
            {
                ajBtreeKeyFetchHitref(cache, qryd->idcache,
                                      fd->Wildquery, qry->ResultsList);
            }
            else
            {
                ajBtreeKeyFetchwildHitref(cache, qryd->idcache,
                                          fd->Wildquery, qry->ResultsList);
            }
        }
        else
        {
            ajBtreeIdentFetchwildHitref(cache,fd->Wildquery,qry->ResultsList);
            ajDebug("ajBtreeIdentFetchwild results:%Lu\n",
                    ajListGetLength(qry->ResultsList));

        }

        fdhits = ajListGetLength(qry->ResultsList);

        switch(fd->Link)
        {
            case AJQLINK_INIT:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(qry->ResultsTable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);
                break;

            case AJQLINK_OR:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeOr(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_AND:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeAnd(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_EOR:
            case AJQLINK_ELSE:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeEor(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_NOT:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeNot(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            default:
                ajErr("Unexpected query link operator number '%u'",
                      fd->Link);
                break;
        }

        lasthits = fdhits;
    }

    ajListIterDel(&iter);
    ajListIterDel(&icache);

    ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);
    for(i=0; allhits[i]; i++)
        ajListPushAppend(qry->ResultsList, (void*) allhits[i]);

    AJFREE(allhits);

    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @func ajTaxdbInit **********************************************************
**
** Initialise taxonomy database internals
**
** @return [void]
**
** @release 6.4.0
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
**
** @release 6.4.0
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




/* @func ajTaxdbExit **********************************************************
**
** Cleans up taxonomy database processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxdbExit(void)
{
    return;
}
