/* @source infoseq application
**
** Displays some simple information about sequences
**
** @author Copyright (C) Jon Ison (jison@ebi.ac.uk) 2006
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @modified 29 June 2006 Jon Ison (major rewrite)
** @modified 04/02/2000 rbsk@sanger - added 'percent GC' computation
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



static AjBool infoseq_printheader(AjBool html,  AjBool instring,
				  const char *text, 
				  ajuint wid, AjBool columns,
				  const AjPStr delimiter, 
				  AjPFile outfile);
static AjBool infoseq_print(AjBool html, AjBool instring,
			    const AjPStr str, AjBool usewid,
			    ajuint wid, AjBool columns,
			    const AjPStr delimiter, AjPFile outfile);




/* @prog infoseq **************************************************************
**
** Displays some simple information about sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* VARIABLE DECLARATIONS */
    AjPSeqall seqall    = NULL;
    AjPSeq    seq       = NULL;
    AjBool    html;
    AjBool    doheader;
    AjBool    dotype;
    AjBool    dousa;
    AjBool    dodb;
    AjBool    doname;
    AjBool    doacc;
    AjBool    dogi;
    AjBool    dosv;
    AjBool    dolength;
    AjBool    doorg;
    AjBool    dodesc;
    AjBool    dopgc;
    AjPFile   outfile   = NULL;
    AjPStr    altusa 	= NULL;  /* default name when the real name is unknown */
    AjPStr    altname   = NULL;
    AjPStr    altacc    = NULL;
    AjPStr    altgi     = NULL;
    AjPStr    altsv     = NULL;
    AjPStr    altdb     = NULL;
    ajint     length;
    AjBool    type      = ajTrue; /* ajTrue if Protein */
    float     pgc       = 0.0;
    AjBool    firsttime = ajTrue;
    const AjPStr usa       = NULL;
    const AjPStr name      = NULL;
    const AjPStr acc       = NULL;
    const AjPStr gi        = NULL;
    const AjPStr sv        = NULL;
    const AjPStr desc      = NULL;
    const AjPStr org       = NULL;
    const AjPStr db        = NULL;
    AjBool columns   = ajFalse;   
    AjPStr delimiter = NULL;      
    AjPStr tempstr   = NULL;      
    AjBool instring  = ajFalse; /* If token was printed and not at 
				   end-of-line yet */
    




    /* ACD PROCESSING */
    embInit("infoseq", argc, argv);

    outfile   = ajAcdGetOutfile("outfile");
    seqall    = ajAcdGetSeqall("sequence");
    html      = ajAcdGetBoolean("html");
    doheader  = ajAcdGetBoolean("heading");
    dousa     = ajAcdGetBoolean("usa");
    dodb      = ajAcdGetBoolean("database");
    doname    = ajAcdGetBoolean("name");
    doacc     = ajAcdGetBoolean("accession");
    dogi      = ajAcdGetBoolean("gi");
    dosv      = ajAcdGetBoolean("seqversion");
    dotype    = ajAcdGetBoolean("type");
    dolength  = ajAcdGetBoolean("length");
    dopgc     = ajAcdGetBoolean("pgc");
    dodesc    = ajAcdGetBoolean("description");
    doorg     = ajAcdGetBoolean("organism");
    columns   = ajAcdGetBoolean("columns"); 
    delimiter = ajAcdGetString("delimiter"); 

    altusa    = ajStrNewC("-");
    altname   = ajStrNewC("-");
    altacc    = ajStrNewC("-");
    altgi     = ajStrNewC("-");
    altsv     = ajStrNewC("-");
    altdb     = ajStrNewC("-");
    tempstr   = ajStrNew();   
    
    if(ajStrMatchC(delimiter, "\\t"))
	ajStrAssignK(&delimiter, '\t');
    


    /* PRINT START OF HTML TABLE */
    if(html)
	ajFmtPrintF(outfile,"<table border cellpadding=4 bgcolor=\"#FFFFF0"
		    "\">\n");



    /* MAIN APPLICATION LOOP */
    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);
	ajSeqTrace(seq);

        /* is this a protein or nucleic sequence? */
        type = ajSeqIsProt(seq);

	if(firsttime)
	{
	    /* Print the header information */
	    if(doheader)
	    {
		/* Start the HTML table title line */
		if(html)
		    ajFmtPrintF(outfile, "<tr>");
		/* else if(columns)
		    ajFmtPrintF(outfile, "%s", "#"); */


		
		if(dousa)
		    instring = infoseq_printheader(html, instring,
						   "USA", 25, 
						   columns, delimiter,
						   outfile);
		if(dodb)
		    instring = infoseq_printheader(html, instring,
						   "Database", 10, 
						   columns, delimiter,
						   outfile);
		if(doname)
		    instring = infoseq_printheader(html, instring,
						   "Name", 15, 
						   columns, delimiter,
						   outfile);
		if(doacc)
		    instring = infoseq_printheader(html, instring,
						   "Accession", 15,
						   columns, delimiter, 
						   outfile);
		if(dogi)
		    instring = infoseq_printheader(html, instring,
						   "GI", 15, 
						   columns, delimiter, 
						   outfile);
		if(dosv)
		    instring = infoseq_printheader(html, instring,
						   "Version", 8, 
						   columns, delimiter,
						   outfile);
		if(dotype)
		    instring = infoseq_printheader(html, instring,
						   "Type", 5, 
						   columns, delimiter, 
						   outfile);
		if(dolength)		
		    instring = infoseq_printheader(html, instring,
						   "Length", 7, 
						   columns, delimiter, 
						   outfile);
		if(!type && dopgc)
		    instring = infoseq_printheader(html, instring,
						   "%GC", 7, 
						   columns, delimiter, 
						   outfile);
		if(doorg)
		    instring = infoseq_printheader(html, instring,
						   "Organism", 20,
						   columns, delimiter, 
						   outfile);
		if(dodesc)
		    instring = infoseq_printheader(html, instring,
						   "Description", 12,
						   columns, delimiter, 
						   outfile);

		/* End the HTML table title line */
		if(html)
		    ajFmtPrintF(outfile, "</tr>\n");
		else
		    ajFmtPrintF(outfile, "\n");
		instring = ajFalse;
	    }
	    firsttime = ajFalse;
	}
	
	/* GET SEQUENCE ATTRIBUTES (strings set to '-' if unknown) */ 
	/* usa */
	usa = ajSeqGetUsaS(seq);
	if(ajStrGetLen(usa) == 0)
	    usa = altusa;

	/* db */
	db = ajSeqGetDbS(seq);
	if(ajStrGetLen(db) == 0)
	    db = altdb;

	/* name */
	name = ajSeqGetNameS(seq);
	if(ajStrGetLen(name) == 0)
	    name = altname;

	/* accession number */
	acc = ajSeqGetAccS(seq);
	if(ajStrGetLen(acc) == 0)
	    acc = altacc;

	/* GI number */
	gi = ajSeqGetGiS(seq);
	if(ajStrGetLen(gi) == 0)
	    gi = altgi;

	/* version number */
	sv = ajSeqGetSvS(seq);
	if(ajStrGetLen(sv) == 0)
	    sv = altsv;

	/* length */
	length = ajSeqGetLen(seq);
	if(dopgc && !type)
	{
	    pgc = ajMeltGC(ajSeqGetSeqS(seq),length);
	    pgc *= 100;			/* percentage */
	}
	
	/* description */
	desc = ajSeqGetDescS(seq);

        org = ajSeqGetTaxS(seq);

	/* start table line */
	if(html)
	    ajFmtPrintF(outfile, "<tr>");

	/* To correspond to # in header line */
	/*	if(doheader && columns)
	    ajFmtPrintF(outfile, " "); */



	if(dousa)
	    instring = infoseq_print(html, instring, usa,
				     ajTrue, 25, columns, 
				     delimiter, outfile);
	if(dodb)
	    instring = infoseq_print(html, instring, db,
				     ajTrue, 15, columns, 
				     delimiter, outfile);
	if(doname)
	    instring = infoseq_print(html, instring, name,
				     ajTrue, 15, columns, 
				     delimiter, outfile);
	if(doacc)
	    instring = infoseq_print(html, instring, acc,
				     ajTrue, 15, columns, 
				     delimiter, outfile);
	if(dogi)
	    instring = infoseq_print(html, instring, gi,
				     ajTrue, 15, columns, 
				     delimiter, outfile);
	if(dosv)
	    instring = infoseq_print(html, instring, sv,
				     ajTrue, 8, columns, 
				     delimiter, outfile);
	if(dotype)
	{
	    if(type)
		ajFmtPrintS(&tempstr, "%c", 'P');
	    else
		ajFmtPrintS(&tempstr, "%c", 'N');
	    instring = infoseq_print(html, instring, tempstr, ajTrue, 5, 
				     columns, delimiter, outfile);
	}
	if(dolength)		
	{
	    ajFmtPrintS(&tempstr, "%d", length);
	    instring = infoseq_print(html, instring, tempstr, ajTrue, 7, 
				     columns, delimiter, outfile);
	}
	if(!type && dopgc)
	{
	    ajFmtPrintS(&tempstr, "%.2f", pgc);
	    instring = infoseq_print(html, instring, tempstr, ajTrue, 7, 
				     columns, delimiter, outfile);
	}	

	if(doorg)
	    instring = infoseq_print(html, instring, org, ajTrue, 20, 
				     columns, delimiter, outfile);

	if(dodesc)
	    instring = infoseq_print(html, instring, desc, ajFalse, 0, 
				     columns, delimiter, outfile);

	/* end table line */
	if(html)
	    ajFmtPrintF(outfile, "</tr>\n");
	else
	    ajFmtPrintF(outfile, "\n");
	instring = ajFalse;
    }


    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");

    ajFileClose(&outfile);

    ajStrDel(&altusa);
    ajStrDel(&altname);
    ajStrDel(&altacc);
    ajStrDel(&altsv);
    ajStrDel(&altdb);
    ajStrDel(&altgi);
    ajStrDel(&delimiter); /* JISON */
    ajStrDel(&tempstr); /* JISON */

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    embExit();

    return 0;
}




