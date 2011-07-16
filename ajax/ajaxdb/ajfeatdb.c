/******************************************************************************
** @source AJAX feature database access functions
**
** These functions control all aspects of AJAX feature database access
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Sep 2010 pmr first version
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
#include "ajfeatdb.h"
#include "ajdas.h"

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


static AjBool featAccessDas(AjPFeattabin ftabin);
static AjBool featAccessChado(AjPFeattabin fttabin);

static AjPSqlconnection featChadoConnect(const AjPQuery qry);
static void featChadoChildfeatureQuery(AjPSqlconnection connection,
			               AjPFeattable  feattab,
                                       const AjPStr srcfeature);

static AjBool featChadoQryfeatureQuery(AjPSqlconnection connection, AjPStr sql,
			               AjPFeattable feattab,
			               ajint qrystart, ajint qryend);

static AjPFeature featChadoChildfeatureRow(AjPFeattable fttab, AjPSqlrow line);
static AjPStr featChadoQryfeatureRow(AjPFeattable fttab, AjPSqlrow row,
				     ajint qrystart, ajint qryend);



/* @datastatic FeatPEmbossQry ************************************************
**
** Btree 'emboss' query structure
**
** @alias FeatSEmbossQry
** @alias FeatOEmbossQry
**
** @attr idcache [AjPBtcache] ID cache
** @attr Caches [AjPList] Caches for each query field
** @attr files [AjPStr*] database filenames
** @attr reffiles [AjPStr*] database reference filenames
** @attr Skip [AjBool*] files numbers to exclude
** @attr List [AjPList] List of files
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

typedef struct FeatSEmbossQry
{
    AjPBtcache idcache;
    AjPList Caches;

    AjPStr *files;
    AjPStr *reffiles;
    AjBool *Skip;

    AjPList List;

    AjPFile libs;
    AjPFile libr;

    ajuint div;
    ajint nentries;

    AjBool Samefile;
    char Padding[4];
} FeatOEmbossQry;

#define FeatPEmbossQry FeatOEmbossQry*




/* @funclist feattabAccess ****************************************************
**
** Functions to access each database or feature access method
**
******************************************************************************/

static AjOFeattabAccess feattabAccess[] =
{
  /* Name      AccessFunction   FreeFunction
     Qlink    Description
     Alias    Entry    Query    All      Chunk */

    {"das", featAccessDas, NULL,
     "&",       "retrieve features from a DAS server",
      AJFALSE, AJTRUE,  AJFALSE,  AJFALSE, AJFALSE
    },
    {"chado", featAccessChado, NULL,
     "",       "retrieve features from a CHADO server",
      AJFALSE, AJTRUE,  AJFALSE,  AJFALSE, AJFALSE
    },
    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE}
};




