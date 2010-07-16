/******************************************************************************
** @source Ensembl Analysis functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.14 $
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

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensanalysis.h"
#include "ensbaseadaptor.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool analysisadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                           const AjPStr statement,
                                           EnsPAssemblymapper am,
                                           EnsPSlice slice,
                                           AjPList analyses);

static AjBool analysisadaptorCacheInsert(EnsPAnalysisadaptor aa,
                                         EnsPAnalysis *Panalysis);

static AjBool analysisadaptorCacheInit(EnsPAnalysisadaptor aa);

static void analysisadaptorCacheClearIdentifier(void **key,
                                                void **value,
                                                void *cl);

static void analysisadaptorCacheClearName(void **key,
                                          void **value,
                                          void *cl);

static AjBool analysisadaptorCacheExit(EnsPAnalysisadaptor aa);

static void analysisadaptorFetchAll(const void *key, void **value, void *cl);




/* @filesection ensanalysis ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPAnalysis] Analysis ***************************************
**
** Functions for manipulating Ensembl Analysis objects
**
** @cc Bio::EnsEMBL::Analysis CVS Revision: 1.27
**
** @nam2rule Analysis
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Analysis by pointer.
** It is the responsibility of the user to first destroy any previous
** Analysis. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAnalysis]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewData Constructor with data values
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [const EnsPAnalysis] Ensembl Analysis
** @argrule Ref object [EnsPAnalysis] Ensembl Analysis
** @argrule Data aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @argrule Data identifier [ajuint] SQL database-internal identifier
** @argrule Data cdate [AjPStr] Creation date
** @argrule Data name [AjPStr] Name
** @argrule Data databasename [AjPStr] Database name
** @argrule Data databaseversion [AjPStr] Database version
** @argrule Data databasefile [AjPStr] Database file
** @argrule Data programname [AjPStr] Program name
** @argrule Data programversion [AjPStr] Program version
** @argrule Data programfile [AjPStr] Program file
** @argrule Data parameters [AjPStr] Parameters
** @argrule Data modulename [AjPStr] Module name
** @argrule Data moduleversion [AjPStr] Module version
** @argrule Data gffsource [AjPStr] GFF source
** @argrule Data gfffeature [AjPStr] GFF feature
** @argrule Data description [AjPStr] Description
** @argrule Data displaylabel [AjPStr] Display label
** @argrule Data displayable [AjBool] Displayable element
** @argrule Data webdata [AjPStr] Web data
**
** @valrule * [EnsPAnalysis] Ensembl Analysis
**
** @fcategory new
******************************************************************************/




/* @func ensAnalysisNewData ***************************************************
**
** Default constructor for an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Analysis::new
** @param [u] cdate [AjPStr] Creation date
** @param [u] name [AjPStr] Name
** @param [u] databasename [AjPStr] Database name
** @param [u] databaseversion [AjPStr] Database version
** @param [u] databasefile [AjPStr] Database file
** @param [u] programname [AjPStr] Program name
** @param [u] programversion [AjPStr] Program version
** @param [u] programfile [AjPStr] Program file
** @param [u] parameters [AjPStr] Parameters
** @param [u] modulename [AjPStr] Module name
** @param [u] moduleversion [AjPStr] Module version
** @param [u] gffsource [AjPStr] GFF source
** @param [u] gfffeature [AjPStr] GFF feature
** @param [u] description [AjPStr] Description
** @param [u] displaylabel [AjPStr] Display label
** @param [r] displayable [AjBool] Displayable element
** @param [u] webdata [AjPStr] Web data
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
** @@
******************************************************************************/

EnsPAnalysis ensAnalysisNewData(EnsPAnalysisadaptor aa,
                                ajuint identifier,
                                AjPStr cdate,
                                AjPStr name,
                                AjPStr databasename,
                                AjPStr databaseversion,
                                AjPStr databasefile,
                                AjPStr programname,
                                AjPStr programversion,
                                AjPStr programfile,
                                AjPStr parameters,
                                AjPStr modulename,
                                AjPStr moduleversion,
                                AjPStr gffsource,
                                AjPStr gfffeature,
                                AjPStr description,
                                AjPStr displaylabel,
                                AjBool displayable,
                                AjPStr webdata)
{
    EnsPAnalysis analysis = NULL;

    if(!name)
        return NULL;

    AJNEW0(analysis);

    analysis->Use = 1;

    analysis->Identifier = identifier;

    analysis->Adaptor = aa;

    if(cdate)
        analysis->CreationDate = ajStrNewRef(cdate);

    if(name)
        analysis->Name = ajStrNewRef(name);

    if(databasename)
        analysis->DatabaseName = ajStrNewRef(databasename);

    if(databaseversion)
        analysis->DatabaseVersion = ajStrNewRef(databaseversion);

    if(databasefile)
        analysis->DatabaseFile = ajStrNewRef(databasefile);

    if(programname)
        analysis->ProgramName = ajStrNewRef(programname);

    if(programversion)
        analysis->ProgramVersion = ajStrNewRef(programversion);

    if(programfile)
        analysis->ProgramFile = ajStrNewRef(programfile);

    if(parameters)
        analysis->Parameters = ajStrNewRef(parameters);

    if(modulename)
        analysis->ModuleName = ajStrNewRef(modulename);

    if(moduleversion)
        analysis->ModuleVersion = ajStrNewRef(moduleversion);

    if(gffsource)
        analysis->GFFSource = ajStrNewRef(gffsource);

    if(gfffeature)
        analysis->GFFFeature = ajStrNewRef(gfffeature);

    if(description)
        analysis->Description = ajStrNewRef(description);

    if(displaylabel)
        analysis->DisplayLabel = ajStrNewRef(displaylabel);

    analysis->Displayable = displayable;

    if(webdata)
        analysis->WebData = ajStrNewRef(webdata);

    return analysis;
}




/* @func ensAnalysisNewObj ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
** @@
******************************************************************************/

