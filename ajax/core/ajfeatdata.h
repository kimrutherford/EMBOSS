/* @include ajfeatdata ********************************************************
**
** AJAX feature data include file
**
** @author Copyright (C) 1999 Richard Bruskiewich
** @version $Revision: 1.35 $
** @modified 2000 Ian Longden.
** @modified 2001 Peter Rice.
** @modified $Date: 2013/06/29 22:31:59 $ by $Author: rice $
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
******************************************************************************/

#ifndef AJFEATDATA_H
#define AJFEATDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajlist.h"
#include "ajtextdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define DEFAULT_GFF_VERSION 3

/*
** some featflag values are retained for reading in old output files
*/

#define AJFEATFLAG_START_BEFORE_SEQ 0x0001  /* <start */
#define AJFEATFLAG_END_AFTER_SEQ    0x0002  /* >end */
#define AJFEATFLAG_GENERATED        0x0004  /* generated for GFF3 */
#define AJFEATFLAG_BETWEEN_SEQ      0x0008  /* x^y */
#define AJFEATFLAG_START_TWO        0x0010  /* x.y.. */
#define AJFEATFLAG_END_TWO          0x0020  /* ..x.y */
#define AJFEATFLAG_POINT            0x0040  /* x */
#define AJFEATFLAG_COMPLEMENT_MAIN  0x0080  /* complement around the join */
#define AJFEATFLAG_MULTIPLE         0x0100  /* part of a multiple i.e. join*/
#define AJFEATFLAG_GROUP            0x0200  /* group */
#define AJFEATFLAG_ORDER            0x0400  /* order */
#define AJFEATFLAG_ONEOF            0x0800  /* one_of */
#define AJFEATFLAG_REMOTEID         0x1000  /* AB012345.6: */
#define AJFEATFLAG_LABEL            0x2000  /* label as location */
#define AJFEATFLAG_START_UNSURE     0x4000  /* unsure position: SwissProt '?' */
#define AJFEATFLAG_END_UNSURE       0x8000  /* unsure position: SwissProt '?' */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPFeattable *********************************************************
**
** Ajax data type for collecting AjPFeatures in a 'feature table'.
** The features themselves may be accessed via iteration methods.
**
** @alias AjSFeattable
** @alias AjOFeattable
**
** @new    ajFeattableNew        Constructor
** @delete ajFeattableDel        Default destructor
** @modify ajFeattableAdd        Adds an AjPFeature to a set
** @new    ajFeatRead            Reads in a feature set in a specified format
**
** @attr Seqid [AjPStr] Sequence name
** @attr Type [AjPStr] Sequence type: P or N
** @attr Features [AjPList] List of AjPFeatures...
** @attr Groups [ajuint] Maximum group number
** @attr Start [ajuint] First position used (like sequence begin)
** @attr End [ajuint] Last position used (like sequence end)
** @attr Len [ajuint] Maximum length
** @attr Offset [ajuint] Offset when trimmed
** @attr Rev [AjBool] reversed if true
** @attr Reversed  [AjBool] true: has been reverse-complemented
** @attr Trimmed   [AjBool] true: has been trimmed
** @attr Db [AjPStr] Database name from input
** @attr Setdb [AjPStr] Database name from command line
** @attr Full [AjPStr] Full name
** @attr Qry [AjPStr] UFO query for re-reading
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr TextPtr [AjPStr] Full text
** @attr Fpos [ajlong] File position (fseek) for Query
** @attr Format [AjEnum] Input format enum
** @attr Circular [AjBool] true: circular nucleotide molecule
** @@
******************************************************************************/

typedef struct AjSFeattable
{
    AjPStr  Seqid;
    AjPStr  Type;
    AjPList Features;
    ajuint  Groups;
    ajuint  Start;
    ajuint  End;
    ajuint  Len;
    ajuint  Offset;
    AjBool  Rev;
    AjBool  Reversed;
    AjBool  Trimmed;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  TextPtr;
    ajlong  Fpos;
    AjEnum  Format;
    AjBool Circular;
}  AjOFeattable;

#define AjPFeattable AjOFeattable*




