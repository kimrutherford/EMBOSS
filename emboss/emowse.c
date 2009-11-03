/* @source emowse application
**
** Finds proteins matching mass spectrometry data
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
#include <math.h>
#include <string.h>




#define FGUESS 128000		/* Uncritical guess of freq data size */
#define MILLION (double)1000000.
#define MAXLIST 50




/* @datastatic EmbPMdata ******************************************************
**
** EMowse data
**
** @alias EmbSMdata
** @alias EmbOMdata
**
** @attr mwt [double] Molecular weight
** @attr sdata [AjPStr] string data
******************************************************************************/

typedef struct EmbSMdata
{
    double mwt;
    AjPStr sdata;
} EmbOMdata;
#define EmbPMdata EmbOMdata*




/* @datastatic PHits **********************************************************
**
** EMowse hits
**
** @alias SHits
** @alias OHits
**
** @attr seq [AjPStr] Sequence
** @attr name [AjPStr] Name
** @attr desc [AjPStr] Description
** @attr found [AjPInt] Found data
** @attr score [double] Score
** @attr mwt [double] Molecular weight
** @attr frags [EmbPMolFrag*] Fragment data
** @attr nf [ajint] Number of fragments
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct SHits
{
    AjPStr seq;
    AjPStr name;
    AjPStr desc;
    AjPInt found;
    double score;
    double mwt;
    EmbPMolFrag* frags;
    ajint    nf;
    char Padding[4];
} OHits;
#define PHits OHits*




static void emowse_read_freqs(AjPFile finf, AjPDouble *freqs);
static AjBool emowse_molwt_outofrange(double thys, double given, double range);
static ajint emowse_read_data(AjPFile inf, EmbPMdata** data);
static ajint emowse_sort_data(const void *a, const void *b);
static ajint emowse_hit_sort(const void *a, const void *b);
static void emowse_match(EmbPMdata const * data, ajint dno, AjPList flist,
			 ajint nfrags, double tol,
			 const AjPSeq seq, AjPList hlist,
			 double partials, double cmw, ajint enz,
			 const AjPDouble freqs);

static ajint emowse_seq_comp(ajint bidx, ajint thys, const AjPSeq seq,
			     EmbPMdata const *data, EmbPMolFrag const *frags);
static ajint emowse_get_index(double actmw, double maxmw, double minmw,
			      EmbPMolFrag const *frags,
			      ajint fno, double *bestmw,
			      ajint *index, ajint thys, const AjPSeq seq,
			      EmbPMdata const *data);

static ajint emowse_seq_search(const AjPStr substr, AjPStr* s);
static AjBool emowse_msearch(const char *seq, const char *pat, AjBool term);
static void emowse_mreverse(char *s);
static ajint emowse_get_orc(AjPStr *orc, const char *s, ajint pos);
static AjBool emowse_comp_search(const AjPStr substr, const char *s);
static void emowse_print_hits(AjPFile outf, AjPList hlist, ajint dno,
			      EmbPMdata const * data);




/* @prog emowse ***************************************************************
**
** Protein identification by mass spectrometry
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq = NULL;
    AjPSeqall seqall = NULL;
    AjPFile outf = NULL;
    AjPFile mwinf = NULL;
    AjPFile ffile = NULL;
    AjPStr enzyme = NULL;
    ajint smolwt;
    ajint range;
    float tol;
    float partials;
    AjPDouble freqs = NULL;
    ajint begin;
    ajint end;
    double smw;
    ajint rno;
    ajint i;

    AjPList flist = NULL;
    EmbPMdata *data = NULL;
    ajint dno;
    ajint nfrags;
    AjPList hlist = NULL;
    AjPFile mfptr = NULL;

    embInit("emowse", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    mwinf    = ajAcdGetInfile("infile");
    enzyme   = ajAcdGetListSingle("enzyme");
    smolwt   = ajAcdGetInt("weight");
    range    = ajAcdGetInt("pcrange");
    ffile    = ajAcdGetDatafile("frequencies");
    tol      = ajAcdGetFloat("tolerance");
    partials = ajAcdGetFloat("partials");
    outf     = ajAcdGetOutfile("outfile");
    mfptr   = ajAcdGetDatafile("aadata");

    embPropAminoRead(mfptr);

    freqs = ajDoubleNewL(FGUESS);
    emowse_read_freqs(ffile, &freqs);
    ajFileClose(&ffile);

    if(ajFmtScanS(enzyme,"%d",&rno)!=1)
	ajFatal("Illegal enzyme entry [%S]",enzyme);


    if(!(dno = emowse_read_data(mwinf,&data)))
	ajFatal("No molecular weights in the file");
    ajFileClose(&mwinf);


    hlist = ajListNew();


    while(ajSeqallNext(seqall,&seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);


	smw = embPropCalcMolwt(ajSeqGetSeqC(seq),--begin,--end);
	if(smolwt)
	    if(emowse_molwt_outofrange(smw,(double)smolwt,(double)range))
		continue;

	flist  = ajListNew();
	nfrags = embMolGetFrags(ajSeqGetSeqS(seq),rno,&flist);

	emowse_match(data,dno,flist,nfrags,(double)tol,seq,hlist,
		     (double)partials,
		     smw,rno,freqs);

	ajListFree(&flist);
    }


    emowse_print_hits(outf,hlist,dno,data);
    for(i=0;i<dno;i++)
    {
	ajStrDel(&data[i]->sdata);
	AJFREE(data[i]);
    }
    AJFREE(data);

    ajListFree(&hlist);

    ajFileClose(&mfptr);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajFileClose(&outf);
    ajStrDel(&enzyme);

    ajDoubleDel(&freqs);
    ajListFree(&flist);

    embExit();

    return 0;
}




/* @funcstatic emowse_read_freqs **********************************************
**
** Undocumented.
**
** @param [u] finf [AjPFile] Undocumented
** @param [w] freqs [AjPDouble*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void emowse_read_freqs(AjPFile finf, AjPDouble *freqs)
{
    ajint c;
    AjPStr  str;
    double f;

    c   = 0;
    str = ajStrNew();

    while(ajFileReadLine(finf,&str))
    {
       	if(sscanf(ajStrGetPtr(str),"%lf",&f)==1)
	    ajDoublePut(freqs,c,f);
	else
	    ajDoublePut(freqs,c,0.);

	++c;
    }

    ajStrDel(&str);

    return;
}




/* @funcstatic emowse_molwt_outofrange ****************************************
**
** Undocumented.
**
** @param [r] thys [double] Undocumented
** @param [r] given [double] Undocumented
** @param [r] range [double] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool emowse_molwt_outofrange(double thys, double given, double range)
{
    double diff;

    diff = given * range / (double)100.;

    if(thys>=given-diff && thys<=given+diff)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic emowse_read_data ***********************************************
**
** Undocumented.
**
** @param [u] inf [AjPFile] Undocumented
** @param [w] data [EmbPMdata**] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint emowse_read_data(AjPFile inf, EmbPMdata** data)
{
    AjPStr str = NULL;
    double v;
    AjPStrTok token = NULL;
    EmbPMdata ptr   = NULL;
    AjPList l;
    ajint n;

    str = ajStrNew();
    l   = ajListNew();

    while(ajFileReadLine(inf,&str))
	if(sscanf(ajStrGetPtr(str),"%lf",&v)==1)
	{
	    AJNEW0(ptr);
	    ptr->mwt = v;
	    ptr->sdata=ajStrNew();
	    ajStrRemoveWhiteExcess(&str);
	    token = ajStrTokenNewC(str," \t\r\n");
	    ajStrTokenNextParseC(&token," \t\r\n",&ptr->sdata);
	    ajStrTokenNextParseC(&token," \t\r\n",&ptr->sdata);
	    ajStrTokenDel(&token);
	    ajListPush(l,(void *)ptr);
	}

    ajListSort(l,emowse_sort_data);
    n = ajListToarray(l,(void ***)data);
    ajListFree(&l);
    ajStrDel(&str);
    ajStrTokenDel(&token);

    return n;
}




/* @funcstatic emowse_sort_data ***********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint emowse_sort_data(const void *a, const void *b)
{
    return (ajint)((*(EmbPMdata const *)a)->mwt -
		   (*(EmbPMdata const *)b)->mwt);
}




/* @funcstatic emowse_hit_sort ************************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint emowse_hit_sort(const void *a, const void *b)
{
    double x;
    double y;

    x = (*(PHits const *)a)->score;
    y = (*(PHits const *)b)->score;

    if(x==y)
	return 0;
    else if(x<y)
	return -1;

    return 1;
}




/* @funcstatic emowse_match ***************************************************
**
** Undocumented.
**
** @param [r] data [EmbPMdata const *] Undocumented
** @param [r] dno [ajint] Undocumented
** @param [u] flist [AjPList] Undocumented
** @param [r] nfrags [ajint] Undocumented
** @param [r] tol [double] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [u] hlist [AjPList] Undocumented
** @param [r] partials [double] Undocumented
** @param [r] cmw [double] Undocumented
** @param [r] rno [ajint] Undocumented
** @param [r] freqs [const AjPDouble] Undocumented
** @@
******************************************************************************/

