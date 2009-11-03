/* @source profit application
**
** Scan a protein database or sequence with a profile or matrix
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
#include <string.h>

#define AZ 27




static void profit_read_profile(AjPFile inf, AjPStr *name, AjPStr *mname,
				ajint *mlen, float *gapopen, float *gapextend,
				ajint *thresh, float *maxs, AjPStr *cons);

static void profit_read_simple(AjPFile inf, AjPStr *name, ajint *mlen,
			       ajint *maxs, ajint *thresh,AjPStr *cons);

static void profit_scan_profile(const AjPStr substr, const AjPStr pname,
				ajint mlen, float * const *fmatrix,
				ajint thresh, float maxs, AjPFile outf);

static void profit_scan_simple(const AjPStr substr,
			       const AjPStr pname,
			       ajint mlen, ajint maxs, ajint thresh,
			       ajint * const *matrix,
			       AjPFile outf);

static void profit_printHits(const AjPStr pname, ajint pos,
			     ajint score, AjPFile outf);

static ajint profit_getType(AjPFile inf);




/* @prog profit ***************************************************************
**
** Scan a sequence or database with a matrix or profile
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq   = NULL;
    AjPFile inf  = NULL;
    AjPFile outf = NULL;

    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr name   = NULL;
    AjPStr mname  = NULL;
    AjPStr pname  = NULL;
    AjPStr line   = NULL;
    AjPStr cons   = NULL;

    ajint type;
    ajint begin;
    ajint end;

    ajint i;
    ajint j;

    ajint **matrix  = NULL;
    float **fmatrix = NULL;

    void  **fptr = NULL;

    ajint mlen;
    ajint maxs;
    float maxfs;
    ajint thresh;

    float gapopen;
    float gapextend;

    const char *p;


    embInit("profit", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    inf    = ajAcdGetInfile("infile");
    outf   = ajAcdGetOutfile("outfile");

    substr = ajStrNew();
    name   = ajStrNew();
    mname  = ajStrNew();
    line   = ajStrNew();

    if(!(type=profit_getType(inf)))
	ajFatal("Unrecognised profile/matrix file format");

    switch(type)
    {
    case 1:
	profit_read_simple(inf, &name, &mlen, &maxs, &thresh, &cons);
	AJCNEW(matrix, mlen);
	fptr=(void **)matrix;

	for(i=0;i<mlen;++i)
	{
	    AJCNEW(matrix[i], AZ);
	    if(!ajReadlineTrim(inf,&line))
		ajFatal("Missing matrix line");
	    p = ajStrGetPtr(line);
	    p = ajSysFuncStrtok(p," \t");
	    for(j=0;j<AZ;++j)
	    {
		sscanf(p,"%d",&matrix[i][j]);
		p = ajSysFuncStrtok(NULL," \t");
	    }
	}
	ajFmtPrintF(outf,"# PROF scan using simple frequency matrix %s\n",
		    ajStrGetPtr(name));
	ajFmtPrintF(outf,"# Scores >= threshold %d (max score %d)\n#\n",thresh,
		    maxs);
	break;
    case 2:
	profit_read_profile(inf,&name,&mname,&mlen,&gapopen,&gapextend,&thresh,
			    &maxfs, &cons);
	AJCNEW(fmatrix, mlen);
	fptr=(void **)fmatrix;
	for(i=0;i<mlen;++i)
	{
	    AJCNEW(fmatrix[i],(AZ+1));
	    if(!ajReadlineTrim(inf,&line))
		ajFatal("Missing matrix line");
	    p = ajStrGetPtr(line);
	    p = ajSysFuncStrtok(p," \t");
	    for(j=0;j<AZ;++j)
	    {
		sscanf(p,"%f",&fmatrix[i][j]);
		p = ajSysFuncStrtok(NULL," \t");
	    }
	}
	ajFmtPrintF(outf,"# PROF scan using Gribskov profile %s\n",
		    ajStrGetPtr(name));
	ajFmtPrintF(outf,"# Scores >= threshold %d (max score %.2f)\n#\n",
		    thresh,maxfs);
	break;
    case 3:
	profit_read_profile(inf,&name,&mname,&mlen,&gapopen,&gapextend,&thresh,
			    &maxfs, &cons);
	AJCNEW(fmatrix, mlen);
	fptr=(void **)fmatrix;
	for(i=0;i<mlen;++i)
	{
	    AJCNEW(fmatrix[i], (AZ+1));
	    if(!ajReadlineTrim(inf,&line))
		ajFatal("Missing matrix line");
	    p = ajStrGetPtr(line);
	    p = ajSysFuncStrtok(p," \t");
	    for(j=0;j<AZ;++j)
	    {
		sscanf(p,"%f",&fmatrix[i][j]);
		p = ajSysFuncStrtok(NULL," \t");
	    }
	}
	ajFmtPrintF(outf,"# PROF scan using Henikoff profile %s\n",
		    ajStrGetPtr(name));
	ajFmtPrintF(outf,"# Scores >= threshold %d (max score %.2f)\n#\n",
		    thresh,maxfs);
	break;
    default:
	ajFatal("Switch type error");
	break;
    }


    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);

	ajStrAssignC(&pname,ajSeqGetNameC(seq));
	strand = ajSeqGetSeqCopyS(seq);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);

	switch(type)
	{
	case 1:
	    profit_scan_simple(substr,pname,mlen,maxs,thresh,matrix,
			       outf);
	    break;
	case 2:
	    profit_scan_profile(substr,pname,mlen,fmatrix,
				thresh,maxfs,outf);
	    break;
	case 3:
	    profit_scan_profile(substr,pname,mlen,fmatrix,thresh,
				maxfs,outf);
	    break;
	default:
	    break;
	}

	ajStrDel(&strand);
    }

    for(i=0;i<mlen;++i)
	AJFREE (fptr[i]);
    AJFREE (fptr);

    ajStrDel(&line);
    ajStrDel(&name);
    ajStrDel(&substr);
    ajStrDel(&mname);
    ajStrDel(&pname);
    ajStrDel(&cons);

    ajSeqDel(&seq);
    ajFileClose(&inf);
    ajFileClose(&outf);

    ajSeqallDel(&seqall);

    embExit();

    return 0;
}




/* @funcstatic profit_getType *************************************************
**
** Get type of matrix
**
** @param [u] inf [AjPFile] profile
** @return [ajint] 1=simple 2=Gribskov 3=Henikoff
** @@
******************************************************************************/

