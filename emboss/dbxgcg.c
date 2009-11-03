/* @source dbxgcg application
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




#define GCGTYPE_OTHER 0
#define GCGTYPE_ID 1
#define GCGTYPE_ACC 2
#define GCGTYPE_DES 3
#define GCGTYPE_KEY 4
#define GCGTYPE_TAX 5
#define GCGTYPE_VER 6


static AjPRegexp dbxgcg_embl_typexp = NULL;
static AjPRegexp dbxgcg_embl_idexp  = NULL;
static AjPRegexp dbxgcg_embl_verexp = NULL;
static AjPRegexp dbxgcg_embl_wrdexp = NULL;
static AjPRegexp dbxgcg_embl_phrexp = NULL;
static AjPRegexp dbxgcg_embl_taxexp = NULL;

static AjPRegexp dbxgcg_gcg_rexp = NULL;
static AjPRegexp dbxgcg_gcg_sexp = NULL;
static AjPRegexp dbxgcg_splitexp = NULL;

static AjPRegexp dbxgcg_pir_idexp  = NULL;
static AjPRegexp dbxgcg_pir_acexp  = NULL;
static AjPRegexp dbxgcg_pir_ac2exp = NULL;
static AjPRegexp dbxgcg_pir_keyexp = NULL;
static AjPRegexp dbxgcg_pir_taxexp = NULL;
static AjPRegexp dbxgcg_pir_tax2exp = NULL;
static AjPRegexp dbxgcg_pir_wrdexp = NULL;
static AjPRegexp dbxgcg_pir_phrexp = NULL;
static AjPRegexp dbxgcg_pir_pirexp = NULL;

static AjBool dbxgcg_ParseEmbl(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *reflibstr);
static AjBool dbxgcg_ParseGenbank(EmbPBtreeEntry entry, AjPFile infr,
				  AjPStr *reflibstr);
static AjBool dbxgcg_ParsePir(EmbPBtreeEntry entry, AjPFile infr,
			      AjPStr *reflibstr);

static AjBool dbxgcg_NextEntry(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);

static ajlong dbxgcg_gcggetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);
static ajlong dbxgcg_pirgetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);

static ajlong dbxgcg_gcgappent(AjPFile infr, AjPFile infs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr);





/* @datastatic DbxgcgPParser *************************************************
**
** Parser definition structure
**
** @alias DbxgcgSParser
** @alias DbxgcgOParser
**
** @attr Name [const char*] Parser name
** @attr GcgType [AjBool] Gcg type parser if true, PIR type if false
** @attr Padding [char[4]] Padding to alignment boundary
** @attr Parser [(AjBool*)] Parser function
** @@
******************************************************************************/

typedef struct DbxgcgSParser
{
    const char* Name;
    AjBool GcgType;
    char   Padding[4];
    AjBool (*Parser) (EmbPBtreeEntry entry, AjPFile infr, AjPStr *reflibstr);
} DbxgcgOParser;
#define DbxgcgPParser DbxgcgOParser*




static DbxgcgOParser parser[] =
{
    {"EMBL", AJTRUE, "", dbxgcg_ParseEmbl},
    {"SWISS", AJTRUE, "", dbxgcg_ParseEmbl},
    {"GENBANK", AJTRUE, "", dbxgcg_ParseGenbank},
    {"PIR", AJFALSE, "", dbxgcg_ParsePir},
    {NULL, 0, "", NULL}
};





