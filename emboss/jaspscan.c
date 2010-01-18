/* @source jaspscan application
**
** Finds transcription factors using JASPAR matrices
** @author Copyright (C) Alan Bleasby (ajb@ebi.ac.uk)
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

#define J_COR "JASPAR_CORE"
#define J_FAM "JASPAR_FAM"
#define J_PHY "JASPAR_PHYLOFACTS"
#define J_CNE "JASPAR_CNE"
#define J_POL "JASPAR_POLII"
#define J_SPL "JASPAR_SPLICE"
#define J_EXT ".pfm"

#define J_LIST "matrix_list.txt"

#define JASPSCAN_RECURS 10 

#define JASPTAB_GUESS 1000


/* @datastatic AjPJsphits *****************************************************
**
** Jaspar hits object
**
** Holds hits from scanning a sequence with a Jaspar matrix
** Also holds matrix information and type.
**
** AjPJsphits is implemented as a pointer to a C data structure.
**
** @alias AjPPStr
** @alias AjSJsphits
** @alias AjOJsphits
**
** @attr matname [AjPStr] matrix name
** @attr start [ajuint] start position
** @attr end [ajuint] end position
** @attr score [float] score
** @attr scorepc [float] percentage score
** @attr threshold [float] threshold score
** @attr maxscore [float] maximum score for a matrix
** @attr type [char] type of Jaspar database (C,F or P)
** @attr Padding [char[7]] padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSJspHits {
  AjPStr matname;
  ajuint start;
  ajuint end;
  float  score;
  float  scorepc;
  float  threshold;
  float  maxscore;
  char type;
  char Padding[7];
} AjOJsphits;
#define AjPJsphits AjOJsphits*




/* @datastatic AjPJspmat ******************************************************
**
** Jaspar matrix object
**
** Also holds matrix information and type from the matrix_list.txt file
**
** AjPJspmat is implemented as a pointer to a C data structure.
**
** @alias AjSJspmat
** @alias AjOJspmat
**
** @attr id [AjPStr] Identifier
** @attr num [AjPStr] Information content (very close to optional content value)
** @attr name [AjPStr] Name or transcription factor
** @attr klass [AjPStr] Class of transcription factor
** @attr species [AjPStr] Species
** @attr sgroup [AjPStr] Taxonomy supergroup
** @attr protseq [AjPStr] Source database accession
** @attr exp [AjPStr] Experiment type (e.g. SELEX)
** @attr pmid [AjPStr] Source medline reference record and PMID number
** @attr content [AjPStr] Shannon information content (floating point number)
** @attr models [AjPStr] Included models
** @attr mcs [AjPStr] MCS reference
** @attr jaspar [AjPStr] Jaspar reference
** @attr transfac [AjPStr] Transfac reference
** @attr desc [AjPStr] Description
** @attr comment [AjPStr] comment
** @attr erttss [AjPStr] End relative to TSS
** @attr srttss [AjPStr] Start relative to TSS
** @attr consens [AjPStr] Consensus
** @attr type [char] Type of Jaspar database (C,F or P)
** @attr Padding [char[7]] padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSJspmat {
    AjPStr id;
    AjPStr num;
    AjPStr name;
    AjPStr klass;
    AjPStr species;
    AjPStr sgroup;
    AjPStr protseq;
    AjPStr exp;
    AjPStr pmid;
    AjPStr content;
    AjPStr models;
    AjPStr mcs;
    AjPStr jaspar;
    AjPStr transfac;
    AjPStr desc;
    AjPStr comment;
    AjPStr erttss;
    AjPStr srttss;
    AjPStr consens;
    char type;
    char Padding[7];
} AjOJspmat;
#define AjPJspmat AjOJspmat*




static void    jaspscan_ParseInput(const AjPStr dir, const AjPStr jaspdir,
				   const AjPStr mats, const AjPStr excl,
				   ajuint *recurs, AjPList ret);
static void    jaspscan_GetFileList(const AjPStr dir, const AjPStr jaspdir,
				    const char *wild, AjPList list);
static void    jaspscan_strdel(void** str, void* cl);
static void    jaspscan_scan(const AjPStr seq, const ajuint begin,
			     const AjPStr mfname, const char type,
			     const float threshold,
			     const AjBool both, AjPList hits);


static AjPJsphits jaspscan_hitsnew(void);
static void       jaspscan_hitsdel(AjPJsphits *thys);

static AjPJspmat  jaspscan_infonew(void);
static void       jaspscan_infodel(AjPJspmat *thys);

static AjPTable jaspscan_ReadCoreList(const AjPStr jaspdir);
static AjPTable jaspscan_ReadFamList(const AjPStr jaspdir);

static void   jaspscan_coretoken(AjPJspmat info, const AjPStr str);
static ajuint jaspscan_readmatrix(const AjPStr mfname, float ***matrix);

static void   jaspscan_ReportHits(AjPFeattable TabRpt, const AjPTable mattab,
				  AjPList hits);

static void   jaspscan_ClearTable(void **key, void **value, void *cl);
static void   jaspscan_CompMat(float **matrix, ajuint cols);




/* @prog jaspscan ************************************************************
**
** Scans DNA sequences for transcription factors
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq   = NULL;
    AjPReport report = NULL;

    AjPStr jaspdir = NULL;
    AjPStr menu    = NULL;
    AjPStr substr  = NULL;
    AjPStr mats    = NULL;
    AjPStr excl    = NULL;

    float thresh = 0.;
    
    ajuint recurs  = 0;
    
    AjPStr dir    = NULL;
    AjPStr mfname = NULL;
    
    AjPList flist = NULL;
    AjPList hits  = NULL;

    AjPStr head   = NULL;
    
    
    ajint begin;
    ajint end;
    ajuint mno;
    
    char cp;
    ajuint i;
    AjPTable mattab = NULL;
    AjPFeattable TabRpt = NULL;
    AjBool both = ajFalse;
    

    embInit("jaspscan", argc, argv);

    seqall     = ajAcdGetSeqall("sequence");
    menu       = ajAcdGetListSingle("menu");
    mats       = ajAcdGetString("matrices");
    excl       = ajAcdGetString("exclude");
    thresh     = ajAcdGetFloat("threshold");
    report     = ajAcdGetReport("outfile");
    both       = ajAcdGetBoolean("both");
    
    jaspdir = ajStrNew();
    substr  = ajStrNew();
    
    flist = ajListNew();
    hits  = ajListNew();
    dir   = ajStrNew();
    head  = ajStrNew();
    
    cp = ajStrGetCharFirst(menu);

    if(cp=='C')
	ajStrAssignC(&jaspdir,J_COR);
    else if(cp=='F')
	ajStrAssignC(&jaspdir,J_FAM);
    else if(cp=='P')
	ajStrAssignC(&jaspdir,J_PHY);
    else if(cp=='N')
	ajStrAssignC(&jaspdir,J_CNE);
    else if(cp=='O')
	ajStrAssignC(&jaspdir,J_POL);
    else if(cp=='S')
	ajStrAssignC(&jaspdir,J_SPL);
    else
	ajFatal("Invalid JASPAR database selection");


    ajStrAssignS(&dir, ajDatafileValuePath());
    if(!ajStrGetLen(dir))
	ajFatal("EMBOSS DATA directory couldn't be determined");


    jaspscan_ParseInput(dir, jaspdir, mats, excl, &recurs, flist);
    mno = ajListGetLength(flist);


    if(cp == 'C')
	mattab = jaspscan_ReadCoreList(jaspdir);
    if(cp == 'F')
	mattab = jaspscan_ReadFamList(jaspdir);
    if(cp == 'P')
	mattab = jaspscan_ReadCoreList(jaspdir);
    if(cp == 'N')
	mattab = jaspscan_ReadCoreList(jaspdir);
    if(cp == 'O')
	mattab = jaspscan_ReadCoreList(jaspdir);
    if(cp == 'S')
	mattab = jaspscan_ReadCoreList(jaspdir);

    ajFmtPrintS(&head,"Database scanned: %S  Threshold: %.3f",jaspdir,thresh);
    ajReportSetHeaderS(report,head);
    
    while(ajSeqallNext(seqall, &seq))
    {
	begin  = ajSeqallGetseqBegin(seqall);
	end    = ajSeqallGetseqEnd(seqall);

	ajStrAssignSubC(&substr,ajSeqGetSeqC(seq),begin-1,end-1);
	ajStrFmtUpper(&substr);

	TabRpt = ajFeattableNewSeq(seq);


	for(i=0; i < mno; ++i)
	{
	    ajListPop(flist,(void **)&mfname);

	    jaspscan_scan(substr,begin,mfname, cp, thresh, both, hits);

            ajListPushAppend(flist, (void **)mfname);
	}

	jaspscan_ReportHits(TabRpt,mattab,hits);

	ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
    }


    while(ajListPop(flist,(void **)&mfname))
        ajStrDel(&mfname);

    
    ajStrDel(&dir);
    ajStrDel(&menu);
    ajStrDel(&excl);
    ajStrDel(&substr);
    ajStrDel(&mats);
    ajStrDel(&head);
    ajStrDel(&jaspdir);

    ajSeqDel(&seq);

    ajTableMapDel(mattab,jaspscan_ClearTable,NULL);
    ajTableFree(&mattab);

    ajListFree(&flist);
    ajListFree(&hits);
    
    ajSeqallDel(&seqall);
    ajReportDel(&report);
    
    embExit();

    return 0;
}




/* @funcstatic jaspscan_GetFileList *******************************************
**
** Add Jaspar matrix files to a list
**
** @param [r] dir [const AjPStr] DATA directory
** @param [r] jaspdir [const AjPStr] Jaspar subdirectory name
** @param [r] wild [const char *] wildcard matrix name
** @param [u] list [AjPList] list for appending matrices
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_GetFileList(const AjPStr dir, const AjPStr jaspdir,
				 const char *wild, AjPList list)
{
    AjPList tlist = NULL;
    AjPStr jdir = NULL;
    AjPStr wstr = NULL;
    AjPStr str  = NULL;
    
    tlist = ajListNew();
    jdir  = ajStrNew();
    wstr  = ajStrNew();
    

    ajFmtPrintS(&jdir,"%S%S",dir,jaspdir);
    if(!ajDirnameFixExists(&jdir))
	ajFatal("EMBOSS_DATA undefined or 'jaspextract' needs to be run");

    ajFmtPrintS(&wstr,"%s%s",wild,J_EXT);

    ajFilelistAddPathWild(tlist,jdir,wstr);

    if(!ajListGetLength(tlist))
	ajWarn("Matrix file(s) %S not found",wstr);

    while(ajListPop(tlist,(void **)&str))
	ajListPushAppend(list,(void *)str);

    ajStrDel(&wstr);
    ajStrDel(&jdir);

    ajListFree(&tlist);

    return;
}




/* @funcstatic jaspscan_ParseInput *******************************************
**
** Parse 'matrices' and 'exclude' inputs
**
** @param [r] dir [const AjPStr] DATA directory
** @param [r] jaspdir [const AjPStr] Jaspar subdirectory name
** @param [r] mats [const AjPStr] comma separated list of matrices
** @param [r] excl [const AjPStr] comma separated list of excludes
** @param [u] recurs [ajuint *] recursion count
** @param [u] ret [AjPList] resultant matrices list
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_ParseInput(const AjPStr dir, const AjPStr jaspdir,
				const AjPStr mats, const AjPStr excl,
				ajuint *recurs, AjPList ret)
{
    ajuint nm = 0;
    ajuint ne = 0;
    AjPStr *carr = NULL;
    AjPStr *earr = NULL;
    AjPFile inf = NULL;
    AjPStr line = NULL;
    AjPStr comm = NULL;
    AjPStr val  = NULL;
    
    ajuint i;
    ajuint j;
    
    char c;

    ajuint rlen = 0;
    

    if(*recurs > JASPSCAN_RECURS)
	ajFatal("Too many recursion levels in matrix list files");
    
    line = ajStrNew();
    comm = ajStrNew();
    

    if(mats)
    {
	nm = ajArrCommaList(mats,&carr);	
	for(i=0; i < nm; ++i)
	{
	    if(ajStrGetCharFirst(carr[i]) != '@')
		ajStrFmtUpper(&carr[i]);
	
	    if(ajStrMatchC(carr[i],"ALL"))
	    {
		jaspscan_GetFileList(dir, jaspdir, "*", ret);
		ajListSortUnique(ret, ajStrVcmp, jaspscan_strdel);
	    }
	    else if(ajStrGetCharFirst(carr[i]) == '@')
	    {
		ajStrTrimStartC(&carr[i],"@");
		inf = ajFileNewInNameS(carr[i]);
		if(!inf)
		    ajFatal("Cannot open list file %S",carr[i]);

		while(ajReadlineTrim(inf,&line))
		{
		    ajStrRemoveWhite(&line);
		    c = ajStrGetCharFirst(line);
		    if(c == '#' || c== '!')
			continue;
		    if(ajStrGetLen(comm))
			ajStrAppendC(&comm,",");
		    ajStrFmtUpper(&line);
		    ajStrAppendS(&comm,line);
		}

		*recurs += 1;
		jaspscan_ParseInput(dir,jaspdir,comm,NULL,recurs,ret);
		*recurs -= 1;
		ajListSortUnique(ret, ajStrVcmp, jaspscan_strdel);	    

		ajFileClose(&inf);
	    }
	    else
	    {
		jaspscan_GetFileList(dir,jaspdir,ajStrGetPtr(carr[i]),ret);
		ajListSortUnique(ret, ajStrVcmp, jaspscan_strdel);
	    }
	}

	for(i=0; i < nm; ++i)
	    ajStrDel(&carr[i]);

	AJFREE(carr);
    }
    
    


    if(excl)
    {
	ne = ajArrCommaList(excl,&earr);
	
	for(i=0; i < ne; ++i)
	{
	    if(ajStrGetCharFirst(earr[i]) != '@')
		ajStrFmtUpper(&earr[i]);
	
	    if(ajStrGetCharFirst(earr[i]) == '@')
	    {
		ajStrTrimStartC(&earr[i],"@");
		inf = ajFileNewInNameS(earr[i]);
		if(!inf)
		    ajFatal("Cannot open list file %S",earr[i]);

		while(ajReadlineTrim(inf,&line))
		{
		    ajStrRemoveWhite(&line);
		    c = ajStrGetCharFirst(line);
		    if(c == '#' || c== '!')
			continue;
		    if(ajStrGetLen(comm))
			ajStrAppendC(&comm,",");
		    ajStrFmtUpper(&line);
		    ajStrAppendS(&comm,line);
		}

		*recurs += 1;
		jaspscan_ParseInput(dir,jaspdir,NULL,comm,recurs,ret);
		*recurs -= 1;
		ajListSortUnique(ret, ajStrVcmp, jaspscan_strdel);	    

		ajFileClose(&inf);
	    }
	    else
	    {
		ajStrAssignS(&line,earr[i]);
		ajStrAppendC(&line,J_EXT);
		rlen = ajListGetLength(ret);
		for(j=0; j < rlen; ++j)
		{
		    ajListPop(ret,(void **)&val);
		    if(ajStrSuffixS(val,line))
			ajStrDel(&val);
		    else
			ajListPushAppend(ret,(void *)val);
		}
		
	    }
	} 


	for(i=0; i < ne; ++i)
	    ajStrDel(&earr[i]);
	AJFREE(earr);
    }
    

    ajStrDel(&line);
    ajStrDel(&comm);

    return;
}




/* @funcstatic jaspscan_strdel *********************************************
**
** Deletes a string when called by ajListSortUnique
**
** @param [r] str [void**] string to delete
** @param [r] cl [void*] not used
** @return [void]
** @@
******************************************************************************/

