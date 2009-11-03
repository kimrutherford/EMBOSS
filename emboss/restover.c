/* @source restover application
 **
 ** Reports restriction enzyme that produce specific overhangs
 ** @author Copyright (C) Bernd Jagla
 ** @@ modified source from Alan
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
 *****************************************************************************/


#include "emboss.h"
#include <string.h>
#include <limits.h>

#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "embossre.equ"


#define EQUGUESS 3500	  /* Estimate of number of equivalent names */




static void restover_printHits(const AjPSeq seq, const AjPStr seqcmp,
			       AjPFile outf, AjPList l,
			       const AjPStr name, ajint hits, ajint begin,
			       ajint end, ajint mincut,
			       ajint maxcut, AjBool plasmid,
			       ajint sitelen, AjBool limit,
			       const AjPTable table, AjBool alpha,
			       AjBool frags, AjBool html);
static void restover_read_equiv(AjPFile equfile, AjPTable table);
static void restover_read_file_of_enzyme_names(AjPStr *enzymes);




/* @prog restover *************************************************************
**
** Finds restriction enzymes that produce a specific overhang
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq     = NULL;
    AjPStr seqcmp  = NULL;
    AjPStr enzymes = NULL;
    AjPFile outf   = NULL;
    ajint begin;
    ajint end;
    ajint min;
    ajint max;
    ajint sitelen;
    AjBool alpha;
    AjBool single;
    AjBool blunt;
    AjBool ambiguity;
    AjBool sticky;
    AjBool plasmid;
    AjBool threeprime;
    AjBool commercial;
    AjBool html;
    AjBool limit;
    AjBool frags;
    AjBool methyl;
    AjPFile dfile;

    AjPFile enzfile  = NULL;
    AjPFile equfile  = NULL;
    AjPFile methfile = NULL;
    
    AjPStr name = NULL;

    AjPTable table = NULL;

    ajint hits;

    AjPList l = NULL;

    embInit("restover", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    seqcmp    = ajAcdGetString("seqcomp");
    ajStrFmtUpper(&seqcmp);
    outf      = ajAcdGetOutfile("outfile");

    /*
    ** Some of these are not needed but I left them in case someone wants to
    ** use them some time ...
    */
    enzymes   = ajStrNewC("all");

    min        = ajAcdGetInt("min");
    max        = ajAcdGetInt("max");
    sitelen    = 2;
    threeprime = ajAcdGetBoolean("threeprime");
    blunt      = ajAcdGetBoolean("blunt");
    sticky     = ajAcdGetBoolean("sticky");
    single     = ajAcdGetBoolean("single");
    html       = ajAcdGetBoolean("html");
    alpha      = ajAcdGetBoolean("alphabetic");
    ambiguity  = ajAcdGetBoolean("ambiguity");
    plasmid    = ajAcdGetBoolean("plasmid");
    commercial = ajAcdGetBoolean("commercial");
    limit      = ajAcdGetBoolean("limit");
    frags      = ajAcdGetBoolean("fragments");
    methyl     = ajAcdGetBoolean("methylation");
    dfile      = ajAcdGetDatafile("datafile");
    methfile   = ajAcdGetDatafile("mfile");

    if(single)
	max = min = 1;

    table = ajTablestrNewLen(EQUGUESS);
    l = ajListNew();

    if(threeprime)
	ajStrReverse(&seqcmp);

    /* read the local file of enzymes names */
    restover_read_file_of_enzyme_names(&enzymes);

    if(!dfile)
    {
	enzfile = ajDatafileNewInNameC(ENZDATA);
	if(!enzfile)
	    ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");
    }
    else
    {
	enzfile = dfile;
    }



    if(limit)
    {
	equfile = ajDatafileNewInNameC(EQUDATA);
	if(!equfile)
	    limit=ajFalse;
	else
	{
	    restover_read_equiv(equfile,table);
	    ajFileClose(&equfile);
	}
    }



    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);
	ajFileSeek(enzfile,0L,0);
	ajSeqFmtUpper(seq);

	hits = embPatRestrictMatch(seq,begin,end,enzfile,methfile,enzymes,
                                   sitelen,plasmid,ambiguity,min,max,blunt,
                                   sticky,commercial,methyl,l);
	ajDebug("hits:%d listlen:%u\n", hits, ajListGetLength(l));
	if(hits)
	{
	    name = ajStrNewC(ajSeqGetNameC(seq));
	    restover_printHits(seq, seqcmp, outf,l,name,hits,begin,end,
			       min,max,plasmid,
			       sitelen,limit,table,alpha,frags,
			       html);
	    ajStrDel(&name);
	}

	ajListFree(&l);
    }


    ajListFree(&l);
    ajSeqDel(&seq);
    ajFileClose(&outf);
    ajFileClose(&dfile);
    ajFileClose(&enzfile);
    ajFileClose(&equfile);
    ajFileClose(&methfile);

    ajSeqallDel(&seqall);
    ajStrDel(&seqcmp);
    ajStrDel(&enzymes);
    ajStrDel(&name);

    ajTablestrFree(&table);

    embExit();

    return 0;
}




