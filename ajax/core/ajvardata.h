/* @include ajvardata *********************************************************
**
** AJAX variation data structures
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.11 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
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

#ifndef AJVARDATA_H
#define AJVARDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define varNumAlt  -1
#define varNumGen  -2
#define varNumAny  -3





/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @enum AjEVarType ***********************************************************
**
** Variation metadata value type for INFO and FORMAT definitions
**
** @value AJVAR_UNKNOWN   Unknown
** @value AJVAR_INT       Integer
** @value AJVAR_FLOAT     Float
** @value AJVAR_CHAR      Character
** @value AJVAR_STR       String
** @value AJVAR_FLAG      Flag (INFO, not FORMAT)
** @value AJVAR_MAX       Above last defined value
******************************************************************************/

typedef enum AjOVarType
{
    AJVAR_UNKNOWN, AJVAR_INT, AJVAR_FLOAT, AJVAR_CHAR, AJVAR_STR,
    AJVAR_FLAG, AJVAR_MAX
} AjEVarType;




/* @data AjPVarHeader *********************************************************
**
** Ajax variation header object.
**
** Holds the metadata definitions for variation data
**
** Based on the requirements for VCF format 4.1
**
** @alias AjSVarHeader
** @alias AjOVarHeader
**
** @attr Header    [AjPList] Tag-value list of general header records
** @attr Fields    [AjPList] List of VarField definitions (INFO and FORMAT)
** @attr Samples   [AjPList] List of VarSample definitions
** @attr Filters   [AjPList] List of Filter tag-value definitions
** @attr Alts      [AjPList] List of Alternate tag-value definitions
** @attr Pedigrees [AjPList] List of Pedigree tag-value definitions
** @attr SampleIds [AjPList] List of SampleID strings from column headings
** @attr RefseqIds [AjPTable] Table of reference sequence name-id pairs
** @@
******************************************************************************/

typedef struct AjSVarHeader
{
    AjPList  Header;
    AjPList  Fields;
    AjPList  Samples;
    AjPList  Filters;
    AjPList  Alts;
    AjPList  Pedigrees;
    AjPList  SampleIds;
    AjPTable RefseqIds;
} AjOVarHeader;

#define AjPVarHeader AjOVarHeader*




/* @data AjPVarField **********************************************************
**
** Ajax variation field description object.
**
** Holds the metadata definitions for variation data fields
**
** Based on the requirements for VCF format 4.1
**
** @alias AjSVarField
** @alias AjOVarField
**
** @attr Field  [AjPStr] Field name (INFO, FILTER, FORMAT)
** @attr Id     [AjPStr] Identifier
** @attr Desc   [AjPStr] Description
** @attr Type   [AjEVarType] Type of value
** @attr Number [ajint] Count of values
** @@
******************************************************************************/

typedef struct AjSVarField
{
    AjPStr Field;
    AjPStr Id;
    AjPStr Desc;
    AjEVarType Type;
    ajint Number;
} AjOVarField;

#define AjPVarField AjOVarField*




/* @data AjPVarSample *********************************************************
**
** Ajax variation sample description object.
**
** Holds the metadata definitions for variation data samples
**
** Based on the requirements for VCF format 4.1
**
** @alias AjSVarSample
** @alias AjOVarSample
**
** @attr Id      [AjPStr] Identifier
** @attr Desc    [AjPStr*] Descriptions for each genome identifier
** @attr Genomes [AjPStr*] Genome identifiers
** @attr Mixture [float*] Mixture proportions, summing to 1.0
** @attr Number  [ajuint] Count of genome identifiers
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSVarSample
{
    AjPStr Id;
    AjPStr *Desc;
    AjPStr *Genomes;
    float *Mixture;
    ajuint Number;
    ajuint Padding;
} AjOVarSample;

#define AjPVarSample AjOVarSample*




/* @data AjPVarData ***********************************************************
**
** Ajax variation data object.
**
** Holds the metadata definitions for variation data records
**
** Based on the requirements for VCF format 4.1
**
** @alias AjSVarAlt
** @alias AjOVarAlt
**
** @attr Chrom   [AjPStr] Chromosome
** @attr Id      [AjPStr] Identifier
** @attr Ref     [AjPStr] Reference sequence(s)
** @attr Alt     [AjPStr] Alternate sequence(s)
** @attr Qual    [AjPStr] Quality
** @attr Filter  [AjPStr] Filter(s) failed
** @attr Info    [AjPStr] Info field metadata
** @attr Format  [AjPStr] Format field metadata
** @attr Samples [AjPList] Sample string records
** @attr Pos     [ajuint] Position
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSVarData
{
    AjPStr Chrom;
    AjPStr Id;
    AjPStr Ref;
    AjPStr Alt;
    AjPStr Qual;
    AjPStr Filter;
    AjPStr Info;
    AjPStr Format;
    AjPList Samples;
    ajuint Pos;
    ajuint Padding;
} AjOVarData;

#define AjPVarData AjOVarData*




/* @data AjPVar ***************************************************************
**
** Ajax variation object.
**
** Holds the variation itself, plus associated information.
**
** @alias AjSVar
** @alias AjOVar
**
** @attr Id        [AjPStr]  Id of term
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr Textptr   [AjPStr]  Full text
** @attr Data      [AjPList] Data records as AjPVarData objects
** @attr Header    [AjPVarHeader] Header record metadata
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Hasdata   [AjBool]  True when data has been loaded
** @@
******************************************************************************/

