/******************************************************************************
** @source AJAX nexus file parsing functions
**
** @author Copyright (C) 2003 Peter Rice
** @version 1.0
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"

/* TO DO *********************************************************
**
** Quoted strings converted to words with underscores
** Allow '' to be '
**
******************************************************************************/

static const char* nexusCharactersDataTypes[] =
{
    "standard",
    "dna",
    "rna",
    "nucleotide",
    "protein",
    "continuous",
    NULL
};

static const char* nexusCharactersStatesFormat[] =
{
    "statespresent",
    "individuals",
    "count",
    "frequency",
    NULL
};

static const char* nexusUnalignedDataTypes[] =
{
    "standard",
    "dna",
    "rna",
    "nucleotide",
    "protein",
    NULL
};

static const char* nexusAssumptionsDefType[] =
{
    "unord",
    "ord",
    "irrev",
    "irrev.up",
    "irrev.dn",
    "dollo",
    "dollo.up",
    "dollo.dn",
    "strat",
    NULL
};

static const char* nexusAssumptionsPolyTCount[] =
{
    "minsteps",
    "maxsteps",
    NULL
};

static const char* nexusAssumptionsGapMode[] =
{
    "missing",
    "newstate",
    NULL
};

static const char* nexusDistancesTriangle[] =
{
    "lower",
    "upper",
    "both",
    NULL
};

static void                nexusArrayDel(AjPStr** pthis);
static AjBool              nexusBlockSave(AjPNexus thys, AjPList list,
					  const AjPStr blockname);
static AjBool              nexusCommand(AjPList list, AjPStr* command,
					AjPStr* cmdstr);
static void                nexusAssumptionsDel(AjPNexusAssumptions* pthys);
static AjPNexusAssumptions nexusAssumptionsNew(void);
static void                nexusCharactersDel(AjPNexusCharacters* pthys);
static AjPNexusCharacters  nexusCharactersNew(void);
static void                nexusCodonsDel(AjPNexusCodons* pthys);
static AjPNexusCodons      nexusCodonsNew(void);
static void                nexusDistancesDel(AjPNexusDistances* pthys);
static AjPNexusDistances   nexusDistancesNew(void);
static void                nexusNotesDel(AjPNexusNotes* pthys);
static AjPNexusNotes       nexusNotesNew(void);
static void                nexusSetsDel(AjPNexusSets* pthys);
static AjPNexusSets        nexusSetsNew(void);
static void                nexusTaxaDel(AjPNexusTaxa* pthys);
static AjPNexusTaxa        nexusTaxaNew(void);
static void                nexusTreesDel(AjPNexusTrees* pthys);
static AjPNexusTrees       nexusTreesNew(void);
static void                nexusUnalignedDel(AjPNexusUnaligned* pthys);
static AjPNexusUnaligned   nexusUnalignedNew(void);
static AjBool              nexusParseAssumptions(AjPNexus thys, AjPList list);
static AjBool              nexusParseCharacters(AjPNexus thys, AjPList list,
						AjBool newtaxa);
static AjBool              nexusParseCodons(AjPNexus thys, AjPList list);
static AjBool              nexusParseDistances(AjPNexus thys, AjPList list);
static AjBool              nexusParseNotes(AjPNexus thys, AjPList list);
static AjBool              nexusParseSets(AjPNexus thys, AjPList list);
static AjBool              nexusParseTaxa(AjPNexus thys, AjPList list);
static AjBool              nexusParseTrees(AjPNexus thys, AjPList list);
static AjBool              nexusParseUnaligned(AjPNexus thys, AjPList list);
static AjBool              nexusGetBool(AjPStr src, AjPRegexp exp,
					ajint isub, AjBool* dest);
static AjBool              nexusGetInt(AjPStr src, AjPRegexp exp,
				       ajint isub, ajuint* dest);
static AjBool              nexusGetChar(AjPStr src, AjPRegexp exp,
				       ajint isub, char* dest);
static AjBool              nexusGetStr(AjPStr src, AjPRegexp exp,
				       ajint isub, AjPStr* dest);
static ajint               nexusGetArray(AjPStr src, AjPRegexp exp,
					 AjPStr** dest);
static AjBool              nexusVocab(const char* title, const AjPStr src,
				      const char* vocab[]);
static AjBool              nexusSetSequences(AjPNexus thys);




/* @func ajNexusParse *********************************************************
**
** Parses a nexus buffered file
**
** @param [u] buff [AjPFilebuff] Input buffered file
** @return [AjPNexus] Nexus data object
** @@
******************************************************************************/

AjPNexus ajNexusParse(AjPFilebuff buff)
{
    AjPNexus ret;
    AjPStr rdline = NULL;
    AjPList blocklist = NULL;
    ajint ok;

    ajint commentlevel = 0;
    AjBool block;
    AjPStr prestr = NULL;
    AjPStr poststr = NULL;
    AjPStr blockname = NULL;

    static AjPRegexp commexp = NULL;
    static AjPRegexp begcommexp = NULL;
    static AjPRegexp endcommexp = NULL;
    static AjPRegexp begexp = NULL;
    static AjPRegexp endexp = NULL;


    if (!commexp)
	commexp = ajRegCompC("[\\[]([^&][^\\[\\]]+)[\\]]$");

    if (!endcommexp)
	endcommexp = ajRegCompC("^([^\\]]*)[\\]]");

    if (!begcommexp)
	begcommexp = ajRegCompC("[\\[]([^\\[\\]]*)$");

    if (!begexp)
	begexp = ajRegCompC("^\\s*[Bb][Ee][Gg][Ii][Nn]\\s+(\\S+)\\s*;\\s*$");

    if (!endexp)
	endexp = ajRegCompC("^\\s*[Ee][Nn][Dd]\\S*;\\s*$");

    ajDebug("ajNexusParse\n");

    ajBuffreadLine(buff, &rdline);

    if(!ajStrPrefixCaseC(rdline, "#NEXUS"))
    {				/* first line test */
	ajFilebuffReset(buff);

	return NULL;
    }

    ajDebug("ajNexusParse: First line ok '%S'\n", rdline);

    ok = ajBuffreadLine(buff, &rdline);
    if(!ok)
	return NULL;

    block = ajFalse;
    commentlevel = 0;
    blocklist = ajListstrNew();
    ret = ajNexusNew();

    while(ok)
    {
	ajStrTrimWhiteEnd(&rdline);
	/* first trim any comments from the input string */

	while (commentlevel && ajRegExec(endcommexp, rdline))
	{
	    ajRegPost(endcommexp, &poststr);
	    ajStrAssignS(&rdline, poststr);
	    --commentlevel;
	    ajDebug("end comment found: %S\n", rdline);
	}

	while (ajRegExec(commexp, rdline))
	{
	    ajRegPre(commexp, &prestr);
	    ajRegPost(commexp, &poststr);
	    ajStrAssignS(&rdline, prestr);
	    ajStrAppendS(&rdline, poststr);
	    ajDebug("comment found: %S\n", rdline);
	}

	while (ajRegExec(begcommexp, rdline))
	{
	    ajRegPre(begcommexp, &prestr);
	    ajStrAssignS(&rdline, prestr);
	    ++commentlevel;
	    ajDebug("begin comment found: %S\n", rdline);
	}

	if (commentlevel)
	{
	    ajDebug("comments only ... skipping\n");
	}
	else if (!block && ajRegExec(begexp, rdline)) /* begin line */
	{
	    block = ajTrue;
	    ajRegSubI(begexp, 1, &blockname);
	}
	else if (block && ajRegExec(endexp, rdline)) /* end line */
	{
	    if (!nexusBlockSave(ret, blocklist, blockname))
		return NULL;

	    ajStrAssignClear(&blockname);
	    block = ajFalse;
	}
	else if (block)				/* the rest */
	{
	    ajStrRemoveWhiteExcess(&rdline);

	    if (ajStrGetLen(rdline))
	    {
		ajListstrPushAppend(blocklist, rdline);
	    }

	    rdline = NULL;
	}

	ok = ajBuffreadLine(buff, &rdline);
    }

    if (block)
    {
	ajDebug("ajNexusParse unclosed block '%S'\n", blockname);

	return NULL;
    }

    ajNexusTrace(ret);

    return ret;
}




/* @func ajNexusNew ***********************************************************
**
** Constructor for AjPNexus
**
** @return [AjPNexus] New AjPNexus object
** @category new [AjPNexus] Default constructor
******************************************************************************/

AjPNexus ajNexusNew(void)
{
    AjPNexus thys;

    AJNEW0(thys);

    return thys;
}




/* @funcstatic nexusTaxaNew ***********************************************
**
** Constructor for AjPNexusTaxa
**
** @return [AjPNexusTaxa] New AjPNexusTaxa object
** @category new [AjPNexusTaxa] Default constructor
******************************************************************************/

