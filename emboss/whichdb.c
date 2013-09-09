/* @source whichdb application
**
** Search all databases for an entry name
**
** @author Copyright (C) Alan Bleasby
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




/* @prog whichdb **************************************************************
**
** Find an entry in all known databases
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr  entry;
    AjPFile outf;
    AjPList dblist = NULL;
    AjPStr  name   = NULL;
    AjPStr  idqry  = NULL;

    AjPStr type = NULL;
    AjPStr comm = NULL;
    AjPStr rel  = NULL;
    AjBool id   = ajFalse;
    AjBool qry  = ajFalse;
    AjBool all  = ajFalse;
    AjBool pro  = ajFalse;
    AjPSeq seq  = NULL;
    AjBool get  = ajFalse;
    AjBool showall  = ajFalse;
    AjPStr lnam = NULL;
    AjPStr snam = NULL;
    AjPStr meth = NULL;
    AjPStr defnam = NULL;

    AjPSeqout seqout = NULL;

    embInit("whichdb", argc, argv);
    
    entry   = ajAcdGetString("entry");
    outf    = ajAcdGetOutfile("outfile");
    get     = ajAcdGetToggle("get");
    showall = ajAcdGetBoolean("showall");
    
    if(!ajStrGetLen(entry))
    {
	ajExitBad();
	return 0;
    }
    
    dblist = ajListNew();
    type   = ajStrNew();
    comm   = ajStrNew();
    rel    = ajStrNew();
    idqry  = ajStrNew();
    seq    = ajSeqNew();
    snam   = ajStrNew();
    meth   = ajStrNew();
    defnam = ajStrNew();
    
    ajNamListListDatabases(dblist);
    
    while(ajListPop(dblist,(void **)&lnam))
    {
	ajStrAssignS(&name,lnam);
	/* ajStrDel(&lnam); */ /* do not delete - internal ajNam string */

	if(!ajNamDbDetails(name,&type,&id,&qry,&all,&comm,&rel,
			   &meth, &defnam))
	    continue;

        if(!ajStrMatchWildC(type, "*Nucleotide*") &&
           !ajStrMatchWildC(type, "*Protein*") &&
           !ajStrMatchWildC(type, "*Sequence*"))
            continue;
           
	if(!id)
	    continue;

	ajFmtPrintS(&idqry,"%S:%S",name,entry);

	if(ajStrMatchWildC(type,"*Protein*"))
	    pro = ajTrue;
	else
	    pro = ajFalse;

	if(showall && outf)
	    ajFmtPrintF(outf,"# Trying '%S'\n",idqry);

	if(!ajSeqGetFromUsa(idqry,pro,seq))
	{
	    if(showall && outf)
		ajFmtPrintF(outf,"# Failed '%S'\n",idqry);
	    continue;
	}

	if(get)
	{
	    ajFmtPrintS(&snam,"%S.%S",entry,name);
	    seqout = ajSeqoutNew();
	    if(!ajSeqoutOpenFilename(seqout,snam))
		ajFatal("Cannot open output file [%S]",snam);
	    ajSeqoutSetFormatC(seqout,"FASTA");
	    ajUser("Writing %S",snam);
	    ajSeqoutWriteSeq(seqout,seq);
	    ajSeqoutClose(seqout);
	    ajSeqoutDel(&seqout);
	}
	else if (outf)
	    ajFmtPrintF(outf,"%S\n",idqry);
    }
    
    
    
    
    
    ajListFree(&dblist);
    ajStrDel(&type);
    ajStrDel(&comm);
    ajStrDel(&meth);
    ajStrDel(&defnam);
    ajStrDel(&rel);
    ajStrDel(&idqry);
    ajStrDel(&snam);
    ajSeqDel(&seq);
    ajStrDel(&entry);
    ajStrDel(&name);
    ajFileClose(&outf);

    embExit();

    return 0;
}