static void jaspscan_strdel(void** str, void* cl) 
{
    (void) cl;				/* make it used */

    ajStrDel((AjPStr*)str);

    return;
}




/* @funcstatic jaspscan_scan *********************************************
**
** Scans a sequence with a matrix
**
** @param [r] seq [const AjPStr] sequence
** @param [r] begin [const ajuint] start position
** @param [r] mfname [const AjPStr] matrix file name
** @param [r] type [const char] Jaspar database type (C,F or P)
** @param [r] threshold [const float] scoring threshold
** @param [r] both [const AjBool] scan reverse strand too
** @param [u] hits [AjPList] hit list
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_scan(const AjPStr seq, const ajuint begin,
			  const AjPStr mfname, const char type,
			  const float threshold,
			  const AjBool both, AjPList hits)
{
    AjPJsphits val = NULL;
    AjPStr mname   = NULL;
    float **matrix = NULL;
    ajuint cols;

    ajuint i;
    ajuint rc;
    ajuint cc;
    ajuint limit;
    
    ajuint slen;
    const char *p;

    char schar;
    float sum  = 0.;
    float rmax = 0.;

    float scorepc  = 0.;
    float maxscore = 0.;

    
    cols = jaspscan_readmatrix(mfname, &matrix);

    maxscore = 0.;
    for(cc = 0; cc < cols; ++cc)
    {
	rmax = 0.;
	for(rc = 0; rc < 4; ++rc)
	    rmax = (rmax > matrix[rc][cc]) ? rmax : matrix[rc][cc];
	maxscore += rmax;
    }
    

 
    slen = ajStrGetLen(seq);

    limit = (slen - cols) + 1;
    if(limit <= 0)
	return;

    mname = ajStrNew();

    ajStrAssignS(&mname,mfname);
    ajFilenameTrimPath(&mname);
    ajFilenameTrimExt(&mname);

    p = ajStrGetPtr(seq);

    for(i=0; i < limit; ++i)
    {
	sum = 0.;
	schar = p[i];
	for(cc = 0; cc < cols; ++cc)
	{
	    schar = p[i+cc];
	    if(schar == 'A')
		sum += matrix[0][cc];
	    else if(schar == 'C')
		sum += matrix[1][cc];
	    else if(schar == 'G')
		sum += matrix[2][cc];
	    else if(schar == 'T')
		sum += matrix[3][cc];
	}


	scorepc = (sum * (float)100.) / maxscore;


	if(scorepc >= threshold)
	{
	    val = jaspscan_hitsnew();
	    val->type = type;
	    ajStrAssignS(&val->matname,mname);
	    val->start = i + begin;
	    val->end = val->start + cols - 1;
	    val->score = sum;
	    val->threshold = threshold;
	    val->scorepc  = scorepc;
	    val->maxscore = maxscore;

	    ajListPushAppend(hits,(void *)val);
	}
    }
    

    if(both)
    {
	jaspscan_CompMat(matrix, cols);

	p = ajStrGetPtr(seq);

	for(i=0; i < limit; ++i)
	{
	    sum = 0.;
	    schar = p[i];
	    for(cc = 0; cc < cols; ++cc)
	    {
		schar = p[i+cc];
		if(schar == 'A')
		    sum += matrix[0][cc];
		else if(schar == 'C')
		    sum += matrix[1][cc];
		else if(schar == 'G')
		    sum += matrix[2][cc];
		else if(schar == 'T')
		    sum += matrix[3][cc];
	    }


	    scorepc = (sum * (float)100.) / maxscore;


	    if(scorepc >= threshold)
	    {
		val = jaspscan_hitsnew();
		val->type = type;
		ajStrAssignS(&val->matname,mname);
		val->end = i + begin;
		val->start = val->end + cols - 1;
		val->score = sum;
		val->threshold = threshold;
		val->scorepc  = scorepc;
		val->maxscore = maxscore;

		ajListPushAppend(hits,(void *)val);
	    }
	}
    }
    

    for(i = 0; i < 4; ++i)
	AJFREE(matrix[i]);
    AJFREE(matrix);


    ajStrDel(&mname);

    return;
}




/* @funcstatic jaspscan_readmatrix *********************************************
**
** Read a Jaspar matrix
**
** @param [r] mfname [const AjPStr] Jaspar matrix file name
** @param [w] matrix [float ***] matrix
**
** @return [ajuint] number of columns
** @@
******************************************************************************/

