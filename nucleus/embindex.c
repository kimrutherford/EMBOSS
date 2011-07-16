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


static AjPStr embindexLine      = NULL;
static AjPStr embindexToken     = NULL;
static AjPStr embindexTstr      = NULL;
static AjPStr embindexPrefix    = NULL;
static AjPStr embindexFormat    = NULL;
static AjPStrTok embindexHandle = NULL;

static AjPBtPri indexPriobj = NULL;
static AjPBtHybrid indexHyb = NULL;

static AjPFile btreeCreateFile(const AjPStr idirectory, const AjPStr dbname,
			       const char *add);




/* @func embBtreeIndexEntry ***************************************************
**
** Add a term to an index entry cache
**
** @param [u] entry [EmbPBtreeEntry] Entry with id
** @param [r] dbno [ajuint] Database number for an identifier index field
** @return [void]
** @@
******************************************************************************/

void embBtreeIndexEntry(EmbPBtreeEntry entry,
                        ajuint dbno)
{
    if(!indexHyb)
        indexHyb = ajBtreeHybNew();

    if(entry->do_id)
    {
        if(ajStrGetLen(entry->id) > entry->idlen)
        {
            if(ajStrGetLen(entry->id) > entry->idmaxlen)
            {
                ajWarn("id '%S' too long (%u), truncating to idlen %d",
                       entry->id, ajStrGetLen(entry->id), entry->idlen);
                entry->idmaxlen = ajStrGetLen(entry->id);
                ajStrAssignS(&entry->maxid, entry->id);
            }
            entry->idtruncate++;
            ajStrTruncateLen(&entry->id,entry->idlen);
        }
    
        ajStrAssignS(&indexHyb->key1,entry->id);
        indexHyb->dbno = dbno;
        indexHyb->offset = entry->fpos;
        indexHyb->refoffset = entry->reffpos;
        indexHyb->dups = 0;
        ajBtreeHybInsertId(entry->idcache,indexHyb);
    }

    return;
}




/* @func embBtreeIndexField **************************************************
**
** Add a term to an index field cache
**
** @param [u] field [EmbPBtreeField] Field with list of data
** @param [r] entry [const EmbPBtreeEntry] Entry with id
** @param [r] dbno [ajuint] Database number for an identifier index field
** @return [void]
** @@
******************************************************************************/

void embBtreeIndexField(EmbPBtreeField field,
                        const EmbPBtreeEntry entry,
                        ajuint dbno)
{
    AjPStr word = NULL;

    if(!indexPriobj)
        indexPriobj = ajBtreePriNew();
    if(!indexHyb)
        indexHyb = ajBtreeHybNew();

    while(ajListPop(field->data,(void **)&word))
    {
        if(ajStrGetLen(word) > field->len)
        {
            if(ajStrGetLen(word) > field->maxlen)
            {
                ajWarn("%S field token '%S' too long (%u), "
                       "truncating to %Slen %d",
                       field->name, word, ajStrGetLen(word),
                       field->name, field->len);
                field->maxlen = ajStrGetLen(word);
                ajStrAssignS(&field->maxkey, word);
            }
            field->truncate++;
            ajStrTruncateLen(&word,field->len);
        }

        if(field->secondary)
        {
            ajStrAssignS(&indexPriobj->id,entry->id);
            ajStrAssignS(&indexPriobj->keyword,word);
            indexPriobj->treeblock = 0;
            ajBtreeInsertKeyword(field->cache, indexPriobj);
        }
        else 
        {
            ajStrAssignS(&indexHyb->key1,word);
            indexHyb->dbno = dbno;
            indexHyb->offset = entry->fpos;
            indexHyb->refoffset = entry->reffpos;
            indexHyb->dups = 0;
            ajBtreeHybInsertId(field->cache,indexHyb);
        }
        ajStrDel(&word);
    }

    return;
}




/* @func embBtreeParseEntry ***************************************************
**
** Parse an entry ID from an input record
**
** @param [r]readline [const AjPStr] INput record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] entry [EmbPBtreeEntry] Entry
** @return [void]
** @@
******************************************************************************/