/* @funcstatic infoseq_printheader ********************************************
**
** Prints out a sequence information record to html or text file. 
**
** @param [r] html  [AjBool] HTML output if true, else plain text
** @param [r] instring [AjBool] Some token already printed (need spaces)
** @param [r] text [const char *] Text to print
** @param [r] wid [ajuint] Field width
** @param [r] columns [AjBool] Print in columns if true
** @param [r] delimiter [const AjPStr] Delimiter between tokens
** @param [u] outfile [AjPFile] Output file
** @return [AjBool] True on success.
** @@
******************************************************************************/
static AjBool infoseq_printheader(AjBool html,  AjBool instring,
				  const char *text, 
				  ajuint wid, AjBool columns,
				  const AjPStr delimiter, 
				  AjPFile outfile)
{
    /* Suppress delimiter on first call (for first string printed out) */
    static AjBool nodelim = AJTRUE;
    static AjBool colfull = AJFALSE;
    ajuint tlen = strlen(text);
    
    /* Reset for each new line */
    if(!instring)
    {
	nodelim = ajTrue;
	colfull = ajFalse;
    }

    if(html)
	ajFmtPrintF(outfile, "<th>%s</th>", text);
    else
    {
	if(columns)
	{
	    if(colfull)
		ajFmtPrintF(outfile, " %-*s", wid, text);
	    else
		ajFmtPrintF(outfile, "%-*s", wid, text);
	    if(tlen >= wid)
		colfull = ajTrue;
	    else
		colfull = ajFalse;
	}
	else
	{
	    if(nodelim)
		ajFmtPrintF(outfile, "%s", text); 
	    else
		ajFmtPrintF(outfile, "%S%s", delimiter, text); 
	}
    }	

    nodelim = ajFalse;

    return ajTrue;
}