/* @funcstatic restover_printHits *********************************************
**
** Print restover hits
**
** @param [r] seq [const AjPSeq] Sequence
** @param [r] seqcmp [const AjPStr] Undocumented
** @param [w] outf [AjPFile] outfile
** @param [u] l [AjPList] hits
** @param [r] name [const AjPStr] sequence name
** @param [r] hits [ajint] number of hits
** @param [r] begin [ajint] start position
** @param [r] end [ajint] end position
** @param [r] mincut [ajint] minimum cuts
** @param [r] maxcut [ajint] maximum cuts
** @param [r] plasmid [AjBool] circular
** @param [r] sitelen [ajint] length of cut site
** @param [r] limit [AjBool] limit count
** @param [r] table [const AjPTable] supplier table
** @param [r] alpha [AjBool] alphabetic sort
** @param [r] frags [AjBool] show fragment lengths
** @param [r] html [AjBool] show html
** @@
******************************************************************************/

static void restover_printHits(const AjPSeq seq, const AjPStr seqcmp,
			       AjPFile outf,
			       AjPList l, const AjPStr name, ajint hits,
			       ajint begin, ajint end,
			       ajint mincut, ajint maxcut, AjBool plasmid,
			       ajint sitelen,
			       AjBool limit, const AjPTable table,
			       AjBool alpha, AjBool frags,
			       AjBool html)
{
    EmbPMatMatch m = NULL;
    AjPStr ps = NULL;
    ajint *fa = NULL;
    ajint *fx = NULL;
    ajint fc = 0;
    ajint fn = 0;
    ajint fb = 0;
    ajint last = 0;
    AjPStr overhead = NULL;

    AjPStr value = NULL;

    ajint i;
    ajint c = 0;

    ajint hang1;
    ajint hang2;


    ps = ajStrNew();
    fn = 0;

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Restrict of %S from %d to %d\n",name,begin,end);

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"#\n");

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Minimum cuts per enzyme: %d\n",mincut);

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Maximum cuts per enzyme: %d\n",maxcut);

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Minimum length of recognition site: %d\n",
		sitelen);
    if(html)
	ajFmtPrintF(outf,"<BR>");

    hits = embPatRestrictRestrict(l,hits,!limit,alpha);

    if(frags)
    {
	fa = AJALLOC(hits*2*sizeof(ajint));
	fx = AJALLOC(hits*2*sizeof(ajint));
    }


    ajFmtPrintF(outf,"# Number of hits with any overlap: %d\n",hits);

    if(html)
	ajFmtPrintF(outf,"<BR>");

    if(html)
	ajFmtPrintF(outf,"</p><table  border cellpadding=4 "
		    "bgcolor=\"#FFFFF0\">\n");
    if(html)
	ajFmtPrintF(outf,
		    "<th>Base Number</th><th>Enzyme</th><th>Site</th>"
		    "<th>5'</th><th>3'</th><th>[5'</th><th>3']</th>\n");
    else
	ajFmtPrintF(outf,"# Base Number\tEnzyme\t\tSite\t\t5'\t3'\t"
		    "[5'\t3']\n");

    for(i=0;i<hits;++i)
    {
	ajListPop(l,(void **)&m);
	ajDebug("hit %d start:%d cut1:%d cut2:%d\n",
		i, m->start, m->cut1, m->cut2);

	hang1 = (ajint)m->cut1 - (ajint)m->start;
	hang2 = (ajint)m->cut2 - (ajint)m->start;

	if(!plasmid && (hang1>100 || hang2>100))
	{
	    embMatMatchDel(&m);
	    continue;
	}

	if(limit)
	{
	    value=ajTableFetch(table,m->cod);
	    if(value)
		ajStrAssignS(&m->cod,value);
	}

	if(m->cut2 >= m->cut1)
	    ajStrAssignSubS(&overhead, ajSeqGetSeqS( seq), m->cut1, m->cut2-1);
	else
	{
	    ajStrAssignSubS(&overhead, ajSeqGetSeqS( seq), m->cut2, m->cut1-1);
	    ajStrReverse(&overhead);
	}

	ajDebug("overhead:%S seqcmp:%S\n", overhead, seqcmp);

	/* Print out only those who have the same overhang. */
	if(ajStrMatchCaseS(overhead, seqcmp))
	{
	    if(html)
	    {
		ajFmtPrintF(outf,
			    "<tr><td>%-d</td><td>%-16s</td><td>%-16s"
			    "</td><td>%d</td><td>%d</td></tr>\n",
			    m->start,ajStrGetPtr(m->cod),ajStrGetPtr(m->pat),
			    m->cut1,m->cut2);
	    }
	    else
		ajFmtPrintF(outf,"\t%-d\t%-16s%-16s%d\t%d\t\n",
			    m->start,ajStrGetPtr(m->cod),ajStrGetPtr(m->pat),
			    m->cut1,m->cut2);
	}

	if(frags)
	    fa[fn++] = m->cut1;

	if(m->cut3 || m->cut4)
	{
	    if(m->cut4 >= m->cut3)
		ajStrAssignSubS(&overhead, ajSeqGetSeqS( seq),
				m->cut3, m->cut4-1);
	    else
	    {
		ajStrAssignSubS(&overhead, ajSeqGetSeqS( seq),
				m->cut4, m->cut3-1);
		ajStrReverse(&overhead);
	    }

	    if(ajStrMatchCaseS(overhead, seqcmp))
	    {
		if(html)
		    ajFmtPrintF(outf,
				"<tr><td>%-d</td><td>%-16s</td><td>%-16s"
				"</td><td></td><td></td><td>%d</td><td>%d"
				"</td></tr>\n",
				m->start,ajStrGetPtr(m->cod),
				ajStrGetPtr(m->pat),
				m->cut1,m->cut2);
		else
		    ajFmtPrintF(outf,"\t%-d\t%-16s%-16s\t\t%d\t%d\t\n",
				m->start,ajStrGetPtr(m->cod),
				ajStrGetPtr(m->pat),
				m->cut1,m->cut2);
	    }
	}

	/* I am not sure what fragments are doing so I left it in ...*/
	/* used in the report tail in restrict - restover does much the same */
	if(m->cut3 || m->cut4)
	{
	    if(frags)
		fa[fn++] = m->cut3;
	    /*	       ajFmtPrintF(*outf,"%d\t%d",m->cut3,m->cut4);*/
	}
	ajStrDel(&overhead);

	embMatMatchDel(&m);
    }



    if(frags)
    {
	ajSortIntInc(fa,fn);
	ajFmtPrintF(outf,"\n\nFragment lengths:\n");
	if(!fn || (fn==1 && plasmid))
	    ajFmtPrintF(outf,"    %d\n",end-begin+1);
	else
	{
	    last = -1;
	    fb = 0;
	    for(i=0;i<fn;++i)
	    {
		if((c=fa[i])!=last)
		    fa[fb++]=c;
		last = c;
	    }
	    fn = fb;
	    /* Calc lengths */

	    for(i=0;i<fn-1;++i)
		fx[fc++] = fa[i+1]-fa[i];
	    if(!plasmid)
	    {
		fx[fc++] = fa[0]-begin+1;
		fx[fc++] = end-fa[fn-1];
	    }
	    else
		fx[fc++] = (fa[0]-begin+1)+(end-fa[fn-1]);

	    ajSortIntDec(fx,fc);
	    for(i=0;i<fc;++i)
		ajFmtPrintF(outf,"    %d\n",fx[i]);
	}
	AJFREE(fa);
	AJFREE(fx);
    }


    ajStrDel(&ps);

    if(html)
	ajFmtPrintF(outf,"</table>\n");

    return;
}