static AjPNexusTaxa nexusTaxaNew(void)
{
    AjPNexusTaxa thys;

    AJNEW0(thys);

    return thys;
}




/* @funcstatic nexusCharactersNew *********************************************
**
** Constructor for AjPNexusCharacters
**
** @return [AjPNexusCharacters] New AjPNexusCharacters object
** @category new [AjPNexusCharacters] Default constructor
******************************************************************************/

static AjPNexusCharacters nexusCharactersNew(void)
{
    AjPNexusCharacters thys;

    AJNEW0(thys);

    ajStrAssignC(&thys->DataType, "standard");
    thys->NewTaxa = ajFalse;
    thys->Missing = '?';
    ajStrAssignC(&thys->Symbols, "0 1");
    thys->Labels = ajTrue;
    thys->Interleave = ajFalse;
    thys->Tokens = ajTrue;
    
    return thys;
}




/* @funcstatic nexusUnalignedNew **********************************************
**
** Constructor for AjPNexusUnaligned
**
** @return [AjPNexusUnaligned] New AjPNexusUnaligned object
** @category new [AjPNexusUnaligned] Default constructor
******************************************************************************/

static AjPNexusUnaligned nexusUnalignedNew(void)
{
    AjPNexusUnaligned thys;

    AJNEW0(thys);

    thys->NewTaxa = ajFalse;
    ajStrAssignC(&thys->DataType, "standard");
    thys->Missing = '?';
    thys->Labels = ajTrue;


    return thys;
}




/* @funcstatic nexusDistancesNew **********************************************
**
** Constructor for AjPNexusDistances
**
** @return [AjPNexusDistances] New AjPNexusDistances object
** @category new [AjPNexusDistances] Default constructor
******************************************************************************/

static AjPNexusDistances nexusDistancesNew(void)
{
    AjPNexusDistances thys;

    AJNEW0(thys);
    thys->NewTaxa = ajFalse;
    thys->Missing = '?';
    ajStrAssignC(&thys->Triangle, "lower");
    thys->Diagonal = ajTrue;
    thys->Labels = ajTrue;
    thys->Interleave = ajFalse;

    return thys;
}




/* @funcstatic nexusSetsNew ***********************************************
**
** Constructor for AjPNexusSets
**
** @return [AjPNexusSets] New AjPNexusSets object
** @category new [AjPNexusSets] Default constructor
******************************************************************************/

static AjPNexusSets nexusSetsNew(void)
{
    AjPNexusSets thys;

    AJNEW0(thys);

    return thys;
}




/* @funcstatic nexusAssumptionsNew ********************************************
**
** Constructor for AjPNexusAssumptions
**
** @return [AjPNexusAssumptions] New AjPNexusAssumptions object
** @category new [AjPNexusAssumptions] Default constructor
******************************************************************************/

static AjPNexusAssumptions nexusAssumptionsNew(void)
{
    AjPNexusAssumptions thys;

    AJNEW0(thys);

    ajStrAssignC(&thys->DefType, "unord");
    ajStrAssignC(&thys->GapMode, "missing");

    return thys;
}




/* @funcstatic nexusCodonsNew ***********************************************
**
** Constructor for AjPNexusCodons
**
** @return [AjPNexusCodons] New AjPNexusCodons object
** @category new [AjPNexusCodons] Default constructor
******************************************************************************/

static AjPNexusCodons nexusCodonsNew(void)
{
    AjPNexusCodons thys;

    AJNEW0(thys);

    return thys;
}




/* @funcstatic nexusTreesNew ***********************************************
**
** Constructor for AjPNexusTrees
**
** @return [AjPNexusTrees] New AjPNexusTrees object
** @category new [AjPNexusTrees] Default constructor
******************************************************************************/

static AjPNexusTrees nexusTreesNew(void)
{
    AjPNexusTrees thys;

    AJNEW0(thys);

    return thys;
}




/* @funcstatic nexusNotesNew ***********************************************
**
** Constructor for AjPNexusNotes
**
** @return [AjPNexusNotes] New AjPNexusNotes object
** @category new [AjPNexusNotes] Default constructor
******************************************************************************/

static AjPNexusNotes nexusNotesNew(void)
{
    AjPNexusNotes thys;

    AJNEW0(thys);

    return thys;
}




/* @func ajNexusDel ***********************************************************
**
** Destructor for AjPNexus
**
** @param [d] pthys [AjPNexus*] AjPNexus object
** @return [void]
** @category delete [AjPNexus] Default destructor
******************************************************************************/

void ajNexusDel(AjPNexus* pthys)
{
    AjPNexus thys;

    if (!*pthys)
        return;

    thys = *pthys;

    nexusTaxaDel(&thys->Taxa);
    nexusCharactersDel(&thys->Characters);
    nexusUnalignedDel(&thys->Unaligned);
    nexusDistancesDel(&thys->Distances);
    nexusSetsDel(&thys->Sets);
    nexusAssumptionsDel(&thys->Assumptions);
    nexusCodonsDel(&thys->Codons);
    nexusTreesDel(&thys->Trees);
    nexusNotesDel(&thys->Notes);

    AJFREE(*pthys);
}




/* @funcstatic nexusTaxaDel ***************************************************
**
** Destructor for AjPNexusTaxa
**
** @param [d] pthys [AjPNexusTaxa*] AjPNexusTaxa object
** @return [void]
** @category delete [AjPNexusTaxa] Default destructor
******************************************************************************/

static void nexusTaxaDel(AjPNexusTaxa* pthys)
{
    AjPNexusTaxa thys;

    if (!*pthys)
        return;

    thys = *pthys;

    nexusArrayDel(&thys->TaxLabels);

    AJFREE(*pthys);
}




/* @funcstatic nexusCharactersDel *********************************************
**
** Destructor for AjPNexusCharacters
**
** @param [d] pthys [AjPNexusCharacters*] AjPNexusCharacters object
** @return [void]
** @category delete [AjPNexusCharacters] Default destructor
******************************************************************************/

