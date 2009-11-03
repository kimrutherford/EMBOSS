/* @source dbxfasta application
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


#define FASTATYPE_SIMPLE    1
#define FASTATYPE_IDACC     2
#define FASTATYPE_GCGID     3
#define FASTATYPE_GCGIDACC 4
#define FASTATYPE_NCBI      5
#define FASTATYPE_DBID      6
#define FASTATYPE_ACCID     7
#define FASTATYPE_GCGACCID  8

static AjPRegexp dbxfasta_wrdexp = NULL;


static AjBool dbxfasta_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
				 AjPRegexp typeexp, ajint idtype);
static AjBool dbxfasta_ParseFasta(EmbPBtreeEntry entry, AjPRegexp typeexp,
				  ajint idtype, const AjPStr line);
static AjPRegexp dbxfasta_getExpr(const AjPStr idformat, ajint *type);


/* @prog dbxfasta ************************************************************
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
    

    AjPRegexp typeexp = NULL;
    ajint idtype = 0;



    embInit("dbxfasta", argc, argv);

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


    typeexp = dbxfasta_getExpr(dbtype,&idtype);


    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	printf("Processing file %s\n",MAJSTRGETPTR(tmpstr));
	if(!(inf=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	

	while(dbxfasta_NextEntry(entry,inf,typeexp,idtype))
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

	    if(entry->do_sv)
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

	    if(entry->do_description)
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

    ajRegFree(&dbxfasta_wrdexp);
    ajRegFree(&typeexp);

    embExit();

    return 0;
}




/* @funcstatic dbxfasta_NextEntry ********************************************
**
** Parse the next entry from a fasta file
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
** @param [u] typeexp [AjPRegexp] regexp corresponding to idtype
** @param [r] idtype [ajint] the kind of parsing required
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxfasta_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
				 AjPRegexp typeexp, ajint idtype)
{
    static AjBool init = AJFALSE;
    AjPStr line = NULL;
    
    if(!init)
    {
        line = ajStrNew();
        init = ajTrue;
    }

    ajStrAssignC(&line,"");

    while(*MAJSTRGETPTR(line) != '>')
    {
	entry->fpos = ajFileTell(inf);
	if(!ajFileReadLine(inf,&line))
	{
	  ajStrDel(&line);
	  return ajFalse;
	}
    }


    dbxfasta_ParseFasta(entry, typeexp, idtype, line);

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic dbxfasta_getExpr ***********************************************
**
** Compile regular expression
**
** @param [r] idformat [const AjPStr] type of ID line
** @param [w] type [ajint *] numeric type
** @return [AjPRegexp] ajTrue on success.
** @@
******************************************************************************/

static AjPRegexp dbxfasta_getExpr(const AjPStr idformat, ajint *type)
{
    AjPRegexp exp = NULL;

    if(ajStrMatchC(idformat,"simple"))
    {
	*type = FASTATYPE_SIMPLE;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"idacc"))
    {
	*type = FASTATYPE_IDACC;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)+[ \t]+\\(?([A-Za-z0-9_-]+)\\)?");
    }
    else if(ajStrMatchC(idformat,"accid"))
    {
	*type = FASTATYPE_ACCID;
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgid"))
    {
	*type = FASTATYPE_GCGID;
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgidacc"))
    {
	*type = FASTATYPE_GCGIDACC;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgaccid"))
    {
	*type = FASTATYPE_GCGACCID;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"ncbi"))
    {
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)"); /* dummy regexp */
	*type = FASTATYPE_NCBI;
    }
    else if(ajStrMatchC(idformat,"dbid"))
    {
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+[ \t]+([A-Za-z0-9_-]+)");
	*type = FASTATYPE_DBID;
    }
    else
	return NULL;

    return exp;
}




/* @funcstatic dbxfasta_ParseFasta ********************************************
**
** Parse the ID, accession from a FASTA format sequence
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] typeexp [AjPRegexp] regular expression
** @param [r] idtype [ajint] type of id line
** @param [r] line [const AjPStr] fasta '>' line
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxfasta_ParseFasta(EmbPBtreeEntry entry, AjPRegexp typeexp,
				  ajint idtype, const AjPStr line)
{
    AjPStr ac  = NULL;
    AjPStr sv  = NULL;
    AjPStr gi  = NULL;
    AjPStr db  = NULL;
    AjPStr de  = NULL;

    AjPStr tmpfd  = NULL;

    AjPStr str = NULL;
    


    if(!dbxfasta_wrdexp)
	dbxfasta_wrdexp = ajRegCompC("([A-Za-z0-9]+)");


    if(!ajRegExec(typeexp,line))
    {
	ajStrDel(&ac);
	ajDebug("Invalid ID line [%S]",line);
	return ajFalse;
    }

    /*
    ** each case needs to set id, ac, sv, de
    ** using empty values if they are not found
    */
    
    ajStrAssignC(&sv, "");
    ajStrAssignC(&gi, "");
    ajStrAssignC(&db, "");
    ajStrAssignC(&de, "");
    ajStrAssignC(&ac, "");
    ajStrAssignC(&entry->id, "");

    switch(idtype)
    {
    case FASTATYPE_SIMPLE:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssignS(&ac,entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_DBID:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssignS(&ac,entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_GCGID:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssignS(&ac,entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_NCBI:
	if(!ajSeqParseNcbi(line,&entry->id,&ac,&sv,&gi,&db,
			   &de))
	    return ajFalse;
	break;
    case FASTATYPE_GCGIDACC:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&ac);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_GCGACCID:
	ajRegSubI(typeexp,1,&ac);
	ajRegSubI(typeexp,2,&entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_IDACC:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&ac);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_ACCID:
	ajRegSubI(typeexp,1,&ac);
	ajRegSubI(typeexp,2,&entry->id);
	ajRegPost(typeexp, &de);
	break;
    default:
	return ajFalse;
    }

    ajStrFmtLower(&entry->id);

    if(entry->do_accession && ajStrGetLen(ac))
    {
	str = ajStrNew();
	ajStrAssignS(&str,ac);
	ajListPush(entry->ac,(void *)str);
    }

    if(ajStrGetLen(gi))
	ajStrAssignS(&sv,gi);

    if(entry->do_sv && ajStrGetLen(sv))
    {
	str = ajStrNew();
	ajStrAssignS(&str,sv);
	ajListPush(entry->ac,(void *)str);
    }
    
    if(entry->do_description && ajStrGetLen(de))
	while(ajRegExec(dbxfasta_wrdexp,de))
	{
	    ajRegSubI(dbxfasta_wrdexp, 1, &tmpfd);
	    str = ajStrNew();
	    ajStrAssignS(&str,tmpfd);
	    ajListPush(entry->de,(void *)str);
	    ajRegPost(dbxfasta_wrdexp, &de);
	}


    ajStrDel(&de);
    ajStrDel(&ac);
    ajStrDel(&sv);
    ajStrDel(&gi);
    ajStrDel(&db);
    ajStrDel(&tmpfd);

    return ajTrue;
}