static ajint profit_getType(AjPFile inf)
{
    AjPStr line = NULL;
    const char *p = NULL;
    ajint ret = 0;

    line=ajStrNew();

    while(ajReadlineTrim(inf,&line))
    {
	p=ajStrGetPtr(line);
	if(!*p || *p=='#' || *p=='!' || *p=='\n') continue;
	break;
    }

    if(!strncmp(p,"Simple",6))
	ret = 1;

    if(!strncmp(p,"Gribskov",8))
	ret = 2;

    if(!strncmp(p,"Henikoff",8))
	ret = 3;

    ajStrDel(&line);

    return ret;
}




/* @funcstatic profit_read_simple *********************************************
**
** Undocumented.
**
** @param [u] inf [AjPFile] mtx file
** @param [w] name [AjPStr*] mtx name
** @param [w] mlen [ajint*] mtx length
** @param [w] maxs [ajint*] mtx max score
** @param [w] thresh [ajint*] mtx threshold
** @param [w] cons [AjPStr*] mtx consensus
** @@
******************************************************************************/


static void profit_read_simple(AjPFile inf, AjPStr *name, ajint *mlen,
			       ajint *maxs, ajint *thresh, AjPStr *cons)
{
    const char *p;

    AjPStr line = NULL;

    line = ajStrNew();

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Name",4))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysFuncStrtok(p," \t");
    p = ajSysFuncStrtok(NULL," \t");
    ajStrAssignC(name,p);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Length",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",mlen);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Maximum",7))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%*s%d",maxs);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Thresh",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",thresh);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Consensus",9))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysFuncStrtok(p," \t\n");
    p = ajSysFuncStrtok(NULL," \t\n");
    ajStrAssignC(cons,p);

    ajStrDel(&line);

    return;
}




/* @funcstatic profit_read_profile ********************************************
**
** Read profile
**
** @param [u] inf [AjPFile] Undocumented
** @param [w] name [AjPStr*] profile name
** @param [w] mname [AjPStr*] mtx name
** @param [w] mlen [ajint*] profile length
** @param [w] gapopen [float*] gap open penalty
** @param [w] gapextend [float*] gap extend penalty
** @param [w] thresh [ajint*] threshold
** @param [w] maxs [float*] maximum score
** @param [w] cons [AjPStr*] consensus sequence
** @@
******************************************************************************/