static ajuint jaspscan_readmatrix(const AjPStr mfname, float ***matrix)
{

    AjPFile inf  = NULL;
    AjPStr line  = NULL;

    ajuint i = 0;
    ajuint cols = 0;

    AJCNEW0(*matrix,4);

    line = ajStrNew();

    inf = ajFileNewInNameS(mfname);
    if(!inf)
	ajFatal("Cannot open matrix file %S",mfname);

    i = 0;
    while(ajReadlineTrim(inf,&line))
    {
	if(!i)
	    cols = ajStrParseCountC(line," \n");

	(*matrix)[i++] = ajArrFloatLine(line," \n",1,cols);
    }
    


    ajStrDel(&line);
    ajFileClose(&inf);
    
    return cols;;
}




/* @funcstatic jaspscan_hitsnew *********************************************
**
** Creates a hits object
**
** @return [AjPJsphits] list of hits
** @@
******************************************************************************/

static AjPJsphits jaspscan_hitsnew(void)
{
    AjPJsphits thys = NULL;
    
    AJNEW0(thys);
    thys->matname = ajStrNew();

    return thys;
}




/* @funcstatic jaspscan_hitsdel *********************************************
**
** Delete a Jaspar hits object
**
** @param [w] thys [AjPJsphits*] Jaspar hits object
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_hitsdel(AjPJsphits *thys)
{
    AjPJsphits pthis;

    pthis = *thys;

    ajStrDel(&pthis->matname);
    AJFREE(pthis);
    
    *thys = NULL;

    return;
}




/* @funcstatic jaspscan_ReadCoreList *****************************************
**
** Read/parse the JASPAR_CORE/JASPAR_PHYLOFACTS  matrix_list.txt files
**
** @param [r] jaspdir [const AjPStr] Jaspar directory
**
** @return [AjPTable] AjPJspmat objects with matrix ID keys
** @@
******************************************************************************/

