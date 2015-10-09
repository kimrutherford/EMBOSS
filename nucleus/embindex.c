/* @source embindex ***********************************************************
**
** B+ Tree Indexing plus Disc Cache.
**
** @author Copyright (c) 2003 Alan Bleasby
** @version $Revision: 1.56 $
** @modified $Date: 2012/07/17 15:05:51 $ by $Author: rice $
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

#include "embindex.h"
#include "ajlist.h"
#include "ajindex.h"
#include "ajreg.h"
#include "ajarr.h"
#include "ajnam.h"

#include <errno.h>

#define BTENTRYFILE     ".ent"
#define KWLIMIT 12


static AjPStr embindexLine      = NULL;
static AjPStr embindexToken     = NULL;
static AjPStr embindexTstr      = NULL;
static AjPStr embindexPrefix    = NULL;
static AjPStr embindexFormat    = NULL;
static AjPStrTok embindexHandle = NULL;

static AjPStr  indexWord = NULL;
static AjPBtId indexId   = NULL;

static AjPFile btreeCreateFile(const AjPStr idirectory, const AjPStr dbname,
			       const char *add);




/* @func embBtreeIndexEntry ***************************************************
**
** Add a term to an index entry cache
**
** @param [u] entry [EmbPBtreeEntry] Entry with id
** @param [r] dbno [ajuint] Database number for an identifier index field
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeIndexEntry(EmbPBtreeEntry entry,
                        ajuint dbno)
{
    AjBool dotrunc = ajFalse;
    ajuint iref;

    if(!indexId)
        indexId = ajBtreeIdNew(entry->refcount);

    if(entry->do_id)
    {
        if(ajStrGetLen(entry->id) > entry->idlen)
        {
            dotrunc = ajTrue;
            if(ajStrGetLen(entry->id) > entry->idmaxlen)
                ajWarn("id '%S' too long (%u), truncating to idlen %d",
                       entry->id, ajStrGetLen(entry->id), entry->idlen);

        }

        if(ajStrGetLen(entry->id) > entry->idmaxlen)
        {
            entry->idmaxlen = ajStrGetLen(entry->id);
            ajStrAssignS(&entry->maxid, entry->id);
        }
    
        if(dotrunc)
        {
            entry->idtruncate++;
            ajStrTruncateLen(&entry->id,entry->idlen);
        }

        ajStrAssignS(&indexId->id,entry->id);
        indexId->dbno = dbno;
        indexId->dups = 0;
        indexId->offset = entry->fpos;
        indexId->refcount = entry->refcount;

        if(entry->refcount)
        {
            for(iref=0; iref < entry->refcount; iref++)
                indexId->refoffsets[iref] = entry->reffpos[iref];
        }

        ajBtreeIdentIndex(entry->idcache,indexId);
    }

    return;
}




/* @func embBtreeIndexField ***************************************************
**
** Add a term to an index field cache
**
** @param [u] field [EmbPBtreeField] Field with list of data
** @param [r] entry [const EmbPBtreeEntry] Entry with id
** @param [r] dbno [ajuint] Database number for an identifier index field
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeIndexField(EmbPBtreeField field,
                        const EmbPBtreeEntry entry,
                        ajuint dbno)
{
    AjBool dotrunc = ajFalse;

    ajuint iref;

    if(!indexId)
        indexId = ajBtreeIdNew(field->refcount);

    while(ajListstrPop(field->data,&indexWord))
    {
        if(ajStrGetLen(indexWord) > field->len)
        {
            dotrunc = ajTrue;
            if(ajStrGetLen(indexWord) > field->maxlen)
                ajWarn("%S field token '%S' too long (%u), "
                       "truncating to %Slen %d",
                       field->name, indexWord, ajStrGetLen(indexWord),
                       field->name, field->len);
        }

        if(ajStrGetLen(indexWord) > field->maxlen)
        {
            field->maxlen = ajStrGetLen(indexWord);
            ajStrAssignS(&field->maxkey, indexWord);
        }

        if(dotrunc)
        {
            field->truncate++;
            ajStrTruncateLen(&indexWord,field->len);
        }

        if(field->secondary)
        {
            ajBtreeKeyIndex(field->cache, indexWord, entry->id);
        }
        else 
        {
            ajStrAssignS(&indexId->id,indexWord);
            indexId->dbno = dbno;
            indexId->dups = 0;
            indexId->offset = entry->fpos;

            if(entry->refcount)
            {
                for(iref=0; iref < entry->refcount; iref++)
                    indexId->refoffsets[iref] = entry->reffpos[iref];
            }

            ajBtreeIdentIndex(field->cache,indexId);
        }
    }

    return;
}




/* @func embBtreeIndexPrimary *************************************************
**
** Add a term to an index field cache
**
** @param [u] field [EmbPBtreeField] Field with list of data
** @param [r] entry [const EmbPBtreeEntry] Entry with id
** @param [r] dbno [ajuint] Database number for an identifier index field
** @return [ajuint] Number of keys added
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint embBtreeIndexPrimary(EmbPBtreeField field,
                            const EmbPBtreeEntry entry,
                            ajuint dbno)
{
    ajuint ret = 0;
    AjBool dotrunc = ajFalse;
    ajuint iref = 0;

    if(!indexId)
        indexId = ajBtreeIdNew(entry->refcount);

    while(embBtreeFieldGetdataS(field, &indexWord))
    {
        if(ajStrGetLen(indexWord) > field->len)
        {
            dotrunc = ajTrue;
            if(ajStrGetLen(indexWord) > field->maxlen)
                ajWarn("%S field token '%S' too long (%u), "
                       "truncating to %Slen %d",
                       field->name, indexWord,
                       ajStrGetLen(indexWord),
                       field->name, field->len);
        }

        if(ajStrGetLen(indexWord) > field->maxlen)
        {
            field->maxlen = ajStrGetLen(indexWord);
            ajStrAssignS(&field->maxkey, indexWord);
        }

        if(dotrunc)
        {
            field->truncate++;
            ajStrTruncateLen(&indexWord,field->len);
        }

        ajStrAssignS(&indexId->id, indexWord);
        indexId->dbno = dbno;
        indexId->dups = 0;
        indexId->offset = entry->fpos;
        indexId->refcount = entry->refcount;

        if(entry->refcount)
        {
            for(iref=0; iref < entry->refcount; iref++)
               indexId->refoffsets[iref] = entry->reffpos[iref];
        }

        ajBtreeIdentIndex(field->cache, indexId);
        ret++;
    }

    return ret;
}




/* @func embBtreeIndexSecondary ***********************************************
**
** Add a term to an index field cache
**
** @param [u] field [EmbPBtreeField] Field with list of data
** @param [r] entry [const EmbPBtreeEntry] Entry identifier
** @return [ajuint] Number of keys added
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint embBtreeIndexSecondary(EmbPBtreeField field,
                              const EmbPBtreeEntry entry)
{
    ajuint ret = 0;
    AjBool dotrunc = ajFalse;

    while(embBtreeFieldGetdataS(field, &indexWord))
    {
        if(ajStrGetLen(indexWord) > field->len)
        {
            dotrunc = ajTrue;
            if(ajStrGetLen(indexWord) > field->maxlen)
                ajWarn("%S field token '%S' too long (%u), "
                       "truncating to %Slen %d",
                       field->name, indexWord,
                       ajStrGetLen(indexWord),
                       field->name, field->len);
        }

        if(ajStrGetLen(indexWord) > field->maxlen)
        {
            field->maxlen = ajStrGetLen(indexWord);
            ajStrAssignS(&field->maxkey, indexWord);
        }

        if(dotrunc)
        {
            field->truncate++;
            ajStrTruncateLen(&indexWord,field->len);
        }

        ajBtreeKeyIndex(field->cache, indexWord, entry->id);
        ret++;
    }

    return ret;
}




/* @func embBtreeParseEntry ***************************************************
**
** Parse an entry ID from an input record
**
** @param [r]readline [const AjPStr] INput record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] entry [EmbPBtreeEntry] Entry
** @return [void]
**
** @release 6.4.0
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
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeParseField(const AjPStr readline, AjPRegexp regexp,
                        EmbPBtreeField field)
{
    AjPStr tmpfd = NULL;

    ajStrAssignS(&embindexLine,readline);

    while(ajRegExec(regexp, embindexLine))
    {
        if(field->freecount)
            tmpfd = field->freelist[--field->freecount];

        ajRegSubI(regexp, 1, &tmpfd);
        ajRegPost(regexp, &embindexLine);

        if(!ajStrGetLen(tmpfd))
        {
            ajStrDel(&tmpfd);
            continue;
        }

        ajListstrPushAppend(field->data,tmpfd);
        ajDebug("++%S '%S'\n", field->name, tmpfd);
        tmpfd = NULL;
    }

    return;
}




/* @func embBtreeFieldGetdataS *************************************************
**
** Return the next field data value as a word
**
** @param [u] field [EmbPBtreeField] Field
** @param [w] Pstr [AjPStr*] Data value field
** @return [AjBool] True if data was found
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool embBtreeFieldGetdataS(EmbPBtreeField field, AjPStr *Pstr)
{
    AjPStr tmpstr = NULL;
    ajuint oldfreesize = 0;

    if(!ajListGetLength(field->data))
        return ajFalse;
    
    ajListPop(field->data,(void **)&tmpstr);
    ajStrAssignS(Pstr, tmpstr);

    if(!field->freelist) 
    {
        field->freecount = 0;
        field->freesize = 16;
        AJCNEW(field->freelist, field->freesize);
    }

    if(field->freesize == field->freecount)
    {
        oldfreesize = field->freesize;
        field->freesize *= 2;
        AJCRESIZE0(field->freelist, oldfreesize, field->freesize);
    }

    field->freelist[field->freecount++] = tmpstr;
    tmpstr = NULL;

    return ajTrue;
}




/* @func embBtreeParseFieldSecond *********************************************
**
** Parse field tokens from an input record using the first and second
** matches to a regular expression.
**
** @param [r] readline [const AjPStr] Input record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] field [EmbPBtreeField] Field
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeParseFieldSecond(const AjPStr readline, AjPRegexp regexp,
                              EmbPBtreeField field)
{
    AjPStr tmpfd = NULL;

    if(ajRegExec(regexp, readline))
    {
        if(field->freecount)
            tmpfd = field->freelist[--field->freecount];

        ajRegSubI(regexp, 1, &tmpfd);
        ajRegSubI(regexp, 1, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListstrPushAppend(field->data, tmpfd);
            ajDebug("++%S '%S'\n", field->name, tmpfd);
            tmpfd = NULL;
            if(field->freecount)
                tmpfd = field->freelist[--field->freecount];
        }

        ajRegSubI(regexp,2, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListstrPushAppend(field->data,tmpfd);
            ajDebug("++%S '%S'\n", field->name, tmpfd);
            tmpfd = NULL;
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
** @param [r] readline [const AjPStr] Input record
** @param [u] regexp [AjPRegexp] Regular expression to extract tokens
** @param [u] field [EmbPBtreeField] Field
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeParseFieldThird(const AjPStr readline, AjPRegexp regexp,
                             EmbPBtreeField field)
{
    AjPStr tmpfd = NULL;

    if(ajRegExec(regexp, readline))
    {
        if(field->freecount)
            tmpfd = field->freelist[--field->freecount];

        ajRegSubI(regexp, 1, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListstrPushAppend(field->data,tmpfd);
            ajDebug("++%S '%S'\n", field->name, tmpfd);
            tmpfd = NULL;
            if(field->freecount)
                tmpfd = field->freelist[--field->freecount];
        }

        ajRegSubI(regexp, 3, &tmpfd);

        if(ajStrGetLen(tmpfd))
        {
            ajListstrPushAppend(field->data,tmpfd);
            ajDebug("++%S '%S'\n", field->name, tmpfd);
            tmpfd = NULL;
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
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeParseFieldTrim(const AjPStr readline, AjPRegexp regexp,
                            EmbPBtreeField field)
{
    AjPStr tmpfd = NULL;

    ajStrAssignS(&embindexLine,readline);

    while(ajRegExec(regexp, embindexLine))
    {
        if(field->freecount)
            tmpfd = field->freelist[--field->freecount];

        ajRegSubI(regexp, 1, &tmpfd);
        ajRegPost(regexp, &embindexLine);

        ajStrTrimWhiteEnd(&tmpfd);

        if(!ajStrGetLen(tmpfd))
        {
            ajStrDel(&tmpfd);
            continue;
        }

        ajListstrPushAppend(field->data,tmpfd);
        ajDebug("++%S '%S'\n", field->name, tmpfd);
        tmpfd = NULL;
    }

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
**
** @release 6.4.0
******************************************************************************/