/* @data AjPFeattabin *********************************************************
**
** Ajax class for feature table input
**
** @alias AjSFeattabin
** @alias AjOFeattabin
** @alias AjPFeattabIn
**
** @new    ajFeattabinNew Constructor
** @new    ajFeattabinNewSS Constructor with format, name and type
** @new    ajFeattabinNewSSF Constructor with format, name, type and input file
** @delete ajFeattabinDel Destructor
**
** @attr Input     [AjPTextin] Text file input object
** @attr Ufo [AjPStr] Original UFO
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr Seqid [AjPStr] Sequence entryname
** @attr Type [AjPStr] Type N or P
** @attr Seqname [AjPStr] name of AjPSeq assoc. with feature table
** @attr Local [AjBool] Opened as a local file if ajTrue
** @attr Rev       [AjBool]    Reverse/complement if true
** @attr Start     [ajint]     Start position
** @attr End       [ajint]     End position
** @attr Circular [AjBool] true: circular nucleotide molecule
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSFeattabin
{
    AjPTextin     Input;
    AjPStr        Ufo;
    AjPStr        Formatstr;
    AjPStr        Filename;
    AjPStr        Seqid;
    AjPStr        Type;
    AjPStr        Seqname;
    AjBool        Local;
    AjBool        Rev;
    ajint         Start;
    ajint         End;
    AjBool        Circular;
    char Padding[4];
}  AjOFeattabin;

#define AjPFeattabin AjOFeattabin*
#define AjPFeattabIn AjOFeattabin*




/* @data AjPFeattaball ********************************************************
**
** Ajax feature all (stream) object.
**
** Inherits an AjPFeattable but allows more data resources to be read from the
** same input by also inheriting the AjPFeattabin input object.
**
** @alias AjSFeattaball
** @alias AjOFeattaball
**
** @attr Feattable [AjPFeattable] Current feature table
** @attr Feattabin [AjPFeattabin] Feature input for reading next
** @attr Totfeattables [ajulong] Count of feature tables so far
** @attr Count [ajuint] Count of feature tables so far
** @attr Multi [AjBool] True if multiple values are expected
** @attr Returned [AjBool] if true: featuure table object has been returned
**                         to a new owner
**                         and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSFeattaball
{
    AjPFeattable Feattable;
    AjPFeattabin Feattabin;
    ajulong Totfeattables;
    ajuint Count;
    AjBool Multi;
    AjBool Returned;
    char Padding[4];
} AjOFeattaball;

#define AjPFeattaball AjOFeattaball*




/* @data AjPFeattabAccess *****************************************************
**
** Ajax feature table access database reading object.
**
** Holds information needed to read a feature table from a database.
** Access methods are defined for each known database type.
**
** Feature tables are read from the database using the defined
** database access function, which is usually a static function
** within ajtextdb.c or ajfeatdb.c
**
** This should be a static data object but is needed for the definition
** of AjPFeatin.
**
** @alias AjSFeattabAccess
** @alias AjOFeattabAccess
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

typedef struct AjSFeattabAccess
{
    const char *Name;
    AjBool (*Access) (AjPFeattabin ftabin);
    AjBool (*AccessFree) (void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOFeattabAccess;

#define AjPFeattabAccess AjOFeattabAccess*




/* @data AjPFeattabOut ********************************************************
**
** Ajax class for feature table output
**
** @alias AjSFeattabOut
** @alias AjOFeattabOut
**
** @new    ajFeattabOutNew Constructor
** @new    ajFeattabOutNewSSF Constructor with format, name, type and
**                            output file
** @delete ajFeattabOutDel Destructor
**
** @attr Ufo [AjPStr] Original output UFO
** @attr Formatstr [AjPStr] Output format name
** @attr Filename [AjPStr] Output filename
** @attr Directory [AjPStr] Output directory
** @attr Seqid [AjPStr] Output entryname
** @attr Type [AjPStr] Type N or P
** @attr Handle [AjPFile] Output file
** @attr Seqname [AjPStr] AjPSeq assoc. with feature table
** @attr Basename [AjPStr] Basename for output file
** @attr Format [ajint] Output format index
** @attr Local [AjBool] Opened as a local file if ajTrue
** @attr Count [ajuint] Number of feature tables written
** @attr Padding [char[4]] Padding to alignment boundary
** @attr Cleanup [void function] Function to write remaining lines on closing
** @@
******************************************************************************/

typedef struct AjSFeattabOut
{
    AjPStr        Ufo;
    AjPStr        Formatstr;
    AjPStr        Filename;
    AjPStr        Directory;
    AjPStr        Seqid;
    AjPStr        Type;
    AjPFile       Handle;
    AjPStr        Seqname;
    AjPStr        Basename;
    ajint         Format;
    AjBool        Local;
    ajuint        Count;
    char          Padding[4];
    void (*Cleanup) (AjPFile filethys);
}  AjOFeattabOut;