static void emowse_match(EmbPMdata const * data, ajint dno, AjPList flist,
			 ajint nfrags, double tol,
			 const AjPSeq seq, AjPList hlist,
			 double partials, double cmw, ajint rno,
			 const AjPDouble freqs)
{
    double actmw;
    double minmw;
    double maxmw;
    double smw;
    ajint ft;
    double qtol;
    double f;
    double sumf;
    double bestmw = 0.;
    static double min = (double)0.;
    static ajint n = 0;

    ajint i;
    ajint j;
    ajint nd;

    ajint x;
    ajint myindex;
    AjBool ispart = ajFalse;
    ajint isumf;
    AjPInt found = NULL;
    PHits hits   = NULL;
    EmbPMolFrag *frags = NULL;


    ajListReverse(flist);
    ajListToarray(flist,(void ***)&frags);


    sumf = (double)1.;

    smw = cmw;


    cmw /= (double)10000.;
    if(cmw>(double)79.)
	cmw = (double)79.;


    found = ajIntNew();


    for(i=0;i<dno;++i)
    {
	actmw = data[i]->mwt;
	qtol  = actmw / (double)100.;
	minmw = actmw - (tol*qtol);
	if(minmw<(double)0.)
	    minmw = (double)0.;
	maxmw = actmw + (tol*qtol);

	x = emowse_get_index(actmw,maxmw,minmw,frags,nfrags,&bestmw,&myindex,
			     i,seq,data);

	if(bestmw > MILLION)
	{
	    bestmw -= MILLION;
	    ispart = ajTrue;
	}
	else
	    ispart = ajFalse;

	if(x == -2)
	{
	    for(j=0;j<dno;++j)
		ajIntPut(&found,j,-1);
	    break;
	}


	if(x<0)
	{
	    ajIntPut(&found,i,-1);
	    continue;
	}
	ajIntPut(&found,i,x);

	if(ispart)
	    bestmw *= partials;

	if(ispart && bestmw < (double)101.)
	    f = (double).99;
	else
	{
	    f = bestmw / (double)100.;
	    if((ajint)f >= 200)
		f = (double) 199.;

	    ft = (rno-1)*16000 + (ajint)cmw*200 + (ajint)f;
	    f  = ajDoubleGet(freqs,ft);
	    if(!f)
		continue;
	}

	sumf *= f;
	if(sumf < (double)1.0e-31)
	    sumf = (double)1.0e-31;
    }


    isumf = (ajint)sumf;

    sumf = (double)1. / sumf;
    sumf *= (double)50000. / (cmw*(double)10000.);


    if(n<MAXLIST && isumf!=1)
    {
	AJNEW0(hits);
	hits->seq   = ajSeqGetSeqCopyS(seq);
	hits->desc  = ajStrNewC(ajStrGetPtr(ajSeqGetDescS(seq)));
	hits->found = found;
	hits->score = sumf;
	hits->mwt   = smw;
	hits->name  = ajStrNewC(ajSeqGetNameC(seq));
	hits->frags = frags;
	hits->nf = nfrags;
	ajListPush(hlist,(void *)hits);
	ajListSort(hlist,emowse_hit_sort);
	ajListPop(hlist,(void **)&hits);
	min = hits->score;
	ajListPush(hlist,(void *)hits);
	++n;
    }
    else if(sumf>min && isumf!=1)
    {
	ajListPop(hlist,(void **)&hits);
	ajStrDel(&hits->seq);
	ajStrDel(&hits->name);
	ajStrDel(&hits->desc);
	ajIntDel(&hits->found);
        nd = hits->nf;
	for(i=0;i<nd;++i)
	    AJFREE(hits->frags[i]);

	AJFREE(hits->frags);

	AJFREE(hits);

	AJNEW0(hits);
	hits->seq   = ajSeqGetSeqCopyS(seq);
	hits->desc  = ajStrNewC(ajStrGetPtr(ajSeqGetDescS(seq)));
	hits->found = found;
	hits->name  = ajStrNewC(ajSeqGetNameC(seq));
	hits->score = sumf;
	hits->mwt   = smw;
	hits->frags = frags;
        hits->nf    = nfrags;

	ajListPush(hlist,(void *)hits);
	ajListSort(hlist,emowse_hit_sort);
	ajListPop(hlist,(void **)&hits);
	min = hits->score;
	ajListPush(hlist,(void *)hits);
    }
    else
    {
	ajIntDel(&found);
	for(i=0;i<nfrags;++i)
	    AJFREE(frags[i]);
	AJFREE(frags);
    }

    return;
}




