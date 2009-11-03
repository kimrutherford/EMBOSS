/* @source embindex.c
**
** B+ Tree Indexing plus Disc Cache.
** Copyright (c) 2003 Alan Bleasby
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

#define BTENTRYFILE     ".ent"
#define KWLIMIT 12




static AjPFile btreeCreateFile(const AjPStr idirectory, const AjPStr dbname,
			       const char *add);




/* @func embBtreeEmblKW **************************************************
**
** Extract keywords from an EMBL KW line 
**
** @param [r] kwline [const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblKW(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(kwline)[5]);

    handle = ajStrTokenNewC(line,"\n;");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(kwlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeEmblTX **************************************************
**
** Extract keywords from an EMBL OC or OS line 
**
** @param [r] txline [const AjPStr] taxonomy line
** @param [w] txlist [AjPList] list of taxons
** @param [r] maxlen [ajuint] max taxon length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblTX(const AjPStr txline, AjPList txlist, ajuint maxlen)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(txline)[5]);

    handle = ajStrTokenNewC(line,"\n;()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimEndC(&token," ");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(txlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeEmblAC **************************************************
**
** Extract accession numbers from an EMBL AC line 
**
** @param [r] acline[const AjPStr] AC line
** @param [w] aclist [AjPList] list of accession numbers
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblAC(const AjPStr acline, AjPList aclist)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    AjPStr tstr      = NULL;
    AjPStr prefix    = NULL;
    AjPStr format    = NULL;
    char *p          = NULL;
    char *q          = NULL;
    ajuint lo = 0;
    ajuint hi = 0;
    ajuint field = 0;
    ajuint i;
    
    line   = ajStrNew();
    token  = ajStrNew();
    tstr   = ajStrNew();
    prefix = ajStrNew();
    format = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(acline)[5]);

    handle = ajStrTokenNewC(line,"\n;");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimWhite(&token);
	if((p=strchr(MAJSTRGETPTR(token),(int)'-')))
	{
	    q = p;
	    while(isdigit((int)*(--q)));
	    ++q;
	    ajStrAssignSubC(&tstr,q,0,(ajuint)(p-q-1));
	    ajStrToUint(tstr,&lo);
	    field = (ajuint) (p-q);
	    ajFmtPrintS(&format,"%%S%%0%uu",field);
	    
	    ++p;
	    q = p;
	    while(!isdigit((int)*q))
		++q;
	    sscanf(q,"%u",&hi);
	    ajStrAssignSubC(&prefix,p,0,(ajuint)(q-p-1));
	    
	    for(i=lo;i<=hi;++i)
	    {
		str = ajStrNew();
		ajFmtPrintS(&str,MAJSTRGETPTR(format),prefix,i);
		ajListPush(aclist,(void *)str);
	    }
	}
	else
	{
	    str = ajStrNew();
	    ajStrAssignS(&str,token);
	    ajListPush(aclist,(void *)str);
	}
    }

    ajStrDel(&tstr);
    ajStrDel(&prefix);
    ajStrDel(&format);
    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}



/* @func embBtreeEmblSV **************************************************
**
** Extract sequence version from an EMBL new format ID line 
**
** @param [r] idline[const AjPStr] AC line
** @param [w] svlist [AjPList] list of accession numbers
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblSV(const AjPStr idline, AjPList svlist)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    AjPStr idstr      = NULL;
    AjPStr svstr      = NULL;
    
    line   = ajStrNew();
    token  = ajStrNew();
    idstr   = ajStrNew();
    svstr   = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(idline)[5]);

    handle = ajStrTokenNewC(line," \t\n;");

    str = ajStrNew();
    if(!ajStrTokenNextParse(&handle,&idstr))
	return;
    if(!ajStrTokenNextParse(&handle,&token))
	return;
    if(!ajStrTokenNextParse(&handle,&svstr))
	return;

    if(!ajStrMatchC(token, "SV"))
	return;

    ajFmtPrintS(&str,"%S.%S", idstr, svstr);

    ajListPush(svlist,(void *)str);

    ajStrDel(&idstr);
    ajStrDel(&svstr);
    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeEmblDE **************************************************
**
** Extract words from an EMBL DE line 
**
** @param [r] deline[const AjPStr] description line
** @param [w] delist [AjPList] list of descriptions
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblDE(const AjPStr deline, AjPList delist, ajuint maxlen)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(deline)[5]);

    handle = ajStrTokenNewC(line,"\n \t()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(delist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeGenBankAC **************************************************
**
** Extract accession numbers from a GenBank ACCESSION line 
**
** @param [r] acline[const AjPStr] AC line
** @param [w] aclist [AjPList] list of accession numbers
**
** @return [void]
** @@
******************************************************************************/