/* @prog dbxgcg **************************************************************
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
    AjPStr refname = NULL;
    AjPStr seqname = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile infs = NULL;
    AjPFile infr = NULL;

    AjPStr word = NULL;
    
    AjPBtId  idobj  = NULL;
    AjPBtPri priobj = NULL;
    AjPBtHybrid hyb = NULL;
    

    embInit("dbxgcg", argc, argv);

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
    for(i=0; i<nfiles; ++i)
    {
	ajListPop(entry->files,(void **) &seqname);
	refname = ajStrNew();
	ajStrAssignS(&refname,seqname);
	ajFileNameExtC(&seqname,"seq");
	ajFileNameExtC(&refname,"ref");
	ajListPushAppend(entry->files,(void *)seqname);
	ajListPushAppend(entry->reffiles,(void *)refname);
    }
    

    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);



    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	printf("Processing file %s\n",MAJSTRGETPTR(tmpstr));
	if(!(infs=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);

	ajListPop(entry->reffiles,(void **)&thysfile);
	ajListPushAppend(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(infr=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	

	while(dbxgcg_NextEntry(entry,infs,infr,dbtype))
	{
	    if(entry->do_id)
	    {
		ajStrFmtLower(&entry->id);
		ajStrAssignS(&hyb->key1,entry->id);
		hyb->dbno = i;
		hyb->offset = entry->fpos;
		hyb->refoffset = entry->reffpos;
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
		    hyb->refoffset = entry->reffpos;
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
		    hyb->refoffset = entry->reffpos;
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
	
	ajFileClose(&infs);
	ajFileClose(&infr);
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
    
    ajRegFree(&dbxgcg_embl_typexp);
    ajRegFree(&dbxgcg_embl_idexp);
    ajRegFree(&dbxgcg_embl_verexp);
    ajRegFree(&dbxgcg_embl_wrdexp);
    ajRegFree(&dbxgcg_embl_phrexp);
    ajRegFree(&dbxgcg_embl_taxexp);

    ajRegFree(&dbxgcg_gcg_rexp);
    ajRegFree(&dbxgcg_gcg_sexp);

    ajRegFree(&dbxgcg_splitexp);

    ajRegFree(&dbxgcg_pir_idexp);
    ajRegFree(&dbxgcg_pir_acexp);
    ajRegFree(&dbxgcg_pir_ac2exp);
    ajRegFree(&dbxgcg_pir_keyexp);
    ajRegFree(&dbxgcg_pir_taxexp);
    ajRegFree(&dbxgcg_pir_tax2exp);
    ajRegFree(&dbxgcg_pir_wrdexp);
    ajRegFree(&dbxgcg_pir_phrexp);
    ajRegFree(&dbxgcg_pir_pirexp);

    embExit();

    return 0;
}




/* @funcstatic dbxgcg_NextEntry ***********************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [AjBool] ajTrue if successful read
** @@
******************************************************************************/