#define AjPFeattabOut AjOFeattabOut*




/* @data AjPFeature ***********************************************************
**
** Ajax Biological Feature object superclass.
**
** Holds generic data describing a single genome feature.
**
** A feature is a description of a
** sequence location which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Type'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Location' in the genome, and may have
** any arbitrary number of descriptor Tags and TagValues associated with it.
**
**
** @alias AjSFeature
** @alias AjOFeature
**
** @new    ajFeatNew             Constructor - must specify the associated
**                               (non-null) AjPFeattable
** @new    ajFeatNewII           Simple constructor with only start and end
**                               positions
** @new    ajFeatNewIIRev        Simple constructor with only start and end
**                               positions, sets feature to be
**                               on the reverse strand
** @new    ajFeatNewProt         Protein-specific constructor -
**                               must specify the associated
**                               (non-null) AjPFeattable
** @delete ajFeatDel             Default destructor
** @new    ajFeatCopy            Copy constructor
**
** @attr Source [AjPStr] Source program name (or EMBL)
** @attr Type [AjPStr] Feature type (feature key) from internal list
**                    for protein or nucleotide
** @attr Subfeatures [AjPList] Subfeatures
** @attr Tags [AjPList] Tag-value list (qualifier list)
** @attr GffTags [AjPList] GFF3 Tag-value list (qualifier list)
** @attr Protein [AjBool] true for a protein feature
** @attr Start [ajuint] Start position
** @attr End [ajuint] End position
** @attr Start2 [ajuint] Second start position - EMBL (a.b)
** @attr End2 [ajuint] Second end position - EMBL ..(a.b)
** @attr Frame [ajint] Frame 1..3, -1..-3 or 0
** @attr Flags [ajuint] Flag bit mask for EMBL location
** @attr Group [ajuint] Group for join/order/one-of
** @attr Remote [AjPStr] Remote ID - EMBL Remote:a.b
** @attr Label [AjPStr] Label name for location - EMBL legacy
** @attr Exon [ajuint] Exon number
** @attr Score [float] Score or 0.0 if none
** @attr Strand [char] Strand +/- or NULL
** @attr Padding [char[7]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSFeature
{
    AjPStr            Source;
    AjPStr            Type;
    AjPList           Subfeatures;
    AjPList           Tags;
    AjPList           GffTags;
    AjBool            Protein;
    ajuint            Start;
    ajuint            End;
    ajuint            Start2;
    ajuint            End2;
    ajint             Frame;
    ajuint            Flags;
    ajuint            Group;
    AjPStr            Remote;
    AjPStr            Label;
    ajuint            Exon;
    float             Score;
    char              Strand;
    char              Padding[7];
} AjOFeature;

#define AjPFeature AjOFeature*




/* @data AjPFeatGfftags *******************************************************
**
** Biological feature GFF3 special tags
**
** @alias AjSFeatGfftags
** @alias AjOFeatGfftags
**
** @attr Id [AjPStr] Identifier attribute. Features with the same id are
**                   one feature with multiple locations
** @attr Name [AjPStr] Display name attribute, can be shared
** @attr Alias [AjPStr] Alias attribute (list) e.g. locus name, can be shared
** @attr Parent [AjPStr] Parent attribute (list) for part_of grouping
** @attr Target [AjPStr] Target attribute "target_id start end [strand]"
** @attr Gap [AjPStr] Gap attribute in exonerate CIGAR format
** @attr DerivesFrom [AjPStr] Derives_from attribute for relations
** @attr Note [AjPStr] Note attribute (list)
** @attr Dbxref [AjPStr] Dbxref attribute (list) in format DBTAG:ID
** @attr OntologyTerm [AjPStr] Ontology_term attribute (list) in format DBTAG:ID
** @attr IsCircular [AjPStr] Is_circular attribute
** @@
******************************************************************************/

typedef struct AjSFeatGfftags
{
    AjPStr            Id;
    AjPStr            Name;
    AjPStr            Alias;
    AjPStr            Parent;
    AjPStr            Target;
    AjPStr            Gap;
    AjPStr            DerivesFrom;
    AjPStr            Note;
    AjPStr            Dbxref;
    AjPStr            OntologyTerm;
    AjPStr            IsCircular;
} AjOFeatGfftags;

#define AjPFeatGfftags AjOFeatGfftags*




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

#endif /* !AJFEATDATA_H */