/* @funcstatic featAccessChado ************************************************
**
** Reads features from CHADO databases
**
** @param [u] fttabin [AjPFeattabin] Feature input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featAccessChado(AjPFeattabin fttabin)
{
    AjPQuery qry = NULL;
    AjPStr seqid = NULL;

    AjPStr filterseqregions = NULL;

    AjIList fielditer     = NULL;
    AjPQueryField field   = NULL;
    AjPFeattable  feattab = NULL;

    AjPStr fieldname = NULL;
    AjPStr condition = NULL;

    AjPSqlconnection connection = NULL;

    AjBool ret = ajTrue;

#if !defined(HAVE_MYSQL) && !defined(HAVE_POSTGRESQL)
    ajWarn("Cannot use access method chado without mysql or postgresql");
    return ajFalse;
#endif

    if(fttabin->Input->Records)
	return ajFalse;

    qry = fttabin->Input->Query;

    ajDebug("featAccesschado: %S fields: %u\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    filterseqregions = ajStrNew();

    fielditer = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(fielditer))
    {
	field = ajListIterGet(fielditer);

	ajStrAssignS(&fieldname, field->Field);

	ajDebug("field:%S - val:%S\n", field->Field, field->Wildquery);

	if(ajStrMatchCaseC(field->Field, "id"))
	{
	    ajStrAssignS(&seqid, field->Wildquery);
	    ajStrAssignC(&fieldname, "uniquename");
	}

	if(ajStrFindAnyC(field->Wildquery,"*?") != -1)
	{
	    ajStrExchangeKK(&field->Wildquery,'*','%');
	    ajStrExchangeKK(&field->Wildquery,'?','_');
	    ajFmtPrintS(&condition," %S LIKE '%S'", fieldname,
	                   field->Wildquery);
	}
	else
	    ajFmtPrintS(&condition,"%S = '%S'", fieldname,
	                   field->Wildquery);

	if (ajStrGetLen(filterseqregions))
	    ajStrAppendC(&filterseqregions," AND ");

	ajStrAppendS(&filterseqregions, condition);

	ajStrDel(&fieldname);
	ajStrDel(&condition);
    }

    ajListIterDel(&fielditer);

    ajDebug("dbfilter: %S\n", qry->DbFilter);

    if(ajStrGetLen(qry->DbFilter))
        ajFmtPrintAppS(&filterseqregions, " AND %S", qry->DbFilter);

    ajDebug("filter: %S\n", filterseqregions);

    feattab = ajFeattableNew(seqid);
    connection = featChadoConnect(qry);

    if(!ajStrGetLen(feattab->Db))
	ajStrAssignS(&feattab->Db, qry->DbName);

    ret = featChadoQryfeatureQuery(connection, filterseqregions, feattab,
                                   fttabin->Start, fttabin->End);

    ajSqlconnectionDel(&connection);

    fttabin->Input->TextData = feattab;

    qry->QryDone = ajTrue;

    ajStrDel(&seqid);
    ajStrDel(&filterseqregions);

    return ret;
}




/* @funcstatic featChadoQryfeatureQuery ***************************************
**
** Queries the connected chado database for the emboss feature query.
** More queries are made for the child features of the queried feature(s)
** using function featChadoChildfeatureQuery.
**
** @param [u] connection [AjPSqlconnection] SQL Database connection
** @param [u] filter [AjPStr] Filter conditions for the SQL query
** @param [u] feattab [AjPFeattable] Feature table
** @param [r] qrystart [ajint] start pos specified as part of the feature query
** @param [r] qryend [ajint] end pos specified as part of the feature query
** @return [AjBool] returns ajFalse if no features found
** @@
******************************************************************************/

static AjBool featChadoQryfeatureQuery(AjPSqlconnection connection,
				       AjPStr filter,
				       AjPFeattable  feattab,
				       ajint qrystart, ajint qryend)
{
    AjPSqlstatement statement = NULL;
    AjBool debugOn = ajFalse;
    ajint i = 0;
    AjPStr sql = NULL;

    AjISqlrow iter = NULL;
    AjPSqlrow row  = NULL;
    AjPStr colstr = NULL;
    AjPStr rowstr = NULL;
    AjPStr regionuniquename = NULL;

    AjBool ret = ajTrue;

    debugOn = ajDebugOn();
    sql = ajStrNew();

    ajFmtPrintS(&sql,
	    "SELECT feature.name, feature.uniquename,\n"
	    " (SELECT COUNT(*) FROM featureloc"
	    " WHERE feature.feature_id = featureloc.srcfeature_id),\n"
	    " srcfeatureloc.srcfeature_id,"
	    " srcfeature.uniquename,"
	    " srcfeatureloc.fmin,"
	    " srcfeatureloc.fmax,"
	    " srcfeatureloc.strand, srcfeatureloc.phase,\n"
	    " feature.seqlen,"
	    " cvterm.name, db.name,"
	    " feature.timelastmodified, feature.is_obsolete\n"
	    " FROM feature\n"
	    " LEFT JOIN cvterm ON cvterm.cvterm_id = feature.type_id\n"
	    " LEFT OUTER JOIN dbxref ON dbxref.dbxref_id = feature.dbxref_id\n"
	    " LEFT OUTER JOIN db USING (db_id)\n"
	    " LEFT OUTER JOIN featureloc srcfeatureloc"
	    " ON feature.feature_id = srcfeatureloc.feature_id\n"
	    " LEFT OUTER JOIN feature srcfeature"
	    " ON srcfeature.feature_id = srcfeatureloc.srcfeature_id\n"
	    " WHERE feature.%S\n",
	    filter
	    );

    ajDebug("SQL, query-feature query:\n%S\n",sql);

    statement = ajSqlstatementNewRun(connection,sql);

    if(!statement)
    {
	ajErr("Could not execute SQL statement [%S]", sql);
	ajExitBad();
	return ajFalse;
    }

    iter = ajSqlrowiterNew(statement);

    if(!iter)
	ret = ajFalse;

    while(!ajSqlrowiterDone(iter))
    {
	row = ajSqlrowiterGet(iter);

	if(debugOn)
	{
	    while(row->Current < row->Columns)
	    {
		ajSqlcolumnToStr(row,&colstr);
		ajFmtPrintAppS(&rowstr, "\t%S",colstr);
	    }

	    ajDebug("qryfeature row %d: %S\n", ++i, rowstr);
	    ajStrSetClear(&rowstr);
	}

	ajSqlcolumnRewind(row);

	regionuniquename = featChadoQryfeatureRow(feattab, row,
	                                          qrystart, qryend);
	featChadoChildfeatureQuery(connection, feattab, regionuniquename);
	ajStrDel(&regionuniquename);
    }

    ajSqlrowiterDel(&iter);
    ajSqlstatementDel(&statement);
    ajStrDel(&colstr);
    ajStrDel(&rowstr);
    ajStrDel(&sql);

    return ret;
}




