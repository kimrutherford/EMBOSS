
#ifndef ENSSTORABLE_H
#define ENSSTORABLE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdatabaseconnection.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsEStorableType ****************************************************
**
** Ensembl Storable Type enumeration.
**
******************************************************************************/

typedef enum EnsOStorableType
{
    ensEStorableTypeNULL,
    ensEStorableTypeAnalysis,
    ensEStorableTypeRepeatconsensus
} EnsEStorableType;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPStorable *********************************************************
**
** Ensembl Storable.
**
** @alias EnsSStorable
** @alias EnsOStorable
**
** @attr Use [ajuint] Use counter
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [void*] Ensembl Object Adaptor
** @attr Type [EnsEStorableType] Ensembl Storable Object Type
** @attr Padding [ajuint] Padding to alignment boundary
**
** @@
******************************************************************************/

typedef struct EnsSStorable
{
    ajuint Use;
    ajuint Identifier;
    void* Adaptor;
    EnsEStorableType Type;
    ajuint Padding;
} EnsOStorable;

#define EnsPStorable EnsOStorable*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

EnsPStorable ensStorableNewCpy(const EnsPStorable storable);

EnsPStorable ensStorableNewIni(EnsEStorableType type,
                               ajuint identifier,
                               void* adaptor);

EnsPStorable ensStorableNewRef(EnsPStorable storable);

void ensStorableDel(EnsPStorable* Pstorable);

void* ensStorableGetAdaptor(const EnsPStorable storable);

ajuint ensStorableGetIdentifier(const EnsPStorable storable);

EnsEStorableType ensStorableGetType(const EnsPStorable storable);

AjBool ensStorableSetIdentifier(EnsPStorable storable, ajuint identifier);

AjBool ensStorableSetAdaptor(EnsPStorable storable, void* adaptor);

AjBool ensStorableIsStored(const EnsPStorable storable,
                           const EnsPDatabaseconnection dbc);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSTORABLE_H */