void embBtreeReportEntry(AjPFile outf, const EmbPBtreeEntry entry)
{
    if(entry->idtruncate)
        ajFmtPrintF(outf,
                    "Entry idlen %u truncated %u IDs. "
                    "Maximum ID length was %u for '%S'.\n",
                    entry->idlen, entry->idtruncate,
                    entry->idmaxlen, entry->maxid);
    else
        ajFmtPrintF(outf,
                    "Entry idlen %u OK. "
                    "Maximum ID length was %u for '%S'.\n",
                    entry->idlen,
                    entry->idmaxlen, entry->maxid);

    if(entry->idmaxlen > entry->idlen)
    {
        ajWarn("Entry idlen %u truncated %u IDs. "
               "Maximum ID length was %u for '%S'.",
               entry->idlen, entry->idtruncate,
               entry->idmaxlen, entry->maxid);
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
**
** @release 6.4.0
******************************************************************************/

void embBtreeReportField(AjPFile outf, const EmbPBtreeField field)
{
    if(field->truncate)
        ajFmtPrintF(outf,
                    "Field %S %Slen %u truncated %u terms. "
                    "Maximum %S term length was %u for '%S'.\n",
                    field->name, field->name, field->len, field->truncate,
                    field->name, field->maxlen, field->maxkey);
    else
        ajFmtPrintF(outf,
                    "Field %S %Slen %u OK. "
                    "Maximum %S term length was %u for '%S'.\n",
                    field->name, field->name, field->len,
                    field->name, field->maxlen, field->maxkey);

    if(field->maxlen > field->len)
    {
        ajWarn("Field %S %Slen %u truncated %u terms. "
               "Maximum %S term length was %u for '%S'.",
               field->name, field->name, field->len, field->truncate,
               field->name, field->maxlen, field->maxkey);
    }

    return;
}




/* @func embBtreeEmblKW *******************************************************
**
** Extract keywords from an EMBL KW line 
**
** @param [r] kwline [const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeEmblKW(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, kwline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPush(kwlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeEmblTX *******************************************************
**
** Extract keywords from an EMBL OC or OS line 
**
** @param [r] txline [const AjPStr] taxonomy line
** @param [w] txlist [AjPList] list of taxons
** @param [r] maxlen [ajuint] max taxon length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeEmblTX(const AjPStr txline, AjPList txlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, txline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;()");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPush(txlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeEmblAC *******************************************************
**
** Extract accession numbers from an EMBL AC line 
**
** @param [r] acline[const AjPStr] AC line
** @param [w] aclist [AjPList] list of accession numbers
**
** @return [void]
**
** @release 3.0.0
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
		ajListstrPush(aclist, str);
		str = NULL;
	    }
	}
	else
	{
	    ajStrAssignS(&str,embindexToken);
	    ajListstrPush(aclist, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeEmblSV *******************************************************
**
** Extract sequence version from an EMBL new format ID line 
**
** @param [r] idline[const AjPStr] AC line
** @param [w] svlist [AjPList] list of accession numbers
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void embBtreeEmblSV(const AjPStr idline, AjPList svlist)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    AjPStr idstr      = NULL;
    AjPStr svstr      = NULL;
    
    ajStrAssignSubS(&embindexLine, idline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine," \t\n\r;");

    if(!ajStrTokenNextParse(&embindexHandle,&idstr))
	return;

    if(!ajStrTokenNextParse(&embindexHandle,&token))
	return;

    if(!ajStrTokenNextParse(&embindexHandle,&svstr))
	return;

    if(!ajStrMatchC(token, "SV"))
	return;

    str = ajStrNewRes(MAJSTRGETLEN(idstr)+MAJSTRGETLEN(svstr)+2);

    ajFmtPrintS(&str,"%S.%S", idstr, svstr);

    ajListstrPush(svlist, str);
    str = NULL;
    
    ajStrDel(&idstr);
    ajStrDel(&svstr);
    ajStrDel(&token);

    return;
}




/* @func embBtreeEmblDE *******************************************************
**
** Extract words from an EMBL DE line 
**
** @param [r] deline[const AjPStr] description line
** @param [w] delist [AjPList] list of descriptions
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeEmblDE(const AjPStr deline, AjPList delist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, deline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r \t()");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPush(delist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeParseEmblKw **************************************************
**
** Extract keywords from an EMBL KW line 
**
** @param [r] readline [const AjPStr] keyword line
** @param [u] field [EmbPBtreeField] list of descriptions
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseEmblKw(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

            ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeParseEmblTx **************************************************
**
** Extract keywords from an EMBL OC or OS line 
**
** @param [r] readline [const AjPStr] taxonomy line
** @param [u] field [EmbPBtreeField] list of descriptions
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseEmblTx(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;

    ajStrAssignSubS(&embindexLine, readline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;()");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimEndC(&embindexToken," ");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

            ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeParseEmblAc **************************************************
**
** Extract accession numbers from an EMBL AC line 
**
** @param [r] readline[const AjPStr] AC line
** @param [u] field [EmbPBtreeField] list of descriptions
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseEmblAc(const AjPStr readline, EmbPBtreeField field)
{
    char *p          = NULL;
    char *q          = NULL;
    ajuint lo = 0;
    ajuint hi = 0;
    ajuint ifield = 0;
    ajuint i;
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimWhite(&embindexToken);

        if(field->freecount)
            str = field->freelist[--field->freecount];

        /* Check for EMBL accession range */
	if((p=strchr(MAJSTRGETPTR(embindexToken),(int)'-')))
	{
	    q = p;

	    while(isdigit((int)*(--q)));

	    ++q;
	    ajStrAssignSubC(&embindexTstr,q,0,(ajuint)(p-q-1));
	    ajStrToUint(embindexTstr,&lo);
	    ifield = (ajuint) (p-q);
	    ajFmtPrintS(&embindexFormat,"%%S%%0%uu",ifield);
	    
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
		ajListstrPushAppend(field->data, str);
		str = NULL;
	    }
	}
	else                    /* simple accession number */
	{
	    ajStrAssignS(&str,embindexToken);
	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeFindEmblAc ***************************************************
**
** Returns first accession number from an EMBL/UniProt AC line 
**
** @param [r] readline[const AjPStr] AC line
** @param [u] field [EmbPBtreeField] list of descriptions
** @param [w] Pstr [AjPStr*] First accession
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeFindEmblAc(const AjPStr readline, EmbPBtreeField field,
                        AjPStr *Pstr)
{
    char *p          = NULL;
    char *q          = NULL;
    ajuint lo = 0;
    ajuint hi = 0;
    ajuint ifield = 0;
    ajuint i;
    
    ajStrAssignSubS(&embindexLine, readline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine," \t\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimWhite(&embindexToken);

        /* Check for EMBL accession range */
	if((p=strchr(MAJSTRGETPTR(embindexToken),(int)'-')))
	{
	    q = p;

	    while(isdigit((int)*(--q)));

	    ++q;
	    ajStrAssignSubC(&embindexTstr,q,0,(ajuint)(p-q-1));
	    ajStrToUint(embindexTstr,&lo);
	    ifield = (ajuint) (p-q);
	    ajFmtPrintS(&embindexFormat,"%%S%%0%uu",ifield);
	    
	    ++p;
	    q = p;

	    while(!isdigit((int)*q))
		++q;

	    sscanf(q,"%u",&hi);
	    ajStrAssignSubC(&embindexPrefix,p,0,(ajuint)(q-p-1));
	    
	    for(i=lo;i<=hi;++i)
	    {
		if(field->freecount)
                    *Pstr = field->freelist[--field->freecount];
                ajFmtPrintS(Pstr,MAJSTRGETPTR(embindexFormat),
			    embindexPrefix,i);
		return;
	    }
	}
	else                    /* simple accession number */
	{
            if(field->freecount)
                *Pstr = field->freelist[--field->freecount];
	    ajStrAssignS(Pstr,embindexToken);
	    return;
	}
    }

    return;
}




/* @func embBtreeParseEmblSv **************************************************
**
** Extract sequence version from an EMBL new format ID line 
**
** @param [r] readline[const AjPStr] AC line
** @param [u] field [EmbPBtreeField] list of descriptions
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseEmblSv(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine," \t\n\r;");

    if(!ajStrTokenNextParse(&embindexHandle,&embindexToken))
	return;

    if(field->freecount)
        str = field->freelist[--field->freecount];

    ajStrAssignS(&str, embindexToken);
    ajStrAppendK(&str, '.');

    if(!ajStrTokenNextParse(&embindexHandle,&embindexToken))
	return;

    if(!ajStrMatchC(embindexToken, "SV"))
	return;

    if(!ajStrTokenNextParse(&embindexHandle,&embindexToken))
	return;

    ajStrAppendS(&str, embindexToken);

    ajListstrPushAppend(field->data, str);
    str = NULL;
    
    return;
}




/* @func embBtreeParseEmblDe **************************************************
**
** Extract words from an EMBL DE line 
**
** @param [r] readline[const AjPStr] description line
** @param [u] field [EmbPBtreeField] list of descriptions
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseEmblDe(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 5, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r \t()");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimWhite(&embindexToken);
	ajStrTrimEndC(&embindexToken,".,:;'\"");
	ajStrTrimStartC(&embindexToken,"'\"");

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

            ajStrAssignS(&str, embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeParseGenbankAc ***********************************************
**
** Extract accession numbers from a GenBank ACCESSION line 
**
** @param [r] readline [const AjPStr] AC line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseGenbankAc(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str       = NULL;
    char *p          = NULL;
    char *q          = NULL;
    ajuint lo = 0;
    ajuint hi = 0;
    ajuint ifield = 0;
    ajuint i;

    ajStrAssignSubS(&embindexLine, readline, 12, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r ");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimWhite(&embindexToken);

        /* check for accession number range */
	if((p=strchr(MAJSTRGETPTR(embindexToken),(int)'-')))
	{
	    q = p;

	    while(isdigit((int)*(--q)));

	    ++q;
	    ajStrAssignSubC(&embindexTstr,q,0,(ajuint)(p-q-1));
	    ajStrToUint(embindexTstr,&lo);
	    ifield = (ajuint) (p-q);
	    ajFmtPrintS(&embindexFormat,"%%S%%0%uu",ifield);
	    
	    ++p;
	    q = p;

	    while(!isdigit((int)*q))
		++q;

	    sscanf(q,"%u",&hi);
	    ajStrAssignSubC(&embindexPrefix,p,0,(ajuint)(q-p-1));
	    
	    for(i=lo;i<=hi;++i)
	    {
                if(field->freecount)
                    str = field->freelist[--field->freecount];

		ajFmtPrintS(&str,MAJSTRGETPTR(embindexFormat),embindexPrefix,i);
		ajListstrPushAppend(field->data, str);
		str = NULL;
	    }
	}
	else                    /* simple accession number */
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

	    ajStrAssignS(&str,embindexToken);
	    ajListstrPushAppend(field->data, str);
        }
    }

    return;
}