/* @funcstatic featChadoChildfeatureQuery *************************************
**
** Query to retrieve features located on the feature specified
** using the srcfeature parameter
**
** @param [u] connection [AjPSqlconnection] SQL Database connection
** @param [u] feattab [AjPFeattable] Feature table
** @param [r] srcfeature [const AjPStr] Unique name of the sequence region
** @return [void]
** @@
******************************************************************************/

static void featChadoChildfeatureQuery(AjPSqlconnection connection,
			               AjPFeattable  feattab,
                                       const AjPStr srcfeature)
{
    AjPSqlstatement statement = NULL;
    AjBool debugOn = ajFalse;
    ajint i = 0;

    AjISqlrow iter = NULL;
    AjPSqlrow row  = NULL;
    AjPStr colstr  = NULL;
    AjPStr rowstr  = NULL;
    AjPStr sql     = NULL;
    AjPFeature seqregionf = NULL;

    debugOn = ajDebugOn();
    sql   = ajStrNew();

    ajListPeekLast(feattab->Features, (void**)&seqregionf);

    /*
     * in SQL query below, feature refers to the sequence region being queried
     * sfeature refers to the features located within the sequence region,
     *
     * it is assumed that results are ordered such that features with the same
     * uniquename follows each other, we should check whether we need to use
     * explicit ORDER BY for any reason...
     */

    ajFmtPrintS(&sql,
	    "SELECT sfeature.name, sfeature.uniquename, relation.name,"
	    " object.uniquename, object_type.name, featureloc.fmin,"
	    " featureloc.fmax, featureloc.strand, featureloc.phase,\n"
	    " cvterm.name, db.name, analysis.program, analysisfeature.rawscore,"
	    " sfeature.timelastmodified, sfeature.is_obsolete\n"
	    " FROM feature\n"
	    " JOIN featureloc"
	    " ON feature.feature_id = featureloc.srcfeature_id\n"
	    " JOIN feature sfeature"
	    " ON sfeature.feature_id = featureloc.feature_id\n"
	    " LEFT OUTER JOIN feature_relationship"
	    " ON sfeature.feature_id = feature_relationship.subject_id\n"
	    " LEFT OUTER JOIN cvterm relation"
	    " ON relation.cvterm_id = feature_relationship.type_id\n"
	    " LEFT OUTER JOIN feature object"
	    " ON object.feature_id = feature_relationship.object_id\n"
	    " LEFT OUTER JOIN cvterm object_type"
	    " ON object.type_id = object_type.cvterm_id\n"
	    " JOIN cvterm ON cvterm.cvterm_id = sfeature.type_id\n"
	    " LEFT OUTER JOIN dbxref ON dbxref.dbxref_id = sfeature.dbxref_id\n"
	    " LEFT OUTER JOIN db USING (db_id)\n"
	    " LEFT OUTER JOIN analysisfeature"
	    " ON sfeature.feature_id=analysisfeature.feature_id\n"
	    " LEFT OUTER JOIN analysis USING(analysis_id)\n"
	    " WHERE \n"
	    " feature.uniquename='%S'"
	    " AND"
	    " ((featureloc.fmin>%d AND featureloc.fmin<%d)"
	    " OR"
	    " (featureloc.fmax>=%d AND featureloc.fmax<=%d))\n",
	    srcfeature,
	    seqregionf->Start, seqregionf->End,
	    seqregionf->Start, seqregionf->End);

    ajDebug("SQL features:\n%S\n",sql);

    statement = ajSqlstatementNewRun(connection,sql);

    if(!statement)
    {
	ajErr("Could not execute SQL statement [%S]", sql);
	ajExitAbort();
    }

    iter = ajSqlrowiterNew(statement);

    while(!ajSqlrowiterDone(iter))
    {
	row = ajSqlrowiterGet(iter);

	if(debugOn)
	{
	    while(row->Current < row->Columns)
	    {
		ajSqlcolumnToStr(row,&colstr);
		ajFmtPrintAppS(&rowstr, "\t%S",colstr);
	    }

	    ajDebug("row %d: %S\n", ++i, rowstr);
	    ajStrSetClear(&rowstr);
	}

	ajSqlcolumnRewind(row);

	featChadoChildfeatureRow(feattab, row);
    }

    ajSqlrowiterDel(&iter);
    ajSqlstatementDel(&statement);
    ajStrDel(&colstr);
    ajStrDel(&rowstr);
    ajStrDel(&sql);

    return;
}