void embBtreeParseEntry(const AjPStr readline, AjPRegexp regexp,
                        EmbPBtreeEntry entry)
{
    if(ajRegExec(regexp, readline))
    {
        ajRegSubI(regexp, 1, &entry->id);
    }

    return;
}




/* @func embBtreeParseField ***************************************************
**
** Parse field tokens from an input record, iterating over a
** regular expression.
**
** @param [r]readline [const AjPStr] Input record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] field [EmbPBtreeField] Field
** @return [void]
** @@
******************************************************************************/

void embBtreeParseField(const AjPStr readline, AjPRegexp regexp,
                        EmbPBtreeField field)
{
    AjPStr tmpstr = NULL;
    AjPStr tmpfd = NULL;

    ajStrAssignS(&tmpstr,readline);

    while(ajRegExec(regexp, tmpstr))
    {
        ajRegSubI(regexp, 1, &tmpfd);
        ajRegPost(regexp, &tmpstr);

        if(!ajStrGetLen(tmpfd))
            continue;

        ajListPush(field->data,ajStrNewS(tmpfd));
        ajDebug("++%S '%S'\n", field->name, tmpfd);
    }

    ajStrDel(&tmpstr);
    ajStrDel(&tmpfd);

    return;
}




/* @func embBtreeParseFieldSecond *********************************************
**
** Parse field tokens from an input record using the first and second
** matches to a regular expression.
**
** @param [r]readline [const AjPStr] Input record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] field [EmbPBtreeField] Field
** @return [void]
** @@
******************************************************************************/

void embBtreeParseFieldSecond(const AjPStr readline, AjPRegexp regexp,
                              EmbPBtreeField field)
{
    AjPStr tmpfd = NULL;

    if(ajRegExec(regexp, readline))
    {
        ajRegSubI(regexp, 1, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListPush(field->data,ajStrNewS(tmpfd));
            ajDebug("++%S '%S'\n", field->name, tmpfd);
        }

        ajRegSubI(regexp,2, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListPush(field->data,ajStrNewS(tmpfd));
            ajDebug("++%S '%S'\n", field->name, tmpfd);
        }
    }

    ajStrDel(&tmpfd);

    return;
}




/* @func embBtreeParseFieldThird **********************************************
**
** Parse field tokens from an input record using the first and third
** matches to a regular expression.
**
** @param [r]readline [const AjPStr] Input record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] field [EmbPBtreeField] Field
** @return [void]
** @@
******************************************************************************/

void embBtreeParseFieldThird(const AjPStr readline, AjPRegexp regexp,
                             EmbPBtreeField field)
{
    AjPStr tmpfd = NULL;

    if(ajRegExec(regexp, readline))
    {
        ajRegSubI(regexp, 1, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListPush(field->data,ajStrNewS(tmpfd));
            ajDebug("++%S '%S'\n", field->name, tmpfd);
        }

        ajRegSubI(regexp, 3, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListPush(field->data,ajStrNewS(tmpfd));
            ajDebug("++%S '%S'\n", field->name, tmpfd);
        }
    }

    ajStrDel(&tmpfd);

    return;
}




/* @func embBtreeParseFieldTrim ***********************************************
**
** Parse field tokens from an input record and trim any trailing whitespace,
** iterating over a regular expression.
**
** @param [r]readline [const AjPStr] Input record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] field [EmbPBtreeField] Field
** @return [void]
** @@
******************************************************************************/

void embBtreeParseFieldTrim(const AjPStr readline, AjPRegexp regexp,
                            EmbPBtreeField field)
{
    AjPStr tmpstr = NULL;
    AjPStr tmpfd = NULL;

    ajStrAssignS(&tmpstr,readline);

    while(ajRegExec(regexp, tmpstr))
    {
        ajRegSubI(regexp, 1, &tmpfd);
        ajRegPost(regexp, &tmpstr);

        ajStrTrimWhiteEnd(&tmpfd);
        if(!ajStrGetLen(tmpfd))
            continue;

        ajListPush(field->data,ajStrNewS(tmpfd));
        ajDebug("++%S '%S'\n", field->name, tmpfd);
    }

    ajStrDel(&tmpstr);
    ajStrDel(&tmpfd);

    return;
}




