#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensstorable_h
#define ensstorable_h

#include "ajax.h"
#include "ensdatabaseconnection.h"




/* EnsEStorableType ***********************************************************
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
    void *Adaptor;
    EnsEStorableType Type;
    ajuint Padding;
} EnsOStorable;

#define EnsPStorable EnsOStorable*




/*
** Prototype definitions
*/

EnsPStorable ensStorableNew(EnsEStorableType type,
                            ajuint identifier,
                            void* adaptor);

EnsPStorable ensStorableNewObj(const EnsPStorable object);

EnsPStorable ensStorableNewRef(EnsPStorable storable);

void ensStorableDel(EnsPStorable* Pstorable);

EnsEStorableType ensStorableGetType(const EnsPStorable storable);

void *ensStorableGetAdaptor(const EnsPStorable storable);

ajuint ensStorableGetIdentifier(const EnsPStorable storable);

AjBool ensStorableSetIdentifier(EnsPStorable storable, ajuint identifier);

AjBool ensStorableSetAdaptor(EnsPStorable storable, void* adaptor);

AjBool ensStorableIsStored(const EnsPStorable storable,
                           const EnsPDatabaseconnection dbc);

/*
** End of prototype definitions
*/




#endif /* ensstorable_h */

#ifdef __cplusplus
}
#endif