EnsPAnalysis ensAnalysisNewObj(const EnsPAnalysis object)
{
    EnsPAnalysis analysis = NULL;

    if(!object)
        return NULL;

    AJNEW0(analysis);

    analysis->Use = 1;

    analysis->Identifier = object->Identifier;

    analysis->Adaptor = object->Adaptor;

    if(object->CreationDate)
        analysis->CreationDate = ajStrNewRef(object->CreationDate);

    if(object->Name)
        analysis->Name = ajStrNewRef(object->Name);

    if(object->DatabaseName)
        analysis->DatabaseName = ajStrNewRef(object->DatabaseName);

    if(object->DatabaseVersion)
        analysis->DatabaseVersion = ajStrNewRef(object->DatabaseVersion);

    if(object->DatabaseFile)
        analysis->DatabaseFile = ajStrNewRef(object->DatabaseFile);

    if(object->ProgramName)
        analysis->ProgramName = ajStrNewRef(object->ProgramName);

    if(object->ProgramVersion)
        analysis->ProgramVersion = ajStrNewRef(object->ProgramVersion);

    if(object->ProgramFile)
        analysis->ProgramFile = ajStrNewRef(object->ProgramFile);

    if(object->Parameters)
        analysis->Parameters = ajStrNewRef(object->Parameters);

    if(object->ModuleName)
        analysis->ModuleName = ajStrNewRef(object->ModuleName);

    if(object->ModuleVersion)
        analysis->ModuleVersion = ajStrNewRef(object->ModuleVersion);

    if(object->GFFSource)
        analysis->GFFSource = ajStrNewRef(object->GFFSource);

    if(object->GFFFeature)
        analysis->GFFFeature = ajStrNewRef(object->GFFFeature);

    if(object->Description)
        analysis->Description = ajStrNewRef(object->Description);

    if(object->DisplayLabel)
        analysis->DisplayLabel = ajStrNewRef(object->DisplayLabel);

    analysis->Displayable = object->Displayable;

    if(object->WebData)
        analysis->WebData = ajStrNewRef(object->WebData);

    return analysis;
}




/* @func ensAnalysisNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
** @@
******************************************************************************/

EnsPAnalysis ensAnalysisNewRef(EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    analysis->Use++;

    return analysis;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Analysis.
**
** @fdata [EnsPAnalysis]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Analysis object
**
** @argrule * Panalysis [EnsPAnalysis*] Ensembl Analysis object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAnalysisDel *******************************************************
**
** Default destructor for an Ensembl Analysis.
**
** @param [d] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [void]
** @@
******************************************************************************/

void ensAnalysisDel(EnsPAnalysis* Panalysis)
{
    EnsPAnalysis pthis = NULL;

    if(!Panalysis)
        return;

    if(!*Panalysis)
        return;

    if(ajDebugTest("ensAnalysisDel"))
    {
        ajDebug("ensAnalysisDel\n"
                "  *Panalysis %p\n",
                *Panalysis);

        ensAnalysisTrace(*Panalysis, 1);
    }

    pthis = *Panalysis;

    pthis->Use--;

    if(pthis->Use)
    {
        *Panalysis = NULL;

        return;
    }

    ajStrDel(&pthis->CreationDate);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->DatabaseName);
    ajStrDel(&pthis->DatabaseVersion);
    ajStrDel(&pthis->DatabaseFile);
    ajStrDel(&pthis->ProgramName);
    ajStrDel(&pthis->ProgramVersion);
    ajStrDel(&pthis->ProgramFile);
    ajStrDel(&pthis->Parameters);
    ajStrDel(&pthis->ModuleName);
    ajStrDel(&pthis->ModuleVersion);
    ajStrDel(&pthis->GFFSource);
    ajStrDel(&pthis->GFFFeature);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->DisplayLabel);
    ajStrDel(&pthis->WebData);

    AJFREE(pthis);

    *Panalysis = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
