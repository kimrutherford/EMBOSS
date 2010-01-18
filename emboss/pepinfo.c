/* @source pepinfo application
**
** Displays properties of the amino acid residues in a peptide sequence
** @author Copyright (C) Mark Faller (mfaller@hgmp.mrc.ac.uk)
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


#define NOY (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT + \
 AJGRAPH_Y_INVERT_TICK + AJGRAPH_X_INVERT_TICK + AJGRAPH_X_TICK + \
 AJGRAPH_X_LABEL + AJGRAPH_Y_LABEL + AJGRAPH_TITLE)




static void pepinfo_plotHistInt2(AjPHist hist,
				 const ajint * results, ajint hist_num,
				 const char* header,
				 const char* xtext, const char * ytext);
static void pepinfo_plotGraph2Float(AjPGraph graphs,
				    const float * results,
				    const char* title_text,
				    const char * xtext, const char * ytext);
static void pepinfo_printFloatResults(AjPFile outfile, const AjPSeq seq,
				      const float * results,
				      const char* header);
static void pepinfo_printIntResults(AjPFile outfile, const AjPSeq seq,
				    const ajint* results, const char* header);




static ajint seq_start;
static ajint seq_end;
static ajint win_mid;
static ajint seq_begin;

/* @prog pepinfo **************************************************************
**
** Plots simple amino acid properties in parallel
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq inseq = NULL;
    AjPFile outfile = NULL;
    ajint hwindow;
    AjPFile aa_properties = NULL;
    AjPFile aa_hydropathy = NULL;

    AjBool do_seq;
    AjBool do_general;
    AjBool do_hydropathy;
    AjBool do_plot;
    AjPStr key;
    AjPStr value;
    AjIList listIter;
    AjPTable table;

    AjPStr tmpa = NULL;
    AjPStr tmpb = NULL;
    ajint i;
    ajint j;
    ajint k;
    ajint cnt;

    ajint * ival;
    ajint * iv[9];
    float * pfloat;
    float * pf[3];
    float num;
    float total;

    AjPGraph graphs = NULL;
    AjPHist hist = NULL;
    ajint numGraphs = 0;

    /*   Data_table aa_props, aa_hydro, aa_acc;*/
    AjPList aa_props = NULL;
    AjPList aa_hydro = NULL;

    const char * propertyTitles[] =
    {
	"Tiny",
	"Small",
	"Aliphatic",
	"Aromatic",
	"Non-polar",
	"Polar",
	"Charged",
	"Positive",
	"Negative"
    };

    const char * hydroTitles[] =
    {
	"Kyte & Doolittle hydropathy parameters",
	"OHM  Hydropathy parameters (Sweet & Eisenberg)",
	"Consensus parameters (Eisenberg et al)"
    };

    embInit("pepinfo", argc, argv);
    ajGraphicsSetPagesize(960, 960);

    ajHistogramSetMark(NOY);

    /* Get parameters */
    inseq         = ajAcdGetSeq("sequence");
    seq_begin     = ajSeqGetBegin(inseq);
    seq_end       = ajSeqGetEnd(inseq);
    seq_start     = seq_begin - 1;
    outfile       = ajAcdGetOutfile("outfile");
    hwindow       = ajAcdGetInt("hwindow");
    do_general    = ajAcdGetBoolean("generalplot");
    do_hydropathy = ajAcdGetBoolean("hydropathyplot");

    aa_properties = ajAcdGetDatafile("aaproperties");
    aa_hydropathy = ajAcdGetDatafile("aahydropathy");

    graphs = ajAcdGetGraphxy("graph");

    do_plot = do_general || do_hydropathy;

    /* Set begin and end position in sequence structure */
    ajSeqSetRange(inseq, seq_begin, seq_end);

    /* Find out which tables are required in the output */
    do_seq = ajFalse;

    if(do_hydropathy)
	numGraphs +=3;

    key   = ajStrNew();
    value = ajStrNew();

    /* if sequence plot required */
    if(do_seq)
	ajDebug("sequence plot\n");


    /* if general properties plot required  */
    if(do_general)
    {
	ajDebug("general plot\n");

	/*initialize properties list*/
	aa_props = ajListNew();
	embDataListRead(aa_props, aa_properties);

	/* Get first table from properties list of tables */
	listIter = ajListIterNewread(aa_props);

	/* calculate plot */
	for(i = 0; i < 9; i++)
	{
	    if(!ajListIterDone(listIter))
	    {
		/* ajalloc new ajint array for storing results */
		AJCNEW(ival,(seq_end-seq_start));
		iv[i] = ival;
		table = ajListIterGet(listIter);
		for(j = seq_start; j < seq_end; j++)
		{
		    ajStrAssignSubS(&key, ajSeqGetSeqS(inseq), j, j);
		    value = ajTableFetch(table, key);
		    if(value != NULL)
		    {
			if(ajStrToInt(value, ival))
			    ival++;
			else
			{
			    ajErr("value is not integer ..%S..\n",
				  value);
			    embExit();
			}
		    }
		    else
		    {
			ajErr("At position %d in seq, couldn't find key "
			      "%S", j, key);
			embExit();
		    }
		}
	    }
	    else
	    {
		ajErr("No more tables in list\n");
		embExit();
	    }
	}

	ajListIterDel(&listIter);

	/* print out results */

	for(i=0; i<9; i++)
	{
	    ajFmtPrintS(&tmpa, "%s residues in %s from position %d to %d",
			propertyTitles[i], ajSeqGetNameC(inseq),
			seq_begin, seq_end);
	    pepinfo_printIntResults(outfile, inseq, iv[i], ajStrGetPtr(tmpa));
	}

	/* plot out results */

	hist = ajHistNewG(9, (seq_end - seq_begin+1), graphs);
	hist->bins = seq_end - seq_begin +1;

	hist->xmin = (float) seq_begin;
	hist->xmax = (float) seq_end;

	hist->displaytype=HIST_SEPARATE;

	ajFmtPrintS(&tmpa, "Properties of residues in %s from position "
		    "%d to %d", ajSeqGetNameC(inseq),seq_begin, seq_end);
	ajHistSetTitleC(hist, ajStrGetPtr(tmpa));

	ajHistSetXlabelC(hist, "Residue Number");
	ajHistSetYlabelC(hist, "");

	for(i=0; i<9; i++)
	{
	    ajFmtPrintS(&tmpa,  "%s residues in %s from position %d to %d",
			propertyTitles[i], ajSeqGetNameC(inseq), seq_begin,
			seq_end);
	    ajFmtPrintS(&tmpb,  "%s residues", propertyTitles[i]);
	    pepinfo_plotHistInt2(hist, iv[i], i,
				 ajStrGetPtr(tmpa), ajStrGetPtr(tmpb), "");
	}


	ajHistDisplay(hist);


	for(i = 0; i < 9; i++)
	    AJFREE(iv[i]);

	/*delete hist object*/
	ajHistDel(&hist);
    }

    /* if hydropathy plot required */
    if(do_hydropathy)
    {
	ajDebug("hydropathy plot\n");

	if(numGraphs)
	    ajGraphxySetflagOverlay(graphs, ajFalse);

	/* get data from amino acid properties */
	aa_hydro = ajListNew();
	embDataListRead(aa_hydro, aa_hydropathy);

	/* Get first table from properties list */
	listIter = ajListIterNewread(aa_hydro);

	/* calculate plot */
	for(i=0; i < 3; i++)
	{
	    /* make sure we have another table from the list to calculate */
	    if(ajListIterDone(listIter))
	    {
		ajErr("No more tables in list\n");
		embExit();
	    }

	    /* Get next table of parameters */
	    table = ajListIterGet(listIter);

	    win_mid = (hwindow / 2);

	    /* get array to store result */
	    AJCNEW(pfloat, (seq_end - seq_start));
	    pf[i] = pfloat;

	    /* Fill in 0.00 for seq begin to win_mid */
	    for(j=0,cnt=0;j<win_mid; j++)
		pfloat[cnt++]=0.0;

	    /* start loop */
	    for(j = seq_start; j<=(seq_end-hwindow); j++)
	    {
		total = 0.00;
		for(k=0; k < hwindow; k++)
		{
		    ajStrAssignSubS(&key, ajSeqGetSeqS(inseq), (j+k), (j+k));
		    value = ajTableFetch(table, key);
		    if(value == NULL)
		    {
			ajErr("At position %d in seq, couldn't find key %s",
			       k, ajStrGetPtr(key));
			embExit();
		    }

		    if(!ajStrIsFloat(value))
		    {
			ajErr("value is not float ..%s..",
			      ajStrGetPtr(value));
		       embExit();
		    }
		    ajStrToFloat(value, &num);
		    total +=num;
		}
		pfloat[cnt++] = total / hwindow;
	    }

	    /* fill in value of 0 for end of sequence */
	    for(j = win_mid+1; j<hwindow; j++)
		pfloat[cnt++] = 0.00;
	}

	ajListIterDel(&listIter);

	/* Print out results */

	for(i=0; i<3; i++)
	{
	    ajFmtPrintS(&tmpa,  "Results from %s", hydroTitles[i]);
	    pepinfo_printFloatResults(outfile, inseq, pf[i],
				      ajStrGetPtr(tmpa));
	}

	/*Plot results*/
	for(i=0; i<3; i++)
	{
	    ajFmtPrintS(&tmpa,
			"Hydropathy plot of residues %d to %d of sequence "
			"%s using %s",seq_begin, seq_end, ajSeqGetNameC(inseq),
			hydroTitles[i]);
	    pepinfo_plotGraph2Float(graphs, pf[i], ajStrGetPtr(tmpa),
				    "Residue Number", "Hydropathy value");
	}

	for(i=0; i<3; i++)
	    AJFREE(pf[i]);
    }

    if(numGraphs)
    {
	if(do_general || do_seq)
	    ajGraphNewpage(graphs, ajFalse);

	ajGraphicsSetCharscale(0.50);
	ajGraphSetTitleC(graphs,"Pepinfo");

	ajGraphxyDisplay(graphs,AJTRUE);
    }

    if(do_plot)
	ajGraphicsClose();

    ajSeqDel(&inseq);
    ajFileClose(&outfile);
    ajFileClose(&aa_properties);
    ajFileClose(&aa_hydropathy);
    ajGraphxyDel(&graphs);
    ajHistDel(&hist);

    /* Delete Data tables*/
    embDataListDel(&aa_props);
    embDataListDel(&aa_hydro);

    ajStrDel(&tmpa);
    ajStrDel(&tmpb);
    ajStrDel(&key);

    embExit();

    return 0;
}




