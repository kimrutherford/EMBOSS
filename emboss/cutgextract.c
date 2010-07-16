/* @source cutgextract application
**
** Create EMBOSS codon usage files from the CUTG database
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

#include "emboss.h"

#define CODONS 64
#define TABLE_ESTIMATE 1000




/* @datastatic CutgPValues ****************************************************
**
** Codon usage table data values
**
** @alias CutgSValues
** @alias CutgOValues
**
** @attr Count [ajint[CODONS]] Number of occurrences for each codon
**                             in standard order
** @attr Division [AjPStr] EMBL/GenBank division
** @attr Doc [AjPStr] Documentation string
** @attr Species [AjPStr] Species
** @attr Warn [ajint] Number of warnings issued
** @attr Skip [ajint] Number of CDSs skipped
** @attr CdsCount [ajint] Number of CDSs counted
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct CutgSValues
{
    ajint Count[CODONS];
    AjPStr Division;
    AjPStr Doc;
    AjPStr Species;
    ajint Warn;
    ajint Skip;
    ajint CdsCount;
    char  Padding[4];
} CutgOValues;
#define CutgPValues CutgOValues*

static AjPStr cutgextractSavepid = NULL;
static AjPStr cutgextractLine = NULL;
static AjPStr cutgextractOrg  = NULL;

static char *cutgextract_next(AjPFile inf, const AjPStr wildspecies,
			      AjPStr* pspecies, AjPStr* pdoc);
static ajint cutgextract_readcodons(AjPFile inf, AjBool allrecords,
				    ajint *count);




/* @prog cutgextract **********************************************************
**
** Extract data from CUTG.
**
** Reads all *.codon files in the input, and builds a table for each organism.
** Writes the tables to the EMBOSS_DATA/CODONS directory at the end.
**
******************************************************************************/