/* @func embBtreeReportEntry **************************************************
**
** Report on indexing of entries
**
** @param [u] outf [AjPFile] Output file
** @param [r] entry [const EmbPBtreeEntry] Entry
**
** @return [void]
******************************************************************************/

void embBtreeReportEntry(AjPFile outf, const EmbPBtreeEntry entry)
{
    if(entry->idmaxlen > entry->idlen)
    {
        ajFmtPrintF(outf,
                    "Entry idlen %u truncated %u IDs. "
                    "Maximum ID length was %u.",
                    entry->idlen, entry->idtruncate, entry->idmaxlen);

        ajWarn("Entry idlen truncated %u IDs. Maximum ID length was %u.",
               entry->idtruncate, entry->idmaxlen);
    }

    return;
}




/* @func embBtreeReportField **************************************************
**
** Report on indexing of field
**
** @param [u] outf [AjPFile] Output file
** @param [r] field [const EmbPBtreeField] Field
**
** @return [void]
******************************************************************************/

void embBtreeReportField(AjPFile outf, const EmbPBtreeField field)
{
    if(field->maxlen > field->len)
    {
        ajFmtPrintF(outf,
                    "Field %S %Slen %u truncated %u terms. "
                    "Maximum %S term length was %u for '%S'.\n",
                    field->name, field->name, field->len, field->truncate,
                    field->name, field->maxlen, field->maxkey);
        ajWarn("Field %S %Slen %u truncated %u terms. "
               "Maximum %S term length was %u for '%S'.",
               field->name, field->name, field->len, field->truncate,
               field->name, field->maxlen, field->maxkey);
    }

    return;
}




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
    
    ajStrAssignSubS(&line, kwline, 5, -1);

    handle = ajStrTokenNewC(line,"\n\r;");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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
    
    ajStrAssignSubS(&line, txline, 5, -1);

    handle = ajStrTokenNewC(line,"\n\r;()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimEndC(&token," ");
	ajStrTrimWhite(&token);

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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
    char *p          = NULL;
    char *q          = NULL;
    ajuint lo = 0;
    ajuint hi = 0;
    ajuint field = 0;
    ajuint i;
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, acline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimWhite(&embindexToken);

	if((p=strchr(MAJSTRGETPTR(embindexToken),(int)'-')))
	{
	    q = p;

	    while(isdigit((int)*(--q)));

	    ++q;
	    ajStrAssignSubC(&embindexTstr,q,0,(ajuint)(p-q-1));
	    ajStrToUint(embindexTstr,&lo);
	    field = (ajuint) (p-q);
	    ajFmtPrintS(&embindexFormat,"%%S%%0%uu",field);
	    
	    ++p;
	    q = p;

	    while(!isdigit((int)*q))
		++q;

	    sscanf(q,"%u",&hi);
	    ajStrAssignSubC(&embindexPrefix,p,0,(ajuint)(q-p-1));
	    
	    for(i=lo;i<=hi;++i)
	    {
		ajFmtPrintS(&str,MAJSTRGETPTR(embindexFormat),
			    embindexPrefix,i);
		ajListPush(aclist,(void *)str);
		str = NULL;
	    }
	}
	else
	{
	    ajStrAssignS(&str,embindexToken);
	    ajListPush(aclist,(void *)str);
	    str = NULL;
	}
    }

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
    
    ajStrAssignSubS(&line, idline, 5, -1);

    handle = ajStrTokenNewC(line," \t\n\r;");

    if(!ajStrTokenNextParse(&handle,&idstr))
	return;

    if(!ajStrTokenNextParse(&handle,&token))
	return;

    if(!ajStrTokenNextParse(&handle,&svstr))
	return;

    if(!ajStrMatchC(token, "SV"))
	return;

    str = ajStrNewRes(MAJSTRGETLEN(idstr)+MAJSTRGETLEN(svstr)+2);

    ajFmtPrintS(&str,"%S.%S", idstr, svstr);

    ajListPush(svlist,(void *)str);
    str = NULL;
    
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
    
    ajStrAssignSubS(&line, deline, 5, -1);

    handle = ajStrTokenNewC(line,"\n\r \t()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimWhite(&token);
	ajStrTrimEndC(&token,".,:'\"");
	ajStrTrimStartC(&token,"'\"");

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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

    ajStrAssignSubS(&line, acline, 12, -1);

    handle = ajStrTokenNewC(line,"\n\r ");

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
		ajFmtPrintS(&str,MAJSTRGETPTR(format),prefix,i);
		ajListPush(aclist,(void *)str);
		str = NULL;
	    }
	}
	else
	{
	    ajStrAssignS(&str,token);
	    ajListPush(aclist,(void *)str);
	    str = NULL;
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
    
    ajStrAssignSubS(&line, kwline, 8, -1);

    handle = ajStrTokenNewC(line,"\n\r;");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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
    
    ajStrAssignSubS(&line, kwline, 10, -1);

    handle = ajStrTokenNewC(line,"\n\r \t()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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
    
    ajStrAssignSubS(&line, kwline, 9, -1);

    handle = ajStrTokenNewC(line,"\n\r;()");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimEndC(&token," ");
	ajStrTrimWhite(&token);

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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
    
    handle = ajStrTokenNewC(kwline,"\n\r ");

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
	    str = NULL;
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);
    
    return;
}




