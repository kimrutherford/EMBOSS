/* @source ensontology ********************************************************
**
** Ensembl Ontology functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.15 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/07/14 14:52:40 $ by $Author: rice $
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

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensontology.h"
#include "enstable.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic ontologyadaptorKTables ****************************************
**
** Array of Ensembl Ontology Adaptor SQL table names
**
******************************************************************************/

static const char *const ontologyadaptorKTables[] =
{
    "ontology",
    (const char *) NULL
};




/* @conststatic ontologyadaptorKColumns ***************************************
**
** Array of Ensembl Ontology Adaptor SQL column names
**
******************************************************************************/

static const char *const ontologyadaptorKColumns[] =
{
    "ontology.ontology_id",
    "ontology.name",
    "ontology.namespace",
    (const char *) NULL
};




/* @conststatic ontologytermadaptorKTables ************************************
**
** Array of Ensembl Ontology Term Adaptor SQL table names
**
******************************************************************************/

static const char *const ontologytermadaptorKTables[] =
{
    "term",
    (const char *) NULL
};




/* @conststatic ontologytermadaptorKColumns ***********************************
**
** Array of Ensembl Ontology Term Adaptor SQL column names
**
******************************************************************************/

static const char *const ontologytermadaptorKColumns[] =
{
    "term.term_id",
    "term.ontology_id",
    "term.accession",
    "term.name",
    "term.definition",
    (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static AjBool ontologyadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ontologies);

static AjBool ontologyadaptorCacheInit(EnsPOntologyadaptor oa);

static AjBool ontologyadaptorCacheInsert(EnsPOntologyadaptor oa,
                                         EnsPOntology *Pontology);

static void ontologyadaptorFetchAll(const void *key,
                                    void **Pvalue,
                                    void *cl);

static AjBool ontologytermadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ots);

static void ontologytermListOntologytermValdel(void **Pvalue);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensontology ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPOntology] Ensembl Ontology *******************************
**
** @nam2rule Ontology Functions for manipulating Ensembl Ontology objects
**
** @cc Split out of the Bio::EnsEMBL::OntologyTerm class
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ontology by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Ontology.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOntology]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ontology [const EnsPOntology] Ensembl Ontology
** @argrule Ini oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ini space [AjPStr] Name space
** @argrule Ref ontology [EnsPOntology] Ensembl Ontology
**
** @valrule * [EnsPOntology] Ensembl Ontology or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOntologyNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
**
** @return [EnsPOntology] Ensembl Ontology or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntology ensOntologyNewCpy(const EnsPOntology ontology)
{
    EnsPOntology pthis = NULL;

    if (!ontology)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = ontology->Identifier;
    pthis->Adaptor    = ontology->Adaptor;

    if (ontology->Name)
        pthis->Name = ajStrNewRef(ontology->Name);

    if (ontology->Space)
        pthis->Space = ajStrNewRef(ontology->Space);

    return pthis;
}




/* @func ensOntologyNewIni ****************************************************
**
** Constructor for an Ensembl Ontology object with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Ontology::new
** @param [u] name [AjPStr] Name
** @param [u] space [AjPStr] Name space
**
** @return [EnsPOntology] Ensembl Ontology or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntology ensOntologyNewIni(EnsPOntologyadaptor oa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr space)
{
    EnsPOntology ontology = NULL;

    if (!name)
        return NULL;

    AJNEW0(ontology);

    ontology->Use        = 1U;
    ontology->Identifier = identifier;
    ontology->Adaptor    = oa;

    if (name)
        ontology->Name = ajStrNewRef(name);

    if (space)
        ontology->Space = ajStrNewRef(space);

    return ontology;
}




/* @func ensOntologyNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ontology [EnsPOntology] Ensembl Ontology
**
** @return [EnsPOntology] Ensembl Ontology or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntology ensOntologyNewRef(EnsPOntology ontology)
{
    if (!ontology)
        return NULL;

    ontology->Use++;

    return ontology;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ontology object.
**
** @fdata [EnsPOntology]
**
** @nam3rule Del Destroy (free) an Ensembl Ontology
**
** @argrule * Pontology [EnsPOntology*] Ensembl Ontology address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOntologyDel *******************************************************
**
** Default destructor for an Ensembl Ontology.
**
** @param [d] Pontology [EnsPOntology*] Ensembl Ontology address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOntologyDel(EnsPOntology *Pontology)
{
    EnsPOntology pthis = NULL;

    if (!Pontology)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOntologyDel"))
    {
        ajDebug("ensOntologyDel\n"
                "  *Pontology %p\n",
                *Pontology);

        ensOntologyTrace(*Pontology, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pontology)
        return;

    pthis = *Pontology;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pontology = NULL;

        return;
    }

    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Space);

    AJFREE(pthis);

    *Pontology = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ontology object.
**
** @fdata [EnsPOntology]
**
** @nam3rule Get Return Ensembl Ontology attribute(s)
** @nam4rule Adaptor Return the Ensembl Ontology Adaptor
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Space Return the name space
**
** @argrule * ontology [const EnsPOntology] Ensembl Ontology
**
** @valrule Adaptor [EnsPOntologyadaptor] Ensembl Ontology Adaptor or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Space [AjPStr] Space or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOntologyGetAdaptor ************************************************
**
** Get the Ensembl Ontology Adaptor member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
**
** @return [EnsPOntologyadaptor] Ensembl Ontology Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologyadaptor ensOntologyGetAdaptor(const EnsPOntology ontology)
{
    return (ontology) ? ontology->Adaptor : NULL;
}




/* @func ensOntologyGetIdentifier *********************************************
**
** Get the SQL database-internal identifier member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOntologyGetIdentifier(const EnsPOntology ontology)
{
    return (ontology) ? ontology->Identifier : 0U;
}




/* @func ensOntologyGetName ***************************************************
**
** Get the name member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::OntologyTerm::ontology
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
**
** @return [AjPStr] Name or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologyGetName(const EnsPOntology ontology)
{
    return (ontology) ? ontology->Name : NULL;
}




/* @func ensOntologyGetSpace **************************************************
**
** Get the name space member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::OntologyTerm::namespace
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
**
** @return [AjPStr] Name space or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologyGetSpace(const EnsPOntology ontology)
{
    return (ontology) ? ontology->Space : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Ontology object.
**
** @fdata [EnsPOntology]
**
** @nam3rule Set Set one member of an Ensembl Ontology
** @nam4rule Adaptor Set the Ensembl Ontology Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Space Set the name space
**
** @argrule * ontology [EnsPOntology] Ensembl Ontology object
** @argrule Adaptor oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Space space [AjPStr] Name space
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOntologySetAdaptor ************************************************
**
** Set the Ensembl Ontology Adaptor member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] ontology [EnsPOntology] Ensembl Ontology
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologySetAdaptor(EnsPOntology ontology,
                             EnsPOntologyadaptor oa)
{
    if (!ontology)
        return ajFalse;

    ontology->Adaptor = oa;

    return ajTrue;
}




/* @func ensOntologySetIdentifier *********************************************
**
** Set the SQL database-internal identifier member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] ontology [EnsPOntology] Ensembl Ontology
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologySetIdentifier(EnsPOntology ontology,
                                ajuint identifier)
{
    if (!ontology)
        return ajFalse;

    ontology->Identifier = identifier;

    return ajTrue;
}




/* @func ensOntologySetName ***************************************************
**
** Set the name member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::OntologyTerm::ontology
** @param [u] ontology [EnsPOntology] Ensembl Ontology
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologySetName(EnsPOntology ontology,
                          AjPStr name)
{
    if (!ontology)
        return ajFalse;

    ajStrDel(&ontology->Name);

    if (name)
        ontology->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensOntologySetSpace **************************************************
**
** Set the name space member of an Ensembl Ontology.
**
** @cc Bio::EnsEMBL::OntologyTerm::namespace
** @param [u] ontology [EnsPOntology] Ensembl Ontology
** @param [u] space [AjPStr] Name space
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologySetSpace(EnsPOntology ontology,
                           AjPStr space)
{
    if (!ontology)
        return ajFalse;

    ajStrDel(&ontology->Space);

    if (space)
        ontology->Space = ajStrNewRef(space);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ontology object.
**
** @fdata [EnsPOntology]
**
** @nam3rule Trace Report Ensembl Ontology members to debug file.
**
** @argrule Trace ontology [const EnsPOntology] Ensembl Ontology
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologyTrace *****************************************************
**
** Trace an Ensembl Ontology.
**
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologyTrace(const EnsPOntology ontology, ajuint level)
{
    AjPStr indent = NULL;

    if (!ontology)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensOntologyTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n"
            "%S  Space '%S'\n",
            indent, ontology,
            indent, ontology->Use,
            indent, ontology->Identifier,
            indent, ontology->Adaptor,
            indent, ontology->Name,
            indent, ontology->Space);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Ontology object.
**
** @fdata [EnsPOntology]
**
** @nam3rule Calculate Calculate Ensembl Ontology values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * ontology [const EnsPOntology] Ensembl Ontology
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologyCalculateMemsize ******************************************
**
** Get the memory size in bytes of an Ensembl Ontology.
**
** @param [r] ontology [const EnsPOntology] Ensembl Ontology
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensOntologyCalculateMemsize(const EnsPOntology ontology)
{
    size_t size = 0;

    if (!ontology)
        return 0;

    size += sizeof (EnsOOntology);

    if (ontology->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ontology->Name);
    }

    if (ontology->Space)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ontology->Space);
    }

    return size;
}




/* @datasection [EnsPOntologyadaptor] Ensembl Ontology Adaptor ****************
**
** @nam2rule Ontologyadaptor Functions for manipulating
** Ensembl Ontology Adaptor objects
**
** @cc Split out of the Bio::EnsEMBL::DBSQL::OntologyTermAdaptor class
**
******************************************************************************/




