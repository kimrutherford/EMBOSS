/* @source ajassemread ********************************************************
**
** AJAX assembly reading functions
**
** These functions control all aspects of AJAX assembly reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.56 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/10 09:27:41 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/


#include "ajlib.h"

#include "ajassemread.h"
#include "ajassem.h"
#include "ajcall.h"
#include "ajfileio.h"
#include "ajutil.h"
#include "ajtextread.h"
#include "ajquery.h"
#include "ajnam.h"
#include "ajseqbam.h"
#include "ajbamindex.h"

#include <errno.h>


AjPTable assemDbMethods = NULL;

static AjPStr asseminReadLine = NULL;


static AjBool asseminAppendAlignmentRecs(AjPSeqBam b, void* data);
static AjPAssemRead asseminBamAlignmentRec2Emboss(AjPSeqBam b);

static AjBool asseminLoadBam(AjPAssemin assemin, AjPAssem assem);
static AjPSeqBamHeader asseminReadBamHeader(AjPAssemin assemin,
					    AjPAssem assem);
static AjPAssemRead asseminReadBamAlignments(AjPSeqBamBgzf gzfile,
					     AjPSeqBamHeader header,
					     AjPSeqBam b);


static AjBool asseminLoadMaf(AjPAssemin thys, AjPAssem assem);

static AjBool asseminLoadSam(AjPAssemin thys, AjPAssem assem);
static AjPAssemRead asseminReadSamAlignments(AjPAssemin assemin,
					     AjPAssem assem);
static AjBool asseminReadSamHeader(AjPAssemin assemin, AjPAssem assem);


/* @datastatic AssemPInFormat *************************************************
**
** Assembly input formats data structure
**
** @alias AssemSInFormat
** @alias AssemOInFormat
**
** @attr Name [const char*] Format name
** @attr Obo  [const char*] Ontology term id from EDAM
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Binary [AjBool] Binary file format
** @attr Padding [AjBool] Padding to alignment boundary
** @attr Load [AjBool function] Input load function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct AssemSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool Binary;
    AjBool Padding;
    AjBool (*Load) (AjPAssemin thys, AjPAssem assem);
} AssemOInFormat;

#define AssemPInFormat AssemOInFormat*


static AssemOInFormat asseminFormatDef[] =
{
/* "Name",        */
/*   "EDAMid" "Description" */
/*     Alias,   Try,     Binary,  Pad,    ReadFunction */
/*     */
  {"unknown",
       "0000", "Unknown format", /* default to first format */
       AJFALSE, AJFALSE, AJFALSE, AJTRUE, &asseminLoadSam},
  {"sam",
       "2573", "SAM Assembly format",
       AJFALSE, AJTRUE, AJFALSE, AJTRUE,  &asseminLoadSam},
  {"maf",
       "2573", "MIRA Assembly format",
   /* http://mira-assembler.sourceforge.net/docs/chap_maf_part.html
    * name may change soon */
       AJFALSE, AJTRUE, AJFALSE, AJTRUE,  &asseminLoadMaf},
  {"bam",
       "2573", "BAM Assembly format",
       AJFALSE, AJTRUE, AJTRUE,  AJTRUE, &asseminLoadBam},
/*  {"ace",
       "0002573", "ACE Assembly format",
//   http://bozeman.mbt.washington.edu/consed/distributions/README.20.0.txt
// http://www.cbcb.umd.edu/research/contig_representation.shtml#ACE
AJFALSE, AJTRUE, AJFALSE, AJTRUE, &asseminLoadAce},*/

  {NULL, NULL, NULL, 0, 0, 0, 0, NULL}
};



static ajuint asseminLoadFmt(AjPAssemin assemin, AjPAssem assem,
			     ajuint format);
static AjBool asseminLoad(AjPAssemin assemin, AjPAssem assem);
static AjBool asseminformatFind(const AjPStr format, ajint* iformat);
static AjBool asseminFormatSet(AjPAssemin assemin, AjPAssem assem);
static AjBool asseminListProcess(AjPAssemin assemin, AjPAssem assem,
                                 const AjPStr listfile);
static void asseminListNoComment(AjPStr* text);
static void asseminQryRestore(AjPAssemin assemin, const AjPQueryList node);
static void asseminQrySave(AjPQueryList node, const AjPAssemin assemin);
static AjBool assemDefine(AjPAssem thys, AjPAssemin assemin);
static AjBool asseminQryProcess(AjPAssemin assemin, AjPAssem assem);
static AjBool assemQueryMatch(const AjPQuery thys, const AjPAssem assem);




/* @filesection ajassemread **************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPAssemin] Assembly input objects ***************************
**
** Function is for manipulating assembly input objects
**
** @nam2rule Assemin
******************************************************************************/




/* @section Assembly Input Constructors ***************************************
**
** All constructors return a new assembly input object by pointer. It
** is the responsibility of the user to first destroy any previous
** assembly input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPAssemin]
**
** @nam3rule  New     Construct a new assembly input object
**
** @valrule   *  [AjPAssemin] New assembly input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajAsseminNew *********************************************************
**
** Creates a new assembly input object.
**
** @return [AjPAssemin] New assembly input object.
** @category new [AjPAssemin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPAssemin ajAsseminNew(void)
{
    AjPAssemin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_ASSEMBLY);

    return pthis;
}





/* @section Assembly Input Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the assembly input object.
**
** @fdata [AjPAssemin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPAssemin*] Assembly input
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajAsseminDel *********************************************************
**
** Deletes an assembly input object.
**
** @param [d] pthis [AjPAssemin*] Assembly input
** @return [void]
** @category delete [AjPAssemin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminDel(AjPAssemin* pthis)
{
    AjPAssemin thys;
    AjPSeqBamBgzf gzfile = NULL;

    if(!pthis)
        return;

    if(!*pthis)
        return;

    thys = *pthis;

    ajDebug("ajAsseminDel called qry:'%S'\n", thys->Input->Qry);

    if(thys->BamInput)
    {
        gzfile = thys->Input->TextData;
        ajSeqBamBgzfClose(gzfile);
    }

    ajTextinDel(&thys->Input);
    AJFREE(*pthis);

    return;
}




/* @section assembly input modifiers ******************************************
**
** These functions use the contents of an assembly input object and
** update them.
**
** @fdata [AjPAssemin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPAssemin] Assembly input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajAsseminClear *******************************************************
**
** Clears an assembly input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPAssemin] Assembly input
** @return [void]
** @category modify [AjPAssemin] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminClear(AjPAssemin thys)
{

    ajDebug("ajAsseminClear called\n");

    if(!thys)
        return;

    ajTextinClear(thys->Input);

    thys->Loading = ajFalse;

    return;
}




/* @func ajAsseminQryC ********************************************************
**
** Resets an assembly input object using a new Universal
** Query Address
**
** @param [u] thys [AjPAssemin] Assembly input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminQryC(AjPAssemin thys, const char* txt)
{
    ajAsseminClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajAsseminQryS ********************************************************
**
** Resets an assembly input object using a new Universal
** Query Address
**
** @param [u] thys [AjPAssemin] Assembly input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminQryS(AjPAssemin thys, const AjPStr str)
{
    ajAsseminClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPAssemin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPAssemin] Assembly input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAsseminTrace *******************************************************
**
** Debug calls to trace the data in an assembly input object.
**
** @param [r] thys [const AjPAssemin] Assembly input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminTrace(const AjPAssemin thys)
{
    ajDebug("assembly input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    return;
}




/* @section Assembly data inputs **********************************************
**
** These functions load the assembly data provided by the first argument
**
** @fdata [AjPAssemin]
**
** @nam3rule Load Load more assembly data
**
** @argrule Load assemin [AjPAssemin] Assembly input object
** @argrule Load assem [AjPAssem] Assembly data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajAsseminLoad ********************************************************
**
** If the file is not yet open, calls asseminQryProcess to convert the query
** into an open file stream.
**
** Uses asseminLoad for the actual file reading.
**
** Returns the results in the AjPAssem object.
**
** @param [u] assemin [AjPAssemin] assembly data input definitions
** @param [w] assem [AjPAssem] assembly data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPAssem] Master assembly data input,
**                  calls specific functions for file access type
**                  and assembly data format.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajAsseminLoad(AjPAssemin assemin, AjPAssem assem)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;
    AjPTextin input = assemin->Input;
    AjPFilebuff buff = input->Filebuff;

    ajDebug("ajAsseminLoad '%F' EOF: %B "
            "records: %u dataread: %B datadone: %B\n",
           ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
           input->Records, input->Dataread, input->Datadone);

    if(buff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajAsseminLoad: input file '%F' still there, try again\n",
		assemin->Input->Filebuff->File);
	ret = asseminLoad(assemin, assem);
	ajDebug("ajAsseminLoad: open buffer  qry: '%S' returns: %B\n",
		assemin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(assemin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(assemin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajAsseminQryS(assemin, node->Qry);
	    ajDebug("++SAVE ASSEMIN '%S' '%S' %d\n",
		    assemin->Input->Qry,
		    assemin->Input->Formatstr, assemin->Input->Format);

            asseminQryRestore(assemin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajAsseminLoad: open list, try '%S'\n",
                    assemin->Input->Qry);

	    if(!asseminQryProcess(assemin, assem) &&
               !ajListGetLength(assemin->Input->List))
		return ajFalse;

	    ret = asseminLoad(assemin, assem);
	    ajDebug("ajAsseminLoad: list qry: '%S' returns: %B\n",
		    assemin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajAsseminLoad: no file yet - test query '%S'\n",
                    assemin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!asseminQryProcess(assemin, assem) &&
               !ajListGetLength(assemin->Input->List))
            {
		return ajFalse;
            }

	    if(ajListGetLength(assemin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = asseminLoad(assemin, assem);
	    ajDebug("ajAsseminLoad: new qry: '%S' returns: %B\n",
		    assemin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(assemin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read assembly data '%S'",
                  assemin->Input->Qry);

	listdata = ajTrue;
	ajListPop(assemin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajAsseminQryS(assemin, node->Qry);
	ajDebug("++SAVE (AGAIN) ASSEMIN '%S' '%S' %d\n",
		assemin->Input->Qry,
		assemin->Input->Formatstr, assemin->Input->Format);

	asseminQryRestore(assemin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!asseminQryProcess(assemin, assem))
	    continue;

	ret = asseminLoad(assemin, assem);
	ajDebug("ajAsseminLoad: list retry qry: '%S' returns: %B\n",
		assemin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read assembly data '%S'",
                  assemin->Input->Qry);

	return ajFalse;
    }


    assemDefine(assem, assemin);

    return ajTrue;
}




/* @funcstatic assemQueryMatch ************************************************
**
** Compares an assembly data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] assem [const AjPAssem] Assembly data.
** @return [AjBool] ajTrue if the assembly data matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool assemQueryMatch(const AjPQuery thys, const AjPAssem assem)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjIList iter       = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;
    const AjPAssemContig c = NULL;

    ajDebug("assemQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    assem->Id, ajListGetLength(thys->QueryFields),
            thys->CaseId, thys->QryDone);

    if(!thys)			   /* no query to test, that's fine */
	return ajTrue;

    if(thys->QryDone)			/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    iterfield = ajListIterNewread(thys->QueryFields);
    while(!ajListIterDone(iterfield))
    {
        field = ajListIterGet(iterfield);

        ajDebug("  field: '%S' Query: '%S'\n",
                field->Field, field->Wildquery);
        if(ajStrMatchC(field->Field, "id"))
        {
            ajDebug("  id test: %Lu\n",
                    ajListGetLength(assem->ContigsOrder));

            iter = ajListIterNewread(assem->ContigsOrder);
            while(!ajListIterDone(iter))
            {
                c = ajListIterGet(iter);

                ajDebug("test case %B '%S'\n", thys->CaseId, c->Name);
                if(thys->CaseId)
                {
                    if(ajStrMatchWildS(c->Name, field->Wildquery))
                    {
                        ajListIterDel(&iterfield);
                        ajListIterDel(&iter);
                        return ajTrue;
                    }
                }
                else
                {
                    if(ajStrMatchWildCaseS(c->Name, field->Wildquery))
                    {
                        ajListIterDel(&iterfield);
                        ajListIterDel(&iter);
                        return ajTrue;
                    }
                }
            }
            ajListIterDel(&iter);

            tested = ajTrue;
        }

    }
        
    ajListIterDel(&iterfield);

    if(!tested)		    /* nothing to test, so accept it anyway */
    {
        ajDebug("  no tests: assume OK\n");
	return ajTrue;
    }
    
    ajDebug("result: %B\n", ok);

    return ok;
}