void embBtreeGenBankAC(const AjPStr acline, AjPList aclist)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    AjPStr tstr      = NULL;
    AjPStr prefix    = NULL;
    AjPStr format    = NULL;
    char *p          = NULL;
    char *q          = NULL;
    ajuint lo = 0;
    ajuint hi = 0;
    ajuint field = 0;
    ajuint i;
    
    line   = ajStrNew();
    token  = ajStrNew();
    tstr   = ajStrNew();
    prefix = ajStrNew();
    format = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(acline)[12]);

    handle = ajStrTokenNewC(line,"\n ");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimWhite(&token);
	if((p=strchr(MAJSTRGETPTR(token),(int)'-')))
	{
	    q = p;
	    while(isdigit((int)*(--q)));
	    ++q;
	    ajStrAssignSubC(&tstr,q,0,(ajuint)(p-q-1));
	    ajStrToUint(tstr,&lo);
	    field = (ajuint) (p-q);
	    ajFmtPrintS(&format,"%%S%%0%uu",field);
	    
	    ++p;
	    q = p;
	    while(!isdigit((int)*q))
		++q;
	    sscanf(q,"%u",&hi);
	    ajStrAssignSubC(&prefix,p,0,(ajuint)(q-p-1));
	    
	    for(i=lo;i<=hi;++i)
	    {
		str = ajStrNew();
		ajFmtPrintS(&str,MAJSTRGETPTR(format),prefix,i);
		ajListPush(aclist,(void *)str);
	    }
	}
	else
	{
	    str = ajStrNew();
	    ajStrAssignS(&str,token);
	    ajListPush(aclist,(void *)str);
	}
    }

    ajStrDel(&tstr);
    ajStrDel(&prefix);
    ajStrDel(&format);
    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeGenBankKW **************************************************