/* @funcstatic pepinfo_printIntResults ****************************************
**
**  prints out a resultsList. Very basic at the moment, really just used to
**  prove I have the results and they are correct. There are several of these
**  for each primitive data type (as I write them). So far have
**  printIntResults and printFloatResults. They are public routines
**
** @param [u] outfile [AjPFile] file to output to.
** @param [r] seq     [const AjPSeq]  Sequence
** @param [r] results [const ajint*]  array of reuslts.
** @param [r] header  [const char*]   header line
** @return [void]
** @@
******************************************************************************/

static void pepinfo_printIntResults(AjPFile outfile, const AjPSeq seq,
				    const ajint* results, const char * header)
{
    ajint i;
    AjPStr aa;

    aa = ajStrNew();
    ajFmtPrintF(outfile,  "Printing out %s\n\n", header);
    ajFmtPrintF(outfile, "Position  Residue\t\t\tResult\n");
    for(i = seq_start; i<seq_end; i++)
    {
       ajStrAssignSubS(&aa, ajSeqGetSeqS(seq), i, i);
       ajFmtPrintF(outfile,  "   %5d%8s%32d\n", (i+1),
			ajStrGetPtr(aa), *results++);
    }

    ajFmtPrintF(outfile,  "\n\n\n");

    ajStrDel(&aa);

    return;
}