static AjPTable jaspscan_ReadCoreList(const AjPStr jaspdir)
{
    AjPTable ret = NULL;
    AjPStr lfile = NULL;
    AjPStr line  = NULL;
    AjPStr key   = NULL;
    AjPStr str   = NULL;
 
    AjPJspmat info = NULL;
    
    AjPFile inf  = NULL;
    const char *p = NULL;
    const char *q = NULL;
    
    lfile = ajStrNew();
    line  = ajStrNew();
    str   = ajStrNew();
    
    ajFmtPrintS(&lfile,"%S%s%s",jaspdir,SLASH_STRING,J_LIST);


    inf = ajDatafileNewInNameS(lfile);
    if(!inf)
	ajFatal("Matrix list file %S not found",lfile);


    ret = ajTablestrNewLen(JASPTAB_GUESS);
    
    
    while(ajReadlineTrim(inf,&line))
    {
	info = jaspscan_infonew();
	ajFmtScanS(line,"%S%S%S",&info->id,&info->num,&info->name);

	p = ajStrGetPtr(line);
	while(*p !='\t')
	    ++p;
	++p;
	while(*p != '\t')
	    ++p;
	++p;
	while(*p != '\t')
	    ++p;
	++p;
	q = p;
	while(*q != ';')
	    ++q;

	ajStrAssignSubC(&info->klass,p,0,q-p-1);
	ajStrRemoveWhiteExcess(&info->klass);

	p = q+1;
	while(*p)
	{
	    q = p;
	    while(* q && *q != ';')
		++q;
	    ajStrAssignSubC(&str,p,0,q-p-1);
	    ajStrRemoveWhiteExcess(&str);

	    jaspscan_coretoken(info, str);
	    
	    if(!*q)
		p = q;
	    else
		p = q +1;
	}

	key = ajStrNew();
	ajStrAssignS(&key,info->id);
	ajTablePut(ret,(void *)key,(void *) info);
    }

    ajFileClose(&inf);

    ajStrDel(&lfile);
    ajStrDel(&line);
    ajStrDel(&str);
	

    return ret;
}




