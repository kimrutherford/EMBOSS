/* @include ajrefseqdata ******************************************************
**
** AJAX reference sequence data structures
**
** These functions control all aspects of AJAX reference sequence
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.8 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
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

#ifndef AJREFSEQDATA_H
#define AJREFSEQDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPRefseq ************************************************************
**
** Ajax refseq object.
**
** Holds the refseq itself, plus associated information.
**
** @alias AjSRefseq
** @alias AjORefseq
**
** @attr Id        [AjPStr]  Id of term
** @attr Db        [AjPStr]  Database name from input
** @attr Setdb     [AjPStr]  Database name from command line
** @attr Full      [AjPStr]  Full name
** @attr Qry       [AjPStr]  Query for re-reading
** @attr Formatstr [AjPStr]  Input format name
** @attr Filename  [AjPStr]  Original filename
** @attr TextPtr   [AjPStr]  Full text
** @attr Desc      [AjPStr]  Description
** @attr Seq       [AjPStr]  Sequence
** @attr Seqlist   [AjPList] Sequence list
** @attr Fpos      [ajlong]  File position
** @attr Format    [AjEnum]  Input format enum
** @attr Count     [ajuint]  Number of lines read
** @@
******************************************************************************/

typedef struct AjSRefseq {
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  TextPtr;
    AjPStr  Desc;
    AjPStr  Seq;
    AjPList Seqlist;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint Count;
} AjORefseq;

#define AjPRefseq AjORefseq*




/* @data AjPRefseqin **********************************************************
**
** Ajax refseq input object.
**
** Holds the input specification and information needed to read
** the refseq and possible further entries
**
** @alias AjSRefseqin
** @alias AjORefseqin
**
** @attr Input [AjPTextin] General text input object
** @attr RefseqData [void*] Format data for reuse, e.g. multiple term input
**                         (unused in current code)
** @@
******************************************************************************/

typedef struct AjSRefseqin {
    AjPTextin Input;
    void *RefseqData;
} AjORefseqin;

#define AjPRefseqin AjORefseqin*




/* @data AjPRefseqall *********************************************************
**
** Ajax refseq all (stream) object.
**
** Inherits an AjPRefseq but allows more refseqs to be read from the
** same input by also inheriting the AjPRefseqin input object.
**
** @alias AjSRefseqall
** @alias AjORefseqall
**
** @attr Refseq [AjPRefseq] Current refseq
** @attr Refseqin [AjPRefseqin] Refseq input for reading next
** @attr Totterms [ajulong] Count of terms so far
** @attr Count [ajuint] Count of terms so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: Refseq object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSRefseqall
{
    AjPRefseq Refseq;
    AjPRefseqin Refseqin;
    ajulong Totterms;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjORefseqall;

#define AjPRefseqall AjORefseqall*




/* @data AjPRefseqAccess ******************************************************
**
** Ajax reference sequence access database reading object.
**
** Holds information needed to read a refseq entry from a database.
** Access methods are defined for each known database type.
**
** Refseq entries are read from the database using the defined
** database access function, which is usually a static function
** within ajrefseqdb.c
**
** This should be a static data object but is needed for the definition
** of AjPRefseqin.
**
** @alias AjSRefseqAccess
** @alias AjORefseqAccess
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

typedef struct AjSRefseqAccess
{
    const char *Name;
    AjBool (*Access)(AjPRefseqin refseqin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjORefseqAccess;

#define AjPRefseqAccess AjORefseqAccess*




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

#endif /* !AJREFSEQDATA_H */