/* @funcstatic emowse_get_index ***********************************************
**
** Undocumented.
**
** @param [r] actmw [double] Undocumented
** @param [r] maxmw [double] Undocumented
** @param [r] minmw [double] Undocumented
** @param [r] frags [EmbPMolFrag const *] Undocumented
** @param [r] fno [ajint] Undocumented
** @param [w] bestmw [double*] Undocumented
** @param [w] myindex [ajint*] Undocumented
** @param [r] thys [ajint] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] data [EmbPMdata const *] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint emowse_get_index(double actmw, double maxmw, double minmw,
			      EmbPMolFrag const *frags,
			      ajint fno, double *bestmw,
			      ajint *myindex, ajint thys, const AjPSeq seq,
			      EmbPMdata const *data)
{
    double mw1;
    double mw2;
    double best;
    ajint cnt;
    ajint fl;
    ajint bidx = 0;


    cnt = fl = 0;
    best = (double)-1.0;

    /*
    **  This assumes the lowest array entry has been sorted to be the highest
    **  mw
    */

    while(cnt<fno)
    {
	mw1 = frags[cnt]->mwt;

	if(mw1>MILLION || mw1>maxmw)
	{
	    ++cnt;
	    continue;
	}

	if(mw1<minmw)
	    break;


	best = mw1;
	bidx = cnt;


	while(cnt+1<fno)
	{
	    mw2 = frags[cnt+1]->mwt;

	    if(mw2>MILLION)
	    {
		++cnt;
		continue;

	    }

	    if(abs((int)(mw2-actmw))>abs((int)(mw1-actmw)))
		break;
	    else
	    {
		mw1  = best = mw2;
		bidx = cnt;
		++cnt;
	    }
	}

	break;
    }

    fl = (cnt==fno) ? 1 : 0;

    if(best != (double)-1.)
    {
	if(!emowse_seq_comp(bidx,thys,seq,data,frags))
	    return -1;
	*bestmw = best;
	*myindex  = bidx;
	return bidx;
    }

    cnt = 0;
    while(cnt<fno)
    {
	mw1 = frags[cnt]->mwt;
	if(mw1<MILLION)
	{
	    ++cnt;
	    continue;
	}
	mw1 -= MILLION;

	if(mw1<minmw)
	    break;

	if(mw1>maxmw)
	{
	    ++cnt;
	    continue;
	}

	best = mw1;
	bidx = cnt;
	while(cnt+1>=fno)
	{
	    mw2 = frags[cnt+1]->mwt;

	    if(mw2<MILLION)
	    {
		++cnt;
		continue;
	    }
	    mw2 -= MILLION;

	    if(fabs(mw2-actmw)>fabs(mw1-actmw))
		break;
	    else
	    {
		mw1 = best = mw2;
		bidx = cnt++;
	    }
	}

	break;
    }

    if(cnt==fno && fl)
	return -2;

    if(best == (double)-1.)
	return -1;

    if(!emowse_seq_comp(bidx,thys,seq,data,frags))
	return -1;

    *bestmw = best + MILLION;
    *myindex  = bidx + (ajint)MILLION;

    return *myindex;
}