/* @funcstatic pepinfo_printFloatResults **************************************
**
** Routine to print out Float results data
**
** @param [u] outfile [AjPFile] file to output to.
** @param [r] seq     [const AjPSeq]  Sequence
** @param [r] results [const float*] array of results.
** @param [r] header  [const char*]   header line
** @return [void]
** @@
******************************************************************************/

static void pepinfo_printFloatResults(AjPFile outfile, const AjPSeq seq,
				      const float * results,
				      const char * header)
{
    ajint i;
    AjPStr aa;

    aa = ajStrNew();
    ajFmtPrintF(outfile,  "Printing out %s\n\n", header);
    ajFmtPrintF(outfile, "Position  Residue\t\t\tResult\n");
    for(i = seq_start; i<seq_end; i++)
    {
       ajStrAssignSubS(&aa, ajSeqGetSeqS(seq), i, i);
       ajFmtPrintF(outfile,  "%5d%8s%32.3f\n", (i+1),
			ajStrGetPtr(aa), *results++);
    }
    ajFmtPrintF(outfile, "\n\n\n");

    ajStrDel(&aa);

    return;
}




/* @funcstatic pepinfo_plotGraph2Float ****************************************
**
** Create and add graph from set of results to graph set.
**
** @param [u] graphs [AjPGraph] Graphs set to add new graph to.
** @param [r] results [const float*]  array of results.
** @param [r] title_text [const char*] title for graph
** @param [r] xtext      [const char*] x label.
** @param [r] ytext      [const char*] y label.
** @return [void]
**
******************************************************************************/