/* @func embBtreeParseGenbankDe ***********************************************
**
** Extract keywords from a GenBank DESCRIPTION line 
**
** @param [r] readline [const AjPStr] AC line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseGenbankDe(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 10, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r \t()");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
	    ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeParseGenbankKw ***********************************************
**
** Extract keywords from a GenBank KEYWORDS line 
**
** @param [r] readline [const AjPStr] AC line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseGenbankKw(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 8, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

	    ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeParseGenbankTx ***********************************************
**
** Extract keywords from a GenBank ORGANISM line 
**
** @param [r] readline [const AjPStr] AC line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseGenbankTx(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;
    
    ajStrAssignSubS(&embindexLine, readline, 9, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;()");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimEndC(&embindexToken," ");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

	    ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeGenBankAC ****************************************************
**
** Extract accession numbers from a GenBank ACCESSION line 
**
** @param [r] acline[const AjPStr] AC line
** @param [w] aclist [AjPList] list of accession numbers
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeGenBankAC(const AjPStr acline, AjPList aclist)
{
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

    ajStrAssignSubS(&embindexLine, acline, 12, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r ");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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
		ajListstrPushAppend(aclist, str);
		str = NULL;
	    }
	}
	else
	{
	    ajStrAssignS(&str,token);
	    ajListstrPushAppend(aclist, str);
	    str = NULL;
	}
    }

    ajStrDel(&tstr);
    ajStrDel(&prefix);
    ajStrDel(&format);
    ajStrDel(&token);
    
    return;
}




/* @func embBtreeGenBankKW ****************************************************
**
** Extract keywords from a GenBank KEYWORDS line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeGenBankKW(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, kwline, 8, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPushAppend(kwlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeGenBankDE ****************************************************
**
** Extract keywords from a GenBank DESCRIPTION line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeGenBankDE(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, kwline, 10, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r \t()");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPushAppend(kwlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeGenBankTX ****************************************************
**
** Extract keywords from a GenBank ORGANISM line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeGenBankTX(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrAssignSubS(&embindexLine, kwline, 9, -1);

    ajStrTokenAssignC(&embindexHandle,embindexLine,"\n\r;()");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPushAppend(kwlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeParseFastaDe *************************************************
**
** Extract keywords from a Fasta description
**
** @param [r] readline [const AjPStr] keyword line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseFastaDe(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;
    
    ajStrTokenAssignC(&embindexHandle,readline,"\n\r ");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

            ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeParseFastaAc *************************************************
**
** Extract sequence version keywords from a Fasta description
**
** @param [r] readline [const AjPStr] keyword line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseFastaAc(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;

    ajStrTokenAssignC(&embindexHandle,readline,"\n\r ");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

	    ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeFastaDE ******************************************************
**
** Extract keywords from a Fasta description
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
** @param [r] maxlen [ajuint] max keyword length
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeFastaDE(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    ajStrTokenAssignC(&embindexHandle,kwline,"\n\r ");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPushAppend(kwlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
    return;
}




/* @func embBtreeParseFastaSv *************************************************
**
** Extract sequence version keywords from a Fasta description
**
** @param [r] readline [const AjPStr] keyword line
** @param [u] field [EmbPBtreeField] Field with list of data
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void embBtreeParseFastaSv(const AjPStr readline, EmbPBtreeField field)
{
    AjPStr str = NULL;

    ajStrTokenAssignC(&embindexHandle,readline,"\n\r ");

    while(ajStrTokenNextParse(&embindexHandle,&embindexToken))
    {
	ajStrTrimEndC(&embindexToken,".");
	ajStrTrimWhite(&embindexToken);

	if(ajStrGetLen(embindexToken))
	{
            if(field->freecount)
                str = field->freelist[--field->freecount];

	    ajStrAssignS(&str,embindexToken);

	    ajListstrPushAppend(field->data, str);
	    str = NULL;
	}
    }

    return;
}




/* @func embBtreeFastaSV ******************************************************
**
** Extract sequence version keywords from a Fasta description
**
** @param [r] kwline[const AjPStr] sequence version or GI string
** @param [w] kwlist [AjPList] list of sequence versions
** @param [r] maxlen [ajuint] max sequence version length
**
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

void embBtreeFastaSV(const AjPStr kwline, AjPList kwlist, ajuint maxlen)
{
    AjPStr token     = NULL;
    AjPStr str       = NULL;

    ajStrTokenAssignC(&embindexHandle,kwline,"\n ");

    while(ajStrTokenNextParse(&embindexHandle,&token))
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

	    ajListstrPushAppend(kwlist, str);
	    str = NULL;
	}
    }

    ajStrDel(&token);
    
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
**
** @release 2.8.0
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
	    ajListstrPushAppend(lfiles, file);
    }

    nfiles =  (ajuint) ajListToarray(lfiles,(void ***)&(*filelist));
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
**
** @release 2.9.0
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




/* @func embBtreeEntryNew *****************************************************
**
** Construct a database entry object
**
** @param [r] refcount [ajuint] Number of reference file(s) per entry
** @return [EmbPBtreeEntry] db entry object pointer
**
** @release 3.0.0
** @@
******************************************************************************/