int main(int argc, char **argv)
{
    const char *codons[]=
    {
	"TAG","TAA","TGA","GCG","GCA","GCT","GCC","TGT", /* 00-07 */
	"TGC","GAT","GAC","GAA","GAG","TTT","TTC","GGT", /* 08-15 */
	"GGG","GGA","GGC","CAT","CAC","ATA","ATT","ATC", /* 16-23 */
	"AAA","AAG","CTA","TTA","TTG","CTT","CTC","CTG", /* 24-31 */
	"ATG","AAT","AAC","CCG","CCA","CCT","CCC","CAA", /* 32-39 */
	"CAG","CGT","CGA","CGC","AGG","AGA","CGG","TCG", /* 40-47 */
	"TCA","AGT","TCT","TCC","AGC","ACG","ACT","ACA", /* 48-55 */
	"ACC","GTA","GTT","GTC","GTG","TGG","TAT","TAC"	 /* 56-63 */
    };

    const char *aa=
	"***AAAACCDDEEFFGGGGHHIIIKKLLLLLLMNNPPPPQQRRRRRRSSSSSSTTTTVVVVWYY";

    AjPFile inf     = NULL;
    AjPFile outf    = NULL;
    char *entryname = NULL;
    AjPStr fname    = NULL;
    AjPStr key      = NULL;
    AjPStr tmpkey   = NULL;
    AjBool allrecords = AJFALSE;

    AjPTable table  = NULL;
    ajint i = 0;
    ajint j = 0;
    ajint k = 0;
    ajint x = 0;
    ajint savecount[3];

    AjPStr *keyarray = NULL;
    CutgPValues *valarray = NULL;
    AjPCod codon  = NULL;
    ajint sum = 0;
    char c;

    AjPList flist = NULL;
    AjPFile logf = NULL;
    AjPStr  entry = NULL;
    AjPStr  baseentry = NULL;
    AjPStr  wild  = NULL;
    AjPStr division = NULL;
    AjPStr release = NULL;
    AjPStr wildspecies = NULL;
    CutgPValues value = NULL;
    AjPStr docstr = NULL;
    AjPStr species = NULL;
    AjPStr filename = NULL;
    ajint nstops;

    embInit("cutgextract",argc,argv);

    tmpkey = ajStrNew();
    fname  = ajStrNew();


    table = ajTablestrNewLen(TABLE_ESTIMATE);


    flist = ajAcdGetDirlist("directory");
    wild  = ajAcdGetString("wildspec");
    release  = ajAcdGetString("release");
    logf = ajAcdGetOutfile("outfile");
    wildspecies = ajAcdGetString("species");
    filename = ajAcdGetString("filename");
    allrecords = ajAcdGetBoolean("allrecords");

    ajStrInsertC(&release, 0, "CUTG");
    ajStrRemoveWhite(&release);

    while(ajListPop(flist,(void **)&entry))
    {
	ajStrAssignS(&baseentry, entry);
	ajFilenameTrimPath(&baseentry);
	ajDebug("Testing file '%S'\n", entry);
	if(!ajStrMatchWildS(baseentry,wild))
	{
	    ajStrDel(&entry);
	    continue;
	}

	ajDebug("... matched wildcard '%S'\n", wild);
	inf = ajFileNewInNameS(entry);
	if(!inf)
	    ajFatal("cannot open file %S",entry);

	ajFmtPrintS(&division, "%F", inf);
	ajFilenameTrimAll(&division);

	while((entryname = cutgextract_next(inf, wildspecies,
					    &species, &docstr)))
	{
	    if(ajStrGetLen(filename))
		ajStrAssignS(&tmpkey,filename);
	    else
		ajStrAssignC(&tmpkey,entryname);

	    /* See if organism is already in the table */
	    value = ajTableFetch(table,tmpkey);
	    if(!value)			/* Initialise */
	    {
		key = ajStrNewS(tmpkey);
		AJNEW0(value);
		ajStrAssignS(&value->Species,species);
		ajStrAssignS(&value->Division, division);
		ajTablePut(table,(void *)key,(void *)value);
	    }
	    for(k=0;k<3;k++)
		savecount[k] = value->Count[k];
	    nstops = cutgextract_readcodons(inf,allrecords, value->Count);
	    if(nstops < 1)
	    {
		value->Skip++;
		continue;
	    }
	    value->CdsCount++;
	    if(nstops>1)
	    {
		value->CdsCount += (nstops - 1);
		value->Warn++;
		ajWarn("Found %d stop codons (%d %d %d) for CDS '%S'",
		       nstops,
		       value->Count[0] - savecount[0],
		       value->Count[1] - savecount[1],
		       value->Count[2] - savecount[2],
		       cutgextractSavepid);
	    }
	}
	ajStrDel(&entry);
	ajFileClose(&inf);
    }

    ajTableToarrayKeysValues(table,(void***) &keyarray, (void***) &valarray);

    i = 0;
    while(keyarray[i])
    {
	key   = keyarray[i];
	value = (CutgPValues) valarray[i++];
	codon = ajCodNew();
	sum   = 0;
	for(j=0;j<CODONS;++j)
	{
	    sum += value->Count[j];
	    x = ajCodIndexC(codons[j]);
	    codon->num[x] = value->Count[j];

	    c = aa[j];
	    if(c=='*')
		codon->aa[x] = 27;
	    else
		codon->aa[x] = c-'A';
	}
	ajCodCalcUsage(codon,sum);

	ajStrAppendC(&key, ".cut");
	if(allrecords)
	{
	    if(value->Warn)
		ajFmtPrintF(logf, "Writing %S CDS: %d Warnings: %d\n",
			    key, value->CdsCount, value->Warn);
	    else
		ajFmtPrintF(logf, "Writing %S CDS: %d\n",
			    key, value->CdsCount);
	}
	else
	{
	    if(value->Skip)
		ajFmtPrintF(logf, "Writing %S CDS: %d Skipped: %d\n",
			    key, value->CdsCount, value->Skip);
	    else
		ajFmtPrintF(logf, "Writing %S CDS: %d\n",
			    key, value->CdsCount);
	}

	ajFmtPrintS(&fname,"CODONS/%S",key);
	outf = ajDatafileNewOutNameS(fname);
	if(!outf)
	    ajFatal("Cannot open output file %S",fname);

	ajCodSetNameS(codon, key);
	ajCodSetSpeciesS(codon, value->Species);
	ajCodSetDivisionS(codon, value->Division);
	ajCodSetReleaseS(codon, release);
	ajCodSetNumcds(codon, value->CdsCount);
	ajCodSetNumcodons(codon, sum);

	ajCodWrite(codon, outf);
	ajFileClose(&outf);


	ajStrDel(&key);
	ajStrDel(&value->Division);
	ajStrDel(&value->Doc);
	ajStrDel(&value->Species);
	AJFREE(value);
	ajCodDel(&codon);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableFree(&table);
    ajListFree(&flist);
    ajStrDel(&wild);
    ajStrDel(&release);
    ajStrDel(&wildspecies);
    ajStrDel(&filename);
    ajFileClose(&logf);

    ajStrDel(&cutgextractSavepid);
    ajStrDel(&cutgextractLine);
    ajStrDel(&cutgextractOrg);

    ajStrDel(&fname);
    ajStrDel(&tmpkey);
    ajStrDel(&species);
    ajStrDel(&docstr);
    ajStrDel(&division);
    ajStrDel(&baseentry);

    embExit();

    return 0;
}




/* @funcstatic cutgextract_next ***********************************************
**
** Reads the first line of a CUTG database entry,
** returning the name of the species.
**
** Each entry has one line beginning with '>' followed by fields
** delimited by a backslash:
**
** GenBank entry name
** GenBank accession number
** GenBank feature location
** Length of feature in nucleotides
** Protein ID
** Genus and species
** Genbank entry description and feature qualifiers
**
** The second line is 64 integers giving the number of times each codon
** appears in this coding sequence.
**
** @param [u] inf [AjPFile] Input CUTG database file
** @param [r] wildspecies [const AjPStr] Wildcard species to select
** @param [w] pspecies [AjPStr*] Species for this entry
** @param [w] pdoc [AjPStr*] Documentation for this entry
** @return [char*] Undocumented
** @@
******************************************************************************/

static char* cutgextract_next(AjPFile inf, const AjPStr wildspecies,
			      AjPStr* pspecies, AjPStr* pdoc)
{
    AjPStrTok handle    = NULL;
    AjPStr token = NULL;
    ajint i;
    ajint len;
    char *p = NULL;
    char c;
    AjBool done = ajFalse;

    if(!cutgextractLine)
	cutgextractLine = ajStrNew();

    if(!cutgextractOrg)
	cutgextractOrg  = ajStrNew();

    ajStrAssignC(&cutgextractLine,"");
    ajStrAssignC(pdoc,"");
    while (!done)
    {

	while(ajStrGetCharFirst(cutgextractLine) != '>')
	    if(!ajReadlineTrim(inf,&cutgextractLine))
		return NULL;

	handle = ajStrTokenNewC(cutgextractLine,"\\\n\t\r");
	for(i=0;i<7;++i) {
	    ajStrTokenNextParseC(&handle,"\\\n\t\r",&token);
	    if(i==5)
	    {
		ajStrAssignC(&cutgextractOrg,"E");
		ajStrAppendS(&cutgextractOrg, token);
		ajStrAssignS(pspecies, token);
		if(ajStrMatchWildS(token,wildspecies))
		{
		    done = ajTrue;
		}
	    }

	    switch(i)
	    {
	    case 0:
		ajStrAppendC(pdoc, "#ID ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		break;
	    case 1:
		ajStrAppendC(pdoc, "#AC ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		break;
	    case 2:
		ajStrAppendC(pdoc, "#FT ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		break;
	    case 3:
		ajStrAppendC(pdoc, "#FL ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		break;
	    case 4:
		ajStrAppendC(pdoc, "#PI ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		ajStrAssignS(&cutgextractSavepid, token);
		break;
	    case 5:
		ajStrAppendC(pdoc, "#OS ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		break;
	    case 6:
		ajStrAppendC(pdoc, "#DE ");
		ajStrAppendS(pdoc, token);
		ajStrAppendC(pdoc, "\n");
		break;
	    default:
		break;
	    }
	}

	ajStrTokenDel(&handle);
	ajStrDel(&token);
	if(!done)
	    if(!ajReadlineTrim(inf,&cutgextractLine))
		return NULL;
    }

    len = ajStrGetLen(cutgextractOrg);
    p   = ajStrGetuniquePtr(&cutgextractOrg);
    for(i=0;i<len;++i)
    {
	c = p[i];
	if(c=='/' || c==' ' || c=='.' || c=='\'')
	    p[i]='_';
    }

    if(p[strlen(p)-1]=='_')
	p[strlen(p)-1]='\0';
    ajStrDel(&token);
    ajStrTokenDel(&handle);

    return p;
}




/* @funcstatic cutgextract_readcodons *****************************************
**
** Reads the codon frequency line from a CUTG database entry.
**
** Codons are reported according to the number of codons per amino acid
** (6, 4, 2, 3, 1, stop) in the order:
** 
** CGA CGC CGG CGU AGA AGG CUA CUC CUG CUU UUA UUG UCA UCC UCG UCU AGC AGU
**  R   R   R   R   R   R   L   L   L   L   L   L   S   S   S   S   S   S
** ACA ACC ACG ACU CCA CCC CCG CCU GCA GCC GCG GCU GGA GGC GGG GGU
**  T   T   T   T   P   P   P   P   A   A   A   A   G   G   G   G
** GUA GUC GUG GUU AAA AAG AAC AAU CAA CAG CAC CAU GAA GAG GAC GAU
**  V   V   V   V   K   K   N   N   Q   Q   H   H   E   E   D   D
** UAC UAU UGC UGU UUC UUU AUA AUC AUU AUG UGG UAA UAG UGA
**  Y   Y   C   C   F   F   I   I   I   M   W   *   *   *
** @param [u] inf [AjPFile] Input CUTG database file
** @param [r] allrecords [AjBool] If false, skip bad records with more than
**                             one stop codon in the standard genetic code
** @param [w] count [ajint*] Codon usage total so far for this species
** @return [ajint] Number of stop codons, assuming a standard genetic code
** @@
******************************************************************************/

static ajint cutgextract_readcodons(AjPFile inf, AjBool allrecords,
				    ajint *count)
{
    static int cutidx[] =
    {
	42,43,46,41,45,44,26,30,31,29,27,28,48,51,47,50,
	52,49,55,56,53,54,36,38,35,37, 4, 6, 3, 5,17,18,
	16,15,57,59,60,58,24,25,34,33,39,40,20,19,11,12,
	10, 9,63,62, 8, 7,14,13,21,23,22,32,61, 1, 0, 2
    };
    AjPStr line  = NULL;
    AjPStr value = NULL;
    ajint thiscount[64];

    AjPStrTok token = NULL;
    ajint i;
    ajint n = 0;
    ajint nstops = 0;


    if(!line)
    {
	line  = ajStrNew();
	value = ajStrNew();
    }

    if(!ajReadlineTrim(inf,&line))
	ajFatal("Premature end of file");


    token = ajStrTokenNewC(line," \n\t\r");
    for(i=0;i<CODONS;++i)
    {
	ajStrTokenNextParseC(&token," \n\t\r",&value);
	ajStrToInt(value,&n);
	thiscount[cutidx[i]] = n;
	if(i>60)
	    nstops += n;
    }

    ajStrDel(&line);
    ajStrDel(&value);
    ajStrTokenDel(&token);

    if(!allrecords)
	if(nstops > 1)
	    return -1;

    for(i=0;i<CODONS;++i)
    {
	count[i] += thiscount[i];
    }	

    return nstops;
}
