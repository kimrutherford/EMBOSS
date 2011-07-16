/* @source tcode application
**
** Fickett TESTCODE statistic
**
** @author Copyright (C) Alan Bleasby (ableasby@embnet.org)
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


#define BIGVAL 10.




/* @datastatic PTestcode ****************************************************
**
** tcode internals
**
** @alias Stestcode
** @alias OTestcode
**
** @attr positions [AjPFloat] Undocumented
** @attr content [AjPFloat] Undocumented
** @attr npositions [ajint] Undocumented
** @attr ncontent [ajint] Undocumented
** @attr pprobA [AjPFloat] Undocumented
** @attr pprobC [AjPFloat] Undocumented
** @attr pprobG [AjPFloat] Undocumented
** @attr pprobT [AjPFloat] Undocumented
** @attr cprobA [AjPFloat] Undocumented
** @attr cprobC [AjPFloat] Undocumented
** @attr cprobG [AjPFloat] Undocumented
** @attr cprobT [AjPFloat] Undocumented
** @attr pweights [AjPFloat] Undocumented
** @attr cweights [AjPFloat] Undocumented
******************************************************************************/

typedef struct AjSTestcode 
{
    AjPFloat positions;
    AjPFloat content;
    ajint    npositions;
    ajint    ncontent;
    AjPFloat pprobA;
    AjPFloat pprobC;
    AjPFloat pprobG;
    AjPFloat pprobT;
    AjPFloat cprobA;
    AjPFloat cprobC;
    AjPFloat cprobG;
    AjPFloat cprobT;
    AjPFloat pweights;
    AjPFloat cweights;
} OTestcode;
#define PTestcode OTestcode*




static AjBool tcode_readdata(PTestcode *table1, AjPFile datafile);
static PTestcode tcode_new(void);
static void tcode_del(PTestcode *thys);
static float tcode_slide(const AjPStr substr, ajint window,
			 const PTestcode tables,
			 ajint pos);
static ajint tcode_index(const AjPFloat array, float value);
static void tcode_report(AjPReport report, const AjPInt from, const AjPInt to,
			 const AjPFloat testcodes, ajint npoints,
			 AjPFeattable ftable, const AjPSeq seq);