static AjBool dbxgcg_NextEntry(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    AjPStr tmpstr = NULL;
    char *p;

    if(!dbxgcg_splitexp)
	dbxgcg_splitexp = ajRegCompC("_0+$");

    entry->reffpos = ajFileTell(infr);
    entry->fpos    = ajFileTell(infs);

    if(!dbxgcg_gcggetent(entry, infs, infr, dbtype) &&
       !dbxgcg_pirgetent(entry, infs, infr, dbtype))
	return ajFalse;

    ajDebug("id '%S' seqfpos:%d reffpos:%d\n",
	    entry->id, entry->fpos, entry->reffpos);

    ajStrAssignC(&tmpstr,ajStrGetPtr(entry->id));

    if(ajRegExec(dbxgcg_splitexp, entry->id))
    {
	p  = strrchr(ajStrGetPtr(tmpstr),'_');
	*p = '\0';
	ajStrAssignC(&entry->id,ajStrGetPtr(tmpstr));
    }

    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @funcstatic dbxgcg_gcggetent ***********************************************
**
** get a single entry from the GCG database files
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [ajlong] Sequence length
** @@
******************************************************************************/

static ajlong dbxgcg_gcggetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    static ajint called   = 0;
    static ajint iparser  = -1;
    AjPStr gcgtype   = NULL;
    AjPStr gcgdate   = NULL;
    ajlong gcglen = 0;
    ajlong rblock;
    AjPStr reflibstr = NULL;
    ajint i;
    AjPStr tmpstr  = NULL;
    AjPStr rline = NULL;
    AjPStr sline = NULL;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(dbtype, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("dbtype '%S' unknown", dbtype);

	ajDebug("dbtype '%S' Parser %d\n", dbtype, iparser);
	called = 1;
    }

    if(!parser[iparser].GcgType)
    {
      ajStrDel(&gcgtype);
      ajStrDel(&gcgdate);
      ajStrDel(&reflibstr);
      ajStrDel(&tmpstr);
      ajStrDel(&rline);
      ajStrDel(&sline);
      return 0;
    }

    if(!dbxgcg_gcg_rexp)
	dbxgcg_gcg_rexp = ajRegCompC("^>>>>([^ \t\n]+)");

    if(!dbxgcg_gcg_sexp)
	dbxgcg_gcg_sexp = ajRegCompC("^>>>>([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)"
			  "[ \t]+([^ \t]+)[ \t]+([0-9]+)");

    ajStrAssignC(&sline, "");

    /* check for seqid first line */
    while(ajStrGetCharFirst(sline)!='>')
    {
	if(!ajFileGets(infs, &sline))
	  {
	    ajStrDel(&gcgtype);
	    ajStrDel(&gcgdate);
	    ajStrDel(&reflibstr);
	    ajStrDel(&tmpstr);
	    ajStrDel(&rline);
	    ajStrDel(&sline);
	    return 0;			/* end of file */
	  }
	ajDebug("... read until next seq %Ld '%S'\n",
		ajFileTell(infs), sline);
    }

    ajDebug("dbxgcg_gcggetent .seq (%S) %Ld '%S'\n",
	    dbtype, ajFileTell(infs), sline);

    /* get the encoding/sequence length info */
    if(!ajRegExec(dbxgcg_gcg_sexp, sline))
    {
        ajDebug("dbxgcg_gcggetent sequence expression FAILED\n");
	return 0;
    }

    ajRegSubI(dbxgcg_gcg_sexp, 1, &entry->id);		/* Entry ID returned */

    ajRegSubI(dbxgcg_gcg_sexp, 2, &gcgdate);
    ajRegSubI(dbxgcg_gcg_sexp, 3, &gcgtype);
    ajRegSubI(dbxgcg_gcg_sexp, 5, &tmpstr);
    ajStrToLong(tmpstr, &gcglen);

    ajDebug("new entry '%S' date:'%S' type:'%S' len:'%S'=%Ld\n",
	    entry->id, gcgdate, gcgtype, tmpstr, gcglen);

    ajStrAssignC(&rline, "");

    ajDebug("dbxgcg_gcggetent .ref (%S) %Ld '%S'\n",
	    dbtype, ajFileTell(infr), rline);

    /* check for refid first line */
    while(ajStrGetCharFirst(rline)!='>')
    {
	if(!ajFileGets(infr, &rline))
	{
	    ajErr("ref ended before seq");
	    break;			/* end of file */
	}
	ajDebug("... read until next ref %Ld '%S'\n", ajFileTell(infr), rline);
    }

    /* get the encoding/sequence length info */

    ajRegExec(dbxgcg_gcg_rexp, rline);
    ajRegSubI(dbxgcg_gcg_rexp, 1, &reflibstr);

    parser[iparser].Parser(entry, infr, &reflibstr);/* writes alistfile data */

    /* get the description line */
    ajFileGets(infs, &sline);

    /* seek to the end of the sequence; +1 to jump over newline */
    if(ajStrGetCharFirst(gcgtype)=='2')
    {
	rblock = (gcglen+3)/4;
	ajFileSeek(infs,rblock+1,SEEK_CUR);
    }
    else
	ajFileSeek(infs,gcglen+1,SEEK_CUR);

    ajStrDel(&gcgtype);
    ajStrDel(&gcgdate);
    ajStrDel(&reflibstr);
    ajStrDel(&tmpstr);
    ajStrDel(&rline);
    ajStrDel(&sline);

    /*
    **  for big entries, need to append until we have all the parts.
    **  They are named with _0 on the first part, _1 on the second and so on.
    **  or _00 on the first part, _01 on the second and so on.
    **  We can look for the "id_" prefix.
    */

    if(!ajStrSuffixC(entry->id, "_0") &&
       !ajStrSuffixC(entry->id,"_00") &&
       !ajStrSuffixC(entry->id,"_000") &&
       !ajStrSuffixC(entry->id,"_0000"))
	return gcglen;

    gcglen += dbxgcg_gcgappent(infr, infs, dbxgcg_gcg_rexp, dbxgcg_gcg_sexp,
			       &entry->id);

    return gcglen;
}




/* @funcstatic dbxgcg_pirgetent ***********************************************
**
** get a single entry from the PIR database files
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [ajlong] Sequence length
** @@
******************************************************************************/