/* @funcstatic emowse_seq_comp ************************************************
**
** Undocumented.
**
** @param [r] bidx [ajint] Undocumented
** @param [r] thys [ajint] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] data [EmbPMdata const *] Undocumented
** @param [r] frags [EmbPMolFrag const *] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint emowse_seq_comp(ajint bidx, ajint thys, const AjPSeq seq,
			     EmbPMdata const *data, EmbPMolFrag const *frags)
{
    ajint beg;
    ajint end;
    ajint len;
    AjPStr result   = NULL;
    const AjPStr str      = NULL;
    AjPStr substr   = NULL;
    AjPStrTok token = NULL;
    const char *p;


    if(!ajStrGetLen(data[thys]->sdata))
	return 1;

    beg = frags[bidx]->begin - 1;
    end = frags[bidx]->end   - 1;

    str = ajSeqGetSeqS(seq);

    result = ajStrNew();
    substr = ajStrNew();
    ajStrAssignSubS(&substr,str,beg,end);
    ajStrFmtUpper(&substr);

    token = ajStrTokenNewC(data[thys]->sdata," \r\t\n");
    
    while(ajStrTokenNextParseC(&token," \r\t\n",&result))
    {
	len = ajStrGetLen(result);
	ajStrFmtUpper(&result);
	p = ajStrGetPtr(result);

	if(p[len-1]!=')')
	    ajFatal("Missing ')' in subline %S",substr);

	if(ajStrPrefixC(result,"SEQ("))
	{
	    ajStrAssignC(&result,p+4);
	    ajStrPasteCountK(&result,5,'\0',1);
	    if(!emowse_seq_search(substr,&result))
		return 0;
	}
	else if(ajStrPrefixC(result,"COMP("))
	{
	    ajStrAssignC(&result,p+5);
	    ajStrPasteCountK(&result, 5, '\0', 1);
	    if(!emowse_comp_search(substr,ajStrGetPtr(result)))
		return 0;
	}
	else
	    ajFatal("Unknown Query type [%S]",result);

    }
    ajStrTokenDel(&token);


    ajStrDel(&substr);
    ajStrDel(&result);

    return 1;
}




/* @funcstatic emowse_mreverse ************************************************
**
** Undocumented.
**
** @param [u] s [char*] Undocumented
** @@
******************************************************************************/