EmbPBtreeEntry embBtreeEntryNew(ajuint refcount)
{
    EmbPBtreeEntry thys;
    ajuint  iref;

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

    thys->id = ajStrNew();

    if(refcount)
    {
        thys->refcount = refcount;

        AJCNEW0(thys->reffpos, refcount);
        AJCNEW0(thys->reffiles, refcount);

        for(iref=0; iref < refcount; iref++)
            thys->reffiles[iref] = ajListNew();
    }

    return thys;
}




/* @func embBtreeEntryDel *****************************************************
**
** Delete a database entry object
**
** @param [d] pthis [EmbPBtreeEntry*] db entry object pointer
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void embBtreeEntryDel(EmbPBtreeEntry* pthis)
{
    EmbPBtreeEntry thys;
    EmbPBtreeField field;
    AjPStr tmpstr = NULL;

    ajuint iref;

    thys = *pthis;

    ajStrDel(&thys->dbname);
    ajStrDel(&thys->idextension);
    ajStrDel(&thys->maxid);
    ajStrDel(&thys->dbrs);
    ajStrDel(&thys->date);
    ajStrDel(&thys->release);
    ajStrDel(&thys->dbtype);

    ajStrDel(&thys->directory);
    ajStrDel(&thys->idirectory);


    while(ajListPop(thys->files,(void **)&tmpstr))
	ajStrDel(&tmpstr);

    ajListFree(&thys->files);

    if(thys->reffiles)
    {
        for(iref=0; iref < thys->refcount; iref++)
        {
            while(ajListPop(thys->reffiles[iref],(void **)&tmpstr))
                ajStrDel(&tmpstr);

            ajListFree(&thys->reffiles[iref]);
        }

        AJFREE(thys->reffiles);
    }

    if(thys->reffpos)
        AJFREE(thys->reffpos);

    while(ajListPop(thys->fields,(void **)&field))
	embBtreeFieldDel(&field);

    ajListFree(&thys->fields);

    ajStrDel(&thys->id);

    AJFREE(*pthis);

    return;
}




/* @func embBtreeEntrySetCompressed *******************************************
**
** Set database entry to be compressed on writing
**
** @param [u] entry [EmbPBtreeEntry] Database entry information
**
** @return [void]
**
** @release 6.4.0
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
**
** @release 3.0.0
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
            field = embBtreeFieldNewS(fields[nfields], entry->refcount);
            ajListPushAppend(entry->fields, field);
            field = NULL;
        }
	++nfields;
    }
    
    return nfields;
}




/* @func embBtreeSetDbInfo ****************************************************
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
**
** @release 3.0.0
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




/* @func embBtreeGetFieldC ****************************************************
**
** Set database fields to index
**
** @param [w] entry [EmbPBtreeEntry] Database entry information
** @param [r] nametxt [const char*] Field name
**
** @return [EmbPBtreeField] Btree index field definition
**
** @release 6.4.0
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




/* @func embBtreeGetFieldS ****************************************************
**
** Set database fields to index
**
** @param [w] entry [EmbPBtreeEntry] Database entry information
** @param [r] name [const AjPStr] Field name
**
** @return [EmbPBtreeField] Btree index field definition
**
** @release 6.4.0
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
**
** @release 3.0.0
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
	    ajListstrPushAppend(entry->files, file);
	    ++count;
	}
    }

    ajListSort(entry->files, &ajStrVcmp);

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
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool embBtreeWriteEntryFile(const EmbPBtreeEntry entry)
{
    AjPFile entfile = NULL;
    ajuint i;
    ajuint iref;
    AjPStr tmpstr = NULL;
    AjPStr refstr = NULL;
    
    /* ajDebug("In embBtreeWriteEntryFile\n"); */

    entfile = btreeCreateFile(entry->idirectory,entry->dbname,BTENTRYFILE);
    if(!entfile)
    {
        ajWarn("Failed to create file '%S' entry->dbname "
               "in directory 'entry->idirectory' "
               "error:%d '%s'",
               errno, strerror(errno));
	return ajFalse;
    }

    ajFmtPrintF(entfile,"# Number of files: %u\n",entry->nfiles);
    ajFmtPrintF(entfile,"# Release: %S\n",entry->release);
    ajFmtPrintF(entfile,"# Date:    %S\n",entry->date);

    if(!entry->refcount)
	ajFmtPrintF(entfile,"Single");
    else
	ajFmtPrintF(entfile,"Reference %u", entry->refcount+1);

    ajFmtPrintF(entfile," filename database\n");
    
    for(i=0;i<entry->nfiles;++i)
	if(!entry->refcount)
	{
	    ajListPop(entry->files,(void **)&tmpstr);
	    ajFmtPrintF(entfile,"%S\n",tmpstr);
	    ajListstrPushAppend(entry->files, tmpstr);
	}
	else
	{
	    ajListPop(entry->files,(void **)&tmpstr);
	    ajFmtPrintF(entfile,"%S",tmpstr);
            for(iref=0; iref < entry->refcount; iref++)
            {
                ajListPop(entry->reffiles[iref],(void **)&refstr);
                ajFmtPrintF(entfile," %S",refstr);
                ajListstrPushAppend(entry->reffiles[iref], refstr);
            }

            ajFmtPrintF(entfile,"\n");
	    ajListstrPushAppend(entry->files, tmpstr);
	}

    ajFileClose(&entfile);
    
    return ajTrue;
}




