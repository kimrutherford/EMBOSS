/******************************************************************************
** @source Ensembl Genetic Variation Sample functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.1 $
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

#include "ensgvsample.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

static const char *gvSampleDisplay[] =
{
    "REFERENCE",
    "DEFAULT",
    "DISPLAYABLE",
    "UNDISPLAYABLE",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

extern EnsPGvsampleadaptor
ensRegistryGetGvsampleadaptor(EnsPDatabaseadaptor dba);

static AjBool gvSampleadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                           const AjPStr statement,
                                           EnsPAssemblymapper am,
                                           EnsPSlice slice,
                                           AjPList gvss);




/* @filesection ensgvsample ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvsample] Genetic Variation Sample ***********************
**
** Functions for manipulating Ensembl Genetic Variation Sample objects
**
** @cc Bio::EnsEMBL::Variation::Sample CVS Revision: 1.2
**
** @nam2rule Gvsample
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Sample by pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Sample. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvsample]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Ref object [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @valrule * [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @fcategory new
******************************************************************************/




/* @func ensGvsampleNew *******************************************************
**
** Default constructor for an Ensembl Genetic Variation Sample.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [r] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] name [AjPStr] Name
** @param [u] description [AjPStr] Description
** @param [r] display [AjEnum] Display
** @param [r] size [ajuint] Size
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
** @@
******************************************************************************/

EnsPGvsample ensGvsampleNew(EnsPGvsampleadaptor gvsa,
                            ajuint identifier,
                            AjPStr name,
                            AjPStr description,
                            AjEnum display,
                            ajuint size)
{
    EnsPGvsample gvs = NULL;

    AJNEW0(gvs);

    gvs->Use = 1;

    gvs->Identifier = identifier;

    gvs->Adaptor = gvsa;

    if(name)
        gvs->Name = ajStrNewRef(name);

    if(description)
        gvs->Description = ajStrNewRef(description);

    gvs->Display = display;

    gvs->Size = size;

    return gvs;
}




/* @func ensGvsampleNewObj ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
** @@
******************************************************************************/

EnsPGvsample ensGvsampleNewObj(const EnsPGvsample object)
{
    EnsPGvsample gvs = NULL;

    if(!object)
        return NULL;

    AJNEW0(gvs);

    gvs->Use = 1;

    gvs->Identifier = object->Identifier;

    gvs->Adaptor = object->Adaptor;

    if(object->Name)
        gvs->Name = ajStrNewRef(object->Name);

    if(object->Description)
        gvs->Description = ajStrNewRef(object->Description);

    gvs->Display = object->Display;

    gvs->Size = object->Size;

    return gvs;
}




/* @func ensGvsampleNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample
** @@
******************************************************************************/