/* @funcstatic featChadoChildfeatureRow ***************************************
**
** Generates an emboss feature obj from a row of chado query results, and
** appends it to the feature table fttab.
**
** @param [u] fttab [AjPFeattable] Feature table
** @param [u] row [AjPSqlrow] Input row
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featChadoChildfeatureRow(AjPFeattable fttab, AjPSqlrow row)
{
    AjPFeature feature = NULL;
    AjPFeature prevft  = NULL;

    AjPStr name = NULL;
    AjPStr type = NULL;
    AjPStr source = NULL;
    AjPStr sourcedb = NULL;
    AjPStr sourceprogram = NULL;
    AjPTagval idtag = NULL;

    ajint start  = 0;
    ajint end    = 0;
    float score  = 0;
    char  strand = '+';
    ajint i = 0;
    ajint frame = 0;

    AjPStr entryid = NULL;
    AjBool updateprevft = ajFalse;
    AjPStr alias = NULL;
    AjPStr relation = NULL;

    /* The object can also be thought of as parent (containing feature),
     * and subject as child (contained feature or subfeature). ref:gmod.org */

    AjPStr object = NULL;
    AjPStr objecttype = NULL;
    AjPTime timelm = NULL;
    AjPStr timelmS = NULL;
    AjBool isObsolete = ajFalse;
    AjPStr isObsoleteS = NULL;

    if(!ajSqlrowGetColumns(row))
	return NULL;

    timelm = ajTimeNew();

    ajSqlcolumnToStr(row, &name);
    ajSqlcolumnToStr(row, &entryid);
    ajSqlcolumnToStr(row, &relation);
    ajSqlcolumnToStr(row, &object);
    ajSqlcolumnToStr(row, &objecttype);
    ajSqlcolumnToInt(row, &start);
    ajSqlcolumnToInt(row, &end);

    ajSqlcolumnToInt(row, &i);
    if( i == 1 )
	strand = '+';
    else if( i == -1 )
	strand = '-';
    else
	strand = '\0';		/* change to \0 later */

    ajSqlcolumnToInt(row, &frame);

    ajSqlcolumnToStr(row, &type);
    ajSqlcolumnToStr(row, &sourcedb);
    ajSqlcolumnToStr(row, &sourceprogram);
    ajSqlcolumnToFloat(row, &score);
    /*ajSqlcolumnToStr(row, &alias);*/
    ajSqlcolumnToTime(row, &timelm);
    ajSqlcolumnToBool(row, &isObsolete);

    if(ajStrGetLen(sourcedb))
	ajStrAssignS(&source, sourcedb);
    else if(ajStrGetLen(sourceprogram))
	ajStrAssignS(&source, sourceprogram);
    else
	source = ajStrNewC(".");

    ajListPeekLast(fttab->Features, (void**)&prevft);

    if(++start==(ajint)prevft->Start && end==(ajint)prevft->End)
    {
	ajListPeekFirst(prevft->GffTags, (void**)&idtag);
	if(ajStrMatchS(entryid, idtag->Value))
	{
	    updateprevft = ajTrue;
	    feature = prevft;
	}
    }

    if(!updateprevft)
    {
	feature = ajFeatNewNuc(fttab,
	                       source,
	                       type,
	                       start,
	                       end,
	                       score,
	                       strand,
	                       frame,
	                       0,0,0, NULL, NULL);

	ajFeatGfftagAddC(feature, "ID", entryid);
	ajFeatGfftagAddC(feature, "Name", name);
	/*ajFeatGfftagAddC(feature, "Alias", alias);*/

	/* FIXME: it looks we replace all type 'synonyms' with SO:0000110
	 * following tag is a workaround until it is fixed */
	if(ajStrMatchC(feature->Type, "SO:0000110"))
	    ajFeatGfftagAddC(feature, "type", type);
    }

    if(ajStrMatchC(relation, "part_of") ||
	    ajStrMatchC(relation, "proper_part_of") ||
	    ajStrMatchC(relation, "partof") ||
	    ajStrMatchC(relation, "producedby"))
    {
	ajFeatGfftagAddC(feature, "Parent", object);
	ajFeatGfftagAddC(feature, "parent_type", objecttype);
    }
    else if(ajStrMatchC(relation, "derives_from"))
    {
	ajFeatGfftagAddC(feature, "Derives_from", object);
	ajFeatGfftagAddC(feature, "parent_type", objecttype);
    }
    else if(ajStrGetLen(relation))
    {
	ajFeatGfftagAdd(feature, relation, object);
	ajFeatGfftagAddC(feature, "object_type", objecttype);
    }


    if(!updateprevft)
    {
	if(timelm)
	{
	    ajFmtPrintS(&timelmS,"%D",timelm);
	    ajFeatGfftagAddC(feature,"timelastmodified", timelmS);
	}

	if(isObsolete)
	{
	    isObsoleteS = ajStrNewC("true");
	    ajFeatGfftagAddC(feature,"isObsolete", isObsoleteS);
	    ajStrDel(&isObsoleteS);
	}
    }

    ajFeatTrace(feature);

    ajStrDel(&name);
    ajStrDel(&entryid);
    ajStrDel(&relation);
    ajStrDel(&object);
    ajStrDel(&objecttype);
    ajStrDel(&type);
    ajStrDel(&source);
    ajStrDel(&sourcedb);
    ajStrDel(&sourceprogram);
    ajStrDel(&alias);
    ajTimeDel(&timelm);
    ajStrDel(&timelmS);

    return feature;
}