/* @func embBtreeGetRsInfo ****************************************************
**
** Get resource information for selected database
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [void]
**
** @release 3.0.0
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

    entry->pricachesize = BT_CACHESIZE;

    if(ajNamRsAttrValueC(MAJSTRGETPTR(entry->dbrs),"cachesize",&value) ||
       ajNamGetValueC("CACHESIZE",&value))
    {
	if(ajStrToUint(value,&n))
	    entry->pricachesize = n;
	else
	    ajErr("Bad value for environment variable 'CACHESIZE'");
    }
    else
    {
	ajDebug("CACHESIZE defaults to %d\n", entry->pricachesize);
    }

    entry->seccachesize = entry->pricachesize;

    if(ajNamRsAttrValueC(MAJSTRGETPTR(entry->dbrs),"seccachesize",&value) ||
       ajNamGetValueC("SECCACHESIZE",&value))
    {
	if(ajStrToUint(value,&n))
	    entry->seccachesize = n;
	else
	    ajErr("Bad value for environment variable 'SECCACHESIZE'");
    }
    else
    {
	ajDebug("SECCACHESIZE defaults to %d\n", entry->seccachesize);
    }

    entry->pripagesize = BT_PAGESIZE;

    if(ajNamRsAttrValueC(MAJSTRGETPTR(entry->dbrs),"pagesize",&value) ||
       ajNamGetValueC("PAGESIZE",&value))
    {
	if(ajStrToUint(value,&n))
	    entry->pripagesize = n;
	else
	    ajErr("Bad value for environment variable 'PAGESIZE'");
    }
    else
    {
	ajDebug("PAGESIZE defaults to %d\n", entry->pripagesize);
    }

    entry->secpagesize = entry->pripagesize;

    if(ajNamRsAttrValueC(MAJSTRGETPTR(entry->dbrs),"secpagesize",&value) ||
       ajNamGetValueC("SECPAGESIZE",&value))
    {
	if(ajStrToUint(value,&n))
	    entry->secpagesize = n;
	else
	    ajErr("Bad value for environment variable 'SECPAGESIZE'");
    }
    else
    {
	ajDebug("SECPAGESIZE defaults to %d\n", entry->secpagesize);
    }

    entry->idlen = BT_KWLIMIT;
    ajStrAssignC(&attrstr, "idlen");

    if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
    {
	if(ajStrToUint(value,&n))
	    entry->idlen = n;
	else
	    ajErr("Bad value for index resource 'idlen'");
    }

    ajStrAssignC(&attrstr, "idpagesize");
    if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
    {
        if(ajStrToUint(value,&n))
	    entry->pripagesize = n;
	else
	    ajErr("Bad value for index resource 'idpagesize'");
    }

    ajStrAssignC(&attrstr, "idsecpagesize");
    if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
    {
        if(ajStrToUint(value,&n))
	    entry->secpagesize = n;
	else
	    ajErr("Bad value for index resource 'idsecpagesize'");
    }

    ajStrAssignC(&attrstr, "idcachesize");
    if(ajNamRsAttrValueS(entry->dbrs, attrstr, &value))
    {
        if(ajStrToUint(value,&n))
	    entry->pricachesize = n;
	else
	    ajErr("Bad value for index resource 'idcachesize'");
    }
        
    ajStrAssignC(&attrstr, "idseccachesize");
    if(ajNamRsAttrValueS(entry->dbrs, attrstr, &value))
    {
        if(ajStrToUint(value,&n))
	    entry->seccachesize = n;
	else
	    ajErr("Bad value for index resource 'idseccachesize'");
    }
        
    if(!entry->secpagesize)
        entry->secpagesize = entry->pripagesize;

    if(!entry->seccachesize)
        entry->seccachesize = entry->pricachesize;

    entry->idorder = (entry->pripagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
        ((entry->idlen + 1) + BT_IDKEYEXTRA);

    entry->idfill  = (entry->pripagesize - BT_BUCKPREAMBLE) /
        ((entry->idlen + 1) + BT_KEYLENENTRY +
         BT_DDOFF + entry->refcount*BT_EXTRA);

    entry->idsecorder = (entry->secpagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
        (BT_OFFKEYLEN + BT_IDKEYEXTRA);

    entry->idsecfill  = (entry->secpagesize - BT_BUCKPREAMBLE) /
        (BT_DOFF + entry->refcount*BT_EXTRA);

/* now process the same values for each index field */

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);

            field->idlen = entry->idlen;

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

            field->pripagesize = entry->pripagesize;
            field->secpagesize = entry->secpagesize;

            ajFmtPrintS(&attrstr, "%Spagesize", field->name);

            if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
            {
                if(ajStrToUint(value,&n))
                {
                    field->pripagesize = n;
                    field->secpagesize = n;
                }
                else
                {
                    ajErr("Bad value for index resource '%S'", attrstr);
                    field->pripagesize = entry->pripagesize;
                }
            }
        

            ajFmtPrintS(&attrstr, "%Ssecpagesize", field->name);

            if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
            {
                if(ajStrToUint(value,&n))
                {
                    field->secpagesize = n;
                }
                else
                {
                    ajErr("Bad value for index resource '%S'", attrstr);
                }
            }
        
            field->pricachesize = entry->pricachesize;
            field->seccachesize = entry->seccachesize;

            ajFmtPrintS(&attrstr, "%Scachesize", field->name);

            if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
            {
                if(ajStrToUint(value,&n))
                {
                    field->pricachesize = n;
                    field->seccachesize = n;
                }
                else
                {
                    ajErr("Bad value for index resource '%Scachesize'",
                          field->name);
                    field->pricachesize = entry->pricachesize;
                }
            }
        
            ajFmtPrintS(&attrstr, "%Sseccachesize", field->name);

            if(ajNamRsAttrValueS(entry->dbrs,attrstr,&value))
            {
                if(ajStrToUint(value,&n))
                {
                    field->seccachesize = n;
                }
                else
                {
                    ajErr("Bad value for index resource '%S'", attrstr);
                }
            }
        
            field->order =
                (field->pripagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
                ((field->len + 1) + BT_IDKEYEXTRA);

            field->fill  =
                (field->pripagesize - BT_BUCKPREAMBLE) /
                ((field->len + 1) + BT_KEYLENENTRY +
                 BT_DDOFF + field->refcount*BT_EXTRA);

            if(!field->secondary)
            {
                field->secorder =
                    (field->secpagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
                    (BT_OFFKEYLEN + BT_IDKEYEXTRA);
                field->secfill  =
                    (field->secpagesize - BT_BUCKPREAMBLE) /
                    (BT_DOFF + field->refcount*BT_EXTRA);
            }
            else
            {
    /*
     *  The secondary tree keys are the IDs of the entries containing
     *  the keywords so we use the entry idlen for their size limit
     */
                field->secorder =
                    (field->secpagesize - (BT_NODEPREAMBLE + BT_PTRLEN)) /
                    ((entry->idlen + 1) + BT_IDKEYEXTRA);
                field->secfill  =
                    (field->secpagesize - BT_BUCKPREAMBLE) /
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
**
** @release 3.0.0
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
	entry->idcache = ajBtreeIdcacheNewS(entry->dbname,
                                            entry->idextension,
                                            entry->idirectory,
                                            "wb+",
                                            entry->compressed,
                                            entry->idlen,
                                            entry->refcount,
                                            entry->pripagesize,
                                            entry->secpagesize,
                                            entry->pricachesize,
                                            entry->seccachesize,
                                            0,
                                            0,
                                            entry->idorder,
                                            entry->idfill,
                                            level,
                                            entry->idsecorder,
                                            entry->idsecfill,
                                            count,
                                            countall);
	if(!entry->idcache)
	    ajFatal("Cannot open ID index");
    }

    if(ajListGetLength(entry->fields))
    {
        iter = ajListIterNewread(entry->fields);

        while(!ajListIterDone(iter))
        {
            field = ajListIterGet(iter);

            if(field->secondary)
                field->cache = ajBtreeSeccacheNewS(entry->dbname,
                                                   field->extension,
                                                   entry->idirectory,
                                                   "wb+",
                                                   field->compressed,
                                                   field->len,
                                                   field->idlen,
                                                   field->pripagesize,
                                                   field->secpagesize,
                                                   field->pricachesize,
                                                   field->seccachesize,
                                                   field->pripagecount,
                                                   field->secpagecount,
                                                   field->order,
                                                   field->fill,
                                                   level,
                                                   field->secorder,
                                                   field->secfill,
                                                   count,
                                                   countall);
            else
                field->cache = ajBtreeIdcacheNewS(entry->dbname,
                                                  field->extension,
                                                  entry->idirectory,
                                                  "wb+",
                                                  field->compressed,
                                                  field->len,
                                                  field->refcount,
                                                  field->pripagesize,
                                                  field->secpagesize,
                                                  field->pricachesize,
                                                  field->seccachesize,
                                                  field->pripagecount,
                                                  field->secpagecount,
                                                  field->order,
                                                  field->fill,
                                                  level,
                                                  field->secorder,
                                                  field->secfill,
                                                  count,
                                                  countall);
            if(!field->cache)
                ajFatal("Cannot open %S index", field->extension);
        }

        ajListIterDel(&iter);
    }

    return ajTrue;
}




/* @func embBtreeCloseCaches **************************************************
**
** Close index files
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
**
** @release 3.0.0
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
/* @func embBtreeProbeCaches **************************************************
**
** Close index files
**
** @param [u] entry [EmbPBtreeEntry] database data
**
** @return [AjBool] true on success
**
** @release 6.0.0
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
**
** @release 3.0.0
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




/* @func embBtreeFieldNewC ****************************************************
**
** Constructor for a Btree index field
**
** @param [r] nametxt [const char*] Name
** @return [EmbPBtreeField] Btree field
**
** @release 6.4.0
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




/* @func embBtreeFieldNewS ****************************************************
**
** Constructor for a Btree index field
**
** @param [r] name [const AjPStr] Name
** @param [r] refcount [ajuint] Number of reference files
** @return [EmbPBtreeField] Btree field
**
** @release 6.4.0
******************************************************************************/

EmbPBtreeField embBtreeFieldNewS(const AjPStr name, ajuint refcount)
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

    ret->refcount = refcount;

    return ret;
}




/* @func embBtreeFieldDel *****************************************************
**
** Destructor for a Btree index field
**
** @param [d] Pthis [EmbPBtreeField*] Btree index field object
** @return [void]
**
** @release 6.4.0
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

    while(thys->freecount)
        ajStrDel(&thys->freelist[--thys->freecount]);

    if(thys->freelist)
        AJFREE(thys->freelist);

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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
** @@
******************************************************************************/

void embBtreeFieldSetSecondary(EmbPBtreeField field)
{
    field->secondary = ajTrue;

    return;
}




/* @func embIndexExit *********************************************************
**
** Cleanup indexing internals on exit
**
** @return [void]
**
** @release 6.0.0
******************************************************************************/

void embIndexExit(void)
{
    ajStrDel(&embindexLine);
    ajStrDel(&embindexToken);
    ajStrDel(&embindexTstr);
    ajStrDel(&embindexPrefix);
    ajStrDel(&embindexFormat);
    ajStrTokenDel(&embindexHandle);

    ajStrDel(&indexWord);
    ajBtreeIdDel(&indexId);

    return;
}