/* @funcstatic jaspscan_ReadFamList *****************************************
**
** Read/parse the JASPAR_FAM matrix_list.txt file
**
** @param [r] jaspdir [const AjPStr] Jaspar directory
**
** @return [AjPTable] AjPJspmat objects with matrix ID keys
** @@
******************************************************************************/

static AjPTable jaspscan_ReadFamList(const AjPStr jaspdir)
{
    AjPTable ret = NULL;
    AjPStr lfile = NULL;
    AjPStr line  = NULL;
    AjPStr key   = NULL;
    AjPStr str   = NULL;
 
    AjPJspmat info = NULL;
    
    AjPFile inf  = NULL;
    const char *p = NULL;
    const char *q = NULL;
    
    lfile = ajStrNew();
    line  = ajStrNew();
    str   = ajStrNew();
    
    ajFmtPrintS(&lfile,"%S%s%s",jaspdir,SLASH_STRING,J_LIST);


    inf = ajDatafileNewInNameS(lfile);
    if(!inf)
	ajFatal("Matrix list file %S not found",lfile);


    ret = ajTablestrNewLen(JASPTAB_GUESS);
    
    
    while(ajReadlineTrim(inf,&line))
    {
	info = jaspscan_infonew();
	ajFmtScanS(line,"%S%S",&info->id,&info->num);

	p = ajStrGetPtr(line);
	while(*p !='\t')
	    ++p;
	++p;
	while(*p != '\t')
	    ++p;
	++p;

	q = p;
	while(*q != '\t')
	    ++q;

	ajStrAssignSubC(&info->name,p,0,q-p-1);

	++q;
	p = q;
	while(*q != ';')
	    ++q;

	ajStrAssignSubC(&info->klass,p,0,q-p-1);
	ajStrRemoveWhiteExcess(&info->klass);

	p = q+1;
	while(*p)
	{
	    q = p;
	    while(* q && *q != ';')
		++q;
	    ajStrAssignSubC(&str,p,0,q-p-1);
	    ajStrRemoveWhiteExcess(&str);

	    jaspscan_coretoken(info, str);
	    
	    if(!*q)
		p = q;
	    else
		p = q +1;
	}

	key = ajStrNew();
	ajStrAssignS(&key,info->id);
	ajTablePut(ret,(void *)key,(void *) info);
    }
    

    ajFileClose(&inf);

    ajStrDel(&lfile);
    ajStrDel(&line);
    ajStrDel(&str);
	

    return ret;
}