/* @funcstatic featChadoQryfeatureRow *****************************************
**
** Generates a new feature from chado query-feature query result row.
** Checks whether the query feature is located on another feature (seq region)
**
** @param [u] fttab    [AjPFeattable] Feature table
** @param [u] row      [AjPSqlrow]    Input row
** @param [r] qrystart [ajint]        Query start position
** @param [r] qryend   [ajint]        Query end position
** @return [AjPStr] New feature ID
** @@
******************************************************************************/

static AjPStr featChadoQryfeatureRow(AjPFeattable fttab, AjPSqlrow row,
	                             ajint qrystart, ajint qryend)
{
    AjPFeature gf  = NULL;
    AjPStr name   = NULL;

    AjPStr source = NULL;
    AjPStr type   = NULL;
    ajint start  = 1;
    ajint seqlen = 0;
    float score  = 0;
    char  strand = '+';
    ajint loccount  = 0; /* this is to be removed; initially thought
                            we can use location count to decide whether
                            the query feature is a sequence region,
                            however current code decides it by checking whether
                            a parent location is defined */
    ajint i = 0;
    ajint frame = 0;
    AjPStr uniquename = NULL;
    AjPStr alias = NULL;
    AjPStr regionid = NULL;
    AjPStr regionuniquename = NULL;
    ajint regionstart  = 1;
    ajint regionend    = 0;
    AjPTime timelm = NULL;
    AjPStr timelmS = NULL;
    AjBool isObsolete = ajFalse;
    AjPStr isObsoleteS = NULL;

    if(!ajSqlrowGetColumns(row))
	return NULL;

    timelm = ajTimeNew();

    ajSqlcolumnToStr(row, &name);
    ajSqlcolumnToStr(row, &uniquename);
    ajSqlcolumnToInt(row, &loccount);
    ajSqlcolumnToStr(row, &regionid);
    ajSqlcolumnToStr(row, &regionuniquename);
    ajSqlcolumnToInt(row, &regionstart);
    ajSqlcolumnToInt(row, &regionend);
    ajDebug("location count:%d  parent:%S parent-id:%S"
	    " region start:%d  region end:%d\n",
	    loccount,
	    regionuniquename, regionid, regionstart, regionend);

    ajSqlcolumnToInt(row, &i);
    if( i == 1 )
	strand = '+';
    else if( i == -1 )
	strand = '-';
    else
	strand = '\0';

    ajSqlcolumnToInt(row, &frame);

    ajSqlcolumnToInt(row, &seqlen);

    ajSqlcolumnToStr(row, &type);
    ajSqlcolumnToStr(row, &source);
    /*ajSqlcolumnToStr(row, &alias);*/
    ajSqlcolumnToTime(row, &timelm);
    ajSqlcolumnToBool(row, &isObsolete);

    if(!ajStrGetLen(source))
	ajStrAssignS(&source, fttab->Db);


    if(qryend)
    {
	start = qrystart;
	seqlen = qryend;
    }

    regionstart++;

    gf = ajFeatNewNuc(fttab, source, type,
                      (ajStrGetLen(regionuniquename) ? regionstart : start),
                      (ajStrGetLen(regionuniquename) ? regionend   : seqlen),
                      score,
                      strand,
                      frame,
                      0,0,0, NULL, NULL);

    ajFeatGfftagAddC(gf, "ID", uniquename);
    ajFeatGfftagAddC(gf, "Name", name);
    /*ajFeatGfftagAddC(gf, "Alias", alias);*/

    if(timelm)
    {
	ajFmtPrintS(&timelmS,"%D",timelm);
	ajFeatGfftagAddC(gf,"timelastmodified", timelmS);
    }

    if(isObsolete)
    {
	isObsoleteS = ajStrNewC("true");
	ajFeatGfftagAddC(gf,"isObsolete", isObsoleteS);
	ajStrDel(&isObsoleteS);
    }

    ajFeatTrace(gf);

    ajStrDel(&name);
    ajStrDel(&type);
    ajStrDel(&source);
    ajStrDel(&alias);
    ajTimeDel(&timelm);
    ajStrDel(&timelmS);
    ajStrDel(&regionid);

    if(ajStrGetLen(regionuniquename))
    {
	if(loccount)
	    ajDebug("Feature '%S' is used as a source feature for %d other"
		    " feature(s) but it is itself located on feature '%S';"
		    " making queries on the parent feature '%S'\n",
		    uniquename, loccount, regionuniquename, regionuniquename);
	ajStrAssignS(&fttab->Seqid, regionuniquename);

	ajStrDel(&uniquename);
	return regionuniquename;
    }

    ajStrDel(&regionuniquename);

    return uniquename;
}