** @fnote None
**
** @nam3rule Get Return Analysis attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Analysis Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetCreationdate Return the creation date
** @nam4rule GetName Return the name
** @nam4rule GetDatabasename Return the database name
** @nam4rule GetDatabaseversion Return the database version
** @nam4rule GetDatabasefile Return the database file
** @nam4rule GetProgramname Return the program name
** @nam4rule GetProgramversion Return the program version
** @nam4rule GetProgramfile Return the program file
** @nam4rule GetParameters Return the parameters
** @nam4rule GetModulename Return the module name
** @nam4rule GetModuleversion Return the module version
** @nam4rule GetGffsource Return the GFF source
** @nam4rule GetGfffeature Return the GFF feature
** @nam4rule GetDescription Return the description
** @nam4rule GetDisplaylabel Return the display label
** @nam4rule GetMemsize Return the memory size used
** @nam4rule GetWebdata Return the web data
** @nam4rule GetDisplayable Return the displayable element
**
** @argrule * analysis [const EnsPAnalysis] Analysis
**
** @valrule Adaptor [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Creationdate [const AjPStr] Creation date
** @valrule Name [const AjPStr] Name
** @valrule Databasename [const AjPStr] Database name
** @valrule Databaseversion [const AjPStr] Database version
** @valrule Databasefile [const AjPStr] Database file
** @valrule Programname [const AjPStr] Program name
** @valrule Programversion [const AjPStr] Program version
** @valrule Programfile [const AjPStr] Program file
** @valrule Parameters [const AjPStr] Parameters
** @valrule Modulename [const AjPStr] Module name
** @valrule Moduleversion [const AjPStr] Module version
** @valrule Gffsource [const AjPStr] GFF source
** @valrule Gfffeature [const AjPStr] GFF feature
** @valrule Description [const AjPStr] Description
** @valrule Displaylabel [const AjPStr] Display label
** @valrule Webdata [const AjPStr] Web data
** @valrule Displayable [AjBool] Displayable element
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisGetAdaptor ************************************************
**
** Get the Ensembl Analysis Adaptor element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @@
******************************************************************************/

EnsPAnalysisadaptor ensAnalysisGetAdaptor(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->Adaptor;
}




/* @func ensAnalysisGetCreationdate *******************************************
**
** Get the creation date element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::created
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Creation date
** @@
******************************************************************************/

const AjPStr ensAnalysisGetCreationdate(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->CreationDate;
}




/* @func ensAnalysisGetDatabasename *******************************************
**
** Get the database name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Database name
** @@
******************************************************************************/

const AjPStr ensAnalysisGetDatabasename(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->DatabaseName;
}




/* @func ensAnalysisGetDatabasefile *******************************************
**
** Get the database file element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_file
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Database file
** @@
******************************************************************************/

const AjPStr ensAnalysisGetDatabasefile(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->DatabaseFile;
}




/* @func ensAnalysisGetDatabaseversion ****************************************
**
** Get the database version element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_version
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Database version
** @@
******************************************************************************/

const AjPStr ensAnalysisGetDatabaseversion(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->DatabaseVersion;
}




/* @func ensAnalysisGetDescription ********************************************
**
** Get the description element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::description
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Description
** @@
******************************************************************************/

const AjPStr ensAnalysisGetDescription(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->Description;
}




/* @func ensAnalysisGetDisplayable ********************************************
**
** Get the displayable element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::displayable
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] Displayable element
** @@
******************************************************************************/

AjBool ensAnalysisGetDisplayable(const EnsPAnalysis analysis)
{
    if(!analysis)
        return ajFalse;

    return analysis->Displayable;
}




/* @func ensAnalysisGetDisplaylabel *******************************************
**
** Get the display label element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::display_label
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Display label
** @@
******************************************************************************/

const AjPStr ensAnalysisGetDisplaylabel(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->DisplayLabel;
}




/* @func ensAnalysisGetGfffeature *********************************************
**
** Get the GFF feature element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_feature
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] GFF feature
** @@
******************************************************************************/

const AjPStr ensAnalysisGetGfffeature(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->GFFFeature;
}




/* @func ensAnalysisGetGffsource **********************************************
**
** Get the GFF source element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_source
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] GFF source
** @@
******************************************************************************/

const AjPStr ensAnalysisGetGffsource(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->GFFSource;
}




/* @func ensAnalysisGetIdentifier *********************************************
**
** Get the SQL database-internal identifier element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensAnalysisGetIdentifier(const EnsPAnalysis analysis)
{
    if(!analysis)
        return 0;

    return analysis->Identifier;
}




/* @func ensAnalysisGetMemsize ************************************************
**
** Get the memory size in bytes of an Ensembl Analysis.
**
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensAnalysisGetMemsize(const EnsPAnalysis analysis)
{
    ajulong size = 0;

    if(!analysis)
        return 0;

    size += sizeof (EnsOAnalysis);

    if(analysis->CreationDate)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->CreationDate);
    }

    if(analysis->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Name);
    }

    if(analysis->DatabaseName)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->DatabaseName);
    }

    if(analysis->DatabaseVersion)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->DatabaseVersion);
    }

    if(analysis->DatabaseFile)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->DatabaseFile);
    }

    if(analysis->ProgramName)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->ProgramName);
    }

    if(analysis->ProgramVersion)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->ProgramVersion);
    }

    if(analysis->ProgramFile)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->ProgramFile);
    }

    if(analysis->Parameters)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Parameters);
    }

    if(analysis->ModuleName)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->ModuleName);
    }

    if(analysis->ModuleVersion)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->ModuleVersion);
    }

    if(analysis->GFFSource)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->GFFSource);
    }

    if(analysis->GFFFeature)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->GFFFeature);
    }

    if(analysis->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Description);
    }

    if(analysis->DisplayLabel)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->DisplayLabel);
    }

    if(analysis->WebData)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->WebData);
    }

    return size;
}




/* @func ensAnalysisGetModulename *********************************************
**
** Get the module name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Module name
** @@
******************************************************************************/

const AjPStr ensAnalysisGetModulename(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->ModuleName;
}




/* @func ensAnalysisGetModuleversion ******************************************
**
** Get the module version element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module_version
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Module version
** @@
******************************************************************************/

const AjPStr ensAnalysisGetModuleversion(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->ModuleVersion;
}




/* @func ensAnalysisGetName ***************************************************
**
** Get the name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::logic_name
** @param  [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Name
** @@
******************************************************************************/

const AjPStr ensAnalysisGetName(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->Name;
}




/* @func ensAnalysisGetParameters *********************************************
**
** Get the parameters element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::parameters
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Parameters
** @@
******************************************************************************/

const AjPStr ensAnalysisGetParameters(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->Parameters;
}




/* @func ensAnalysisGetProgramfile ********************************************
**
** Get the program file element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_file
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Program file
** @@
******************************************************************************/

const AjPStr ensAnalysisGetProgramfile(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->ProgramFile;
}




/* @func ensAnalysisGetProgramname ********************************************
**
** Get the program name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Program name
** @@
******************************************************************************/

const AjPStr ensAnalysisGetProgramname(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->ProgramName;
}




/* @func ensAnalysisGetProgramversion *****************************************
**
** Get the program version element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_version
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Program version
** @@
******************************************************************************/

const AjPStr ensAnalysisGetProgramversion(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->ProgramVersion;
}





/* @func ensAnalysisGetWebdata ************************************************
**
** Get the web data element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::web_data
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [const AjPStr] Web data
** @@
******************************************************************************/

const AjPStr ensAnalysisGetWebdata(const EnsPAnalysis analysis)
{
    if(!analysis)
        return NULL;

    return analysis->WebData;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
** @fnote None
**
** @nam3rule Set Set one element of an Ensembl Analysis
** @nam4rule SetAdaptor Set the Ensembl Analysis Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetCreationdate Set the creation date
** @nam4rule SetName Set the name
** @nam4rule SetDatabasename Set the database name
** @nam4rule SetDatabaseversion Set the database version
** @nam4rule SetDatabasefile Set the database file
** @nam4rule SetProgramname Set the program name
** @nam4rule SetProgramversion Set the program version
** @nam4rule SetProgramfile Set the program file
** @nam4rule SetParameters Set the parameters
** @nam4rule SetModulename Set the module name
** @nam4rule SetModuleversion Set the module version
** @nam4rule SetGffsource Set the GFF source
** @nam4rule SetGfffeature Set the GFF feature
** @nam4rule SetDescription Set the description
** @nam4rule SetDisplaylabel Set the display label
** @nam4rule SetWebdata Set the web data
** @nam4rule SetDisplayable Set the displayable element
**
** @argrule * analysis [EnsPAnalysis] Ensembl Analysis object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensAnalysisSetAdaptor ************************************************
**
** Set the Ensembl Analysis Adaptor element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetAdaptor(EnsPAnalysis analysis,
                             EnsPAnalysisadaptor aa)
{
    if(!analysis)
        return ajFalse;

    analysis->Adaptor = aa;

    return ajTrue;
}




/* @func ensAnalysisSetCreationdate *******************************************
**
** Set the creation date element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::created
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetCreationdate(EnsPAnalysis analysis, AjPStr cdate)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->CreationDate);

    analysis->CreationDate = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensAnalysisSetDatabasename *******************************************
**
** Set the database name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] databasename [AjPStr] Database name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetDatabasename(EnsPAnalysis analysis, AjPStr databasename)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->DatabaseName);

    analysis->DatabaseName = ajStrNewRef(databasename);

    return ajTrue;
}




/* @func ensAnalysisSetDatabasefile *******************************************
**
** Set the database file element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_file
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] databasefile [AjPStr] Database file
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetDatabasefile(EnsPAnalysis analysis, AjPStr databasefile)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->DatabaseFile);

    analysis->DatabaseFile = ajStrNewRef(databasefile);

    return ajTrue;
}




/* @func ensAnalysisSetDatabaseversion ****************************************
**
** Set the database version element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_version
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] databaseversion [AjPStr] Database version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetDatabaseversion(EnsPAnalysis analysis,
                                     AjPStr databaseversion)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->DatabaseVersion);

    analysis->DatabaseVersion = ajStrNewRef(databaseversion);

    return ajTrue;
}




/* @func ensAnalysisSetDescription ********************************************
**
** Set the description element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::description
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetDescription(EnsPAnalysis analysis, AjPStr description)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->Description);

    analysis->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensAnalysisSetDisplayable ********************************************
**
** Set the displayable element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::displayable
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [r] displayable [AjBool] Displayable element
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetDisplayable(EnsPAnalysis analysis, AjBool displayable)
{
    if(!analysis)
        return ajFalse;

    analysis->Displayable = displayable;

    return ajTrue;
}




/* @func ensAnalysisSetDisplaylabel *******************************************
**
** Set the display label element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::display_label
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] displaylabel [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetDisplaylabel(EnsPAnalysis analysis, AjPStr displaylabel)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->DisplayLabel);

    analysis->DisplayLabel = ajStrNewRef(displaylabel);

    return ajTrue;
}




/* @func ensAnalysisSetIdentifier *********************************************
**
** Set the SQL database-internal identifier element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetIdentifier(EnsPAnalysis analysis, ajuint identifier)
{
    if(!analysis)
        return ajFalse;

    analysis->Identifier = identifier;

    return ajTrue;
}




/* @func ensAnalysisSetModulename *********************************************
**
** Set the module name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] modulename [AjPStr] Module name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetModulename(EnsPAnalysis analysis, AjPStr modulename)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->ModuleName);

    analysis->ModuleName = ajStrNewRef(modulename);

    return ajTrue;
}




/* @func ensAnalysisSetModuleversion ******************************************
**
** Set the module version element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module_version
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] moduleversion [AjPStr] Module version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetModuleversion(EnsPAnalysis analysis,
                                   AjPStr moduleversion)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->ModuleVersion);

    analysis->ModuleVersion = ajStrNewRef(moduleversion);

    return ajTrue;
}




/* @func ensAnalysisSetName ***************************************************
**
** Set the name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::logic_name
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetName(EnsPAnalysis analysis, AjPStr name)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->Name);

    analysis->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensAnalysisSetParameters *********************************************
**
** Set the parameters element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::parameters
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] parameters [AjPStr] Parameters
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetParameters(EnsPAnalysis analysis, AjPStr parameters)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->Parameters);

    analysis->Parameters = ajStrNewRef(parameters);

    return ajTrue;
}




/* @func ensAnalysisSetProgramfile ********************************************
**
** Set the program file element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_file
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] programfile [AjPStr] Program file
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetProgramfile(EnsPAnalysis analysis, AjPStr programfile)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->ProgramFile);

    analysis->ProgramFile = ajStrNewRef(programfile);

    return ajTrue;
}




/* @func ensAnalysisSetProgramname ********************************************
**
** Set the program name element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] programname [AjPStr] Program name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetProgramname(EnsPAnalysis analysis, AjPStr programname)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->ProgramName);

    analysis->ProgramName = ajStrNewRef(programname);

    return ajTrue;
}




/* @func ensAnalysisSetProgramversion *****************************************
**
** Set the program version element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_version
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] programversion [AjPStr] Program version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetProgramversion(EnsPAnalysis analysis,
                                    AjPStr programversion)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->ProgramVersion);

    analysis->ProgramVersion = ajStrNewRef(programversion);

    return ajTrue;
}




/* @func ensAnalysisSetGfffeature *********************************************
**
** Set the GFF feature element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_feature
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] gfffeature [AjPStr] GFF feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetGfffeature(EnsPAnalysis analysis, AjPStr gfffeature)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->GFFFeature);

    analysis->GFFFeature = ajStrNewRef(gfffeature);

    return ajTrue;
}




/* @func ensAnalysisSetGffsource **********************************************
**
** Set the GFF source element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_source
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] gffsource [AjPStr] GFF source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetGffsource(EnsPAnalysis analysis, AjPStr gffsource)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->GFFSource);

    analysis->GFFSource = ajStrNewRef(gffsource);

    return ajTrue;
}




/* @func ensAnalysisSetWebdata ************************************************
**
** Set the web data element of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::web_data
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] webdata [AjPStr] Web data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisSetWebdata(EnsPAnalysis analysis, AjPStr webdata)
{
    if(!analysis)
        return ajFalse;

    ajStrDel(&analysis->WebData);

    analysis->WebData = ajStrNewRef(webdata);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
** @nam3rule Trace Report Ensembl Analysis elements to debug file.
**
** @argrule Trace analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAnalysisTrace *****************************************************
**
** Trace an Ensembl Analysis.
**
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisTrace(const EnsPAnalysis analysis, ajuint level)
{
    AjPStr indent = NULL;

    if(!analysis)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensAnalysisTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  CreationDate '%S'\n"
            "%S  Name '%S'\n"
            "%S  DatabaseName '%S'\n"
            "%S  DatabaseVersion '%S'\n"
            "%S  DatabaseFile '%S'\n"
            "%S  ProgramName '%S'\n"
            "%S  ProgramVersion '%S'\n"
            "%S  ProgramFile '%S'\n"
            "%S  Parameters '%S'\n"
            "%S  ModuleName '%S'\n"
            "%S  ModuleVersion '%S'\n"
            "%S  GFFSource '%S'\n"
            "%S  GFFFeature '%S'\n"
            "%S  Description %p\n"
            "%S  DisplayLabel '%S'\n"
            "%S  Displayable '%B'\n"
            "%S  WebData %p\n",
            indent, analysis,
            indent, analysis->Use,
            indent, analysis->Identifier,
            indent, analysis->Adaptor,
            indent, analysis->CreationDate,
            indent, analysis->Name,
            indent, analysis->DatabaseName,
            indent, analysis->DatabaseVersion,
            indent, analysis->DatabaseFile,
            indent, analysis->ProgramName,
            indent, analysis->ProgramVersion,
            indent, analysis->ProgramFile,
            indent, analysis->Parameters,
            indent, analysis->ModuleName,
            indent, analysis->ModuleVersion,
            indent, analysis->GFFSource,
            indent, analysis->GFFFeature,
            indent, analysis->Description,
            indent, analysis->DisplayLabel,
            indent, analysis->Displayable,
            indent, analysis->WebData);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section testing properties ************************************************
**
** @fdata [EnsPAnalysis]
** @fnote None
**
** @nam3rule Is Test Analysis property
** @nam4rule IsDatabase Analysis is based on a database
**
** @suffix
**
** @argrule Is analysis [const EnsPAnalysis] Ensembl Analysis
**
** @valrule * [AjBool] Analysis boolean property
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisIsDatabase ************************************************
**
** Test whether an Ensembl Analysis is based on a database.
**
** @cc Bio::EnsEMBL::Analysis::has_database
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue if the Ensembl Analysis was defined and has a
**                  database attached.
** @@
******************************************************************************/

AjBool ensAnalysisIsDatabase(const EnsPAnalysis analysis)
{
    if(!analysis)
        return ajFalse;

    if(analysis->DatabaseName && ajStrGetLen(analysis->DatabaseName))
        return ajTrue;
    else
        return ajFalse;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Analyses
**
** @fdata [EnsPAnalysis]
**
** @nam3rule  Match Compare two Ensembl Analyses.
**
** @argrule * analysis1 [const EnsPAnalysis] Ensembl Analysis
** @argrule * analysis2 [const EnsPAnalysis] Ensembl Analysis
**
** @valrule * [AjBool] True on success
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisMatch *****************************************************
**
** Test for matching two Ensembl Analyses.
**
** @cc Bio::EnsEMBL::Analysis::compare
** @param [r] analysis1 [const EnsPAnalysis] First Ensembl Analysis
** @param [r] analysis2 [const EnsPAnalysis] Second Ensembl Analysis
**
** @return [AjBool] ajTrue if the Ensembl Analyses are equal
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name and version elements
** is performed.
******************************************************************************/

AjBool ensAnalysisMatch(const EnsPAnalysis analysis1,
                        const EnsPAnalysis analysis2)
{
    if(ajDebugTest("ensAnalysisMatch"))
        ajDebug("ensAnalysisMatch\n"
                "  analysis1 %p\n"
                "  analysis2 %p\n",
                analysis1,
                analysis2);

    if(!analysis1)
        return ajFalse;

    if(!analysis2)
        return ajFalse;

    if(analysis1 == analysis2)
        return ajTrue;

    if(!ajStrMatchCaseS(analysis1->Name, analysis2->Name))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->DatabaseName, analysis2->DatabaseName))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->DatabaseVersion,
                        analysis2->DatabaseVersion))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->DatabaseFile, analysis2->DatabaseFile))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->ProgramName, analysis2->ProgramName))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->ProgramVersion,
                        analysis2->ProgramVersion))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->ProgramFile, analysis2->ProgramFile))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->Parameters, analysis2->Parameters))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->ModuleName, analysis2->ModuleName))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->ModuleVersion, analysis2->ModuleVersion))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->GFFSource, analysis2->GFFSource))
        return ajFalse;

    if(!ajStrMatchCaseS(analysis1->GFFFeature, analysis2->GFFFeature))
        return ajFalse;

    return ajTrue;
}