/* @funcstatic ontologyadaptorFetchAllbyStatement *****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Ontology objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] ontologies [AjPList] AJAX List of Ensembl Ontology objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool ontologyadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ontologies)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name  = NULL;
    AjPStr space = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntology ontology  = NULL;
    EnsPOntologyadaptor oa = NULL;

    if (ajDebugTest("ontologyadaptorFetchAllbyStatement"))
        ajDebug("ontologyadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  ontologies %p\n",
                ba,
                statement,
                am,
                slice,
                ontologies);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!ontologies)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    oa = ensRegistryGetOntologyadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name       = ajStrNew();
        space      = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &space);

        ontology = ensOntologyNewIni(oa, identifier, name, space);

        ajListPushAppend(ontologies, (void *) ontology);

        ajStrDel(&name);
        ajStrDel(&space);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ontology Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Ontology Adaptor.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOntologyadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPOntologyadaptor] Ensembl Ontology Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOntologyadaptorNew ************************************************
**
** Default constructor for an Ensembl Ontology Adaptor.
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
** @see ensRegistryGetOntologyadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPOntologyadaptor] Ensembl Ontology Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologyadaptor ensOntologyadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPOntologyadaptor oa = NULL;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensOntologyadaptorNew"))
        ajDebug("ensOntologyadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(oa);

    oa->Adaptor = ensBaseadaptorNew(
        dba,
        ontologyadaptorKTables,
        ontologyadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &ontologyadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** ontologyadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint,
    ** which calls ontologyadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetOntologyadaptor. At that point, however,
    ** the Ensembl Ontology Adaptor has not been stored in the Registry.
    ** Therefore, each ensOntologyadaptorFetch function has to test the
    ** presence of the adaptor-internal cache and eventually initialise
    ** before accessing it.
    **
    ** ontologyadaptorCacheInit(oa);
    */

    return oa;
}




/* @section cache *************************************************************
**
** Functions for maintaining the Ensembl Ontology Adaptor-internal cache of
** Ensembl Ontology objects.
**
** @fdata [EnsPOntologyadaptor]
**
** @nam3rule Cache Process an Ensembl Ontology Adaptor-internal cache
** @nam4rule Clear Clear the Ensembl Ontology Adaptor-internal cache
**
** @argrule * oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
**
** @valrule * [AjBool] True on success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic ontologyadaptorCacheInit ***************************************
**
** Initialise the internal Ensembl Ontology cache of an
** Ensembl Ontology Adaptor.
**
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool ontologyadaptorCacheInit(EnsPOntologyadaptor oa)
{
    AjPList ontologies = NULL;

    EnsPOntology ontology = NULL;

    if (ajDebugTest("ontologyadaptorCacheInit"))
        ajDebug("ontologyadaptorCacheInit\n"
                "  oa %p\n",
                oa);

    if (!oa)
        return ajFalse;

    if (oa->CacheByIdentifier)
        return ajFalse;
    else
    {
        oa->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            oa->CacheByIdentifier,
            (void (*)(void **)) &ensOntologyDel);
    }

    if (oa->CacheByNamespace)
        return ajFalse;
    else
    {
        oa->CacheByNamespace = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            oa->CacheByNamespace,
            (void (*)(void **)) &ensOntologyDel);
    }

    ontologies = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(oa->Adaptor,
                                       (const AjPStr) NULL,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       ontologies);

    while (ajListPop(ontologies, (void **) &ontology))
    {
        ontologyadaptorCacheInsert(oa, &ontology);

        /*
        ** Both caches hold internal references to the
        ** Ensembl Ontology objects.
        */

        ensOntologyDel(&ontology);
    }

    ajListFree(&ontologies);

    return ajTrue;
}




/* @funcstatic ontologyadaptorCacheInsert *************************************
**
** Insert an Ensembl Ontology into the Ensembl Ontology Adaptor-internal cache.
** If an Ensembl Ontology with the same name member is already present in the
** Ensembl Ontology Adaptor-internal cache, the Ensembl Ontology is deleted and
** a pointer to the cached Ensembl Ontology is returned.
**
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [u] Pontology [EnsPOntology*] Ensembl Ontology address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool ontologyadaptorCacheInsert(EnsPOntologyadaptor oa,
                                         EnsPOntology *Pontology)
{
    ajuint *Pidentifier = NULL;

    AjPStr key = NULL;

    EnsPOntology ontology1 = NULL;
    EnsPOntology ontology2 = NULL;

    if (!oa)
        return ajFalse;

    if (!oa->CacheByIdentifier)
        return ajFalse;

    if (!oa->CacheByNamespace)
        return ajFalse;

    if (!Pontology)
        return ajFalse;

    if (!*Pontology)
        return ajFalse;

    /* Search the identifer cache. */

    ontology1 = (EnsPOntology) ajTableFetchmodV(
        oa->CacheByIdentifier,
        (const void *) &((*Pontology)->Identifier));

    /* Search the name cache. */

    key = ajFmtStr("%S:%S", (*Pontology)->Name, (*Pontology)->Space);

    ontology2 = (EnsPOntology) ajTableFetchmodS(oa->CacheByNamespace, key);

    if ((!ontology1) && (!ontology2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Pontology)->Identifier;

        ajTablePut(oa->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensOntologyNewRef(*Pontology));

        /* Insert into the namespace cache. */

        ajTablePut(oa->CacheByNamespace,
                   (void *) ajStrNewS(key),
                   (void *) ensOntologyNewRef(*Pontology));
    }

    if (ontology1 && ontology2 && (ontology1 == ontology2))
    {
        ajDebug("ontologyadaptorCacheInsert replaced Ensembl Ontology %p "
                "with one already cached %p.\n",
                *Pontology, ontology1);

        ensOntologyDel(Pontology);

        ensOntologyNewRef(ontology1);

        Pontology = &ontology1;
    }

    if (ontology1 && ontology2 && (ontology1 != ontology2))
        ajDebug("ontologyadaptorCacheInsert detected Ensembl Ontology objects "
                "in the identifier and namespace cache with identical keys "
                "('%S:%S' and '%S:%S') but different addresses (%p and %p).\n",
                ontology1->Name, ontology1->Space,
                ontology2->Name, ontology2->Space,
                ontology1, ontology2);

    if (ontology1 && (!ontology2))
        ajDebug("ontologyadaptorCacheInsert detected an Ensembl Ontology "
                "in the identifier, but not in the namespace cache.\n");

    if ((!ontology1) && ontology2)
        ajDebug("ontologyadaptorCacheInsert detected an Ensembl Ontology "
                "in the namespace, but not in the identifier cache.\n");

    ajStrDel(&key);

    return ajTrue;
}