/* @funcstatic restover_read_equiv ********************************************
**
** Load table with equivalent RE names
**
** @param [u] equfile [AjPFile] names
** @param [u] table [AjPTable] table to store names in
** @@
******************************************************************************/

static void restover_read_equiv(AjPFile equfile, AjPTable table)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    const char *p;

    line = ajStrNew();

    while(ajReadlineTrim(equfile,&line))
    {
	p=ajStrGetPtr(line);
	if(!*p || *p=='#' || *p=='!')
	    continue;
	p=ajSysFuncStrtok(p," \t\n");
	key=ajStrNewC(p);
	p=ajSysFuncStrtok(NULL," \t\n");
	value=ajStrNewC(p);
	ajTablePut(table,(void *)key, (void *)value);
    }

    ajStrDel(&line);

    return;
}




/* @funcstatic restover_read_file_of_enzyme_names *****************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [w] enzymes [AjPStr*] enzymes to search for, can be passed as
**                             'all' or '@file'
** @return [void]
** @@
******************************************************************************/

static void restover_read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file = NULL;
    AjPStr line;
    const char *p = NULL;

    if (ajStrFindC(*enzymes, "@") == 0)
    {
	ajStrTrimC(enzymes, "@");	/* remove the @ */
	file = ajFileNewInNameS(*enzymes);
	if (file == NULL)
	    ajFatal ("Cannot open the file of enzyme names: '%S'", enzymes);

	/* blank off the enzyme file name and replace with the enzyme names */
	ajStrSetClear(enzymes);

	line = ajStrNew();
	while(ajReadlineTrim(file, &line))
	{
	    p = ajStrGetPtr(line);
	    if (!*p || *p == '#' || *p == '!')
		continue;

	    ajStrAppendS(enzymes, line);
	    ajStrAppendC(enzymes, ",");
	}
	ajStrDel(&line);

	ajFileClose(&file);
    }

    return;
}
