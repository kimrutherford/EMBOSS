/* @source tfextract application
**
** Extracts pattern information from TRANSFAC site.dat file
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




/* @prog tfextract ************************************************************
**
** Extract data from TRANSFAC
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf  = NULL;
    AjPFile fout = NULL;
    AjPFile iout = NULL;
    AjPFile vout = NULL;
    AjPFile pout = NULL;
    AjPFile oout = NULL;
    AjPFile fp   = NULL;

    AjPStr line;
    AjPStr acc;
    AjPStr id;
    AjPStr bf;
    AjPStr pattern;
    AjPStr pfname;
    AjPList sqlist  = NULL;
    AjPStr  patline = NULL;
    AjPStr  tmpseq  = NULL;
    AjBool fullseq = ajFalse;
    
    const char *p;
    char *q;

    AjBool gid=ajFalse;
    AjBool done=ajFalse;

    embInit("tfextract",argc,argv);

    inf = ajAcdGetInfile("infile");

    pfname = ajStrNewC("tffungi");
    fout = ajDatafileNewOutNameS(pfname);

    ajStrAssignC(&pfname,"tfinsect");
    iout = ajDatafileNewOutNameS(pfname);

    ajStrAssignC(&pfname,"tfvertebrate");
    vout = ajDatafileNewOutNameS(pfname);

    ajStrAssignC(&pfname,"tfplant");
    pout = ajDatafileNewOutNameS(pfname);

    ajStrAssignC(&pfname,"tfother");
    oout = ajDatafileNewOutNameS(pfname);

    ajStrDel(&pfname);

    line    = ajStrNew();
    id      = ajStrNewC("");
    bf      = ajStrNew();
    acc     = ajStrNew();

    patline = ajStrNew();
    tmpseq =  ajStrNew();
    
    sqlist  = ajListNew();
    
    while(ajReadlineTrim(inf,&line))
    {
	p = ajStrGetPtr(line);
	
	if(ajStrPrefixC(line,"ID"))
	{
	    gid  = ajTrue;
	    done = ajFalse;
	    fp = oout;
	    p = ajSysFuncStrtok(p," \t\n");
	    p = ajSysFuncStrtok(NULL," \t\n");
	    ajStrAssignC(&id,p);
	}

	if(ajStrPrefixC(line,"AC"))
	{
	    p = ajSysFuncStrtok(p," \t\n");
	    p = ajSysFuncStrtok(NULL," \t\n");
	    ajStrAssignC(&acc,p);
	}


	if(ajStrPrefixC(line,"BF"))
	{
	    p = strpbrk(p," \t\n");
	    while(*p && *p==' ')
		++p;
	    ajStrAssignC(&bf,p);
	}

	if(ajStrPrefixC(line,"SQ") || ajStrPrefixC(line,"SE"))
	{
            if(ajStrSuffixC(line,".."))
                continue;
            
            while(!ajStrPrefixC(line,"XX"))
            {
                fullseq = ajFalse;
                ajFmtScanS(line,"%*S%S",&patline);
                ajStrAppendS(&tmpseq,patline);

                if(ajStrSuffixC(line,"."))
                {
                    ajStrTrimEndC(&tmpseq,".");
                    pattern = ajStrNew();
                    ajStrAssignS(&pattern,tmpseq);


                    q = ajStrGetuniquePtr(&pattern);

                    while(*q)
                    {
                        if(*q=='[')
                            *q = '(';

                        if(*q==':')
                            *q = ',';
                        
                        if(*q==']')
                            *q = ')';
                        ++q;
                    }

                    ajListPush(sqlist,(void *)pattern);
                    ajStrAssignC(&tmpseq,"");
                    fullseq = ajTrue;
                }

                ajReadlineTrim(inf,&line);
            }
            
            if(!fullseq)
                ajErr("Unterminated sequence line for AC = %S",acc);
	}

	if(ajStrPrefixC(line,"OC") && !done)
	{
	    if(strstr(p,"Fungi"))
	    {
		done = ajTrue;
		fp = fout;
	    }
	    else if(strstr(p,"saccharomycetaceae"))
	    {
		done = ajTrue;
		fp = fout;
	    }
	    else if(strstr(p,"arthropoda"))
	    {
		done = ajTrue;
		fp = iout;
	    }
	    else if(strstr(p,"vertebrata"))
	    {
		done=ajTrue;
		fp = vout;
	    }
	    else if(strstr(p,"plantae"))
		fp = pout;
	}

	if(ajStrPrefixC(line,"//") && ajListGetLength(sqlist) && gid)
	{
	    if(!ajStrGetLen(bf))
            {
                while(ajListPop(sqlist,(void **) &pattern))
                {
                    ajFmtPrintF(fp,"%-20s %S %S\n",ajStrGetPtr(id),pattern,acc);
                    ajStrDel(&pattern);
                }
            }
	    else
	    {
                while(ajListPop(sqlist,(void **) &pattern))
                {
                    ajFmtPrintF(fp,"%-20s %S %S %S\n",ajStrGetPtr(id),pattern,
                                acc,bf);
                    ajStrDel(&pattern);
                }
                
		ajStrAssignC(&bf,"");
	    }
	}
    }


    ajFileClose(&inf);
    ajFileClose(&fout);
    ajFileClose(&iout);
    ajFileClose(&vout);
    ajFileClose(&pout);
    ajFileClose(&oout);

    ajListFree(&sqlist);
    
    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&bf);
    ajStrDel(&acc);
    ajStrDel(&pattern);
    ajStrDel(&patline);
    ajStrDel(&tmpseq);

    embExit();

    return 0;
}