/* @prog tcode ***************************************************************
**
** Ficket TESTCODE statistic
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall    = NULL;
    AjPSeq seq          = NULL;
    AjPReport report    = NULL;
    AjPFeattable ftable = NULL;
    AjPFile datafile    = NULL;
    PTestcode table1  = NULL;
    const AjPStr seqstr       = NULL;
    AjPStr substr	= NULL;
    AjBool plot         = ajFalse;
    
    ajint window;
    ajint pos;
    ajint len;
    ajint step;
    
    float testcode = 0.;
    ajint npoints  = 0;
    
    AjPFloat testcodes = NULL;
    AjPInt   from      = NULL;
    AjPInt   to        = NULL;

    AjPGraph graph     = NULL;
    AjPGraphdata this  = NULL;

    ajint i;
    float ymin = 0.;
    float ymax = 0.;
    
    AjPStr title = NULL;

    embInit("tcode", argc, argv);

    report   = ajAcdGetReport("outfile");
    seqall   = ajAcdGetSeqall("sequence");
    window   = ajAcdGetInt("window");
    datafile = ajAcdGetDatafile("datafile");
    plot     = ajAcdGetToggle("plot");
    step     = ajAcdGetInt("step");
    
    graph = ajAcdGetGraphxy("graph");

    title = ajStrNewS(ajGraphGetTitleS(graph));

    testcodes = ajFloatNew();
    from      = ajIntNew();
    to        = ajIntNew();



    table1 = tcode_new();
    substr = ajStrNew();
    
    if(!tcode_readdata(&table1,datafile))
	ajFatal("Incorrect format Testcode data file");
    
    while(ajSeqallNext(seqall,&seq))
    {
	ajSeqTrim(seq);
	if(!ftable)
	    ftable = ajFeattableNewSeq(seq);
	

	ajSeqFmtUpper(seq);
	seqstr = ajSeqGetSeqS(seq);
	len    = ajStrGetLen(seqstr);


	ajFloatPut(&testcodes,len-window+1,0.);
	ajIntPut(&from,len-window+1,0);
	ajIntPut(&to,len-window+1,0);

	
	pos = 0;
	npoints = 0;
	
	while(pos+window < len)
	{
	    testcode = tcode_slide(seqstr,window,table1,pos);

	    ajIntPut(&from,npoints,pos+1+ajSeqGetOffset(seq));
	    ajIntPut(&to,npoints,pos+window+ajSeqGetOffset(seq));
	    ajFloatPut(&testcodes,npoints,testcode);
	    
	    pos += step;
	    ++npoints;
	}

        ajDebug("npoints: %d", npoints);

	if(report)
	    tcode_report(report, from, to, testcodes, npoints, ftable, seq);
	if(plot)
	{
	    this = ajGraphdataNewI(npoints);
	    ajGraphdataSetTypeC(this,"2D plot");

	    for(i=0;i<npoints;++i)
	    {
		this->x[i] = (float)((ajIntGet(to,i) + ajIntGet(from,i))
				    / 2);
		this->y[i] = ajFloatGet(testcodes,i);
	    }

	    ajGraphicsCalcRange(this->y,npoints,&ymin,&ymax);

	    ajGraphdataSetTruescale(this,this->x[0],this->x[npoints-1],
				   (float)0.,(float)1.37);

	    ajGraphdataSetYlabelC(this,"TESTCODE value");
	    ajGraphdataSetXlabelC(this,"Sequence mid position");

	    ajGraphdataAddposLine(this,this->x[0],(float)0.74,
				  this->x[npoints-1],
				  (float)0.74,
                                  ajGraphicsCheckColourC("RED"));
	    ajGraphdataAddposLine(this,this->x[0],(float)0.95,
				  this->x[npoints-1],
				  (float)0.95,
                                  ajGraphicsCheckColourC("GREEN"));

	    ajGraphxySetflagOverlay(graph,ajFalse);
            ajGraphShowTitle(graph, ajTrue);
	    ajGraphSetTitleS(graph, title);
	    ajGraphAppendTitleS(graph, ajSeqGetUsaS(seq));

	    ajGraphDataReplace(graph,this);

	    ajGraphxyDisplay(graph,ajFalse);
	}
	
	ajFeattableClear(ftable);
    }

    if(report)
    {
        ajReportSetSeqstats(report, seqall);
        ajReportClose(report);
        ajReportDel(&report);
    }

    tcode_del(&table1);

    ajFloatDel(&testcodes);
    ajIntDel(&from);
    ajIntDel(&to);
    ajStrDel(&substr);
    ajStrDel(&title);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajFileClose(&datafile);
    ajFeattableDel(&ftable);

    ajGraphicsClose();
    ajGraphxyDel(&graph);

    embExit();

    return 0;
}




/* @funcstatic tcode_readdata ********************************************
**
** Read Etcode.dat data file
**
** @param [w] table1 [PTestcode*] data object
** @param [u] datafile [AjPFile] data file object 
** @return [AjBool] true if successful read
** @@
******************************************************************************/