static ajlong dbxgcg_pirgetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    AjPStr reflibstr = NULL;
    ajint i;
    static ajint called  = 0;
    static ajint iparser = -1;
    ajlong gcglen;
    AjPStr rline = NULL;
    AjPStr sline = NULL;
    ajlong spos = 0;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(dbtype, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("dbtype '%S' unknown", dbtype);
	ajDebug("dbtype '%S' Parser %d\n", dbtype, iparser);
	called = 1;
    }

    if(parser[iparser].GcgType)
	return 0;

    if(!dbxgcg_pir_pirexp)
	dbxgcg_pir_pirexp = ajRegCompC("^>..;([^ \t\n]+)");

    /* skip to seqid first line */
    while(ajStrGetCharFirst(sline)!='>')
	if(!ajFileGets(infs, &sline))
	    return 0;			/* end of file */

    ajDebug("dbxgcg_pirgetent .seq (%S) %Ld '%S' \n",
	    dbtype, ajFileTell(infs), sline);

    ajRegExec(dbxgcg_pir_pirexp, sline);

    /* skip to refid first line */
    while(ajStrGetCharFirst(rline)!='>')
	if(!ajFileGets(infr, &rline))
	{
	    ajErr("ref ended before seq"); /* end of file */
	    break;
	}

    /* get the encoding/sequence length info */

    ajRegExec(dbxgcg_pir_pirexp, rline);
    ajRegSubI(dbxgcg_pir_pirexp, 1, &reflibstr);
    ajRegSubI(dbxgcg_pir_pirexp, 1, &entry->id);

    ajDebug("dbigcg_pirgetent seqid '%S' spos: %Ld\n",
	    entry->id, ajFileTell(infs));
    ajDebug("dbxgcg_pirgetent refid '%S' spos: %Ld\n",
	    entry->id, ajFileTell(infr));

    parser[iparser].Parser(entry, infr, &reflibstr);/* writes alistfile data */

    /* get the description line */
    ajFileGets(infs, &sline);
    gcglen = 0;

    /* seek to the end of the sequence; +1 to jump over newline */
    while(ajStrGetCharFirst(sline)!='>')
    {
	spos = ajFileTell(infs);
	if(!ajFileGets(infs, &sline))
	{
	    spos = 0;
	    break;
	}
	gcglen += ajStrGetLen(sline);
    }

    if(spos)
	ajFileSeek(infs, spos, 0);

    ajDebug("dbxgcg_pirgetent end spos %Ld line '%S'\n", spos, sline);

    ajStrDel(&rline);
    ajStrDel(&sline);
    ajStrDel(&reflibstr);

    return gcglen;
}




/* @funcstatic dbxgcg_gcgappent ***********************************************
**
** Go to end of a split GCG entry
**
** @param [u] infr [AjPFile] Reference file
** @param [u] infs [AjPFile] Sequence file
** @param [u] rexp [AjPRegexp] Regular expression to find ID in ref file
** @param [u] sexp [AjPRegexp] Regular expression to find ID in seq file
** @param [w] libstr [AjPStr*] ID
** @return [ajlong] Sequence length for this section
** @@
******************************************************************************/

static ajlong dbxgcg_gcgappent(AjPFile infr, AjPFile infs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr)
{
    AjPStr reflibstr = NULL;
    AjPStr seqlibstr = NULL;
    AjPStr testlibstr = NULL;
    AjPStr tmpstr = NULL;
    AjPStr rline  = NULL;
    AjPStr sline  = NULL;
    ajint ilen;

    AjBool isend;
    const char *p;
    char *q;
    ajlong rpos;
    ajlong spos;

    /*
    ** keep reading until the end of entry is reached
    ** and return the extra number of bases
    */

    if(!testlibstr)
	testlibstr = ajStrNew();

    ajStrAssignS(&tmpstr,*libstr);

    ajDebug("dbi_gcgappent '%S'\n", tmpstr);

    p = ajStrGetPtr(tmpstr);
    q = strrchr(p,'_');
    *q = '\0';


    ajFmtPrintS(&testlibstr, "%s_",p);
    ilen = ajStrGetLen(testlibstr);

    isend = ajFalse;

    while(!isend)
    {
        spos = ajFileTell(infs);
	ajFileGets(infs,&sline);
	while(strncmp(ajStrGetPtr(sline),">>>>",4))
	{
	    spos = ajFileTell(infs);
	    if(!ajFileGets(infs, &sline))
	    {
	      ajStrDel(&reflibstr);
	      ajStrDel(&seqlibstr);
	      ajStrDel(&testlibstr);
	      ajStrDel(&tmpstr);
	      ajStrDel(&rline);
	      ajStrDel(&sline);
	      ajDebug("end of file on seq\n");
	      return 1L;
	    }
	}

	ajRegExec(sexp, sline);
	ajRegSubI(sexp, 1, &seqlibstr);

	rpos = ajFileTell(infr);
	ajFileGets(infr, &rline);

	while(ajStrGetCharFirst(rline)!='>')
	{
	  rpos = ajFileTell(infr);
	  if(!ajFileGets(infr, &rline))
	  {
	    ajDebug("end of file on seq\n");
	    ajDebug("ref ended before seq\n");
	    ajErr("ref ended before seq\n");
	    break;
	  }
	}

	ajRegExec(rexp, rline);
	ajRegSubI(rexp, 1, &reflibstr);

	if(ajStrCmpLenS(reflibstr, testlibstr, ilen) ||
	   ajStrCmpLenS(seqlibstr, testlibstr, ilen))
	    isend = ajTrue;

	ajDebug("gcgappent %B test: '%S' seq: '%S' ref: '%S'\n",
		isend, testlibstr, seqlibstr, reflibstr);
    }

    ajDebug("gcgappent done at seq: '%S' ref: '%S'\n", seqlibstr, reflibstr);

    ajStrAssignC(libstr,p);

    ajFileSeek(infr, rpos, 0);
    ajFileSeek(infs, spos, 0);

    ajStrDel(&reflibstr);
    ajStrDel(&seqlibstr);
    ajStrDel(&testlibstr);
    ajStrDel(&tmpstr);
    ajStrDel(&rline);
    ajStrDel(&sline);

    return 1L;
}