/* @datasection [EnsPAnalysisadaptor] Analysis Adaptor ************************
**
** Functions for manipulating Ensembl Analysis Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor CVS Revision: 1.66
**
** @nam2rule Analysisadaptor
**
******************************************************************************/

static const char *analysisadaptorTables[] =
{
    "analysis",
    "analysis_description",
    NULL
};

static const char *analysisadaptorColumns[] =
{
    "analysis.analysis_id",
    "analysis.created",
    "analysis.logic_name",
    "analysis.db",
    "analysis.db_version",
    "analysis.db_file",
    "analysis.program",
    "analysis.program_version",
    "analysis.program_file",
    "analysis.parameters",
    "analysis.module",
    "analysis.module_version",
    "analysis.gff_source",
    "analysis.gff_feature",
    "analysis_description.description",
    "analysis_description.display_label",
    "analysis_description.displayable",
    "analysis_description.web_data",
    NULL
};

static EnsOBaseadaptorLeftJoin analysisadaptorLeftJoin[] =
{
    {
        "analysis_description",
        "analysis.analysis_id = analysis_description.analysis_id"
    },
    {NULL, NULL}
};

static const char *analysisadaptorDefaultCondition = NULL;

static const char *analysisadaptorFinalCondition = NULL;




/* @funcstatic analysisadaptorFetchAllBySQL ***********************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Analysis objects.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::_objFromHashref
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] analyses [AjPList] AJAX List of Ensembl Analyses
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool analysisadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                           const AjPStr statement,
                                           EnsPAssemblymapper am,
                                           EnsPSlice slice,
                                           AjPList analyses)
{
    ajuint identifier = 0;

    AjBool displayable = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr cdate           = NULL;
    AjPStr name            = NULL;
    AjPStr databasename    = NULL;
    AjPStr databaseversion = NULL;
    AjPStr databasefile    = NULL;
    AjPStr programname     = NULL;
    AjPStr programversion  = NULL;
    AjPStr programfile     = NULL;
    AjPStr parameters      = NULL;
    AjPStr modulename      = NULL;
    AjPStr moduleversion   = NULL;
    AjPStr gffsource       = NULL;
    AjPStr gfffeature      = NULL;
    AjPStr description     = NULL;
    AjPStr displaylabel    = NULL;
    AjPStr webdata         = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    if(ajDebugTest("analysisadaptorFetchAllBySQL"))
        ajDebug("analysisadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  analyses %p\n",
                dba,
                statement,
                am,
                slice,
                analyses);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!analyses)
        return ajFalse;

    aa = ensRegistryGetAnalysisadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier      = 0;
        cdate           = ajStrNew();
        name            = ajStrNew();
        databasename    = ajStrNew();
        databaseversion = ajStrNew();
        databasefile    = ajStrNew();
        programname     = ajStrNew();
        programversion  = ajStrNew();
        programfile     = ajStrNew();
        parameters      = ajStrNew();
        modulename      = ajStrNew();
        moduleversion   = ajStrNew();
        gffsource       = ajStrNew();
        gfffeature      = ajStrNew();
        description     = ajStrNew();
        displaylabel    = ajStrNew();
        displayable     = ajFalse;
        webdata         = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &databasename);
        ajSqlcolumnToStr(sqlr, &databaseversion);
        ajSqlcolumnToStr(sqlr, &databasefile);
        ajSqlcolumnToStr(sqlr, &programname);
        ajSqlcolumnToStr(sqlr, &programversion);
        ajSqlcolumnToStr(sqlr, &programfile);
        ajSqlcolumnToStr(sqlr, &parameters);
        ajSqlcolumnToStr(sqlr, &modulename);
        ajSqlcolumnToStr(sqlr, &moduleversion);
        ajSqlcolumnToStr(sqlr, &gffsource);
        ajSqlcolumnToStr(sqlr, &gfffeature);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &displaylabel);
        ajSqlcolumnToBool(sqlr, &displayable);
        ajSqlcolumnToStr(sqlr, &webdata);

        analysis = ensAnalysisNewData(aa,
                                      identifier,
                                      cdate,
                                      name,
                                      databasename,
                                      databaseversion,
                                      databasefile,
                                      programname,
                                      programversion,
                                      programfile,
                                      parameters,
                                      modulename,
                                      moduleversion,
                                      gffsource,
                                      gfffeature,
                                      description,
                                      displaylabel,
                                      displayable,
                                      webdata);

        ajListPushAppend(analyses, (void *) analysis);

        ajStrDel(&cdate);
        ajStrDel(&name);
        ajStrDel(&databasename);
        ajStrDel(&databaseversion);
        ajStrDel(&databasefile);
        ajStrDel(&programname);
        ajStrDel(&programversion);
        ajStrDel(&programfile);
        ajStrDel(&parameters);
        ajStrDel(&modulename);
        ajStrDel(&moduleversion);
        ajStrDel(&gffsource);
        ajStrDel(&gfffeature);
        ajStrDel(&description);
        ajStrDel(&displaylabel);
        ajStrDel(&webdata);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @funcstatic analysisadaptorCacheInsert *************************************
**
** Insert an Ensembl Analysis into the Analysis Adaptor-internal cache.
** If an Analysis with the same name element is already present in the
** adaptor cache, the Analysis is deleted and a pointer to the cached Analysis
** is returned.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [u] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool analysisadaptorCacheInsert(EnsPAnalysisadaptor aa,
                                         EnsPAnalysis *Panalysis)
{
    ajuint *Pidentifier = NULL;

    EnsPAnalysis analysis1 = NULL;
    EnsPAnalysis analysis2 = NULL;

    if(!aa)
        return ajFalse;

    if(!aa->CacheByIdentifier)
        return ajFalse;

    if(!aa->CacheByName)
        return ajFalse;

    if(!Panalysis)
        return ajFalse;

    if(!*Panalysis)
        return ajFalse;

    /* Search the identifer cache. */

    analysis1 = (EnsPAnalysis) ajTableFetch(
        aa->CacheByIdentifier,
        (const void *) &((*Panalysis)->Identifier));

    /* Search the name cache. */

    analysis2 = (EnsPAnalysis) ajTableFetch(
        aa->CacheByName,
        (const void *) (*Panalysis)->Name);

    if((!analysis1) && (!analysis2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Panalysis)->Identifier;

        ajTablePut(aa->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensAnalysisNewRef(*Panalysis));

        /* Insert into the name cache. */

        ajTablePut(aa->CacheByName,
                   (void *) ajStrNewS((*Panalysis)->Name),
                   (void *) ensAnalysisNewRef(*Panalysis));
    }

    if(analysis1 && analysis2 && (analysis1 == analysis2))
    {
        ajDebug("analysisadaptorCacheInsert replaced Analysis %p with "
                "one already cached %p.\n",
                *Panalysis, analysis1);

        ensAnalysisDel(Panalysis);

        ensAnalysisNewRef(analysis1);

        Panalysis = &analysis1;
    }

    if(analysis1 && analysis2 && (analysis1 != analysis2))
        ajDebug("analysisadaptorCacheInsert detected Analyses in the "
                "identifier and name cache with identical names "
                "('%S' and '%S') but different addresses (%p and %p).\n",
                analysis1->Name, analysis2->Name, analysis1, analysis2);

    if(analysis1 && (!analysis2))
        ajDebug("analysisadaptorCacheInsert detected an Ensembl Analysis "
                "in the identifier, but not in the name cache.\n");

    if((!analysis1) && analysis2)
        ajDebug("analysisadaptorCacheInsert detected an Ensembl Analysis "
                "in the name, but not in the identifier cache.\n");

    return ajTrue;
}