**
** Extract keywords from a GenBank KEYWORDS line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeGenBankKW(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(kwline)[8]);

    handle = ajStrTokenNewC(line,"\n;");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(kwlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeGenBankDE **************************************************
**
** Extract keywords from a GenBank DESCRIPTION line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeGenBankDE(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(kwline)[10]);

    handle = ajStrTokenNewC(line,"\n \t()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(kwlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeGenBankTX **************************************************
**
** Extract keywords from a GenBank ORGANISM line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeGenBankTX(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssignC(&line, &MAJSTRGETPTR(kwline)[9]);

    handle = ajStrTokenNewC(line,"\n;()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimEndC(&token," ");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(kwlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeFastaDE **************************************************
**
** Extract keywords from a Fasta description
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeFastaDE(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    token = ajStrNew();
    
    handle = ajStrTokenNewC(kwline,"\n ");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);
	if(ajStrGetLen(token))
	{
	    str = ajStrNew();
	    if(maxlen)
	    {
		if(ajStrGetLen(token) > maxlen)
		    ajStrAssignSubS(&str,token,0,maxlen-1);
		else
		    ajStrAssignS(&str,token);
		
	    }
	    else
		ajStrAssignS(&str,token);
	    ajStrFmtLower(&str);
	    ajListPush(kwlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    
    return;
}




/* @func embBtreeReadDir ******************************************************
**
** Read files to index
**
** @param [w] filelist [AjPStr**] list of files to read
** @param [r] fdirectory [const AjPStr] Directory to scan
** @param [r] files [const AjPStr] Filename to search for (or NULL)
** @param [r] exclude [const AjPStr] list of files to exclude
**
** @return [ajuint] number of matching files
** @@
******************************************************************************/

ajuint embBtreeReadDir(AjPStr **filelist, const AjPStr fdirectory,
		      const AjPStr files, const AjPStr exclude)
{
    AjPList lfiles = NULL;
    ajuint nfiles;
    ajuint nremove;
    ajuint i;
    ajuint j;
    AjPStr file    = NULL;
    AjPStr *removelist = NULL;

    /* ajDebug("In ajBtreeReadDir\n"); */

    lfiles = ajListNew();
    nfiles = ajFileScan(fdirectory,files,&lfiles,ajFalse,ajFalse,NULL,NULL,
			ajFalse,NULL);

    nremove = ajArrCommaList(exclude,&removelist);
    
    for(i=0;i<nfiles;++i)
    {
	ajListPop(lfiles,(void **)&file);
	ajFileNameTrim(&file);
	for(j=0;j<nremove && ! ajStrMatchWildS(file,removelist[j]);++j);
	if(j == nremove)
	    ajListPushAppend(lfiles,(void *)file);
    }

    nfiles =  ajListToarray(lfiles,(void ***)&(*filelist));
    ajListFree(&lfiles);

    for(i=0; i<nremove;++i)
	ajStrDel(&removelist[i]);
    AJFREE(removelist);

    return nfiles;
}




/* @funcstatic btreeCreateFile ************************************************
**
** Open B+tree file for writing
**
** @param [r] idirectory [const AjPStr] Directory for index files
** @param [r] dbname [const AjPStr] name of database
** @param [r] add [const char *] type of file
**
** @return [AjPFile] opened file
** @@
******************************************************************************/

static AjPFile btreeCreateFile(const AjPStr idirectory, const AjPStr dbname,
			       const char *add)
{
    AjPStr filename = NULL;
    AjPFile fp      = NULL;
    
    /* ajDebug("In btreeCreateFile\n"); */

    filename = ajStrNew();

    ajFmtPrintS(&filename,"%S%s%S%s",idirectory,SLASH_STRING,dbname,add);
    
    fp =  ajFileNewOut(filename);

    ajStrDel(&filename);
    return fp;
}




/* @func embBtreeEntryNew ***********************************************
**
** Construct a database entry object
**
** @return [EmbPBtreeEntry] db entry object pointer
** @@
******************************************************************************/

EmbPBtreeEntry embBtreeEntryNew(void)
{
    EmbPBtreeEntry thys;

    AJNEW0(thys);

    thys->do_id          = ajFalse;
    thys->do_accession   = ajFalse;
    thys->do_sv          = ajFalse;
    thys->do_keyword     = ajFalse;
    thys->do_taxonomy    = ajFalse;
    thys->do_description = ajFalse;

    thys->dbname  = ajStrNew();
    thys->dbrs    = ajStrNew();
    thys->date    = ajStrNew();
    thys->release = ajStrNew();
    thys->dbtype  = ajStrNew();

    thys->directory  = ajStrNew();
    thys->idirectory = ajStrNew();
    
    thys->files    = ajListNew();
    thys->reffiles = ajListNew();

    thys->id = ajStrNew();
    thys->ac = ajListNew();
    thys->de = ajListNew();
    thys->sv = ajListNew();
    thys->kw = ajListNew();
    thys->tx = ajListNew();

    return thys;
}




/* @func embBtreeEntryDel ***********************************************
**
** Delete a database entry object
**
** @param [d] thys [EmbPBtreeEntry*] db entry object pointer
** @return [void]
** @@
******************************************************************************/

void embBtreeEntryDel(EmbPBtreeEntry* thys)
{
    EmbPBtreeEntry pthis;
    AjPStr tmpstr = NULL;
    
    pthis = *thys;

    ajStrDel(&pthis->dbname);
    ajStrDel(&pthis->dbrs);
    ajStrDel(&pthis->date);
    ajStrDel(&pthis->release);
    ajStrDel(&pthis->dbtype);

    ajStrDel(&pthis->directory);
    ajStrDel(&pthis->idirectory);


    while(ajListPop(pthis->files,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListFree(&pthis->files);

    while(ajListPop(pthis->reffiles,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListFree(&pthis->reffiles);

    ajStrDel(&pthis->id);
    ajListFree(&pthis->ac);
    ajListFree(&pthis->de);
    ajListFree(&pthis->sv);
    ajListFree(&pthis->kw);
    ajListFree(&pthis->tx);

    *thys = NULL;
    AJFREE(pthis);

    return;
}




/* @func embBtreeSetFields ***********************************************
**
** Set database fields to index
**
** @param [w] entry [EmbPBtreeEntry] Database entry information
** @param [r] fields [AjPStr const *] user specified fields
**
** @return [ajuint] number of fields set
** @@
******************************************************************************/

ajuint embBtreeSetFields(EmbPBtreeEntry entry, AjPStr const *fields)
{
    ajuint nfields;


    nfields = 0;
    while(fields[nfields])
    {
	if(ajStrMatchCaseC(fields[nfields], "id"))
	    entry->do_id = ajTrue;

	else if(ajStrMatchCaseC(fields[nfields], "acc"))
	    entry->do_accession = ajTrue;

	else if(ajStrMatchCaseC(fields[nfields], "sv"))
	    entry->do_sv = ajTrue;

	else if(ajStrMatchCaseC(fields[nfields], "des"))
	    entry->do_description = ajTrue;

	else if(ajStrMatchCaseC(fields[nfields], "key"))
	    entry->do_keyword = ajTrue;

	else if(ajStrMatchCaseC(fields[nfields], "org"))
	    entry->do_taxonomy = ajTrue;

	else
	    ajWarn("Parsing unknown field '%S': ignored",
		   fields[nfields]);
	++nfields;
    }
    
    return nfields;
}




/* @func embBtreeSetDbInfo ***********************************************
**
** Set general database information
**
** @param [w] entry [EmbPBtreeEntry] Database entry information
** @param [r] name [const AjPStr] user specified name
** @param [r] dbrs [const AjPStr] user specified resource
** @param [r] date [const AjPStr] user specified date
** @param [r] release [const AjPStr] user specified release
** @param [r] type [const AjPStr] user specified type
** @param [r] directory [const AjPStr] user specified directory
** @param [r] idirectory [const AjPStr] user specified index directory
**
** @return [void]
** @@
******************************************************************************/

void embBtreeSetDbInfo(EmbPBtreeEntry entry, const AjPStr name,
		       const AjPStr dbrs,
		       const AjPStr date, const AjPStr release,
		       const AjPStr type, const AjPStr directory,
		       const AjPStr idirectory)
{
    ajStrAssignS(&entry->dbname, name);
    ajStrAssignS(&entry->date, date);
    ajStrAssignS(&entry->release, release);
    ajStrAssignS(&entry->dbtype, type);
    ajStrAssignS(&entry->dbrs, dbrs);
    
    ajStrAssignS(&entry->directory,directory);
    ajStrAssignS(&entry->idirectory,idirectory);

    return;
}




/* @func embBtreeGetFiles *****************************************************
**
** Read files to index
**
** @param [u] entry [EmbPBtreeEntry] list of files to read
** @param [r] fdirectory [const AjPStr] Directory to scan
** @param [r] files [const AjPStr] Filename to search for (or NULL)
** @param [r] exclude [const AjPStr] list of files to exclude
**
** @return [ajuint] number of matching files
** @@
******************************************************************************/

ajuint embBtreeGetFiles(EmbPBtreeEntry entry, const AjPStr fdirectory,
		       const AjPStr files, const AjPStr exclude)
{
    ajuint nfiles;
    ajuint nremove;
    ajuint i;
    ajuint j;
    AjPStr file    = NULL;
    AjPStr *removelist = NULL;
    ajuint count = 0;
    
    /* ajDebug("In embBtreeGetFiles\n"); */

    nfiles = ajFileScan(fdirectory,files,&entry->files,ajFalse,ajFalse,NULL,
			NULL,ajFalse,NULL);

    nremove = ajArrCommaList(exclude,&removelist);

    count = 0;
    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&file);
	ajFileNameTrim(&file);
	for(j=0;j<nremove && !ajStrMatchWildS(file,removelist[j]);++j);
	if(j == nremove)
	{
	    ajListPushAppend(entry->files,(void *)file);
	    ++count;
	}
    }

    ajListSort(entry->files,ajStrVcmp);

    entry->nfiles = count;

    for(i=0; i<nremove;++i)
	ajStrDel(&removelist[i]);
    AJFREE(removelist);

    return count;
}




/* @func embBtreeWriteEntryFile ***********************************************
**
** Put files to entry file
**
** @param [r] entry [const EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool embBtreeWriteEntryFile(const EmbPBtreeEntry entry)
{
    AjPFile entfile = NULL;
    ajuint i;
    AjPStr tmpstr = NULL;
    AjPStr refstr = NULL;
    AjBool do_ref;
    
    /* ajDebug("In embBtreeWriteEntryFile\n"); */

    entfile = btreeCreateFile(entry->idirectory,entry->dbname,BTENTRYFILE);
    if(!entfile)
	return ajFalse;
    
    ajFmtPrintF(entfile,"# Number of files: %u\n",entry->nfiles);
    ajFmtPrintF(entfile,"# Release: %S\n",entry->release);
    ajFmtPrintF(entfile,"# Date:    %S\n",entry->date);

    do_ref = (ajListGetLength(entry->reffiles)) ? ajTrue : ajFalse;
    if(!do_ref)
	ajFmtPrintF(entfile,"Single");
    else
	ajFmtPrintF(entfile,"Dual");
    ajFmtPrintF(entfile," filename database\n");
    
    for(i=0;i<entry->nfiles;++i)
    {
	if(!do_ref)
	{
	    ajListPop(entry->files,(void **)&tmpstr);
	    ajFmtPrintF(entfile,"%S\n",tmpstr);
	    ajListPushAppend(entry->files,(void *)tmpstr);
	}
	else
	{
	    ajListPop(entry->files,(void **)&tmpstr);
	    ajListPop(entry->reffiles,(void **)&refstr);
	    ajFmtPrintF(entfile,"%S %S\n",tmpstr, refstr);
	    ajListPushAppend(entry->files,(void *)tmpstr);
	    ajListPushAppend(entry->reffiles,(void *)refstr);
	}
	
    }

    ajFileClose(&entfile);
    
    return ajTrue;
}




/* @func embBtreeGetRsInfo ***********************************************
**
** Get resource information for selected database
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [void]
** @@
******************************************************************************/

void embBtreeGetRsInfo(EmbPBtreeEntry entry)
{
    const char *resource;
    AjPStr value = NULL;
    ajuint  n = 0;
    
    value = ajStrNew();
    

    resource = ajStrGetPtr(entry->dbrs);
    if(!ajNamRsAttrValueC(resource,"type",&value))
	ajFatal("Missing resource entry (%S) for indexing",entry->dbrs);
    if(!ajStrMatchCaseC(value,"Index"))
	ajFatal("Incorrect 'type' field for resource (%S)",entry->dbrs);



    if(!ajNamRsAttrValueC(resource,"idlen",&value))
	entry->idlen = BTREE_DEF_IDLEN;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->idlen = n;
	else
	{
	    ajErr("Bad value for index resource 'idlen'");
	    entry->idlen = BTREE_DEF_IDLEN;
	}
    }
    
    if(!ajNamRsAttrValueC(resource,"acclen",&value))
	entry->aclen = BTREE_DEF_ACLEN;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->aclen = n;
	else
	{
	    ajErr("Bad value for index resource 'acclen'");
	    entry->aclen = BTREE_DEF_ACLEN;
	}
    }

    if(!ajNamRsAttrValueC(resource,"svlen",&value))
	entry->svlen = BTREE_DEF_SVLEN;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->svlen = n;
	else
	{
	    ajErr("Bad value for index resource 'svlen'");
	    entry->svlen = BTREE_DEF_SVLEN;
	}
    }

    if(!ajNamRsAttrValueC(resource,"keylen",&value))
	entry->kwlen = BTREE_DEF_KWLEN;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->kwlen = n;
	else
	{
	    ajErr("Bad value for index resource 'keylen'");
	    entry->kwlen = BTREE_DEF_KWLEN;
	}
    }

    if(!ajNamRsAttrValueC(resource,"deslen",&value))
	entry->delen = BTREE_DEF_DELEN;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->delen = n;
	else
	{
	    ajErr("Bad value for index resource 'deslen'");
	    entry->delen = BTREE_DEF_DELEN;
	}
    }

    if(!ajNamRsAttrValueC(resource,"orglen",&value))
	entry->txlen = BTREE_DEF_TXLEN;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->txlen = n;
	else
	{
	    ajErr("Bad value for index resource 'orglen'");
	    entry->txlen = BTREE_DEF_TXLEN;
	}
    }

    if(!ajNamGetValueC("CACHESIZE",&value))
	entry->cachesize = BTREE_DEF_CACHESIZE;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->cachesize = n;
	else
	{
	    ajErr("Bad value for environment variable 'CACHESIZE'");
	    entry->cachesize = BTREE_DEF_CACHESIZE;
	}
    }

    if(!ajNamGetValueC("PAGESIZE",&value))
	entry->pagesize = BTREE_DEF_PAGESIZE;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->pagesize = n;
	else
	{
	    ajErr("Bad value for environment variable 'PAGESIZE'");
	    entry->pagesize = BTREE_DEF_PAGESIZE;
	}
    }

    
    entry->idorder = (entry->pagesize - 60) / ((entry->idlen + 1) + 12);
    entry->idfill  = (entry->pagesize - 16) / (entry->idlen + 28);
    entry->acorder = (entry->pagesize - 60) / ((entry->aclen + 1) + 12);
    entry->acfill  = (entry->pagesize - 16) / (entry->aclen + 28);
    entry->svorder = (entry->pagesize - 60) / ((entry->svlen + 1) + 12);
    entry->svfill  = (entry->pagesize - 16) / (entry->svlen + 28);

    entry->kworder = (entry->pagesize - 60) / ((entry->kwlen + 1) + 12);
    entry->kwfill  = (entry->pagesize - 16) / (entry->kwlen + 28);
    entry->deorder = (entry->pagesize - 60) / ((entry->delen + 1) + 12);
    entry->defill  = (entry->pagesize - 16) / (entry->delen + 28);
    entry->txorder = (entry->pagesize - 60) / ((entry->txlen + 1) + 12);
    entry->txfill  = (entry->pagesize - 16) / (entry->txlen + 28);

    entry->kwsecorder = (entry->pagesize - 60) / ((entry->idlen + 1) + 12);
    entry->desecorder = (entry->pagesize - 60) / ((entry->idlen + 1) + 12);
    entry->txsecorder = (entry->pagesize - 60) / ((entry->idlen + 1) + 12);

    entry->kwsecfill  = (entry->pagesize - 16) / (entry->idlen + 4);
    entry->desecfill  = (entry->pagesize - 16) / (entry->idlen + 4);
    entry->txsecfill  = (entry->pagesize - 16) / (entry->idlen + 4);

    entry->idsecorder = (entry->pagesize - 60) / 24;
    entry->idsecfill  = (entry->pagesize - 60) / 20;

    entry->acsecorder = (entry->pagesize - 60) / 24;
    entry->acsecfill  = (entry->pagesize - 60) / 20;

    entry->svsecorder = (entry->pagesize - 60) / 24;
    entry->svsecfill  = (entry->pagesize - 60) / 20;
    
    ajStrDel(&value);

    return;
} 