/* @funcstatic dbxgcg_ParseEmbl ***********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxgcg_ParseEmbl(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *id)
{
    AjPStr tmpstr  = NULL;
    AjPStr tmpline = NULL;
    AjPStr tmpfd   = NULL;
    AjPStr typStr  = NULL;
    AjPStr tmpacnum = NULL;
    ajint lineType;
    ajlong rpos;
    AjPStr rline = NULL;

    AjPStr str = NULL;
    

    if(!dbxgcg_embl_typexp)
	dbxgcg_embl_typexp = ajRegCompC("^([A-Z][A-Z]) +");

    if(!dbxgcg_embl_wrdexp)
	dbxgcg_embl_wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbxgcg_embl_verexp)
	dbxgcg_embl_verexp = ajRegCompC("([A-Za-z0-9]+[.][0-9]+)");

    if(!dbxgcg_embl_phrexp)
	dbxgcg_embl_phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbxgcg_embl_taxexp)
	dbxgcg_embl_taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbxgcg_embl_idexp)
	dbxgcg_embl_idexp = ajRegCompC("^ID   ([^ \t;]+)");

    rpos = ajFileTell(infr);
    while(ajFileGets(infr, &rline))
    {
	if(ajStrGetCharFirst(rline) == '>')
	    break;
	
        rpos = ajFileTell(infr);
	ajStrAssignS(&tmpstr,rline);

	if(ajRegExec(dbxgcg_embl_typexp, tmpstr))
	{
	    ajRegSubI(dbxgcg_embl_typexp, 1, &typStr);
	    if(ajStrMatchC(typStr, "ID"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(typStr, "SV"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(typStr, "AC"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DE"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(typStr, "KW"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(typStr, "OS"))
		lineType = GCGTYPE_TAX;
	    else if(ajStrMatchC(typStr, "OC"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(dbxgcg_embl_typexp, &tmpline);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(dbxgcg_embl_idexp, rline);
	    ajRegSubI(dbxgcg_embl_idexp, 1, id);
	    ajDebug("++id '%S'\n", *id);
	    continue;
	}

	if(lineType == GCGTYPE_ACC && entry->do_accession)
	{
	    while(ajRegExec(dbxgcg_embl_wrdexp, tmpline))
	    {
		ajRegSubI(dbxgcg_embl_wrdexp, 1, &tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		if(!tmpacnum)
		    ajStrAssignS(&tmpacnum, tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->ac,(void *)str);

		ajRegPost(dbxgcg_embl_wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_DES && entry->do_description)
	{
	    while(ajRegExec(dbxgcg_embl_wrdexp, tmpline))
	    {
		ajRegSubI(dbxgcg_embl_wrdexp, 1, &tmpfd);
		ajDebug("++des '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->de,(void *)str);

		ajRegPost(dbxgcg_embl_wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_VER && entry->do_sv)
	{
	    while(ajRegExec(dbxgcg_embl_verexp, tmpline))
	    {
		ajRegSubI(dbxgcg_embl_verexp, 1, &tmpfd);
		ajDebug("++sv '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->sv,(void *)str);

		ajRegPost(dbxgcg_embl_verexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && entry->do_keyword)
	{
	    while(ajRegExec(dbxgcg_embl_phrexp, tmpline))
	    {
		ajRegSubI(dbxgcg_embl_phrexp, 1, &tmpfd);
		ajRegPost(dbxgcg_embl_phrexp, &tmpline);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajDebug("++key '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->kw,(void *)str);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && entry->do_taxonomy)
	{
	    while(ajRegExec(dbxgcg_embl_taxexp, tmpline))
	    {
		ajRegSubI(dbxgcg_embl_taxexp, 1, &tmpfd);
		ajRegPost(dbxgcg_embl_taxexp, &tmpline);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajDebug("++tax '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->tx,(void *)str);
	    }
	    continue;
	}
    }

    if(rpos)
        ajFileSeek(infr, rpos, 0);

    ajStrDel(&tmpacnum);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpline);
    ajStrDel(&tmpfd);
    ajStrDel(&typStr);
    ajStrDel(&rline);

    return ajFalse;
}




/* @funcstatic dbxgcg_ParseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxgcg_ParseGenbank(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *id)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp morexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp verexp = NULL;
    ajlong rpos = 0;
    static AjPStr tmpstr  = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr rline   = NULL;
    static AjPStr tmpfd   = NULL;
    static AjPStr typStr  = NULL;
    ajint lineType=GCGTYPE_OTHER;

    AjPStr str = NULL;
    
    if(!typexp)
	typexp = ajRegCompC("^(  )?([A-Z]+)");

    if(!morexp)
	morexp = ajRegCompC("^            ");

    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!phrexp)
	phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!taxexp)
	taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!verexp)
	verexp = ajRegCompC("([A-Za-z0-9]+)( +GI:([0-9]+))?");

    while(ajFileGets(infr, &rline))
    {
	if(ajStrGetCharFirst(rline) == '>')
	    break;
	
        rpos = ajFileTell(infr);
	ajStrAssignS(&tmpstr,rline);

	if(ajRegExec(typexp, tmpstr))
	{
	    ajRegSubI(typexp, 2, &typStr);
	    if(ajStrMatchC(typStr, "LOCUS"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(typStr, "VERSION"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(typStr, "ACCESSION"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DEFINITION"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(typStr, "KEYWORDS"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(typStr, "ORGANISM"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(typexp, &tmpline);
	    ajDebug("++type line %d\n", lineType);
	}
	else if(lineType != GCGTYPE_OTHER && ajRegExec(morexp, rline))
	{
	    ajRegPost(morexp, &tmpline);
	    ajDebug("++more line %d\n", lineType);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(wrdexp, tmpline);
	    ajRegSubI(wrdexp, 1, id);
	}
	else if(lineType == GCGTYPE_ACC && entry->do_accession)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->ac,(void *)str);

		ajRegPost(wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_DES && entry->do_description)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
	        ajRegSubI(wrdexp, 1, &tmpfd);
		ajDebug("++des '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->de,(void *)str);

		ajRegPost(wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && entry->do_keyword)
	{
	    while(ajRegExec(phrexp, tmpline))
	    {
	        ajRegSubI(phrexp, 1, &tmpfd);
		ajRegPost(phrexp, &tmpline);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajDebug("++key '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->kw,(void *)str);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && entry->do_taxonomy)
	{
	    while(ajRegExec(taxexp, tmpline))
	    {
	        ajRegSubI(taxexp, 1, &tmpfd);
		ajRegPost(taxexp, &tmpline);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajDebug("++tax '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->tx,(void *)str);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_VER && entry->do_sv)
	{
	    if(ajRegExec(verexp, tmpline))
	    {
		ajRegSubI(verexp, 1, &tmpfd);
		ajDebug("++ver '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->sv,(void *)str);

		ajRegSubI(verexp, 3, &tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajDebug("++ver gi: '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssignS(&str,tmpfd);
		ajListPush(entry->sv,(void *)str);
	    }
	    continue;
	}

    }

    if(rpos)
	ajFileSeek(infr, rpos, 0);

    return ajFalse;
}




/* @funcstatic dbxgcg_ParsePir ************************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbxgcg_ParsePir(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *id)
{
    ajlong rpos;
    AjPStr tmpstr  = NULL;
    AjPStr tmpline = NULL;
    AjPStr rline   = NULL;
    AjPStr tmpfd   = NULL;

    AjPStr str = NULL;

    if(!dbxgcg_pir_wrdexp)
	dbxgcg_pir_wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbxgcg_pir_idexp)
	dbxgcg_pir_idexp = ajRegCompC("^>..;([^;.\n\r]+)");

    if(!dbxgcg_pir_phrexp)				/* allow . for "sp." */
	dbxgcg_pir_phrexp = ajRegCompC(" *([^,;\n\r]+)");

    if(!dbxgcg_pir_tax2exp)				/* allow . for "sp." */
	dbxgcg_pir_tax2exp = ajRegCompC(" *([^,;\n\r()]+)");

    if(!dbxgcg_pir_acexp)
	dbxgcg_pir_acexp = ajRegCompC("^C;Accession:");

    if(!dbxgcg_pir_ac2exp)
	dbxgcg_pir_ac2exp = ajRegCompC("([A-Za-z0-9]+)");

    if(!dbxgcg_pir_taxexp)
	dbxgcg_pir_taxexp = ajRegCompC("^C;Species:");

    if(!dbxgcg_pir_keyexp)
	dbxgcg_pir_keyexp = ajRegCompC("^C;Keywords:");

    rpos = ajFileTell(infr);

    ajDebug("++id '%S'\n", *id);


    ajFileGets(infr, &rline);
    ajDebug("line-2 '%S'\n", rline);
    if(entry->do_description)
    {
	while(ajRegExec(dbxgcg_pir_wrdexp, rline))
	{
	    ajRegSubI(dbxgcg_pir_wrdexp, 1, &tmpfd);
	    ajDebug("++des '%S'\n", tmpfd);

	    str = ajStrNew();
	    ajStrAssignS(&str,tmpfd);
	    ajListPush(entry->de,(void *)str);

	    ajRegPost(dbxgcg_pir_wrdexp, &rline);
	}
    }

    while(ajStrGetCharFirst(rline)!='>')
    {
        rpos = ajFileTell(infr);
	ajStrAssignS(&tmpstr,rline);

	if(ajRegExec(dbxgcg_pir_acexp, rline))
	{
	    ajRegPost(dbxgcg_pir_acexp, &tmpline);
	    while(ajRegExec(dbxgcg_pir_ac2exp, tmpline))
	    {
		ajRegSubI(dbxgcg_pir_ac2exp, 1, &tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		if(entry->do_accession)
		{
		    str = ajStrNew();
		    ajStrAssignS(&str,tmpfd);
		    ajListPush(entry->ac,(void *)str);
		}

		ajRegPost(dbxgcg_pir_ac2exp, &tmpline);
	    }
	}

	if(entry->do_keyword)
	{
	    if(ajRegExec(dbxgcg_pir_keyexp, rline))
	    {
		ajRegPost(dbxgcg_pir_keyexp, &tmpline);
		while(ajRegExec(dbxgcg_pir_phrexp, tmpline))
		{
		    ajRegSubI(dbxgcg_pir_phrexp, 1, &tmpfd);
		    ajDebug("++key '%S'\n", tmpfd);
		    ajStrTrimWhiteEnd(&tmpfd);

		    str = ajStrNew();
		    ajStrAssignS(&str,tmpfd);
		    ajListPush(entry->kw,(void *)str);

		    ajRegPost(dbxgcg_pir_phrexp, &tmpline);
		}
	    }
	}

	if(entry->do_taxonomy)
	{
	    if(ajRegExec(dbxgcg_pir_taxexp, rline))
	    {
		ajRegPost(dbxgcg_pir_taxexp, &tmpline);
		while(ajRegExec(dbxgcg_pir_tax2exp, tmpline))
		{
		    ajRegSubI(dbxgcg_pir_tax2exp, 1, &tmpfd);
		    ajStrTrimWhiteEnd(&tmpfd);
		    ajDebug("++tax '%S'\n", tmpfd);

		    str = ajStrNew();
		    ajStrAssignS(&str,tmpfd);
		    ajListPush(entry->tx,(void *)str);

		    ajRegPost(dbxgcg_pir_tax2exp, &tmpline);
		}
	    }
	}

	if(!ajFileGets(infr, &rline))
	{
	    rpos = 0;
	    break;
	}
    }

    if(rpos)
	ajFileSeek(infr, rpos, 0);

    ajStrDel(&rline);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpline);
    ajStrDel(&tmpfd);

    return ajFalse;
}
