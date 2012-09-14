/* @source ajvar **************************************************************
**
** AJAX variation functions
**
** These functions control all aspects of AJAX variation
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.9 $
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

#include "ajvar.h"
#include "ajlist.h"
#include "ajvarread.h"
#include "ajvarwrite.h"
#include "ajtagval.h"


static AjPStr varTempQry = NULL;
static const char * varTypenames[] = {
    "Unknown", "Integer", "Float", "Char", "String", "Flag"
};

static void varMakeQry(const AjPVar thys, AjPStr* qry);




/* @filesection ajvar *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPVar] Variation data ***************************************
**
** Function is for manipulating variation data objects
**
** @nam2rule Var Variation data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPVar]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVar] Variation data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarNew *************************************************************
**
** Variation data constructor
**
** @return [AjPVar] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPVar ajVarNew(void)
{
    AjPVar ret;

    AJNEW0(ret);

    ret->Data = ajListNew();
    ret->Header = ajVarheaderNew();

    return ret;
}




/* @section Variation data destructors *****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation data object.
**
** @fdata [AjPVar]
**
** @nam3rule Del Destructor
**
** @argrule Del Pvar [AjPVar*] Variation data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarDel *************************************************************
**
** Variation data destructor
**
** @param [d] Pvar       [AjPVar*] Variation data object to delete
** @return [void] 
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarDel(AjPVar *Pvar)
{
    AjPVar var;
    AjPVarData vardata = NULL;

    if(!Pvar) return;
    if(!(*Pvar)) return;

    var = *Pvar;

    ajStrDel(&var->Id);
    ajStrDel(&var->Db);
    ajStrDel(&var->Setdb);
    ajStrDel(&var->Full);
    ajStrDel(&var->Qry);
    ajStrDel(&var->Formatstr);
    ajStrDel(&var->Filename);

    ajStrDel(&var->Textptr);
    ajVarheaderDel(&var->Header);

    while(ajListPop(var->Data,(void **)&vardata))
	ajVardataDel(&vardata);

    ajListFree(&var->Data);

    AJFREE(*Pvar);
    *Pvar = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a variation data object
**
** @fdata [AjPVar]
**
** @nam3rule Get Return a value
** @nam4rule Db Source database name
** @nam4rule Id Identifier string
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * var [const AjPVar] Variation data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetId [const AjPStr] Identifier string
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarGetDb ***********************************************************
**
** Return the database name
**
** @param [r] var [const AjPVar] Variation
**
** @return [const AjPStr] Database name
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajVarGetDb(const AjPVar var)
{
    return var->Db;
}




/* @func ajVarGetId ***********************************************************
**
** Return the identifier
**
** @param [r] var [const AjPVar] Variation
**
** @return [const AjPStr] Returned id
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajVarGetId(const AjPVar var)
{
    return var->Id;
}




/* @func ajVarGetQryC *********************************************************
**
** Returns the query string of a variation data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] var [const AjPVar] Variation data object.
** @return [const char*] Query as a character string.
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajVarGetQryC(const AjPVar var)
{
    return MAJSTRGETPTR(ajVarGetQryS(var));
}




/* @func ajVarGetQryS *********************************************************
**
** Returns the query string of a variation data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] var [const AjPVar] Variation data object.
** @return [const AjPStr] Query as a string.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajVarGetQryS(const AjPVar var)
{
    ajDebug("ajVarGetQryS '%S'\n", var->Qry);

    if(ajStrGetLen(var->Qry))
	return var->Qry;

    varMakeQry(var, &varTempQry);

    return varTempQry;
}




/* @funcstatic varMakeQry *****************************************************
**
** Sets the query for a variation data object.
**
** @param [r] thys [const AjPVar] Variation data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void varMakeQry(const AjPVar thys, AjPStr* qry)
{
    ajDebug("varMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajVarTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(qry, "%S-id:%S", thys->Db, thys->Id);
    else
    {
	ajFmtPrintS(qry, "%S::%S:%S", thys->Formatstr,
                    thys->Filename,thys->Id);
    }

    ajDebug("      result: <%S>\n",
	    *qry);

    return;
}




/* @section variation data modifiers *******************************************
**
** Variation data modifiers
**
** @fdata [AjPVar]
**
** @nam3rule Clear Clear internal values
** @nam3rule Reset Reset internal data values
**
** @argrule * var [AjPVar] Variation data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarClear ***********************************************************
**
** Resets all data for a variation data object so that it can be reused.
**
** @param [u] var [AjPVar] Variation data object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarClear(AjPVar var)
{
    AjPVarData vardata = NULL;

    if(MAJSTRGETLEN(var->Id))
       ajStrSetClear(&var->Id);

    if(MAJSTRGETLEN(var->Db))
       ajStrSetClear(&var->Db);

    if(MAJSTRGETLEN(var->Setdb))
       ajStrSetClear(&var->Setdb);

    if(MAJSTRGETLEN(var->Full))
       ajStrSetClear(&var->Full);

    if(MAJSTRGETLEN(var->Qry))
       ajStrSetClear(&var->Qry);

    if(MAJSTRGETLEN(var->Formatstr))
       ajStrSetClear(&var->Formatstr);

    if(MAJSTRGETLEN(var->Filename))
       ajStrSetClear(&var->Filename);

    ajStrDel(&var->Textptr);

    ajVarheaderClear(var->Header);

    while(ajListPop(var->Data,(void **)&vardata))
	ajVardataDel(&vardata);

    var->Hasdata = ajFalse;
    var->Fpos = 0L;
    var->Format = 0;

    return;
}




/* @func ajVarReset ***********************************************************
**
** Resets all data for a variation data object so that it can be reused.
**
** @param [u] var [AjPVar] Variation data object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarReset(AjPVar var)
{
    AjPVarData vardata = NULL;

    ajDebug("ajVarReset data: %Lu\n", ajListGetLength(var->Data));

    while(ajListPop(var->Data,(void **)&vardata))
	ajVardataDel(&vardata);

    return;
}




/* @datasection [AjPVarData] Variation data records****************************
**
** Function is for manipulating variation data record objects
**
** @nam2rule Vardata Variation data record objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPVarData]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarData] Variation data record object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVardataNew *********************************************************
**
** Variation data record constructor
**
** @return [AjPVarData] New object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPVarData ajVardataNew(void)
{
    AjPVarData ret;

    AJNEW0(ret);

    ret->Samples = ajListNew();

    return ret;
}




/* @section Variation data record destructors *********************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation data object.
**
** @fdata [AjPVarData]
**
** @nam3rule Del Destructor
**
** @argrule Del Pvar [AjPVarData*] Variation data record
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVardataDel *********************************************************
**
** Variation data record destructor
**
** @param [d] Pvar       [AjPVarData*] Variation data record object to delete
** @return [void] 
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVardataDel(AjPVarData *Pvar)
{
    AjPVarData var;
    AjPStr samplestr = NULL;

    if(!Pvar) return;
    if(!(*Pvar)) return;

    var = *Pvar;

    ajStrDel(&var->Chrom);
    ajStrDel(&var->Id);
    ajStrDel(&var->Ref);
    ajStrDel(&var->Alt);
    ajStrDel(&var->Qual);
    ajStrDel(&var->Filter);
    ajStrDel(&var->Info);
    ajStrDel(&var->Format);

    while(ajListPop(var->Samples,(void **)&samplestr))
	ajStrDel(&samplestr);
    ajListFree(&var->Samples);

    AJFREE(*Pvar);
    *Pvar = NULL;

    return;
}




/* @section variation data record modifiers ************************************
**
** Variation data record modifiers
**
** @fdata [AjPVarData]
**
** @nam3rule Clear clear internal values
**
** @argrule * var [AjPVarData] Variation data record object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVardataClear *******************************************************
**
** Resets all data for a variation data record object so that it can be reused.
**
** @param [u] var [AjPVarData] Variation data record object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVardataClear(AjPVarData var)
{
    AjPStr samplestr = NULL;

    if(MAJSTRGETLEN(var->Chrom))
       ajStrSetClear(&var->Chrom);

    if(MAJSTRGETLEN(var->Id))
       ajStrSetClear(&var->Id);

    if(MAJSTRGETLEN(var->Ref))
       ajStrSetClear(&var->Ref);

    if(MAJSTRGETLEN(var->Alt))
       ajStrSetClear(&var->Alt);

    if(MAJSTRGETLEN(var->Qual))
       ajStrSetClear(&var->Qual);

    if(MAJSTRGETLEN(var->Filter))
       ajStrSetClear(&var->Filter);

    if(MAJSTRGETLEN(var->Info))
       ajStrSetClear(&var->Info);

    if(MAJSTRGETLEN(var->Format))
       ajStrSetClear(&var->Format);

    while(ajListPop(var->Samples,(void **)&samplestr))
	ajStrDel(&samplestr);

    var->Pos = 0;

    return;
}




/* @datasection [AjPVarField] Variation field *********************************
**
** Function is for manipulating variation field objects
**
** @nam2rule Varfield Variation field objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPVarField]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarField] Variation field object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarfieldNew ********************************************************
**
** Variation data constructor
**
** @return [AjPVarField] New object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPVarField ajVarfieldNew(void)
{
    AjPVarField ret;

    AJNEW0(ret);

    return ret;
}




/* @section Variation field destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation data object.
**
** @fdata [AjPVarField]
**
** @nam3rule Del Destructor
**
** @argrule Del Pvar [AjPVarField*] Variation field
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarfieldDel ********************************************************
**
** Variation field destructor
**
** @param [d] Pvar       [AjPVarField*] Variation field object to delete
** @return [void] 
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarfieldDel(AjPVarField *Pvar)
{
    AjPVarField var;

    if(!Pvar) return;
    if(!(*Pvar)) return;

    var = *Pvar;

    ajStrDel(&var->Id);
    ajStrDel(&var->Field);
    ajStrDel(&var->Desc);

    AJFREE(*Pvar);
    *Pvar = NULL;

    return;
}




/* @section variation field modifiers ******************************************
**
** Variation field modifiers
**
** @fdata [AjPVarField]
**
** @nam3rule Clear clear internal values
** @nam3rule Parse Read field data from a VCF header record
**
** @argrule * var [AjPVarField] Variation field object
** @argrule Parse str [const AjPStr] VCF header record
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarfieldClear ******************************************************
**
** Resets all data for a variation field object so that it can be reused.
**
** @param [u] var [AjPVarField] Variation field object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarfieldClear(AjPVarField var)
{
    if(MAJSTRGETLEN(var->Id))
       ajStrSetClear(&var->Id);

    if(MAJSTRGETLEN(var->Desc))
       ajStrSetClear(&var->Desc);

    if(MAJSTRGETLEN(var->Field))
       ajStrSetClear(&var->Field);

    var->Type = AJVAR_UNKNOWN;
    var->Number = 0;

    return;
}




/* @section variation field casts *********************************************
**
** Variation field casts
**
** @fdata [AjPVarField]
**
** @nam3rule Get Return an attribute value
** @nam4rule GetNumber Return the header value for the Number field
** @nam4rule GetType   Return the header value for the Type field
**
** @argrule * var [const AjPVarField] Variation field object
**
** @valrule * [const char*] Header field value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarfieldGetNumber **************************************************
**
** Return the header value for the Number field
**
** @param [r] var [const AjPVarField] Variation field object
** @return [const char*] Header field value
**
** @release 6.5.0
** @@
******************************************************************************/