/* @funcstatic jaspscan_coretoken *****************************************
**
** Parse matrix_list.txt key/value pairs
**
** @param [u] info [AjPJspmat] Jaspar matrix information
** @param [r] str [const AjPStr] key/value pair
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_coretoken(AjPJspmat info, const AjPStr str)
{
    const char *p;
    const char *q;
    AjPStr key   = NULL;
    AjPStr value = NULL;

    value = ajStrNew();
    key   = ajStrNew();
    
    p = ajStrGetPtr(str);
    q = p;
    
    while(*q != '"' && *q != '\t')
	++q;

    --q;
    
    ajStrAssignSubC(&key,p,0,q-p-1);
    
    while(*q != '"')
	++q;

    if(*(q+1) != '"')
    {
	p = q + 1;
	++q;
	while(*q != '"')
	    ++q;
	ajStrAssignSubC(&value,p,0,q-p-1);
        
	if(ajStrMatchC(key,"acc"))
	    ajStrAssignS(&info->protseq,value);
	if(ajStrMatchC(key,"medline"))
	    ajStrAssignS(&info->pmid,value);
	if(ajStrMatchC(key,"species"))
	    ajStrAssignS(&info->species,value);
	if(ajStrMatchC(key,"sysgroup"))
	    ajStrAssignS(&info->sgroup,value);
	if(ajStrMatchC(key,"total_ic"))
	    ajStrAssignS(&info->content,value);
	if(ajStrMatchC(key,"type"))
	    ajStrAssignS(&info->exp,value);
	if(ajStrMatchC(key,"included_models"))
	    ajStrAssignS(&info->models,value);
	if(ajStrMatchC(key,"MCS"))
	    ajStrAssignS(&info->mcs,value);
	if(ajStrMatchC(key,"jaspar"))
	    ajStrAssignS(&info->jaspar,value);
	if(ajStrMatchC(key,"transfac"))
	    ajStrAssignS(&info->transfac,value);

	if(ajStrMatchC(key,"Description"))
	    ajStrAssignS(&info->desc,value);
	if(ajStrMatchC(key,"description"))
	    ajStrAssignS(&info->desc,value);
	if(ajStrMatchC(key,"comment"))
	    ajStrAssignS(&info->comment,value);
	if(ajStrMatchC(key,"End relative to TSS"))
	    ajStrAssignS(&info->erttss,value);
	if(ajStrMatchC(key,"Start relative to TSS"))
	    ajStrAssignS(&info->srttss,value);
	if(ajStrMatchC(key,"consensus"))
	    ajStrAssignS(&info->consens,value);
    }


    ajStrDel(&key);
    ajStrDel(&value);

    return;
}




/* @funcstatic jaspscan_infonew *********************************************
**
** Creates a hits object
**
** @return [AjPJspmat] matrix information
** @@
******************************************************************************/