#if AJFALSE
/* @funcstatic ontologyadaptorCacheRemove *************************************
**
** Remove an Ensembl Ontology from the Ensembl Ontology Adaptor-internal cache.
**
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [u] ontology [EnsPOntology] Ensembl Ontology
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool ontologyadaptorCacheRemove(EnsPOntologyadaptor oa,
                                         EnsPOntology ontology)
{
    AjPStr key = NULL;

    EnsPOntology ontology1 = NULL;
    EnsPOntology ontology2 = NULL;

    if (!oa)
        return ajFalse;

    if (!ontology)
        return ajFalse;

    ontology1 = (EnsPOntology) ajTableRemove(
        oa->CacheByIdentifier,
        (const void *) &ontology->Identifier);

    key = ajFmtStr("%S:%S", ontology->Name, ontology->Space);

    ontology2 = (EnsPOntology) ajTableRemove(
        oa->CacheByNamespace,
        (const void *) key);

    if (ontology1 && (!ontology2))
        ajWarn("ontologyadaptorCacheRemove could remove Ensembl Ontology with "
               "identifier %u and key '%S' only from the identifier cache.\n",
               ontology->Identifier,
               newkey);

    if ((!ontology1) && ontology2)
        ajWarn("ontologyadaptorCacheRemove could remove Ensembl Ontology with "
               "identifier %u and key '%S' only from the name cache.\n",
               ontology->Identifier,
               newkey);

    ajStrDel(&key);

    ensOntologyDel(&ontology1);
    ensOntologyDel(&ontology2);

    return ajTrue;
}

#endif /* AJFALSE */




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ontology Adaptor object.
**
** @fdata [EnsPOntologyadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Ontology Adaptor
**
** @argrule * Poa [EnsPOntologyadaptor*] Ensembl Ontology Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOntologyadaptorDel ************************************************
**
** Default destructor for an Ensembl Ontology Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Poa [EnsPOntologyadaptor*] Ensembl Ontology Adaptor address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOntologyadaptorDel(EnsPOntologyadaptor *Poa)
{
    EnsPOntologyadaptor pthis = NULL;

    if (!Poa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOntologyadaptorDel"))
        ajDebug("ensOntologyadaptorDel\n"
                "  *Poa %p\n",
                *Poa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Poa)
        return;

    pthis = *Poa;

    ajTableDel(&pthis->CacheByIdentifier);
    ajTableDel(&pthis->CacheByNamespace);

    ensBaseadaptorDel(&pthis->Adaptor);

    AJFREE(pthis);

    *Poa = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ontology Adaptor object.
**
** @fdata [EnsPOntologyadaptor]
**
** @nam3rule Get Return Ensembl Ontology Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * oa [const EnsPOntologyadaptor] Ensembl Ontology Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOntologyadaptorGetBaseadaptor *************************************
**
** Get the Ensembl Base Adaptor member of an Ensembl Ontology Adaptor.
**
** @param [r] oa [const EnsPOntologyadaptor] Ensembl Ontology Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPBaseadaptor ensOntologyadaptorGetBaseadaptor(
    const EnsPOntologyadaptor oa)
{
    if (!oa)
        return NULL;

    return (oa) ? oa->Adaptor : NULL;
}




/* @func ensOntologyadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Ontology Adaptor.
**
** @param [r] oa [const EnsPOntologyadaptor] Ensembl Ontology Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensOntologyadaptorGetDatabaseadaptor(
    const EnsPOntologyadaptor oa)
{
    if (!oa)
        return NULL;

    return (oa) ? ensBaseadaptorGetDatabaseadaptor(oa->Adaptor) : NULL;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Ontology objects from an
** Ensembl SQL database.
**
** @fdata [EnsPOntologyadaptor]
**
** @nam3rule Fetch Fetch Ensembl Ontology object(s)
** @nam4rule All Fetch all Ensembl Ontology objects
** @nam4rule Allby Fetch all Ensembl Ontology objects matching a criterion
** @nam4rule By Fetch one Ensembl Ontology object matching a criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
** @nam5rule Name Fetch by a name
**
** @argrule * oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @argrule All ontologies [AjPList] AJAX List of Ensembl Ontology objects
** @argrule AllbyName name [const AjPStr] Ensembl Ontology name
** @argrule Allby ontologies [AjPList] AJAX List of Ensembl Ontology objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByName name [const AjPStr] Ensembl Ontology name
** @argrule ByName space [const AjPStr] Ensembl Ontology name space
** @argrule By Pontology [EnsPOntology*] Ensembl Ontology address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic ontologyadaptorFetchAll ****************************************
**
** An ajTableMap "apply" function to return all Ensembl Ontology objects from
** the Ensembl Ontology Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] Pvalue [void**] Ensembl Ontology value data address
** @param [u] cl [void*]
** AJAX List of Ensembl Ontology objects, passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void ontologyadaptorFetchAll(const void *key,
                                    void **Pvalue,
                                    void *cl)
{
    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void *) ensOntologyNewRef(*((EnsPOntology *) Pvalue)));

    return;
}




/* @func ensOntologyadaptorFetchAll *******************************************
**
** Fetch all Ensembl Ontology objects.
**
** The caller is responsible for deleting the Ensembl Ontology objects before
** deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyAdaptor::fetch_all
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [u] ontologies [AjPList] AJAX List of Ensembl Ontology objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologyadaptorFetchAll(EnsPOntologyadaptor oa,
                                  AjPList ontologies)
{
    if (!oa)
        return ajFalse;

    if (!ontologies)
        return ajFalse;

    if (!oa->CacheByIdentifier)
        ontologyadaptorCacheInit(oa);

    ajTableMap(oa->CacheByIdentifier,
               &ontologyadaptorFetchAll,
               (void *) ontologies);

    return ajTrue;
}




/* @func ensOntologyadaptorFetchAllbyName *************************************
**
** Fetch all Ensembl Ontology objects by a name.
**
** The caller is responsible for deleting the Ensembl Ontology objects before
** deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyAdaptor::???
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [r] name [const AjPStr] Ensembl Ontology name
** @param [u] ontologies [AjPList] AJAX List of Ensembl Ontology objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologyadaptorFetchAllbyName(EnsPOntologyadaptor oa,
                                        const AjPStr name,
                                        AjPList ontologies)
{
    void **valarray = NULL;

    register ajuint i = 0U;

    if (!oa)
        return ajFalse;

    if (!(name && ajStrGetLen(name)))
        return ajFalse;

    if (!ontologies)
        return ajFalse;

    if (!oa->CacheByIdentifier)
        ontologyadaptorCacheInit(oa);

    ajTableToarrayValues(oa->CacheByIdentifier, &valarray);

    for (i = 0U; valarray[i]; i++)
        if (ajStrMatchS(ensOntologyGetName((EnsPOntology) valarray[i]), name))
            ajListPushAppend(
                ontologies,
                (void *) ensOntologyNewRef((EnsPOntology) valarray[i]));

    AJFREE(valarray);

    return ajTrue;
}




/* @func ensOntologyadaptorFetchByIdentifier **********************************
**
** Fetch an Ensembl Ontology by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Ontology.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyAdaptor::fetch_by_dbID
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pontology [EnsPOntology*] Ensembl Ontology address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologyadaptorFetchByIdentifier(EnsPOntologyadaptor oa,
                                           ajuint identifier,
                                           EnsPOntology *Pontology)
{
    AjPList ontologies = NULL;

    AjPStr constraint = NULL;

    EnsPOntology ontology = NULL;

    if (!oa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pontology)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!oa->CacheByIdentifier)
        ontologyadaptorCacheInit(oa);

    *Pontology = (EnsPOntology) ajTableFetchmodV(oa->CacheByIdentifier,
                                                 (const void *) &identifier);

    if (*Pontology)
    {
        ensOntologyNewRef(*Pontology);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    constraint = ajFmtStr("ontology.ontology_id = %u", identifier);

    ontologies = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(oa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       ontologies);

    if (ajListGetLength(ontologies) > 1)
        ajWarn("ensOntologyadaptorFetchByIdentifier got more than one "
               "Ensembl Ontology for (PRIMARY KEY) identifier %u.\n",
               identifier);

    ajListPop(ontologies, (void **) Pontology);

    ontologyadaptorCacheInsert(oa, Pontology);

    while (ajListPop(ontologies, (void **) &ontology))
    {
        ontologyadaptorCacheInsert(oa, &ontology);

        ensOntologyDel(&ontology);
    }

    ajListFree(&ontologies);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensOntologyadaptorFetchByName ****************************************
**
** Fetch an Ensembl Ontology by its name.
** The caller is responsible for deleting the Ensembl Ontology.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyAdaptor::fetch_by_logic_name
** @param [u] oa [EnsPOntologyadaptor] Ensembl Ontology Adaptor
** @param [r] name [const AjPStr] Ensembl Ontology name
** @param [r] space [const AjPStr] Ensembl Ontology name space
** @param [wP] Pontology [EnsPOntology*] Ensembl Ontology address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologyadaptorFetchByName(EnsPOntologyadaptor oa,
                                     const AjPStr name,
                                     const AjPStr space,
                                     EnsPOntology *Pontology)
{
    char *txtname  = NULL;
    char *txtspace = NULL;

    AjPList ontologies = NULL;

    AjPStr constraint = NULL;
    AjPStr key        = NULL;

    EnsPOntology ontology = NULL;

    if (!oa)
        return ajFalse;

    if (!(name && ajStrGetLen(name)))
        return ajFalse;

    if (!(space && ajStrGetLen(space)))
        return ajFalse;

    if (!Pontology)
        return ajFalse;

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!oa->CacheByNamespace)
        ontologyadaptorCacheInit(oa);

    key = ajFmtStr("%S:%S", name, space);

    *Pontology = (EnsPOntology) ajTableFetchmodV(oa->CacheByNamespace,
                                                 (const void *) key);

    if (*Pontology)
    {
        ensOntologyNewRef(*Pontology);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ensBaseadaptorEscapeC(oa->Adaptor, &txtname, name);
    ensBaseadaptorEscapeC(oa->Adaptor, &txtspace, space);

    constraint = ajFmtStr("ontology.name = '%s' AND ontology.namespace = '%s'",
                          txtname, txtspace);

    ajCharDel(&txtname);
    ajCharDel(&txtspace);

    ontologies = ajListNew();

    ensBaseadaptorFetchAllbyConstraint(oa->Adaptor,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       ontologies);

    if (ajListGetLength(ontologies) > 1)
        ajWarn("ensOntologyadaptorFetchByName got more than one "
               "Ensembl Ontology for (UNIQUE) name '%S'.\n",
               name);

    ajListPop(ontologies, (void **) Pontology);

    ontologyadaptorCacheInsert(oa, Pontology);

    while (ajListPop(ontologies, (void **) &ontology))
    {
        ontologyadaptorCacheInsert(oa, &ontology);

        ensOntologyDel(&ontology);
    }

    ajListFree(&ontologies);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @datasection [EnsPOntologysynonym] Ensembl Ontology Synonym ****************
**
** @nam2rule Ontologysynonym Functions for manipulating
** Ensembl Ontology Synonym objects
**
** @cc Split out of the Bio::EnsEMBL::OntologyTerm class
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ontology Synonym by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Ontology Synonym.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOntologysynonym]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy os [const EnsPOntologysynonym] Ensembl Ontology Synonym
** @argrule Ini ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini name [AjPStr] Name
** @argrule Ref os [EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @valrule * [EnsPOntologysynonym] Ensembl Ontology Synonym or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOntologysynonymNewCpy *********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @return [EnsPOntologysynonym] Ensembl Ontology Synonym or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologysynonym ensOntologysynonymNewCpy(const EnsPOntologysynonym os)
{
    EnsPOntologysynonym pthis = NULL;

    if (!os)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = os->Identifier;
    pthis->Adaptor    = os->Adaptor;

    if (os->Name)
        pthis->Name = ajStrNewRef(os->Name);

    return pthis;
}




/* @func ensOntologysynonymNewIni *********************************************
**
** Constructor for an Ensembl Ontology Synonym object with initial values.
**
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [u] name [AjPStr] Name
**
** @return [EnsPOntologysynonym] Ensembl Ontology Synonym or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologysynonym ensOntologysynonymNewIni(EnsPOntologytermadaptor ota,
                                             ajuint identifier,
                                             AjPStr name)
{
    EnsPOntologysynonym os = NULL;

    if (!name)
        return NULL;

    AJNEW0(os);

    os->Use        = 1U;
    os->Identifier = identifier;
    os->Adaptor    = ota;

    if (name)
        os->Name = ajStrNewRef(name);

    return os;
}




/* @func ensOntologysynonymNewRef *********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] os [EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @return [EnsPOntologysynonym] Ensembl Ontology Synonym or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologysynonym ensOntologysynonymNewRef(EnsPOntologysynonym os)
{
    if (!os)
        return NULL;

    os->Use++;

    return os;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ontology Synonym object.
**
** @fdata [EnsPOntologysynonym]
**
** @nam3rule Del Destroy (free) an Ensembl Ontology Synonym
**
** @argrule * Pos [EnsPOntologysynonym*] Ensembl Ontology Synonym address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOntologysynonymDel ************************************************
**
** Default destructor for an Ensembl Ontology Synonym.
**
** @param [d] Pos [EnsPOntologysynonym*] Ensembl Ontology Synonym address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOntologysynonymDel(EnsPOntologysynonym *Pos)
{
    EnsPOntologysynonym pthis = NULL;

    if (!Pos)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOntologysynonymDel"))
    {
        ajDebug("ensOntologysynonymDel\n"
                "  *Pos %p\n",
                *Pos);

        ensOntologysynonymTrace(*Pos, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pos)
        return;

    pthis = *Pos;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pos = NULL;

        return;
    }

    ajStrDel(&pthis->Name);

    AJFREE(pthis);

    *Pos = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ontology Synonym object.
**
** @fdata [EnsPOntologysynonym]
**
** @nam3rule Get Return Ensembl Ontology Synonym attribute(s)
** @nam4rule Adaptor Return the Ensembl Ontology Term Adaptor
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
**
** @argrule * os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @valrule Adaptor [EnsPOntologytermadaptor]
** Ensembl Ontology Term Adaptor or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOntologysynonymGetAdaptor *****************************************
**
** Get the Ensembl Ontology Term Adaptor member of an
** Ensembl Ontology Synonym.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @return [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologytermadaptor ensOntologysynonymGetAdaptor(
    const EnsPOntologysynonym os)
{
    return (os) ? os->Adaptor : NULL;
}




/* @func ensOntologysynonymGetIdentifier **************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Ontology Synonym.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOntologysynonymGetIdentifier(
    const EnsPOntologysynonym os)
{
    return (os) ? os->Identifier : 0U;
}




/* @func ensOntologysynonymGetName ********************************************
**
** Get the name member of an Ensembl Ontology Synonym.
**
** @param [r] os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @return [AjPStr] Name or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologysynonymGetName(
    const EnsPOntologysynonym os)
{
    return (os) ? os->Name : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Ontology Synonym object.
**
** @fdata [EnsPOntologysynonym]
**
** @nam3rule Set Set one member of an Ensembl Ontology Synonym
** @nam4rule Adaptor Set the Ensembl Ontology Term Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
**
** @argrule * os [EnsPOntologysynonym] Ensembl Ontology Synonym object
** @argrule Adaptor ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOntologysynonymSetAdaptor *****************************************
**
** Set the Ensembl Ontology Term Adaptor member of an
** Ensembl Ontology Synonym.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] os [EnsPOntologysynonym] Ensembl Ontology Synonym
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologysynonymSetAdaptor(EnsPOntologysynonym os,
                                    EnsPOntologytermadaptor ota)
{
    if (!os)
        return ajFalse;

    os->Adaptor = ota;

    return ajTrue;
}




/* @func ensOntologysynonymSetIdentifier **************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Ontology Synonym.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] os [EnsPOntologysynonym] Ensembl Ontology Synonym
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologysynonymSetIdentifier(EnsPOntologysynonym os,
                                       ajuint identifier)
{
    if (!os)
        return ajFalse;

    os->Identifier = identifier;

    return ajTrue;
}




/* @func ensOntologysynonymSetName ********************************************
**
** Set the name member of an Ensembl Ontology Synonym.
**
** @param [u] os [EnsPOntologysynonym] Ensembl Ontology Synonym
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologysynonymSetName(EnsPOntologysynonym os,
                                 AjPStr name)
{
    if (!os)
        return ajFalse;

    ajStrDel(&os->Name);

    if (name)
        os->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ontology Synonym object.
**
** @fdata [EnsPOntologysynonym]
**
** @nam3rule Trace Report Ensembl Ontology Synonym members to debug file.
**
** @argrule Trace os [const EnsPOntologysynonym] Ensembl Ontology Synonym
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologysynonymTrace **********************************************
**
** Trace an Ensembl Ontology Synonym.
**
** @param [r] os [const EnsPOntologysynonym] Ensembl Ontology Synonym
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologysynonymTrace(const EnsPOntologysynonym os, ajuint level)
{
    AjPStr indent = NULL;

    if (!os)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensOntologysynonymTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Name '%S'\n",
            indent, os,
            indent, os->Use,
            indent, os->Identifier,
            indent, os->Adaptor,
            indent, os->Name);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Ontology Synonym object.
**
** @fdata [EnsPOntologysynonym]
**
** @nam3rule Calculate Calculate Ensembl Ontology Synonym values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologysynonymCalculateMemsize ***********************************
**
** Get the memory size in bytes of an Ensembl Ontology Synonym.
**
** @param [r] os [const EnsPOntologysynonym] Ensembl Ontology Synonym
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensOntologysynonymCalculateMemsize(const EnsPOntologysynonym os)
{
    size_t size = 0;

    if (!os)
        return 0;

    size += sizeof (EnsOOntologysynonym);

    if (os->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(os->Name);
    }

    return size;
}




/* @datasection [EnsPOntologyterm] Ensembl Ontology Term **********************
**
** @nam2rule Ontologyterm Functions for manipulating
** Ensembl Ontology Term objects
**
** @cc Bio::EnsEMBL::OntologyTerm
** @cc CVS Revision: 1.18
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ontology Term by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Ontology Term.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy ot [const EnsPOntologyterm] Ensembl Ontology Term
** @argrule Ini ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini ontology [EnsPOntology] Ensembl Ontology
** @argrule Ini accession [AjPStr] Accession
** @argrule Ini definition [AjPStr] Definition
** @argrule Ini name [AjPStr] Name
** @argrule Ini subsets [AjPStr] Sub-sets
** @argrule Ref ot [EnsPOntologyterm] Ensembl Ontology Term
**
** @valrule * [EnsPOntologyterm] Ensembl Ontology Term or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOntologytermNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [EnsPOntologyterm] Ensembl Ontology Term or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologyterm ensOntologytermNewCpy(const EnsPOntologyterm ot)
{
    AjIList iter = NULL;

    EnsPOntologysynonym os = NULL;

    EnsPOntologyterm pthis = NULL;

    if (!ot)
        return NULL;

    AJNEW0(pthis);

    pthis->Use        = 1U;
    pthis->Identifier = ot->Identifier;
    pthis->Adaptor    = ot->Adaptor;
    pthis->Ontology   = ensOntologyNewRef(ot->Ontology);

    if (ot->Accession)
        pthis->Accession = ajStrNewRef(ot->Accession);

    if (ot->Definition)
        pthis->Definition = ajStrNewRef(ot->Definition);

    if (ot->Name)
        pthis->Name = ajStrNewRef(ot->Name);

    if (ot->Subsets)
        pthis->Subsets = ajStrNewRef(ot->Subsets);

    /*
    ** FIXME: The AJAX Table objects for child and parent
    ** Ensembl Ontology Term objects are not copied.
    */

    if (ot->Ontologysynonyms)
    {
        pthis->Ontologysynonyms = ajListNew();

        iter = ajListIterNew(ot->Ontologysynonyms);

        while (!ajListIterDone(iter))
        {
            os = (EnsPOntologysynonym) ajListIterGet(iter);

            ajListPushAppend(pthis->Ontologysynonyms,
                             (void *) ensOntologysynonymNewRef(os));
        }

        ajListIterDel(&iter);
    }

    return pthis;
}




/* @func ensOntologytermNewIni ************************************************
**
** Constructor for an Ensembl Ontology Term object with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::OntologyTerm::new
** @param [u] ontology [EnsPOntology] Ensembl Ontology
** @param [uN] accession [AjPStr] Accession
** @param [uN] definition [AjPStr] Definition
** @param [uN] name [AjPStr] Name
** @param [uN] subsets [AjPStr] Sub-sets
**
** @return [EnsPOntologyterm] Ensembl Ontology Term or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologyterm ensOntologytermNewIni(EnsPOntologytermadaptor ota,
                                       ajuint identifier,
                                       EnsPOntology ontology,
                                       AjPStr accession,
                                       AjPStr definition,
                                       AjPStr name,
                                       AjPStr subsets)
{
    EnsPOntologyterm ot = NULL;

    if (!ontology)
        return NULL;

    AJNEW0(ot);

    ot->Use        = 1U;
    ot->Identifier = identifier;
    ot->Adaptor    = ota;
    ot->Ontology   = ensOntologyNewRef(ontology);

    if (accession)
        ot->Accession = ajStrNewRef(accession);

    if (definition)
        ot->Definition = ajStrNewRef(definition);

    if (name)
        ot->Name = ajStrNewRef(name);

    if (subsets)
        ot->Subsets = ajStrNewRef(subsets);

    return ot;
}




/* @func ensOntologytermNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
**
** @return [EnsPOntologyterm] Ensembl Ontology Term or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologyterm ensOntologytermNewRef(EnsPOntologyterm ot)
{
    if (!ot)
        return NULL;

    ot->Use++;

    return ot;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ontology Term object.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule Del Destroy (free) an Ensembl Ontology Term
**
** @argrule * Pot [EnsPOntologyterm*] Ensembl Ontology Term address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOntologytermDel ***************************************************
**
** Default destructor for an Ensembl Ontology Term.
**
** @param [d] Pot [EnsPOntologyterm*] Ensembl Ontology Term address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOntologytermDel(EnsPOntologyterm *Pot)
{
    EnsPOntologysynonym os = NULL;

    EnsPOntologyterm pthis = NULL;

    if (!Pot)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensOntologytermDel"))
    {
        ajDebug("ensOntologytermDel\n"
                "  *Pot %p\n",
                *Pot);

        ensOntologytermTrace(*Pot, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!*Pot)
        return;

    pthis = *Pot;

    pthis->Use--;

    if (pthis->Use)
    {
        *Pot = NULL;

        return;
    }

    ensOntologyDel(&pthis->Ontology);

    ajStrDel(&pthis->Accession);
    ajStrDel(&pthis->Definition);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Subsets);

    ajTableDel(&pthis->Children);
    ajTableDel(&pthis->Parents);

    while (ajListPop(pthis->Ontologysynonyms, (void **) &os))
        ensOntologysynonymDel(&os);

    ajListFree(&pthis->Ontologysynonyms);

    AJFREE(pthis);

    *Pot = NULL;

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Ontology Term object.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule Get Return Ensembl Ontology Term attribute(s)
** @nam4rule Accession Return the accession
** @nam4rule Adaptor Return the Ensembl Ontology Term Adaptor
** @nam4rule Definition Return the definition
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Name Return the name
** @nam4rule Ontology Return the Ensembl Ontology
** @nam4rule Subsets Return the sub-sets
**
** @argrule * ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @valrule Accession [AjPStr] Accession or NULL
** @valrule Adaptor [EnsPOntologytermadaptor]
** Ensembl Ontology Term Adaptor or NULL
** @valrule Definition [AjPStr] Definition or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Name [AjPStr] Name or NULL
** @valrule Ontology [EnsPOntology] Ensembl Ontology or NULL
** @valrule Subsets [AjPStr] Sub-sets
**
** @fcategory use
******************************************************************************/




/* @func ensOntologytermGetAccession ******************************************
**
** Get the accession member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::accession
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [AjPStr] Accession or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologytermGetAccession(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Accession : NULL;
}




/* @func ensOntologytermGetAdaptor ********************************************
**
** Get the Ensembl Ontology Term Adaptor member of an
** Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologytermadaptor ensOntologytermGetAdaptor(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Adaptor : NULL;
}




/* @func ensOntologytermGetDefinition *****************************************
**
** Get the definition member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::definition
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [AjPStr] Definition or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologytermGetDefinition(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Definition : NULL;
}




/* @func ensOntologytermGetIdentifier *****************************************
**
** Get the SQL database-internal identifier member of an
** Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ensOntologytermGetIdentifier(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Identifier : 0U;
}




/* @func ensOntologytermGetName ***********************************************
**
** Get the name member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::name
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [AjPStr] Name or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologytermGetName(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Name : NULL;
}




/* @func ensOntologytermGetOntology *******************************************
**
** Get the Ensembl Ontology member of an
** Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::ontology
** @cc Bio::EnsEMBL::OntologyTerm::namespace
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [EnsPOntology] Ensembl Ontology or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntology ensOntologytermGetOntology(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Ontology : NULL;
}




/* @func ensOntologytermGetSubsets ********************************************
**
** Get the sub-sets member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::subsets
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [AjPStr] Sub-sets or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPStr ensOntologytermGetSubsets(const EnsPOntologyterm ot)
{
    return (ot) ? ot->Subsets : NULL;
}




/* @section load on demand ****************************************************
**
** Functions for returning members of an Ensembl Ontology Term object,
** which may need loading from an Ensembl SQL database on demand.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule Load Return Ensembl Ontology Term attribute(s) loaded on demand
** @nam4rule Ontologysynonyms Return all Ensembl Ontology Synonym objects
**
** @argrule * ot [EnsPOntologyterm] Ensembl Ontology Term
**
** @valrule Ontologysynonyms [const AjPList]
** AJAX List of Ensembl Ontology Synonym objects or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensOntologytermLoadOntologysynonyms **********************************
**
** Load all Ensembl Ontology Synonym objects of an Ensembl Ontology Term.
**
** This is not a simple accessor function, it will fetch
** Ensembl Ontology Synonym objects from an Ensembl SQL database in case the
** AJAX List is not defined.
**
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
**
** @return [const AjPList]
** AJAX List of Ensembl Ontology Synonym objects or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPList ensOntologytermLoadOntologysynonyms(EnsPOntologyterm ot)
{
    if (!ot)
        return NULL;

    if (ot->Ontologysynonyms)
        return ot->Ontologysynonyms;

    if (!ot->Adaptor)
    {
        ajDebug("ensOntologytermLoadOntologysynonyms cannot fetch "
                "Ensembl Ontology Synonym objects for an "
                "Ensembl Ontology Term without an "
                "Ensembl Ontology Term Adaptor.\n");

        return NULL;
    }

    ot->Ontologysynonyms = ajListNew();

    ensOntologytermadaptorRetrieveAllOntologysynonyms(ot->Adaptor,
                                                      ot,
                                                      ot->Ontologysynonyms);

    return ot->Ontologysynonyms;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Ontology Term object.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule Set Set one member of an Ensembl Ontology Term
** @nam4rule Accession Set the accession
** @nam4rule Adaptor Set the Ensembl Ontology Term Adaptor
** @nam4rule Definition Set the definition
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Name Set the name
** @nam4rule Ontology Set the Ensembl Ontology
** @nam4rule Subsets Set sub-sets
**
** @argrule * ot [EnsPOntologyterm] Ensembl Ontology Term object
** @argrule Accession accession [AjPStr] Accession
** @argrule Adaptor ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @argrule Definition definition [AjPStr] Definition
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Name name [AjPStr] Name
** @argrule Ontology ontology [EnsPOntology] Ensembl Ontology
** @argrule Subsets subsets [AjPStr] Sub-sets
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensOntologytermSetAccession ******************************************
**
** Set the accession member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::accession
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] accession [AjPStr] Accession
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetAccession(EnsPOntologyterm ot,
                                   AjPStr accession)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->Accession);

    if (accession)
        ot->Accession = ajStrNewRef(accession);

    return ajTrue;
}




/* @func ensOntologytermSetAdaptor ********************************************
**
** Set the Ensembl Ontology Term Adaptor member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetAdaptor(EnsPOntologyterm ot,
                                 EnsPOntologytermadaptor ota)
{
    if (!ot)
        return ajFalse;

    ot->Adaptor = ota;

    return ajTrue;
}




/* @func ensOntologytermSetDefinition *****************************************
**
** Set the definition member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::definition
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] definition [AjPStr] Definition
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetDefinition(EnsPOntologyterm ot,
                                    AjPStr definition)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->Definition);

    if (definition)
        ot->Definition = ajStrNewRef(definition);

    return ajTrue;
}




/* @func ensOntologytermSetIdentifier *****************************************
**
** Set the SQL database-internal identifier member of an
** Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetIdentifier(EnsPOntologyterm ot,
                                    ajuint identifier)
{
    if (!ot)
        return ajFalse;

    ot->Identifier = identifier;

    return ajTrue;
}




/* @func ensOntologytermSetName ***********************************************
**
** Set the name member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::name
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetName(EnsPOntologyterm ot,
                              AjPStr name)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->Name);

    if (name)
        ot->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensOntologytermSetOntology *******************************************
**
** Set the Ensembl Ontology member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::ontology
** @cc Bio::EnsEMBL::OntologyTerm::namespace
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] ontology [EnsPOntology] Ensembl Ontology
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetOntology(EnsPOntologyterm ot,
                                  EnsPOntology ontology)
{
    if (!ot)
        return ajFalse;

    ensOntologyDel(&ot->Ontology);

    ot->Ontology = ensOntologyNewRef(ontology);

    return ajTrue;
}




/* @func ensOntologytermSetSubsets ********************************************
**
** Set the sub-sets member of an Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::OntologyTerm::subsets
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] subsets [AjPStr] Sub-sets
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermSetSubsets(EnsPOntologyterm ot,
                                 AjPStr subsets)
{
    if (!ot)
        return ajFalse;

    ajStrDel(&ot->Subsets);

    if (subsets)
        ot->Subsets = ajStrNewRef(subsets);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Ontology Term object.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule Trace Report Ensembl Ontology Term members to debug file.
**
** @argrule Trace ot [const EnsPOntologyterm] Ensembl Ontology Term
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologytermTrace *************************************************
**
** Trace an Ensembl Ontology Term.
**
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermTrace(const EnsPOntologyterm ot, ajuint level)
{
    AjPStr indent = NULL;

    if (!ot)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensOntologytermTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Accession '%S'\n"
            "%S  Definition '%S'\n"
            "%S  Name '%S'\n"
            "%S  Ontology %p\n"
            "%S  Subsets '%S'\n",
            indent, ot,
            indent, ot->Use,
            indent, ot->Identifier,
            indent, ot->Adaptor,
            indent, ot->Accession,
            indent, ot->Definition,
            indent, ot->Name,
            indent, ot->Ontology,
            indent, ot->Subsets);

    ensOntologyTrace(ot->Ontology, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Ontology Term object.
**
** @fdata [EnsPOntologyterm]
**
** @nam3rule Calculate Calculate Ensembl Ontology Term values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensOntologytermCalculateMemsize **************************************
**
** Get the memory size in bytes of an Ensembl Ontology Term.
**
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.5.0
** @@
******************************************************************************/

size_t ensOntologytermCalculateMemsize(const EnsPOntologyterm ot)
{
    size_t size = 0;

    if (!ot)
        return 0;

    size += sizeof (EnsOOntologyterm);

    if (ot->Accession)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->Accession);
    }

    if (ot->Definition)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->Definition);
    }

    if (ot->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->Name);
    }

    size += ensOntologyCalculateMemsize(ot->Ontology);

    if (ot->Subsets)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(ot->Subsets);
    }

    return size;
}