static AjBool tcode_readdata(PTestcode *table1, AjPFile datafile)
{
    PTestcode table = NULL;
    AjPStr line  = NULL;
    AjBool ok    = ajTrue;
    float  val   = 0.;
    float  vala  = 0.;
    float  valc  = 0.;
    float  valg  = 0.;
    float  valt  = 0.;
    char   c     = '\0';
    ajint  i     = 0;


    line  = ajStrNew();
    table = *table1;
    

    ok  = ajTrue;
    val = BIGVAL;

    while(val && ok)
    {
	ok = ajReadlineTrim(datafile,&line);
	c  = *ajStrGetPtr(line);

	if(c=='#' || c=='\n')
	    continue;

	ajFmtScanS(line,"%f",&val);
	ajFloatPut(&table->positions,table->npositions++,val);
    }

    ok  = ajTrue;
    val = BIGVAL;

    while(val && ok)
    {
	ok = ajReadlineTrim(datafile,&line);
	c = *ajStrGetPtr(line);

	if(c=='#' || c=='\n')
	    continue;

	ajFmtScanS(line,"%f",&val);
	ajFloatPut(&table->content,table->ncontent++,val);
    }
    
    i  = 0;
    ok = ajTrue;
    while(ok && i != table->npositions)
    {
	ok = ajReadlineTrim(datafile,&line);
	c = *ajStrGetPtr(line);

	if(c=='#' || c=='\n')
	    continue;

	ajFmtScanS(line,"%f%f%f%f",&vala,&valc,&valg,&valt);

	ajFloatPut(&table->pprobA,i,vala);
	ajFloatPut(&table->pprobC,i,valc);
	ajFloatPut(&table->pprobG,i,valg);
	ajFloatPut(&table->pprobT,i,valt);

	++i;
    }
    
    i  = 0;
    ok = ajTrue;
    while(ok && i != table->ncontent)
    {
	ok = ajReadlineTrim(datafile,&line);
	c = *ajStrGetPtr(line);

	if(c=='#' || c=='\n')
	    continue;

	ajFmtScanS(line,"%f%f%f%f",&vala,&valc,&valg,&valt);

	ajFloatPut(&table->cprobA,i,vala);
	ajFloatPut(&table->cprobC,i,valc);
	ajFloatPut(&table->cprobG,i,valg);
	ajFloatPut(&table->cprobT,i,valt);

	++i;
    }

    i  = 0;
    ok = ajTrue;
    while(ok && i != 4)
    {
	ok = ajReadlineTrim(datafile,&line);
	c = *ajStrGetPtr(line);

	if(c=='#' || c=='\n')
	    continue;

	ajFmtScanS(line,"%f",&val);

	ajFloatPut(&table->pweights,i,val);
	++i;
    }

    i  = 0;
    ok = ajTrue;
    while(ok && i != 4)
    {
	ok = ajReadlineTrim(datafile,&line);
	c = *ajStrGetPtr(line);

	if(c=='#' || c=='\n')
	    continue;

	ajFmtScanS(line,"%f",&val);

	ajFloatPut(&table->cweights,i,val);

	++i;
    }

    ajStrDel(&line);

    return ok;
}




/* @funcstatic tcode_new ********************************************
**
** Testcode data object constructor
**
** @return [PTestcode] allocated object
** @@
******************************************************************************/

static PTestcode tcode_new(void)
{
    PTestcode ret = NULL;

    AJNEW0(ret);
    
    ret->positions = ajFloatNew();
    ret->content   = ajFloatNew();
    
    ret->pprobA    = ajFloatNew();
    ret->pprobC    = ajFloatNew();
    ret->pprobG    = ajFloatNew();
    ret->pprobT    = ajFloatNew();

    ret->cprobA    = ajFloatNew();
    ret->cprobC    = ajFloatNew();
    ret->cprobG    = ajFloatNew();
    ret->cprobT    = ajFloatNew();

    ret->pweights  = ajFloatNew();
    ret->cweights  = ajFloatNew();
    
    return ret;
}




/* @funcstatic tcode_del ********************************************
**
** Testcode data object destructor
**
** @param [w] thys [PTestcode*] testcodedata object
** @return [void]
** @@
******************************************************************************/