#if AJFALSE
/* @funcstatic analysisadaptorCacheRemove *************************************
**
** Remove an Ensembl Analysis from the Analysis Adaptor-internal cache.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool analysisadaptorCacheRemove(EnsPAnalysisadaptor aa,
                                         EnsPAnalysis analysis)
{
    ajuint *Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPAnalysis analysis1 = NULL;
    EnsPAnalysis analysis2 = NULL;

    if(!aa)
        return ajFalse;

    if(!analysis)
        return ajFalse;

    /* Remove the table nodes. */

    analysis1 = (EnsPAnalysis)
        ajTableRemoveKey(aa->CacheByIdentifier,
                         (const void *) &analysis->Identifier,
                         (void **) &Pidentifier);

    analysis2 = (EnsPAnalysis)
        ajTableRemoveKey(aa->CacheByName,
                         (const void *) analysis->Name,
                         (void **) &key);

    if(analysis1 && (!analysis2))
        ajWarn("analysisadaptorCacheRemove could remove Analysis with "
               "identifier %u and name '%S' only from the identifier cache.\n",
               analysis->Identifier,
               analysis->Name);

    if((!analysis1) && analysis2)
        ajWarn("analysisadaptorCacheRemove could remove Analysis with "
               "identifier %u and name '%S' only from the name cache.\n",
               analysis->Identifier,
               analysis->Name);

    /* Delete the keys. */

    AJFREE(Pidentifier);

    ajStrDel(&key);

    /* Delete (or at least de-reference) the Ensembl Analyses. */

    ensAnalysisDel(&analysis1);
    ensAnalysisDel(&analysis2);

    return ajTrue;
}