static void emowse_mreverse(char *s)
{
    ajint i;
    ajint len;
    char *p;
    AjPStr rev;
    size_t stlen;
    
    rev = ajStrNewC(s);
    ajStrReverse(&rev);
    p = ajStrGetuniquePtr(&rev);

    stlen = strlen(s);
    len   = (ajint) stlen;
    
    for(i=0;i<len;++i)
    {
	if(p[i]==']')
	{
	    p[i]='[';
	    continue;
	}

	if(p[i]=='[')
	    p[i]=']';
    }

    strcpy(s,ajStrGetPtr(rev));

    ajStrDel(&rev);

    return;
}




/* @funcstatic emowse_seq_search **********************************************
**
** Undocumented.
**
** @param [r] substr [const AjPStr] Undocumented
** @param [u] str [AjPStr*] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool emowse_seq_search(const AjPStr substr, AjPStr *str)
{
    const char *p;
    char *q;
    char* s;
    static AjPStr t = NULL;

    if(!t)
	t = ajStrNew();

    p = ajStrGetPtr(substr);
    ajStrAssignC(&t,p);
    q = ajStrGetuniquePtr(&t);
    s = ajStrGetuniquePtr(str);

    if(!strncmp(s,"B-",2))
    {
	if(!emowse_msearch(q,s+2,ajFalse))
	    return ajFalse;
    }
    else if(!strncmp(s,"N-",2))
    {
	if(!emowse_msearch(q,s+2,ajTrue))
	    return ajFalse;
    }
    else if(!strncmp(s,"C-",2))
    {
	emowse_mreverse(s+2);
	emowse_mreverse(q);

	if(!emowse_msearch(q,s+2,ajTrue))
	    return ajFalse;
    }
    else if(!strncmp(s,"*-",2))
    {
	if(!emowse_msearch(q,s+2,ajFalse))
	    return ajTrue;
	emowse_mreverse(s+2);

	if(!emowse_msearch(q,s+2,ajFalse))
	    return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic emowse_msearch *************************************************
**
** Undocumented.
**
** @param [r] seq [const char*] Undocumented
** @param [r] pat [const char*] Undocumented
** @param [r] term [AjBool] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool emowse_msearch(const char *seq, const char *pat, AjBool term)
{
    AjPStr orc = NULL;

    ajint qpos;
    ajint fpos;

    AjBool fl;
    ajint i;
    ajint t;
    ajint ofpos;
    const char *p;

    qpos = ofpos = fpos = 0;
    fl = ajFalse;


    orc = ajStrNew();

    while(pat[qpos])
    {
	if((term && fl) || !seq[fpos])
	{
	    ajStrDel(&orc);
	    return ajFalse;
	}


	if(pat[qpos]=='X')
	{
	    ++qpos;
	    ++fpos;
	    continue;
	}

	if(pat[qpos]=='[')
	{
	    ++qpos;
	    while(pat[qpos]!=']')
	    {
		if(!pat[qpos])
		    ajFatal("Missing ']' in term %s",pat);

		ajStrAppendK(&orc,pat[qpos++]);
	    }

	    t = ajStrGetLen(orc);
	    p = ajStrGetPtr(orc);
	    for(i=0;i<t;++i)
		if(p[i]==seq[fpos])
		    break;

	    if(t==i)
	    {
		fpos = ++ofpos;
		qpos=0;
		fl=ajTrue;
		continue;
	    }
	    else
	    {
		++fpos;
		++qpos;
		continue;
	    }
	}

	if(pat[qpos]==seq[fpos])
	{
	    ++fpos;
	    ++qpos;
	    continue;
	}
	else
	{
	    fpos = ++ofpos;
	    qpos = 0;
	    fl   = ajTrue;
	}
    }


    ajStrDel(&orc);

    return ajTrue;
}




/* @funcstatic emowse_comp_search *********************************************
**
** Undocumented.
**
** @param [r] substr [const AjPStr] Undocumented
** @param [r] s [const char*] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool emowse_comp_search(const AjPStr substr, const char *s)
{
    AjPInt arr;
    ajint i;
    ajint len;
    ajint v;
    ajint n;
    ajint w;

    ajint qpos;
    char c;
    AjPStr orc;
    const char *r;


    const char *p;
    AjPStr t;

    p   = ajStrGetPtr(substr);
    len = ajStrGetLen(substr);

    arr = ajIntNewL(256);

    for(i=0;i<256;++i)
	ajIntPut(&arr,i,0);

    for(i=0;i<len;++i)
    {
	v = ajIntGet(arr,(ajint)p[i]);
	ajIntPut(&arr,(ajint)p[i],v+1);
    }


    t = ajStrNewC(s);
    ajStrRemoveWhite(&t);

    p    = ajStrGetPtr(t);
    qpos = 0;
    orc  = ajStrNew();

    while((c=p[qpos]))
    {
	if(c=='*')
	{
	    n = emowse_get_orc(&orc,p,qpos);
	    r = ajStrGetPtr(orc);
	    qpos += (n+3);
	    for(i=0;i<n;++i)
		if(ajIntGet(arr,r[i]))
		    break;

	    if(i==n)
	    {
		ajStrDel(&t);
		ajStrDel(&orc);
		ajIntDel(&arr);
		return ajFalse;
	    }

	    continue;
	}

	i = qpos;
	while((c=p[i])>='0' && c<='9')
	    ++i;

	if(i==qpos || p[i]!='[')
	    ajFatal("Bad integer [%s]",p);

	if(sscanf(p+qpos,"%d",&v)!=1)
	    ajFatal("Bad integer [%s]",p);

	qpos = --i;
	ajStrSetClear(&orc);
	n = emowse_get_orc(&orc,p,qpos);
	r = ajStrGetPtr(orc);
	qpos += (n+3);
	w = 0;

	for(i=0;i<n;++i)
	    w += ajIntGet(arr,(ajint)r[i]);

	if(w!=v)
	{
	    ajStrDel(&t);
	    ajStrDel(&orc);
	    ajIntDel(&arr);
	    return ajFalse;
	}
    }


    ajStrDel(&orc);
    ajStrDel(&t);
    ajIntDel(&arr);

    return ajTrue;
}




/* @funcstatic emowse_get_orc *************************************************
**
** Undocumented.
**
** @param [u] orc [AjPStr*] Undocumented
** @param [r] s [const char*] Undocumented
** @param [r] pos [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint emowse_get_orc(AjPStr *orc, const char *s, ajint pos)
{
    ajint i;

    i = 0;
    if(s[++pos]!='[')
	ajFatal("Bad query given [%s]",s);
    ++pos;

    while(s[pos]!=']')
    {
	if(!s[pos])
	    ajFatal("Unterminated square brackets [%s]",s);
	ajStrAppendK(orc,s[pos++]);
	++i;
    }

    return i;
}




/* @funcstatic emowse_print_hits **********************************************
**
** Undocumented.
**
** @param [u] outf [AjPFile] Undocumented
** @param [u] hlist [AjPList] Undocumented
** @param [r] dno [ajint] Undocumented
** @param [r] data [EmbPMdata const *] Undocumented
** @@
******************************************************************************/