/* @func embBtreeFastaSV **************************************************
**
** Extract sequence version keywords from a Fasta description
**
** @param [r] kwline[const AjPStr] sequence version or GI string
** @param [w] kwlist [AjPList] list of sequence versions
** @param [r] maxlen [ajuint] max sequence version length
**
** @return [void]
** @@
******************************************************************************/

void embBtreeFastaSV(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;

    handle = ajStrTokenNewC(kwline,"\n ");

    while(ajStrTokenNextParse(&handle,&token))
    {
	ajStrTrimEndC(&token,".");
	ajStrTrimWhite(&token);

	if(ajStrGetLen(token))
	{
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
	    str = NULL;
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
    nfiles = ajFilelistAddPathWild(lfiles, fdirectory, files);

    nremove = ajArrCommaList(exclude,&removelist);
    
    for(i=0;i<nfiles;++i)
    {
	ajListPop(lfiles,(void **)&file);
	ajFilenameTrimPath(&file);

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

    if(!ajStrGetLen(idirectory))
        ajFmtPrintS(&filename,"%S%s",dbname,add);
    else
        ajFmtPrintS(&filename,"%S%s%S%s",idirectory,SLASH_STRING,dbname,add);

    fp =  ajFileNewOutNameS(filename);

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

    thys->dbname  = ajStrNew();
    thys->dbrs    = ajStrNew();
    thys->date    = ajStrNew();
    thys->release = ajStrNew();
    thys->dbtype  = ajStrNew();

    thys->directory  = ajStrNew();
    thys->idirectory = ajStrNew();
    thys->idextension = ajStrNew();
    thys->maxid       = ajStrNew();
    
    thys->files    = ajListNew();
    thys->reffiles = ajListNew();

    thys->id = ajStrNew();

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
    EmbPBtreeField field;
    AjPStr tmpstr = NULL;
    
    pthis = *thys;

    ajStrDel(&pthis->dbname);
    ajStrDel(&pthis->idextension);
    ajStrDel(&pthis->maxid);
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

    while(ajListPop(pthis->fields,(void **)&field))
	embBtreeFieldDel(&field);

    ajListFree(&pthis->fields);

    ajStrDel(&pthis->id);

    *thys = NULL;
    AJFREE(pthis);

    return;
}




/* @func embBtreeEntrySetCompressed *******************************************
**
** Set database entry to be compressed on writing
**
** @param [u] entry [EmbPBtreeEntry] Database entry information
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEntrySetCompressed(EmbPBtreeEntry entry)
{
    entry->compressed = ajTrue;

    return;
}




/* @func embBtreeSetFields ****************************************************
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
    EmbPBtreeField field = NULL;

    nfields = 0;

    if(!entry->fields)
        entry->fields = ajListNew();

    while(fields[nfields])
    {
	if(ajStrMatchCaseC(fields[nfields], "id"))
	    entry->do_id = ajTrue;

	else
        {
            field = embBtreeFieldNewS(fields[nfields]);
            ajListPushAppend(entry->fields, field);
            field = NULL;
        }
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
    ajStrAssignC(&entry->idextension, "xid");
    ajStrAssignS(&entry->date, date);
    ajStrAssignS(&entry->release, release);
    ajStrAssignS(&entry->dbtype, type);
    ajStrAssignS(&entry->dbrs, dbrs);
    
    ajStrAssignS(&entry->directory,directory);
    ajStrAssignS(&entry->idirectory,idirectory);

    return;
}




/* @func embBtreeGetFieldC ***********************************************
**
** Set database fields to index
**
** @param [w] entry [EmbPBtreeEntry] Database entry information
** @param [r] nametxt [const char*] Field name
**
** @return [EmbPBtreeField] Btree index field definition
** @@
******************************************************************************/

EmbPBtreeField embBtreeGetFieldC(EmbPBtreeEntry entry, const char * nametxt)
{
    EmbPBtreeField ret = NULL;
    EmbPBtreeField field = NULL;

    AjIList iter;

    if(!ajListGetLength(entry->fields))
        return NULL;

    iter = ajListIterNewread(entry->fields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        if(ajStrMatchC(field->name, nametxt))
        {
            ret = field;
            break;
        }
    }

    ajListIterDel(&iter);

    return ret;
}




/* @func embBtreeGetFieldS ***********************************************
**
** Set database fields to index
**
** @param [w] entry [EmbPBtreeEntry] Database entry information
** @param [r] name [const AjPStr] Field name
**
** @return [EmbPBtreeField] Btree index field definition
** @@
******************************************************************************/

EmbPBtreeField embBtreeGetFieldS(EmbPBtreeEntry entry, const AjPStr name)
{
    EmbPBtreeField ret = NULL;
    EmbPBtreeField field = NULL;

    AjIList iter;

    if(!ajListGetLength(entry->fields))
        return NULL;

    iter = ajListIterNewread(entry->fields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        if(ajStrMatchS(field->name, name))
        {
            ret = field;
            break;
        }
    }

    ajListIterDel(&iter);

    return ret;
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

    nfiles = ajFilelistAddPathWild(entry->files, fdirectory,files);

    nremove = ajArrCommaList(exclude,&removelist);

    count = 0;

    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&file);
	ajFilenameTrimPath(&file);

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
    AjPStr attrstr = NULL;
    AjPStr value = NULL;
    ajuint  n = 0;
    AjIList iter;
    EmbPBtreeField field;

    value = ajStrNew();

    ajStrAssignC(&attrstr, "type");
    if(!ajNamRsAttrValueS(entry->dbrs, attrstr, &value))
	ajFatal("Missing resource entry (%S) for indexing",entry->dbrs);

    if(!ajStrMatchCaseC(value,"Index"))
	ajFatal("Incorrect 'type' field for resource (%S)",entry->dbrs);


    if(ajNamRsAttrValueC(MAJSTRGETPTR(entry->dbrs),"cachesize",&value) ||
       ajNamGetValueC("CACHESIZE",&value))
          {
	if(ajStrToUint(value,&n))
	{
	    entry->cachesize = n;
	}
	else
	{
	    ajErr("Bad value for environment variable 'CACHESIZE'");
	    entry->cachesize = BT_CACHESIZE;
	}
    }
    else
    {
	entry->cachesize = BT_CACHESIZE;
	ajUser("CACHESIZE defaults to %d", entry->cachesize);
    }

    if(ajNamRsAttrValueC(MAJSTRGETPTR(entry->dbrs),"pagesize",&value) ||
       ajNamGetValueC("PAGESIZE",&value))
    {
	if(ajStrToUint(value,&n))
	{
	    entry->pagesize = n;
	}
	else
	{
	    ajErr("Bad value for environment variable 'PAGESIZE'");
	    entry->pagesize = BT_PAGESIZE;
	}
    }
    else
    {
	entry->pagesize = BT_PAGESIZE;
	ajUser("PAGESIZE defaults to %d", entry->pagesize);
    }

    ajStrAssignC(&attrstr, "idlen");
    if(!ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
	entry->idlen = BT_KWLIMIT;
    else
    {
	if(ajStrToUint(value,&n))
	    entry->idlen = n;
	else
	{
	    ajErr("Bad value for index resource 'idlen'");
	    entry->idlen = BT_KWLIMIT;
	}
    }

    ajStrAssignC(&attrstr, "idpagesize");
    if(!ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
	entry->idpagesize = entry->pagesize;
    else
    {
        if(ajStrToUint(value,&n))
	{
	    entry->idpagesize = n;
	}
	else
	{
	    ajErr("Bad value for index resource 'idpagesize'");
	    entry->idpagesize = entry->pagesize;
	}
    }

    ajStrAssignC(&attrstr, "idcachesize");
    if(!ajNamRsAttrValueS(entry->dbrs, attrstr, &value))
	entry->idcachesize = entry->cachesize;
    else
    {
        if(ajStrToUint(value,&n))
	{
	    entry->idcachesize = n;
	}
	else
	{
	    ajErr("Bad value for index resource 'idcachesize'");
	    entry->idcachesize = entry->cachesize;
	}
    }
        
    entry->idorder = (entry->idpagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
        ((entry->idlen + 1) + BT_IDKEYEXTRA);

    entry->idfill  = (entry->idpagesize - BT_BUCKPREAMBLE) /
        ((entry->idlen + 1) + BT_KEYLENENTRY + BT_DDOFFROFF);

    entry->idsecorder = (entry->idpagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
        (BT_OFFKEYLEN + BT_IDKEYEXTRA);

    entry->idsecfill  = (entry->idpagesize - BT_BUCKPREAMBLE) /
        BT_DOFFROFF;

/* now process the same values for each index field */

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);
            ajFmtPrintS(&attrstr, "%Slen", field->name);
            if(!ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
                field->len = ajBtreeFieldGetLenS(field->name);
            else
            {
                if(ajStrToUint(value,&n))
                    field->len = n;
                else
                {
                    ajErr("Bad value for index resource '%S'", attrstr);
                    field->len = 15;
                }
            }

            ajFmtPrintS(&attrstr, "%Spagesize", field->name);

            if(!ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
                field->pagesize = entry->pagesize;
            else
            {
                if(ajStrToUint(value,&n))
                {
                    field->pagesize = n;
                }
                else
                {
                    ajErr("Bad value for index resource '%S'", attrstr);
                    field->pagesize = entry->pagesize;
                }
            }
        
            ajFmtPrintS(&attrstr, "%Scachesize", field->name);
            ajDebug("embBtreeGetRsInfo '%S':\n", attrstr);
            if(!ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
            {
                ajDebug("embBtreeGetRsInfo '%S' no value, use entry %u\n",
                        attrstr, entry->cachesize);
                field->cachesize = entry->cachesize;
            }
            else
            {
                if(ajStrToUint(value,&n))
                {
                    ajDebug("embBtreeGetRsInfo '%S' defined %u\n",
                            attrstr, n);
                    field->cachesize = n;
                }
                else
                {
                    ajDebug("embBtreeGetRsInfo '%S' bad value '%S'\n",
                            attrstr, value);
                    ajErr("Bad value for index resource '%Scachesize'",
                          field->name);
                    field->cachesize = entry->cachesize;
                }
            }
        
            field->order =
                (field->pagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
                ((field->len + 1) + BT_IDKEYEXTRA);

            field->fill  =
                (field->pagesize - BT_BUCKPREAMBLE) /
                ((field->len + 1) + BT_KEYLENENTRY + BT_DDOFFROFF);

    /*
     *  The secondary tree keys are the IDs of the entries containing
     *  the keywords
     */
            if(!field->secondary)
            {
                field->secorder =
                    (field->pagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
                    (BT_OFFKEYLEN + BT_IDKEYEXTRA);
                field->secfill  =
                    (field->pagesize - BT_BUCKPREAMBLE) /
                    BT_DOFFROFF;
            }
            else
            {
                field->secorder =
                    (field->pagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
                    ((entry->idlen + 1) + BT_IDKEYEXTRA);
                field->secfill  =
                    (field->pagesize - BT_BUCKPREAMBLE) /
                    ((entry->idlen + 1) + BT_KEYLENENTRY);
            }
        }

        ajListIterDel(&iter);
    }
    
    ajStrDel(&attrstr);
    ajStrDel(&value);

    return;
}




/* @func embBtreeOpenCaches ***************************************************
**
** Open index files for writing
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool embBtreeOpenCaches(EmbPBtreeEntry entry)
{
    ajuint level    = 0;
    ajlong count    = 0L;
    ajlong countall = 0L;
    AjIList iter;
    EmbPBtreeField field;
    
    if(entry->do_id)
    {
	entry->idcache = ajBtreeCacheNewS(entry->dbname,
                                          entry->idextension,
                                          entry->idirectory,
                                          "wb+",
                                          entry->compressed,
                                          entry->idlen,
                                          entry->idpagesize,
                                          entry->idcachesize,
                                          entry->idpagecount,
                                          entry->idorder,
                                          entry->idfill,
                                          level,
                                          entry->idsecorder,
                                          entry->idsecfill,
                                          count,
                                          countall);
	if(!entry->idcache)
	    ajFatal("Cannot open ID index");
	
	ajBtreeCreateRootNode(entry->idcache,0L);
    }

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);

            if(field->secondary)
                field->cache = ajBtreeSecCacheNewS(entry->dbname,
                                                   field->extension,
                                                   entry->idirectory,
                                                   "wb+",
                                                   field->compressed,
                                                   field->len,
                                                   field->pagesize,
                                                   field->cachesize,
                                                   field->pagecount,
                                                   field->order,
                                                   field->fill,
                                                   level,
                                                   field->secorder,
                                                   field->secfill,
                                                   count,
                                                   countall);
            else
                field->cache = ajBtreeCacheNewS(entry->dbname,
                                                field->extension,
                                                entry->idirectory,
                                                "wb+",
                                                field->compressed,
                                                field->len,
                                                field->pagesize,
                                                field->cachesize,
                                                field->pagecount,
                                                field->order,
                                                field->fill,
                                                level,
                                                field->secorder,
                                                field->secfill,
                                                count,
                                                countall);
            if(!field->cache)
                ajFatal("Cannot open %S index", field->extension);

            ajBtreeCreateRootNode(field->cache,0L);
        }
        ajListIterDel(&iter);
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
    AjIList iter;
    EmbPBtreeField field;

    if(entry->do_id)
    {
	ajBtreeCacheDel(&entry->idcache);
    }

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);

            ajBtreeCacheDel(&field->cache);
         }
        ajListIterDel(&iter);
    }

    return ajTrue;
}




#if 0
/* @func embBtreeProbeCaches ***********************************************
**
** Close index files
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool embBtreeProbeCaches(EmbPBtreeEntry entry)
{
    AjIList iter;
    EmbPBtreeField field;

    if(entry->do_id)
    {
	ajBtreeProbePriArray(entry->idcache);
	ajBtreeProbeSecArray(entry->idcache);
    }

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);
            if(ajStrMatchC(field->extension, "ac") ||
               ajStrMatchC(field->extension, "sv"))
            {
            	ajBtreeProbePriArray(field->cache);
                ajBtreeProbeSecArray(field->cache);
            }
        }
        ajListIterDel(&iter);
    }

    return ajTrue;
}

#endif




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
    AjIList iter;
    EmbPBtreeField field;

    if(entry->do_id)
	ajBtreeWriteParamsS(entry->idcache, entry->dbname,
                            entry->idextension, entry->idirectory);

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);
            ajBtreeWriteParamsS(field->cache, entry->dbname,
                                field->extension, entry->idirectory);
        }
        ajListIterDel(&iter);
    }
    
    return ajTrue;
}




/* @func embBtreeFieldNewC *****************************************************
**
** Constructor for a Btree index field
**
** @param [r] nametxt [const char*] Name
** @return [EmbPBtreeField] Btree field
******************************************************************************/

EmbPBtreeField embBtreeFieldNewC(const char* nametxt)
{
    EmbPBtreeField ret = NULL;

    AJNEW0(ret);

    ajStrAssignC(&ret->name, nametxt);
    ajStrAssignS(&ret->extension, ajBtreeFieldGetExtensionC(nametxt));
    ret->secondary = ajBtreeFieldGetSecondaryC(nametxt);

    if(!ajStrGetLen(ret->extension))
    {
        ajStrAssignK(&ret->extension, 'x');
        ajStrAppendC(&ret->extension, nametxt);
    }

    ret->data = ajListNew();

    return ret;
}




/* @func embBtreeFieldNewS *****************************************************
**
** Constructor for a Btree index field
**
** @param [r] name [const AjPStr] Name
** @return [EmbPBtreeField] Btree field
******************************************************************************/

EmbPBtreeField embBtreeFieldNewS(const AjPStr name)
{
    EmbPBtreeField ret = NULL;

    AJNEW0(ret);

    ret->name = ajStrNewS(name);
    ret->extension = ajStrNewS(ajBtreeFieldGetExtensionS(name));
    ret->secondary = ajBtreeFieldGetSecondaryS(name);

    if(!ajStrGetLen(ret->extension))
    {
        ajStrAssignK(&ret->extension, 'x');
        ajStrAppendS(&ret->extension, name);
    }

    ret->maxkey = ajStrNewC("");

    ret->data = ajListNew();

    return ret;
}




/* @func embBtreeFieldDel ******************************************************
**
** Destructor for a Btree index field
**
** @param [d] Pthis [EmbPBtreeField*] Btree index field object
** @return [void]
******************************************************************************/

void embBtreeFieldDel(EmbPBtreeField *Pthis)
{
    EmbPBtreeField  thys;

    if(!Pthis) return;

    thys = *Pthis;

    ajStrDel(&thys->name);
    ajStrDel(&thys->extension);
    ajStrDel(&thys->maxkey);
    ajListstrFree(&thys->data);

    AJFREE(*Pthis);
    *Pthis = NULL;

    return;
}




/* @func embBtreeFieldSetCompressed *******************************************
**
** Set database field to be compressed on writing
**
** @param [u] field [EmbPBtreeField] Database field information
**
** @return [void]
** @@
******************************************************************************/

void embBtreeFieldSetCompressed(EmbPBtreeField field)
{
    field->compressed = ajTrue;

    return;
}




/* @func embBtreeFieldSetIdtype ***********************************************
**
** Set database field to be identifier type (not secondary) on writing
**
** @param [u] field [EmbPBtreeField] Database field information
**
** @return [void]
** @@
******************************************************************************/

void embBtreeFieldSetIdtype(EmbPBtreeField field)
{
    field->secondary = ajFalse;

    return;
}




/* @func embBtreeFieldSetSecondary ********************************************
**
** Set database field to be secondary on writing
**
** @param [u] field [EmbPBtreeField] Database field information
**
** @return [void]
** @@
******************************************************************************/

void embBtreeFieldSetSecondary(EmbPBtreeField field)
{
    field->secondary = ajTrue;

    return;
}




/* @func embIndexExit **********************************************************
**
** Cleanup indexing internals on exit
**
** @return [void]
******************************************************************************/

void embIndexExit(void)
{
    ajStrDel(&embindexLine);
    ajStrDel(&embindexToken);
    ajStrDel(&embindexTstr);
    ajStrDel(&embindexPrefix);
    ajStrDel(&embindexFormat);
    ajStrTokenDel(&embindexHandle);

    ajBtreePriDel(&indexPriobj);
    ajBtreeHybDel(&indexHyb);

    return;
}