static AjPJspmat jaspscan_infonew(void)
{
    AjPJspmat thys = NULL;
    
    AJNEW0(thys);

    thys->id       = ajStrNew();
    thys->num      = ajStrNew();
    thys->name     = ajStrNew();
    thys->klass    = ajStrNew();
    thys->species  = ajStrNew();
    thys->sgroup   = ajStrNew();
    thys->protseq  = ajStrNew();
    thys->exp      = ajStrNew();
    thys->pmid     = ajStrNew();
    thys->models   = ajStrNew();
    thys->mcs      = ajStrNew();
    thys->jaspar   = ajStrNew();
    thys->transfac = ajStrNew();
    thys->content  = ajStrNew();
    thys->desc     = ajStrNew();
    thys->comment  = ajStrNew();
    thys->erttss   = ajStrNew();
    thys->srttss   = ajStrNew();
    thys->consens  = ajStrNew();
    
    return thys;
}




/* @funcstatic jaspscan_infodel *********************************************
**
** Delete a Jaspar information object
**
** @param [w] thys [AjPJspmat*] Jaspar hits object
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_infodel(AjPJspmat *thys)
{
    AjPJspmat pthis;

    pthis = *thys;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->num);
    ajStrDel(&pthis->name);
    ajStrDel(&pthis->klass);
    ajStrDel(&pthis->species);
    ajStrDel(&pthis->sgroup);
    ajStrDel(&pthis->protseq);
    ajStrDel(&pthis->exp);
    ajStrDel(&pthis->pmid);
    ajStrDel(&pthis->models);
    ajStrDel(&pthis->mcs);
    ajStrDel(&pthis->jaspar);
    ajStrDel(&pthis->transfac);
    ajStrDel(&pthis->content);
    ajStrDel(&pthis->desc);
    ajStrDel(&pthis->comment);
    ajStrDel(&pthis->erttss);
    ajStrDel(&pthis->srttss);
    ajStrDel(&pthis->consens);

    AJFREE(*thys);

    *thys = NULL;

    return;
}




/* @funcstatic jaspscan_ReportHits ********************************************
**
** Report Jaspar hits
**
** @param [u] TabRpt [AjPFeattable] report object
** @param [r] mattab [const AjPTable] matrix information
** @param [u] hits [AjPList] List of hits removed as they are printed
**
** @return [void]
** @@
******************************************************************************/