static void profit_read_profile(AjPFile inf, AjPStr *name, AjPStr *mname,
				ajint *mlen, float *gapopen, float *gapextend,
				ajint *thresh, float *maxs, AjPStr *cons)
{
    const char *p;
    AjPStr line = NULL;

    line = ajStrNew();

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Name",4))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysFuncStrtok(p," \t");
    p = ajSysFuncStrtok(NULL," \t");
    ajStrAssignC(name,p);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Matrix",6))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysFuncStrtok(p," \t");
    p = ajSysFuncStrtok(NULL," \t");
    ajStrAssignC(mname,p);


    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Length",6))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",mlen);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Max_score",9))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",maxs);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Threshold",9))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%d",thresh);


    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Gap_open",8))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",gapopen);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Gap_extend",10))
	ajFatal("Incorrect profile/matrix file format");
    sscanf(p,"%*s%f",gapextend);

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature EOF in profile file");
    p = ajStrGetPtr(line);

    if(strncmp(p,"Consensus",9))
	ajFatal("Incorrect profile/matrix file format");
    p = ajSysFuncStrtok(p," \t\n");
    p = ajSysFuncStrtok(NULL," \t\n");
    ajStrAssignC(cons,p);

    ajStrDel(&line);

    return;
}




/* @funcstatic profit_scan_simple *********************************************
**
** Scan sequence with a frequency matrix
**
** @param [r] substr [const AjPStr] sequence
** @param [r] pname [const AjPStr] profile name
** @param [r] mlen [ajint] mtx length
** @param [r] maxs [ajint] max score
** @param [r] thresh [ajint] threshold
** @param [r] matrix [ajint* const *] freq mtx
** @param [u] outf [AjPFile] outfile
** @@
******************************************************************************/

static void profit_scan_simple(const AjPStr substr,
			       const AjPStr pname,
			       ajint mlen, ajint maxs, ajint thresh,
			       ajint * const *matrix,
			       AjPFile outf)
{
    ajint len;
    ajint i;
    ajint j;
    ajint lim;
    const char *p;

    ajint score;
    ajint sum;


    len = ajStrGetLen(substr);
    lim = len-mlen+1;
    p = ajStrGetPtr(substr);

    for(i=0;i<lim;++i)
    {
	sum = 0;
	for(j=0;j<mlen;++j)
	    sum += matrix[j][ajBasecodeToInt(*(p+i+j))];
	score = sum * 100 / maxs;
	if(score >= thresh)
	    profit_printHits(pname,i,score,outf);
    }

    return;
}




/* @funcstatic profit_printHits ***********************************************
**
** Print results for profit
**
** @param [r] pname [const AjPStr] profile name
** @param [r] pos [ajint] position
** @param [r] score [ajint] score
** @param [u] outf [AjPFile] outfile
** @@
******************************************************************************/


static void profit_printHits(const AjPStr pname, ajint pos,
			     ajint score, AjPFile outf)
{
    ajFmtPrintF(outf,"%s %d Percentage: %d\n",ajStrGetPtr(pname),pos+1,score);

    return;
}




/* @funcstatic profit_scan_profile ********************************************
**
** Scan sequence with a profile
**
** @param [r] substr [const AjPStr] sequence
** @param [r] pname [const AjPStr] profile name
** @param [r] mlen [ajint] profile length
** @param [r] fmatrix [float* const *] score matrix
** @param [r] thresh [ajint] threshold
** @param [r] maxs [float] max score
** @param [u] outf [AjPFile] outfile
** @@
******************************************************************************/


static void profit_scan_profile (const AjPStr substr,
				 const AjPStr pname,
				 ajint mlen,
				 float * const *fmatrix,
				 ajint thresh, float maxs, AjPFile outf)
{
    ajint len;
    ajint i;
    ajint j;
    ajint lim;
    const char *p;

    float score;
    float sum;


    len = ajStrGetLen(substr);
    lim = len-mlen+1;
    p = ajStrGetPtr(substr);

    for(i=0;i<lim;++i)
    {
	sum=0.0;
	for(j=0;j<mlen;++j)
	    sum += fmatrix[j][ajBasecodeToInt(*(p+i+j))];
	score = sum * (float)100. / maxs;
	if((ajint)score >= thresh)
	    profit_printHits(pname,i,(ajint)score,outf);
    }

    return;
}