#endif




/* @funcstatic analysisadaptorCacheInit ***************************************
**
** Initialise the internal Analysis cache of an Ensembl Analysis Adaptor.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool analysisadaptorCacheInit(EnsPAnalysisadaptor aa)
{
    AjPList analyses = NULL;

    EnsPAnalysis analysis = NULL;

    if(ajDebugTest("analysisadaptorCacheInit"))
        ajDebug("analysisadaptorCacheInit\n"
                "  aa %p\n",
                aa);

    if(!aa)
        return ajFalse;

    if(aa->CacheByIdentifier)
        return ajFalse;
    else
        aa->CacheByIdentifier =
            ajTableNewFunctionLen(0, ensTableCmpUint, ensTableHashUint);

    if(aa->CacheByName)
        return ajFalse;
    else
        aa->CacheByName = ajTablestrNewCaseLen(0);

    analyses = ajListNew();

    ensBaseadaptorGenericFetch(aa->Adaptor,
                               (const AjPStr) NULL,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               analyses);

    while(ajListPop(analyses, (void **) &analysis))
    {
        analysisadaptorCacheInsert(aa, &analysis);

        /* Both caches hold internal references to the Analysis objects. */

        ensAnalysisDel(&analysis);
    }

    ajListFree(&analyses);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Analysis Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Analysis Adaptor. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPAnalysisadaptor]
** @fnote None
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @fcategory new
******************************************************************************/