const char* ajVarfieldGetNumber(const AjPVarField var)
{
    static char numberfield[128] = {' ', '\0'};

    switch(var->Number)
    {
        case varNumAny:
            return ".";
        case varNumAlt:
            return "A";
        case varNumGen:
            return "G";
        default:
            sprintf(numberfield, "%d", var->Number);
    }

    return numberfield;
}




/* @func ajVarfieldGetType ****************************************************
**
** Return the header value for the Type field
**
** @param [r] var [const AjPVarField] Variation field object
** @return [const char*] Header field value
**
** @release 6.5.0
** @@
******************************************************************************/

const char* ajVarfieldGetType(const AjPVarField var)
{
    if(var->Type <= AJVAR_UNKNOWN || var->Type >= AJVAR_MAX)
        return "Invalid";

    return  varTypenames[var->Type];
}




/* @datasection [AjPVarHeader] Variation header *******************************
**
** Function is for manipulating variation header objects
**
** @nam2rule Varheader Variation header objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPVarHeader]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarHeader] Variation header object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarheaderNew *******************************************************
**
** Variation header constructor
**
** @return [AjPVarHeader] New object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPVarHeader ajVarheaderNew(void)
{
    AjPVarHeader ret;

    AJNEW0(ret);

    ret->Header    = ajListNew();
    ret->Fields    = ajListNew();
    ret->Samples   = ajListNew();
    ret->Filters   = ajListNew();
    ret->Alts      = ajListNew();
    ret->Pedigrees = ajListNew();
    ret->SampleIds       = ajListNew();
    ret->RefseqIds = ajTablestrNew(16);

    return ret;
}




/* @section Variation header destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation data object.
**
** @fdata [AjPVarHeader]
**
** @nam3rule Del Destructor
**
** @argrule Del Pvar [AjPVarHeader*] Variation header
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarheaderDel *******************************************************
**
** Variation header destructor
**
** @param [d] Pvar       [AjPVarHeader*] Variation header object to delete
** @return [void] 
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarheaderDel(AjPVarHeader *Pvar)
{
    AjPVarHeader var;
    AjPVarField varfield = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjPStr tmpstr = NULL;

    if(!Pvar) return;
    if(!(*Pvar)) return;

    var = *Pvar;

    while(ajListPop(var->Header,(void **)&tagval))
	ajTagvalDel(&tagval);
    ajListFree(&var->Header);

    while(ajListPop(var->Fields,(void **)&varfield))
	ajVarfieldDel(&varfield);
    ajListFree(&var->Fields);

    while(ajListPop(var->Samples,(void **)&varsample))
	ajVarsampleDel(&varsample);
    ajListFree(&var->Samples);

    while(ajListPop(var->Filters,(void **)&tagval))
	ajTagvalDel(&tagval);
    ajListFree(&var->Filters);

    while(ajListPop(var->Alts,(void **)&tagval))
	ajTagvalDel(&tagval);
    ajListFree(&var->Alts);

    while(ajListPop(var->Pedigrees,(void **)&tagval))
	ajTagvalDel(&tagval);
    ajListFree(&var->Pedigrees);

    while(ajListPop(var->SampleIds,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListFree(&var->SampleIds);

    ajTableSetDestroyvalue(var->RefseqIds,
	    (void(*)(void**))&ajMemFree);
    ajTableDel(&var->RefseqIds);

    AJFREE(*Pvar);
    *Pvar = NULL;

    return;
}




/* @section variation header modifiers *****************************************
**
** Variation header modifiers
**
** @fdata [AjPVarHeader]
**
** @nam3rule Clear clear internal values
**
** @argrule * var [AjPVarHeader] Variation header object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarheaderClear *****************************************************
**
** Resets all data for a variation header object so that it can be reused.
**
** @param [u] var [AjPVarHeader] Variation header object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarheaderClear(AjPVarHeader var)
{
    AjPVarField varfield = NULL;
    AjPVarSample varsample = NULL;
    AjPTagval tagval = NULL;
    AjPStr tmpstr = NULL;

    if(!var) return;

    while(ajListPop(var->Header,(void **)&tagval))
	ajTagvalDel(&tagval);

    while(ajListPop(var->Fields,(void **)&varfield))
	ajVarfieldDel(&varfield);

    while(ajListPop(var->Samples,(void **)&varsample))
	ajVarsampleDel(&varsample);

    while(ajListPop(var->Filters,(void **)&tagval))
	ajTagvalDel(&tagval);

    while(ajListPop(var->Alts,(void **)&tagval))
	ajTagvalDel(&tagval);

    while(ajListPop(var->Pedigrees,(void **)&tagval))
	ajTagvalDel(&tagval);

    while(ajListPop(var->SampleIds,(void **)&tmpstr))
	ajStrDel(&tmpstr);

   return;
}




/* @datasection [AjPVarSample] Variation sample *******************************
**
** Function is for manipulating variation sample objects
**
** @nam2rule Varsample Variation sample objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPVarSample]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarSample] Variation sample object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarsampleNew *******************************************************
**
** Variation sample constructor
**
** @return [AjPVarSample] New object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPVarSample ajVarsampleNew(void)
{
    AjPVarSample ret;

    AJNEW0(ret);

    return ret;
}




/* @section Variation sample destructors **************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation sample object.
**
** @fdata [AjPVarSample]
**
** @nam3rule Del Destructor
**
** @argrule Del Pvar [AjPVarSample*] Variation sample
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarsampleDel *******************************************************
**
** Variation sample destructor
**
** @param [d] Pvar       [AjPVarSample*] Variation sample object to delete
** @return [void] 
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarsampleDel(AjPVarSample *Pvar)
{
    AjPVarSample var;

    if(!Pvar) return;
    if(!(*Pvar)) return;

    var = *Pvar;

    ajStrDel(&var->Id);
    ajStrDelarray(&var->Desc);
    ajStrDelarray(&var->Genomes);
    AJFREE(var->Mixture);

    AJFREE(*Pvar);
    *Pvar = NULL;

    return;
}




/* @section variation sample modifiers *****************************************
**
** Variation sample modifiers
**
** @fdata [AjPVarSample]
**
** @nam3rule Clear clear internal values
**
** @argrule * var [AjPVarSample] Variation sample object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarsampleClear *****************************************************
**
** Resets all data for a variation sample object so that it can be reused.
**
** @param [u] var [AjPVarSample] Variation sample object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarsampleClear(AjPVarSample var)
{
    if(MAJSTRGETLEN(var->Id))
       ajStrSetClear(&var->Id);

    ajStrDelarray(&var->Desc);
    ajStrDelarray(&var->Genomes);
    AJFREE(var->Mixture);

    var->Number = 0;

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Var Variation internals
**
******************************************************************************/




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajVarExit ************************************************************
**
** Cleans up variation processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarExit(void)
{
    ajVarinExit();
    ajVaroutExit();

    return;
}