/* @func embBtreeOpenCaches ***********************************************
**
** Open index files
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool embBtreeOpenCaches(EmbPBtreeEntry entry)
{
    char *basenam = NULL;
    char *idir    = NULL;
    ajuint level   = 0;
    ajuint slevel  = 0;
    ajuint count   = 0;
    
    basenam = entry->dbname->Ptr;
    idir    = entry->idirectory->Ptr;


    if(entry->do_id)
    {
	entry->idcache = ajBtreeSecCacheNewC(basenam,ID_EXTENSION,idir,"w+",
					     entry->pagesize, entry->idorder,
					     entry->idfill, level,
					     entry->cachesize,
					     entry->idsecorder, slevel,
					     entry->idsecfill, count,
					     entry->kwlen);
	if(!entry->idcache)
	    ajFatal("Cannot open ID index");
	
	ajBtreeCreateRootNode(entry->idcache,0L);
    }

    if(entry->do_accession)
    {
	entry->accache = ajBtreeSecCacheNewC(basenam,AC_EXTENSION,idir,"w+",
					     entry->pagesize,
					     entry->acorder, entry->acfill,
					     level,
					     entry->cachesize,
					     entry->acsecorder, slevel,
					     entry->acsecfill, count,
					     entry->kwlen);
	if(!entry->accache)
	    ajFatal("Cannot open ACC index");

	ajBtreeCreateRootNode(entry->accache,0L);
    }

    if(entry->do_sv)
    {
	entry->svcache = ajBtreeSecCacheNewC(basenam,SV_EXTENSION,idir,"w+",
					     entry->pagesize, entry->svorder,
					     entry->svfill, level,
					     entry->cachesize,
					     entry->svsecorder, slevel,
					     entry->svsecfill, count,
					     entry->kwlen);
	if(!entry->svcache)
	    ajFatal("Cannot open SV index");


	ajBtreeCreateRootNode(entry->svcache,0L);
    }


    if(entry->do_keyword)
    {
	entry->kwcache = ajBtreeSecCacheNewC(basenam,KW_EXTENSION,idir,"w+",
					     entry->pagesize,
					     entry->kworder, entry->kwfill,
					     level,
					     entry->cachesize,
					     entry->kwsecorder, slevel,
					     entry->kwsecfill, count,
					     entry->kwlen);
	if(!entry->kwcache)
	    ajFatal("Cannot open KW index");


	ajBtreeCreateRootNode(entry->kwcache,0L);
    }
    
    if(entry->do_description)
    {
	entry->decache = ajBtreeSecCacheNewC(basenam,DE_EXTENSION,idir,"w+",
					     entry->pagesize,
					     entry->deorder, entry->defill,
					     level,
					     entry->cachesize,
					     entry->desecorder, slevel,
					     entry->desecfill, count,
					     entry->delen);
	if(!entry->decache)
	    ajFatal("Cannot open DE index");


	ajBtreeCreateRootNode(entry->decache,0L);
    }
    
    if(entry->do_taxonomy)
    {
	entry->txcache = ajBtreeSecCacheNewC(basenam,TX_EXTENSION,idir,"w+",
					     entry->pagesize,
					     entry->txorder, entry->txfill,
					     level,
					     entry->cachesize,
					     entry->txsecorder, slevel,
					     entry->txsecfill, count,
					     entry->txlen);
	if(!entry->txcache)
	    ajFatal("Cannot open TX index");


	ajBtreeCreateRootNode(entry->txcache,0L);
    }
    

    return ajTrue;
}




/* @func embBtreeCloseCaches ***********************************************
**
** Close index files
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool embBtreeCloseCaches(EmbPBtreeEntry entry)
{

    if(entry->do_id)
    {
	ajBtreeFreePriArray(entry->idcache);
	ajBtreeFreeSecArray(entry->idcache);

	ajBtreeCacheSync(entry->idcache,0L);
	ajBtreeCacheDel(&entry->idcache);
    }

    if(entry->do_accession)
    {
	ajBtreeFreePriArray(entry->accache);
	ajBtreeFreeSecArray(entry->accache);

	ajBtreeCacheSync(entry->accache,0L);
	ajBtreeCacheDel(&entry->accache);
    }

    if(entry->do_sv)
    {
	ajBtreeFreePriArray(entry->svcache);
	ajBtreeFreeSecArray(entry->svcache);

	ajBtreeCacheSync(entry->svcache,0L);
	ajBtreeCacheDel(&entry->svcache);
    }


    if(entry->do_keyword)
    {
	ajBtreeCacheSync(entry->kwcache,0L);
	ajBtreeCacheDel(&entry->kwcache);
    }
    
    if(entry->do_description)
    {
	ajBtreeCacheSync(entry->decache,0L);
	ajBtreeCacheDel(&entry->decache);
    }
    
    if(entry->do_taxonomy)
    {
	ajBtreeCacheSync(entry->txcache,0L);
	ajBtreeCacheDel(&entry->txcache);
    }
    

    return ajTrue;
}




/* @func embBtreeDumpParameters ***********************************************
**
** Write index parameter files
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool embBtreeDumpParameters(EmbPBtreeEntry entry)
{
    char *basenam = NULL;
    char *idir    = NULL;

    basenam = entry->dbname->Ptr;
    idir    = entry->idirectory->Ptr;
    

    if(entry->do_id)
	ajBtreeWriteParams(entry->idcache, basenam, ID_EXTENSION, idir);

    if(entry->do_accession)
	ajBtreeWriteParams(entry->accache, basenam, AC_EXTENSION, idir);

    if(entry->do_sv)
	ajBtreeWriteParams(entry->svcache, basenam, SV_EXTENSION, idir);

    if(entry->do_keyword)
	ajBtreeWriteParams(entry->kwcache, basenam, KW_EXTENSION, idir);
    
    if(entry->do_description)
	ajBtreeWriteParams(entry->decache, basenam, DE_EXTENSION, idir);
    
    if(entry->do_taxonomy)
	ajBtreeWriteParams(entry->txcache, basenam, TX_EXTENSION, idir);    

    return ajTrue;
}