/* @funcstatic assemDefine ****************************************************
**
** Make sure all assembly data object attributes are defined
** using values from the assembly input object if needed
**
** @param [w] thys [AjPAssem] Assembly data returned.
** @param [u] assemin [AjPAssemin] Assembly data input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool assemDefine(AjPAssem thys, AjPAssemin assemin)
{

    /* if values are missing in the assembly object, we can use defaults
       from assemin or calculate where possible */

    /* assign the dbname if defined in the assemin object */
    if(ajStrGetLen(assemin->Input->Db))
      ajStrAssignS(&thys->Db, assemin->Input->Db);

    return ajTrue;
}





/* @funcstatic asseminLoadFmt *************************************************
**
** Tests whether assembly data can be read using the specified format.
** Then tests whether the assembly data matches assembly data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] assemin [AjPAssemin] Assembly data input object
** @param [w] assem [AjPAssem] Assembly data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the assembly data type failed
**                  3 if it failed to read a assembly data
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Load function
** asseminLoadXxxxxx where Xxxxxxx is the supported assembly data format.
**
** Some of the assemReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint asseminLoadFmt(AjPAssemin assemin, AjPAssem assem,
                           ajuint format)
{
    AjPTextin input = assemin->Input;
    AjPFilebuff buff = input->Filebuff;

    ajDebug("++asseminLoadFmt format %d (%s) '%S'\n",
	    format, asseminFormatDef[format].Name,
	    input->Qry);

    if(!input->Dataread)
        input->Records = 0;

    /* Calling funclist asseminFormatDef() */
    if((*asseminFormatDef[format].Load)(assemin, assem))
    {
        input->Dataread = ajTrue;
	ajDebug("asseminLoadFmt success with format %d (%s)\n",
		format, asseminFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                assem->Id);
	input->Format = format;
	ajStrAssignC(&input->Formatstr, asseminFormatDef[format].Name);
	ajStrAssignC(&assem->Formatstr, asseminFormatDef[format].Name);
	ajStrAssignEmptyS(&assem->Db, input->Db);
	ajStrAssignS(&assem->Filename, input->Filename);

	if(assemQueryMatch(input->Query, assem))
	{
            /* ajAsseminTrace(assemin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajAssemClear(assem);

        return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(buff),
		ajFilebuffIsEof(buff));

	if (!ajFilebuffIsBuffered(buff) &&
	    ajFilebuffIsEof(buff))
	    return FMT_EOF;

	ajFilebuffReset(buff);
	ajDebug("Format %d (%s) failed, file buffer reset by asseminLoadFmt\n",
		format, asseminFormatDef[format].Name);
	/* ajFilebuffTraceFull(assemin->Filebuff, 10, 10);*/
    }

    ajDebug("++asseminLoadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic asseminLoad ****************************************************
**
** Given data in a assemin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] assemin [AjPAssemin] Assembly data input object
** @param [w] assem [AjPAssem] Assembly data object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool asseminLoad(AjPAssemin assemin, AjPAssem assem)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPTextin input = assemin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjBool ok;

    AjPTextAccess  textaccess  = input->Query->TextAccess;
    AjPAssemAccess assemaccess = input->Query->Access;

    if(!input->Records)
    {
        ajAssemClear(assem);
    ajDebug("asseminLoad: cleared\n");
    }

    if(input->Single && input->Count && !input->Datacount)
    {
	/*
	** One assembly data item at a time is read.
	** The first assembly data item was read by ACD
	** for the following ones we need to reset the AjPAssemin
	**
	** Single is set by the access method
	*/

	ajDebug("asseminLoad: single access - count %d - call access"
		" routine again\n",
		input->Count);
	/* Calling funclist asseminAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(input))
            {
                ajDebug("asseminLoad: (*textaccess->Access)(assemin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(assemaccess)
        {
            if(!(*assemaccess->Access)(assemin))
            {
                ajDebug("asseminLoad: (*assemaccess->Access)(assemin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = input->Filebuff;
    }

    ajDebug("asseminLoad: assemin format %d '%S'\n", input->Format,
	    input->Formatstr);

    input->Count++;

    if(!input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(input->Filebuff, &asseminReadLine);

        if(!ajStrIsWhite(asseminReadLine))
        {
            ajFilebuffClear(input->Filebuff,1);
            break;
        }
    }

    if(!input->Format)
    {			   /* no format specified, try all defaults */

        /*regfile = ajFileIsFile(ajFilebuffGetFile(assemin->Input->Filebuff));*/

	for(i = 1; asseminFormatDef[i].Name; i++)
	{
	    if(!asseminFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("asseminLoad:try format %d (%s)\n",
		    i, asseminFormatDef[i].Name);

	    istat = asseminLoadFmt(assemin, assem, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++asseminLoad OK, set format %d\n",
                        input->Format);
		assemDefine(assem, assemin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("asseminload: (a1) "
                        "asseminLoadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("asseminLoad: (b1) "
                        "asseminLoadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("asseminLoad: (c1) "
                        "asseminLoadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("asseminLoad: (d1) "
                        "asseminLoadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("assembly data '%S' has zero length, ignored",
		       ajAssemGetQryS(assem));
		ajDebug("asseminLoad: (e1) "
                        "asseminLoadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from asseminLoadFmt\n", stat);
	    }

	    ajAssemClear(assem);

	    if(input->Format)
		break;			/* we read something */

            ajFilebuffTrace(input->Filebuff);
	}

	if(!input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("asseminLoad:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++asseminLoad set format %d\n",
                input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("asseminLoad: one format specified\n");
	ajFilebuffSetUnbuffered(input->Filebuff);

	ajDebug("++asseminLoad known format %d\n",
                input->Format);
	istat = asseminLoadFmt(assemin, assem, input->Format);

	switch(istat)
	{
	case FMT_OK:
	    assemDefine(assem, assemin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("asseminLoad: (a2) "
                    "asseminLoadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("asseminLoad: (b2) "
                    "asseminLoadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("asseminLoad: (c2) "
                    "asseminLoadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("asseminLoad: (d2) "
                    "asseminLoadFmt stat == EOF *try again*\n");
            if(assemin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(input->Filebuff),
                      asseminFormatDef[input->Format].Name,
                      input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("assmebly data '%S' has zero length, ignored",
		   ajAssemGetQryS(assem));
	    ajDebug("asseminLoad: (e2) "
                    "asseminLoadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from asseminLoadFmt\n", stat);
	}

	ajAssemClear(assem); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("asseminLoad failed - try again with format %d '%s' code %d\n",
	    input->Format,
            asseminFormatDef[input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    input->Search, input->ChunkEntries,
            input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(input))
            return ajFalse;
	else if(assemaccess && !(*assemaccess->Access)(assemin))
            return ajFalse;
        buff = input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(input->Search &&
          (input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = asseminLoadFmt(assemin, assem, input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    assemDefine(assem, assemin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("asseminLoad: (a3) "
                    "asseminLoadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("asseminLoad: (b3) "
                    "asseminLoadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("asseminLoad: (c3) "
                    "asseminLoadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("asseminLoad: (d3) "
                    "asseminLoadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajAssemGetQryS(assem));
	    ajDebug("asseminLoad: (e3) "
                    "asseminLoadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from asseminLoadFmt\n", stat);
	}

	ajAssemClear(assem); /* 1 : read, failed to match id/acc/query */
    }

    if(input->Format)
	ajDebug("asseminLoad: *failed* to read assembly data %S "
                "using format %s\n",
		input->Qry,
                asseminFormatDef[input->Format].Name);
    else
	ajDebug("asseminLoad: *failed* to read assembly data %S "
                "using any format\n",
		assemin->Input->Qry);

    return ajFalse;
}




/* @funcstatic asseminReadBamHeader *******************************************
**
** Read header information from BAM files
**
** @param [u] assemin [AjPAssemin] Assembly input object
** @param [w] assem [AjPAssem] Assembly object
** @return [AjPSeqBamHeader] BAM header object on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPSeqBamHeader asseminReadBamHeader(AjPAssemin assemin, AjPAssem assem)
{
    AjPFilebuff buff = NULL;
    AjPFile infile = NULL;
    AjPAssemContig contig = NULL;
    AjPAssemContig* contigs = NULL;
    AjPSeqBamBgzf gzfile = NULL;
    AjPSeqBamHeader header = NULL;
    AjPBamIndex idx = NULL;
    char* targetname = NULL;
    ajint tid;
    ajint filestat = 0;
    ajint tid_new = 0;
    AjPTextin input;
    AjIList iterfield  = NULL;
    AjPQuery qry = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    input = assemin->Input;
    buff = input->Filebuff;
    infile = ajFilebuffGetFile(buff);
    qry = input->Query;

    ajDebug("asseminReadBamHeader ...\n");

    if(!infile)
	return NULL;

    ajFileTrace(infile);
    ajFilebuffTrace(buff);

    /* reset to beginning of file -
     ** has at least been tested for blank lines */
    filestat = ajFileSeek(infile, 0L, SEEK_SET);
    if(filestat != 0)
    {
	ajDebug("asseminReadBamHeader rewind failed errno %d: %s\n",
	        errno, strerror(errno));
	return NULL;
    }

    assemin->BamInput = ajTrue;
    gzfile = ajSeqBamBgzfNew(ajFilebuffGetFileptr(buff), "r");
    assemin->Input->TextData = gzfile;

    header = ajSeqBamHeaderRead(gzfile);

    if (!header)
    {
	ajDebug("ajSeqBamHeaderReadHeader failed\n");
	ajSeqBamBgzfClose(gzfile);
	ajFileSeek(infile,filestat,0);
	ajFilebuffResetPos(buff);
	ajFileTrace(infile);
	ajFilebuffTrace(buff);
	return NULL;
    }

    input->Records++;

    if (ajListGetLength(qry->QueryFields))
	assemin->BamIdx = ajBamIndexLoad(ajFileGetNameC(infile));

    idx = assemin->BamIdx;

    /* read reference sequence names and lengths */

    contigs = AJCALLOC(header->n_targets, sizeof(AjPAssemContig));
    for(tid=0; tid < header->n_targets; tid++)
    {

	targetname = header->target_name[tid];
	ajDebug("bam target[%d] '%s'\n", tid, targetname);

	if(idx)
	{
	    ok = ajFalse;
	    iterfield = ajListIterNewread(qry->QueryFields);
	    while(!ajListIterDone(iterfield))
	    {
		field = ajListIterGet(iterfield);
		if(ajStrMatchC(field->Field, "id"))
		{
		    if(qry->CaseId)
		    {
			if(ajCharMatchWildS(targetname,
			                    field->Wildquery))
			    ok = ajTrue;
		    }
		    else
		    {
			if(ajCharMatchWildCaseS(targetname,
			                        field->Wildquery))
			    ok = ajTrue;
		    }
		}
	    }
	    ajListIterDel(&iterfield);
	    if(!ok)
		continue;

	    assem->Count = tid_new;
	    tid_new++;

	    ajBamFetch(gzfile, idx, tid,
		    assemin->cbegin,
		    assemin->cend,
		    assem,
		    asseminAppendAlignmentRecs);
	}

	AJNEW0(contig);
	contig->Name = ajStrNewC(targetname);
	ajDebug("reference sequence:%S\n", contig->Name);
	ajTablePut(assem->Contigs, contig->Name, contig);
	ajListPushAppend(assem->ContigsOrder, contig);
	contig->Length = header->target_len[tid];
	ajDebug("ref seq length:%d\n", contig->Length);
	contigs[tid] = contig;
    }

    assem->SO = ajAssemsortorderGetType(ajSeqBamHeaderGetSortorder(header));
    ajListToarray(assem->ContigsOrder, (void***)&assem->ContigArray);

    AJFREE(contigs);

    return header;
}




/* @funcstatic asseminLoadBam *************************************************
**
** Given data in an assembly structure, tries to read everything needed
** using binary alignment/map (BAM) format.
**
** @param [u] assemin [AjPAssemin] Assembly input object
** @param [w] assem [AjPAssem] Assembly object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool asseminLoadBam(AjPAssemin assemin, AjPAssem assem)
{
    AjPTextin input = assemin->Input;
    AjPFilebuff buff  = input->Filebuff;
    AjPFile infile = NULL;
    AjPSeqBamBgzf gzfile = NULL;
    AjPSeqBam b = NULL;
    AjPSeqBamHeader header = NULL;
    AjPTable contigtags = NULL;
    AjPTable readgroups = NULL;
    ajuint iread = 0;
    ajuint maxread = 1;

    ajDebug("asseminLoadBam '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            assem->Hasdata);

    if(input->Datadone)
    {
        return ajFalse;
    }

    infile = ajFilebuffGetFile(buff);

    if(!infile)
	return ajFalse;

    ajFileTrace(infile);
    ajFilebuffTrace(buff);

    if(!input->Records)
    {
        ajAssemClear(assem);

	assem->BamHeader = asseminReadBamHeader(assemin, assem);

	if(!assem->BamHeader)
	    return ajFalse;

	readgroups = ajSeqBamHeaderGetReadgroupTags(assem->BamHeader);
	ajAssemSetReadgroups(assem, readgroups);

	contigtags = ajSeqBamHeaderGetRefseqTags(assem->BamHeader);
	ajAssemSetContigattributes(assem, contigtags);

	assem->SO = ajAssemsortorderGetType(
		ajSeqBamHeaderGetSortorder(assem->BamHeader));

	ajTableSetDestroyvalue(readgroups, (void(*)(void**))&ajTableDel);
	ajTableDel(&readgroups);

	ajTableSetDestroyvalue(contigtags, (void(*)(void**))&ajTableDel);
	ajTableDel(&contigtags);

	return ajTrue;
    }


    /*
    ** On later calls, read a chunk of data
    */

    /* clear current chunk */

    ajDebug("continue processing: maxread %u Datacount %u\n",
            maxread, input->Datacount);

    ajAssemReset(assem);
    assem->Hasdata = ajTrue;

    iread = 0;

    header = assem->BamHeader;
    gzfile = input->TextData;

    if(!assemin->BamIdx)
    {
	AJNEW0(b); /* todo: should store b in an iter obj
	 so won't need to be allocated each time before
	 reading a bam record here */

	assem->rec = asseminReadBamAlignments(gzfile,
                                              header, b);
	AJFREE(b->data);
	AJFREE(b);

	if(assem->rec)
        {
            ajListPushAppend(assem->Reads, assem->rec);
            iread++;
            input->Records++;
            input->Datacount += iread;
	    return ajTrue;
        }
    }

    ajSeqBamBgzfClose(gzfile);
    ajSeqBamHeaderDel(&assem->BamHeader);

    ajBamIndexDel(&assemin->BamIdx);

    input->Datadone = ajTrue;
    ajFilebuffClear(buff, -1);
    buff->File->End = ajTrue;
    input->TextData = NULL;

    return ajTrue;
}




/* @funcstatic asseminReadBamAlignments ***************************************
**
** Read BAM alignment records, when a query was not made
**
** TODO: should we update this function to append only the alignments in
**       the query contigs?
**       samtools simply rejects any queries when there is no index found
**
** @param [u] gzfile [AjPSeqBamBgzf] Bam file handler
** @param [u] header [AjPSeqBamHeader] Bam header
** @param [u] b [AjPSeqBam] Bam data
** @return [AjPAssemRead] Read data object
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPAssemRead asseminReadBamAlignments(AjPSeqBamBgzf gzfile,
                                             AjPSeqBamHeader header,
                                             AjPSeqBam b)
{
    ajint  ret = 0;
    AjPAssemRead r  = NULL;
    const char* library = NULL;
    const char* querylib = NULL; /* TODO: not yet implemented" */

    /* next BAM record */

    ret = ajSeqBamRead(gzfile, b);
    if(ret < -1)
	ajErr("seqReadBamAlignments truncated file return %d\n", ret);

    if(ret == -1)
 	return NULL;

    if (querylib)
    {
	library = ajSeqBamGetLibrary(header, b);
	if(strcmp(library,querylib))
	{
	    return NULL;
	    /* fixme: should try reading the next record until a match*/
	}
    }

    r = asseminBamAlignmentRec2Emboss(b);

    return r;
}




/* @funcstatic asseminAppendAlignmentRecs *************************************
**
** Append BAM alignment records to the assembly object after reformatting
** the alignment data.
**
** @param [u] b [AjPSeqBam] BAM record
** @param [u] data [void*] Assembly object
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool asseminAppendAlignmentRecs(AjPSeqBam b, void*data)
{
    AjPAssemRead r = NULL;
    AjPAssem assem = NULL;

    assem = (AjPAssem)data;

    r = asseminBamAlignmentRec2Emboss(b);

    if(r->Rnext == r->Reference)
    {
	r->Reference = ajTableGetLength(assem->Contigs);
	r->Rnext = r->Reference;
    }
    else
    {
	r->Reference = ajTableGetLength(assem->Contigs);
	r->Rnext = -1; /* FIXME: proper mapping needed */
    }

    ajListPushAppend(assem->Reads, r);

    return ajTrue;
}




/* @funcstatic asseminBamAlignmentRec2Emboss **********************************
**
** Load alignment records into assembly object
**
** @param [u] b [AjPSeqBam] BAM record
** @return [AjPAssemRead] Assembly object
**
******************************************************************************/

static AjPAssemRead asseminBamAlignmentRec2Emboss(AjPSeqBam b)
{
    AjPStr cigarstr = NULL;
    AjPStr seqstr   = NULL;
    AjPStr qualstr  = NULL;
    AjPStr tagstr   = NULL;
    unsigned char dp;
    unsigned char* d;
    AjPAssemRead r  = NULL;
    AjPSeqBamCore c = NULL;
    AjPAssemTag tag = NULL;
    ajuint i=0;
    ajuint dpos=0;
    ajint  cigop=0;
    ajuint cigend=0;
    ajuint cigint=0;

    c = &b->core;
    ajDebug("rID: %d pos: %d bin: %hd mapQual: %d read_name_len: %d"
	    " flag_nc: %hd cigar_len: %hd read_len: %d"
	    " mate_rID: %d mate_pos: %d ins_size: %d\n",
	    c->tid, c->pos, c->bin, c->qual, c->l_qname,
	    c->flag, c->n_cigar, c->l_qseq,
	    c->mtid, c->mpos, c->isize);
    ajDebug("l_aux: %d data_len:%d m_data:%d\n",
	    b->l_aux, b->data_len, b->m_data);

    AJNEW0(r);

    r->Flag = c->flag;

    if(r->Flag & BAM_FREVERSE)
    {
	r->y1 = c->pos+1;
	r->Reversed = ajTrue;
    }
    else
	r->x1 = c->pos+1;

    r->Reference = c->tid;
    r->MapQ = c->qual;
    r->Tlen = c->isize;
    r->Rnext = c->mtid;
    r->Pnext = c->mpos+1;

    d = b->data;
    dpos = 0;
    ajStrAssignC(&r->Name, (const char*) &d[dpos]);
    ajDebug("read name: %p '%S'\n", dpos, r->Name);

    dpos += (c->l_qname); /* l_qname includes trailing null */
    ajStrAssignC(&cigarstr, "");

    for(i=0; i < c->n_cigar; i++)
    {
	memcpy(&cigint, &d[dpos], 4);
	cigop = cigint & BAM_CIGAR_MASK;
	cigend = cigint >> BAM_CIGAR_SHIFT;

	ajFmtPrintAppS(&cigarstr, "%u%c",
		cigend, cigarcode[cigop]);
	dpos += 4;
    }

    ajDebug("cigar: %p %S\n", dpos, cigarstr);

    ajStrAssignRef(&r->Cigar, cigarstr);
    ajStrDel(&cigarstr);

    ajStrAssignC(&seqstr, "");
    for(i=0; i < (ajuint) c->l_qseq; i++)
    {
	ajStrAppendK(&seqstr,
	             bam_nt16_rev_table[MAJSEQBAMSEQI(&d[dpos], i)]);
    }

    dpos += (c->l_qseq+1)/2;
    ajDebug("seq: %p '%S'\n", dpos, seqstr);

    ajStrAssignRef(&r->Seq, seqstr);
    ajStrDel(&seqstr);

    if(c->flag & BAM_FREVERSE)
	r->Reversed = ajTrue;

    if(d[dpos] == 0xFF)
    {
	ajDebug("qual: MISSING\n");
	dpos += c->l_qseq;
	ajStrAssignK(&r->SeqQ, '*');
    }
    else
    {
	qualstr = ajStrNewRes(c->l_qseq+1);

	for(i=0; i < (ajuint) c->l_qseq; i++)
	    ajStrAppendK(&qualstr, (char)(33 + d[dpos++]));

	ajStrAssignRef(&r->SeqQ, qualstr);
	ajDebug("qual: %p %S\n", dpos, qualstr);
	ajStrDel(&qualstr);
    }

    /*  tag values */

    while (dpos < (ajuint) b->data_len)
    {
	ajStrAssignC(&tagstr, "");
	AJNEW0(tag);

	ajStrAppendK(&tagstr, d[dpos++]);
	ajStrAppendK(&tagstr, d[dpos++]);
	ajStrAssignS(&tag->Name, tagstr);

	dp = d[dpos++];

	tag->type = dp;

	ajStrSetClear(&tagstr);

	if (dp == 'Z' || dp == 'H')
	{
	    ajFmtPrintAppS(&tagstr,"%s", &d[dpos]);
	    while(d[dpos])
		dpos++;
	    dpos++;
	}
	else if (dp == 'f')
	{
	    ajFmtPrintAppS(&tagstr,"%f", (float) *(&d[dpos]));
	    dpos += 4;
	}
	else if (dp == 'd')
	{
	    ajFmtPrintAppS(&tagstr,"%lf", (double) *(&d[dpos]));
	    dpos += 8;
	}
	else if (dp == 'A')
	{
	    ajStrAssignK(&tagstr, d[dpos++]);
	}
	else if (dp == 'c')
	{
	    ajFmtPrintAppS(&tagstr,"%d",
	                   (ajint) (signed char) d[dpos++]);
	}
	else if (dp == 's')
	{
	    ajFmtPrintAppS(&tagstr,"%hd",
	                   (ajshort) *(&d[dpos]));
	    dpos += 2;
	}
	else if (dp == 'i')
	{
	    ajFmtPrintAppS(&tagstr,"%d",
	                   (ajint) *(&d[dpos]));
	    dpos += 4;
	}
	else if (dp == 'C')
	{
	    ajFmtPrintAppS(&tagstr,"%u",
	                   (ajuint) d[dpos++]);
	}
	else if (dp == 'S')
	{
	    ajFmtPrintAppS(&tagstr,"%hu",
	                   (ajushort) *(&d[dpos]));
	    dpos += 2;
	}
	else if (dp == 'I')
	{
	    ajFmtPrintAppS(&tagstr,"%u",
	                   (ajuint) d[dpos]);
	    dpos += 4;
	}
	else {
	    ajWarn("Unknown BAM aux type hex(%x) uint(%u) '%c'\n",
	           (ajuint) dp, (ajuint) dp, dp);
	}

	ajStrAssignS(&tag->Comment, tagstr);

	if(!r->Tags)
	    r->Tags = ajListNew();
	ajListPushAppend(r->Tags, tag);

	ajDebug("tags: %p '%S'\n", dpos, tagstr);
	ajStrDel(&tagstr);
    }

    return r;
}




/* @funcstatic asseminLoadMaf *************************************************
**
** Reads assemblies in MAF format, used by the MIRA assembly software.
**
** @param [u] assemin [AjPAssemin] Assembly input object
** @param [w] assem [AjPAssem] Assembly object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool asseminLoadMaf(AjPAssemin assemin, AjPAssem assem)
{
    AjPTextin input = assemin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjPFile infile = NULL;
    AjPStrTok handle = NULL;
    AjPStr keyword   = NULL;
    AjPStr token     = NULL;
    AjPAssemContig contig = NULL;
    AjPAssemRead r   = NULL;
    AjPAssemTag tag  = NULL;
    AjBool ok = ajTrue;
    ajlong fpos     = 0;
    ajint contigid  = -1;

    AjBool endofaReadorContig = ajFalse;
    ajuint iread = 0;
    ajuint maxread = 1000;

    ajDebug("asseminLoadMaf '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            assem->Hasdata);


    /* ajFilebuffTrace(buff); */

    if(input->Datadone)
    {
        return ajFalse;
    }

    infile = ajFilebuffGetFile(buff);

    if(!infile)
	return ajFalse;

    /*
    ** On first call, we read the header only
    */

    if(!input->Records)
    {
        ajAssemClear(assem);
 
        ajListPeekLast(assem->ContigsOrder, (void**)&contig);
        contigid = (ajuint) ajListGetLength(assem->ContigsOrder)-1;

        while (ajBuffreadLinePos(buff, &asseminReadLine, &fpos))
        {
            input->Records++;

            ajStrTokenAssignC(&handle, asseminReadLine, " \n\r\t");
            ajStrTokenNextParse(&handle, &keyword);
            ajStrTokenNextParse(&handle, &token);

            switch(CASE2(ajStrGetCharFirst(asseminReadLine),
                         ajStrGetCharPos(asseminReadLine, 1)))
            {
                case CASE2('\\','\\'):
                    ajDebug("start of read data for the contig\n");
                    // means end-of-contig-definition
                    endofaReadorContig=ajTrue;
                    break;
                case CASE2('C','O'):
                    ajDebug("new contig: %S", token);
                    AJNEW0(contig);
                    ajStrAssignS(&contig->Name, token);
                    ajTablePut(assem->Contigs, contig->Name, contig);
                    ajListPushAppend(assem->ContigsOrder, contig);
                    contigid = (ajuint) ajListGetLength(assem->ContigsOrder)-1;
                    contig->Tags = ajListNew();
                    break;
                case CASE2('N', 'R'):
                    ajStrToInt(token, &contig->Nreads);
                    ajDebug("number of reads in contig %d\n", contig->Nreads);
                    break;
                case CASE2('L','C'):
                    ajStrToInt(token, &contig->Length);
                    break;
                case CASE2('C','S'):
                    ajStrAssignRef(&contig->Consensus, token);
                    break;
                case CASE2('C','Q'):
                    ajStrAssignS(&contig->ConsensusQ, token);
                    break;
                case CASE2('C','T'):
                    AJNEW0(tag);
                    ajStrAssignS(&tag->Name, token);
                    ajStrTokenNextParse(&handle, &token);
                    ajStrToUint(token, &tag->x1);
                    ajStrTokenNextParse(&handle, &token);
                    ajStrToUint(token, &tag->y1);
                    ajStrTokenRestParse(&handle, &token);
                    ajStrTrimEndC(&token, "\r\n");
                    ajStrAssignS(&tag->Comment, token);
                    ajListPushAppend(contig->Tags, tag);
                    tag = NULL;
                    break;
                default:
                    ajWarn("unknown MAF contig tag: %S - %S\n",
                           keyword, asseminReadLine);
            }
            ajStrDel(&keyword);
            ajStrDel(&token);
            ajStrDel(&asseminReadLine);
            ajStrTokenDel(&handle);

            if(endofaReadorContig)
            {
                return ajTrue;
            }
        }
    }

    ajAssemReset(assem);
    assem->Hasdata = ajTrue;

    ajListPeekLast(assem->ContigsOrder, (void**)&contig);
    contigid = (ajuint) ajListGetLength(assem->ContigsOrder)-1;

    ok = ajTrue;
    while (ok && (iread < maxread))
    {
        ok = ajBuffreadLinePos(buff, &asseminReadLine, &fpos);
        if(!ok)
            break;

        ajStrTokenAssignC(&handle, asseminReadLine, " \n\r\t");
        ajStrTokenNextParse(&handle, &keyword);
        ajStrTokenNextParse(&handle, &token);


        input->Records++;

        switch(CASE2(ajStrGetCharFirst(asseminReadLine),
                     ajStrGetCharPos(asseminReadLine, 1)))
        {
            case CASE2('R','D'):
        	AJNEW0(r);
		r->Flag = 0x0;
		ajStrAssignS(&r->Name, token);
		r->Reference = contigid;
                iread++;
		break;
            case CASE2('R','S'):
		ajStrAssignS(&r->Seq, token);
		break;
            case CASE2('R','Q'):
		ajStrAssignS(&r->SeqQ, token);
		break;
            case CASE2('T','N'):
		ajStrAssignS(&r->Template, token);
		break;
            case CASE2('D','I'):
		r->Direction = ajStrGetCharFirst(token);
		break;
            case CASE2 ('T', 'F'):
		ajStrToInt(token, &r->TemplateSizeMin);
		break;
            case CASE2('T','T'):
		ajStrToInt(token, &r->TemplateSizeMax);
		break;
            case CASE2('S', 'F'):
		ajStrAssignS(&r->File, token);
		break;
            case CASE2('C','L'):
		ajStrToInt(token, &r->ClipLeft);
		break;
            case CASE2('C','R'):
		ajStrToInt(token, &r->ClipRight);
		break;
            case CASE2('S','L'):
		ajStrToInt(token, &r->VectorLeft);
		break;
            case CASE2('S', 'R'):
		ajStrToInt(token, &r->VectorRight);
		break;
            case CASE2('Q','L'):
		 ajStrToInt(token, &r->QualLeft);
		break;
            case CASE2('Q','R'):
		ajStrToInt(token, &r->QualRight);
		break;
            case CASE2('S','T'):
		ajStrAssignS(&r->Technology, token);
		break;
            case CASE2('E','R'):
        	ajDebug("end of the read");
                break;
            case CASE2('R','T'):
                AJNEW0(tag);
                ajStrAssignS(&tag->Name, token);
                ajStrTokenNextParseC(&handle, " \t", &token);
                ajStrToUint(token, &tag->x1);
                ajStrTokenNextParseC(&handle, " \t", &token);
                ajStrToUint(token, &tag->y1);
                if(!r->Tags)
                    r->Tags = ajListNew();
                ajListPushAppend(r->Tags, tag);
                tag = NULL;
                break;
            case CASE2('L','R'):
            {
                /* TODO: read length
                 * MIRA writes LR lines for reads with more than 2000 bases
                 */
                break;
            }
            case CASE2('A','O'):
            {
                /* TODO */

                break;

            }
            case CASE2('/','/'):
                ajDebug("end of the read data for the contig\n");
                break;
            case CASE2('E','C'):
                ajDebug("end of contig\n");
                break;
            case CASE2('A','T'):
        	/* placement of the read */
        	ajStrToInt(token, &r->x1);
        	ajStrTokenNextParse(&handle, &token);
        	ajStrToInt(token, &r->y1);
        	ajStrTokenNextParse(&handle, &token);
        	ajStrToInt(token, &r->x2);
        	ajStrTokenNextParse(&handle, &token);
        	ajStrToInt(token, &r->y2);

        	if (r->x1 > r->y1)
        	    r->Flag |= BAM_FREVERSE;

                assem->rec = r;
                endofaReadorContig=ajTrue;
                break;
            default:
        	ajWarn("unknown MAF read tag: %S - %S\n",
                       keyword, asseminReadLine);
        }
        if(endofaReadorContig)
        {
            if(assem->rec)
            {
                ajListPushAppend(assem->Reads, assem->rec);
                assem->rec = NULL;
                r = NULL;
            }

            input->Datacount += iread;

            ajStrDel(&keyword);
            ajStrDel(&token);
            ajStrTokenDel(&handle);

            return ajTrue;
        }
     }

    ajStrDel(&keyword);
    ajStrDel(&token);
    ajStrTokenDel(&handle);

    if(assem->rec)
    {
        ajListPushAppend(assem->Reads, assem->rec);
        assem->rec = NULL;
        r = NULL;
    }

    input->Datacount += iread;

    ajDebug("data done ok: %B Datacount: %u iread: %u/%u\n",
            ok, input->Datacount, iread, maxread);

    if(!ok) 
    {
        input->Datadone = ajTrue;
        ajFilebuffClear(buff, 0);
    }

    return ajTrue;
}




/* @funcstatic asseminReadSamHeader *******************************************
**
** Reads assemblies in sequence alignment/map (SAM) format.
**
** @param [w] assemin [AjPAssemin] Assembly input object
** @param [u] assem [AjPAssem] Assembly object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool asseminReadSamHeader(AjPAssemin assemin, AjPAssem assem)
{
    AjPTextin input = assemin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjPStr tmp = NULL;

    AjPTable hlinemap = NULL;
    AjPAssemContig contig = NULL;
    AjPTable readgroups = NULL;

    AjBool badformat = ajFalse;
    const char *cp = NULL;

    ajlong fpos     = 0;

    ajDebug("asseminReadSamHeader\n");

    readgroups = ajTablecharNew(16);
    ajTableSetDestroyvalue(readgroups, (void(*)(void**))ajTableDel);

    /* === header section === */

    while(ajBuffreadLinePos(buff, &asseminReadLine, &fpos)
	    && ajStrGetCharFirst(asseminReadLine) == '@')
    {

        input->Records++;
	ajStrTrimWhiteEnd(&asseminReadLine);

	/* 
	 * TODO:following function prints warning messages,
	 * should we have a silence argument
	 * if we are going to try different formats?
	 */ 
	hlinemap = ajSeqBamHeaderLineParse(ajStrGetPtr(asseminReadLine));
	if(!hlinemap)
	{
	    badformat = ajTrue;
	    continue; /* should this return rather than continue */
	}

	switch(((const char*)ajTableFetchC(hlinemap, "type"))[0])
	{
	    case 'H':
	    /* @HD
	     * VN: Format version. Accepted format: /^[0-9]+\.[0-9]+$/.
	     * SO: Sorting order of alignments.
	     * Valid values: unknown (default), unsorted, queryname and
	     * coordinate. For coordinate sort, the major sort key is the RNAME
	     * field, with order defined by the order of @SQ lines
	     * in the header. The minor sort key is the POS field.
	     * Alignments with ‘*’ in RNAME field follow alignments with
	     * some other value but otherwise are in arbitrary order.
	    **/

		cp = ajTableFetchC(hlinemap, "SO");
		if(cp)
		{
		    assem->SO = ajAssemsortorderGetType(cp);

		    ajDebug("sort order: %s %d %s\n", cp, assem->SO,
			    ajAssemGetSortorderC(assem));

		}
		break;
	    case 'S':  /* @SQ */
		cp = ajTableFetchC(hlinemap, "SN");
		if(cp)
		{
		    AJNEW0(contig);
		    ajStrAssignC(&contig->Name, cp);
		    ajDebug("reference sequence:%S\n", contig->Name);
		    ajTablePut(assem->Contigs, contig->Name, contig);
		    ajListPushAppend(assem->ContigsOrder, contig);

		    cp = ajTableFetchC(hlinemap, "LN");
		    if(cp)
		    {
			tmp = ajStrNewC(cp);
			ajStrToInt(tmp, &contig->Length);
			ajDebug("ref seq length:%d\n", contig->Length);
			ajStrDel(&tmp);
		    }
		}
		break;
	    case 'R': /* @RG */
		cp = ajTableFetchC(hlinemap, "ID");
		if(cp)
		{
		    ajTablePut(readgroups, ajCharNewC(cp), hlinemap);
		    continue; /* in order not to delete the table hlinemap */
		}
		break;
	    case 'P': /* @PG */
		break;
	    case 'C': /* @CO */
		break;
	    default:
		badformat = ajTrue;
		break;
	}

	if(badformat)
	{
	    ajErr("bad SAM header line: '%S'", asseminReadLine);
	    return ajFalse;
	}

	ajTableDel(&hlinemap);
    }

    if(!input->Records)
    {
        return ajFalse;
    }
    
    ajAssemSetReadgroups(assem, readgroups);

    ajListToarray(assem->ContigsOrder, (void***)&assem->ContigArray);
    ajTableDel(&readgroups);

    return ajTrue;
}




/* @funcstatic asseminLoadSam *************************************************
**
** Reads assemblies in sequence alignment/map (SAM) format.
**
** @param [w] assemin [AjPAssemin] Assembly input object
** @param [u] assem [AjPAssem] Assembly object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool asseminLoadSam(AjPAssemin assemin, AjPAssem assem)
{
    AjPTextin input = assemin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjPFile infile = NULL;
    ajlong fpos = 0;
    ajuint iread = 0;
    ajuint maxread = 1;
    AjBool ok = ajTrue;

    ajDebug("asseminLoadSam '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            assem->Hasdata);

    if(input->Datadone)
    {
        return ajFalse;
    }

    infile = ajFilebuffGetFile(buff);

    if(!infile)
	return ajFalse;

    if(!input->Records)
    {
        ajAssemClear(assem);

	return asseminReadSamHeader(assemin, assem);
    }
    
    ajAssemReset(assem);
    assem->Hasdata = ajTrue;

    while(ok && (iread < maxread))
    {

	if(input->Count != 2 ) /* first SAM record may have been read
				  while reading the header */
	{
	    ok = ajBuffreadLinePos(buff, &asseminReadLine, &fpos);
	    if(!ok)
		break;
	}

	assem->rec = asseminReadSamAlignments(assemin, assem);

	if(assem->rec)
	{
            input->Records++;
            ajListPushAppend(assem->Reads, assem->rec);
            iread++;
	}
    }

    input->Datacount += iread;

    if(!ok) 
    {
        input->Datadone = ajTrue;
        ajFilebuffClear(buff, 0);
    }
    else if(!iread) 
    {
        input->Datadone = ajTrue;
        ajFilebuffClear(buff, 0);
        return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic asseminReadSamAlignments ***************************************
**
** Reads read/alignment records from SAM files
**
** @param [w] assemin [AjPAssemin] Assembly input object
** @param [u] assem [AjPAssem] Assembly object
** @return [AjPAssemRead] read/alignment record on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPAssemRead asseminReadSamAlignments(AjPAssemin assemin, AjPAssem assem)
{
    AjPFilebuff buff = NULL;
    AjPFile infile = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjPAssemContig contig = NULL;
    AjPAssemRead r = NULL;
    AjPAssemContig* contigs = NULL;
    AjPAssemTag tag = NULL;
    const char *cp = NULL;
    ajint iqual;
    ajuint seqlen = 0;
    ajint qmin = 33;
    ajint qmax = 126;
    ajuint pos;
    ajint i=0;


    buff = assemin->Input->Filebuff;
    infile = ajFilebuffGetFile(buff);

    contigs = assem->ContigArray;

    if(ajStrParseCountC(asseminReadLine, "\t") < 11)
	return NULL;


    AJNEW0(r);

    ajStrTokenAssignC(&handle, asseminReadLine, "\t\n");

    ajStrTokenNextParseNoskip(&handle,&token); /* QNAME */
    ajStrAssignS(&r->Name, token);
    ajDebug("QNAME '%S' '%S'\n", token, r->Name);

    ajStrTokenNextParseNoskip(&handle,&token); /* FLAG */
    ajDebug("FLAG  '%S'\n", token);
    ajStrToInt(token, &r->Flag);

    ajStrTokenNextParseNoskip(&handle,&token); /* RNAME */
    contig = ajTableFetchmodS(assem->Contigs, token);

    if(contig)
    {
	r->Reference = -1;

	if(contigs)
	    for(i=0;contigs[i];i++)
	    {
		if(contigs[i] == contig)
		{
		    r->Reference = i;
		    break;
		}
	    }

	contig->Nreads++;
    }
    else
    {
	if( !ajStrMatchC(token, "*") &&
		!ajTableFetchmodS(assem->ContigsIgnored, token) )
	{
	    ajWarn("reference '%S' is recognised as '*', "
		    "no header (@SQ) entries was found for it", token);
	    ajTablePut(assem->ContigsIgnored, token, (void*)assemin);
	}

	contig = ajTableFetchmodC(assem->Contigs, "*");

	if(!contig)
	{
	    AJNEW0(contig);
	    ajStrAssignC(&contig->Name,"*");
	    ajDebug("reference sequence:%S\n", contig->Name);
	    ajTablePut(assem->Contigs, contig->Name, contig);
	    ajListPushAppend(assem->ContigsOrder, contig);
	}

	r->Reference = ajListGetLength(assem->ContigsOrder)-1;
	contig->Nreads++;
    }
    ajDebug("RNAME '%S'\n", token);

    ajStrTokenNextParseNoskip(&handle,&token); /* POS */
    ajDebug("POS   '%S'\n", token);

    if(ajStrGetLen(token))
    {
	if(!ajStrToUint(token, &pos))
	{
	    ajErr("SAM %F '%S' invalid POS value %S\n",
	          infile, r->Name, token);
	    return NULL;
	}

	if(r->Flag & BAM_FREVERSE)
	{
	    r->y1 = pos;
	    r->Reversed = ajTrue;
	}
	else
	    r->x1 = pos;
    }

    ajStrTokenNextParseNoskip(&handle,&token); /* MAPQ */
    ajDebug("MAPQ  '%S'\n", token);
    ajStrToInt(token, &r->MapQ);

    ajStrTokenNextParseNoskip(&handle,&token); /* CIGAR */
    ajStrAssignS(&r->Cigar, token);
    ajDebug("CIGAR '%S'\n", token);

    ajStrTokenNextParseNoskip(&handle,&token); /* RNEXT */

    if(ajStrMatchC(token,"="))
	r->Rnext = r->Reference;
    else if(ajStrMatchC(token,"*"))
	r->Rnext = -1;
    else
    {
	contig = ajTableFetchmodS(assem->Contigs, token);

	if(contig)
	{
	    for(i=0;contigs[i];i++)
	    {
		if(contigs[i] == contig)
		{
		    r->Rnext = i;
		    break;
		}
	    }
	}
	else
	    r->Rnext = -1;
    }

    /* in earlier SAM specs PNEXT was known as MPOS */
    ajStrTokenNextParseNoskip(&handle,&token); /* PNEXT */
    if(ajStrGetLen(token))
    {
	if(!ajStrToLong(token, &r->Pnext))
	{
	    ajErr("SAM %F '%S' invalid PNEXT(MPOS) value %S\n",
	          infile, r->Name, token);
	    return NULL;
	}
    }

    ajStrTokenNextParseNoskip(&handle,&token); /* TLEN */
    if(ajStrGetLen(token))
    {
	if(!ajStrToInt(token, &r->Tlen))
	{
	    ajErr("SAM %F '%S' invalid TLEN(ISIZE) value %S\n",
	          infile, r->Name, token);
	    return NULL;
	}
    }

    ajStrTokenNextParseNoskip(&handle,&token); /* SEQ */
    ajDebug("SEQ   '%S'\n", token);
    ajStrAssignS(&r->Seq, token);
    seqlen = MAJSTRGETLEN(token);

    ajStrTokenNextParseNoskip(&handle,&token); /* QUAL */
    ajDebug("QUAL  '%S'\n", token);

    ajStrAssignS(&r->SeqQ, token);

    if(ajStrCmpC(token,"*")!=0)
    {
	if(MAJSTRGETLEN(token) != seqlen)
	{
	    ajErr("SAM quality length mismatch '%F' '%S' "
		    "expected: %u found: %u '%S' '%S'",
		    infile, r->Name,
		    seqlen, ajStrGetLen(token), r->Seq, token);
	    return NULL;
	}

	cp = MAJSTRGETPTR(token);

	ajStrAssignS(&r->SeqQ, token);

	while (*cp)
	{
	    iqual = *cp++;
	    if(iqual < qmin)
	    {
		ajWarn("SAM '%F' sequence '%S' "
			"quality value %d '%c' too low",
			infile, r->Name,
			(ajint) (cp - MAJSTRGETPTR(token)), (char) iqual);
		iqual = qmin;
	    }
	    else if(iqual > qmax)
	    {
		ajWarn("SAM '%F' sequence '%S' "
			"quality value '%c' too high",
			infile, r->Name,
			(char) iqual);
		iqual = qmax;
	    }
	}
    }

    while(ajStrTokenNextParseNoskip(&handle,&token)) /* tags */
    {
	AJNEW0(tag);

	ajStrAssignSubS(&tag->Name, token,0,1);
	tag->type = ajStrGetCharPos(token,3);
	ajStrAssignSubS(&tag->Comment, token, 5, -1);

	if(!r->Tags)
	    r->Tags = ajListNew();

	ajListPushAppend(r->Tags, tag);
    }


    ajStrDel(&token);
    ajStrTokenDel(&handle);

    return r;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Assembly input internals
**
** @nam2rule Assemin Assembly input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Asseminprint
**
** @fcategory output
**
******************************************************************************/




/* @section Print *************************************************************
**
** Printing to a file
**
** @fdata [none]
**
** @nam3rule Book Print as docbook table
** @nam3rule Html Print as html table
** @nam3rule Wiki Print as wiki table
** @nam3rule Text Print as text
**
** @argrule * outf [AjPFile] output file
** @argrule Text full [AjBool] Print all details
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAsseminprintBook ***************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported assembly formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input assembly formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; asseminFormatDef[i].Name; i++)
    {
	if(!asseminFormatDef[i].Alias)
        {
            namestr = ajStrNewC(asseminFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; asseminFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],asseminFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            asseminFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            asseminFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            asseminFormatDef[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }
        

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajAsseminprintHtml ***************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; asseminFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, asseminFormatDef[i].Name);

	if(!asseminFormatDef[i].Alias)
        {
            for(j=i+1; asseminFormatDef[j].Name; j++)
            {
                if(asseminFormatDef[j].Load == asseminFormatDef[i].Load)
                {
                    ajFmtPrintAppS(&namestr, " %s", asseminFormatDef[j].Name);
                    if(!asseminFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               asseminFormatDef[j].Name,
                               asseminFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			asseminFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			asseminFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajAsseminprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; asseminFormatDef[i].Name; i++)
	if(full || !asseminFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			asseminFormatDef[i].Name,
			asseminFormatDef[i].Alias,
			asseminFormatDef[i].Try,
			asseminFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAsseminprintWiki ***************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; asseminFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, asseminFormatDef[i].Name);

	if(!asseminFormatDef[i].Alias)
        {
            for(j=i+1; asseminFormatDef[j].Name; j++)
            {
                if(asseminFormatDef[j].Load == asseminFormatDef[i].Load)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   asseminFormatDef[j].Name);
                    if(!asseminFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               asseminFormatDef[j].Name,
                               asseminFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			asseminFormatDef[i].Try,
			asseminFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "|}\n\n");
    ajStrDel(&namestr);

    return;
}




/* @section Miscellaneous *****************************************************
**
** Functions to initialise and clean up internals
**
** @fdata [none]
**
** @nam3rule Exit Clean up and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajAsseminExit ********************************************************
**
** Cleans up assembly input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAsseminExit(void)
{
    /* Query processing regular expressions */

    ajTableDel(&assemDbMethods);

    ajStrDel(&asseminReadLine);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for assembly datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajAsseminLoad
** @nam5rule Qlinks  Known query link operators for ajAsseminLoad
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajAsseminTypeGetFields ***********************************************
**
** Returns the listof known field names for ajAsseminLoad
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajAsseminTypeGetFields(void)
{
    return "id acc";
}




/* @func ajAsseminTypeGetQlinks ***********************************************
**
** Returns the listof known query link operators for ajAsseminLoad
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajAsseminTypeGetQlinks(void)
{
    return "";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Assemaccess Functions to manage assemdb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] assemdb functions call table
**
** @nam3rule Get Return a value
** @nam4rule Db Database access functions table
** @nam3rule Method Lookup an access method by name
** @nam4rule Test Return true if the access method exists
** @nam4rule MethodGet Return a method value
** @nam5rule Qlinks Return query link operators for a named method
** @nam5rule Scope Return scope (entry, query or all) for a named method
**
** @argrule Method method [const AjPStr] Method name
**
** @valrule *Db [AjPTable] Call table of function names and references
** @valrule *Qlinks [const char*] Query link operators
** @valrule *Scope [ajuint] Scope flags
** @valrule *Test [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAssemaccessGetDb ***************************************************
**
** Returns the table in which assembly database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajAssemaccessGetDb(void)
{
    if(!assemDbMethods)
        assemDbMethods = ajCallTableNew();
    return assemDbMethods;
    
}




/* @func ajAssemaccessMethodGetQlinks *****************************************
**
** Tests for a named method for assembly data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajAssemaccessMethodGetQlinks(const AjPStr method)
{
    const AjPAssemAccess methoddata; 

    methoddata = ajCallTableGetS(assemDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajAssemaccessMethodGetScope ******************************************
**
** Tests for a named method for assembly data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajAssemaccessMethodGetScope(const AjPStr method)
{
    const AjPAssemAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(assemDbMethods, method);
    if(!methoddata)
        return 0;

    if(methoddata->Entry)
        ret |= AJMETHOD_ENTRY;
    if(methoddata->Query)
        ret |= AJMETHOD_QUERY;
    if(methoddata->All)
        ret |= AJMETHOD_ALL;

    return ret;
}




/* @func ajAssemaccessMethodTest **********************************************
**
** Tests for a named method for assembly data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajAssemaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(assemDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic asseminQryRestore **********************************************
**
** Restores an assembly input specification from an AjPQueryList node
**
** @param [w] assemin [AjPAssemin] Assembly input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void asseminQryRestore(AjPAssemin assemin, const AjPQueryList node)
{
    assemin->Input->Format = node->Format;
    assemin->Input->Fpos   = node->Fpos;
    ajStrAssignS(&assemin->Input->Formatstr, node->Formatstr);
    ajStrAssignS(&assemin->Input->QryFields, node->QryFields);

    return;
}




/* @funcstatic asseminQrySave *************************************************
**
** Saves an assembly input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] assemin [const AjPAssemin] Assembly input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void asseminQrySave(AjPQueryList node, const AjPAssemin assemin)
{
    node->Format   = assemin->Input->Format;
    node->Fpos     = assemin->Input->Fpos;
    ajStrAssignS(&node->Formatstr, assemin->Input->Formatstr);
    ajStrAssignS(&node->QryFields, assemin->Input->QryFields);

    return;
}




/* @funcstatic asseminQryProcess **********************************************
**
** Converts an assembly data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using asseminListProcess which in turn invokes asseminQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and assembly data reading
** will have to scan for the entry/entries we need.
**
** @param [u] assemin [AjPAssemin] Assembly data input structure.
** @param [u] assem [AjPAssem] Assembly data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool asseminQryProcess(AjPAssemin assemin, AjPAssem assem)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool assemmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPAssemAccess assemaccess = NULL;
    AjBool foo;

    textin = assemin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("asseminQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, asseminformatFind);
    ajDebug("asseminQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    ajQuerystrParseRange(&qrystr, &assemin->cbegin, &assemin->cend, &foo);


    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("asseminQryProcess ... listfile '%S'\n", qrystr);
        ret = asseminListProcess(assemin, assem, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("asseminQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, asseminformatFind, &assemmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    asseminFormatSet(assemin, assem);

    ajDebug("asseminQryProcess ... read nontext: %B '%S'\n",
            assemmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(assemmethod)
    {
        ajDebug("asseminQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("asseminQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("asseminQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(assemDbMethods,qry->Method);
        assemaccess = qry->Access;
        return (*assemaccess->Access)(assemin);
    }

    ajDebug("asseminQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic asseminListProcess *********************************************
**
** Processes a file of queries.
** This function is called by, and calls, asseminQryProcess. There is
** a depth check to avoid infinite loops, for example where a list file
** refers to itself.
**
** This function produces a list (AjPList) of queries with all list references
** expanded into lists of queries.
**
** Because queries in a list can have their own format
** the prior settings are stored with each query in the list node so that they
** can be restored after.
**
** @param [u] assemin [AjPAssemin] Assembly data input
** @param [u] assem [AjPAssem] Assembly data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool asseminListProcess(AjPAssemin assemin, AjPAssem assem,
                               const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    AjPStrTok handle = NULL;
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;

    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++asseminListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!assemin->Input->List)
	assemin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &asseminReadLine))
    {
	asseminListNoComment(&asseminReadLine);

	if(ajStrGetLen(asseminReadLine))
	{
	    ajStrTokenAssignC(&handle, asseminReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", asseminReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        asseminQrySave(node, assemin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace assemin->Input->List\n");
    ajQuerylistTrace(assemin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(assemin->Input->List, &list);

    ajDebug("Trace combined assemin->Input->List\n");
    ajQuerylistTrace(assemin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(assemin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajAsseminQryS(assemin, node->Qry);
	asseminQryRestore(assemin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", assemin->Input->Qry);
	ret = asseminQryProcess(assemin, assem);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++asseminListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic asseminListNoComment *******************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void asseminListNoComment(AjPStr* text)
{
    ajuint i;
    char *cp;

    i = ajStrGetLen(*text);

    if(!i)				/* empty string */
	return;

    MAJSTRGETUNIQUESTR(text);
    
    cp = strchr(ajStrGetPtr(*text), '#');

    if(cp)
    {					/* comment found */
	*cp = '\0';
	ajStrSetValid(text);
    }

    return;
}




/* @funcstatic asseminFormatSet ***********************************************
**
** Sets the input format for assembly data using the assembly data
** input object's defined format
**
** @param [u] assemin [AjPAssemin] Assembly data input.
** @param [u] assem [AjPAssem] Assembly data
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool asseminFormatSet(AjPAssemin assemin, AjPAssem assem)
{

    if(ajStrGetLen(assemin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                assemin->Input->Formatstr);

	if(asseminformatFind(assemin->Input->Formatstr,
                             &assemin->Input->Format))
	{
	    ajStrAssignS(&assem->Formatstr,
                         assemin->Input->Formatstr);
	    assem->Format = assemin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    assemin->Input->Formatstr,
		    assemin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    assemin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPAssemload] Assembly Loader ********************************
**
** Function is for manipulating assembly loader objects
**
** @nam2rule Assemload Assembly loader objects
**
******************************************************************************/




/* @section Assembly Loader Constructors **************************************
**
** All constructors return a new assembly loader object by pointer. It
** is the responsibility of the user to first destroy any previous
** assembly loader object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPAssemload]
**
** @nam3rule New Constructor
**
** @valrule * [AjPAssemload] Assembly loader object
**
** @fcategory new
**
******************************************************************************/




/* @func ajAssemloadNew *******************************************************
**
** Creates a new assembly loader object.
**
** @return [AjPAssemload] New assembly loader object.
**
** @release 6.5.0
** @@
******************************************************************************/

AjPAssemload ajAssemloadNew(void)
{
    AjPAssemload pthis;

    AJNEW0(pthis);

    pthis->Assemin = ajAsseminNew();
    pthis->Assem   = ajAssemNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Assembly Loader Destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the assembly loader object.
**
** @fdata [AjPAssemload]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPAssemload*] Assembly loader
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajAssemloadDel *******************************************************
**
** Deletes an assembly loader object.
**
** @param [d] pthis [AjPAssemload*] Assembly loader
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajAssemloadDel(AjPAssemload* pthis)
{
    AjPAssemload thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajAsseminDel(&thys->Assemin);
    if(!thys->Returned)
        ajAssemDel(&thys->Assem);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section Assembly loader modifiers *****************************************
**
** These functions use the contents of an assembly loader object and
** update them.
**
** @fdata [AjPAssemload]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPAssemload] Assembly loader object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajAssemloadClear *****************************************************
**
** Clears an assembly loader object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPAssemload] Assembly loader
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajAssemloadClear(AjPAssemload thys)
{

    ajDebug("ajAssemloadClear called\n");

    ajAsseminClear(thys->Assemin);

    ajAssemClear(thys->Assem);

    thys->Loading = ajFalse;

    return;
}




/* @section Assembly loading **************************************************
**
** These functions use an assembly loader object to read data
**
** @fdata [AjPAssemload]
**
** @nam3rule More Test whether more data can be loaded
** @nam3rule Next Load next assembly data
**
** @argrule * thys [AjPAssemload] Assembly loader object
** @argrule * Passem [AjPAssem*] Assembly object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajAssemloadMore ******************************************************
**
** Test whether more assembly data can be loaded
**
** Return the results in the AjPAssem object but leave the file open for
** future calls.
**
** @param [w] thys [AjPAssemload] Assembly loader
** @param [u] Passem [AjPAssem*] Assembly updated
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajAssemloadMore(AjPAssemload thys, AjPAssem *Passem)
{
    AjPAssemin  assemin = thys->Assemin;
    AjPTextin input = assemin->Input;

    ajDebug("ajAssemloadMore count:%u dataread: %B datadone: %B BamIdx: %p\n",
            thys->Count, input->Dataread, input->Datadone, thys->Assemin->BamIdx);

    if(thys->Assemin->BamIdx)
    {
	*Passem = thys->Assem;
        thys->Returned = ajTrue;
	ajBamIndexDel(&thys->Assemin->BamIdx);
	input->Datadone = ajTrue;
	return ajTrue;
    }

    if(input->Datadone)
    {
        input->Dataread = ajFalse;
        input->Datadone = ajFalse;
        ajDebug("ajAssemloadMore done\n");
        return ajFalse;
    }
    
    if(!assemin->Loading)
    {
        assemin->Loading = ajTrue;
        thys->Count++;

	*Passem = thys->Assem;
	thys->Returned = ajTrue;

	ajDebug("ajAssemloadNext initial\n");

	return ajTrue;
    }

    if(ajAsseminLoad(thys->Assemin, thys->Assem))
    {
        *Passem = thys->Assem;
	thys->Returned = ajTrue;

	ajDebug("ajAssemloadMore success\n");

	return ajTrue;
    }

    *Passem = NULL;

    ajDebug("ajAssemloadMore failed to read\n");

    return ajFalse;
}




/* @func ajAssemloadNext ******************************************************
**
** Parse an assembly query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPAssem object but leave the file open for
** future calls.
**
** @param [w] thys [AjPAssemload] Assembly loader
** @param [u] Passem [AjPAssem*] Assembly returned
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajAssemloadNext(AjPAssemload thys, AjPAssem *Passem)
{
    AjPAssemin  assemin = thys->Assemin;
    AjPTextin input = assemin->Input;

    ajDebug("..ajAssemloadNext count:%u dataread: %B datadone: %B\n",
            thys->Count, input->Dataread, input->Datadone);

    if(!thys->Loading)
    {
        thys->Loading = ajTrue;
        assemin->Loading = ajFalse;

	*Passem = thys->Assem;
	thys->Returned = ajTrue;

	ajDebug("ajAssemloadNext initial\n");

	return ajTrue;
    }


    if(ajAsseminLoad(thys->Assemin, thys->Assem))
    {
        assemin->Loading = ajFalse;

	*Passem = thys->Assem;
	thys->Returned = ajTrue;

	ajDebug("ajAssemloadNext success\n");

	return ajTrue;
    }

    *Passem = NULL;

    ajDebug("ajAssemloadNext failed\n");

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Asseminformat Assembly data input format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for input formats
**
** @fdata [none]
**
** @nam3rule Find Return index to named format
** @nam3rule Term Test format EDAM term
** @nam3rule Test Test format value
**
** @argrule Find format [const AjPStr] Format name
** @argrule Term term [const AjPStr] Format EDAM term
** @argrule Test format [const AjPStr] Format name
** @argrule Find iformat [ajint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @funcstatic asseminformatFind **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool asseminformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("asseminformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; asseminFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, asseminFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, asseminFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", asseminFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajAsseminformatTerm **************************************************
**
** Tests whether a assembly data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajAsseminformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; asseminFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(term, asseminFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajAsseminformatTest **************************************************
**
** Tests whether a named assembly data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajAsseminformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; asseminFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, asseminFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}