/* @funcstatic featChadoConnect ***********************************************
**
** Connects to the chado database required by the query
**
** @param [r] qry [const AjPQuery] Query object
** @return [AjPSqlconnection] SQL Database connection
** @@
******************************************************************************/

static AjPSqlconnection featChadoConnect(const AjPQuery qry)
{
    AjESqlconnectionClient client;

    ajint iport = 3306;

    AjPStr url = NULL;
    AjPUrlref uo  = NULL;

    AjPStr password   = NULL;
    AjPStr socketfile = NULL;

    AjPSqlconnection connection = NULL;

    url = ajStrNew();

    if(!ajNamDbGetUrl(qry->DbName, &url))
    {
	ajErr("no URL defined for database %S", qry->DbName);

	return ajFalse;
    }

    uo = ajHttpUrlrefNew();

    ajHttpUrlrefParseC(&uo, ajStrGetPtr(url));
    ajHttpUrlrefSplitPort(uo);
    ajHttpUrlrefSplitUsername(uo);

    if(ajStrMatchCaseC(uo->Method,"mysql"))
        client = ajESqlconnectionClientMySQL;
    else if(ajStrMatchCaseC(uo->Method,"postgresql"))
    {
        client = ajESqlconnectionClientPostgreSQL;
        iport = 5432;
    }
    else
        client = ajESqlconnectionClientNULL;

    if(!ajStrGetLen(uo->Port))
        ajFmtPrintS(&uo->Port,"%d",iport);

    if(ajStrGetLen(uo->Password))
    {
        password = ajStrNew();
        ajStrAssignS(&password,uo->Password);
    }

    connection = ajSqlconnectionNewData(client,uo->Username,password,
                                        uo->Host,uo->Port,socketfile,
                                        uo->Absolute);

    ajStrDel(&password);

    if(!connection)
        ajErr("Could not connect to database server");

    ajStrDel(&url);
    ajHttpUrlrefDel(&uo);

    return connection;
}




