/* @source inforesidue application
**
** Return information on amino acid residues
** @author Copyright (C) Jon Ison (jison@ebi.ac.uk)
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
#include "math.h"

#define DAYHOFF_FILE "Edayhoff.freq"

/* @prog inforesidue ***********************************************************
**
** Return information on amino acid residues
**
******************************************************************************/



int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPStr code = NULL;
    AjPFile mfptr = NULL;
    AjPFile wfptr = NULL;
    AjPFile outf = NULL;

    EmbPPropMolwt *mwdata;
    EmbPPropAmino *aadata;

    float *dhstat = NULL;

    char    code1;
    AjPStr  code3 = NULL;
    ajint idx    = 0;
    ajuint i;
    ajuint iend;
    AjPStr  propstr = NULL;
    float charge;
    char csign;

    /* ACD File Processing */
    embInit("inforesidue", argc, argv);
    code = ajAcdGetString("code");
    mfptr   = ajAcdGetDatafile("aadata");
    wfptr   = ajAcdGetDatafile("mwdata");
    outf = ajAcdGetOutfile("outfile");

    aadata = embPropEaminoRead(mfptr);
    mwdata = embPropEmolwtRead(wfptr);
    if(!embReadAminoDataFloatC(DAYHOFF_FILE,&dhstat,(float)0.001))
	ajFatal("Set the EMBOSS_DATA environment variable");


    /* Application logic */

    ajStrFmtUpper(&code);
    iend = ajStrGetLen(code);
    ajFmtPrintF(outf, "%-4s %-5s %-20s %6s %9s %-30s %s\n",
                "Code", "Short", "Mnemonic",
                "Charge", "MolWt",
                "Properties", "Ambiguity");
    for(i=0;i<iend;i++)
    {
        code1=ajStrGetCharPos(code,i);
        if(ajResidueExistsChar(code1))
        {
            idx = ajBasecodeToInt(code1);
            ajResidueToTriplet(code1, &code3);
            ajStrFmtTitle(&code3);

            if(!embPropGetProperties(aadata[idx], &propstr))
                ajStrAssignC(&propstr, "(none)");

            charge = embPropGetCharge(aadata[idx]);
            if(charge > 0.0)
                csign = '+';
            else if(charge < 0.0)
                csign = '-';
            else
                csign = ' ';

            ajFmtPrintF(outf, "%-4c %-5S %-20S %3c%3.1f %9.4f %-30S %S\n",
                        code1, code3, ajResidueGetMnemonic(code1),
                        csign, fabs(charge),
                        embPropMolwtGetMolwt(mwdata[idx]),
                        propstr, ajResidueGetCodes(code1));
        }
        else
        {
            ajFmtPrintF(outf, "%-4c %-5s %-20s %6s %9s %-30s %s\n",
                        code1, ".", "invalid", ".", ".", ".", ".");

        }
        
    }
    
    

    /* Memory management and exit */
    ajStrDel(&code);
    ajFileClose(&outf);

    embExit();

    return 0;
}

