/* @source btwisted application
**
** Calculates twist in B DNA
** @author Copyright (C) David Martin (dmartin@hgmp.mrc.ac.uk)
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




static AjPTable btwisted_getdinucdata(AjPFile anglesfile);




/* @prog btwisted *************************************************************
**
** Calculates the twisting in a B-DNA sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq   seq   = NULL;
    AjPFile  angles   = NULL;
    AjPFile  energies = NULL;
    AjPFile  result   = NULL;

    AjPTable angletable  = NULL;
    AjPTable energytable = NULL;

    AjPStr       nucs   = NULL;
    const AjPStr valstr = NULL;

    const char * dinuc = NULL;
    ajint    len;
    ajint    begin;
    ajint    end;
    ajint    i;
    float  val;
    float  anglesum  = 0.0;
    float  energysum = 0.0;
    float  twists    = 0.0;

    float  basesperturn  = 0.0;
    float  energyperbase = 0.0;

    embInit ("btwisted", argc, argv);

    seq    = ajAcdGetSeq ("sequence");
    angles  = ajAcdGetDatafile("angledata");
    energies  = ajAcdGetDatafile("energydata");
    result = ajAcdGetOutfile ("outfile");


    nucs = ajStrNew();

    angletable  = btwisted_getdinucdata(angles);
    energytable = btwisted_getdinucdata(energies);

    ajFileClose(&angles);
    ajFileClose(&energies);

    begin = ajSeqGetBegin(seq);
    end   = ajSeqGetEnd(seq);

    len   = end-begin+1;

    dinuc = ajSeqGetSeqC(seq);

    for(i=begin-1; i<end-1; ++i)
    {
	ajStrAssignSubC(&nucs,dinuc,i,i+1);
	if(!(valstr = ajTableFetchS(angletable, nucs)))
	    ajFatal("Incomplete table");

	ajStrToFloat(valstr,&val);
	anglesum += val;
	if(!(valstr = ajTableFetchS(energytable, nucs)))
	    ajFatal("Incomplete table");

	ajStrToFloat(valstr,&val);
	energysum += val;
    }

    twists        = anglesum / (float)360.0 ;
    basesperturn  = (float) len * (float)360.0 /anglesum;
    energyperbase = energysum/(float) (len-1);

    ajFmtPrintF(result, "# Output from BTWISTED\n");
    ajFmtPrintF(result, "# Twisting calculated from %d to %d of %s\n",
		begin, end, ajSeqGetNameC(seq));
    ajFmtPrintF(result,"Total twist (degrees): %.1f\n", anglesum);
    ajFmtPrintF(result,"Total turns : %.2f\n", twists);
    ajFmtPrintF(result,"Average bases per turn: %.2f\n", basesperturn);
    ajFmtPrintF(result,"Total stacking energy : %.2f\n", energysum);
    ajFmtPrintF(result,"Average stacking energy per dinucleotide: %.2f\n",
		energyperbase);

    ajTablestrFree(&angletable);
    ajTablestrFree(&energytable);

    ajStrDel(&nucs);
    ajFileClose(&result);
    ajSeqDel(&seq);

    embExit ();

    return 0;
}




/* @funcstatic btwisted_getdinucdata ******************************************
**
** Undocumented.
**
** @param [u] inf [AjPFile] Data file
** @return [AjPTable] Data values table
** @@
******************************************************************************/

static AjPTable btwisted_getdinucdata(AjPFile inf)
{
    AjPStr valstr   = NULL;
    AjPStr key      = NULL;
    AjPStr line     = NULL;
    AjPStrTok token = NULL;
    AjPTable table  = NULL;

    valstr = ajStrNew();
    line = ajStrNew();

    table = ajTablestrNewCase(20);

    while(ajReadlineTrim(inf,&line))
    {
	if(*ajStrGetPtr(line)=='#')
	    continue;
	token = ajStrTokenNewC(line," \n\t\r");
	key   = ajStrNew();
	ajStrTokenNextParseC(token," \n\t\r",&key);
	valstr = ajStrNew();
	ajStrTokenNextParseC(token," \n\t\r",&valstr);
	ajTablePut(table,(void *)key,(void *) valstr);
	ajStrTokenDel(&token);
    }


    ajStrDel(&line);

    return table;
}