static void jaspscan_ReportHits(AjPFeattable TabRpt, const AjPTable mattab,
				AjPList hits)
{
    AjPJsphits hit = NULL;
    AjPJspmat info = NULL;
    AjPFeature feat = NULL;
    AjPStr str = NULL;
    float fnum = 0.;
    
    str = ajStrNew();

    while(ajListPop(hits,(void **)&hit))
    {
	if(hit->start <= hit->end)
	    feat = ajFeatNewII(TabRpt,hit->start,hit->end);
	else
	    feat = ajFeatNewIIRev(TabRpt,hit->start,hit->end);

	ajFmtPrintS(&str,"*pc %.3f",hit->scorepc);
	ajFeatTagAdd(feat, NULL, str);

	ajFmtPrintS(&str,"*id %S",hit->matname);
	ajFeatTagAdd(feat, NULL, str);

	info = ajTableFetch(mattab,hit->matname);
	if(!info)
	    ajWarn("No info for matrix %S",hit->matname);
	
	if(ajStrGetLen(info->name))
	{
	    ajFmtPrintS(&str,"*name %S",info->name);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->species))
	{
	    ajFmtPrintS(&str,"*species %S",info->species);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->klass))
	{
	    ajFmtPrintS(&str,"*class %S",info->klass);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->sgroup))
	{
	    ajFmtPrintS(&str,"*supergroup %S",info->sgroup);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->protseq))
	{
	    ajFmtPrintS(&str,"*pseq %S",info->protseq);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->exp))
	{
	    ajFmtPrintS(&str,"*exp %S",info->exp);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->pmid))
	{
	    ajFmtPrintS(&str,"*pmid %S",info->pmid);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->num))
	{
	    ajStrToFloat(info->num,&fnum);
	    ajFmtPrintS(&str,"*info %.3f",fnum);
	    ajFeatTagAdd(feat, NULL, str);
	}
	
	if(ajStrGetLen(info->jaspar))
	{
	    ajFmtPrintS(&str,"*jaspar %S",info->jaspar);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->transfac))
	{
	    ajFmtPrintS(&str,"*transfac %S",info->transfac);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->mcs))
	{
	    ajStrToFloat(info->mcs,&fnum);
	    ajFmtPrintS(&str,"*mcs %.3f",fnum);
	    ajFeatTagAdd(feat, NULL, str);
	}

	if(ajStrGetLen(info->models))
	{
	    ajFmtPrintS(&str,"*built %S",info->models);
	    ajFeatTagAdd(feat, NULL, str);
	}

	
	jaspscan_hitsdel(&hit);
    }

    ajStrDel(&str);
    
    return;
}




/* @funcstatic jaspscan_ClearTable ************************************
**
** Clear info table
**
** @param [r] key [void**] Standard argument, table key.
** @param [r] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
**
** @return [void]
** @@
*********************************************************************/

static void jaspscan_ClearTable(void **key, void **value, void *cl)
{
    AjPJspmat info = (AjPJspmat) *value;
    AjPStr skey = (AjPStr) *key;
    
    ajStrDel(&skey);
    jaspscan_infodel(&info);

    (void) cl;				/* make it used */

    *key = NULL;
    *value = NULL;

    return;
}




/* @funcstatic jaspscan_CompMat ************************************
**
** Complement the matrix
**
** @param [u] matrix [float**] matrix
** @param [r] cols [ajuint] number of columns
**
** @return [void]
** @@
*********************************************************************/

static void jaspscan_CompMat(float **matrix, ajuint cols)
{
    float *mtmp = NULL;
    float *fp   = NULL;
    float *fq   = NULL;
    float ftmp  = 0.;
    ajuint i;

    mtmp = matrix[0];
    matrix[0] = matrix[3];
    matrix[3] = mtmp;

    mtmp = matrix[1];
    matrix[1] = matrix[2];
    matrix[2] = mtmp;

    for(i=0; i<4; ++i)
    {
	fp = matrix[i];
	fq = fp + cols - 1;

	while(fp < fq)
	{
	    ftmp = *fp;
	    *fp = *fq;
	    *fq = ftmp;
	    ++fp;
	    --fq;
	}
    }

    return;
}
