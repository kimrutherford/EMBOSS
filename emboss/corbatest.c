/* @source corba test for EBI EMBL database
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

#include <stdio.h>
#include "emboss.h"




#ifndef HAVE_ORB_ORBIT_H
/* @header main ************************************************************
**
******************************************************************************/
int main(int argc, char **argv)
{
    fprintf(stderr,"CORBA support has not been compiled\n");
    return 0;
}
#else




/* @prog corbatest ************************************************************
**
** Testing
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf = NULL;
    AjPStr entry = NULL;
    AjPCorbafeat feat  = NULL;
    AjPStr seq    = NULL;
    AjBool dofeat = ajTrue;

    char *exerr  = NULL;
    ajint  exint = 0;
    ajint  nfeat = 0;
    ajint  nlocs = 0;
    ajint  i;
    ajint  j;
    ajint  k;
    ajint  nval;
    ajint  ntags;

    embInit("corbatest", argc, argv);

    entry = ajAcdGetString("entry");
    outf  = ajAcdGetOutfile("outfile");

    if(!(seq = ajSeqCorbaEmbl(ajStrGetPtr(entry),&exerr,&exint,&feat,dofeat)))
    {
	fprintf(stderr,"Error: %s\tCode: %d\n",exerr,exint);
	return -1;
    }

    ajFmtPrintF(outf,"Sequence: %S\n",seq);

    for(i=0;i<feat->Ntypes;++i)
    {
	ajFmtPrintF(outf,"Name: %S\tSource: %S\tID: %S\n",feat->Types[i]->Name,
		    feat->Types[i]->Source,feat->Types[i]->Id);
	ajFmtPrintF(outf,"Start: %d\tEnd: %d\tStrand: %d\n",
		    feat->Types[i]->Start,feat->Types[i]->End,
		    feat->Types[i]->Strand);
	ajFmtPrintF(outf,"Locations...\n");
	nlocs = feat->Types[i]->Nlocs;
	for(j=0;j<nlocs;++j)
	{
	    ajFmtPrintF(outf,"Start: Pos=%d Ext=%d Fuzzy=%d\n",
			feat->Types[i]->LSpos[j],
			feat->Types[i]->LSex[j],
			feat->Types[i]->LSfuzzy[j]);
	    ajFmtPrintF(outf,"End:   Pos=%d Ext=%d Fuzzy=%d\n",
			feat->Types[i]->LEpos[j],
			feat->Types[i]->LEex[j],
			feat->Types[i]->LEfuzzy[j]);
	    ajFmtPrintF(outf,"Strand: %d\n",feat->Types[i]->LStrand[j]);
	}

	ntags = feat->Types[i]->Ntags;
	for(j=0;j<ntags;++j)
	{
	    nval = feat->Types[i]->Nval[j];
	    for(k=0;k<nval;++k)
		ajFmtPrintF(outf,"Tag: %S\tVal: %S\n",
			    feat->Types[i]->Tag[j],feat->Types[i]->Val[j][k]);
	}
    }

    ajStrDel(&seq);
    ajFileClose(&outf);

    ajCorbafeatDel(&feat);

    embExit();

    return 0;
}

#endif