static void emowse_print_hits(AjPFile outf, AjPList hlist, ajint dno,
			      EmbPMdata const * data)
{
    PHits hits   = NULL;
    AjIList iter = NULL;
    ajint c;
    ajint i;
    ajint j;
    ajint pvt;
    double conf;
    AjBool partial;
    AjPFloat nmarray = NULL;
    ajint nmn;
    ajint len;
    ajint v;
    AjPStr substr = NULL;


    nmarray = ajFloatNew();
    substr  = ajStrNew();

    ajListReverse(hlist);


    ajFmtPrintF(outf,"\nUsing data fragments of:\n");

    for(i=0;i<dno;++i)
	ajFmtPrintF(outf,"         %7.1f  %S\n",data[i]->mwt,data[i]->sdata);

    ajFmtPrintF(outf,"\n");

    iter = ajListIterNewread(hlist);
    c    = 0;
    while(!ajListIterDone(iter))
    {
	hits = (PHits) ajListIterGet(iter);
	ajFmtPrintF(outf,"%-3d %-13S%-62.62S\n",++c,hits->name,hits->desc);
    }
    ajListIterDel(&iter);


    iter = ajListIterNewread(hlist);
    c    = 0;
    while(!ajListIterDone(iter))
    {
	hits = (PHits) ajListIterGet(iter);

	for(i=pvt=0;i<dno;++i)
	    if(ajIntGet(hits->found,i) > -1)
		++pvt;
	conf = (double)pvt / (double)dno;


	ajFmtPrintF(outf,"\n    %-3d: %-12S   %.3e %-8.1f   %-6.3f\n",++c,
		    hits->name,hits->score,hits->mwt,conf);
	ajFmtPrintF(outf,"         %S\n",hits->desc);
	ajFmtPrintF(outf,"         Mw     Start  End    Seq\n");

	for(i=nmn=0;i<dno;++i)
	{
	    partial = (ajIntGet(hits->found,i)>1000000) ? ajTrue : ajFalse;

	    if((v=ajIntGet(hits->found,i))>-1)
	    {
		if(v>=(ajint)MILLION)
		    v -= (ajint)MILLION;

		ajStrAssignSubC(&substr,ajStrGetPtr(hits->seq),
			     hits->frags[v]->begin-1,
			     hits->frags[v]->end-1);

		len = ajStrGetLen(substr);
		ajFmtPrintF(outf,"        ");

		if(partial)
		    ajFmtPrintF(outf,"*");
		else
		    ajFmtPrintF(outf," ");

		if(hits->frags[v]->mwt > MILLION)
		    hits->frags[v]->mwt -= MILLION;

		ajFmtPrintF(outf,"%-6.1f %-6d %-6d %-45.45S",
			    hits->frags[v]->mwt,hits->frags[v]->begin,
			    hits->frags[v]->end,substr);
		if(len>45)
		    ajFmtPrintF(outf,"...");
		ajFmtPrintF(outf,"\n");
	    }
	    else
		ajFloatPut(&nmarray,nmn++,(float)data[i]->mwt);
	}
	if(nmn)
	{
	    ajFmtPrintF(outf,"         No Match      ");
	    for(i=0,j=0;i<nmn;++i,++j)
	    {
		ajFmtPrintF(outf,"%-6.1f ",ajFloatGet(nmarray,i));

		if(j==6 && i!=nmn-1)
		{
		    ajFmtPrintF(outf,"\n                       ");
		    j = 0;
		}
	    }
	    ajFmtPrintF(outf,"\n");
	}

	ajStrDel(&hits->name);
	ajStrDel(&hits->seq);
	ajStrDel(&hits->desc);
	ajIntDel(&hits->found);
	len = hits->nf;

	for(i=0;i<len;++i)
	    AJFREE(hits->frags[i]);
	AJFREE(hits->frags);
	AJFREE(hits);
    }

    ajListIterDel(&iter);
    ajStrDel(&substr);
    ajFloatDel(&nmarray);


    return;
}