/* @datasection [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor *******
**
** @nam2rule Ontologytermadaptor Functions for manipulating
** Ensembl Ontology Term Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor
** @cc CVS Revision: 1.44
** @cc CVS Tag: branch-ensembl-66
**
******************************************************************************/




/* @funcstatic ontologytermadaptorFetchAllbyStatement *************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Ontology Term objects.
**
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool ontologytermadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList ots)
{
    ajuint identifier = 0U;
    ajuint oid        = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr accession  = NULL;
    AjPStr definition = NULL;
    AjPStr name       = NULL;
    AjPStr subsets    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntology ontology  = NULL;
    EnsPOntologyadaptor oa = NULL;

    EnsPOntologyterm        ot  = NULL;
    EnsPOntologytermadaptor ota = NULL;

    if (ajDebugTest("ontologytermadaptorFetchAllbyStatement"))
        ajDebug("ontologytermadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  ots %p\n",
                ba,
                statement,
                am,
                slice,
                ots);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!ots)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    oa  = ensRegistryGetOntologyadaptor(dba);
    ota = ensRegistryGetOntologytermadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        oid        = 0;
        accession  = ajStrNew();
        name       = ajStrNew();
        definition = ajStrNew();
        subsets    = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &oid);
        ajSqlcolumnToStr(sqlr, &accession);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &definition);
        ajSqlcolumnToStr(sqlr, &subsets);

        ensOntologyadaptorFetchByIdentifier(oa, oid, &ontology);

        ot = ensOntologytermNewIni(ota,
                                   identifier,
                                   ontology,
                                   accession,
                                   definition,
                                   name,
                                   subsets);

        ajListPushAppend(ots, (void *) ot);

        ensOntologyDel(&ontology);

        ajStrDel(&accession);
        ajStrDel(&name);
        ajStrDel(&definition);
        ajStrDel(&subsets);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Ontology Term Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Ontology Term Adaptor.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPOntologytermadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensOntologytermadaptorNew ********************************************
**
** Default constructor for an Ensembl Ontology Term Adaptor.
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
** @see ensRegistryGetOntologytermadaptor
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

EnsPOntologytermadaptor ensOntologytermadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if (!dba)
        return NULL;

    if (ajDebugTest("ensOntologytermadaptorNew"))
        ajDebug("ensOntologytermadaptorNew\n"
                "  dba %p\n",
                dba);

    return ensBaseadaptorNew(
        dba,
        ontologytermadaptorKTables,
        ontologytermadaptorKColumns,
        (const EnsPBaseadaptorLeftjoin) NULL,
        (const char *) NULL,
        (const char *) NULL,
        &ontologytermadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Ontology Term Adaptor object.
**
** @fdata [EnsPOntologytermadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Ontology Term Adaptor
**
** @argrule * Pota [EnsPOntologytermadaptor*]
** Ensembl Ontology Term Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensOntologytermadaptorDel ********************************************
**
** Default destructor for an Ensembl Ontology Term Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pota [EnsPOntologytermadaptor*]
** Ensembl Ontology Term Adaptor address
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ensOntologytermadaptorDel(EnsPOntologytermadaptor *Pota)
{
    ensBaseadaptorDel(Pota);

	return;
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Ontology Term objects from an
** Ensembl SQL database.
**
** @fdata [EnsPOntologytermadaptor]
**
** @nam3rule Fetch Fetch Ensembl Ontology Term object(s)
** @nam4rule All Fetch all Ensembl Ontology Term objects
** @nam4rule Allby Fetch all Ensembl Ontology Term objects matching a criterion
** @nam5rule Ancestor Fetch all descendant Ensembl Ontology Term objects,
** down to and including any leaf Ensembl Ontology Term objects.
** @nam5rule Child Fetch all immediate parent Ensembl Ontology Term objects
** by a child Ensembl Ontology Term
** @nam5rule Descendant Fetch all ancestor Ensembl Ontology Term objects,
** up to and including any any root Ensembl Ontology Term objects.
** @nam5rule Name Fetch all by a name
** @nam5rule Parent Fetch all immediate child Ensembl Ontology Term objects
** by a parent Ensembl Ontology Term
** @nam4rule By Fetch one Ensembl Ontology Term object matching a criterion
** @nam5rule Accession Fetch by an accession number
** @nam5rule Identifier Fetch by an SQL database internal identifier
** @nam5rule Name Fetch by a name
**
** @argrule * ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @argrule All ots [AjPList] AJAX List of Ensembl Ontology Term objects
** @argrule AllbyAncestor ot [EnsPOntologyterm] Ensembl Ontology Term
** @argrule AllbyChild ot [EnsPOntologyterm] Ensembl Ontology Term
** @argrule AllbyDescendant ot [EnsPOntologyterm] Ensembl Ontology Term
** @argrule AllbyDescendant subset [const AjPStr] Ontology subset
** @argrule AllbyDescendant closest [AjBool] Closest only
** @argrule AllbyName termname [const AjPStr] Term name
** @argrule AllbyName ontologyname [const AjPStr] Ontology name
** @argrule AllbyParent ot [EnsPOntologyterm] Ensembl Ontology Term
** @argrule Allby ots [AjPList] AJAX List of Ensembl Ontology Term objects
** @argrule ByAccession accession [const AjPStr] Accession number
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByName name [const AjPStr] Ensembl Ontology Term name
** @argrule By Pot [EnsPOntologyterm*] Ensembl Ontology Term address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensOntologytermadaptorFetchAll ***************************************
**
** Fetch all Ensembl Ontology Term objects.
**
** The caller is responsible for deleting the Ensembl Ontology Term objects
** before deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_all
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchAll(
    EnsPOntologytermadaptor ota,
    AjPList ots)
{
    if (!ota)
        return ajFalse;

    if (!ots)
        return ajFalse;

    return ensBaseadaptorFetchAll(ota, ots);
}




/* @func ensOntologytermadaptorFetchAllbyAncestor *****************************
**
** Fetch all descendant Ensembl Ontology Term objects, down to and including
** any leaf Ensembl Ontology Term objects.
**
** The caller is responsible for deleting the Ensembl Ontology Term objects
** before deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_all_by_ancestor_term
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchAllbyAncestor(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    AjPList ots)
{
    ajuint identifier = 0U;
    ajuint oid        = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr accession  = NULL;
    AjPStr definition = NULL;
    AjPStr name       = NULL;
    AjPStr statement  = NULL;
    AjPStr subsets    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntology ontology  = NULL;
    EnsPOntologyadaptor oa = NULL;

    EnsPOntologyterm pot = NULL;

    if (!ota)
        return ajFalse;

    if (!ot)
        return ajFalse;

    if (!ots)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ota);

    oa = ensRegistryGetOntologyadaptor(dba);

    statement = ajFmtStr(
        "SELECT DISTINCT "
        "child_term.term_id, "
        "child_term.ontology_id, "
        "child_term.accession, "
        "child_term.name, "
        "child_term.definition, "
        "child_term.subsets "
        "FROM "
        "term child_term "
        "JOIN "
        "closure "
        "ON "
        "(closure.child_term_id = child_term.term_id) "
        "WHERE "
        "closure.parent_term_id = %u "
        "AND "
        "closure.distance > 0 "
        "ORDER BY "
        "closure.distance, "
        "child_term.accession",
        ot->Identifier);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        oid        = 0;
        accession  = ajStrNew();
        name       = ajStrNew();
        definition = ajStrNew();
        subsets    = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &oid);
        ajSqlcolumnToStr(sqlr, &accession);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &definition);
        ajSqlcolumnToStr(sqlr, &subsets);

        ensOntologyadaptorFetchByIdentifier(oa, oid, &ontology);

        pot = ensOntologytermNewIni(ota,
                                    identifier,
                                    ontology,
                                    accession,
                                    definition,
                                    name,
                                    subsets);

        ajListPushAppend(ots, (void *) pot);

        ensOntologyDel(&ontology);

        ajStrDel(&accession);
        ajStrDel(&name);
        ajStrDel(&definition);
        ajStrDel(&subsets);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @funcstatic ontologytermListOntologytermValdel *****************************
**
** An ajTableSetDestroyvalue "valdel" function to clear AJAX Table value data.
** This function removes and deletes Ensembl Ontology Term objects
** from an AJAX List object, before deleting the AJAX List object.
**
** @param [d] Pvalue [void**] AJAX List address
** @see ajTableSetDestroyvalue
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void ontologytermListOntologytermValdel(void **Pvalue)
{
    EnsPOntologyterm ot = NULL;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    while (ajListPop(*((AjPList *) Pvalue), (void **) &ot))
        ensOntologytermDel(&ot);

    ajListFree((AjPList *) Pvalue);

    return;
}




/* @func ensOntologytermadaptorFetchAllbyChild ********************************
**
** Fetch all immediate parent Ensembl Ontology Term objects
** by a child Ensembl Ontology Term.
**
** The caller is responsible for deleting the Ensembl Ontology Term objects
** before deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_all_by_parent_term
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchAllbyChild(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    AjPList ots)
{
    void **valarray = NULL;

    register ajuint i = 0U;

    ajuint identifier = 0U;
    ajuint oid        = 0U;

    AjIList iter = NULL;
    AjPList list = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr accession  = NULL;
    AjPStr definition = NULL;
    AjPStr name       = NULL;
    AjPStr relation   = NULL;
    AjPStr statement  = NULL;
    AjPStr subsets    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntology ontology  = NULL;
    EnsPOntologyadaptor oa = NULL;

    EnsPOntologyterm pot = NULL;

    if (!ota)
        return ajFalse;

    if (!ot)
        return ajFalse;

    if (!ots)
        return ajFalse;

    if (!ot->Parents)
    {
        ot->Parents = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            ot->Parents,
            &ontologytermListOntologytermValdel);

        dba = ensBaseadaptorGetDatabaseadaptor(ota);

        oa  = ensRegistryGetOntologyadaptor(dba);

        statement = ajFmtStr(
            "SELECT "
            "parent_term.term_id, "
            "parent_term.ontology_id, "
            "parent_term.accession, "
            "parent_term.name, "
            "parent_term.definition, "
            "parent_term.subsets, "
            "relation_type.name "
            "FROM "
            "term parent_term "
            "JOIN "
            "relation "
            "ON "
            "(relation.parent_term_id = parent_term.term_id) "
            "JOIN "
            "relation_type "
            "USING "
            "(relation_type_id) "
            "WHERE "
            "relation.child_term_id = %u",
            ensOntologytermGetIdentifier(ot));

        sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

        sqli = ajSqlrowiterNew(sqls);

        while (!ajSqlrowiterDone(sqli))
        {
            identifier = 0;
            oid        = 0;
            accession  = ajStrNew();
            name       = ajStrNew();
            definition = ajStrNew();
            subsets    = ajStrNew();
            relation   = ajStrNew();

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToUint(sqlr, &identifier);
            ajSqlcolumnToUint(sqlr, &oid);
            ajSqlcolumnToStr(sqlr, &accession);
            ajSqlcolumnToStr(sqlr, &name);
            ajSqlcolumnToStr(sqlr, &definition);
            ajSqlcolumnToStr(sqlr, &subsets);
            ajSqlcolumnToStr(sqlr, &relation);

            ensOntologyadaptorFetchByIdentifier(oa, oid, &ontology);

            pot = ensOntologytermNewIni(ota,
                                        identifier,
                                        ontology,
                                        accession,
                                        definition,
                                        name,
                                        subsets);

            list = ajTableFetchmodS(ot->Parents, relation);

            if (list == NULL)
            {
                list = ajListNew();

                ajTablePut(ot->Parents,
                           (void *) ajStrNewS(relation),
                           (void *) list);
            }

            ajListPushAppend(list, (void *) pot);

            ensOntologyDel(&ontology);

            ajStrDel(&accession);
            ajStrDel(&name);
            ajStrDel(&definition);
            ajStrDel(&subsets);
            ajStrDel(&relation);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(dba, &sqls);

        ajStrDel(&statement);
    }

    ajTableToarrayValues(ot->Parents, &valarray);

    for (i = 0U; valarray[i]; i++)
    {
        iter = ajListIterNew((AjPList) valarray[i]);

        while (!ajListIterDone(iter))
        {
            pot = (EnsPOntologyterm) ajListIterGet(iter);

            ajListPushAppend(ots, (void *) ensOntologytermNewRef(pot));
        }

        ajListIterDel(&iter);
    }

    AJFREE(valarray);

    return ajTrue;
}




/* @func ensOntologytermadaptorFetchAllbyDescendant ***************************
**
** Fetch all ancestor Ensembl Ontology Term objects, up to and including any
** any root Ensembl Ontology Term objects.
**
** The caller is responsible for deleting the Ensembl Ontology Term objects
** before deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_all_by_ancestor_term
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [rN] subset [const AjPStr] The subset within the ontolgy to which
** the query should be restricted. The subset may be specified as a SQL
** pattern, e.g., "%goslim%" (but "goslim%" might not do what you expect),
** or as a specific subset name, e.g., "goslim_generic".
** @param [r] closest [AjBool] Closest only
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchAllbyDescendant(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    const AjPStr subset,
    AjBool closest,
    AjPList ots)
{
    char *txtsubset = NULL;

    ajuint identifier  = 0U;
    ajuint distance    = 0U;
    ajuint oid         = 0U;
    ajuint mindistance = 0U;

    AjBool lastrecord = ajFalse;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr accession  = NULL;
    AjPStr definition = NULL;
    AjPStr name       = NULL;
    AjPStr statement  = NULL;
    AjPStr subsets    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntology ontology  = NULL;
    EnsPOntologyadaptor oa = NULL;

    EnsPOntologyterm pot = NULL;

    if (!ota)
        return ajFalse;

    if (!ot)
        return ajFalse;

    if (!ots)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ota);

    oa  = ensRegistryGetOntologyadaptor(dba);

    statement = ajFmtStr(
        "SELECT DISTINCT "
        "parent_term.term_id, "
        "parent_term.ontology_id, "
        "parent_term.accession, "
        "parent_term.name, "
        "parent_term.definition, "
        "parent_term.subsets, "
        "closure.distance "
        "FROM "
        "term parent_term "
        "JOIN "
        "closure "
        "ON "
        "(closure.parent_term_id = parent_term.term_id) "
        "WHERE "
        "closure.child_term_id = %u "
        "AND "
        "closure.distance > 0",
        ot->Identifier);

    if (subset && ajStrGetLen(subset))
    {
        ensBaseadaptorEscapeC(ot->Adaptor, &txtsubset, subset);

        if (ajStrFindC(subset, "%") != -1)
            ajFmtPrintAppS(&statement,
                           " AND parent_term.subsets LIKE '%s'",
                           txtsubset);
        else
            ajFmtPrintAppS(&statement,
                           " AND FIND_IN_SET('%s', parent_term.subsets) > 0",
                           txtsubset);

        ajCharDel(&txtsubset);
    }

    ajStrAppendC(&statement,
                 " ORDER BY closure.distance, parent_term.accession");

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0U;
        oid        = 0U;
        accession  = ajStrNew();
        name       = ajStrNew();
        definition = ajStrNew();
        subsets    = ajStrNew();
        distance   = 0U;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &oid);
        ajSqlcolumnToStr(sqlr, &accession);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &definition);
        ajSqlcolumnToStr(sqlr, &subsets);
        ajSqlcolumnToUint(sqlr, &distance);

        mindistance = (mindistance) ? mindistance : distance;

        if ((!closest) || (distance == mindistance))
        {
            ensOntologyadaptorFetchByIdentifier(oa, oid, &ontology);

            pot = ensOntologytermNewIni(ota,
                                        identifier,
                                        ontology,
                                        accession,
                                        definition,
                                        name,
                                        subsets);

            ajListPushAppend(ots, (void *) pot);

            ensOntologyDel(&ontology);
        }
        else
            lastrecord = ajTrue;

        ajStrDel(&accession);
        ajStrDel(&name);
        ajStrDel(&definition);
        ajStrDel(&subsets);

        if (lastrecord)
            break;
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensOntologytermadaptorFetchAllbyName *********************************
**
** Fetch all Ensembl Ontology Term objects by a name.
**
** The caller is responsible for deleting the Ensembl Ontology Term objects
** before deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_all_by_name
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [r] termname [const AjPStr] Term name
** @param [rN] ontologyname [const AjPStr] Ontology name
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchAllbyName(
    EnsPOntologytermadaptor ota,
    const AjPStr termname,
    const AjPStr ontologyname,
    AjPList ots)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPStr statement = NULL;

    if (!ota)
        return ajFalse;

    if (!termname)
        return ajFalse;

    if (!ots)
        return ajFalse;

    ensBaseadaptorEscapeC(ota, &txtname, termname);

    statement = ajStrNewC(
        "SELECT "
        "DISTINCT "
        "term.term_id, "
        "term.ontology_id, "
        "term.accession, "
        "term.name, "
        "term.definition, "
        "term.subsets "
        "FROM ");

    if ((ontologyname != NULL) && (ajStrGetLen(ontologyname) > 0))
        ajStrAppendC(
            &statement,
            "ontology "
            "JOIN "
            "term "
            "USING "
            "(ontology_id) ");
    else
        ajStrAppendC(
            &statement,
            "term ");

    ajFmtPrintAppS(
        &statement,
        "LEFT JOIN "
        "synonym "
        "USING "
        "(term_id) "
        "WHERE "
        "(term.name LIKE '%s' OR synonym.name LIKE '%s')",
        txtname, txtname);

    ajCharDel(&txtname);

    if ((ontologyname != NULL) && (ajStrGetLen(ontologyname) > 0))
    {
        ensBaseadaptorEscapeC(ota, &txtname, ontologyname);
        ajFmtPrintAppS(&statement, " AND ontology.name = '%s'", txtname);
        ajCharDel(&txtname);
    }

    result = ontologytermadaptorFetchAllbyStatement(
        ota,
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        ots);

    ajStrDel(&statement);

    return result;
}




/* @func ensOntologytermadaptorFetchAllbyParent *******************************
**
** Fetch all immediate child Ensembl Ontology Term objects
** by a parent Ensembl Ontology Term.
**
** The caller is responsible for deleting the Ensembl Ontology Term objects
** before deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_all_by_parent_term
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [u] ot [EnsPOntologyterm] Ensembl Ontology Term
** @param [u] ots [AjPList] AJAX List of Ensembl Ontology Term objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchAllbyParent(
    EnsPOntologytermadaptor ota,
    EnsPOntologyterm ot,
    AjPList ots)
{
    void **valarray = NULL;

    register ajuint i = 0U;

    ajuint identifier = 0U;
    ajuint oid        = 0U;

    AjIList iter = NULL;
    AjPList list = NULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr accession  = NULL;
    AjPStr definition = NULL;
    AjPStr name       = NULL;
    AjPStr relation   = NULL;
    AjPStr statement  = NULL;
    AjPStr subsets    = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntology ontology  = NULL;
    EnsPOntologyadaptor oa = NULL;

    EnsPOntologyterm pot = NULL;

    if (!ota)
        return ajFalse;

    if (!ot)
        return ajFalse;

    if (!ots)
        return ajFalse;

    if (!ot->Children)
    {
        ot->Children = ajTablestrNew(0);

        ajTableSetDestroyvalue(
            ot->Children,
            &ontologytermListOntologytermValdel);

        dba = ensBaseadaptorGetDatabaseadaptor(ota);

        oa  = ensRegistryGetOntologyadaptor(dba);

        statement = ajFmtStr(
            "SELECT "
            "child_term.term_id, "
            "child_term.ontology_id, "
            "child_term.accession, "
            "child_term.name, "
            "child_term.definition, "
            "child_term.subsets, "
            "relation_type.name "
            "FROM "
            "term child_term "
            "JOIN "
            "relation "
            "ON "
            "(relation.child_term_id = child_term.term_id) "
            "JOIN "
            "relation_type "
            "USING "
            "(relation_type_id) "
            "WHERE "
            "relation.parent_term_id = %u",
            ensOntologytermGetIdentifier(ot));

        sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

        sqli = ajSqlrowiterNew(sqls);

        while (!ajSqlrowiterDone(sqli))
        {
            identifier = 0;
            oid        = 0;
            accession  = ajStrNew();
            name       = ajStrNew();
            definition = ajStrNew();
            subsets    = ajStrNew();
            relation   = ajStrNew();

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToUint(sqlr, &identifier);
            ajSqlcolumnToUint(sqlr, &oid);
            ajSqlcolumnToStr(sqlr, &accession);
            ajSqlcolumnToStr(sqlr, &name);
            ajSqlcolumnToStr(sqlr, &definition);
            ajSqlcolumnToStr(sqlr, &subsets);
            ajSqlcolumnToStr(sqlr, &relation);

            ensOntologyadaptorFetchByIdentifier(oa, oid, &ontology);

            pot = ensOntologytermNewIni(ota,
                                        identifier,
                                        ontology,
                                        accession,
                                        definition,
                                        name,
                                        subsets);

            list = ajTableFetchmodS(ot->Children, relation);

            if (list == NULL)
            {
                list = ajListNew();

                ajTablePut(ot->Children,
                           (void *) ajStrNewS(relation),
                           (void *) list);
            }

            ajListPushAppend(list, (void *) pot);

            ensOntologyDel(&ontology);

            ajStrDel(&accession);
            ajStrDel(&name);
            ajStrDel(&definition);
            ajStrDel(&subsets);
            ajStrDel(&relation);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(dba, &sqls);

        ajStrDel(&statement);
    }

    ajTableToarrayValues(ot->Children, &valarray);

    for (i = 0U; valarray[i]; i++)
    {
        iter = ajListIterNew((AjPList) valarray[i]);

        while (!ajListIterDone(iter))
        {
            pot = (EnsPOntologyterm) ajListIterGet(iter);

            ajListPushAppend(ots, (void *) ensOntologytermNewRef(pot));
        }

        ajListIterDel(&iter);
    }

    AJFREE(valarray);

    return ajTrue;
}




/* @func ensOntologytermadaptorFetchByAccession *******************************
**
** Fetch an Ensembl Ontology Term by its accession number.
** The caller is responsible for deleting the Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_by_accession
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [r] accession [const AjPStr] Accession number
** @param [wP] Pot [EnsPOntologyterm*] Ensembl Ontology Term address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchByAccession(
    EnsPOntologytermadaptor ota,
    const AjPStr accession,
    EnsPOntologyterm *Pot)
{
    char *txtaccession = NULL;

    AjBool result = AJFALSE;

    AjPList ots = NULL;

    AjPStr statement = NULL;

    EnsPOntologyterm ot = NULL;

    if (!ota)
        return ajFalse;

    if ((accession == NULL) || (ajStrGetLen(accession) == 0))
        return ajFalse;

    if (!Pot)
        return ajFalse;

    ensBaseadaptorEscapeC(ota, &txtaccession, accession);

    statement = ajFmtStr(
        "SELECT "
        "term.term_id, "
        "term.ontology_id, "
        "term.accession, "
        "term.name, "
        "term.definition, "
        "term.subsets "
        "FROM "
        "term "
        "WHERE "
        "term.accession = '%s'",
        txtaccession);

    ajCharDel(&txtaccession);

    ots = ajListNew();

    result = ontologytermadaptorFetchAllbyStatement(
        ota,
        statement,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        ots);

    if (ajListGetLength(ots) > 1)
        ajDebug("ensOntologytermadaptorFetchByAccession got more than one "
                "Ensembl Ontology Term for accession '%S'.\n",
                accession);

    ajListPop(ots, (void **) Pot);

    while (ajListPop(ots, (void **) &ot))
        ensOntologytermDel(&ot);

    ajListFree(&ots);

    ajStrDel(&statement);

    return result;
}




/* @func ensOntologytermadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Ontology Term by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Ontology Term.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::fetch_by_dbID
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pot [EnsPOntologyterm*] Ensembl Ontology Term address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorFetchByIdentifier(
    EnsPOntologytermadaptor ota,
    ajuint identifier,
    EnsPOntologyterm *Pot)
{
    if (!ota)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Pot)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(ota, identifier, (void **) Pot);
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Ontology Term objects
** from an Ensembl SQL database.
**
** @fdata [EnsPOntologytermadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Ontology Term-releated object(s)
** @nam4rule All Retrieve all Ensembl Ontology Term-releated objects
** @nam5rule Ontologysynonyms Fetch all Ensembl Ontology Synonym objects
**
** @argrule * ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @argrule Ontologysynonyms ot [const EnsPOntologyterm] Ensembl Ontology Term
** @argrule Ontologysynonyms oss [AjPList]
** AJAX List of Ensembl Ontology Synonym objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensOntologytermadaptorRetrieveAllOntologysynonyms ********************
**
** Retrieve all Ensembl Ontology Synonym objects of Ensembl Ontology Term.
**
** The caller is responsible for deleting the Ensembl Ontology Synonym objects
** before deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::OntologyTermAdaptor::_fetch_synonyms_by_dbID
** @param [u] ota [EnsPOntologytermadaptor] Ensembl Ontology Term Adaptor
** @param [r] ot [const EnsPOntologyterm] Ensembl Ontology Term
** @param [u] oss [AjPList] AJAX List of Ensembl Ontology Synonym objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensOntologytermadaptorRetrieveAllOntologysynonyms(
    EnsPOntologytermadaptor ota,
    const EnsPOntologyterm ot,
    AjPList oss)
{
    ajuint identifier = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr name      = NULL;
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPOntologysynonym os = NULL;

    if (!ota)
        return ajFalse;

    if (!ot)
        return ajFalse;

    if (!oss)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ota);

    statement = ajFmtStr(
        "SELECT "
        "synonym.name "
        "FROM "
        "synonym "
        "WHERE "
        "synonym.term_id = %u",
        ensOntologytermGetIdentifier(ot));

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        name       = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &name);

        os = ensOntologysynonymNewIni(ota, identifier, name);

        ajListPushAppend(oss, (void *) os);

        ajStrDel(&name);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}
