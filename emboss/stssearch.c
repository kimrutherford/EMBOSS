/* @source stssearch application
**
** Gribskov statistical plot of synonymous codon usage
**
** @author Unknown
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




/* @datastatic Primer *********************************************************
**
** stssearch internals
**
** @alias primers
**
** @attr Name [AjPStr] Undocumented
** @attr Prima [AjPRegexp] Undocumented
** @attr Primb [AjPRegexp] Undocumented
** @attr Oligoa [AjPStr] Undocumented
** @attr Oligob [AjPStr] Undocumented
******************************************************************************/

typedef struct primers
{
    AjPStr Name;
    AjPRegexp Prima;
    AjPRegexp Primb;
    AjPStr Oligoa;
    AjPStr Oligob;
} *Primer;




AjPFile out    = NULL;
AjPSeq seq     = NULL;
AjPStr seqstr  = NULL;
AjPStr revstr  = NULL;
ajint nprimers = 0;
ajint ntests   = 0;




static void stssearch_primDel(void **x,  void *cl);
static void stssearch_primTest(void **x,void *cl);




/* @prog stssearch ************************************************************
**
** Searches a DNA database for matches with a set of STS primers
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPFile primfile;
    AjPStr rdline = NULL;

    Primer primdata;
    AjPStrTok handle = NULL;

    AjPList primList = NULL;

    embInit("stssearch", argc, argv);

    primfile = ajAcdGetInfile("infile");
    out      = ajAcdGetOutfile("outfile");
    seqall   = ajAcdGetSeqall("seqall");

    while(ajReadlineTrim(primfile, &rdline))
    {
	if(ajStrGetCharFirst(rdline) == '#')
	    continue;
	if(ajStrSuffixC(rdline, ".."))
	    continue;

	AJNEW(primdata);
	primdata->Name   = NULL;
	primdata->Oligoa = NULL;
	primdata->Oligob = NULL;

	handle = ajStrTokenNewC(rdline, " \t");
	ajStrTokenNextParse(&handle, &primdata->Name);

	if(!(nprimers % 1000))
	    ajDebug("Name [%d]: '%S'\n", nprimers, primdata->Name);

	ajStrTokenNextParse(&handle, &primdata->Oligoa);
	ajStrFmtUpper(&primdata->Oligoa);
	primdata->Prima = ajRegComp(primdata->Oligoa);

	ajStrTokenNextParse(&handle, &primdata->Oligob);
	ajStrFmtUpper(&primdata->Oligob);
	primdata->Primb = ajRegComp(primdata->Oligob);
	ajStrTokenDel(&handle);

	if(!nprimers)
	    primList = ajListNew();

	ajListPushAppend(primList, primdata);
	nprimers++;
    }

    if(!nprimers)
	ajFatal("No primers read\n");

    ajDebug("%d primers read\n", nprimers);

    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqFmtUpper(seq);
	ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
	ajStrAssignS(&revstr, ajSeqGetSeqS(seq));
	ajSeqstrReverse(&revstr);
	ajDebug("Testing: %s\n", ajSeqGetNameC(seq));
	ntests = 0;
	ajListMap(primList, stssearch_primTest, NULL);
    }

    ajFileClose(&out);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajFileClose(&out);
    ajStrDel(&revstr);
    ajStrDel(&seqstr);
    ajFileClose(&primfile);
    ajListMap(primList, stssearch_primDel, NULL);
    ajListFree(&primList);
    ajStrDel(&rdline);


    embExit();

    return 0;
}


/* @funcstatic stssearch_primDel **********************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/


static void stssearch_primDel(void **x,  void *cl)
{
    Primer* p;
    Primer primdata;

   (void) cl;				/* make it used */

    p = (Primer*) x;
    primdata = *p;

    ajStrDel(&primdata->Name);
    ajRegFree(&primdata->Prima);
    ajRegFree(&primdata->Primb);
    ajStrDel(&primdata->Oligoa);
    ajStrDel(&primdata->Oligob);
    AJFREE(*p);
}


/* @funcstatic stssearch_primTest *********************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/


static void stssearch_primTest(void **x,void *cl)
{
    Primer* p;
    Primer primdata;

    AjBool testa;
    AjBool testb;
    AjBool testc;
    AjBool testd;
    ajint ioff;

    (void) cl;				/* make it used */

    p = (Primer*) x;
    primdata = *p;

    ntests++;

    if(!(ntests % 1000))
	ajDebug("completed tests: %d\n", ntests);

    testa = ajRegExec(primdata->Prima, seqstr);

    if(testa)
    {
	ioff = ajRegOffset(primdata->Prima);
	ajDebug("%s: %S PrimerA matched at %d\n",
		ajSeqGetNameC(seq), primdata->Name, ioff);
	ajFmtPrintF(out, "%s: %S PrimerA matched at %d\n",
		    ajSeqGetNameC(seq), primdata->Name, ioff);
	ajRegTrace(primdata->Prima);
    }

    testb = ajRegExec(primdata->Primb, seqstr);
    if(testb)
    {
	ioff = ajRegOffset(primdata->Primb);
	ajDebug("%s: %S PrimerB matched at %d\n",
		ajSeqGetNameC(seq), primdata->Name, ioff);
	ajFmtPrintF(out, "%s: %S PrimerB matched at %d\n",
		    ajSeqGetNameC(seq), primdata->Name, ioff);
	ajRegTrace(primdata->Primb);
    }

    testc = ajRegExec(primdata->Prima, revstr);
    if(testc)
    {
	ioff = ajStrGetLen(seqstr) - ajRegOffset(primdata->Prima);
	ajDebug("%s: (rev) %S PrimerA matched at %d\n",
		ajSeqGetNameC(seq), primdata->Name, ioff);
	ajFmtPrintF(out, "%s: (rev) %S PrimerA matched at %d\n",
		    ajSeqGetNameC(seq), primdata->Name, ioff);
	ajRegTrace(primdata->Prima);
    }

    testd = ajRegExec(primdata->Primb, revstr);
    if(testd)
    {
	ioff = ajStrGetLen(seqstr) - ajRegOffset(primdata->Primb);
	ajDebug("%s: (rev) %S PrimerB matched at %d\n",
		ajSeqGetNameC(seq), primdata->Name, ioff);
	ajFmtPrintF(out, "%s: (rev) %S PrimerB matched at %d\n",
		    ajSeqGetNameC(seq), primdata->Name, ioff);
	ajRegTrace(primdata->Primb);
    }

    return;
}