static void nexusCharactersDel(AjPNexusCharacters* pthys)
{
    AjPNexusCharacters thys;

    if (!*pthys)
        return;

    thys = *pthys;

    ajStrDel(&thys->DataType);
    ajStrDel(&thys->Symbols);
    ajStrDel(&thys->Equate);
    ajStrDel(&thys->Items);
    ajStrDel(&thys->StatesFormat);
    ajStrDel(&thys->Eliminate);

    nexusArrayDel(&thys->CharStateLabels);
    nexusArrayDel(&thys->CharLabels);
    nexusArrayDel(&thys->StateLabels);
    nexusArrayDel(&thys->Matrix);
    nexusArrayDel(&thys->Sequences);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusUnalignedDel **********************************************
**
** Destructor for AjPNexusUnaligned
**
** @param [d] pthys [AjPNexusUnaligned*] AjPNexusUnaligned object
** @return [void]
** @category delete [AjPNexusUnaligned] Default destructor
******************************************************************************/

static void nexusUnalignedDel(AjPNexusUnaligned* pthys)
{
    AjPNexusUnaligned thys;

    if (!*pthys)
        return;

    thys = *pthys;

    ajStrDel(&thys->DataType);
    ajStrDel(&thys->Symbols);
    ajStrDel(&thys->Equate);

    nexusArrayDel(&thys->Matrix);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusDistancesDel **********************************************
**
** Destructor for AjPNexusDistances
**
** @param [d] pthys [AjPNexusDistances*] AjPNexusDistances object
** @return [void]
** @category delete [AjPNexusDistances] Default destructor
******************************************************************************/

static void nexusDistancesDel(AjPNexusDistances* pthys)
{
    AjPNexusDistances thys;

    if (!*pthys)
        return;

    thys = *pthys;

    ajStrDel(&thys->Triangle);

    nexusArrayDel(&thys->Matrix);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusSetsDel ***************************************************
**
** Destructor for AjPNexusSets
**
** @param [d] pthys [AjPNexusSets*] AjPNexusSets object
** @return [void]
** @category delete [AjPNexusSets] Default destructor
******************************************************************************/

static void nexusSetsDel(AjPNexusSets* pthys)
{
    AjPNexusSets thys;

    if (!*pthys)
        return;

    thys = *pthys;

    nexusArrayDel(&thys->CharSet);
    nexusArrayDel(&thys->StateSet);
    nexusArrayDel(&thys->ChangeSet);
    nexusArrayDel(&thys->TaxSet);
    nexusArrayDel(&thys->TreeSet);
    nexusArrayDel(&thys->CharPartition);
    nexusArrayDel(&thys->TaxPartition);
    nexusArrayDel(&thys->TreePartition);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusAssumptionsDel ********************************************
**
** Destructor for AjPNexusAssumptions
**
** @param [d] pthys [AjPNexusAssumptions*] AjPNexuAssumptionss object
** @return [void]
** @category delete [AjPNexusAssumptions] Default destructor
******************************************************************************/

static void nexusAssumptionsDel(AjPNexusAssumptions* pthys)
{
    AjPNexusAssumptions thys;

    if (!*pthys)
        return;

    thys = *pthys;

    ajStrDel(&thys->DefType);
    ajStrDel(&thys->PolyTCount);
    ajStrDel(&thys->GapMode);
    nexusArrayDel(&thys->UserType);
    nexusArrayDel(&thys->TypeSet);
    nexusArrayDel(&thys->WtSet);
    nexusArrayDel(&thys->ExSet);
    nexusArrayDel(&thys->AncStates);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusCodonsDel *************************************************
**
** Destructor for AjPNexusCodons
**
** @param [d] pthys [AjPNexusCodons*] AjPNexusCodons object
** @return [void]
** @category delete [AjPNexusCodons] Default destructor
******************************************************************************/

static void nexusCodonsDel(AjPNexusCodons* pthys)
{
    AjPNexusCodons thys;

    if(!*pthys)
        return;

    thys = *pthys;

    nexusArrayDel(&thys->CodonPosSet);
    nexusArrayDel(&thys->GeneticCode);
    nexusArrayDel(&thys->CodeSet);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusTreesDel **************************************************
**
** Destructor for AjPNexusTrees
**
** @param [d] pthys [AjPNexusTrees*] AjPNexusTrees object
** @return [void]
** @category delete [AjPNexusTrees] Default destructor
******************************************************************************/

static void nexusTreesDel(AjPNexusTrees* pthys)
{
    AjPNexusTrees thys;

    if (!*pthys)
        return;

    thys = *pthys;

    nexusArrayDel(&thys->Translate);
    nexusArrayDel(&thys->Tree);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusNotesDel ************************************************
**
** Destructor for AjPNexusNotes
**
** @param [d] pthys [AjPNexusNotes*] AjPNexusNotes object
** @return [void]
** @category delete [AjPNexusNotes] Default destructor
******************************************************************************/

static void nexusNotesDel(AjPNexusNotes* pthys)
{
    AjPNexusNotes thys;

    if (!*pthys)
        return;

    thys = *pthys;

    nexusArrayDel(&thys->Text);
    nexusArrayDel(&thys->Picture);

    AJFREE(*pthys);

    return;
}




/* @funcstatic nexusBlockSave *************************************************
**
** Saves a nexus block in the appropriate block type
**
** Note: data is saved as a character block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [w] list [AjPList] List of block records
** @param [r] blockname [const AjPStr] Block name
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusBlockSave(AjPNexus thys, AjPList list,
			     const AjPStr blockname)
{
    if (ajStrMatchCaseC(blockname, "taxa"))
    {
	if (!nexusParseTaxa(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "characters"))
    {
	if (!nexusParseCharacters(thys, list, ajFalse))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "unaligned"))
    {
	if (!nexusParseUnaligned(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "distances"))
    {
	if (!nexusParseDistances(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "sets"))
    {
	if (!nexusParseSets(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "assumptions"))
    {
	if (!nexusParseAssumptions(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "codons"))
    {
	if (!nexusParseCodons(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "trees"))
    {
	if (!nexusParseTrees(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "notes"))
    {
	if (!nexusParseNotes(thys, list))
	    return ajFalse;
    }
    else if (ajStrMatchCaseC(blockname, "data"))
    {
	if (!nexusParseCharacters(thys, list, ajTrue))
	    return ajFalse;
    }
    else
	ajDebug("nexusBlockSave: unknown block '%S'\n", blockname);

    return ajTrue;
}




/* @funcstatic nexusCommand ********************************************
**
** Finds the next command and command string in a block
**
** @param [u] list [AjPList] List of block records
** @param [w] command [AjPStr*] Command name
** @param [w] cmdstr [AjPStr*] Command string
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusCommand(AjPList list, AjPStr* command, AjPStr* cmdstr)
{
    AjPStr rdline = NULL;
    AjPStr tmpstr = NULL;

    static AjPRegexp begcommand = NULL;
    static AjPRegexp endcommand = NULL;

    AjBool incommand = ajFalse;

    if (!begcommand)
	begcommand = ajRegCompC("^\\s*(\\S*)\\s*");

    if (!endcommand)
	endcommand = ajRegCompC(";\\s*$");

    while (ajListstrPop(list, &rdline))
    {
	if (!incommand)
	{
	    if (ajRegExec(begcommand, rdline))
	    {
		ajRegSubI(begcommand, 1, command);
		ajRegPost(begcommand, cmdstr);

		if (ajRegExec(endcommand, *cmdstr))
		{
		    ajRegPre(endcommand, &tmpstr);
		    ajStrAssignS(cmdstr, tmpstr);

		    return ajTrue;
		}

		incommand = ajTrue;
	    }
	}
	else if (incommand)
	{
	    if (ajRegExec(endcommand, rdline))
	    {
		ajRegPre(endcommand, &tmpstr);
		ajStrAppendS(cmdstr, tmpstr);

		return ajTrue;
	    }
	    else
            {
		if (ajStrGetLen(*cmdstr))
		    ajStrAppendK(cmdstr, '\n');

		ajStrAppendS(cmdstr, rdline);
	    }
	}
    }

    return ajFalse;
}




/* @funcstatic nexusParseTaxa ********************************************
**
** Parses Nexus commands for a Taxa block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseTaxa(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;
    ajuint i;

    static AjPRegexp dimexp = NULL;
    static AjPRegexp taxlabexp = NULL;

    if (!dimexp)
	dimexp = ajRegCompCaseC("\\bntax\\s*=\\s*(\\d+)");

    if (!taxlabexp)
	taxlabexp = ajRegCompCaseC("\\S+");

    if (!thys->Taxa)
	thys->Taxa = nexusTaxaNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "dimensions"))
	{
	    if (!ajRegExec(dimexp, cmdstr))
	    {
		ajDebug("Invalid Taxa '%S'\n", command);
		break;
	    }

	    if (nexusGetInt(cmdstr, dimexp, 1,  &thys->Taxa->Ntax))
	    {
		if (!thys->Ntax)
		    thys->Ntax = thys->Taxa->Ntax;
	    }
	}
	else if (ajStrMatchCaseC(command, "taxlabels"))
	{
	    if (!thys->Taxa->Ntax)
	    {
		ajDebug("nexusParseTaxa failed: dimension Ntax not found\n");

		return ajFalse;
	    }

	    i = nexusGetArray(cmdstr, taxlabexp, &thys->Taxa->TaxLabels);

	    if (i != thys->Taxa->Ntax)
	    {
		ajDebug("nexusParseTaxa failed: Ntax %d read %d TaxLabels\n",
			thys->Taxa->Ntax, i);

		return ajFalse;
	    }
	}
    }

    if (!thys->Taxa->Ntax)
    {
	ajDebug("nexusParseTaxa failed: taxa data not found\n");

	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic nexusParseCharacters *******************************************
**
** Parses Nexus commands for a Characters block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @param [r] newtaxa [AjBool] If true, set NewTaxa
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseCharacters(AjPNexus thys, AjPList list, AjBool newtaxa)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;
    ajuint i;

    static AjPRegexp newlinexp = NULL;
    static AjPRegexp dimexp1 = NULL;
    static AjPRegexp dimexp2 = NULL;
    static AjPRegexp dimexp3 = NULL;
    static AjPRegexp formexp1 = NULL;
    static AjPRegexp formexp2 = NULL;
    static AjPRegexp formexp3 = NULL;
    static AjPRegexp formexp4 = NULL;
    static AjPRegexp formexp5 = NULL;
    static AjPRegexp formexp6 = NULL;
    static AjPRegexp formexp7 = NULL;
    static AjPRegexp formexp8 = NULL;
    static AjPRegexp formexp9 = NULL;
    static AjPRegexp formexp10 = NULL;
    static AjPRegexp formexp11 = NULL;
    static AjPRegexp formexp12 = NULL;
    static AjPRegexp formexp13 = NULL;
    static AjPRegexp taxlabexp = NULL;
    static AjPRegexp chstlabexp = NULL;
    static AjPRegexp chlabexp = NULL;
    static AjPRegexp optexp1 = NULL;

    if (!newlinexp)
    {
	newlinexp = ajRegCompC("[^\n]+");
	dimexp1 = ajRegCompCaseC("\\bnewtaxa\\b");
	dimexp2 = ajRegCompCaseC("\\bntax\\s*=\\s*(\\d+)");
	dimexp3 = ajRegCompCaseC("\\bnchar\\s*=\\s*(\\d+)");
	formexp1 = ajRegCompCaseC("\\bdatatype\\s*=\\s*(\\S+)");
	formexp2 = ajRegCompCaseC("\\brespectcase\\b");
	formexp3 = ajRegCompCaseC("\\bmissing\\s*=\\s*(\\S+)");
	formexp4 = ajRegCompCaseC("\\bgap\\s*=\\s*(\\S+)");
	formexp5 = ajRegCompCaseC("\\bsymbols\\s*=\\s*['\"]([^'\"]+)[\"']");
	formexp6 = ajRegCompCaseC("\\bequate\\s*=\\s*['\"]([^'\"]+)['\"]");
	formexp7 = ajRegCompCaseC("\\bmatchchar\\s*=\\s*(\\S+)");
	formexp8 = ajRegCompCaseC("\\b(no)?labels");
	formexp9 = ajRegCompCaseC("\\btranspose\\b");
	formexp10 = ajRegCompCaseC("\\binterleave\\b");
	formexp11 = ajRegCompCaseC("\\bitems\\s*=\\s*[(]([^)]+)[)]\\b");
	formexp12 = ajRegCompCaseC("\\bstatesformat\\s*=\\s*[{]([^}]+)[}]\\b");
	formexp13 = ajRegCompCaseC("\\b(no)?tokens");
	taxlabexp = ajRegCompCaseC("\\S+");
	chstlabexp = ajRegCompCaseC("\\d+\\s+[^,]+");
	chlabexp = ajRegCompCaseC("\\S+");
	optexp1 = ajRegCompCaseC("\\bgapmode\\s*=\\s*(\\S+)");
    }
 
    if (!thys->Characters)
	thys->Characters = nexusCharactersNew();

    if(newtaxa)
	thys->Characters->NewTaxa = ajTrue;

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "dimensions"))
	{
	    if(ajRegExec(dimexp1, cmdstr))
		thys->Characters->NewTaxa = ajTrue;

	    if(nexusGetInt(cmdstr, dimexp2, 1, &thys->Characters->Ntax))
	    {
		if (thys->Ntax)
		    ajDebug("nexus characters dimension ntax - "
			    "taxlabels already done\n");
		else
		{
		    if (!thys->Taxa)
			thys->Taxa = nexusTaxaNew();
		    thys->Ntax = thys->Taxa->Ntax = thys->Characters->Ntax;
		}
	    }

	    if(!nexusGetInt(cmdstr, dimexp3, 1, &thys->Characters->Nchar))
	    {
		ajDebug("nexusParseCharacters failed: nchar undefined\n");
		return ajFalse;
	    }
	}
	else if (ajStrMatchCaseC(command, "format"))
	{
	    if(nexusGetStr(cmdstr, formexp1, 1, &thys->Characters->DataType))
		nexusVocab("characters format datatype",
			   thys->Characters->DataType,
			   nexusCharactersDataTypes);

	    if(ajRegExec(formexp2, cmdstr))
		thys->Characters->RespectCase = ajTrue;

	    nexusGetChar(cmdstr, formexp3, 1, &thys->Characters->Missing);
	    nexusGetChar(cmdstr, formexp4, 1, &thys->Characters->Gap);
	    nexusGetStr(cmdstr, formexp5, 1, &thys->Characters->Symbols);
	    nexusGetStr(cmdstr, formexp6, 1, &thys->Characters->Equate);
	    nexusGetChar(cmdstr, formexp7, 1, &thys->Characters->MatchChar);
	    nexusGetBool(cmdstr, formexp8, 1, &thys->Characters->Labels);

	    if(ajRegExec(formexp9, cmdstr))
		thys->Characters->Transpose = ajTrue;

	    if(ajRegExec(formexp10, cmdstr))
		thys->Characters->Interleave = ajTrue;

	    nexusGetStr(cmdstr, formexp11, 0, &thys->Characters->Items);

	    if(nexusGetStr(cmdstr, formexp12, 0,
			   &thys->Characters->StatesFormat))
		nexusVocab("characters format statesformat",
			   thys->Characters->StatesFormat,
			   nexusCharactersStatesFormat);
	    nexusGetBool(cmdstr, formexp13, 0, &thys->Characters->Tokens);
	}
	else if (ajStrMatchCaseC(command, "eliminate"))
	{
	    ajStrRemoveWhiteExcess(&cmdstr);
	    ajStrAssignS(&thys->Characters->Eliminate, cmdstr);
	}
	else if (ajStrMatchCaseC(command, "taxlabels"))
	{
	    if (!thys->Taxa)
		thys->Taxa = nexusTaxaNew();

	    i = nexusGetArray(cmdstr, taxlabexp, &thys->Taxa->TaxLabels);

	    if (i != thys->Taxa->Ntax)
	    {
		ajDebug("nexusParseCharacters "
			"failed: Ntax %d read %d TaxLabels\n",
			thys->Taxa->Ntax, i);

		return ajFalse;
	    }
	}
	else if (ajStrMatchCaseC(command, "charstatelabels"))
	{
	    i = nexusGetArray(cmdstr, chstlabexp,
			    &thys->Characters->CharStateLabels);
	}
	else if (ajStrMatchCaseC(command, "charlabels"))
	{
	    i = nexusGetArray(cmdstr, chlabexp,
			    &thys->Characters->CharLabels);
	}
	else if (ajStrMatchCaseC(command, "statelabels"))
	{
	    i = nexusGetArray(cmdstr, chstlabexp,
			    &thys->Characters->StateLabels);
	}
	else if (ajStrMatchCaseC(command, "matrix"))
	{
	    i = nexusGetArray(cmdstr, newlinexp, &thys->Characters->Matrix);
	}
	else if (ajStrMatchCaseC(command, "options")) /* not 1997 standard */
	{
	    if (!thys->Assumptions)
		thys->Assumptions = nexusAssumptionsNew();

	    if(nexusGetStr(cmdstr, optexp1, 1, /* should be in assumptions */
			&thys->Assumptions->GapMode))
		nexusVocab("characters options gapmode",
			   thys->Assumptions->GapMode,
			   nexusAssumptionsGapMode);
	}
	else
	{
	    ajDebug("nexusParseCharacters unknown command '%S' ignored\n",
		    command);
	}
    }

    if (!thys->Characters->Nchar)
    {
	ajDebug("nexusParseCharacters failed: dimension nchar undefined\n");

	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic nexusParseUnaligned ********************************************
**
** Parses Nexus commands for a Unaligned block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseUnaligned(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;

    static AjPRegexp newlinexp = NULL;
    static AjPRegexp dimexp1 = NULL;
    static AjPRegexp dimexp2 = NULL;
    static AjPRegexp formexp1 = NULL;
    static AjPRegexp formexp2 = NULL;
    static AjPRegexp formexp3 = NULL;
    static AjPRegexp formexp5 = NULL;
    static AjPRegexp formexp6 = NULL;
    static AjPRegexp formexp8 = NULL;

    if (!dimexp1)
    {
	dimexp1 = ajRegCompCaseC("\\bnewtaxa\\b");
	dimexp2 = ajRegCompCaseC("\\bntax\\s*=\\s*(\\d+)");
	formexp1 = ajRegCompCaseC("\\bdatatype\\s*=\\s*(\\S+)");
	formexp2 = ajRegCompCaseC("\\brespectcase\\b");
	formexp3 = ajRegCompCaseC("\\bmissing\\s*=\\s*(\\S+)");
	formexp5 = ajRegCompCaseC("\\bsymbols\\s*=\\s*['\"]([^'\"]+)['\"]");
	formexp6 = ajRegCompCaseC("\\bequate\\s*=\\s*['\"]([^'\"]+)['\"]");
	formexp8 = ajRegCompCaseC("\\b(no)?labels");
    }

    if (!thys->Unaligned)
	thys->Unaligned = nexusUnalignedNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "dimensions"))
	{
	    if(ajRegExec(dimexp1, cmdstr))
		thys->Unaligned->NewTaxa = ajTrue;

	    if(nexusGetInt(cmdstr, dimexp2, 1, &thys->Unaligned->Ntax))
	    {
		if (thys->Ntax)
		    ajDebug("nexus unaligned dimension ntax - "
			    "taxlabels already done\n");
		else
		{
		    if (!thys->Taxa)
			thys->Taxa = nexusTaxaNew();
		    thys->Ntax = thys->Taxa->Ntax = thys->Unaligned->Ntax;
		}
	    }
	}
	else if (ajStrMatchCaseC(command, "format"))
	{
	    if(nexusGetStr(cmdstr, formexp1, 1, &thys->Unaligned->DataType))
		nexusVocab("unaligned format datatype",
			   thys->Unaligned->DataType,
			   nexusUnalignedDataTypes);

	    if(ajRegExec(formexp2, cmdstr))
		thys->Unaligned->RespectCase = ajTrue;

	    nexusGetChar(cmdstr, formexp3, 1, &thys->Unaligned->Missing);
	    nexusGetStr(cmdstr, formexp5, 1, &thys->Unaligned->Symbols);
	    nexusGetStr(cmdstr, formexp6, 1, &thys->Unaligned->Equate);
	    nexusGetBool(cmdstr, formexp8, 1, &thys->Unaligned->Labels);
	}
	else if (ajStrMatchCaseC(command, "matrix"))
	{
	    nexusGetArray(cmdstr, newlinexp, &thys->Unaligned->Matrix);
	}
	else
	{
	    ajDebug("nexusParseUnaligned unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusParseDistances ********************************************
**
** Parses Nexus commands for a Distances block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseDistances(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;
    ajuint i;

    static AjPRegexp newlinexp = NULL;
    static AjPRegexp dimexp1 = NULL;
    static AjPRegexp dimexp2 = NULL;
    static AjPRegexp dimexp3 = NULL;
    static AjPRegexp formexp1 = NULL;
    static AjPRegexp formexp2 = NULL;
    static AjPRegexp formexp3 = NULL;
    static AjPRegexp formexp4 = NULL;
    static AjPRegexp formexp5 = NULL;
    static AjPRegexp taxlabexp = NULL;

    if (!dimexp1)
    {
	dimexp1 = ajRegCompCaseC("\\bnewtaxa\\b");
	dimexp2 = ajRegCompCaseC("\\bntax\\s*=\\s*(\\d+)");
	dimexp3 = ajRegCompCaseC("\\bnchar\\s*=\\s*(\\d+)");
	formexp1 = ajRegCompCaseC("\\btriangle\\s*=\\s*(\\S+)");
	formexp2 = ajRegCompCaseC("\\b(no)?diagonal\\b");
	formexp3 = ajRegCompCaseC("\\b(no)?labels\\b");
	formexp4 = ajRegCompCaseC("\\bmissing\\s*=\\s*(\\S+)");
	formexp5 = ajRegCompCaseC("\\binterleave\\b");
	taxlabexp = ajRegCompCaseC("\\S+");
    }

    if (!thys->Distances)
	thys->Distances = nexusDistancesNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "dimensions"))
	{
	    if(ajRegExec(dimexp1,cmdstr))
	       thys->Distances->NewTaxa = ajTrue;

	    if(nexusGetInt(cmdstr, dimexp2, 1, &thys->Distances->Ntax))
	    {
		if (thys->Ntax)
		    ajDebug("nexus distances dimension ntax - "
			    "taxlabels already done\n");
		else
		{
		    if (!thys->Taxa)
			thys->Taxa = nexusTaxaNew();

		    thys->Ntax = thys->Taxa->Ntax = thys->Distances->Ntax;
		}
	    }

	    /* nchar is optional for distances only */
	    nexusGetInt(cmdstr, dimexp3, 1, &thys->Distances->Nchar);
	}
	else if (ajStrMatchCaseC(command, "format"))
	{
	    if(nexusGetStr(cmdstr, formexp1, 1, &thys->Distances->Triangle))
		nexusVocab("distances format triangle",
			   thys->Distances->Triangle,
			   nexusDistancesTriangle);

	    nexusGetBool(cmdstr, formexp2, 1, &thys->Distances->Diagonal);
	    nexusGetBool(cmdstr, formexp3, 1, &thys->Distances->Labels);
	    nexusGetChar(cmdstr, formexp4, 1, &thys->Distances->Missing);

	    if(ajRegExec(formexp5,cmdstr))
	       thys->Distances->Interleave = ajTrue;
	}
	else if (ajStrMatchCaseC(command, "taxlabels"))
	{
	    if (!thys->Taxa)
		thys->Taxa = nexusTaxaNew();

	    i = nexusGetArray(cmdstr, taxlabexp, &thys->Taxa->TaxLabels);

	    if (i != thys->Taxa->Ntax)
	    {
		ajDebug("nexusParseCharacters failed: "
			"Ntax %d read %d TaxLabels\n",
			thys->Taxa->Ntax, i);

		return ajFalse;
	    }
	}
	else if (ajStrMatchCaseC(command, "matrix"))
	    i = nexusGetArray(cmdstr, newlinexp, &thys->Characters->Matrix);
	else
	{
	    ajDebug("nexusParseDistances unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusParseSets ********************************************
**
** Parses Nexus commands for a Sets block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseSets(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;

    static AjPRegexp newlinexp = NULL;

    if (!newlinexp)
	newlinexp = ajRegCompC("[^\n]+");

    if (!thys->Sets)
	thys->Sets = nexusSetsNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	ajDebug("nexusParseSets %S: '%S'\n", command, cmdstr);

	if (ajStrMatchCaseC(command, "charset"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Sets->CharSet);
	else if (ajStrMatchCaseC(command, "stateset"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Sets->StateSet);
	else if (ajStrMatchCaseC(command, "changeset"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Sets->ChangeSet);
	else if (ajStrMatchCaseC(command, "taxset"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Sets->TaxSet);
	else if (ajStrMatchCaseC(command, "treeset"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Sets->TreeSet);
	else if (ajStrMatchCaseC(command, "charpartition"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Sets->CharPartition);
	else if (ajStrMatchCaseC(command, "taxpartition"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Sets->TaxPartition);
	else if (ajStrMatchCaseC(command, "treepartition"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Sets->TreePartition);
	else
	{
	    ajDebug("nexusParseSets unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusParseAssumptions ******************************************
**
** Parses Nexus commands for a Assumptions block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseAssumptions(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;

    static AjPRegexp newlinexp = NULL;
    static AjPRegexp optexp1 = NULL;
    static AjPRegexp optexp2 = NULL;
    static AjPRegexp optexp3 = NULL;

    if (!newlinexp)
    {
	newlinexp = ajRegCompC("[^\n]+");
	optexp1 = ajRegCompCaseC("\\bdeftype\\s*=\\s*(\\S+)");
	optexp2 = ajRegCompCaseC("\\bpolycount\\s*=\\s*(\\S+)");
	optexp3 = ajRegCompCaseC("\\bgapmode\\s*=\\s*(\\S+)");
    }

    if (!thys->Assumptions)
	thys->Assumptions = nexusAssumptionsNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "options"))
	{
	    if(nexusGetStr(cmdstr, optexp1, 1, &thys->Assumptions->DefType))
		nexusVocab("assumptions options deftype",
			   thys->Assumptions->DefType,
			   nexusAssumptionsDefType);
	    if(nexusGetStr(cmdstr, optexp2, 1, &thys->Assumptions->PolyTCount))
		nexusVocab("assumptions options polytcount",
			   thys->Assumptions->PolyTCount,
			   nexusAssumptionsPolyTCount);
	    if(nexusGetStr(cmdstr, optexp3, 1, &thys->Assumptions->GapMode))
		nexusVocab("assumptions options gapmode",
			   thys->Assumptions->GapMode,
			   nexusAssumptionsGapMode);
	}
	else if (ajStrMatchCaseC(command, "usertype"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Assumptions->UserType);
	else if (ajStrMatchCaseC(command, "typeset"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Assumptions->TypeSet);
	else if (ajStrMatchCaseC(command, "wtset"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Assumptions->WtSet);
	else if (ajStrMatchCaseC(command, "exset"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Assumptions->ExSet);
	else if (ajStrMatchCaseC(command, "ancstates"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Assumptions->AncStates);
	else if (ajStrMatchCaseC(command, "taxset")) /* not 1997 standard */
	{
	    if (!thys->Sets)
		thys->Sets = nexusSetsNew();
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Sets->TaxSet);
	}
	else if (ajStrMatchCaseC(command, "charset")) /* not 1997 standard */
	{
	    if (!thys->Sets)
		thys->Sets = nexusSetsNew();
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Sets->CharSet);
	}
	else
	{
	    ajDebug("nexusParseAssumptions unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusParseCodons ********************************************
**
** Parses Nexus commands for a Codons block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseCodons(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;

    static AjPRegexp newlinexp = NULL;

    if (!newlinexp)
	newlinexp = ajRegCompC("[^\n]+");

    if (!thys->Codons)
	thys->Codons = nexusCodonsNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "codonposset"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Codons->CodonPosSet);
	else if (ajStrMatchCaseC(command, "geneticcode"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Codons->GeneticCode);
	else if (ajStrMatchCaseC(command, "codeset"))
	    nexusGetArray(cmdstr, newlinexp,
			  &thys->Codons->CodeSet);
	else
	{
	    ajDebug("nexusParseCodons unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusParseTrees ********************************************
**
** Parses Nexus commands for a Trees block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseTrees(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;

    static AjPRegexp newlinexp = NULL;

    if (!newlinexp)
	newlinexp = ajRegCompC("[^\n]+");

    if (!thys->Trees)
	thys->Trees = nexusTreesNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "translate"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Trees->Translate);
	else if (ajStrMatchCaseC(command, "tree"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Trees->Tree);
	else
	{
	    ajDebug("nexusParseTrees unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusParseNotes ********************************************
**
** Parses Nexus commands for a Notes block
**
** @param [w] thys [AjPNexus] Nexus object
** @param [u] list [AjPList] List of block records
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusParseNotes(AjPNexus thys, AjPList list)
{
    AjPStr command = NULL;
    AjPStr cmdstr = NULL;

    static AjPRegexp newlinexp = NULL;

    if (!newlinexp)
	newlinexp = ajRegCompC("[^\n]+");

    if (!thys->Notes)
	thys->Notes = nexusNotesNew();

    while (nexusCommand(list, &command, &cmdstr))
    {
	if (ajStrMatchCaseC(command, "text"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Notes->Text);
	else if (ajStrMatchCaseC(command, "picture"))
	    nexusGetArray(cmdstr, newlinexp, &thys->Notes->Picture);
	else
	{
	    ajDebug("nexusParseNotes unknown command '%S' ignored\n",
		    command);
	}
    }

    return ajTrue;
}




/* @funcstatic nexusGetArray **************************************************
**
** Gets an array of strings from a parsed nexus command
** Appends to an existing array to handle multiple commands
**
** @param [w] src [AjPStr] Command string
** @param [u] exp [AjPRegexp] Compiled regular expression for parsing
** @param [w] dest [AjPStr**] Array generated
** @return [ajint] Number of strings returned
******************************************************************************/

static ajint nexusGetArray(AjPStr src, AjPRegexp exp,
			    AjPStr** dest)
{
    AjPList strlist;
    AjPStr tmpstr = NULL;
    AjPStr tmpsrc = NULL;
    ajint i;

    strlist = ajListstrNew();
    ajStrAssignS(&tmpsrc, src);

    while (ajRegExec(exp, tmpsrc))
    {
	ajRegSubI(exp, 0, &tmpstr);
	ajListstrPushAppend(strlist, tmpstr);
	tmpstr = NULL;
	ajRegPost(exp, &tmpstr);
	ajStrAssignS(&tmpsrc, tmpstr);
    }

    i = ajListstrToarrayAppend(strlist, dest);
    ajListstrFree(&strlist);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpsrc);

    return i;
}




/* @funcstatic nexusGetStr ****************************************************
**
** Gets a string from a parsed nexus command
**
** @param [w] src [AjPStr] Command string
** @param [u] exp [AjPRegexp] Compiled regular expression for parsing
** @param [r] isub [ajint] Substring number to extract
** @param [w] dest [AjPStr*] String generated
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusGetStr(AjPStr src, AjPRegexp exp, ajint isub, AjPStr* dest)
{
    if (ajRegExec(exp, src))
    {
	ajRegSubI(exp, isub, dest);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic nexusGetChar ***************************************************
**
** Gets a single character from a parsed nexus command
**
** @param [w] src [AjPStr] Command string
** @param [u] exp [AjPRegexp] Compiled regular expression for parsing
** @param [r] isub [ajint] Substring number to extract
** @param [w] dest [char*] String generated
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusGetChar(AjPStr src, AjPRegexp exp, ajint isub, char* dest)
{
    static AjPStr tmpstr = NULL;
    if (ajRegExec(exp, src))
    {
	ajRegSubI(exp, isub, &tmpstr);

	if (ajStrGetLen(tmpstr))
	{
	    *dest = ajStrGetCharFirst(tmpstr);

	    return ajTrue;
	}

	*dest = '\0';

	return ajFalse;
    }

    return ajFalse;
}




/* @funcstatic nexusGetInt ****************************************************
**
** Gets an integer from a parsed nexus command
**
** @param [w] src [AjPStr] Command string
** @param [u] exp [AjPRegexp] Compiled regular expression for parsing
** @param [r] isub [ajint] Substring number to extract
** @param [w] dest [ajuint*] Integer generated
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusGetInt(AjPStr src, AjPRegexp exp, ajint isub, ajuint* dest)
{
    static AjPStr tmpstr = NULL;

    if (ajRegExec(exp, src))
    {
	ajRegSubI(exp, isub, &tmpstr);

	return (ajStrToUint(tmpstr, dest));
    }

    return ajFalse;
}




/* @funcstatic nexusGetBool **************************************************
**
** Sets a nexus bool from a parsed nexus command in the form [no]name.
** We know name is found, we test for the "no" part.
**
** @param [w] src [AjPStr] Command string
** @param [u] exp [AjPRegexp] Compiled regular expression for parsing
** @param [r] isub [ajint] Substring number to extract
** @param [w] dest [AjBool*] Boolean generated
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusGetBool(AjPStr src, AjPRegexp exp, ajint isub, AjBool* dest)
{
    static AjPStr tmpstr = NULL;

    if (ajRegExec(exp, src))
    {
	if (!ajRegSubI(exp, isub, &tmpstr))
	    *dest = ajTrue;
	else
	    *dest = ajFalse;

	return ajTrue;
	    
    }

    return ajFalse;
}




/* @funcstatic nexusVocab ***************************************************
**
** Tests a string against a controlled vocabulary
**
** @param [r] title [const char*] Vocabulary title
** @param [r] src [const AjPStr] String to be tested
** @param [r] vocab [const char* []] List of known values, ending in a NULL
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusVocab(const char* title, const AjPStr src,
			 const char* vocab[])
{
    ajint i;

    for (i=0; vocab[i]; i++)
	if (ajStrMatchCaseC(src, vocab[i]))
	    return ajTrue;

    ajDebug("nexus vocab failed: '%S' unknown for %s\n",
	    src, title);

    return ajFalse;
}




/* @func ajNexusTrace *********************************************************
**
** Reports the contents of a Nexus object to the debug file
**
** @param [r] thys [const AjPNexus] nexus object
** @return [void]
******************************************************************************/

void ajNexusTrace(const AjPNexus thys)
{
    ajint i;

    ajDebug("Nexus trace\n");
    ajDebug("===========\n");

    ajDebug ("  Ntax: %d\n", thys->Ntax);

    if (!thys->Taxa)
	ajDebug("Taxa: <null>\n");
    else
    {
	ajDebug("Taxa\n");
	ajDebug("----\n");
	ajDebug("  Ntax: %d\n", thys->Taxa->Ntax);

	if (thys->Taxa->TaxLabels)
	{
	    ajDebug("  TaxLabels:\n");

	    for (i=0; thys->Taxa->TaxLabels[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Taxa->TaxLabels[i]);
	}
	else
		ajDebug("  TaxLabels: <null>\n");
    }

    if (!thys->Characters)
	ajDebug("Characters: <null>\n");
    else
    {
	ajDebug("Characters\n");
	ajDebug("----------\n");
	ajDebug("  NewTaxa: %B\n", thys->Characters->NewTaxa);
	ajDebug("  Ntax: %d\n", thys->Characters->Ntax);
	ajDebug("  Nchar: %d\n", thys->Characters->Nchar);

	if (thys->Characters->DataType)
	    ajDebug("  DataType: '%S'\n", thys->Characters->DataType);
	else
	    ajDebug("  DataType: <null>\n");

	ajDebug("  RespectCase: %B\n", thys->Characters->RespectCase);

	if (thys->Characters->Missing)
	    ajDebug("  Missing: '%c'\n", thys->Characters->Missing);
	else
	    ajDebug("  Missing: <null>\n");

	if (thys->Characters->Gap)
	    ajDebug("  Gap: '%c'\n", thys->Characters->Gap);
	else
	    ajDebug("  Gap: <null>\n");

	if (thys->Characters->Symbols)
	    ajDebug("  Symbols: '%S'\n", thys->Characters->Symbols);
	else
	    ajDebug("  Symbols: <null>\n");

	if (thys->Characters->Equate)
	    ajDebug("  Equate: '%S'\n", thys->Characters->Equate);
	else
	    ajDebug("  Equate: <null>\n");

	if (thys->Characters->MatchChar)
	    ajDebug("  MatchChar: '%c'\n", thys->Characters->MatchChar);
	else
	    ajDebug("  MatchChar: <null>\n");

	ajDebug("  Labels: %B\n", thys->Characters->Labels);
	ajDebug("  Transpose: %B\n", thys->Characters->Transpose);
	ajDebug("  Interleave: %B\n", thys->Characters->Interleave);

	if (thys->Characters->Items)
	    ajDebug("  Items: '%S'\n", thys->Characters->Items);
	else
	    ajDebug("  Items: <null>\n");

	if (thys->Characters->StatesFormat)
	    ajDebug("  StatesFormat: '%S'\n", thys->Characters->StatesFormat);
	else
	    ajDebug("  StatesFormat: <null>\n");

	ajDebug("  Tokens: %B\n", thys->Characters->Tokens);

	if (thys->Characters->Eliminate)
	    ajDebug("  Eliminate: '%S'\n", thys->Characters->Eliminate);
	else
	    ajDebug("  Eliminate: <null>\n");

	if (thys->Characters->CharStateLabels)
	{
	    ajDebug("  CharStateLabels:\n");

	    for (i=0; thys->Characters->CharStateLabels[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Characters->CharStateLabels[i]);
	}
	else
	    ajDebug("  CharStateLabels: <null>\n");

	if (thys->Characters->CharLabels)
	{
	    ajDebug("  CharLabels:\n");
	    for (i=0; thys->Characters->CharLabels[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Characters->CharLabels[i]);
	}
	else
	    ajDebug("  CharLabels: <null>\n");

	if (thys->Characters->StateLabels)
	{
	    ajDebug("  StateLabels:\n");

	    for (i=0; thys->Characters->StateLabels[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Characters->StateLabels[i]);
	}
	else
	    ajDebug("  StateLabels: <null>\n");

	if (thys->Characters->Matrix)
	{
	    ajDebug("  Matrix:\n");

	    for (i=0; thys->Characters->Matrix[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Characters->Matrix[i]);
	}
	else
	    ajDebug("  Matrix: <null>\n");
    }

    if (!thys->Unaligned)
	ajDebug("Unaligned: <null>\n");
    else
    {
	ajDebug("Unaligned\n");
	ajDebug("---------\n");
	ajDebug("  NewTaxa: %B\n", thys->Unaligned->NewTaxa);
	ajDebug("  Ntax: %d\n", thys->Unaligned->Ntax);

	if (thys->Unaligned->DataType)
	    ajDebug("  DataType: '%S'\n", thys->Unaligned->DataType);
	else
	    ajDebug("  DataType: <null>\n");

	ajDebug("  RespectCase: %B\n", thys->Unaligned->RespectCase);

	if (thys->Unaligned->Missing)
	    ajDebug("  Missing: '%c'\n", thys->Unaligned->Missing);
	else
	    ajDebug("  Missing: <null>\n");

	if (thys->Unaligned->Symbols)
	    ajDebug("  Symbols: '%S'\n", thys->Unaligned->Symbols);
	else
	    ajDebug("  Symbols: <null>\n");

	if (thys->Unaligned->Equate)
	    ajDebug("  Equate: '%S'\n", thys->Unaligned->Equate);
	else
	    ajDebug("  Equate: <null>\n");

	ajDebug("  Labels: %B\n", thys->Unaligned->Labels);

	if (thys->Unaligned->Matrix)
	{
	    ajDebug("  Matrix:\n");

	    for (i=0; thys->Unaligned->Matrix[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Unaligned->Matrix[i]);
	}
	else
	    ajDebug("  Matrix: <null>\n");
    }

    if (!thys->Distances)
	ajDebug("Distances: <null>\n");
    else
    {
	ajDebug("Distances\n");
	ajDebug("---------\n");
	ajDebug("NewTaxa: %B\n", thys->Distances->NewTaxa);
	ajDebug("  Ntax: %d\n", thys->Distances->Ntax);
	ajDebug("  Nchar: %d\n", thys->Distances->Nchar);

	if (thys->Distances->Triangle)
	    ajDebug("  Triangle: '%S'\n", thys->Distances->Triangle);
	else
	    ajDebug("  Triangle: <null>\n");

	ajDebug("  Diagonal: %B\n", thys->Distances->Diagonal);
	ajDebug("  Labels: %B\n", thys->Distances->Labels);

	if (thys->Distances->Missing)
	    ajDebug("  Missing: '%c'\n", thys->Distances->Missing);
	else
	    ajDebug("  Missing: <null>\n");

	ajDebug("  Interleave: %B\n", thys->Distances->Interleave);

	if (thys->Distances->Matrix)
	{
	    ajDebug("  Matrix:\n");

	    for (i=0; thys->Distances->Matrix[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Distances->Matrix[i]);
	}
	else
	    ajDebug("  Matrix: <null>\n");
    }

    if (!thys->Sets)
	ajDebug("Sets: <null>\n");
    else
    {
	ajDebug("Sets\n");
	ajDebug("----\n");

	if (thys->Sets->CharSet)
	{
	    ajDebug("  CharSet:\n");

	    for (i=0; thys->Sets->CharSet[i]; i++)
		ajDebug("   [%d] '%S'\n",
			i, thys->Sets->CharSet[i]);
	}
	else
	    ajDebug("  CharSet: <null>\n");

	if (thys->Sets->StateSet)
	{
	    ajDebug("  StateSet:\n");

	    for (i=0; thys->Sets->StateSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->StateSet[i]);
	}
	else
	    ajDebug("  StateSet: <null>\n");

	if (thys->Sets->ChangeSet)
	{
	    ajDebug("  ChangeSet:\n");

	    for (i=0; thys->Sets->ChangeSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->ChangeSet[i]);
	}
	else
	    ajDebug("  ChangeSet: <null>\n");

	if (thys->Sets->TaxSet)
	{
	    ajDebug("  TaxSet:\n");

	    for (i=0; thys->Sets->TaxSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->TaxSet[i]);
	}
	else
	    ajDebug("  TaxSet: <null>\n");

	if (thys->Sets->TreeSet)
	{
	    ajDebug("  TreeSet:\n");

	    for (i=0; thys->Sets->TreeSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->TreeSet[i]);
	}
	else
	    ajDebug("  TreeSet: <null>\n");

	if (thys->Sets->CharPartition)
	{
	    ajDebug("  CharPartition:\n");

	    for (i=0; thys->Sets->CharPartition[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->CharPartition[i]);
	}
	else
	    ajDebug("  CharPartition: <null>\n");

	if (thys->Sets->TaxPartition)
	{
	    ajDebug("  TaxPartition:\n");

	    for (i=0; thys->Sets->TaxPartition[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->TaxPartition[i]);
	}
	else
	    ajDebug("  TaxPartition: <null>\n");

	if (thys->Sets->TreePartition)
	{
	    ajDebug("  TreePartition:\n");

	    for (i=0; thys->Sets->TreePartition[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Sets->TreePartition[i]);
	}
	else
	    ajDebug("  TreePartition: <null>\n");
    }

    if (!thys->Assumptions)
	ajDebug("Assumptions: <null>\n");
    else
    {
	ajDebug("Assumptions\n");
	ajDebug("-----------\n");

	if (thys->Assumptions->DefType)
	    ajDebug("  DefType: '%S'\n", thys->Assumptions->DefType);
	else
	    ajDebug("  DefType: <null>\n");

	if (thys->Assumptions->PolyTCount)
	    ajDebug("  PolyTCount: '%S'\n", thys->Assumptions->PolyTCount);
	else
	    ajDebug("  PolyTCount: <null>\n");

	if (thys->Assumptions->GapMode)
	    ajDebug("  GapMode: '%S'\n", thys->Assumptions->GapMode);
	else
	    ajDebug("  GapMode: <null>\n");

	if (thys->Assumptions->UserType)
	{
	    ajDebug("  UserType:\n");

	    for (i=0; thys->Assumptions->UserType[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Assumptions->UserType[i]);
	}
	else
	    ajDebug("  UserType: <null>\n");

	if (thys->Assumptions->TypeSet)
	{
	    ajDebug("  TypeSet:\n");

	    for (i=0; thys->Assumptions->TypeSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Assumptions->TypeSet[i]);
	}
	else
	    ajDebug("  TypeSet: <null>\n");

	if (thys->Assumptions->WtSet)
	{
	    ajDebug("  WtSet:\n");

	    for (i=0; thys->Assumptions->WtSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Assumptions->WtSet[i]);
	}
	else
	    ajDebug("  WtSet: <null>\n");

	if (thys->Assumptions->ExSet)
	{
	    ajDebug("  ExSet:\n");
	    for (i=0; thys->Assumptions->ExSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Assumptions->ExSet[i]);
	}
	else
	    ajDebug("  ExSet: <null>\n");

	if (thys->Assumptions->AncStates)
	{
	    ajDebug("  AncStates:\n");

	    for (i=0; thys->Assumptions->AncStates[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Assumptions->AncStates[i]);
	}
	else
	    ajDebug("  AncStates: <null>\n");
    }

    if (!thys->Codons)
	ajDebug("Codons: <null>\n");
    else
    {
	ajDebug("Codons\n");
	ajDebug("------\n");

	if (thys->Codons->CodonPosSet)
	{
	    ajDebug("  CodonPosSet:\n");

	    for (i=0; thys->Codons->CodonPosSet[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Codons->CodonPosSet[i]);
	}
	else
	    ajDebug("  CodonPosSet: <null>\n");

	if (thys->Codons->GeneticCode)
	{
	    ajDebug("  GeneticCode:\n");

	    for (i=0; thys->Codons->GeneticCode[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Codons->GeneticCode[i]);
	}
	else
	    ajDebug("  GeneticCode: <null>\n");

	if (thys->Codons->CodeSet)
	{
	    ajDebug("  CodeSet:\n");

	    for (i=0; thys->Codons->CodeSet[i]; i++)
		ajDebug("  CodeSet  [%d] '%S'\n", i, thys->Codons->CodeSet[i]);
	}
	else
	    ajDebug("  CodeSet: <null>\n");
    }

    if (!thys->Trees)
	ajDebug("Trees: <null>\n");
    else
    {
	ajDebug("Trees\n");
	ajDebug("-----\n");

	if (thys->Trees->Translate)
	{
	    ajDebug("  Translate:\n");

	    for (i=0; thys->Trees->Translate[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Trees->Translate[i]);
	}
	else
	    ajDebug("  Translate: <null>\n");

	if (thys->Trees->Tree)
	{
	    ajDebug("  Tree:\n");

	    for (i=0; thys->Trees->Tree[i]; i++)
		ajDebug("    [%d] '%S'\n",
			i, thys->Trees->Tree[i]);
	}
	else
	    ajDebug("  Tree: <null>\n");
    }

    if (!thys->Notes)
	ajDebug("Notes: <null>\n");
    else
    {
	ajDebug("Notes\n");
	ajDebug("-----\n");

	if (thys->Notes->Text)
	{
	    ajDebug("  Text:\n");

	    for (i=0; thys->Notes->Text[i]; i++)
		ajDebug("    [%d] '%S'\n", i, thys->Notes->Text[i]);
	}
	else
	    ajDebug("  Text: <null>\n");

	if (thys->Notes->Picture)
	{
	    ajDebug("  Picture:\n");

	    for (i=0; thys->Notes->Picture[i]; i++)
		ajDebug("    [%d] '%S'\n", i, thys->Notes->Picture[i]);
	}
	else
	    ajDebug("  Picture: <null>\n");
    }

    return;
}




/* @func ajNexusGetTaxa *******************************************************
**
** Returns the taxa as a string array
**
** @param [r] thys [const AjPNexus] Nexus object
** @return [AjPStr*] taxa string array, NULL terminated, read only
******************************************************************************/

AjPStr* ajNexusGetTaxa(const AjPNexus thys)
{
    if(!thys)
	return NULL;

    if (thys->Taxa)
	return thys->Taxa->TaxLabels;

    return NULL;
}




/* @func ajNexusGetNtaxa ******************************************************
**
** Returns the number of taxa
**
** @param [r] thys [const AjPNexus] Nexus object
** @return [ajuint] Number of taxa
** @@
******************************************************************************/

ajuint ajNexusGetNtaxa(const AjPNexus thys)
{
    if(!thys)
	return 0;

    if (thys->Taxa)
	return thys->Taxa->Ntax;

    return 0;
}




/* @func ajNexusGetSequences **************************************************
**
** Returns the sequences from the character matrix as a string array.
**
** Sequences are in the same order as the taxa returned by ajNexusGetTaxa.
** Sequences may need to be set if not already done.
**
** @param [u] thys [AjPNexus] Nexus object
** @return [AjPStr*] taxa string array, NULL terminated, read only
******************************************************************************/

AjPStr* ajNexusGetSequences(AjPNexus thys)
{
    if(!thys)
	return NULL;

    nexusSetSequences(thys);

    if (thys->Characters)
	return thys->Characters->Sequences;

    return NULL;
}




/* @funcstatic nexusSetSequences **********************************************
**
** Sets the sequences from the character matrix as a string array
**
** @param [u] thys [AjPNexus] Nexus object
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool nexusSetSequences(AjPNexus thys)
{
    AjPTable seqtab;
    AjPStr firstseq = NULL;
    ajint i;
    AjPStr taxlabel = NULL;
    AjPStr tmpstr = NULL;
    AjPStr rdline = NULL;
    AjPStr seqstr = NULL;
    AjBool havetaxa = ajFalse;
    ajuint itax=0;
    AjPRegexp word;
    char gapch;

    ajDebug("nexusSetSequences\n");

    if(!thys)
	return ajFalse;


    if (!thys->Characters)		/* no characters defined */
    {
	ajDebug("Failed - No characters defined\n");

	return ajFalse;
    }

    if (thys->Characters->Sequences)	/* already done */
    {
	ajDebug("Success - Sequences already done\n");

	return ajTrue;
    }

    if (!thys->Characters->Nchar)	/* must have been defined */
    {
	ajDebug("Failed - must have been defined\n");

	return ajFalse;
    }

    if (!thys->Ntax)			/* taxa required */
    {
	ajDebug("Failed - number of taxa required\n");

	return ajFalse;
    }

    if (!thys->Taxa)			/* taxa required */
    {
	ajDebug("Failed - taxa names required\n");

	return ajFalse;
    }

    if (!thys->Characters->Matrix)	/* matrix required */
    {
	ajDebug("Failed - matrix required\n");

	return ajFalse;
    }

    if (ajStrMatchCaseC(thys->Characters->DataType, /* must be sequence data */
			"continuous") ||
	ajStrMatchCaseC(thys->Characters->DataType, /* must be sequence data */
			"standard"))
    {
	ajDebug("Failed - not sequence data\n");

	return ajFalse;
    }

    if(thys->Characters->Gap)
	gapch = thys->Characters->Gap;
    else
	gapch = '.';

    word = ajRegCompC("\\S+");

    seqtab = ajTablestrNewLen(thys->Taxa->Ntax);

    if (thys->Taxa->TaxLabels)
    {
	havetaxa = ajTrue;

	for (i=0; thys->Taxa->TaxLabels[i]; i++)
	{
	    seqstr = ajStrNewRes(thys->Characters->Nchar+1);
	    ajTablePut(seqtab, thys->Taxa->TaxLabels[i], seqstr);
	    seqstr = NULL;
	}
    }
    else
    {
	AJCNEW0(thys->Taxa->TaxLabels, (thys->Ntax+1));
    }

    taxlabel = NULL;
    itax=0;

    for (i=0; thys->Characters->Matrix[i]; i++)
    {
	ajStrAssignS(&rdline, thys->Characters->Matrix[i]);
	ajStrRemoveWhiteExcess(&rdline);

	if (!taxlabel || thys->Characters->Interleave ||
	    (ajStrGetLen(seqstr) >= thys->Characters->Nchar))
	{				/* next tax label */
	    if (!ajRegExec(word, rdline))
		continue;

	    ajRegSubI(word, 0, &taxlabel);
	    ajRegPost(word, &tmpstr);
	    ajStrAssignS(&rdline, tmpstr);
	    ajStrQuoteStripAll(&taxlabel);

	    if (!havetaxa)
	    {
		ajStrAssignS(&thys->Taxa->TaxLabels[itax], taxlabel);
		seqstr = ajStrNewRes(thys->Characters->Nchar+1);
		ajTablePut(seqtab, thys->Taxa->TaxLabels[itax], seqstr);
		seqstr = NULL;
		itax++;
		if (itax >= thys->Ntax)
		    havetaxa = ajTrue;
	    }

	    seqstr = ajTableFetch(seqtab, taxlabel);

	    if (!seqstr)
	    {
		ajErr("Unknown taxon '%S' in nexus data", taxlabel);
		nexusArrayDel(&thys->Characters->Sequences);
		ajTableFree(&seqtab);

		return ajFalse;
	    }
	}

	if (!i)
	    ajStrAssignS(&firstseq, taxlabel);

	while (ajRegExec(word, rdline))
	{
	    ajRegSubI(word, 0, &tmpstr);
	    ajStrAppendS(&seqstr, tmpstr);
	    ajTablePut(seqtab, taxlabel, seqstr); /* just to make sure */
	    ajRegPost(word, &tmpstr);
	    ajStrAssignS(&rdline, tmpstr);
	}

	ajStrExchangeKK(&seqstr, gapch, '-');
    }

    ajRegFree(&word);
    ajStrDel(&tmpstr);
    ajStrDel(&rdline);

    AJCNEW0(thys->Characters->Sequences, (thys->Ntax+1));

    for (i=0; thys->Taxa->TaxLabels[i]; i++)
    {
	thys->Characters->Sequences[i] = ajTableFetch(seqtab,
						    thys->Taxa->TaxLabels[i]);
	if (ajStrGetLen(thys->Characters->Sequences[i]) !=
	    thys->Characters->Nchar)
	{
	    ajErr("Nexus sequence length for '%S' is %d, expected %d\n",
		    thys->Taxa->TaxLabels[i],
		    ajStrGetLen(thys->Characters->Sequences[i]),
		    thys->Characters->Nchar);
	    nexusArrayDel(&thys->Characters->Sequences);
	    ajTableFree(&seqtab);

            return ajFalse;
	}
    }

    ajTableFree(&seqtab);

    return ajTrue;
}




/* @funcstatic nexusArrayDel **************************************************
**
** Default destructor for AJAX string arrays.
** Called for each element in the array, which must end with a NULL.
**
** If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [d] pthis [AjPStr**] Pointer to the string array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

static void nexusArrayDel(AjPStr** pthis)
{
    AjPStr* thys;
    ajint i;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    for (i=0; thys[i];i++)
	ajStrDel(&thys[i]);

    AJFREE(*pthis);

    return;
}
