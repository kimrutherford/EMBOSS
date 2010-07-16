/******************************************************************************
** @source Ensembl Genetic Variation Individual functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @version $Revision: 1.10 $
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

#include "ensgvindividual.h"




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* gvindividualGender *********************************************************
**
** The Ensembl Genetic Variation Individual gender element is enumerated in
** both, the SQL table definition and the data structure. The following
** strings are used for conversion in database operations and correspond to
** EnsEGvindividualGender.
**
******************************************************************************/

static const char *gvindividualGender[] =
{
    NULL,
    "Male",
    "Female",
    "Unknown",
    NULL
};




/* gvindividualType ***********************************************************
**
** The Ensembl Genetic Variation Individual type element is enumerated in
** both, the SQL table definition and the data structure. The following
** strings are used for conversion in database operations and correspond to
** EnsEGvindividualType.
**
******************************************************************************/

static const char *gvindividualType[] =
{
    NULL,
    "Fully_inbred",
    "Partly_inbred",
    "Outbred",
    "Mutant",
    "Unknown",
    NULL
};




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool gvindividualadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                               const AjPStr statement,
                                               EnsPAssemblymapper am,
                                               EnsPSlice slice,
                                               AjPList gvis);




/* @filesection ensgvindividual ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvindividual] Genetic Variation Individual ***************
**
** Functions for manipulating Ensembl Genetic Variation Individual objects
**
** @cc Bio::EnsEMBL::Variation::Individual CVS Revision: 1.9
**
** @nam2rule Gvindividual
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Individual by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Individual. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvindividual]
** @fnote None
**
** @nam3rule New Constructor
** @nam4rule NewObj Constructor with existing object
** @nam4rule NewRef Constructor by incrementing the reference counter
**
** @argrule Obj object [const EnsPGvindividual] Ensembl Genetic Variation
**                                              Individual
** @argrule Ref gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @valrule * [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @fcategory new
******************************************************************************/




/* @func ensGvindividualNew ***************************************************
**
** Default constructor for an Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @cc Bio::EnsEMBL::Variation::Individual::new
** @param [u] father [EnsPGvindividual] Father
** @param [u] mother [EnsPGvindividual] Mother
** @param [u] gender [EnsEGvindividualGender] Gender
** @param [u] type [EnsEGvindividualType] Type
** @param [u] description [AjPStr] Description
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual or NULL
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualNew(EnsPGvindividualadaptor gvia,
                                    ajuint identifier,
                                    EnsPGvsample gvs,
                                    EnsPGvindividual father,
                                    EnsPGvindividual mother,
                                    EnsEGvindividualGender gender,
                                    EnsEGvindividualType type,
                                    AjPStr description)
{
    EnsPGvindividual gvi = NULL;

    if(!gvs)
        return NULL;

    AJNEW0(gvi);

    gvi->Use = 1;

    gvi->Identifier = identifier;

    gvi->Adaptor = gvia;

    gvi->Gvsample = ensGvsampleNewRef(gvs);

    gvi->Father = ensGvindividualNewRef(father);

    gvi->Mother = ensGvindividualNewRef(mother);

    gvi->Gender = gender;

    gvi->Type = type;

    if(description)
        gvi->Description = ajStrNewRef(description);

    return gvi;
}




/* @func ensGvindividualNewObj ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] object [const EnsPGvindividual] Ensembl Genetic Variation
**                                            Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual or NULL
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualNewObj(const EnsPGvindividual object)
{
    EnsPGvindividual gvi = NULL;

    if(!object)
        return NULL;

    AJNEW0(gvi);

    gvi->Use = 1;

    gvi->Identifier = object->Identifier;

    gvi->Adaptor = object->Adaptor;

    gvi->Gvsample = ensGvsampleNewRef(object->Gvsample);

    gvi->Father = ensGvindividualNewRef(object->Father);

    gvi->Mother = ensGvindividualNewRef(object->Mother);

    gvi->Gender = object->Gender;

    gvi->Type = object->Type;

    if(object->Description)
        gvi->Description = ajStrNewRef(object->Description);

    return gvi;
}




/* @func ensGvindividualNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualNewRef(EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    gvi->Use++;

    return gvi;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the Ensembl Genetic Variation Individual.
**
** @fdata [EnsPGvindividual]
** @fnote None
**
** @nam3rule Del Destroy (free) a Genetic Variation Individual object
**
** @argrule * Pgvi [EnsPGvindividual*] Genetic Variation Individual
**                                     object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvindividualDel ***************************************************
**
** Default destructor for an Ensembl Genetic Variation Individual.
**
** @param [d] Pgvi [EnsPGvindividual*] Ensembl Genetic Variation Individual
**                                     address
**
** @return [void]
** @@
******************************************************************************/