EnsPGvsample ensGvsampleNewRef(EnsPGvsample gvs)
{
    if(!gvs)
        return NULL;

    gvs->Use++;

    return gvs;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Genetic Variation Sample.
**
** @fdata [EnsPGvsample]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Sample object
**
** @argrule * Pgvs [EnsPGvsample*] Genetic Variation Sample object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvsampleDel *******************************************************
**
** Default destructor for an Ensembl Genetic Variation Sample.
**
** @param [d] Pgvs [EnsPGvsample*] Ensembl Genetic Variation Sample address
**
** @return [void]
** @@
******************************************************************************/

void ensGvsampleDel(EnsPGvsample *Pgvs)
{
    EnsPGvsample pthis = NULL;

    if(!Pgvs)
        return;

    if(!*Pgvs)
        return;

    if(ajDebugTest("ensGvsampleDel"))
    {
        ajDebug("ensGvsampleDel\n"
                "  *Psample %p\n",
                *Pgvs);

        ensGvsampleTrace(*Pgvs, 1);
    }

    pthis = *Pgvs;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvs = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pgvs = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Sample attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Sample Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetName Return the name
** @nam4rule GetDescription Return the description
** @nam4rule GetDisplay Return the display
** @nam4rule GetSize Return the size
**
** @argrule * gvs [const EnsPGvsample] Genetic Variation Sample
**
** @valrule Adaptor [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                        Sample Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Name [AjPStr] Name
** @valrule Description [AjPStr] Description
** @valrule Display [AjEnum] Display
** @valrule Size [ajuint] Size
**
** @fcategory use
******************************************************************************/




/* @func ensGvsampleGetAdaptor ************************************************
**
** Get the Ensembl Genetic Variation Sample Adaptor element of an
** Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [EnsPGvsampleadaptor] Ensembl Genetic Variation Sample Adaptor
** @@
******************************************************************************/

EnsPGvsampleadaptor ensGvsampleGetAdaptor(const EnsPGvsample gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Adaptor;
}




/* @func ensGvsampleGetIdentifier *********************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvsampleGetIdentifier(const EnsPGvsample gvs)
{
    if(!gvs)
        return 0;

    return gvs->Identifier;
}




/* @func ensGvsampleGetName ***************************************************
**
** Get the name element of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjPStr] Name
** @@
******************************************************************************/

AjPStr ensGvsampleGetName(const EnsPGvsample gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Name;
}




/* @func ensGvsampleGetDescription ********************************************
**
** Get the description element of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensGvsampleGetDescription(const EnsPGvsample gvs)
{
    if(!gvs)
        return NULL;

    return gvs->Description;
}




/* @func ensGvsampleGetDisplay ************************************************
**
** Get the display element of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjEnum] Display
** @@
******************************************************************************/

AjEnum
ensGvsampleGetDisplay(const EnsPGvsample gvs)
{
    if(!gvs)
        return ensEGvsampleDisplayNULL;

    return gvs->Display;
}




/* @func ensGvsampleGetSize ***************************************************
**
** Get the size element of an Ensembl Genetic Variation Sample.
**
** @param  [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [ajuint] Size
** @@
******************************************************************************/

ajuint ensGvsampleGetSize(const EnsPGvsample gvs)
{
    if(!gvs)
        return 0;

    return gvs->Size;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Sample
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation Sample Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetDescription Set the description
** @nam4rule SetDisplay Set the display
** @nam4rule SetSize Set the size
**
** @argrule * gvs [EnsPGvsample] Ensembl Genetic Variation Sample object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvsampleSetAdaptor ************************************************
**
** Set the Ensembl Genetic Variation Sample Adaptor element of an
** Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleSetAdaptor(EnsPGvsample gvs, EnsPGvsampleadaptor gvsa)
{
    if(!gvs)
        return ajFalse;

    gvs->Adaptor = gvsa;

    return ajTrue;
}




/* @func ensGvsampleSetIdentifier *********************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleSetIdentifier(EnsPGvsample gvs, ajuint identifier)
{
    if(!gvs)
        return ajFalse;

    gvs->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvsampleSetName ***************************************************
**
** Set the name element of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleSetName(EnsPGvsample gvs, AjPStr name)
{
    if(!gvs)
        return ajFalse;

    ajStrDel(&gvs->Name);

    if(name)
        gvs->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensGvsampleSetDescription ********************************************
**
** Set the description element of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleSetDescription(EnsPGvsample gvs, AjPStr description)
{
    if(!gvs)
        return ajFalse;

    ajStrDel(&gvs->Description);

    if(description)
        gvs->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvsampleSetDisplay ************************************************
**
** Set the display element of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] display [AjEnum] Display
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool
ensGvsampleSetDisplay(EnsPGvsample gvs, AjEnum display)
{
    if(!gvs)
        return ajFalse;

    gvs->Display = display;

    return ajTrue;
}




/* @func ensGvsampleSetSize ***************************************************
**
** Set the size element of an Ensembl Genetic Variation Sample.
**
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] size [ajuint] Size
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleSetSize(EnsPGvsample gvs, ajuint size)
{
    if(!gvs)
        return ajFalse;

    gvs->Size = size;

    return ajTrue;
}




/* @func ensGvsampleGetMemSize ************************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [ajuint] Memory size
** @@
******************************************************************************/

ajuint ensGvsampleGetMemSize(const EnsPGvsample gvs)
{
    ajuint size = 0;

    if(!gvs)
        return 0;

    size += (ajuint) sizeof (EnsOGvsample);

    if(gvs->Name)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvs->Name);
    }

    if(gvs->Description)
    {
        size += (ajuint) sizeof (AjOStr);

        size += ajStrGetRes(gvs->Description);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Sample object.
**
** @fdata [EnsPGvsample]
** @nam3rule Trace Report Ensembl Genetic Variation Sample elements to
**                 debug file
**
** @argrule Trace gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvsampleTrace *****************************************************
**
** Trace an Ensembl Genetic Variation Sample.
**
** @param [r] gvs [const EnsPGvsample] Ensembl Genetic Variation Sample
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleTrace(const EnsPGvsample gvs, ajuint level)
{
    AjPStr indent = NULL;

    if(!gvs)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvsampleTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Description '%S'\n"
            "%S  Display '%s'\n"
            "%S  Size %u\n",
            indent, gvs,
            indent, gvs->Use,
            indent, gvs->Identifier,
            indent, gvs->Adaptor,
            indent, gvs->Name,
            indent, gvs->Description,
            indent, ensGvsampleDisplayToChar(gvs->Display),
            indent, gvs->Size);

    ajStrDel(&indent);

    return ajTrue;
}




/* @func ensGvsampleDisplayFromStr ********************************************
**
** Convert an AJAX String into an Ensembl Genetic Variaton Sample
** display element.
**
** @param [r] display [const AjPStr] Display string
**
** @return [AjEnum] Ensembl Genetic Variation Sample display element or
**                  ensEGvsampleDisplayNULL
** @@
******************************************************************************/

AjEnum
ensGvsampleDisplayFromStr(const AjPStr display)
{
    register ajint i = 0;

    AjEnum edisplay = ensEGvsampleDisplayNULL;

    for(i = 1; gvSampleDisplay[i]; i++)
        if(ajStrMatchC(display, gvSampleDisplay[i]))
            edisplay = i;

    if(!edisplay)
        ajDebug("ensGvsampleDisplayFromStr encountered "
                "unexpected string '%S'.\n", display);

    return edisplay;
}




/* @func ensGvsampleDisplayToChar *********************************************
**
** Convert an Ensembl Genetic Variation Sample display element into a
** C-type (char*) string.
**
** @param [r] display [const AjEnum] Ensembl Genetic Variation Sample
**                                   display enumerator
**
** @return [const char*] Ensembl Genetic Variation Sample display
**                       C-type (char*) string
** @@
******************************************************************************/

const char*
ensGvsampleDisplayToChar(const AjEnum display)
{
    register ajint i = 0;

    if(!display)
        return NULL;

    for(i = 1; gvSampleDisplay[i] && (i < display); i++);

    if(!gvSampleDisplay[i])
        ajDebug("ensGvsampleDisplayToChar encountered an "
                "out of boundary error on display %d.\n", display);

    return gvSampleDisplay[i];
}




/* @datasection [EnsPGvsampleadaptor] Genetic Variation Sample Adaptor ********
**
** Functions for manipulating Ensembl Genetic Variation Sample Adaptor
** objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor CVS Revision: 1.5
**
** @nam2rule Gvsampleadaptor
**
******************************************************************************/

static const char *gvSampleadaptorTables[] =
{
    "sample",
    NULL
};

static const char *gvSampleadaptorColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "sample.display",
    NULL
};

static EnsOBaseadaptorLeftJoin gvSampleadaptorLeftJoin[] =
{
    {NULL, NULL}
};

static const char *gvSampleadaptorDefaultCondition = NULL;

static const char *gvSampleadaptorFinalCondition = NULL;




/* @funcstatic gvSampleAdaptorFetchAllBySQL ***********************************
**
** Fetch all Ensembl Genetic Variation Sample objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Sample
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvSampleadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                           const AjPStr statement,
                                           EnsPAssemblymapper am,
                                           EnsPSlice slice,
                                           AjPList gvss)
{
    ajuint identifier = 0;
    ajuint size       = 0;

    AjEnum edisplay = ensEGvsampleDisplayNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name        = NULL;
    AjPStr description = NULL;
    AjPStr display     = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(ajDebugTest("gvSampleadaptorFetchAllBySQL"))
        ajDebug("gvSampleadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvss %p\n",
                dba,
                statement,
                am,
                slice,
                gvss);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvss)
        return ajFalse;

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier  = 0;
        name        = ajStrNew();
        size        = 0;
        description = ajStrNew();
        display     = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToUint(sqlr, &size);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &display);

        edisplay = ensGvsampleDisplayFromStr(display);

        gvs = ensGvsampleNew(gvsa,
                             identifier,
                             name,
                             description,
                             edisplay,
                             size);

        ajListPushAppend(gvss, (void *) gvs);

        ajStrDel(&name);
        ajStrDel(&description);
        ajStrDel(&display);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    return ajTrue;
}




/* @func ensGvsampleadaptorNew ************************************************
**
** Default constructor for an Ensembl Genetic Variation Sample Adaptor.
**
** @cc Bio::EnsEMBL::DBSQL::Baseadaptor::new
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvsampleadaptor] Ensembl Genetic Variation Sample Adaptor
**                               or NULL
** @@
******************************************************************************/

EnsPGvsampleadaptor ensGvsampleadaptorNew(EnsPDatabaseadaptor dba)
{
    return ensBaseadaptorNew(dba,
                             gvSampleadaptorTables,
                             gvSampleadaptorColumns,
                             gvSampleadaptorLeftJoin,
                             gvSampleadaptorDefaultCondition,
                             gvSampleadaptorFinalCondition,
                             gvSampleadaptorFetchAllBySQL);
}




/* @func ensGvsampleadaptorDel ************************************************
**
** Default destructor for an Ensembl Gentic Variation Sample Adaptor.
**
** @param [d] Pgvsa [EnsPGvsampleadaptor*] Ensembl Genetic Variation
**                                         Sample Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensGvsampleadaptorDel(EnsPGvsampleadaptor *Pgvsa)
{
    if(!Pgvsa)
        return;

    ensBaseadaptorDel(Pgvsa);

    return;
}




/* @func ensGvsampleadaptorGetAdaptor *****************************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Sample Adaptor.
**
** @param [u] gvsa [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensGvsampleadaptorGetAdaptor(EnsPGvsampleadaptor gvsa)
{
    if(!gvsa)
        return NULL;

    return gvsa;
}




/* @func ensGvsampleadaptorFetchAllByDisplay **********************************
**
** Fetch all Ensembl Genetic Variation Samples by display.
**
** @param [r] gvsa [const EnsPGvsampleAdaptor] Ensembl Genetic Variation
**                                             Sample Adaptor
** @param [r] display [AjEnum] Display
** @param [u] gvss [AjPList] AJAX List of Ensembl Genetic Variation Samples
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool
ensGvsampleadaptorFetchAllByDisplay(EnsPGvsampleadaptor gvsa,
                                    AjEnum display,
                                    AjPList gvss)
{
    AjPStr constraint = NULL;

    if(!gvsa)
        return ajFalse;

    if(!gvss)
        return ajFalse;

    constraint = ajFmtStr(
        "sample.display = '%s'",
        ensGvsampleDisplayToChar(display));

    ensBaseadaptorGenericFetch(gvsa,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               gvss);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvsampleadaptorFetchAllSynonymsByIdentifier ***********************
**
** Fetch all Ensembl Genetic Variation Sample synonyms for an
** Ensembl Genetic Variation Sample identifier.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor::fetch_synonyms
** @param [r] gvsa [const EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                             Sample Adaptor
** @param [r] identifier [ajuint] Ensembl Genetic Variation Sample identifier
** @param [r] source [const AjPStr] Source
** @param [u] synonyms [AjPList] AJAX List of synonym AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleadaptorFetchAllSynonymsByIdentifier(
    const EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    const AjPStr source,
    AjPList synonyms)
{
    char *txtsource = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvsa)
        return ajFalse;

    if(!synonyms)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvsa);

    if(!dba)
        return ajFalse;

    if(source)
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.name "
            "FROM "
            "sample_synonym, "
            "source "
            "WHERE "
            "sample_synonym.sample_id = %u "
            "AND "
            "sample_synonym.source_id = source.source_id "
            "AND "
            "source.name = '%s'",
            identifier,
            txtsource);

        ajCharDel(&txtsource);
    }
    else
        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.name "
            "FROM "
            "sample_synonym "
            "WHERE "
            "sample_synonym.sample_id = %u",
            identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        name = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToStr(sqlr, &name);

        ajListPushAppend(synonyms, (void *) ajStrNewRef(name));

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvsampleadaptorFetchAllIdentifiersBySynonym ***********************
**
** Fetch all Ensembl Genetic Variation Sample identifiers for an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor::fetch_sample_by_synonym
** @param [r] gvsa [const EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                             Sample Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [r] source [const AjPStr] Source
** @param [u] idlist [AjPList] AJAX List of Ensembl Genetic Variation
**                             Sample identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleadaptorFetchAllIdentifiersBySynonym(
    const EnsPGvsampleadaptor gvsa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList idlist)
{
    char *txtsynonym = NULL;
    char *txtsource  = NULL;

    ajuint *Pidentifier = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvsa)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvsa);

    if(!dba)
        return ajFalse;

    ensDatabaseadaptorEscapeC(dba, &txtsynonym, synonym);

    if(source)
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.sample_id "
            "FROM "
            "sample_synonym, "
            "source "
            "WHERE "
            "sample_synonym.name = '%s' "
            "AND "
            "sample_synonym.source_id = source.source_id "
            "AND "
            "source.name = '%s'",
            txtsynonym,
            txtsource);

        ajCharDel(&txtsource);
    }
    else
        statement = ajFmtStr(
            "SELECT "
            "sample_synonym.sample_id "
            "FROM "
            "sample_synonym "
            "WHERE "
            "sample_synonym.name = '%s'",
            txtsynonym);

    ajCharDel(&txtsynonym);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        AJNEW0(Pidentifier);

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, Pidentifier);

        ajListPushAppend(idlist, (void *) Pidentifier);
    }

    ajSqlrowiterDel(&sqli);

    ajSqlstatementDel(&sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvsampleadaptorFetchByIdentifier **********************************
**
** Fetch an Ensembl Genetic Variation Sample by its SQL database-internal
** identifier.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::SampleAdaptor::fetch_by_dbID
** @param [u] gvsa [EnsPGvsampleAdaptor] Ensembl Genetic Variation
**                                       Sample Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvs [EnsPGvsample*] Ensembl Genetic Variation Sample address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvsampleadaptorFetchByIdentifier(EnsPGvsampleadaptor gvsa,
                                           ajuint identifier,
                                           EnsPGvsample *Pgvs)
{
    if(!gvsa)
        return ajFalse;

    if(!Pgvs)
        return ajFalse;

    *Pgvs = (EnsPGvsample) ensBaseadaptorFetchByIdentifier(gvsa, identifier);

    return ajTrue;
}




/*
** TODO: The following method is missing.
** Bio::EnsEMBL::Variation::DBSQL::Sampleadaptor::fetch_all_by_dbID_list
*/