static void tcode_del(PTestcode *thys)
{
    PTestcode pthis = NULL;

    pthis = *thys;

    ajFloatDel(&pthis->positions);
    ajFloatDel(&pthis->content);
    

    ajFloatDel(&pthis->pprobA);
    ajFloatDel(&pthis->pprobC);
    ajFloatDel(&pthis->pprobG);
    ajFloatDel(&pthis->pprobT);

    ajFloatDel(&pthis->cprobA);
    ajFloatDel(&pthis->cprobC);
    ajFloatDel(&pthis->cprobG);
    ajFloatDel(&pthis->cprobT);

    ajFloatDel(&pthis->pweights);
    ajFloatDel(&pthis->cweights);


    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @funcstatic tcode_slide ********************************************
**
** Return a single TESTCODE value
**
** @param [r] substr [const AjPStr] sequence
** @param [r] window [ajint] size of sliding window
** @param [r] table [const PTestcode] testcode data object
** @param [r] pos [ajint] start position within sequence
**
** @return [float] Testcode value
** @@
******************************************************************************/

static float tcode_slide(const AjPStr substr, ajint window,
			 const PTestcode table,
			 ajint pos)
{
    ajint asum = 0;
    ajint csum = 0;
    ajint gsum = 0;
    ajint tsum = 0;
    ajint sum  = 0;
    
    float acomp = 0.;
    float ccomp = 0.;
    float gcomp = 0.;
    float tcomp = 0.;
    
    float apos = 0.;
    float cpos = 0.;
    float gpos = 0.;
    float tpos = 0.;

    ajint idx = 0;

    float p1 = 0.;
    float p2 = 0.;
    float p3 = 0.;
    float p4 = 0.;
    float p5 = 0.;
    float p6 = 0.;
    float p7 = 0.;
    float p8 = 0.;
    
    
    const char  *p = NULL;
    char  c;
    
    ajint scores[3][4];
    
    
    ajint i;
    ajint j;
    
    ajint vmax = 0;
    ajint vmin = 0;
    
    float testcode = 0.;
    


    for(i=0;i<3;++i)
	for(j=0;j<4;++j)
	    scores[i][j] = 0;

    p = ajStrGetPtr(substr);

    for(i=0;i<window;++i)
    {
	c = p[i+pos];
	switch(c)
	{
	case 'A':
	    ++scores[i%3][0];
	    break;
	case 'C':
	    ++scores[i%3][1];
	    break;
	case 'G':
	    ++scores[i%3][2];
            break;
        case 'T':
	    ++scores[i%3][3];
	default:
	    break;
	}
    }
    
    asum = scores[0][0] + scores[1][0] + scores[2][0];
    csum = scores[0][1] + scores[1][1] + scores[2][1];
    gsum = scores[0][2] + scores[1][2] + scores[2][2];
    tsum = scores[0][3] + scores[1][3] + scores[2][3];

    sum = asum + csum + gsum + tsum;

    acomp = (float) ((float)asum / (float)sum);
    ccomp = (float) ((float)csum / (float)sum);
    gcomp = (float) ((float)gsum / (float)sum);
    tcomp = (float) ((float)tsum / (float)sum);
    
    vmax = (scores[0][0] > scores[1][0]) ? scores[0][0] : scores[1][0];
    vmax = (vmax > scores[2][0]) ? vmax : scores[2][0];

    vmin = (scores[0][0] < scores[1][0]) ? scores[0][0] : scores[1][0];
    vmin = (vmin < scores[2][0]) ? vmin : scores[2][0];
    
    apos = (float) ((float)vmax / (float) ((float)vmin + 1.0));

    vmax = (scores[0][1] > scores[1][1]) ? scores[0][1] : scores[1][1];
    vmax = (vmax > scores[2][1]) ? vmax : scores[2][1];

    vmin = (scores[0][1] < scores[1][1]) ? scores[0][1] : scores[1][1];
    vmin = (vmin < scores[2][1]) ? vmin : scores[2][1];
    
    cpos = (float) ((float)vmax / (float) ((float)vmin + 1.0));

    vmax = (scores[0][2] > scores[1][2]) ? scores[0][2] : scores[1][2];
    vmax = (vmax > scores[2][2]) ? vmax : scores[2][2];

    vmin = (scores[0][2] < scores[1][2]) ? scores[0][2] : scores[1][2];
    vmin = (vmin < scores[2][2]) ? vmin : scores[2][2];
    
    gpos = (float) ((float)vmax / (float) ((float)vmin + 1.0));

    vmax = (scores[0][3] > scores[1][3]) ? scores[0][3] : scores[1][3];
    vmax = (vmax > scores[2][3]) ? vmax : scores[2][3];

    vmin = (scores[0][3] < scores[1][3]) ? scores[0][3] : scores[1][3];
    vmin = (vmin < scores[2][3]) ? vmin : scores[2][3];
    
    tpos = (float) ((float)vmax / (float) ((float)vmin + 1.0));
    

    idx = tcode_index(table->positions,apos);
    p1  = ajFloatGet(table->pprobA,idx) * ajFloatGet(table->pweights,0);

    idx = tcode_index(table->positions,cpos);
    p2  = ajFloatGet(table->pprobC,idx) * ajFloatGet(table->pweights,1);

    idx = tcode_index(table->positions,gpos);
    p3  = ajFloatGet(table->pprobG,idx) * ajFloatGet(table->pweights,2);

    idx = tcode_index(table->positions,tpos);
    p4  = ajFloatGet(table->pprobT,idx) * ajFloatGet(table->pweights,3);


    idx = tcode_index(table->content,acomp);
    p5  = ajFloatGet(table->cprobA,idx) * ajFloatGet(table->cweights,0);

    idx = tcode_index(table->content,ccomp);
    p6  = ajFloatGet(table->cprobC,idx) * ajFloatGet(table->cweights,1);

    idx = tcode_index(table->content,gcomp);
    p7  = ajFloatGet(table->cprobG,idx) * ajFloatGet(table->cweights,2);

    idx = tcode_index(table->content,tcomp);
    p8  = ajFloatGet(table->cprobT,idx) * ajFloatGet(table->cweights,3);


    testcode = p1+p2+p3+p4+p5+p6+p7+p8;

    return testcode;
}




/* @funcstatic tcode_index ********************************************
**
** Return an index into a TESTCODE data object probability array
**
** @param [r] array [const AjPFloat] probability array
** @param [r] value [float] value to return index for
**
** @return [ajint] index
** @@
******************************************************************************/

static ajint tcode_index(const AjPFloat array, float value)
{
    ajint i = 0;
    float thisval = 0.;
    AjBool found  = ajFalse;

    while(!found)
    {
	thisval = ajFloatGet(array,i);

	if(value >= thisval)
	{
	    found = ajTrue;
	    continue;
	}
	++i;
    }

    return i;
}




/* @funcstatic tcode_report ********************************************
**
** Print TESTCODE results
**
** @param [u] report [AjPReport] report object
** @param [r] from [const AjPInt] window start positions
** @param [r] to [const AjPInt] window end positions
** @param [r] testcodes [const AjPFloat] testcode values for windows
** @param [r] npoints [ajint] number of data points
** @param [u] ftable [AjPFeattable] feature table for loading report
** @param [r] seq [const AjPSeq] original sequence
**
** @return [void]
** @@
******************************************************************************/

static void tcode_report(AjPReport report, const AjPInt from, const AjPInt to,
			 const AjPFloat testcodes, ajint npoints,
			 AjPFeattable ftable, const AjPSeq seq)
{
    AjPFeature feat = NULL;
    AjPStr tmpstr   = NULL;
    ajint i;
    float fval = 0.;
    AjPStr coding = NULL;

    AjPStr source = NULL;
    AjPStr type   = NULL;
    char   strand = '.';

    tmpstr = ajStrNew();
    coding = ajStrNew();
    type   = ajStrNewC("misc_feature");
    source = ajStrNew();
    
    ajFmtPrintS(&tmpstr,"Fickett TESTCODE statistic");
    ajReportSetHeaderS(report,tmpstr);


    for(i=0;i<npoints;++i)
    {
	feat = ajFeatNew(ftable,source,type,ajIntGet(from,i),ajIntGet(to,i),
			 (fval=ajFloatGet(testcodes,i)),strand,0);
	if(fval < 0.74)
	    ajFmtPrintS(&coding,"*Estimation Non-coding");
	else if(fval >= 0.95)
	    ajFmtPrintS(&coding,"*Estimation Coding");
	else
	    ajFmtPrintS(&coding,"*Estimation No opinion");
	
	ajFeatTagAdd(feat,NULL,coding);
    }


    ajReportWrite(report,ftable,seq);

    ajStrDel(&tmpstr);
    ajStrDel(&coding);
    ajStrDel(&source);
    ajStrDel(&type);
    
    return;
}
