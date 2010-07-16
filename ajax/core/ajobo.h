#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajobo_h
#define ajobo_h

#include "ajax.h"




/* @data AjPObo ***************************************************************
**
** OBO parsed data
**
** @attr Termtable [AjPTable] Table of terms by id
** @attr Termnametable [AjPTable] Table of terms by name
** @attr Typedeftable [AjPTable] Table of typedefs
** @attr Instancetable [AjPTable] Table of instances
** @attr Annotable [AjPTable] Table of annotation stanzas
** @attr Formulatable [AjPTable] Table of formula stanzas
** @attr Misctable [AjPTable] Table of other stanzas
** @@
******************************************************************************/

typedef struct AjSObo
{
    AjPTable Termtable;
    AjPTable Termnametable;
    AjPTable Typedeftable;
    AjPTable Instancetable;
    AjPTable Annotable;
    AjPTable Formulatable;
    AjPTable Misctable;
} AjOObo;
#define AjPObo AjOObo*




/* @data AjPOboTag ************************************************************
**
** OBO tag name, value and comment.
**
** @attr Name [AjPStr] Tag name
** @attr Value [AjPStr] Tag value
** @attr Modifier [AjPStr] Tag modifier
** @attr Comment [AjPStr] Tag comment
** @attr Linenumber [ajuint] OBO file line number
** @attr Padding [ajuint] Padding to align structure
** @@
******************************************************************************/

typedef struct AjSOboTag
{
    AjPStr Name;
    AjPStr Value;
    AjPStr Modifier;
    AjPStr Comment;
    ajuint Linenumber;
    ajuint Padding;
} AjOOboTag;
#define AjPOboTag AjOOboTag*




/* @data AjPOboTerm ***********************************************************
**
** Term object
** Holds name and identifier of a single OBO term
**
** @alias AjSOboTerm
** @alias AjOOboTerm
**
** @attr Id        [AjPStr]  Id of term
** @attr Trueid    [AjPStr]  True id of an alt_id
** @attr Name      [AjPStr]  Name of term
** @attr Namespace [AjPStr]  Namespace of term
** @attr Def       [AjPStr]  Definition of term
** @attr Comment   [AjPStr]  Comment tag in term stanza
** @attr Taglist   [AjPList]  List of other AjPObotag tag name-value pairs
** @attr Obsolete  [AjBool]  True if term is obsolete
** @attr Builtin   [AjBool]  True if term is an OBO Built-in term
******************************************************************************/

typedef struct AjSOboTerm
{
    AjPStr  Id;
    AjPStr  Trueid;
    AjPStr  Name;
    AjPStr  Namespace;
    AjPStr  Def;
    AjPStr  Comment;
    AjPList Taglist;
    AjBool  Obsolete;
    AjBool  Builtin;
} AjOOboTerm;
#define AjPOboTerm AjOOboTerm*




/* @data AjPOboAlias *********************************************************
**
** Alias name for an OBO identifier
**
** @alias AjSOboAlias
** @alias AjOOboAlias
**
** @attr Alias     [AjPStr]  Alias name of term
** @attr Id        [AjPStr]  Id of term
******************************************************************************/

typedef struct AjSOboAlias
{
    AjPStr  Alias;
    AjPStr  Id;
} AjOOboAlias;
#define AjPOboAlias AjOOboAlias*




/*
** Prototype definitions
*/

AjPObo           ajOboNew(void);

AjPOboTerm       ajOboTermNew(void);
void             ajOboTermDel(AjPOboTerm *Pterm);
AjPObo           ajOboParseObo (AjPFile infile, const char* validations);
AjPOboTag        ajOboTagNew(const AjPStr name, const AjPStr value,
                             const AjPStr modifier, const AjPStr comment,
                             ajuint linenum);
void             ajOboTagDel(AjPOboTag* Ptag);
AjPOboTerm       ajOboFetchTerm(const AjPObo thys, const AjPStr query);
AjPOboTerm       ajOboFetchName(const AjPObo thys, const AjPStr query);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