/* @func ensAnalysisadaptorNew ************************************************
**
** Default constructor for an Ensembl Analysis Adaptor.
**
** Ensembl Object Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Object Adaptor connected to a particular database is
** sufficient to instantiate any number of Ensembl Objects from the database.
** Each Ensembl Object will have a weak reference to the Object Adaptor that
** instantiated it. Therefore, Ensembl Object Adaptors should not be
** instantiated directly, but rather obtained from the Ensembl Registry,
** which will in turn call this function if neccessary.
**
** @see ensRegistryGetDatabaseadaptor
** @see ensRegistryGetAnalysisadaptor
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAnalysisadaptor] Ensembl Analyis Adaptor or NULL
** @@
******************************************************************************/

EnsPAnalysisadaptor ensAnalysisadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAnalysisadaptor aa = NULL;

    if(!dba)
        return NULL;

    if(ajDebugTest("ensAnalysisadaptorNew"))
        ajDebug("ensAnalysisadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(aa);

    aa->Adaptor = ensBaseadaptorNew(
        dba,
        analysisadaptorTables,
        analysisadaptorColumns,
        analysisadaptorLeftJoin,
        analysisadaptorDefaultCondition,
        analysisadaptorFinalCondition,
        analysisadaptorFetchAllBySQL);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** analysisadaptorCacheInit function calls ensBaseadaptorGenericFetch,
    ** which calls analysisadaptorFetchAllBySQL, which calls
    ** ensRegistryGetAnalysisadaptor. At that point, however, the Analysis
    ** Adaptor has not been stored in the Registry. Therefore, each
    ** ensAnalysisadaptorFetch function has to test the presence of the
    ** adaptor-internal cache and eventually initialise before accessing it.
    **
    **  analysisadaptorCacheInit(aa);
    */

    return aa;
}




/* @funcstatic analysisadaptorCacheClearIdentifier ****************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Analysis
** Adaptor-internal Analysis cache. This function deletes the unsigned integer
** identifier key and the Ensembl Analysis value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Analysis value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void analysisadaptorCacheClearIdentifier(void **key,
                                                void **value,
                                                void *cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);

    ensAnalysisDel((EnsPAnalysis *) value);

    return;
}




/* @funcstatic analysisadaptorCacheClearName **********************************
**
** An ajTableMapDel 'apply' function to clear the Ensembl Analysis
** Adaptor-internal Analysis cache. This function deletes the
** AJAX String key data and the Ensembl Analysis value data.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] Ensembl Analysis value data address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void analysisadaptorCacheClearName(void **key,
                                          void **value,
                                          void *cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    ajStrDel((AjPStr *) key);

    ensAnalysisDel((EnsPAnalysis *) value);

    return;
}




/* @funcstatic analysisadaptorCacheExit ***************************************
**
** Clears the internal Analysis cache of an Ensembl Analysis Adaptor.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool analysisadaptorCacheExit(EnsPAnalysisadaptor aa)
{
    if(!aa)
        return ajFalse;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(aa->CacheByIdentifier,
                  analysisadaptorCacheClearIdentifier,
                  NULL);

    ajTableFree(&aa->CacheByIdentifier);

    /* Clear and delete the name cache. */

    ajTableMapDel(aa->CacheByName,
                  analysisadaptorCacheClearName,
                  NULL);

    ajTableFree(&aa->CacheByName);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Analysis Adaptor.