/* @funcstatic featAccessDas **************************************************
**
** Feature access method for DAS feature sources
**
** @param [u] fttabin [AjPFeattabin] Feature input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featAccessDas(AjPFeattabin fttabin)
{
    AjPStr host         = NULL;
    AjPStr path         = NULL;
    AjIList iter        = NULL;
    AjPQueryField field = NULL;
    AjPQuery qry        = NULL;
    AjPTextin textin    = NULL;
    AjPStr dasqueryurl  = NULL;

    AjBool ret = ajTrue;
    ajint port = 80;

    textin = fttabin->Input;

    qry = textin->Query;

    if(qry->QryDone)
	return ajFalse;

    if(!ajHttpQueryUrl(qry, &port, &host, &path))
    {
	ajStrDel(&host);
	ajStrDel(&path);

	return ajFalse;
    }

    if(ajStrGetCharLast(path)!='/')
	ajStrAppendK(&path,'/');

    dasqueryurl = ajStrNew();

    iter = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
	field = ajListIterGet(iter);

	if(ajStrMatchCaseC(field->Field, "id"))
	{
	    if(!ajStrGetLen(dasqueryurl))
		ajFmtPrintS(&dasqueryurl,"segment=%S",
		            field->Wildquery);
	    else
		ajFmtPrintS(&dasqueryurl,"%S;segment=%S",
		            dasqueryurl,
		            field->Wildquery);

	    /* TODO: segment specific start,end positions */
	    if(fttabin->End > 0)
		ajFmtPrintS(&dasqueryurl,"%S:%u,%u",
		            dasqueryurl,fttabin->Start,fttabin->End);
	}
	else {
	    if(!ajStrGetLen(dasqueryurl))
		ajFmtPrintS(&dasqueryurl,"%S=%S",
		            field->Field,
		            field->Wildquery);
	    else
		ajFmtPrintS(&dasqueryurl,"%S;%S=%S",
		            dasqueryurl,
		            field->Field,
		            field->Wildquery);

	    /* TODO: segment specific start,end positions */
	    if(fttabin->End > 0)
		ajFmtPrintS(&dasqueryurl,"%S:%u,%u",
		            dasqueryurl,fttabin->Start,fttabin->End);
	}

    }

    ajFmtPrintS(&path,"%Sfeatures?%S",path, dasqueryurl);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName, qry->DbProxy,
                                  host, port, path);

    if (textin->Filebuff)
	ajFilebuffHtmlNoheader(textin->Filebuff);
    else
	ret = ajFalse;

    qry->QryDone = ajTrue;

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&dasqueryurl);

    ajListIterDel(&iter);

    return ret;
}




/* @func ajFeatdbInit *********************************************************
**
** Initialise feature database internals
**
** @return [void]
******************************************************************************/

void ajFeatdbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajFeattabaccessGetDb();

    while(feattabAccess[i].Name)
    {
        ajCallTableRegister(table, feattabAccess[i].Name,
                            (void*) &feattabAccess[i]);
	i++;
    }

    return;
}





/* @func ajFeatdbPrintAccess **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajFeatdbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Feature access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; feattabAccess[i].Name; i++)
	if(full || !feattabAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			feattabAccess[i].Name,  feattabAccess[i].Alias,
			feattabAccess[i].Entry, feattabAccess[i].Query,
			feattabAccess[i].All,   feattabAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajFeatdbExit *********************************************************
**
** Cleans up feature database processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajFeatdbExit(void)
{
    return;
}