void ensGvindividualDel(EnsPGvindividual *Pgvi)
{
    EnsPGvindividual pthis = NULL;

    if(!Pgvi)
        return;

    if(!*Pgvi)
        return;

    if(ajDebugTest("ensGvindividualDel"))
    {
        ajDebug("ensGvindividualDel\n"
                "  *Pgvi %p\n",
                *Pgvi);

        ensGvindividualTrace(*Pgvi, 1);
    }

    pthis = *Pgvi;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvi = NULL;

        return;
    }

    ensGvsampleDel(&pthis->Gvsample);

    ensGvindividualDel(&pthis->Father);

    ensGvindividualDel(&pthis->Mother);

    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pgvi = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Individual attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Individual Adaptor
** @nam4rule GetIdentifier Return the SQL database-internal identifier
** @nam4rule GetGvsample Return the Ensembl Genetic Variation Sample
** @nam4rule GetFather Return the father Ensembl Genetic Variation Individual
** @nam4rule GetMother Return the mother Ensembl Genetic Variation Individual
** @nam4rule GetGender Return the gender
** @nam4rule GetType Return the type
**
** @argrule * gvi [const EnsPGvindividual] Genetic Variation Individual
**
** @valrule Adaptor [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                            Individual Adaptor
** @valrule Identifier [ajuint] SQL database-internal identifier
** @valrule Gvsample [EnsPGvsample] Ensembl Genetic Variation Sample
** @valrule Father [EnsPGvindividual] Ensembl Genetic Variation Individual
** @valrule Mother [EnsPGvindividual] Ensembl Genetic Variation Individual
** @valrule Gender [EnsEGvindividualGender] Gender
** @valrule Type [EnsEGvindividualType] Type
** @valrule Description [AjPStr] Description
**
** @fcategory use
******************************************************************************/




/* @func ensGvindividualGetAdaptor ********************************************
**
** Get the Ensembl Genetic Variation Individual Adaptor element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                   Individual Adaptor
** @@
******************************************************************************/

EnsPGvindividualadaptor ensGvindividualGetAdaptor(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Adaptor;
}




/* @func ensGvindividualGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [ajuint] Internal database identifier
** @@
******************************************************************************/

ajuint ensGvindividualGetIdentifier(const EnsPGvindividual gvi)
{
    if(!gvi)
        return 0;

    return gvi->Identifier;
}




/* @func ensGvindividualGetGvsample *******************************************
**
** Get the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample
** @@
******************************************************************************/

EnsPGvsample ensGvindividualGetGvsample(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Gvsample;
}




/* @func ensGvindividualGetFather *********************************************
**
** Get the father Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualGetFather(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Father;
}




/* @func ensGvindividualGetMother *********************************************
**
** Get the mother Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualGetMother(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Mother;
}




/* @func ensGvindividualGetGender *********************************************
**
** Get the gender element of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsEGvindividualGender] Gender or ensEGvindividualGenderNULL
** @@
******************************************************************************/

EnsEGvindividualGender ensGvindividualGetGender(const EnsPGvindividual gvi)
{
    if(!gvi)
        return ensEGvindividualGenderNULL;

    return gvi->Gender;
}




/* @func ensGvindividualGetType ***********************************************
**
** Get the type element of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsEGvindividualType] Type or ensEGvindividualTypeNULL
** @@
******************************************************************************/

EnsEGvindividualType ensGvindividualGetType(const EnsPGvindividual gvi)
{
    if(!gvi)
        return ensEGvindividualTypeNULL;

    return gvi->Type;
}




/* @func ensGvindividualGetDescription ****************************************
**
** Get the description element of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [AjPStr] Description
** @@
******************************************************************************/

AjPStr ensGvindividualGetDescription(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Description;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
** @fnote None
**
** @nam3rule Set Set one element of a Genetic Variation Population
** @nam4rule SetAdaptor Set the Ensembl Genetic Variation Population Adaptor
** @nam4rule SetIdentifier Set the SQL database-internal identifier
** @nam4rule SetName Set the name
** @nam4rule SetSample Set the Ensembl Genetic Variation Sample
** @nam4rule SetFather Set the Ensembl Genetic Variation Individual
** @nam4rule SetMother Set the Ensembl Genetic Variation Individual
** @nam4rule SetGender Set the gender
** @nam4rule SetType Set the type
** @nam4rule SetDescription Set the description
**
** @argrule * gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
**                                   object
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvindividualSetAdaptor ********************************************
**
** Set the Ensembl Genetic Variation Individual Adaptor element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetAdaptor(EnsPGvindividual gvi,
                                 EnsPGvindividualadaptor gvia)
{
    if(!gvi)
        return ajFalse;

    gvi->Adaptor = gvia;

    return ajTrue;
}




/* @func ensGvindividualSetIdentifier *****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetIdentifier(EnsPGvindividual gvi,
                                    ajuint identifier)
{
    if(!gvi)
        return ajFalse;

    gvi->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvindividualSetGvsample *******************************************
**
** Set the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetGvsample(EnsPGvindividual gvi,
                                  EnsPGvsample gvs)
{
    if(!gvi)
        return ajFalse;

    ensGvsampleDel(&gvi->Gvsample);

    gvi->Gvsample = ensGvsampleNewRef(gvs);

    return ajTrue;
}




/* @func ensGvindividualSetFather *********************************************
**
** Set the father Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] father [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetFather(EnsPGvindividual gvi,
                                EnsPGvindividual father)
{
    if(!gvi)
        return ajFalse;

    ensGvindividualDel(&gvi->Father);

    gvi->Father = ensGvindividualNewRef(father);

    return ajTrue;
}




/* @func ensGvindividualSetMother *********************************************
**
** Set the mother Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] mother [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetMother(EnsPGvindividual gvi,
                                EnsPGvindividual mother)
{
    if(!gvi)
        return ajFalse;

    ensGvindividualDel(&gvi->Mother);

    gvi->Mother = ensGvindividualNewRef(mother);

    return ajTrue;
}




/* @func ensGvindividualSetGender *********************************************
**
** Set the gender element of an Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] gender [EnsEGvindividualGender] Gender
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetGender(EnsPGvindividual gvi,
                                EnsEGvindividualGender gender)
{
    if(!gvi)
        return ajFalse;

    gvi->Gender = gender;

    return ajTrue;
}




/* @func ensGvindividualSetType ***********************************************
**
** Set the type element of an Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] type [EnsEGvindividualType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetType(EnsPGvindividual gvi,
                              EnsEGvindividualType type)
{
    if(!gvi)
        return ajFalse;

    gvi->Type = type;

    return ajTrue;
}




/* @func ensGvindividualSetDescription ****************************************
**
** Set the description element of an Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetDescription(EnsPGvindividual gvi,
                                     AjPStr description)
{
    if(!gvi)
        return ajFalse;

    ajStrDel(&gvi->Description);

    if(description)
        gvi->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvindividualGenderFromStr *****************************************
**
** Convert an AJAX String into an Ensembl Genetic Variaton Individual
** gender element.
**
** @param [r] gender [const AjPStr] Gender string
**
** @return [EnsEGvindividualGender] Ensembl Genetic Variation Individual gender
**                                  or ensEGvindividualGenderNULL
** @@
******************************************************************************/

EnsEGvindividualGender ensGvindividualGenderFromStr(const AjPStr gender)
{
    register EnsEGvindividualGender i = ensEGvindividualGenderNULL;

    EnsEGvindividualGender egender = ensEGvindividualGenderNULL;

    for(i = ensEGvindividualGenderMale; gvindividualGender[i]; i++)
        if(ajStrMatchC(gender, gvindividualGender[i]))
            egender = i;

    if(!egender)
        ajDebug("ensGvindividualGenderFromStr encountered "
                "unexpected string '%S'.\n", gender);

    return egender;
}




/* @func ensGvindividualTypeFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Genetic Variaton Individual
** type element.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEGvindividualType] Ensembl Genetic Variation Individual type
**                                or ensEGvindividualTypeNULL
** @@
******************************************************************************/

EnsEGvindividualType ensGvindividualTypeFromStr(const AjPStr type)
{
    register EnsEGvindividualType i = ensEGvindividualTypeNULL;

    EnsEGvindividualType etype = ensEGvindividualTypeNULL;

    for(i = ensEGvindividualTypeFullyInbred; gvindividualType[i]; i++)
        if(ajStrMatchC(type, gvindividualType[i]))
            etype = i;

    if(!etype)
        ajDebug("ensGvindividualTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @func ensGvindividualGenderToChar ******************************************
**
** Convert an Ensembl Genetic Variation Individual gender element into a
** C-type (char*) string.
**
** @param [r] gender [EnsEGvindividualGender] Ensembl Genetic Variation
**                                            Individual gender
**
** @return [const char*] Ensembl Genetic Variation Individual gender
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvindividualGenderToChar(EnsEGvindividualGender gender)
{
    register EnsEGvindividualGender i = ensEGvindividualGenderNULL;

    if(!gender)
        return NULL;

    for(i = ensEGvindividualGenderMale;
        gvindividualGender[i] && (i < gender);
        i++);

    if(!gvindividualGender[i])
        ajDebug("ensGvindividualGenderToChar encountered an "
                "out of boundary error on gender %d.\n", gender);

    return gvindividualGender[i];
}




/* @func ensGvindividualTypeToChar ********************************************
**
** Convert an Ensembl Genetic Variation Individual type element into a
** C-type (char*) string.
**
** @param [r] type [EnsEGvindividualType] Ensembl Genetic Variation
**                                        Individual type
**
** @return [const char*] Ensembl Genetic Variation Individual type
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvindividualTypeToChar(EnsEGvindividualType type)
{
    register EnsEGvindividualType i = ensEGvindividualTypeNULL;

    if(!type)
        return NULL;

    for(i = ensEGvindividualTypeFullyInbred;
        gvindividualType[i] && (i < type);
        i++);

    if(!gvindividualType[i])
        ajDebug("ensGvindividualTypeToChar encountered an "
                "out of boundary error on type %d.\n", type);

    return gvindividualType[i];
}




/* @func ensGvindividualGetMemsize ********************************************
**
** Get the memory size in bytes of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [ajulong] Memory size
** @@
******************************************************************************/

ajulong ensGvindividualGetMemsize(const EnsPGvindividual gvi)
{
    ajulong size = 0;

    if(!gvi)
        return 0;

    size += sizeof (EnsOGvindividual);

    size += ensGvsampleGetMemsize(gvi->Gvsample);

    size += ensGvindividualGetMemsize(gvi->Father);

    size += ensGvindividualGetMemsize(gvi->Mother);

    if(gvi->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvi->Description);
    }

    return size;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
** @nam3rule Trace Report Ensembl Genetic Variation Individual elements to
**                 debug file
**
** @argrule Trace gvi [const EnsPGvindividual] Ensembl Genetic Variation
**                                             Individual
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvindividualTrace *************************************************
**
** Trace an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualTrace(const EnsPGvindividual gvi, ajuint level)
{
    AjPStr indent = NULL;

    if(!gvi)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvindividualTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Sample %p\n"
            "%S  Father %p\n"
            "%S  Mother %p\n"
            "%S  Gender '%s'\n"
            "%S  Type '%s'\n"
            "%S  Description '%S'\n",
            indent, gvi,
            indent, gvi->Use,
            indent, gvi->Identifier,
            indent, gvi->Adaptor,
            indent, gvi->Gvsample,
            indent, gvi->Father,
            indent, gvi->Mother,
            indent, ensGvindividualGenderToChar(gvi->Gender),
            indent, ensGvindividualTypeToChar(gvi->Type),
            indent, gvi->Description);

    ensGvsampleTrace(gvi->Gvsample, level + 1);

    ensGvindividualTrace(gvi->Father, level + 1);
    ensGvindividualTrace(gvi->Mother, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @datasection [EnsPGvindividualadaptor] Genetic Variation Individual Adaptor
**
** Functions for manipulating Ensembl Genetic Variation Individual Adaptor
** objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor CVS Revision: 1.15
**
** @nam2rule Gvindividualadaptor
**
******************************************************************************/

static const char *gvindividualadaptorTables[] =
{
    "sample",
    "individual",
    "individual_type",
    NULL
};




static const char *gvindividualadaptorColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "individual.gender",
    "individual.father_individual_sample_id",
    "individual.mother_individual_sample_id",
    "individual_type.name",
    "individual_type.description",
    NULL
};




static EnsOBaseadaptorLeftJoin gvindividualadaptorLeftJoin[] =
{
    {NULL, NULL}
};




static const char *gvindividualadaptorDefaultCondition =
    "sample.sample_id = individual.sample_id "
    "AND "
    "individual.individual_type_id = individual_type.individual_type_id";




static const char *gvindividualadaptorFinalCondition = NULL;




/* @funcstatic gvindividualadaptorFetchAllBySQL *******************************
**
** Fetch all Ensembl Genetic Variation Individual objects via an SQL statement.
**
** @param [r] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvindividualadaptorFetchAllBySQL(EnsPDatabaseadaptor dba,
                                               const AjPStr statement,
                                               EnsPAssemblymapper am,
                                               EnsPSlice slice,
                                               AjPList gvis)
{
    ajuint sidentifier = 0;
    ajuint fatherid    = 0;
    ajuint motherid    = 0;
    ajuint ssize       = 0;

    EnsEGvsampleDisplay esdisplay  = ensEGvsampleDisplayNULL;
    EnsEGvindividualGender egender = ensEGvindividualGenderNULL;
    EnsEGvindividualType etype     = ensEGvindividualTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr sname        = NULL;
    AjPStr sdescription = NULL;
    AjPStr sdisplay     = NULL;
    AjPStr gender       = NULL;
    AjPStr tname        = NULL;
    AjPStr tdescription = NULL;

    EnsPGvindividual gvi         = NULL;
    EnsPGvindividual father      = NULL;
    EnsPGvindividual mother      = NULL;
    EnsPGvindividualadaptor gvia = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(ajDebugTest("gvindividualadaptorFetchAllBySQL"))
        ajDebug("gvindividualadaptorFetchAllBySQL\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvis %p\n",
                dba,
                statement,
                am,
                slice,
                gvis);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    gvia = ensRegistryGetGvindividualadaptor(dba);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        sidentifier  = 0;
        sname        = ajStrNew();
        ssize        = 0;
        sdescription = ajStrNew();
        sdisplay     = ajStrNew();
        gender       = ajStrNew();
        fatherid     = 0;
        motherid     = 0;
        tname        = ajStrNew();
        tdescription = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &sidentifier);
        ajSqlcolumnToStr(sqlr, &sname);
        ajSqlcolumnToUint(sqlr, &ssize);
        ajSqlcolumnToStr(sqlr, &sdescription);
        ajSqlcolumnToStr(sqlr, &sdisplay);
        ajSqlcolumnToStr(sqlr, &gender);
        ajSqlcolumnToUint(sqlr, &fatherid);
        ajSqlcolumnToUint(sqlr, &motherid);
        ajSqlcolumnToStr(sqlr, &tname);
        ajSqlcolumnToStr(sqlr, &tdescription);

        esdisplay = ensGvsampleDisplayFromStr(sdisplay);
        egender   = ensGvindividualGenderFromStr(gender);
        etype     = ensGvindividualTypeFromStr(tname);

        gvs = ensGvsampleNew(gvsa,
                             sidentifier,
                             sname,
                             sdescription,
                             esdisplay,
                             ssize);

        ensGvindividualadaptorFetchByIdentifier(gvia, fatherid, &father);

        ensGvindividualadaptorFetchByIdentifier(gvia, motherid, &mother);

        gvi = ensGvindividualNew(gvia,
                                 sidentifier,
                                 gvs,
                                 father,
                                 mother,
                                 egender,
                                 etype,
                                 tdescription);

        ajListPushAppend(gvis, (void *) gvi);

        ensGvsampleDel(&gvs);

        ensGvindividualDel(&father);
        ensGvindividualDel(&mother);

        ajStrDel(&sname);
        ajStrDel(&sdescription);
        ajStrDel(&sdisplay);
        ajStrDel(&gender);
        ajStrDel(&tname);
        ajStrDel(&tdescription);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @func ensGvindividualadaptorNew ********************************************
**
** Default constructor for an Ensembl Genetic Variation Individual Adaptor.
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
** @see ensRegistryGetGvindividualadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::Individualadaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                   Individual Adaptor or NULL
** @@
******************************************************************************/

EnsPGvindividualadaptor ensGvindividualadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        gvindividualadaptorTables,
        gvindividualadaptorColumns,
        gvindividualadaptorLeftJoin,
        gvindividualadaptorDefaultCondition,
        gvindividualadaptorFinalCondition,
        gvindividualadaptorFetchAllBySQL);
}




/* @func ensGvindividualadaptorDel ********************************************
**
** Default destructor for an Ensembl Gentic Variation Individual Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvi [EnsPGvindividualadaptor*] Ensembl Genetic Variation
**                                             Individual Adaptor address
**
** @return [void]
** @@
******************************************************************************/

void ensGvindividualadaptorDel(EnsPGvindividualadaptor *Pgvi)
{
    if(!Pgvi)
        return;

    ensGvsampleadaptorDel(Pgvi);

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Individual Adaptor object.
**
** @fdata [EnsPGvindividualadaptor]
** @fnote None
**
** @nam3rule Get Return Genetic Variation Individual Adaptor attribute(s)
** @nam4rule GetAdaptor Return the Ensembl Genetic Variation Sample Adaptor
**
** @argrule * gvia [const EnsPGvindividualadaptor] Genetic Variation
**                                                 Individual Adaptor
**
** @valrule Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
**
** @fcategory use
******************************************************************************/




/* @func ensGvindividualadaptorGetAdaptor *************************************
**
** Get the Ensembl Genetic Variation Sample Adaptor element of an
** Ensembl Genetic Variation Individual Adaptor.
**
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

EnsPBaseadaptor ensGvindividualadaptorGetAdaptor(
    EnsPGvindividualadaptor gvia)
{
    if(!gvia)
        return NULL;

    return gvia;
}




/* @func ensGvindividualadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Genetic Variation Individual via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::fetch_by_dbID
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [r] identifier [ajuint] SQL database-internal Individual identifier
** @param [wP] Pgvi [EnsPGvindividual*] Ensembl Genetic Variation
**                                      Individual address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchByIdentifier(
    const EnsPGvindividualadaptor gvia,
    ajuint identifier,
    EnsPGvindividual *Pgvi)
{
    if(!gvia)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvi)
        return ajFalse;

    *Pgvi = (EnsPGvindividual)
        ensBaseadaptorFetchByIdentifier(gvia, identifier);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllByName *********************************
**
** Fetch all Ensembl Genetic Variation Individuals by an
** Ensembl Genetic Variation Sample name.
**
** Individual names may be non-unique, therefore the function returns an
** AJAX List of Ensembl Genetic Variation Individuals.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::fetch_all_by_name
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Sample name
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individuals
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllByName(EnsPGvindividualadaptor gvia,
                                            const AjPStr name,
                                            AjPList gvis)
{
    char *txtname = NULL;

    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    ensBaseadaptorEscapeC(gvia, &txtname, name);

    constraint = ajFmtStr("sample.name = '%s'", txtname);

    ajCharDel(&txtname);

    ensBaseadaptorGenericFetch(gvia,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               gvis);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllBySynonym ******************************
**
** Fetch all Ensembl Genetic Variation Individuals by an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_individual_by_synonym
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individuals
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllBySynonym(
    const EnsPGvindividualadaptor gvia,
    const AjPStr synonym,
    AjPList gvis)
{
    ajuint *Pidentifier = 0;

    AjPList idlist = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvindividual gvi = NULL;

    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvia)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    idlist = ajListNew();

    ensGvsampleadaptorFetchAllIdentifiersBySynonym(gvsa,
                                                   synonym,
                                                   (AjPStr) NULL,
                                                   idlist);

    while(ajListPop(idlist, (void **) &Pidentifier))
    {
        ensGvindividualadaptorFetchByIdentifier(gvia,
                                                *Pidentifier,
                                                &gvi);

        ajListPushAppend(gvis, (void *) gvi);

        AJFREE(Pidentifier);
    }

    ajListFree(&idlist);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllByPopulation ***************************
**
** Fetch all Ensembl Genetic Variation Individuals by an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_all_by_Population
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individuals
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllByPopulation(
    const EnsPGvindividualadaptor gvia,
    const EnsPGvpopulation gvp,
    AjPList gvis)
{
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    if(!gvp->Identifier)
    {
        ajDebug("ensGvindividualadaptorFetchAllByPopulation "
                "cannot fetch Population for Individual "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** This function does not use the Ensembl Base Adaptor functionality
    ** because an additional 'population_structure' table is required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.description, "
        "sample.display, "
        "individual.gender, "
        "individual.father_individual_sample_id, "
        "individual.mother_individual_sample_id, "
        "individual_type.name, "
        "individual_type.description "
        "FROM "
        "individual, "
        "individual_population, "
        "sample, "
        "individual_type "
        "WHERE "
        "individual.sample_id = "
        "individual_population.individual_sample_id "
        "AND "
        "individual.sample_id = sample.sample_id "
        "AND "
        "individual.individual_type_id = "
        "individual_type.individual_type_id "
        "AND "
        "individual_population.population_sample_id = %u",
        gvp->Identifier);

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    gvindividualadaptorFetchAllBySQL(dba,
                                     statement,
                                     (EnsPAssemblymapper) NULL,
                                     (EnsPSlice) NULL,
                                     gvis);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllByParentIndividual *********************
**
** Fetch all Ensembl Genetic Variation Individuals by a parent
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_all_by_parent_Individual
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [r] parent [const EnsPGvindividual] Ensembl Genetic Variation
**                                            Individual
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individuals
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllByParentIndividual(
    const EnsPGvindividualadaptor gvia,
    const EnsPGvindividual parent,
    AjPList gvis)
{
    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!parent)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    if(!parent->Identifier)
    {
        ajDebug("ensGvindividualadaptorFetchAllByParentIndividual cannot get "
                "Individuals for parent Indeividual without identifier.\n");

        return ajFalse;
    }

    if(parent->Gender == ensEGvindividualGenderMale)
    {
        constraint = ajFmtStr("individual.father_individual_sample_id = %u",
                              parent->Identifier);

        ensBaseadaptorGenericFetch(gvia,
                                   constraint,
                                   (EnsPAssemblymapper) NULL,
                                   (EnsPSlice) NULL,
                                   gvis);

        ajStrDel(&constraint);

        return ajTrue;
    }
    else if(parent->Gender == ensEGvindividualGenderFemale)
    {
        constraint = ajFmtStr("individual.mother_individual_sample_id = %u",
                              parent->Identifier);

        ensBaseadaptorGenericFetch(gvia,
                                   constraint,
                                   (EnsPAssemblymapper) NULL,
                                   (EnsPSlice) NULL,
                                   gvis);

        ajStrDel(&constraint);

        return ajTrue;
    }

    /* The gender is unknown, so assume this is the father. */

    constraint = ajFmtStr("individual.father_individual_sample_id = %u",
                          parent->Identifier);

    ensBaseadaptorGenericFetch(gvia,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               gvis);

    ajStrDel(&constraint);

    if(ajListGetLength(gvis))
        return ajTrue;

    /* No result for the father, so assume it is the mother. */

    constraint = ajFmtStr("individual.mother_individual_sample_id = %u",
                          parent->Identifier);

    ensBaseadaptorGenericFetch(gvia,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               gvis);

    ajStrDel(&constraint);

    /* Return regardless of a result. */

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllStrains ********************************
**
** Fetch all Ensembl Genetic Variation Individuals by a parent
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::fetch_all_strains
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individuals
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllStrains(
    const EnsPGvindividualadaptor gvia,
    AjPList gvis)
{
    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    constraint = ajStrNewC("individual_type.name = 'fully_inbred'");

    ensBaseadaptorGenericFetch(gvia,
                               constraint,
                               (EnsPAssemblymapper) NULL,
                               (EnsPSlice) NULL,
                               gvis);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchReferenceStrainName ***********************
**
** Fetch the reference strain name.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     get_reference_strain_name
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [w] Pname [AjPStr*] Reference strain name AJAX String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchReferenceStrainName(
    const EnsPGvindividualadaptor gvia,
    AjPStr *Pname)
{
    AjPList gvss = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvia)
        return ajFalse;

    if(!Pname)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    gvss = ajListNew();

    ensGvsampleadaptorFetchAllByDisplay(gvsa,
                                        ensEGvsampleDisplayReference,
                                        gvss);

    /* There should be only one reference strain. */

    while(ajListPop(gvss, (void **) &gvs))
    {
        ajStrAssignS(Pname, ensGvsampleGetName(gvs));

        ensGvsampleDel(&gvs);
    }

    ajListFree(&gvss);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllDefaultStrains *************************
**
** Fetch all default strain names.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     get_default_strains
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [u] names [AjPList] AJAX List of default strain name AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllDefaultStrains(
    const EnsPGvindividualadaptor gvia,
    AjPList names)
{
    AjPList gvss = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvia)
        return ajFalse;

    if(!names)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    gvss = ajListNew();

    ensGvsampleadaptorFetchAllByDisplay(gvsa,
                                        ensEGvsampleDisplayDefault,
                                        gvss);

    while(ajListPop(gvss, (void **) &gvs))
    {
        ajListPushAppend(names,
                         (void *) ajStrNewS(ensGvsampleGetName(gvs)));

        ensGvsampleDel(&gvs);
    }

    ajListFree(&gvss);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllDisplayStrains *************************
**
** Fetch all display strain names.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     get_display_strains
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [u] names [AjPList] AJAX List of display strain name AJAX Strings
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllDisplayStrains(
    const EnsPGvindividualadaptor gvia,
    AjPList names)
{
    AjPList gvss = NULL;

    AjPStr name      = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvia)
        return ajFalse;

    if(!names)
        return ajFalse;

    name = ajStrNew();

    /* Fetch the reference strain name. */

    ensGvindividualadaptorFetchReferenceStrainName(gvia, &name);

    ajListPushAppend(names, (void *) name);

    /* Fetch all default strain names. */

    ensGvindividualadaptorFetchAllDefaultStrains(gvia, names);

    /* Fetch all display strain names. */

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    gvss = ajListNew();

    ensGvsampleadaptorFetchAllByDisplay(gvsa,
                                        ensEGvsampleDisplayDisplayable,
                                        gvss);

    while(ajListPop(gvss, (void **) &gvs))
    {
        ajListPushAppend(names,
                         (void *) ajStrNewS(ensGvsampleGetName(gvs)));

        ensGvsampleDel(&gvs);
    }

    ajListFree(&gvss);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllStrainsWithCoverage ********************
**
** Fetch all Ensembl Genetic Variation Strain identifiers for which
** read coverage information is available.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_all_strains_with_coverage
** @param [r] gvia [const EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                                 Individual Adaptor
** @param [u] idlist [AjPList] AJAX List of Ensembl Genetic Variation Sample
**                             identifiers
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllStrainsWithCoverage(
    const EnsPGvindividualadaptor gvia,
    AjPList idlist)
{
    ajuint *Pidentifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvia)
        return ajFalse;

    if(!idlist)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    statement = ajStrNewC(
        "SELECT "
        "DISTINCT "
        "read_coverage.sample_id "
        "FROM "
        "read_coverage");

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

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}