**
** @fdata [EnsPAnalysisadaptor]
** @fnote None
**
** @nam3rule Del Destroy (free) an Ensembl Analysis Adaptor object.
**
** @argrule * Paa [EnsPAnalysisadaptor*] Ensembl Analysis Adaptor
**                                       object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAnalysisadaptorDel *********************************************
**
** Default destructor for an Ensembl Analysis Adaptor.
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Paa [EnsPAnalysisadaptor*] Ensembl Analysis Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensAnalysisadaptorDel(EnsPAnalysisadaptor* Paa)
{
    EnsPAnalysisadaptor pthis = NULL;

    if(!Paa)
        return;

    if(!*Paa)
        return;

    pthis = *Paa;

    analysisadaptorCacheExit(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Paa = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Analysis Adaptor object.
**
** @fdata [EnsPAnalysisadaptor]
** @fnote None
**
** @nam3rule Get Return Ensembl Analysis Adaptor attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Base Adaptor
**
** @argrule * aa [const EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @valrule Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisadaptorGetBaseadaptor *************************************
**
** Get the Ensembl Base Adaptor element of an Ensembl Analysis Adaptor.
**
** @param [r] aa [const EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensAnalysisadaptorGetBaseadaptor(
    const EnsPAnalysisadaptor aa)
{
    if(!aa)
        return NULL;

    return aa->Adaptor;
}




/* @func ensAnalysisadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor element of an Ensembl Analysis Adaptor.
**
** @param [r] aa [const EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAnalysisadaptorGetDatabaseadaptor(
    const EnsPAnalysisadaptor aa)
{
    if(!aa)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(aa->Adaptor);
}




/* @section object retrieval **************************************************
**
** Functions for retrieving Ensembl Analysis objects from an
** Ensembl Core database.
**
** @fdata [EnsPAnalysisadaptor]
** @fnote None
**
** @nam3rule Fetch Retrieve Ensembl Analysis object(s)
** @nam4rule FetchAll Retrieve all Ensembl Analysis objects
** @nam5rule FetchAllBy Retrieve all Ensembl Analysis objects
**                      matching a criterion
** @nam4rule FetchBy Retrieve one Ensembl Analysis object
**                   matching a criterion
**
** @argrule * aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @argrule FetchAll [AjPList] AJAX List of Ensembl Analysis objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic analysisadaptorFetchAll ****************************************
**
** An ajTableMap 'apply' function to return all Analysis objects from the
** Ensembl Analysis Adaptor-internal cache.
**
** @param [u] key [const void *] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Analysis value data address
** @param [u] cl [void*] AJAX List of Ensembl Analysis objects,
**                       passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
** @@
******************************************************************************/

static void analysisadaptorFetchAll(const void *key, void **value, void *cl)
{
    if(!key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    if(!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void *) ensAnalysisNewRef(*((EnsPAnalysis *) value)));

    return;
}




/* @func ensAnalysisadaptorFetchAll *******************************************
**
** Fetch all Ensembl Analyses.
**
** The caller is responsible for deleting the Ensembl Analyses before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_all
** @param [r] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [u] analyses [AjPList] AJAX List of Ensembl Analyses
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchAll(EnsPAnalysisadaptor aa,
                                  AjPList analyses)
{
    if(!aa)
        return ajFalse;

    if(!analyses)
        return ajFalse;

    if(!aa->CacheByIdentifier)
        analysisadaptorCacheInit(aa);

    ajTableMap(aa->CacheByIdentifier,
               analysisadaptorFetchAll,
               (void *) analyses);

    return ajTrue;
}




/* @func ensAnalysisadaptorFetchByIdentifier **********************************
**
** Fetch an Ensembl Analysis by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Analysis.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_by_dbID
** @param [r] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchByIdentifier(EnsPAnalysisadaptor aa,
                                           ajuint identifier,
                                           EnsPAnalysis *Panalysis)
{
    AjPList analyses = NULL;

    AjPStr constraint = NULL;

    EnsPAnalysis analysis = NULL;

    if(!aa)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Panalysis)
        return ajFalse;

    /*
    ** Initally, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!aa->CacheByIdentifier)
        analysisadaptorCacheInit(aa);

    *Panalysis = (EnsPAnalysis)
        ajTableFetch(aa->CacheByIdentifier, (const void *) &identifier);

    if(*Panalysis)
    {
        ensAnalysisNewRef(*Panalysis);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    constraint = ajFmtStr("analysis.analysis_id = %u", identifier);

    analyses = ajListNew();

    ensBaseadaptorGenericFetch(aa->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               analyses);

    if(ajListGetLength(analyses) > 1)
        ajWarn("ensAnalysisadaptorFetchByIdentifier got more than one "
               "Ensembl Analysis for (PRIMARY KEY) identifier %u.\n",
               identifier);

    ajListPop(analyses, (void **) Panalysis);

    analysisadaptorCacheInsert(aa, Panalysis);

    while(ajListPop(analyses, (void **) &analysis))
    {
        analysisadaptorCacheInsert(aa, &analysis);

        ensAnalysisDel(&analysis);
    }

    ajListFree(&analyses);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensAnalysisadaptorFetchByName ****************************************
**
** Fetch an Ensembl Analysis by its name.
** The caller is responsible for deleting the Ensembl Analysis.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_by_logic_name
** @param [r] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] name [const AjPStr] Ensembl Analysis name
** @param [wP] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchByName(EnsPAnalysisadaptor aa,
                                     const AjPStr name,
                                     EnsPAnalysis *Panalysis)
{
    char *txtname = NULL;

    AjPList analyses = NULL;

    AjPStr constraint = NULL;

    EnsPAnalysis analysis = NULL;

    if(!aa)
        return ajFalse;

    if((!name) && (!ajStrGetLen(name)))
        return ajFalse;

    if(!Panalysis)
        return ajFalse;

    /*
    ** Initally, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if(!aa->CacheByName)
        analysisadaptorCacheInit(aa);

    *Panalysis = (EnsPAnalysis)
        ajTableFetch(aa->CacheByName, (const void *) name);

    if(*Panalysis)
    {
        ensAnalysisNewRef(*Panalysis);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ensBaseadaptorEscapeC(aa->Adaptor, &txtname, name);

    constraint = ajFmtStr("analysis.logic_name = '%s'", txtname);

    ajCharDel(&txtname);

    analyses = ajListNew();

    ensBaseadaptorGenericFetch(aa->Adaptor,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               analyses);

    if(ajListGetLength(analyses) > 1)
        ajWarn("ensAnalysisadaptorFetchByName got more than one "
               "Ensembl Analysis for (UNIQUE) name '%S'.\n",
               name);

    ajListPop(analyses, (void **) Panalysis);

    analysisadaptorCacheInsert(aa, Panalysis);

    while(ajListPop(analyses, (void **) &analysis))
    {
        analysisadaptorCacheInsert(aa, &analysis);

        ensAnalysisDel(&analysis);
    }

    ajListFree(&analyses);

    ajStrDel(&constraint);

    return ajTrue;
}




static const char* analysisadaptorFeatureClasses[] =
{
    "AffyFeature", "affy_feature",
    "Densityfeature", "density_type", /* density_type.analysis_id */
    "DnaAlignFeature", "dna_align_feature",
    "Gene", "gene",
    "Markerfeature", "marker_feature",
    "Predictiontranscript", "prediction_transcript",
    "OligoFeature", "oligo_feature",
    "Proteinalignfeature", "protein_align_feature",
    "Proteinfeature", "protein_feature",
    "QtlFeature", "qtl_feature",
    "Repeatfeature", "repeat_feature",
    "Simplefeature", "simple_feature",
    NULL, NULL
};




/* @func ensAnalysisadaptorFetchAllByFeatureclass *****************************
**
** Fetch all Ensembl Analyses referenced by an Ensembl Feature Class.
**
** Please see the analysisadaptorFeatureClasses array for a list of valid
** Feature classes that reference Analyses.
**
** The caller is responsible for deleting the Ensembl Analyses before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_all_by_feature_class
** @param [r] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] class [const AjPStr] Ensembl Feature class
** @param [u] analyses [AjPList] AJAX List of Ensembl Analyses
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchAllByFeatureclass(EnsPAnalysisadaptor aa,
                                                const AjPStr class,
                                                AjPList analyses)
{
    register ajuint i = 0;
    ajuint identifier = 0;
    ajuint match      = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!aa)
        return ajFalse;

    if((!class) && (!ajStrGetLen(class)))
        return ajFalse;

    if(!analyses)
        return ajFalse;

    for(i = 0; analysisadaptorFeatureClasses[i]; i += 2)
        if(ajStrMatchC(class, analysisadaptorFeatureClasses[i]))
            match = i + 1;

    if(match)
    {
        dba = ensBaseadaptorGetDatabaseadaptor(aa->Adaptor);

        statement =
            ajFmtStr("SELECT DISTINCT %s.analysis_id FROM %s",
                     analysisadaptorFeatureClasses[match],
                     analysisadaptorFeatureClasses[match]);

        sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

        sqli = ajSqlrowiterNew(sqls);

        while(!ajSqlrowiterDone(sqli))
        {
            identifier = 0;

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToUint(sqlr, &identifier);

            ensAnalysisadaptorFetchByIdentifier(aa,
                                                identifier,
                                                &analysis);

            if(analysis)
                ajListPushAppend(analyses, (void *) analysis);
            else
                ajWarn("ensAnalysisadaptorFetchAllByFeatureclass found "
                       "Ensembl Analysis identifier %u in the '%s' table, "
                       "which is not referenced in the 'analysis' table.\n",
                       identifier,
                       analysisadaptorFeatureClasses[match]);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(dba, &sqls);

        ajStrDel(&statement);
    }
    else
    {
        ajDebug("ensAnalysisadaptorFetchAllByFeatureclass got invalid "
                "Feature class '%S'\n",
                class);

        return ajFalse;
    }

    return ajTrue;
}