static void pepinfo_plotGraph2Float(AjPGraph graphs,
				    const float * results,
				    const  char * title_text,
				    const char * xtext, const char * ytext)
{

    AjPGraphdata plot;

    ajint npts = 0;

    float ymin = 64000.;
    float ymax = -64000.;

    npts = seq_end - seq_start;

    ajGraphicsCalcRange(results,npts,&ymin,&ymax);

    /*
    **  initialise plot, the number of points will be the length of the data
    **  in the results structure
    */
    plot = ajGraphdataNewI(npts);

    /*Set up rest of plot information*/
    ajGraphdataSetTitleC(plot, title_text);
    ajGraphdataSetXlabelC(plot, xtext);
    ajGraphdataSetYlabelC(plot, ytext);
    ajGraphdataSetMinmax(plot,(float)1,(float)npts,ymin,ymax);
    ajGraphdataSetTruescale(plot,(float)1,(float)npts,ymin,ymax);
    ajGraphdataSetTypeC(plot,"2D Plot");

    ajGraphdataCalcXY(plot, npts, (float)seq_begin, 1.0, results);
    ajGraphDataAdd(graphs, plot);

    return;
}




/* @funcstatic pepinfo_plotHistInt2 *******************************************
**
** Add a histogram data to the set set.
**
** @param [u] hist   [AjPHist] Histogram set to add new set to.
** @param [r] results [const ajint*]  array of results.
** @param [r] hist_num [ajint] the number of the histogram set.
** @param [r] header  [const char*] title.
** @param [r] xtext   [const char*] x label.
** @param [r] ytext   [const char*] y label.
** @return [void]
**
******************************************************************************/

static void pepinfo_plotHistInt2(AjPHist hist,
				 const ajint * results,
				 ajint hist_num, const char * header,
				 const char * xtext, const char * ytext)
{
    ajint npts;
    ajint i;

    float *farray;

    npts = seq_end - seq_start;

    AJCNEW(farray, npts);
    for(i=0; i<npts; i++)
	farray[i] = (float) results[i];

    ajHistDataCopy(hist, hist_num, farray);

    ajHistSetmultiTitleC(hist, hist_num, header);
    ajHistSetmultiXlabelC(hist, hist_num, xtext);
    ajHistSetmultiYlabelC(hist, hist_num, ytext);

    AJFREE(farray);

    return;
}