typedef struct AjSVar
{
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  Textptr;
    AjPList Data;
    AjPVarHeader Header;
    ajlong  Fpos;
    AjEnum  Format;
    AjBool Hasdata;
} AjOVar;

#define AjPVar AjOVar*




/* @data AjPVarin *************************************************************
**
** Ajax variation input object.
**
** Holds the input specification and information needed to read
** the variation and possible further entries
**
** @alias AjSVarin
** @alias AjOVarin
**
** @attr Input [AjPTextin] General text input object
** @attr Begin     [ajint]     Start position
** @attr End       [ajint]     End position
** @attr Loading [AjBool] True if data is now loading
** @attr Padding [char[4]] Padding to alignment boundary
** @attr VarData [void*] Format data for reuse, e.g. multiple term input
**                       (unused in current code)
** @@
******************************************************************************/

typedef struct AjSVarin
{
    AjPTextin Input;
    ajint Begin;
    ajint End;
    AjBool Loading;
    char Padding[4];
    void *VarData;
} AjOVarin;

#define AjPVarin AjOVarin*




/* @data AjPVarload ***********************************************************
**
** Ajax variation loader object.
**
** Inherits an AjPVar but allows more variations to be read from the
** same input by also inheriting the AjPVarin input object.
**
** @alias AjSVarload
** @alias AjOVarload
**
** @attr Var [AjPVar] Current variation
** @attr Varin [AjPVarin] Variation input for reading next
** @attr Count [ajuint] Count of terms so far
** @attr Loading [AjBool] True if data is now loading
** @attr Returned [AjBool] if true: Variation object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSVarload
{
    AjPVar Var;
    AjPVarin Varin;
    ajuint Count;
    AjBool Loading;
    AjBool Returned;
    ajuint Padding;
} AjOVarload;

#define AjPVarload AjOVarload*




/* @data AjPVarall ************************************************************
**
** Ajax variation all (stream) object.
**
** Inherits an AjPVar but allows more variations to be read from the
** same input by also inheriting the AjPVarin input object.
**
** @alias AjSVarall
** @alias AjOVarall
**
** @attr Loader [AjPVarload] Variation loader for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Variation object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSVarall
{
    AjPVarload Loader;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    ajuint Padding;
} AjOVarall;

#define AjPVarall AjOVarall*




/* @data AjPVarAccess *********************************************************
**
** Ajax variation access database reading object.
**
** Holds information needed to read a variation entry from a database.
** Access methods are defined for each known database type.
**
** Variation entries are read from the database using the defined
** database access function, which is usually a static function
** within ajvardb.c
**
** This should be a static data object but is needed for the definition
** of AjPVarin.
**
** @alias AjSVarAccess
** @alias AjOVarAccess
**
** @attr Name [const char*] Access method name used in emboss.default
** @attr Access [AjBool function] Access function
** @attr AccessFree [AjBool function] Access cleanup function
** @attr Qlink [const char*] Supported query link operators
** @attr Desc [const char*] Description
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Chunked [AjBool] Supports retrieval of entries in chunks
** @attr Padding [AjBool] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSVarAccess
{
    const char *Name;
    AjBool (*Access)(AjPVarin varin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOVarAccess;

#define AjPVarAccess AjOVarAccess*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJVARDATA_H */
