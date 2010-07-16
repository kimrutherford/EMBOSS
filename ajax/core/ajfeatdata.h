#ifdef __cplusplus
extern "C"
{
#endif

/*
**
** ajfeatdata.h - AJAX Sequence Feature include file
**            Version 1.0 - March 2001
**
*/

#ifndef ajfeatdata_h
#define ajfeatdata_h

/******************************************************************************
** Table Classes  *************************************************************
******************************************************************************/




/* @data AjPFeattabIn *********************************************************
**
** Ajax class for feature table input
**
** @alias AjSFeattabIn
** @alias AjOFeattabIn
**
** @new    ajFeattabInNew Constructor
** @new    ajFeattabInNewSS Constructor with format, name and type
** @new    ajFeattabInNewSSF Constructor with format, name, type and input file
** @delete ajFeattabInDel Destructor
**
** @attr Ufo [AjPStr] Original UFO
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr Seqid [AjPStr] Sequence entryname
** @attr Type [AjPStr] Type N or P
** @attr Handle [AjPFilebuff] Input buffered file
** @attr Seqname [AjPStr] name of AjPSeq assoc. with feature table
** @attr Format [ajint] Input format index
** @attr Local [AjBool] Opened as a local file if ajTrue
** @@
******************************************************************************/

typedef struct AjSFeattabIn {
  AjPStr        Ufo;
  AjPStr        Formatstr;
  AjPStr        Filename;
  AjPStr        Seqid;
  AjPStr        Type;
  AjPFilebuff   Handle;
  AjPStr        Seqname;
  ajint         Format;
  AjBool        Local;
}  AjOFeattabIn;

#define AjPFeattabIn AjOFeattabIn*




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
** @attr Cleanup [(void*)] Function to write remaining lines on closing
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
** @attr DefFormat [ajuint] Original input or 'source' format of feature table
** @attr Start [ajuint] First position used (like sequence begin)
** @attr End [ajuint] Last position used (like sequence end)
** @attr Len [ajuint] Maximum length
** @attr Offset [ajuint] Offset when trimmed
** @attr Groups [ajuint] Number of current group being added
** @@
******************************************************************************/

typedef struct AjSFeattable
{
    AjPStr            Seqid;
    AjPStr            Type;
    AjPList           Features;
    ajuint            DefFormat;
    ajuint            Start;
    ajuint            End;
    ajuint            Len;
    ajuint            Offset;
    ajuint            Groups;
}  AjOFeattable;

#define AjPFeattable AjOFeattable*


/******************************************************************************
** Feature Classes  ***********************************************************
******************************************************************************/




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
** @attr Protein [AjBool] true for a protein feature
** @attr Start [ajuint] Start position
** @attr End [ajuint] End position
** @attr Start2 [ajuint] Second start position - EMBL (a.b)
** @attr End2 [ajuint] Second end position - EMBL ..(a.b)
** @attr Frame [ajint] Frame 1..3, -1..-3 or 0
** @attr Tags [AjPList] Tag-value list (qualifier list)
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

typedef struct AjSFeature {

  AjPStr            Source;
  AjPStr            Type;
  AjBool            Protein;
  ajuint            Start;
  ajuint            End;
  ajuint            Start2;
  ajuint            End2;
  ajint             Frame;
  AjPList           Tags;
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


#endif /* ajfeatdata_h */

#ifdef __cplusplus
}
#endif