/* @funcstatic infoseq_print **************************************************
**
** Prints out a sequence information record to html or text file. 
**
** @param [r] html  [AjBool] Undocumented
** @param [r] instring [AjBool] Undocumented
** @param [r] str [const AjPStr] Undocumented
** @param [r] usewid [AjBool] Undocumented
** @param [r] wid [ajuint] Undocumented
** @param [r] columns [AjBool] Undocumented
** @param [r] delimiter [const AjPStr] Undocumented
** @param [u] outfile [AjPFile] Undocumented
** @return [AjBool]  True on success.
** @@
******************************************************************************/
static AjBool infoseq_print(AjBool html, AjBool instring, const AjPStr str,
			    AjBool usewid, ajuint wid, AjBool columns,
			    const AjPStr delimiter, AjPFile outfile)
{
    /* Suppress delimiter on first call (for first string printed out) */
    static AjBool nodelim = AJTRUE;
    static AjBool colfull = AJFALSE;
    ajuint tlen = ajStrGetLen(str);

    /* Reset for each new line */
    if(instring == ajFalse)
    {
	nodelim = ajTrue;
	colfull = ajFalse;
    }
   

    if(html)
	ajFmtPrintF(outfile, "<td>%S</td>", str);
    else
    {
	if(columns)
	{
	    if(colfull)
		ajFmtPrintF(outfile, " ");
	    if(usewid)
	    {
		ajFmtPrintF(outfile, "%-*S", wid, str);
		if(tlen >= wid)
		    colfull = ajTrue;
		else
		    colfull = ajFalse;
	    }
	    else
	    {
		ajFmtPrintF(outfile, "%S", str);
	    }
	}
	else
	{
	    if(nodelim)
		ajFmtPrintF(outfile, "%S", str); 
	    else
		ajFmtPrintF(outfile, "%S%S", delimiter, str); 
	}
    }	

    nodelim = ajFalse;
    
    return ajTrue;
}
