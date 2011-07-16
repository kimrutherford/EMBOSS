/*
** This is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include <math.h>

#include "ajax.h"

static AjPRegexp featoutRegUfoFmt = NULL;
static AjPRegexp featoutRegUfoFile = NULL;
static AjPRegexp featoutRegQuote = NULL;

static AjPStr featoutUfoTest = NULL;
static AjPStr featoutFormatTmp = NULL;
static AjPStr featoutValCopy     = NULL;
static AjPStr featoutValTmp     = NULL;
static AjPStr featoutSubStr      = NULL;
static AjPStr featoutTmpStr      = NULL;
static AjPStr featoutFmtTmp      = NULL;
static AjPStr featoutStr = NULL;
static AjPStr featoutLimTmp = NULL;




static AjBool       featWriteEmbl(const AjPFeature thys, AjPFile file,
                                  const AjPStr Seqid, AjBool IsEmbl);
static AjBool       featTypePirOut(AjPStr* type);
static void         featTagEmblDefault(AjPStr* pout,
				       const AjPStr tag, AjPStr* pval);
static void         featTagGff2Default(AjPStr* pout, const AjPStr tag,
				       AjPStr* pval);
static void         featCleanDasgff(AjPFile file);
static void         featDumpEmbl(const AjPFeature thys, const AjPStr location,
				 AjPFile file, const AjPStr Seqid,
				 AjBool IsEmbl);
static void         featDumpRefseq(const AjPFeature thys,
                                   const AjPStr location,
                                   AjPFile file, const AjPStr Seqid);
static void         featDumpRefseqp(const AjPFeature thys,
                                    const AjPStr location,
                                    AjPFile file, const AjPStr Seqid);
static void         featDumpGff2(const AjPFeature thys,
				 const AjPFeattable owner,
				 AjPFile file);
static void         featDumpGff3(const AjPFeature thys,
                                 const AjPFeature parent,
				 const AjPFeattable owner,
				 AjPFile file);
static AjBool       featDumpGff3Tag(AjPFile file, const char* tag,
	                            const AjPStr value, AjBool firsttag);
static void         featDumpPir(const AjPFeature thys, const AjPStr location,
				AjPFile file);
static void         featDumpSwiss(const AjPFeature thys, AjPFile file,
				  const AjPFeature gftop);
static AjBool       featFindOutFormatC(const char* format, ajint* iformat);
static AjBool       featFindOutFormatS(const AjPStr format, ajint* iformat);
static void         featLocEmblWrapC(AjPStr* pval, ajuint margin,
				     const char* prefix, const char* preftyp,
				     AjPStr* retstr);
static void         featTagEmblWrapC(AjPStr* pval, ajuint margin,
				     const char* prefix,
				     AjPStr* retstr);
static void         featTagQuoteEmbl(AjPStr* pval);
static void         featTagQuoteGff2(AjPStr* pval);
static void         featTagQuoteGff3(AjPStr* pval);
static void         featTagSwissWrapC(AjPStr* pval, ajuint margin,
				      const char* prefix,
				      AjPStr* retstr);

static AjBool       featoutUfoProcess(AjPFeattabOut thys, const AjPStr ufo);

static AjBool       feattableWriteEmbl(const AjPFeattable Feattab,
				       AjPFile file,
				       AjBool IsEmbl);
static AjBool       feattableWriteRefseq(const AjPFeattable Feattab,
                                         AjPFile file);
static AjBool       feattableWriteRefseqp(const AjPFeattable Feattab,
                                          AjPFile file);

static AjBool       featTagAllLimit(AjPStr* pval, const AjPStr values);
static void         featTagDasgffDefault(AjPStr* pout, const AjPStr tag,
                                         AjPStr* pval);



/* @datastatic FeatPOutFormat *************************************************
**
** Feature output formats
**
** @alias FeatSOutFormat
** @alias FeatOOutFormat
**
** @attr Name [const char*] Format name
** @attr Nucleotide [AjBool] True if suitable for nucleotide data
** @attr Protein [AjBool] True if suitable for protein data
** @attr Write [(AjBool*)] Function to write data
** @attr Desc [const char*] Description
** @attr Alias [AjBool] True if name is an alias for an identical definition
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSOutFormat
{
    const char* Name;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool (*Write) (AjPFeattabOut ftout, const AjPFeattable thys);
    const char* Desc;
    AjBool Alias;
    ajint Padding;
} FeatOOutFormat;
#define FeatPOutFormat FeatOOutFormat*




/* @funclist featOutFormatDef *************************************************
**
** Feature output formats
**
** Includes functions to write the output file (ajFeattableWrite)
**
******************************************************************************/

static FeatOOutFormat featOutFormatDef[] =
{
    /* Name     Nucleotide Prot
         VocInit             WriteFunction
	 Description       Alias   Padding */
    {"unknown", AJFALSE,   AJFALSE,
	 ajFeattableWriteGff3,
	 "unknown format", AJFALSE, 0},
    {"gff",     AJTRUE,    AJTRUE,
	 ajFeattableWriteGff3,
	 "GFF version 3", AJFALSE, 0},
    {"gff2",    AJTRUE,    AJTRUE,
	 ajFeattableWriteGff2,
	 "GFF version 2", AJFALSE, 0},
    {"gff3",     AJTRUE,    AJTRUE,
	 ajFeattableWriteGff3,
	 "GFF version 3", AJTRUE,  0},
    {"embl",      AJTRUE,    AJFALSE,
	 ajFeattableWriteEmbl,
	 "embl format", AJFALSE, 0},
    {"em",        AJTRUE,    AJFALSE,
	 ajFeattableWriteEmbl,
	 "embl format", AJTRUE,  0},
    {"genbank",   AJTRUE,    AJFALSE,
	 ajFeattableWriteGenbank,
	 "genbank format", AJFALSE, 0},
    {"gb",        AJTRUE,    AJFALSE,
	 ajFeattableWriteGenbank,
	 "genbank format", AJTRUE, 0},
    {"ddbj",      AJTRUE,    AJFALSE,
	 ajFeattableWriteDdbj,
	 "ddbj format", AJFALSE, 0},
    {"refseq",    AJTRUE,    AJFALSE,
	 ajFeattableWriteGenbank,
	 "genbank format", AJTRUE, 0},
    {"refseqp",   AJFALSE,   AJTRUE,
	 ajFeattableWriteRefseqp,
	 "genbank format", AJTRUE, 0},
    {"pir",       AJFALSE,   AJTRUE,
	 ajFeattableWritePir,
	 "PIR format", AJFALSE, 0},
    {"nbrf",      AJFALSE,   AJTRUE,
	 ajFeattableWritePir,
	 "PIR format", AJTRUE,  0},
    {"swiss",     AJFALSE,   AJTRUE,
	  ajFeattableWriteSwiss,
	 "SwissProt format", AJFALSE, 0},
    {"sw",        AJFALSE,   AJTRUE,
	 ajFeattableWriteSwiss,
	 "SwissProt format", AJTRUE, 0},
    {"swissprot", AJFALSE,   AJTRUE,
	 ajFeattableWriteSwiss,
	 "SwissProt format", AJTRUE, 0},
    {"dasgff",    AJTRUE,   AJTRUE,
	 ajFeattableWriteDasgff,
	 "DAS GFF format", AJFALSE, 0},
    {"debug",     AJTRUE,    AJTRUE,
	  ajFeattableWriteDebug,
	 "Debugging trace of full internal data content", AJFALSE, 0},
    {NULL, AJFALSE, AJFALSE, NULL, NULL, AJFALSE, 0}
};

static FeatPOutFormat featOutFormat = featOutFormatDef;




/* @datastatic FeatPTypeOut ***************************************************
**
** Feature output types
**
** @alias FeatSTypeOut
** @alias FeatOTypeOut
**
** @attr Name [const char*] Specified name
** @attr Value [const char*] Internal type "P" or "N"
** @@
******************************************************************************/

typedef struct FeatSTypeOut
{
    const char* Name;
    const char* Value;
} FeatOTypeOut;
#define FeatPTypeOut FeatOTypeOut*




static FeatOTypeOut featOutTypes[] =
{
    {"P", "P"},
    {"protein", "P"},
    {"N", "N"},
    {"nucleotide", "N"},
    {"any", ""},
    {NULL, NULL}
};




/* @func ajFeattabOutOpen *****************************************************
**
** Processes the specified UFO, and opens the resulting output file.
**
** If the AjPFeattabOut has a filename, this is used in preference.
** If not, the ufo is processed.
**
** @param [u] thys [AjPFeattabOut] Features table output object
** @param [r] ufo [const AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutOpen(AjPFeattabOut thys, const AjPStr ufo)
{
    /*ajDebug("ajFeattabOutOpen ufo:'%S' dir:'%S' file:'%S'\n",
	    ufo, thys->Directory, thys->Filename);*/
    if(thys->Handle)
	return ajTrue;

    if(ajStrGetLen(thys->Filename))
    {
	if(!featFindOutFormatS(thys->Formatstr, &thys->Format))
	    ajErr("unknown output feature format '%S' "
		  "will write as %s instead\n",
		  thys->Formatstr, featOutFormatDef[thys->Format].Name);
    }
    else
    {
	if(!featoutUfoProcess(thys, ufo))
	    return ajFalse;
    }

    /*ajDebug("trying to open dir:'%S' file:'%S' fmt:'%S'\n",
	    thys->Directory, thys->Filename, thys->Formatstr);*/
    thys->Handle = ajFileNewOutNamePathS(thys->Filename, thys->Directory);

    if(!thys->Handle)
	return ajFalse;
   /* ajDebug("after opening '%S'\n", thys->Filename);*/

    return ajTrue;
}




/* @func ajFeattabOutFile *****************************************************
**
** Returns the feature table output file object
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjPFile] File object
** @@
******************************************************************************/

AjPFile ajFeattabOutFile(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutFile\n");*/
    return thys->Handle;
}




/* @func ajFeattabOutFilename *************************************************
**
** Returns the name of a feature table output file
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjPStr] Filename
** @@
******************************************************************************/

AjPStr ajFeattabOutFilename(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutFilename\n");*/
    if(ajStrGetLen(thys->Filename))
	return thys->Filename;

    return NULL;
}




/* @func ajFeattabOutIsOpen ***************************************************
**
** Checks whether feature output file has already been opened
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjBool] ajTrue if file is open
** @@
******************************************************************************/

AjBool ajFeattabOutIsOpen(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutIsOpen\n");*/
    if(thys->Handle)
	return ajTrue;

    return ajFalse;
}




/* @func ajFeattabOutIsLocal **************************************************
**
** Checks whether feature output file has already been opened
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjBool] ajTrue if file is open
** @@
******************************************************************************/

AjBool ajFeattabOutIsLocal(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutIsLocal Handle %x Local %B\n",
	    thys->Handle, thys->Local);*/

    if(thys->Handle && thys->Local)
	return ajTrue;

    return ajFalse;
}




/* @func ajFeattabOutSet ******************************************************
**
** Processes the specified UFO, and specifies the resulting output file.
**
** @param [u] thys [AjPFeattabOut] Features table output object
** @param [r] ufo [const AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSet(AjPFeattabOut thys, const AjPStr ufo)
{
   /* ajDebug("ajFeattabOutSet ufo:'%S' dir:'%S' file:'%S'\n",
	    ufo, thys->Directory, thys->Filename);*/
    if(thys->Handle)
	return ajTrue;

    if(!featoutUfoProcess(thys, ufo))
	return ajFalse;

    return ajTrue;
}




/* @func ajFeattabOutSetBasename **********************************************
**
** Sets the base output filename for feature table output
**
** @param [u] thys [AjPFeattabOut] feature table output
** @param [r] basename [const AjPStr] Output base filename
** @return [void]
** @@
******************************************************************************/

void ajFeattabOutSetBasename(AjPFeattabOut thys, const AjPStr basename)
{
    AjPStr tmpname = NULL;

    tmpname = ajStrNewS(basename);    
    ajFilenameTrimAll(&tmpname);    
    ajStrAssignEmptyS(&thys->Basename, tmpname);
    ajStrDel(&tmpname);
    /*ajDebug("ajFeattabOutSetBasename '%S' result '%S'\n",	
	    basename, thys->Basename);*/
    
    return;  
}




/* @func ajFeattabOutSetSeqname ***********************************************
**
** Processes the specified UFO, and specifies the resulting output file.
**
** @param [u] thys [AjPFeattabOut] Features table output object
** @param [r] name [const AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSetSeqname(AjPFeattabOut thys, const AjPStr name)
{
   /* ajDebug("ajFeattabOutSet ufo:'%S' dir:'%S' file:'%S'\n",
	    ufo, thys->Directory, thys->Filename);*/
    ajStrAssignS(&thys->Seqname, name);

    return ajTrue;
}




/* @func ajFeattabOutNewCSF ***************************************************
**
** Constructor for an empty feature table output object, using an
** existing open output file (the file sequence data is already written to)
**
** @param [r] fmt [const char*] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] file [AjPFile] Output file
** @return [AjPFeattabOut] Feature table output object
** @category new [AjPFeattabOut] Constructor with format, name, type
**                               and output file
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNewCSF(const char* fmt, const AjPStr name,
				 const char* type, AjPFile file)
{
    AjPFeattabOut pthis;
    ajint iformat = 0;

    /*ajDebug("ajFeattabOutNewCSF '%S' '%S' '%s' '%F'\n",
	    fmt, name, type, file);*/

    if(!featFindOutFormatC(fmt, &iformat))
	return NULL;

    pthis = ajFeattabOutNew();
    ajStrAssignC(&pthis->Formatstr, featOutFormat[iformat].Name);
    pthis->Format = iformat;
    ajFeattabOutSetTypeC(pthis, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Handle = file;
    pthis->Local = ajTrue;

    /*ajDebug("ajFeatTabOutNewSSF %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabOutNewSSF ***************************************************
**
** Constructor for an empty feature table output object, using an
** existing open output file (the file sequence data is already written to)
**
** @param [r] fmt [const AjPStr] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] file [AjPFile] Output file
** @return [AjPFeattabOut] Feature table output object
** @category new [AjPFeattabOut] Constructor with format, name, type
**                               and output file
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNewSSF(const AjPStr fmt, const AjPStr name,
				 const char* type, AjPFile file)
{
    AjPFeattabOut pthis;
    ajint iformat = 0;

    /*ajDebug("ajFeattabOutNewSSF '%S' '%S' '%s' '%F'\n",
	    fmt, name, type, file);*/

    if(!featFindOutFormatS(fmt, &iformat))
	return NULL;

    pthis = ajFeattabOutNew();
    ajStrAssignC(&pthis->Formatstr, featOutFormat[iformat].Name);
    pthis->Format = iformat;
    ajFeattabOutSetTypeC(pthis, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Handle = file;
    pthis->Local = ajTrue;

    /*ajDebug("ajFeatTabOutNewSSF %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabOutNew ******************************************************
**
** Constructor for an empty feature table output object
**
** @return [AjPFeattabOut] Feature table input object
** @category new [AjPFeattabOut] Constructor
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNew(void)
{
    AjPFeattabOut pthis;
    AJNEW0(pthis);

    /*ajDebug("ajFeattabOutNew %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabOutClear ****************************************************
**
** Clears all data for a feature table output object
**
** @param [d] thys [AjPFeattabOut *] feature format
** @return [void] Feature table output object
** @category delete [AjPFeattabOut] Destructor
** @@
******************************************************************************/

void ajFeattabOutClear(AjPFeattabOut *thys)
{
    AjPFeattabOut pthis;

    pthis = *thys;

    if(!pthis)
        return;

    ajStrSetClear(&pthis->Ufo);
    ajStrSetClear(&pthis->Formatstr);
    ajStrSetClear(&pthis->Filename);
    ajStrSetClear(&pthis->Directory);
    ajStrSetClear(&pthis->Seqid);
    ajStrSetClear(&pthis->Type);
    ajStrSetClear(&pthis->Seqname);
    ajStrSetClear(&pthis->Basename);

    if(pthis->Cleanup)
        pthis->Cleanup(pthis->Handle);

    pthis->Cleanup = NULL;
    pthis->Count = 0;

    /* "Local" tables have borrowed an open file with NewSSF */
    if(!pthis->Local)
	ajFileClose(&pthis->Handle);

    return;
}




/* @func ajFeattabOutDel ******************************************************
**
** Destructor for a feature table output object
**
** @param [d] thys [AjPFeattabOut *] feature format
** @return [void] Feature table output object
** @category delete [AjPFeattabOut] Destructor
** @@
******************************************************************************/

void ajFeattabOutDel(AjPFeattabOut *thys)
{
    AjPFeattabOut pthis;

    pthis = *thys;
    if(!pthis)
        return;

    ajStrDel(&pthis->Ufo);
    ajStrDel(&pthis->Formatstr);
    ajStrDel(&pthis->Filename);
    ajStrDel(&pthis->Directory);
    ajStrDel(&pthis->Seqid);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Seqname);
    ajStrDel(&pthis->Basename);

    if(pthis->Cleanup)
        pthis->Cleanup(pthis->Handle);

    pthis->Cleanup = NULL;
    pthis->Count = 0;

    /* "Local" tables have borrowed an open file with NewSSF */
    if(!pthis->Local)
	ajFileClose(&pthis->Handle);

    AJFREE(pthis);

    return;
}




/* @func ajFeattableWriteUfo **************************************************
**
** Parses a UFO, opens an output file, and writes a feature table to it.
**
** @param [u] thys [AjPFeattabOut] Feature output object
** @param [r] ftable [const AjPFeattable] Feature table to be written
** @param [r] ufo [const AjPStr] UFO feature spec (ignored if already open)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeattableWriteUfo(AjPFeattabOut thys, const AjPFeattable ftable,
		      const AjPStr ufo)
{
    if(!ajFeattabOutIsOpen(thys))
       ajFeattabOutOpen(thys, ufo);

    return ajFeattableWrite(thys, ftable);
}




/* @obsolete ajFeatUfoWrite
** @replace ajFeattableWriteUfo (1,2,3/2,1,3)
*/

__deprecated AjBool ajFeatUfoWrite(const AjPFeattable thys,
                                   AjPFeattabOut featout,
                                   const AjPStr ufo)
{
    return ajFeattableWriteUfo(featout, thys, ufo);
}





/* @funcstatic featoutUfoProcess **********************************************
**
** Converts a UFO Uniform Feature Object into an open output file.
**
** First tests for "format::" and sets format if it is found
**
** Then checks for a filename, and defaults to the sequence name (as stored
** in the AjPFeattabOut) or "unknown" if there is no name.
**
** @param [u] thys [AjPFeattabOut] Feature table to be written.
** @param [r] ufo [const AjPStr] UFO.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featoutUfoProcess(AjPFeattabOut thys, const AjPStr ufo)
{   
    AjBool fmtstat = ajFalse;	/* status returns from regex tests */
    AjBool filstat = ajFalse;	/* status returns from regex tests */

    AjPFeattabOut featout = thys;
    
    if(!featoutRegUfoFmt)
	featoutRegUfoFmt = ajRegCompC("^([A-Za-z0-9]*):+(.*)$");

    /*
    ** \1 format
    ** \2 remainder
    */
    if(!featoutRegUfoFile)
	featoutRegUfoFile = ajRegCompC("^([^:]+)$");
    
    /*ajDebug("featoutUfoProcess UFO '%S'\n", ufo);*/
    
    ajStrAssignS(&featoutUfoTest, ufo);
    
    if(ajStrGetLen(ufo))
    {
	fmtstat = ajRegExec(featoutRegUfoFmt, featoutUfoTest);
	/*ajDebug("feat format regexp: %B\n", fmtstat);*/
    }
    
    if(fmtstat)
    {
	ajRegSubI(featoutRegUfoFmt, 1, &featout->Formatstr);
	ajStrAssignEmptyC(&featout->Formatstr,
		  featOutFormat[0].Name); /* unknown */
	ajRegSubI(featoutRegUfoFmt, 2, &featoutUfoTest);   /* trim off format */
	/*ajDebug("found feat format %S\n", featout->Formatstr);*/
    }
    else
    {
	/*ajDebug("no feat format specified in UFO '%S' try '%S' or 'gff'\n",
		ufo, featout->Formatstr);*/
	ajStrAssignEmptyC(&featout->Formatstr, "gff");
    }
    
    if(!featFindOutFormatS(featout->Formatstr, &featout->Format))
	ajErr("unknown output feature format '%S' "
	      "will write as gff3 instead\n",
	      featout->Formatstr );

    /* now go for the filename */
    
    filstat = ajRegExec(featoutRegUfoFile, featoutUfoTest);

    /*ajDebug("featRegUfoFile: %B\n", filstat);*/

    if(filstat)
    {
	ajRegSubI(featoutRegUfoFile, 1, &featout->Filename);
	/*ajDebug("set from UFO featout Filename '%S'\n",
	  featout->Filename);*/
    }
    else
    {
	if(ajStrGetLen(featout->Basename))
	    ajFmtPrintS(&featoutUfoTest, "%S.%S", featout->Basename,
			featout->Formatstr);
	else if(ajStrGetLen(featout->Seqname))
	    ajFmtPrintS(&featoutUfoTest, "%S.%S", featout->Seqname,
			featout->Formatstr);
	else
	    ajFmtPrintS(&featoutUfoTest, "unknown.%S", featout->Formatstr);

	ajStrAssignEmptyS(&featout->Filename, featoutUfoTest);
	/*ajDebug("generate featout filename '%S' dir '%S'\n",
	        featout->Filename, featout->Directory);*/
    }
    
    /*ajDebug("\n");*/
    
    return ajTrue;
}




/* @funcstatic featFindOutFormatC *********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const char*] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindOutFormatC(const char* format, ajint* iformat)
{
    ajint i = 0;

    /*ajDebug("featFindOutFormatC '%s'\n", format);*/
    if(!*format)
	return ajFalse;

    ajStrAssignC(&featoutFormatTmp, format);
    ajStrFmtLower(&featoutFormatTmp);

    for(i=0; featOutFormat[i].Name; i++)
    {
	/*ajDebug("test %d '%s' len=%d\n",
		i, featOutFormat[i].Name,ajStrGetLen(featoutFormatTmp));*/
	if(ajStrMatchC(featoutFormatTmp,
		       featOutFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDelStatic(&featoutFormatTmp);
	    /*ajDebug("found '%s' at %d\n", featOutFormat[i].Name, i);*/

	    return ajTrue;
	}
    }


    ajStrDelStatic(&featoutFormatTmp);
    *iformat = 1;

    return ajFalse;
}




/* @funcstatic featFindOutFormatS *********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindOutFormatS(const AjPStr format, ajint* iformat)
{
    return featFindOutFormatC(ajStrGetPtr(format), iformat);
}




/* @func ajFeatOutFormatDefault ***********************************************
**
** Sets the default output format.
** Checks the _OUTFEATFORMAT variable,
** and uses GFF3 if no other definition is found.
**
** @param [w] pformat [AjPStr*] Default output feature format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatOutFormatDefault(AjPStr* pformat)
{
    if(ajStrGetLen(*pformat))
    {
	/*ajDebug("... output feature format '%S'\n", *pformat);*/
    }
    else
    {
	if(ajNamGetValueC("outfeatformat", pformat))
	{
	    /*ajDebug("ajFeatOutFormatDefault '%S' from EMBOSS_OUTFEATFORMAT\n",
		     *pformat);*/
	}
	else
	{
	    ajStrAssignEmptyC(pformat, "gff"); /* use the real name */
	    /*ajDebug("... output feature format not set, default to '%S'\n",
		    *pformat);*/
	}
    }

    return ajTrue;
}




/* @func ajFeattableWrite *****************************************************
**
** Generic interface function for writing features to a file
** defined by a feature table output object.
**
** @param  [u] ftout   [AjPFeattabOut]  Specifies the external source
**                                       (file) of the features to be written
** @param  [r] features [const AjPFeattable]  Feature set to be written out
** @return [AjBool]  Returns ajTrue if successful; ajFalse otherwise
** @@
******************************************************************************/

AjBool ajFeattableWrite(AjPFeattabOut ftout, const AjPFeattable features)
{
    AjBool result = ajFalse;

    AjPFile file;
    ajint format;
    AjBool isnuc = ajFalse;
    AjBool isprot = ajFalse;
    AjBool ok = ajFalse;

    if(features)
    {
	if(!ftout)
	    return ajFalse;

	if(ajFeattableIsNuc(features))
	    isnuc = ajTrue;

	if(ajFeattableIsProt(features))
	    isprot = ajTrue;

	ok = ajFalse;

	if(isnuc && featOutFormat[ftout->Format].Nucleotide)
	    ok = ajTrue;
	else if(isprot && featOutFormat[ftout->Format].Protein)
	    ok = ajTrue;

	if(!ok)
	{
	    if(isnuc)
		ajErr("Feature format '%S' not supported for nucleotide "
                      "sequence features",
		      ftout->Formatstr);
	    else if(isprot)
		ajErr("Feature format '%S' not supported for protein "
                      "sequence features",
		      ftout->Formatstr);
	    else
		ajErr("Feature format '%S' failed: unknown sequence feature "
                      "type",
		      ftout->Formatstr);
	    return ajFalse;
	}

	/*ajDebug("ajFeattableWrite Validating arguments\n");*/

	file = ftout->Handle;

	if(!file)
	    return ajFalse;

	format  = ftout->Format;

	/*ajDebug("ajFeattableWrite format is %d OK\n",ftout->Format);*/

	ajFeatVocabInit(featOutFormat[format].Name);
	result = featOutFormat[format].Write(ftout, features);

        ftout->Count++;
	return result;
    }

   /* ajDebug(" NO Features to output\n");*/
    return AJTRUE;
}




/* @obsolete ajFeatWrite
** @rename ajFeattableWrite
*/

__deprecated AjBool ajFeatWrite(AjPFeattabOut ftout,
                                const AjPFeattable features)
{
    return ajFeattableWrite(ftout, features);
}




/* @func ajFeattableWriteGff2 *************************************************
**
** Write feature table in GFF 2.0 format
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] Feattab [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGff2(AjPFeattabOut ftout, const AjPFeattable Feattab)
{
    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    /* ajDebug("ajFeattableWriteGff2 Checking arguments\n"); */
    if(!file)
	return ajFalse;
    
    /* Print header first */
    ajFmtPrintF(file, "##gff-version 2.0\n") ;
    
    ajFmtPrintF(file, "##date %D\n", ajTimeRefTodayFmt("GFF")) ;
    
    if(ajStrMatchC(Feattab->Type, "N"))
	ajFmtPrintF(file, "##Type %s %S\n", "DNA",
		    Feattab->Seqid);
    else if(ajStrMatchC(Feattab->Type, "P"))
	ajFmtPrintF(file, "##Type %s %S\n", "Protein",
		    Feattab->Seqid);
    else
	ajFmtPrintF(file, "##Type unknown <%S> %S\n",
		    Feattab->Type, Feattab->Seqid);
    

  /* For all features... relatively simple because internal structures
     are deliberately styled on GFF */

    if(Feattab->Features)
    {
	iter = ajListIterNewread(Feattab->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
	    featDumpGff2(gf, Feattab, file);
	}

	ajListIterDel(&iter);
    }

    return ajTrue;
}




/* @func ajFeattableWriteGff3 *************************************************
**
** Write feature table in GFF format
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] Feattab [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGff3(AjPFeattabOut ftout, const AjPFeattable Feattab)
{
    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    /* ajDebug("ajFeattableWriteGff3 Checking arguments\n"); */
    if(!file)
	return ajFalse;
    
    /* Print GFF3-specific header first with ## tags */

    ajFmtPrintF(file, "##gff-version 3\n") ;
    
    ajFmtPrintF(file, "##sequence-region %S %u %u\n",
		Feattab->Seqid, ajFeattableGetBegin(Feattab),
		ajFeattableGetEnd(Feattab));

    /* EMBOSS specific header with #! tags */

    ajFmtPrintF(file, "#!Date %D\n", ajTimeRefTodayFmt("GFF"));

    if(ajStrMatchC(Feattab->Type, "P"))
	ajFmtPrintF(file, "#!Type Protein\n");
    else
	ajFmtPrintF(file, "#!Type DNA\n");

    ajFmtPrintF(file, "#!Source-version EMBOSS %S\n", ajNamValueVersion());


  /* For all features... relatively simple because internal structures
     are deliberately styled on GFF */

    if(Feattab->Features)
    {
	iter = ajListIterNewread(Feattab->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
            featDumpGff3(gf, NULL, Feattab, file);
	}

	ajListIterDel(&iter);
    }

    return ajTrue;
}




/* @func ajFeattableWriteDdbj *************************************************
**
** Write a feature table in DDBJ format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteDdbj(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteEmbl(thys,ftout->Handle,ajFalse);
}




/* @func ajFeattableWriteEmbl *************************************************
**
** Write a feature table in EMBL format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteEmbl(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteEmbl(thys,ftout->Handle,ajTrue);
}




/* @func ajFeattableWriteGenbank **********************************************
**
** Write a feature table in GenBank format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGenbank(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteEmbl(thys,ftout->Handle,ajFalse);
}




/* @func ajFeattableWriteRefseq ***********************************************
**
** Write a feature table in Refseq format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteRefseq(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteRefseq(thys,ftout->Handle);
}




/* @func ajFeattableWriteRefseqp **********************************************
**
** Write a feature table in Refseqp format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteRefseqp(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteRefseqp(thys,ftout->Handle);
}




/* @funcstatic featWriteEmbl **************************************************
**
** Write a feature in EMBL format. Include any subfeatures
**
** @param [r] thys [const AjPFeature] Feature object
** @param [u] file [AjPFile] Output file
** @param [r] Seqid  [const AjPStr] Sequence ID
** @param [r] IsEmbl [AjBool] ajTrue for EMBL (different line prefix)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featWriteEmbl(const AjPFeature thys, AjPFile file,
                            const AjPStr Seqid, AjBool IsEmbl)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    AjBool whole        = ajFalse;           /* has "complement(" been added */
    AjPStr location     = NULL;              /* location list as a string */
    AjPStr temp         = NULL;
    AjPStr pos          = NULL;
    ajuint exon         = 0;

    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);

    if((thys->Flags & FEATFLAG_MULTIPLE)
       && ajListGetLength(thys->Subfeatures))
    {
        if(thys->Flags & FEATFLAG_COMPLEMENT_MAIN)
            whole = ajTrue;

        if(whole)
            ajStrAppendC(&location,"complement(");
        
        /* location from subfeatures */
        if(thys->Flags & FEATFLAG_ORDER)
            ajStrAppendC(&location,"order(");
        else
            ajStrAppendC(&location,"join(");

        iter = ajListIterNewread(thys->Subfeatures);

        while(!ajListIterDone(iter))
        {
            gf = ajListIterGet(iter);

            if(exon++)
                ajStrAppendC(&location,",");

	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);

            /* location start position */
	    if(gf->Flags & FEATFLAG_REMOTEID)
	    {
		ajFmtPrintAppS(&pos,"%S:",gf->Remote);
		/* ajDebug("remote: %S\n", gf->Remote); */
	    }
	    
	    if(gf->Flags & FEATFLAG_LABEL)
	    {
		ajFmtPrintAppS(&pos,"%S",gf->Label);
		/* ajDebug("label: %S\n", gf->Label); */
	    }
	    else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ)
	    {
		ajFmtPrintAppS(&pos,"<%d",gf->Start);
		/* ajDebug("<start\n"); */
	    }
	    else if(gf->Flags & FEATFLAG_START_TWO)
	    {
		ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
		/* ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2); */
	    }
	    else
	    {
		ajFmtPrintAppS(&pos,"%d",gf->Start);
		/* ajDebug("start\n"); */
	    }
	    
            /* location range and end position */
	    if(!(gf->Flags & FEATFLAG_POINT))
	    {
		if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
		{
		    ajFmtPrintAppS(&pos,"^%d",gf->End);
		    /* ajDebug("between ^end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_AFTER_SEQ)
		{
		    ajFmtPrintAppS(&pos,"..>%d",gf->End);
		    /* ajDebug(">end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_TWO)
		{
		    ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
		    /* ajDebug("end2 (%d.%d)\n", gf->End2, gf->End); */
		}
		else
		{
		    ajFmtPrintAppS(&pos,"..%d",gf->End);
		    /* ajDebug(".. end\n"); */
		}
	    }
	    if(gf->Strand == '-' && !whole)
	    {
		ajStrAssignC(&temp,"complement(");
		ajStrAppendS(&temp,pos);
		ajStrAppendC(&temp,")");
		/* ajDebug("strand '-', wrap exon with complement()\n"); */
	    }
	    else 
	    {
		ajStrAssignS(&temp,pos);
		/* ajDebug("simple exon\n"); */
	    }
	    ajStrAppendS(&location, temp);
        }

        ajListIterDel(&iter);

        
        ajStrAppendC(&location,")");
        if(whole)
            ajStrAppendC(&location,")");

        featDumpEmbl(thys, location, file, Seqid, IsEmbl);

        ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
        return ajTrue;
    }

    /* simple feature location */

    if(thys->Flags & FEATFLAG_COMPLEMENT_MAIN)
        whole = ajTrue;

    if(whole)
        ajStrAppendC(&location,"complement(");

    /* location start position */
    if(thys->Flags & FEATFLAG_REMOTEID)
    {
        ajFmtPrintAppS(&pos,"%S:",thys->Remote);
        /* ajDebug("remote: %S\n", thys->Remote); */
    }
	    
    if(thys->Flags & FEATFLAG_LABEL)
    {
        ajFmtPrintAppS(&pos,"%S",thys->Label);
        /* ajDebug("label: %S\n", thys->Label); */
    }
    else if(thys->Flags & FEATFLAG_START_BEFORE_SEQ)
    {
        ajFmtPrintAppS(&pos,"<%d",thys->Start);
        /* ajDebug("<start\n"); */
    }
    else if(thys->Flags & FEATFLAG_START_TWO)
    {
        ajFmtPrintAppS(&pos,"(%d.%d)",thys->Start,thys->Start2);
        /* ajDebug("start2 (%d.%d)\n", thys->Start, thys->Start2); */
    }
    else
    {
        ajFmtPrintAppS(&pos,"%d",thys->Start);
        /* ajDebug("start\n"); */
    }
	    
    /* location range and end position */
    if(!(thys->Flags & FEATFLAG_POINT))
    {
        if(thys->Flags & FEATFLAG_BETWEEN_SEQ)
        {
            ajFmtPrintAppS(&pos,"^%d",thys->End);
            /* ajDebug("between ^end\n"); */
        }
        else if(thys->Flags & FEATFLAG_END_AFTER_SEQ)
        {
            ajFmtPrintAppS(&pos,"..>%d",thys->End);
            /* ajDebug(">end\n"); */
        }
        else if(thys->Flags & FEATFLAG_END_TWO)
        {
            ajFmtPrintAppS(&pos,"..(%d.%d)",thys->End2,thys->End);
            /* ajDebug("end2 (%d.%d)\n", thys->End2, thys->End); */
        }
        else
        {
            ajFmtPrintAppS(&pos,"..%d",thys->End);
            /* ajDebug(".. end\n"); */
        }
    }
    if(thys->Strand == '-' && !whole)
    {
        ajStrAssignC(&temp,"complement(");
        ajStrAppendS(&temp,pos);
        ajStrAppendC(&temp,")");
        /* ajDebug("strand '-', wrap exon with complement()\n"); */
    }
    else 
    {
        ajStrAssignS(&temp,pos);
        /* ajDebug("simple exon\n"); */
    }
    
    ajStrAppendS(&location, temp);

    if(whole)
        ajStrAppendC(&location,")");

    featDumpEmbl(thys, location, file, Seqid, IsEmbl);

    ajStrDel(&location);
    ajStrDel(&pos);
    ajStrDel(&temp);
        
    /* any other features grouped under this one - write separately */

    if(ajListGetLength(thys->Subfeatures))
    {
        iter = ajListIterNewread(thys->Subfeatures);

        while(!ajListIterDone(iter))
        {
            gf = ajListIterGet(iter);
            
            featWriteEmbl(gf, file, Seqid, IsEmbl);
        }

        ajListIterDel(&iter);

    }
    

    return ajTrue;
}




/* @funcstatic feattableWriteEmbl *********************************************
**
** Write a feature table in EMBL format.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] file [AjPFile] Output file
** @param [r] IsEmbl [AjBool] ajTrue for EMBL (different line prefix)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteEmbl(const AjPFeattable thys, AjPFile file,
			         AjBool IsEmbl)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    
    /* Check arguments */
    
    /* ajDebug("feattableWriteEmbl Checking arguments\n"); */

    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsNuc(thys))
	return ajFalse;
    
    /* feature table heading */
    
    if(IsEmbl)
    {
	ajFmtPrintF(file, "FH   Key             Location/Qualifiers\n");
	ajFmtPrintF(file, "FH\n");
    }
    else
	ajFmtPrintF(file, "FEATURES             Location/Qualifiers\n");
    
    if(!thys->Features)
        return ajFalse;

    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*ajFeatCompByGroup);
    
    iter = ajListIterNewread(thys->Features);

    while(!ajListIterDone(iter))
    {
        gf = ajListIterGet(iter);

        featWriteEmbl(gf, file, thys->Seqid, IsEmbl);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic feattableWriteRefseq *******************************************
**
** Write a feature table in REFSEQ format.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] file [AjPFile] Output file
* @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteRefseq(const AjPFeattable thys, AjPFile file)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    AjPFeature gfprev   = NULL;
    AjBool join         = ajFalse;
    AjBool whole        = ajFalse;           /* has "complement(" been added */
    AjPStr location     = NULL;              /* location list as a string */
    AjPStr temp         = NULL;
    AjPStr pos          = NULL;
    ajuint oldgroup = UINT_MAX;
    
    /* Check arguments */
    
    /* ajDebug("feattableWriteRefseq Checking arguments\n"); */

    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsNuc(thys))
	return ajFalse;
    
    /* feature table heading */
    
    ajFmtPrintF(file, "FEATURES             Location/Qualifiers\n");
    
    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*ajFeatCompByGroup);
    
    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

	    if((oldgroup != gf->Group) && gfprev) /* previous location ready */
	    {
		if(join)
		{
		    ajStrAppendC(&location,")"); /* close bracket for join */
		    /* ajDebug("join: closing ')' appended\n"); */
		}

		if(whole)
		{
		    ajStrInsertC(&location,0,"complement(");
		    ajStrAppendC(&location,")");
		    /* ajDebug("wrap with complement(), reset whole %b to N\n",
		       whole); */
		    whole = ajFalse;
		}
		
		/* ajDebug("calling featDumpRefseq for gfprev\n"); */
		/* ajDebug("location: '%S'\n", location); */
		featDumpRefseq(gfprev, location, file,
                               thys->Seqid); /* gfprev has tag data */
		
		/* reset the values from previous */
		/* ajDebug("reset location\n"); */
		ajStrSetClear(&location);
		/* ajDebug("reset join  %b to N\n", join); */
		join = ajFalse;
	    }
	    
	    oldgroup = gf->Group;
	    
	    /* process the new gf */
	    
	    /* ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	       gf->Type, gf->Group,gf->Exon, gf->Flags,
	       ajListGetLength(gf->Tags)); */
	    
	    if(gf->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    {
		/* ajDebug("set2 whole %b to Y\n", whole); */
		whole =ajTrue;
	    }
	    
	    if(ajStrGetLen(location))   /* one location already there */
	    {
		if(!join)
		{
		    /* ajDebug("insert 'join(', set join Y\n"); */
		    if(gf->Flags & FEATFLAG_ORDER)
			ajStrInsertC(&location,0,"order(");
		    else
			ajStrInsertC(&location,0,"join(");
		    join = ajTrue;
		}

		ajStrAppendC(&location,",");
		/* ajDebug("append ','\n"); */
	    }
	    
	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);
	    
	    if(gf->Flags & FEATFLAG_REMOTEID)
	    {
		ajFmtPrintAppS(&pos,"%S:",gf->Remote);
		/* ajDebug("remote: %S\n", gf->Remote); */
	    }
	    
	    if(gf->Flags & FEATFLAG_LABEL)
	    {
		ajFmtPrintAppS(&pos,"%S",gf->Label);
		/* ajDebug("label: %S\n", gf->Label); */
	    }
	    else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ)
	    {
		ajFmtPrintAppS(&pos,"<%d",gf->Start);
		/* ajDebug("<start\n"); */
	    }
	    else if(gf->Flags & FEATFLAG_START_TWO)
	    {
		ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
		/* ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2); */
	    }
	    else
	    {
		ajFmtPrintAppS(&pos,"%d",gf->Start);
		/* ajDebug("start\n"); */
	    }
	    
	    if(!(gf->Flags & FEATFLAG_POINT))
	    {
		if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
		{
		    ajFmtPrintAppS(&pos,"^%d",gf->End);
		    /* ajDebug("between ^end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_AFTER_SEQ)
		{
		    ajFmtPrintAppS(&pos,"..>%d",gf->End);
		    /* ajDebug(">end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_TWO)
		{
		    ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
		    /* ajDebug("end2 (%d.%d)\n", gf->End2, gf->End); */
		}
		else
		{
		    ajFmtPrintAppS(&pos,"..%d",gf->End);
		    /* ajDebug(".. end\n"); */
		}
	    }
	    
	    if(gf->Strand == '-' && !whole)
	    {
		ajStrAssignC(&temp,"complement(");
		ajStrAppendS(&temp,pos);
		ajStrAppendC(&temp,")");
		/* ajDebug("strand '-', wrap exon with complement()\n"); */
	    }
	    else 
	    {
		ajStrAssignS(&temp,pos);
		/* ajDebug("simple exon\n"); */
	    }
	    ajStrSetClear(&pos);
	    ajStrAppendS(&location,temp);
	    /* this is the parent/only feature */
	    if(!(gf->Flags & FEATFLAG_CHILD))
		gfprev=gf;
	}
	
	ajListIterDel(&iter);

	if(gfprev)
	{
	    /* Don't forget the last one !!! */
	    if(join)
	    {
		ajStrAppendC(&location,")");	/* close bracket for join */
		/* ajDebug("last: join: closing ')' appended\n"); */
	    }
	    if(whole)
	    {
		ajStrInsertC(&location,0,"complement(");
		ajStrAppendC(&location,")");
		/*
                ** ajDebug("last: wrap with complement(), reset whole %b "
                **            "to N\n", whole);
                */
		whole = ajFalse;
	    }
	
	    /* ajDebug("last: calling featDumpRefseq for gfprev\n"); */
	    /* ajDebug("location: '%S'\n", location); */
	
	    featDumpRefseq(gfprev, location, file,
                            thys->Seqid) ; /* gfprev has tag data */
	}

	ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
    }
    
    /* ajDebug("ajFeattableWriteRefseq Done\n"); */
    
    return ajTrue;
}




/* @funcstatic feattableWriteRefseqp ******************************************
**
** Write a feature table in REFSEQP format.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] file [AjPFile] Output file
* @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteRefseqp(const AjPFeattable thys, AjPFile file)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    AjPFeature gfprev   = NULL;
    AjBool join         = ajFalse;
    AjBool whole        = ajFalse;           /* has "complement(" been added */
    AjPStr location     = NULL;              /* location list as a string */
    AjPStr temp         = NULL;
    AjPStr pos          = NULL;
    ajuint oldgroup = UINT_MAX;
    
    /* Check arguments */
    
    /* ajDebug("feattableWriteRefseqp Checking arguments\n"); */

    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsNuc(thys))
	return ajFalse;
    
    /* feature table heading */
    
    ajFmtPrintF(file, "FEATURES             Location/Qualifiers\n");
    
    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*ajFeatCompByGroup);
    
    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

	    if((oldgroup != gf->Group) && gfprev) /* previous location ready */
	    {
		if(join)
		{
		    ajStrAppendC(&location,")"); /* close bracket for join */
		    /* ajDebug("join: closing ')' appended\n"); */
		}

		if(whole)
		{
		    ajStrInsertC(&location,0,"complement(");
		    ajStrAppendC(&location,")");
		    /* ajDebug("wrap with complement(), reset whole %b to N\n",
		       whole); */
		    whole = ajFalse;
		}
		
		/* ajDebug("calling featDumpRefseqp for gfprev\n"); */
		/* ajDebug("location: '%S'\n", location); */
		featDumpRefseqp(gfprev, location, file,
                                thys->Seqid); /* gfprev has tag data */
		
		/* reset the values from previous */
		/* ajDebug("reset location\n"); */
		ajStrSetClear(&location);
		/* ajDebug("reset join  %b to N\n", join); */
		join = ajFalse;
	    }
	    
	    oldgroup = gf->Group;
	    
	    /* process the new gf */
	    
	    /* ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	       gf->Type, gf->Group,gf->Exon, gf->Flags,
	       ajListGetLength(gf->Tags)); */
	    
	    if(gf->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    {
		/* ajDebug("set2 whole %b to Y\n", whole); */
		whole =ajTrue;
	    }
	    
	    if(ajStrGetLen(location))   /* one location already there */
	    {
		if(!join)
		{
		    /* ajDebug("insert 'join(', set join Y\n"); */
		    if(gf->Flags & FEATFLAG_ORDER)
			ajStrInsertC(&location,0,"order(");
		    else
			ajStrInsertC(&location,0,"join(");
		    join = ajTrue;
		}

		ajStrAppendC(&location,",");
		/* ajDebug("append ','\n"); */
	    }
	    
	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);
	    
	    if(gf->Flags & FEATFLAG_REMOTEID)
	    {
		ajFmtPrintAppS(&pos,"%S:",gf->Remote);
		/* ajDebug("remote: %S\n", gf->Remote); */
	    }
	    
	    if(gf->Flags & FEATFLAG_LABEL)
	    {
		ajFmtPrintAppS(&pos,"%S",gf->Label);
		/* ajDebug("label: %S\n", gf->Label); */
	    }
	    else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ)
	    {
		ajFmtPrintAppS(&pos,"<%d",gf->Start);
		/* ajDebug("<start\n"); */
	    }
	    else if(gf->Flags & FEATFLAG_START_TWO)
	    {
		ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
		/* ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2); */
	    }
	    else
	    {
		ajFmtPrintAppS(&pos,"%d",gf->Start);
		/* ajDebug("start\n"); */
	    }
	    
	    if(!(gf->Flags & FEATFLAG_POINT))
	    {
		if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
		{
		    ajFmtPrintAppS(&pos,"^%d",gf->End);
		    /* ajDebug("between ^end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_AFTER_SEQ)
		{
		    ajFmtPrintAppS(&pos,"..>%d",gf->End);
		    /* ajDebug(">end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_TWO)
		{
		    ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
		    /* ajDebug("end2 (%d.%d)\n", gf->End2, gf->End); */
		}
		else
		{
		    ajFmtPrintAppS(&pos,"..%d",gf->End);
		    /* ajDebug(".. end\n"); */
		}
	    }
	    
	    if(gf->Strand == '-' && !whole)
	    {
		ajStrAssignC(&temp,"complement(");
		ajStrAppendS(&temp,pos);
		ajStrAppendC(&temp,")");
		/* ajDebug("strand '-', wrap exon with complement()\n"); */
	    }
	    else 
	    {
		ajStrAssignS(&temp,pos);
		/* ajDebug("simple exon\n"); */
	    }
	    ajStrSetClear(&pos);
	    ajStrAppendS(&location,temp);
	    /* this is the parent/only feature */
	    if(!(gf->Flags & FEATFLAG_CHILD))
		gfprev=gf;
	}
	
	ajListIterDel(&iter);

	if(gfprev)
	{
	    /* Don't forget the last one !!! */
	    if(join)
	    {
		ajStrAppendC(&location,")");	/* close bracket for join */
		/* ajDebug("last: join: closing ')' appended\n"); */
	    }
	    if(whole)
	    {
		ajStrInsertC(&location,0,"complement(");
		ajStrAppendC(&location,")");
		/*
                ** ajDebug("last: wrap with complement(), reset whole %b "
                **            "to N\n", whole);
                */
		whole = ajFalse;
	    }
	
	    /* ajDebug("last: calling featDumpRefseqp for gfprev\n"); */
	    /* ajDebug("location: '%S'\n", location); */
	
	    featDumpRefseqp(gfprev, location, file,
                            thys->Seqid) ; /* gfprev has tag data */
	}

	ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
    }
    
    /* ajDebug("ajFeattableWriteRefseqp Done\n"); */
    
    return ajTrue;
}




/* @func ajFeattableWriteSwiss ************************************************
**
** Write a feature table in SwissProt format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteSwiss(AjPFeattabOut ftout,
                             const AjPFeattable thys)
{
    AjIList iter     = NULL;
    AjIList itersub  = NULL;
    AjPFeature gf    = NULL;
    AjPFeature gfsub = NULL;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    ajDebug("ajFeattableWriteSwiss Checking arguments\n");

    if(!file)
	return ajFalse;

    if(!ajFeattableIsProt(thys))
	return ajFalse;
    
    /* no FH header in SwissProt */

    /* For all features... */

    if(!thys->Features)
        return ajTrue;
    
    iter = ajListIterNewread(thys->Features);

    while(!ajListIterDone(iter))
    {
        gf = ajListIterGet(iter);

        if(ajListGetLength(gf->Subfeatures))
        {
            itersub = ajListIterNewread(gf->Subfeatures);

            while(!ajListIterDone(itersub))
            {
                gfsub = ajListIterGet(itersub);

                featDumpSwiss(gfsub, file, gf);
            }
            ajListIterDel(&itersub);
        }
        else
        {
            featDumpSwiss(gf, file, gf);
	}

    }
    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajFeattableWritePir **************************************************
**
** Write a feature table in PIR format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWritePir(AjPFeattabOut ftout, const AjPFeattable thys)
{
    AjIList iter      = NULL;
    AjIList itersub   = NULL;
    AjPFeature gf     = NULL;
    AjPFeature gfsub  = NULL;
    AjPStr location   = NULL;        /* location list as a string */
    AjPStr pos        = NULL;
    AjPFile file = ftout->Handle;
/*
    const AjPTable tagstable = NULL;
    const AjPTable typetable = NULL;
*/
    /* Check arguments */
    
    /*ajDebug("ajFeattableWritePir Checking arguments\n");*/
    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsProt(thys))
	return ajFalse;
    
    if(!thys->Features)
        return ajTrue;
    
    /* tagstable = ajFeatVocabGetTagsProt("pir");  */
    /* typetable = ajFeatVocabGetTypesProt("pir"); */

    location = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*ajFeatCompByGroup);
    
    iter = ajListIterNewread(thys->Features);
	
    while(!ajListIterDone(iter))
    {
        gf = ajListIterGet(iter);
        ajStrSetClear(&location);
	    
        if(ajListGetLength(gf->Subfeatures))
        {
            itersub = ajListIterNewread(gf->Subfeatures);

            while(!ajListIterDone(itersub))
            {
                gfsub = ajListIterGet(itersub);

                if(ajStrGetLen(location))
                    ajStrAppendC(&location,",");

                ajFmtPrintS(&pos,"%d",gfsub->Start);
                if(gfsub->End != gfsub->Start)
                    ajFmtPrintAppS(&pos,"-%d",gfsub->End);

                ajStrAppendS(&location,pos);
            }
            featDumpPir(gf, location, file);
            ajListIterDel(&itersub);
        }
        else
        {
	    ajFmtPrintS(&pos,"%d",gf->Start);
	    if(gf->End != gf->Start)
		ajFmtPrintAppS(&pos,"-%d",gf->End);
	    ajStrAppendS(&location,pos);
            featDumpPir(gf, location, file);
	}
    }

    ajListIterDel(&iter);
    ajStrDel(&location);
    ajStrDel(&pos);

    /*ajDebug("ajFeattableWritePir Done\n");*/
    
    return ajTrue;
}




/* @func ajFeattableWriteDasgff ***********************************************
**
** Write a feature table in DAS GFF format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteDasgff(AjPFeattabOut ftout, const AjPFeattable thys)
{
    /*
    ** Things to do:
    **
    ** Where do the tag-values go? In <NOTE> for now
    **
    ** Check standards for id, type etc. for table and features
    */

/*#define MULTINOTE 1*/
    AjPFile file = ftout->Handle;
    AjIList iter     = NULL;
    AjIList tagiter     = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    AjPTagval item = NULL;
    AjPFeature gf    = NULL;
/*    AjPFeature gftop = NULL; */
    const AjPTable tagstable = NULL;
    const AjPTable typetable = NULL;
    ajuint oldgroup   = UINT_MAX;
    ajuint nfeat = 0;
    char frame;
    AjBool knowntag = ajTrue;

    AjPStr mytagname = NULL;
    AjPStr mytagval = NULL;
#ifdef MULTINOTE
    ajuint ntags;
    ajuint itag;
#endif
    const char* cp;

    /* Check arguments */
    ajDebug("ajFeattableWriteDasgff Checking arguments\n");

    if(!file)
	return ajFalse;

    if(!ftout->Count)
    {
        ftout->Cleanup = featCleanDasgff;
        ajFmtPrintF(file,
                    "<?xml version=\"1.0\" standalone=\"no\"?>\n");
/* deleted in DAS 1.6
        ajFmtPrintF(file,
                    "<!DOCTYPE DASGFF SYSTEM \"http://www.biodas.org/dtd"
                    "/dasgff.dtd\">\n");
*/
        ajFmtPrintF(file,
                    "<DASGFF>\n");
        ajFmtPrintF(file,
                    "  <GFF version=\"1.0\" href=\"url\">\n");
    }

    ajFmtPrintF(file,
                "    <SEGMENT id=\"%S\" start=\"%d\" stop=\"%d\"\n",
                thys->Seqid,
                1+thys->Offset+thys->Start,
                1+thys->Offset+thys->End);

    /* label is optional */
    /*
    ajFmtPrintF(outseq->File,
                    "                label=\"%s\">", "");
    */

    /* type attribute is optional and may not mean this */
    /*
    if(ajFeattableIsProt(thys))
        ajFmtPrintF(outseq->File,
                    " type=\"Protein\">");
    else
        ajFmtPrintF(outseq->File,
                    " type=\"DNA\">");
    */
    ajFmtPrintF(file,
                "                version=\"%s\">\n",
                "0.0");

    
    /* For all features... */

    /* SEGMENT requires at least one feature - if none, make one up */

    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
            nfeat++;

            if(gf->Protein)
            {
                typetable = ajFeatVocabGetTypesProt("gff3protein");
                tagstable = ajFeatVocabGetTagsProt("gff3protein");
                ajDebug("ajFeattableWriteDasgff GFF3 protein table\n");
            }
            else
            {
                typetable = ajFeatVocabGetTypesNuc("gff3");
                tagstable = ajFeatVocabGetTagsNuc("gff3");
                ajDebug("ajFeattableWriteDasgff GFF3 nucleotide table\n");
            }

            outtyp = ajFeattypeGetExternal(gf->Type, typetable);

	    if(oldgroup != gf->Group)
	    {
		oldgroup = gf->Group;
/*		gftop = gf; */
	    }
	    else
	    {
#if 0
		if(!(gf->Flags & FEATFLAG_CHILD))
		    gftop = gf; /* this is the parent/only feature */
#endif
	    }

            ajFmtPrintF(file,
                        "      <FEATURE id=\"%S.%S.%d\">\n",
                        thys->Seqid, gf->Source, gf->Group);

            ajFeatGetTagC(gf,"category",1, &mytagval);

            if(ajStrGetLen(mytagval)==0)
        	ajStrAssignS(&mytagval, ajFeatTypeGetCategory(gf->Type));

            ajFmtPrintF(file,
                        "         <TYPE id=\"%S\" category=\"%S\" "
                        "reference=\"no\" subparts=\"no\">%S</TYPE>\n",
                        gf->Type, mytagval, outtyp);
            ajStrDel(&mytagval);

            /*
            ** METHOD id should be something the server can return
            ** For application results this is the application name
            */
            if(ajStrGetLen(gf->Source))
                ajFmtPrintF(file,
                            "         <METHOD id=\"%S\"></METHOD>\n",
                            gf->Source);
            else
                ajFmtPrintF(file,
                        "         <METHOD></METHOD>\n",
                        gf->Source);

            /*
            ** START and END are optional in DAS 1.6 but we always have them
            */
            ajFmtPrintF(file,
                        "         <START>%d</START>\n",
                        gf->Start);
            ajFmtPrintF(file,
                        "         <END>%d</END>\n",
                        gf->End);
            /*
            ** SCORE is optional in DAS 1.6
            **
            ** we don't know a value 'zero' means actual 'zero'
            ** or 'inapplicable',
            ** looks safer not printing the tag when the score is zero
            */
            if(!E_FPZERO(gf->Score,U_FEPS))
        	ajFmtPrintF(file,
        	            "         <SCORE>%.f</SCORE>\n",
        	            gf->Score);
            /*
            ** ORIENTATION is optional in DAS 1.6
            ** for proteins or non-transcriptional features
            */
            if(gf->Strand)
                ajFmtPrintF(file,
                            "         <ORIENTATION>%c</ORIENTATION>\n",
                            gf->Strand); /* 0 or - or + */

            else
                ajFmtPrintF(file,
                            "         <ORIENTATION>0</ORIENTATION>\n");

            /*
            ** PHASE is optional in DAS 1.6
            ** for proteins or non-translational features
            */
            if(ajFeatTypeIsCds(gf))
                frame = ajFeatframeGetFrameNuc(gf->Frame);
            else
                frame = ajFeatframeGetFrame(gf->Frame);

            if(gf->Frame == 0)
                ajFmtPrintF(file,
                            "         <PHASE>-</PHASE>\n");
            else
                ajFmtPrintF(file,
                            "         <PHASE>%c</PHASE>\n",
                            frame);

            /*
            ** tag-value pairs are written as NOTES
            ** We write one note for each
            ** This displays well in various DAS client browsers
            */
            if(gf->Tags)
            {
#ifdef MULTINOTE
                ntags = ajListGetLength(gf->Tags);
                itag = 0;
#endif
                tagiter = ajListIterNewread(gf->Tags);

                while(!ajListIterDone(tagiter))
                {
                    item = (AjPTagval)ajListIterGet(tagiter);
                    outtag = ajFeattagGetNameS(item->Tag, tagstable, &knowntag);

                    if(!outtag)
                    {
                        ajFeatWarn("Unknown GFF3 feature tag '%S'",
                                 item->Tag);
                        continue;
                    }

                    ajFeattagFormat(outtag, tagstable, &featoutFmtTmp);

#ifdef MULTINOTE
                    if(ntags == 1)
                    {
                        ajFmtPrintF(file,
                                        "         <NOTE>");
                    }
                    else 
                    {
                        if(!itag)
                            ajFmtPrintF(file,
                                        "         <NOTE>\n");
                    }
#else
                    ajFmtPrintF(file,
                                "         <NOTE>");
#endif                    

                    ajFmtPrintS(&featoutStr, "%S", outtag);
                    ajStrAssignS(&featoutValTmp, item->Value);
                    cp = ajStrGetPtr(featoutFmtTmp);

                    switch(CASE2(cp[0], cp[1]))
                    {
                        case CASE2('L','I') :  /* limited */
                            /*ajDebug("case limited\n");*/
                            ajFeattagGetLimit(outtag, tagstable,
                                              &featoutLimTmp);
                            featTagAllLimit(&featoutValTmp, featoutLimTmp);
                            ajFmtPrintAppS(&featoutStr, ":%S", featoutValTmp);
                            break;
                        case CASE2('Q', 'L') : /* limited, escape quotes */
                            /*ajDebug("case qlimited\n");*/
                            ajFeattagGetLimit(outtag, tagstable,
                                              &featoutLimTmp);
                            featTagAllLimit(&featoutValTmp, featoutLimTmp);
                            /*featTagQuoteGff3(&featoutValTmp);*/
                            ajFmtPrintAppS(&featoutStr, ":%S", featoutValTmp);
                            break;
                        case CASE2('T','E') :/* no space/quotes, wrap margin */
                            /*ajDebug("case text\n");*/
                            ajStrRemoveWhite(&featoutValTmp);
                            ajFmtPrintAppS(&featoutStr, ":%S", featoutValTmp);
                            break;
                        case CASE2('Q','T') :/* escape quotes, wrap at space */
                            /*ajDebug("case qtext\n");*/
                            /*featTagQuoteGff3(&featoutValTmp);*/

                            if(ajStrMatchC(outtag, "note") &&
                               ajStrGetCharFirst(featoutValTmp) == '*')
                            {
                                ajStrCutStart(&featoutValTmp,1);
                                ajStrExtractWord(featoutValTmp, &mytagval,
                                                 &mytagname);
                                ajFmtPrintS(&featoutStr, "%S:%S",
                                            mytagname, mytagval);
                                ajStrDel(&mytagname);
                                ajStrDel(&mytagval);
                            }
                            else
                                ajFmtPrintAppS(&featoutStr, ":%S",
                                               featoutValTmp);

                            break;
                        case CASE2('Q','W') :/* escape quotes, remove space */
                            /*ajDebug("case qtext\n");*/
                            /*featTagQuoteGff3(&featoutValTmp);*/
                            ajStrRemoveWhite(&featoutValTmp);
                            ajFmtPrintAppS(&featoutStr, ":%S", featoutValTmp);
                            break;
                        case CASE2('Q', 'S') : /* special regexp, quoted */
                            /*ajDebug("case qspecial\n");*/

                            if(!ajFeattagSpecialGff3(outtag, &featoutValTmp))
                                featTagDasgffDefault(&featoutStr, outtag,
                                                     &featoutValTmp);
                             else
                            {
                                /*featTagQuoteGff3(&featoutValTmp);*/
                                ajFmtPrintAppS(&featoutStr, ":%S",
                                               featoutValTmp);
                            }

                            break;
                        case CASE2('S','P') :	/* special regexp */
                            /*ajDebug("case special\n");*/

                            if(!ajFeattagSpecialGff3(outtag, &featoutValTmp))
                                featTagDasgffDefault(&featoutStr, outtag,
                                                     &featoutValTmp);
                            else
                                ajFmtPrintAppS(&featoutStr, ":%S",
                                               featoutValTmp);
                            
                            break;
                        case CASE2('V','O') :	/* no value, so an error here */
                            /*ajDebug("case void\n");*/
                            break;
                        default:
                            ajFeatWarn("Unknown GFF3 feature tag type '%S' "
                                     "for '%S'",
                                     featoutFmtTmp, outtag);
                    }
                    
/* TESTING: for single NOTE write ';' after each tag-value pair */

#ifdef MULTINOTE
                    if(++itag < ntags)
                        ajFmtPrintAppS(&featoutStr,";");

                    if(ntags == 1)
                    {
                        ajFmtPrintF(file,
                                    "%S</NOTE>\n",featoutStr);

                    }
                    else 
                    {
                        ajFmtPrintF(file, "           %S\n", featoutStr);

                        if(itag == ntags)
                            ajFmtPrintF(file,
                                        "         </NOTE>\n");

                    }
#else
                    ajFmtPrintF(file,
                                "%S</NOTE>\n",featoutStr);
#endif
                    
                }
                ajListIterDel(&tagiter);
                
            }

            /* link to more information about the feature */
            /*
            ajFmtPrintF(file,
                        "         <LINK href=\"%s\">%s</LINK>\n",
                        "url-here", "text-here);
            */
            
            /* target in an alignment */
            /*
            ajFmtPrintF(file,
                        "         <TARGET id\"%s\" start=\"%d\" "
                        "stop=\"%d\">%s</TARGET>\n",
                        "target-id", target->start, target->stop,"target-name");
            */


/* GROUP is deprecated in DAS 1.6 - replaced by PARENT and PART */
/*           ajFmtPrintF(file,
                       "         <GROUP id=\"%S.%d\" label=\"%S.%d\" "
                       "type=\"%S.%d\">\n",
                       thys->Seqid, gf->Group,
                       thys->Seqid, gf->Group,
                       thys->Seqid, gf->Group);*/
/* link to more GROUP information */
/*
            ajFmtPrintF(file,
                        "               <LINK href=\"%s\">%s</LINK>\n",
                        "url-here", "text-here);
*/

/* target in a GROUP alignment */
/*
            ajFmtPrintF(file,
                        "               <TARGET id\"%s\" start=\"%d\" "
                        "stop=\"%d\">%s</TARGET>\n",
                        "target-id", target->start, target->stop,"target-name");
           ajFmtPrintF(file,
                       "         </GROUP>\n");
*/

           ajFmtPrintF(file,
                        "      </FEATURE>\n");
 	}
	ajListIterDel(&iter) ;
    }

    /* it seems unclear whether DASGFF allows empty segments
    ** this block can be used if some dummy features is mandatory
    ** but note that any feature will appear in a DASGFF viewer
    ** unless there is some official dummy type available
    **
    ** Common practice is to ignore the DTD and write an empty SEGMENT
    ** so this section should not be needed. DAS 1.6 will correct the spec
    */

/*
  if(!nfeat)
    {
            ajFmtPrintF(file,
                        "      <FEATURE id=\"%S\">\n",
                        thys->Seqid);

            ajFmtPrintF(file,
                        "         <TYPE id=\"%s\" category=\"%s\" "
                        "reference=\"no\" subparts=\"no\">%s</TYPE>\n",
                        "dummy", "feature", "dummy)";
            if(ajStrGetLen(gf->Source))
                ajFmtPrintF(file,
                            "         <METHOD id=\"%S\"></METHOD>\n",
                            gf->Source);
            else
                ajFmtPrintF(file,
                        "         <METHOD></METHOD>\n");
            ajFmtPrintF(file,
                        "         <START>1</START>\n");
            ajFmtPrintF(file,
                        "         <END>%d</END>\n",
                        thys->Len);
            ajFmtPrintF(file,
                        "         <SCORE>0.0</SCORE>\n");
            ajFmtPrintF(file,
                        "         <ORIENTATION>0</ORIENTATION>\n");
            ajFmtPrintF(file,
                        "         <PHASE>-</PHASE>\n");
            ajFmtPrintF(file,
                        "         <NOTE>%s</NOTE>\n",
                        "Written by EMBOSS");
            ajFmtPrintF(file,
                        "      </FEATURE>\n");
    }
*/   
    ajFmtPrintF(file,
                "    </SEGMENT>\n");

    return ajTrue;
}




/* @funcstatic featCleanDasgff ************************************************
**
** Writes the remaining lines to complete and close a DASGFF XML file
**
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/


static void featCleanDasgff(AjPFile file)
{

    ajFmtPrintF(file,
                "  </GFF>\n");
    ajFmtPrintF(file,
                "</DASGFF>\n");

    return;
}




/* @func ajFeattabOutSetType **************************************************
**
** Sets the type for feature output
**
** @param [u] thys [AjPFeattabOut] Feature output object
** @param [r] type [const AjPStr] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSetType(AjPFeattabOut thys, const AjPStr type)
{
    return ajFeattabOutSetTypeC(thys, ajStrGetPtr(type));
}




/* @func ajFeattabOutSetTypeC *************************************************
**
** Sets the type for feature output
**
** @param [u] thys [AjPFeattabOut] Feature output object
** @param [r] type [const char*] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSetTypeC(AjPFeattabOut thys, const char* type)
{
    ajint i = 0;

    if(!*type)
	return ajTrue;

    for(i=0; featOutTypes[i].Name; i++)
    {
	if(ajCharMatchCaseC(featOutTypes[i].Name, type))
	{
	    if(featOutTypes[i].Value)
		ajStrAssignC(&thys->Type, featOutTypes[i].Value);
	    return ajTrue;
	}
    }

    ajErr("Unrecognised feature output type '%s'", type);

    return ajFalse;
}




/* @funcstatic featLocEmblWrapC ***********************************************
**
** Splits EMBL feature location at the last possible comma
** and adds the appropriate prefix (e.g. the EMBL FT line type)
**
** @param  [u] Ploc [AjPStr*] location as a string
** @param  [r] margin [ajuint] Right margin
** @param  [r] prefix [const char*] Left margin prefix string
** @param  [r] preftyp [const char*] Left margin prefix string for first line
**                            (includes the feature key)
** @param [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featLocEmblWrapC(AjPStr *Ploc, ajuint margin,
			     const char* prefix, const char* preftyp,
			     AjPStr* retstr)
{
    ajint left  = 0;
    ajint width = 0;
    ajlong len   = 0L;
    ajlong i;
    ajlong j;
    ajlong k;
    ajlong last;

    left = strlen(prefix);
    width = margin - left;	    /* available width for printing */

    ajStrRemoveWhite(Ploc);	     /* no white space in locations */
    len = ajStrGetLen(*Ploc);

    k = width;			/* for safety - will be set in time */

    ajDebug("featLocEmblWrapC %Ld <%d> '%S'\n", len, width, *Ploc);
    for(i=0; i < len; i+= k)
    {
	last = i + width - 1;

	ajDebug("try %Ld to %Ld (len %Ld)\n", i, last, len);

	if((last+1) >= len)		/* no need to split */
	{
	    ajStrAssignSubS(&featoutTmpStr, *Ploc, i, len-1);
	    ajDebug("last %Ld >= len %Ld\n", last, len);
	    j = 0;
	}
	else
	{
	    ajStrAssignSubS(&featoutTmpStr, *Ploc, i, last); /* save max string */
	    j = ajStrFindlastC(featoutTmpStr, ","); /* last comma in featoutTmpStr */
	    ajDebug("comma at %Ld\n", j);
	}

	if(j < 1)			/* no comma found */
	{
	    ajDebug("no comma j=%Ld k=%u '%S'\n",
		    j, ajStrGetLen(featoutTmpStr), featoutTmpStr);
	    j = ajStrGetLen(featoutTmpStr);
	    k = j;
	}
	else
	{				/* print up to last comma */
	    j++;
	    k = j;			/* start after the comma */
	}
	ajDebug("%Ld +%Ld k=%Ld featoutTmpStr: '%.*S'\n",
		i, j, k, (ajuint) j, featoutTmpStr);
	if(!i)
	  ajFmtPrintAppS(retstr, "%s%.*S\n", preftyp,(ajuint)j, featoutTmpStr);
	else
	  ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,(ajuint)j, featoutTmpStr);
    }

    return;
}




/* @funcstatic featTagEmblWrapC ***********************************************
**
** Splits feature table output at the last possible space (or
** the last column if there are no spaces) and adds the appropriate
** prefix (e.g. the EMBL FT line type)
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] margin [ajuint] Right margin
** @param  [r] prefix [const char*] Left margin prefix string
** @param  [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagEmblWrapC(AjPStr *pval, ajuint margin, const char* prefix,
			     AjPStr* retstr)
{
    ajint left  = 0;
    ajint width = 0;
    ajlong len   = 0L;
    ajlong i;
    ajlong j;
    ajlong k;
    ajlong last;

    left = strlen(prefix);
    width = margin - left;	    /* available width for printing */

    ajStrRemoveWhiteExcess(pval);	/* single spaces only */
    len = ajStrGetLen(*pval);

    k = width;			/* for safety - will be set in time */

    /* ajDebug("featTagEmblWrapC %Ld <%Ld> '%S'\n", len, width, *pval); */

    for(i=0; i < len; i+= k)
    {
	last = i + width - 1;

	/* ajDebug("try %Ld to %Ld (len %Ld)\n", i, last, len); */

	if((last+1) >= len)		/* no need to split */
	{
	    ajStrAssignSubS(&featoutTmpStr, *pval, i, len-1);
	    /* ajDebug("last %Ld >= len %Ld\n", last, len); */
	    j = 0;
	}
	else if(ajStrGetCharPos(*pval, (last+1)) == ' ') /* split at max width */
	{
	    ajStrAssignSubS(&featoutTmpStr, *pval, i, last);
	    j = last + 1 - i;
	}
	else
	{
	    ajStrAssignSubS(&featoutTmpStr, *pval, i, last); /* save max string */
	    j = ajStrFindlastC(featoutTmpStr, " "); /* last space in featoutTmpStr */
	    /* ajDebug("space at %Ld\n", j); */
	}

	if(j < 1)			/* no space found */
	{
	    j = ajStrGetLen(featoutTmpStr);
	    k = j;
	}
	else				/* print up to last space */
	{
	    k = j + 1;			/* start after the space */
	}

	/* ajDebug("%Ld +%Ld '%.*S'\n", i, j, (ajuint)j, featoutTmpStr); */
	ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,(ajuint)j, featoutTmpStr);
    }

    return;
}




/* @funcstatic featTagSwissWrapC **********************************************
**
** Splits feature table output at the last possible space (or
** the last column if there are no spaces) and adds the appropriate
** prefix (e.g. the SwissProt FT line type)
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] margin [ajuint] Right margin
** @param  [r] prefix [const char*] Left margin prefix string
** @param  [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagSwissWrapC(AjPStr *pval, ajuint margin, const char* prefix,
			      AjPStr* retstr)
{
    ajuint left  = 0;
    ajuint width = 0;
    ajuint len   = 0;
    ajulong i;
    ajulong j;
    ajulong k;
    ajlong iftid = -1;
    AjBool isftidstart = ajFalse;
/*    AjBool isftid = ajFalse; */

    AjPStr valstr = NULL;
    ajulong last;
    
    left = strlen(prefix);
    width = margin - left;	/* available width for printing */
    
    k = width; /* will be reset in the loop */
    
    ajDebug("featTagSwissWrapC %d <%d> '%S'\n",
       ajStrGetLen(*pval), width, *pval);
    
    if(ajStrGetLen(*pval) <= left)	/* no need to wrap */
    {
	ajStrAssignS(retstr, *pval);
	ajStrAppendK(retstr, '\n');
	/*ajDebug("simple '%S'\n", *retstr);*/

	return;
    }   

    ajStrAssignSubS(retstr, *pval, 0, left-1);
    ajStrAssignSubS(&valstr, *pval, left, -1);
    len = ajStrGetLen(valstr);
    /*ajDebug("rest '%S'\n", valstr);*/
    
    if(ajStrPrefixC(valstr, "/FTId="))
    {
        ajStrTrimEndC(retstr, " ");
        ajStrAppendK(retstr, '\n');
        isftidstart = ajTrue;
    }
    

    for(i=0; i < len; i+= k)
    {
	last = i + width - 1;

        ajStrAssignSubS(&featoutTmpStr, valstr, (ajlong) i, len-1);
        iftid = ajStrFindC(featoutTmpStr, " /FTId=");
/*        isftid = ajFalse; */
	/*ajDebug("try %d to %d (len %d) iftid:%d\n", i, last, len, iftid);*/

        if(iftid >= 0 && iftid+(ajint)i <= (ajint)last)
        {
            if(iftid)
            {
/*                isftid = ajTrue; */
	      ajStrAssignSubS(&featoutTmpStr, valstr, (ajlong) i,
			      (ajlong) i+iftid-1);
                j = iftid;
            }
            else
            {
	      ajStrAssignSubS(&featoutTmpStr, valstr, (ajlong) i,
			      (ajlong) i+iftid-2);
                j = 0;
            }
            
	    /*ajDebug("' /FTId=' found iftid:%d isftid:%B, j:%d\n",
	      iftid, isftid, j);*/
        }
	else if((last+1) >= len)		/* no need to split */
	{
	  ajStrAssignSubS(&featoutTmpStr, valstr, (ajlong) i, len-1);
	    /*ajDebug("last %d >= len %d\n", last, len);*/
	    j = 0;
	}
        /* split at max width */
	else if(ajStrGetCharPos(valstr, (ajlong) (last+1)) == ' ')
	{
	  ajStrAssignSubS(&featoutTmpStr, valstr, (ajlong) i, (ajlong) last);
	    j = width;
	    /*ajDebug("split at max width last+1 %d\n", last+1);*/
	}
	else
	{
             /* save max string */
	  ajStrAssignSubS(&featoutTmpStr, valstr, (ajlong) i, (ajlong) last);
            /* last space in featoutTmpStr */
	    j = ajStrFindlastC(featoutTmpStr, " ");
	    /*ajDebug("space at %d\n", j);*/
	}

	if(j < 1)			/* no space found */
	{
	    j = ajStrGetLen(featoutTmpStr);
	    k = j;
	}
	else				/* print up to last space */
	{
	    k = j + 1;			/* start after the space */
	}
	/*ajDebug("%Ld +%Ld (%Ld) '%.*S'\n",
		  i, j, k, (ajuint)j, featoutTmpStr);*/

	if(i || isftidstart)
	  ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,(ajuint)j, featoutTmpStr);
	else
	  ajFmtPrintAppS(retstr, "%.*S\n", (ajuint)j, featoutTmpStr);
    }

    ajStrDel(&valstr);

    return;
}




/* @funcstatic featDumpEmbl ***************************************************
**
** Write details of single feature to file in EMBL/GenBank/DDBJ format
**
** @param [r] feat     [const AjPFeature] Feature
** @param [r] location [const AjPStr] location list
** @param [u] file     [AjPFile] Output file
** @param [r] Seqid    [const AjPStr] Sequence ID
** @param [r] IsEmbl   [AjBool] ajTrue if writing EMBL format (FT prefix)
** @return [void]
** @@
******************************************************************************/

static void featDumpEmbl(const AjPFeature feat, const AjPStr location,
			 AjPFile file, const AjPStr Seqid, AjBool IsEmbl)
{
    AjIList iter   = NULL;
    ajint i        = 0;
    AjPTagval tv = NULL;
    const AjPStr tmptyp  = NULL;		/* these come from AjPTable */
    const AjPStr tmptag  = NULL;		/* so please, please */
    /* don't delete them */
    AjPStr tmplim = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr           = NULL;
    AjPStr preftyploc = NULL;
    AjPStr preftyptag = NULL;
    AjPStr tmploc     = NULL;
    const AjPTable tagstable; 
    const AjPTable typetable; 

    ajDebug("featDumpEmbl '%S'\n", location);
    
    /* print the location */
    
    tagstable = ajFeatVocabGetTagsNuc("embl");
    typetable = ajFeatVocabGetTypesNuc("embl");

    ajStrAssignS(&tmploc, location);
    tmptyp = ajFeattypeGetExternal(feat->Type, typetable);
    
    if(IsEmbl)
    {
	ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "FT", tmptyp);
	ajFmtPrintS(&preftyptag, "%s                   ", "FT");
    }
    else
    {
	ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
	ajFmtPrintS(&preftyptag, "%s                   ", "  ");
    }
    
    if(IsEmbl)
	featLocEmblWrapC(&tmploc, 80,
			 ajStrGetPtr(preftyptag),
			 ajStrGetPtr(preftyploc), &wrapstr);
    else
	featLocEmblWrapC(&tmploc, 79,
			 ajStrGetPtr(preftyptag),
			 ajStrGetPtr(preftyploc), &wrapstr);

    /*ajDebug("tmploc: '%S' wrapstr; '%S'\n", tmploc, wrapstr);*/
    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDel(&wrapstr);
    
    /* print the qualifiers */
    
    iter = ajListIterNewread(feat->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	++i;
	tmptag = ajFeattagGetNameS(tv->Tag, tagstable, &knowntag);
	ajFeattagFormat(tmptag, tagstable, &featoutFmtTmp);
	/* ajDebug(" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
	/* ajDebug(" %3d  %S format: '%S'\n", i, tmptag, featoutFmtTmp); */
	ajFmtPrintS(&featoutStr, "/%S", tmptag);

	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);
	    cp = ajStrGetPtr(featoutFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :		/* limited */
		/* ajDebug("case limited\n"); */
		ajFeattagGetLimit(tmptag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/* ajDebug("case qlimited\n"); */
		ajFeattagGetLimit(tmptag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		featTagQuoteEmbl(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/* ajDebug("case qspecial\n"); */
		if(!ajFeattagSpecial(tmptag, &featoutValTmp))
		{
		    ajFeatWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featoutStr, tmptag, &featoutValTmp);
		}
		else
		{
		    featTagQuoteEmbl(&featoutValTmp);
		    ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/* ajDebug("case special\n"); */
		if(!ajFeattagSpecial(tmptag, &featoutValTmp))
		{
		    ajFeatWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featoutStr, tmptag, &featoutValTmp);
		}
		else
		    ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);

		break;
	    case CASE2('T','E') :     /* no space, no quotes, wrap at margin */
		/* ajDebug("case text\n"); */
		ajStrRemoveWhite(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    case CASE2('V','O') :	     /* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    case CASE2('Q','T') :	   /* escape quotes, wrap at space */
		/* ajDebug("case qtext\n"); */
		featTagQuoteEmbl(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    case CASE2('Q','W') :	   /* escape quotes, remove space */
		/* ajDebug("case qword\n"); */
		featTagQuoteEmbl(&featoutValTmp);
		ajStrRemoveWhite(&featoutValTmp); /* no white space needed */
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    default:
		ajFeatWarn("Unknown EMBL feature tag type '%S' for '%S'",
                           featoutFmtTmp, tmptag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featoutFmtTmp);*/
	}

	if(IsEmbl)
	    featTagEmblWrapC(&featoutStr, 80,
			     ajStrGetPtr(preftyptag), &wrapstr);
	else
	    featTagEmblWrapC(&featoutStr, 79,
			     ajStrGetPtr(preftyptag), &wrapstr);

	ajFmtPrintF(file, "%S", wrapstr);
	ajStrDel(&wrapstr);
    }
    
    /* ajDebug("featDumpEmbl Done %d tags\n", i); */
    
    ajListIterDel(&iter);
    ajStrDel(&preftyploc);
    ajStrDel(&preftyptag);
    ajStrDel(&tmploc);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpRefseq *************************************************
**
** Write details of single feature to file in Refseq format
**
** @param [r] feat     [const AjPFeature] Feature
** @param [r] location [const AjPStr] location list
** @param [u] file     [AjPFile] Output file
** @param [r] Seqid    [const AjPStr] Sequence ID
** @return [void]
** @@
******************************************************************************/

static void featDumpRefseq(const AjPFeature feat, const AjPStr location,
                           AjPFile file, const AjPStr Seqid)
{
    AjIList iter   = NULL;
    ajint i        = 0;
    AjPTagval tv = NULL;
    const AjPStr tmptyp  = NULL;		/* these come from AjPTable */
    const AjPStr tmptag  = NULL;		/* so please, please */
    /* don't delete them */
    AjPStr tmplim = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr           = NULL;
    AjPStr preftyploc = NULL;
    AjPStr preftyptag = NULL;
    AjPStr tmploc     = NULL;
    const AjPTable tagstable; 
    const AjPTable typetable; 
    
    /* ajDebug("featDumpRefseq Start\n"); */
    
    /* print the location */
    
    tagstable = ajFeatVocabGetTagsNuc("embl");
    typetable = ajFeatVocabGetTypesNuc("embl");

    ajStrAssignS(&tmploc, location);
    tmptyp = ajFeattypeGetExternal(feat->Type, typetable);
    
    ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
    ajFmtPrintS(&preftyptag, "%s                   ", "  ");

    
    featLocEmblWrapC(&tmploc, 79,
                     ajStrGetPtr(preftyptag),
                     ajStrGetPtr(preftyploc), &wrapstr);

    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDel(&wrapstr);
    
    /* print the qualifiers */
    
    iter = ajListIterNewread(feat->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	++i;
	tmptag = ajFeattagGetNameS(tv->Tag, tagstable, &knowntag);
	ajFeattagFormat(tmptag, tagstable, &featoutFmtTmp);
	/* ajDebug(" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
	/* ajDebug(" %3d  %S format: '%S'\n", i, tmptag, featoutFmtTmp); */
	ajFmtPrintS(&featoutStr, "/%S", tmptag);

	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);
	    cp = ajStrGetPtr(featoutFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :		/* limited */
		/* ajDebug("case limited\n"); */
		ajFeattagGetLimit(tmptag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/* ajDebug("case qlimited\n"); */
		ajFeattagGetLimit(tmptag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		featTagQuoteEmbl(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/* ajDebug("case qspecial\n"); */
		if(!ajFeattagSpecial(tmptag, &featoutValTmp))
		{
		    ajFeatWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featoutStr, tmptag, &featoutValTmp);
		}
		else
		{
		    featTagQuoteEmbl(&featoutValTmp);
		    ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/* ajDebug("case special\n"); */
		if(!ajFeattagSpecial(tmptag, &featoutValTmp))
		{
		    ajFeatWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featoutStr, tmptag, &featoutValTmp);
		}
		else
		    ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);

		break;
	    case CASE2('T','E') :     /* no space, no quotes, wrap at margin */
		/* ajDebug("case text\n"); */
		ajStrRemoveWhite(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    case CASE2('V','O') :	     /* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    case CASE2('Q','T') :	   /* escape quotes, wrap at space */
		/* ajDebug("case qtext\n"); */
		featTagQuoteEmbl(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    case CASE2('Q','W') :	   /* escape quotes, remove space */
		/* ajDebug("case qword\n"); */
		featTagQuoteEmbl(&featoutValTmp);
		ajStrRemoveWhite(&featoutValTmp);	/* no white space needed */
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    default:
		ajFeatWarn("Unknown EMBL feature tag type '%S' for '%S'",
		       featoutFmtTmp, tmptag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featoutFmtTmp);*/
	}

        featTagEmblWrapC(&featoutStr, 79,
                         ajStrGetPtr(preftyptag), &wrapstr);

	ajFmtPrintF(file, "%S", wrapstr);
	ajStrDel(&wrapstr);
    }
    
    /* ajDebug("featDumpRefseq Done %d tags\n", i); */
    
    ajListIterDel(&iter);
    ajStrDel(&preftyploc);
    ajStrDel(&preftyptag);
    ajStrDel(&tmploc);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpRefseqp ************************************************
**
** Write details of single feature to file in RefSeq protein format
**
** @param [r] feat     [const AjPFeature] Feature
** @param [r] location [const AjPStr] location list
** @param [u] file     [AjPFile] Output file
** @param [r] Seqid    [const AjPStr] Sequence ID
** @return [void]
** @@
******************************************************************************/

static void featDumpRefseqp(const AjPFeature feat, const AjPStr location,
                            AjPFile file, const AjPStr Seqid)
{
    AjIList iter   = NULL;
    ajint i        = 0;
    AjPTagval tv = NULL;
    const AjPStr tmptyp  = NULL;		/* these come from AjPTable */
    const AjPStr tmptag  = NULL;		/* so please, please */
                                                /* don't delete them */
    AjPStr tmplim = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr           = NULL;
    AjPStr preftyploc = NULL;
    AjPStr preftyptag = NULL;
    AjPStr tmploc     = NULL;
    const AjPTable tagstable; 
    const AjPTable typetable; 
    
    /* ajDebug("featDumpRefseqp Start\n"); */
    
    /* print the location */
    
    tagstable = ajFeatVocabGetTagsProt("refseqp");
    typetable = ajFeatVocabGetTypesProt("refseqp");

    ajStrAssignS(&tmploc, location);
    tmptyp = ajFeattypeGetExternal(feat->Type, typetable);
    
    ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
    ajFmtPrintS(&preftyptag, "%s                   ", "  ");

    
    featLocEmblWrapC(&tmploc, 79,
                     ajStrGetPtr(preftyptag),
                     ajStrGetPtr(preftyploc), &wrapstr);

    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDel(&wrapstr);
    
    /* print the qualifiers */
    
    iter = ajListIterNewread(feat->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	++i;
	tmptag = ajFeattagGetNameS(tv->Tag, tagstable, &knowntag);
	ajFeattagFormat(tmptag, tagstable, &featoutFmtTmp);
	/* ajDebug(" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
	/* ajDebug(" %3d  %S format: '%S'\n", i, tmptag, featoutFmtTmp); */
	ajFmtPrintS(&featoutStr, "/%S", tmptag);

	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);
	    cp = ajStrGetPtr(featoutFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :		/* limited */
		/* ajDebug("case limited\n"); */
		ajFeattagGetLimit(tmptag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/* ajDebug("case qlimited\n"); */
		ajFeattagGetLimit(tmptag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		featTagQuoteEmbl(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/* ajDebug("case qspecial\n"); */
		if(!ajFeattagSpecial(tmptag, &featoutValTmp))
		{
		    ajFeatWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featoutStr, tmptag, &featoutValTmp);
		}
		else
		{
		    featTagQuoteEmbl(&featoutValTmp);
		    ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/* ajDebug("case special\n"); */
		if(!ajFeattagSpecial(tmptag, &featoutValTmp))
		{
		    ajFeatWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featoutStr, tmptag, &featoutValTmp);
		}
		else
		    ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);

		break;
	    case CASE2('T','E') :     /* no space, no quotes, wrap at margin */
		/* ajDebug("case text\n"); */
		ajStrRemoveWhite(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    case CASE2('V','O') :	     /* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    case CASE2('Q','T') :	   /* escape quotes, wrap at space */
		/* ajDebug("case qtext\n"); */
		featTagQuoteEmbl(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    case CASE2('Q','W') :	   /* escape quotes, remove space */
		/* ajDebug("case qword\n"); */
		featTagQuoteEmbl(&featoutValTmp);
		ajStrRemoveWhite(&featoutValTmp);	/* no white space needed */
		ajFmtPrintAppS(&featoutStr, "=%S\n", featoutValTmp);
		break;
	    default:
		ajFeatWarn("Unknown EMBL feature tag type '%S' for '%S'",
		       featoutFmtTmp, tmptag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featoutFmtTmp);*/
	}

        featTagEmblWrapC(&featoutStr, 79,
                         ajStrGetPtr(preftyptag), &wrapstr);

	ajFmtPrintF(file, "%S", wrapstr);
	ajStrDel(&wrapstr);
    }
    
    /* ajDebug("featDumpRefseqp Done %d tags\n", i); */
    
    ajListIterDel(&iter);
    ajStrDel(&preftyploc);
    ajStrDel(&preftyptag);
    ajStrDel(&tmploc);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpPir ****************************************************
**
** Write details of single feature to output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] location [const AjPStr] Location as a string
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpPir(const AjPFeature thys, const AjPStr location,
			AjPFile file)
{
    AjIList iter  = NULL;
    AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
                                        /* don't delete them */
    AjPStr outcomm = NULL;
    AjPStr outfmt  = NULL;
    AjPTagval tv        = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjBool typmod;
    AjPFeature copy = NULL;
    const AjPTable tagstable;

    tagstable = ajFeatVocabGetTagsProt("pir");

    copy = ajFeatNewFeat(thys);

    ajStrAssignClear(&outcomm);
    
    ajStrAssignS(&outtyp, copy->Type);
    typmod = featTypePirOut(&outtyp);	/* try to pick the best type if any */

    /* if changed, we append the original internal type */
    if(typmod)
    {
	ajDebug("typmod %B append ', %S'\n",
                typmod, ajFeatTypeProt(copy->Type));
	ajFeatSetDescApp(copy, ajFeatTypeProt(copy->Type));
    }

/* make sure it's PIR */

    ajStrFmtTitle(&outtyp);
    ajStrExchangeCC(&outtyp, "_", " ");
    
    ajFmtPrintF(file, "F;%S/%S:", location, outtyp);
    
    /* For all tag-values... */
    
    iter = ajFeatTagIter(copy);
    
    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	outtag = ajFeattagGetNameS(tv->Tag, tagstable, &knowntag);
	ajFeattagFormat(outtag, tagstable, &outfmt);
	/*ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, outfmt, tv->Value);*/

	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);

	    if(ajStrMatchCaseC(outtag, "comment"))
	    {
		ajFmtPrintAppS(&outcomm, " #%S", featoutValTmp);
		continue;
	    }

	    cp = ajStrGetPtr(outfmt);

	    switch(CASE2(cp[0], cp[1]))
	    {
                default:
                    ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", outfmt);*/
	}
	
	ajFmtPrintF(file, "%S", featoutStr);
	ajStrDelStatic(&featoutStr);
    }
    
    ajListIterDel(&iter);
    ajFeatDel(&copy);
    ajFmtPrintF(file, "%S\n", outcomm);
    ajStrDel(&outcomm);
    ajStrDel(&outfmt);
    ajStrDel(&outtyp);

    return;
}




/* @funcstatic featDumpSwiss **************************************************
**
** Write details of single feature to output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [u] file [AjPFile] Output file
** @param [r] gftop [const AjPFeature] Parent feature
** @return [void]
** @@
******************************************************************************/

static void featDumpSwiss(const AjPFeature thys, AjPFile file,
			  const AjPFeature gftop)
{
    AjIList iter  = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    /* don't delete them */
    AjPStr outfmt = NULL;
    AjPStr tmplim = NULL;
    AjPTagval tv       = NULL;
    ajint i =0;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr        = NULL;
    AjPStr fromstr = NULL;
    AjPStr tostr   = NULL;
    AjBool wasnote = ajFalse;
    const AjPTable tagstable; 
    const AjPTable typetable; 
    
    tagstable = ajFeatVocabGetTagsProt("swiss");
    typetable = ajFeatVocabGetTypesProt("swiss");

    outtyp = ajFeattypeGetExternal(thys->Type, typetable);
    
    if(thys->Flags & FEATFLAG_START_UNSURE)
    {
	if(thys->Start)
	    ajFmtPrintS(&fromstr, "?%d", thys->Start);
	else
	    ajFmtPrintS(&fromstr, "?");
    }
    else if(thys->Flags & FEATFLAG_START_BEFORE_SEQ)
	ajFmtPrintS(&fromstr, "<%d", thys->Start);
    else
	ajFmtPrintS(&fromstr, "%d", thys->Start);

    
    if(thys->Flags & FEATFLAG_END_UNSURE)
    {
	if(thys->End)
	    ajFmtPrintS(&tostr, "?%d", thys->End);
	else
	    ajFmtPrintS(&tostr, "?");
    }
    else if(thys->Flags & FEATFLAG_END_AFTER_SEQ)
	ajFmtPrintS(&tostr, ">%d", thys->End);
    else
	ajFmtPrintS(&tostr, "%d", thys->End);
    
    ajFmtPrintS(&featoutStr, "FT   %-8.8S %6.6S %6.6S",
		outtyp, fromstr, tostr);
    
    /* For all tag-values... from gftop which could be the same as thys */
    
    iter = ajFeatTagIter(gftop);
    
    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	outtag = ajFeattagGetNameS(tv->Tag, tagstable, &knowntag);
	ajFeattagFormat(outtag, tagstable, &outfmt);
	ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, outfmt, tv->Value);

	if(i++)
	    ajFmtPrintAppS(&featoutStr, " ") ;
	else
	    ajFmtPrintAppS(&featoutStr, "       ") ;

	/* ajFmtPrintAppS(&featoutStr, "%S", outtag); */ /* tag type is silent */

	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);
	    cp = ajStrGetPtr(outfmt);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :	/* limited */
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/*ajDebug("case limited\n");*/
		ajFeattagGetLimit(outtag, tagstable, &tmplim);
		featTagAllLimit(&featoutValTmp, tmplim);
		ajFmtPrintAppS(&featoutStr, "%S.", featoutValTmp);
		ajStrDel(&tmplim);
                wasnote = ajFalse;
		break;
	    case CASE2('T','A') :	/* tag=text */
		/*ajDebug("case tagval\n");*/
		if(ajStrMatchCaseC(outtag, "ftid")) /* fix case for tag */
		    ajFmtPrintAppS(&featoutStr, "/FTId=%S.",featoutValTmp);
		else			/* lower case is fine */
		    ajFmtPrintAppS(&featoutStr, "/%S=%S",outtag, featoutValTmp);
                wasnote = ajFalse;
		break;
	    case CASE2('T','E') :     /* simple text, wrap at space */
		/*ajDebug("case text\n");*/
		ajFmtPrintAppS(&featoutStr, "%S.", featoutValTmp);
                wasnote = ajTrue;
		break;
	    case CASE2('B','T') :	/* bracketed, wrap at space */
		/*ajDebug("case btext\n");*/
                if(wasnote)
                {
                    ajStrCutEnd(&featoutStr, 2); /* remove ". " */
                    ajStrAppendK(&featoutStr, ' '); /* replace the space */
                }
                ajFmtPrintAppS(&featoutStr, "(%S).", featoutValTmp);
                wasnote = ajFalse;
		break;
	    default:
		ajFeatWarn("Unknown SWISS feature tag type '%S' for '%S'",
		       outfmt, outtag);
                wasnote = ajFalse;
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", outfmt);*/
	}
    }
    
    ajListIterDel(&iter);
    
    
    featTagSwissWrapC(&featoutStr, 75, "FT                                ",
		      &wrapstr);
    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDelStatic(&featoutStr);
    ajStrDel(&wrapstr);
    ajStrDel(&tostr);
    ajStrDel(&fromstr);
    ajStrDel(&outfmt);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpGff2 ***************************************************
**
** Write details of single feature to GFF 2.0 output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] owner [const AjPFeattable] Feature table
**                                       (used for the sequence name)
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGff2(const AjPFeature thys, const AjPFeattable owner,
			 AjPFile file)
{
    AjIList iter  = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    /* don't delete them */
    AjPTagval tv       = NULL;
    ajint i = 0;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr flagdata      = NULL;
    const AjPTable tagstable = NULL;
    const AjPTable typetable = NULL;

    if(thys->Protein)
    {
        tagstable = ajFeatVocabGetTagsProt("gff2protein");
        typetable = ajFeatVocabGetTypesProt("gff2protein");
    }
    else
    {
        tagstable = ajFeatVocabGetTagsNuc("gff2");
        typetable = ajFeatVocabGetTypesNuc("gff2");
    }
    
    /* header done by calling routine */
    
    /*ajDebug("featDumpGff...\n");*/
    
    /* simple line-by line with Gff tags */
    
    outtyp = ajFeattypeGetExternal(thys->Type, typetable);
    
    /*ajDebug("Type '%S' => '%S'\n", thys->Type, outtyp);*/
    
    if(ajFeatstrandGetStrand(thys->Strand) == '-' && thys->End < thys->Start)
        ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
                    owner->Seqid,
                    thys->Source,
                    outtyp,
                    thys->End,
                    thys->Start,
                    thys->Score,
                    ajFeatstrandGetStrand(thys->Strand),
                    ajFeatframeGetFrame(thys->Frame));
    else
        ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
                    owner->Seqid,
                    thys->Source,
                    outtyp,
                    thys->Start,
                    thys->End,
                    thys->Score,
                    ajFeatstrandGetStrand(thys->Strand),
                    ajFeatframeGetFrame(thys->Frame));

    if(thys->Flags)
	ajFmtPrintS(&flagdata, "0x%x", thys->Flags);

    if(thys->Start2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "start2:%d", thys->Start2);
    }

    if(thys->End2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "end2:%d", thys->End2);
    }

    if(ajStrGetLen(thys->Remote))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "remoteid:%S", thys->Remote);
    }

    if(ajStrGetLen(thys->Label))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "label:%S", thys->Label);
    }
    
    /* group and flags */
    
    ajFmtPrintF(file, "Sequence \"%S.%d\"",
		owner->Seqid, thys->Group) ;
    i++;
    
    if(ajStrGetLen(flagdata))
    {
	/*
	 ** Move this code up to run for all features - to preserve the order
	 ** when rewriting in EMBL format
	     if(FEATFLAG_MULTIPLE)
             {
	       (void) ajFmtPrintF(file, "Sequence \"%S.%d\" ; ",
	                          owner->Seqid, thys->Group) ;
	       i++;
	     }
	 */
	if(i++)
	    ajFmtPrintF(file, " ; ") ;

	ajFmtPrintF(file, "FeatFlags \"%S\"", flagdata) ;
    }
    
    /* For all tag-values... */
    
    iter = ajFeatTagIter(thys);
    
    while(!ajListIterDone(iter))
    {
	tv     = ajListIterGet(iter);
	outtag = ajFeattagGetNameS(tv->Tag, tagstable, &knowntag);
	ajFeattagFormat(outtag, tagstable, &featoutFmtTmp);

	/*ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, featoutFmtTmp, tv->Value);*/
	if(i++)
	    ajFmtPrintF(file, " ; ") ;

	ajFmtPrintAppS(&featoutStr, "%S", outtag);
	
	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);
	    cp = ajStrGetPtr(featoutFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :	/* limited */
		/*ajDebug("case limited\n");*/
		ajFeattagGetLimit(outtag, tagstable, &featoutLimTmp);
		featTagAllLimit(&featoutValTmp, featoutLimTmp);
		ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/*ajDebug("case qlimited\n");*/
		ajFeattagGetLimit(outtag, tagstable, &featoutLimTmp);
		featTagAllLimit(&featoutValTmp, featoutLimTmp);
		featTagQuoteGff2(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
		break;
	    case CASE2('T','E') : /* no space, no quotes, wrap at margin */
		/*ajDebug("case text\n");*/
		ajStrRemoveWhite(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
		break;
	    case CASE2('Q','T') :	/* escape quotes, wrap at space */
		/*ajDebug("case qtext\n");*/
		featTagQuoteGff2(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
		break;
	    case CASE2('Q','W') :	/* escape quotes, remove space */
		/*ajDebug("case qtext\n");*/
		featTagQuoteGff2(&featoutValTmp);
		ajStrRemoveWhite(&featoutValTmp);
		ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/*ajDebug("case qspecial\n");*/
		if(!ajFeattagSpecialGff2(outtag, &featoutValTmp))
		    featTagGff2Default(&featoutStr, outtag, &featoutValTmp);
		else
		{
		    featTagQuoteGff2(&featoutValTmp);
		    ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/*ajDebug("case special\n");*/
		if(!ajFeattagSpecialGff2(outtag, &featoutValTmp))
		    featTagGff2Default(&featoutStr, outtag, &featoutValTmp);
		else
		    ajFmtPrintAppS(&featoutStr, " %S", featoutValTmp);

		break;
	    case CASE2('V','O') :	/* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    default:
		ajFeatWarn("Unknown GFF 2.0 feature tag type '%S' for '%S'",
		       featoutFmtTmp, outtag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featoutFmtTmp);*/
	}
	
	ajFmtPrintF(file, "%S", featoutStr);
	ajStrDelStatic(&featoutStr);
    }

    ajFmtPrintF(file, "\n") ;    
    ajListIterDel(&iter);
    ajStrDel(&flagdata);
    
    return;
}




/* @funcstatic featDumpGff3Tag ************************************************
**
** Write tag-value pairs of features to GFF 3.0 output file
**
** @param [u] file [AjPFile] Output file
** @param [r] tag [const char*] Tag name
** @param [r] value [const AjPStr] Tag value
** @param [r] firsttag [AjBool] whether this is the first tag to be printed
** @return [AjBool] always returns ajFalse
** @@
******************************************************************************/

static AjBool featDumpGff3Tag(AjPFile file, const char* tag,
	                      const AjPStr value, AjBool firsttag)
{

    if(strlen(tag)==0)
    {
	ajFeatWarn("empty tag name for tag value '%S'", value);
	tag="Note";
    }

    if(firsttag)
	ajFmtPrintF(file, "%s=%S", tag, value);
    else
	ajFmtPrintF(file, ";%s=%S", tag, value);

    (void)firsttag;

    return ajFalse;
}




/* @funcstatic featDumpGff3 ***************************************************
**
** Write details of single feature to GFF 3.0 output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] parent [const AjPFeature] Parent feature
** @param [r] owner [const AjPFeattable] Feature table
**                                       (used for the sequence name)
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGff3(const AjPFeature thys, const AjPFeature parent,
                         const AjPFeattable owner, AjPFile file)
{
    AjIList iter  = NULL;
    AjIList ftiter  = NULL;
    AjPFeature gf = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;
    const char* tagname = NULL;
    AjPTagval tv       = NULL;
    ajint i = 0;
    const char* cp;
    AjPStr flagdata      = NULL;
    const AjPTable tagstable = NULL;
    const AjPTable typetable = NULL;
    char frame;
    ajuint tmpbeg = 0;
    ajuint tmpend = 0;
    const AjPStr tmpid;
    AjPFeatGfftags gfftags = NULL;
    AjBool firsttag = ajTrue;

    ajDebug("featDumpGff3 prot: %B type '%S'\n",
            thys->Protein, thys->Type);

    if(thys->Protein)
    {
        tagstable = ajFeatVocabGetTagsProt("gff3protein");
        typetable = ajFeatVocabGetTypesProt("gff3protein");
    }
    else
    {
        tagstable = ajFeatVocabGetTagsNuc("gff3");
        typetable = ajFeatVocabGetTypesNuc("gff3");
    }
    
    /* header done by calling routine */
    
    /*ajDebug("featDumpGff3...\n");*/
    
    /* simple line-by line with Gff3 tags */
    
    outtyp = ajFeattypeGetExternal(thys->Type, typetable);
    
    /*ajDebug("Type '%S' => '%S'\n", thys->Type, outtyp);*/

    if(ajFeatTypeIsCds(thys))
	frame = ajFeatframeGetFrameNuc(thys->Frame);
    else
	frame = ajFeatframeGetFrame(thys->Frame);

    if(thys->Flags & FEATFLAG_START_BEFORE_SEQ)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "start_before:true");
    }

    if(thys->Flags & FEATFLAG_END_AFTER_SEQ)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "end_after:true");
    }

/*   FEATFLAG_CHILD known from subfeatures */

    if(thys->Flags & FEATFLAG_BETWEEN_SEQ)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "between:true");
    }

    if(thys->Start2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "start2:%d", thys->Start2);
    }

    if(thys->End2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "end2:%d", thys->End2);
    }

/*   FEATFLAG_POINT known from start=end */

/*   FEATFLAG_COMPLEMENT_MAIN known from parent feature strand */

/*   FEATFLAG_MULTIPLE known from subfeatures */

/*   FEATFLAG_GROUP obsolete, known from subfeatures */

/*   FEATFLAG_ORDER obsolete, known from subfeatures */

/*   FEATFLAG_ONEOF obsolete, known from subfeatures */

/*   FEATFLAG_REMOTEID known from non-matching seqid for a subfeature */

    if(ajStrGetLen(thys->Label))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "label:%S", thys->Label);
    }
    
    if(thys->Flags & FEATFLAG_START_UNSURE)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "start_unsure:true");
    }

    if(thys->Flags & FEATFLAG_END_UNSURE)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, ",");

	ajFmtPrintAppS(&flagdata, "end_unsure:true");
    }

    if(ajFeatstrandGetStrand(thys->Strand) == '-' && thys->End < thys->Start)
    {
        tmpbeg = thys->End;
        tmpend = thys->Start;
    }
    else
    {
        tmpbeg = thys->Start;
        tmpend = thys->End;
    }

    if(MAJSTRGETLEN(thys->Remote))
        tmpid = thys->Remote;
    else
        tmpid = owner->Seqid;

    if(thys->Score)
	ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
	            tmpid,
	            thys->Source,
	            outtyp,
	            tmpbeg,
	            tmpend,
	            thys->Score,
	            ajFeatstrandGetStrand(thys->Strand),
	            frame);
    else
	ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%c\t%c\t%c\t",
	            tmpid,
	            thys->Source,
	            outtyp,
	            tmpbeg,
	            tmpend,
	            '.',
	            ajFeatstrandGetStrand(thys->Strand),
	            frame);
       
    gfftags = ajFeatGetGfftags(thys);

    /* For all GFF tag-values... */
    
    if(ajStrGetLen(gfftags->Id))
	firsttag=featDumpGff3Tag(file, "ID", gfftags->Id,firsttag);
    if(ajStrGetLen(gfftags->Name))
	firsttag=featDumpGff3Tag(file, "Name", gfftags->Name, firsttag);
    if(ajStrGetLen(gfftags->Alias))
	firsttag=featDumpGff3Tag(file, "Alias", gfftags->Alias, firsttag);
    if(ajStrGetLen(gfftags->Parent))
	firsttag=featDumpGff3Tag(file, "Parent", gfftags->Parent, firsttag);
    if(ajStrGetLen(gfftags->Target))
	firsttag=featDumpGff3Tag(file, "Target", gfftags->Target, firsttag);
    if(ajStrGetLen(gfftags->Gap))
	firsttag=featDumpGff3Tag(file, "Gap", gfftags->Gap, firsttag);
    if(ajStrGetLen(gfftags->DerivesFrom))
	firsttag=featDumpGff3Tag(file, "Derives_from", gfftags->DerivesFrom,
	                         firsttag);
    if(ajStrGetLen(gfftags->Note))
	firsttag=featDumpGff3Tag(file, "Note", gfftags->Note, firsttag);
    if(ajStrGetLen(gfftags->Dbxref))
	firsttag=featDumpGff3Tag(file, "Dbxref", gfftags->Dbxref, firsttag);
    if(ajStrGetLen(gfftags->OntologyTerm))
	firsttag=featDumpGff3Tag(file, "Ontology_term", gfftags->OntologyTerm,
	                         firsttag);
    if(ajStrGetLen(gfftags->IsCircular))
	firsttag=featDumpGff3Tag(file, "Is_circular", gfftags->IsCircular,
	                         firsttag);

    /* group and flags */
    
    if(parent && !ajStrGetLen(gfftags->Parent))
    {
        ajFmtPrintF(file, "%sParent=%S.%d",
                    (firsttag?"":";"),
	            owner->Seqid, parent->Group);
        firsttag = ajFalse;
    }
    else if(!parent && !ajStrGetLen(gfftags->Id) && !ajStrGetLen(gfftags->Name))
    {
        ajFmtPrintF(file, "%sID=%S.%d",
	            (firsttag?"":";"),
	            owner->Seqid, thys->Group);
        firsttag = ajFalse;
    }

    ajFeatGfftagsDel(&gfftags);

    i++;
    
    if(ajStrGetLen(flagdata))
	firsttag=featDumpGff3Tag(file, "featflags", flagdata, firsttag);
    
    /* For all other tag-values... */
    
    iter = ajFeatTagIter(thys);

    while(!ajListIterDone(iter))
    {
	tv     = ajListIterGet(iter);
	outtag = tv->Tag;
	
	if(!outtag)
	{
	    ajFeatWarn("Unknown GFF3 feature tag '%S'", tv->Tag);
	    continue;
	}

	ajFeattagFormat(outtag, tagstable, &featoutFmtTmp);
	/*ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, featoutFmtTmp, tv->Value);*/

	tagname = ajStrGetPtr(outtag);
	
	if(tv->Value)
	{
	    ajStrAssignS(&featoutValTmp, tv->Value);
	    cp = ajStrGetPtr(featoutFmtTmp);

	    if(strlen(cp)>0)
	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :	/* limited */
		/*ajDebug("case limited\n");*/
		ajFeattagGetLimit(outtag, tagstable, &featoutLimTmp);
		featTagAllLimit(&featoutValTmp, featoutLimTmp);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/*ajDebug("case qlimited\n");*/
		ajFeattagGetLimit(outtag, tagstable, &featoutLimTmp);
		featTagAllLimit(&featoutValTmp, featoutLimTmp);
		break;
	    case CASE2('T','E') : /* no space, no quotes, wrap at margin */
		/*ajDebug("case text\n");*/
		ajStrRemoveWhite(&featoutValTmp);
		break;
	    case CASE2('Q','T') :	/* escape quotes, wrap at space */
		/*ajDebug("case qtext\n");*/
		break;
	    case CASE2('Q','W') :	/* escape quotes, remove space */
		/*ajDebug("case qtext\n");*/
		ajStrRemoveWhite(&featoutValTmp);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/*ajDebug("case qspecial\n");*/
		/* todo: double check whether we need following line */
		ajFeattagSpecialGff3(outtag, &featoutValTmp);
		break;
	    case CASE2('S','P') :	/* special regexp */
		/*ajDebug("case special\n");*/
		/* todo: double check whether we need following lines */
		ajFeattagSpecialGff3(outtag, &featoutValTmp);
		break;
	    case CASE2('V','O') :	/* no value, so append 'true' */
		/*ajDebug("case void\n");*/
                ajStrAssignC(&featoutValTmp, "true");
		break;
	    default:
		ajFeatWarn("Unknown GFF3 feature tag type '%S' for '%S'",
		       featoutFmtTmp, outtag);
		continue;
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featoutFmtTmp);*/
	    ajStrAssignC(&featoutValTmp, "true");
	}
	
        featTagQuoteGff3(&featoutValTmp);
	firsttag=featDumpGff3Tag(file, tagname, featoutValTmp, firsttag);
    }


    ajFmtPrintF(file, "\n") ;
    ajListIterDel(&iter);
    ajStrDel(&flagdata);

    if(ajListGetLength(thys->Subfeatures))
    {
        ftiter = ajListIterNewread(thys->Subfeatures);

        while(!ajListIterDone(ftiter))
        {
            gf = ajListIterGet(ftiter);

            featDumpGff3(gf, thys, owner, file);
        }

        ajListIterDel(&ftiter);
        return;
    }
    
    return;
}




/* @func ajFeattableWriteDebug ************************************************
**
** Print contents of a feature table to a file
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] ftable [const AjPFeattable] Feature table
** @return [AjBool] Always true
******************************************************************************/

AjBool ajFeattableWriteDebug(AjPFeattabOut ftout, const AjPFeattable ftable)
{
    return ajFeattablePrint(ftable, ftout->Handle);
}




/* @func ajFeattablePrint *****************************************************
**
** Print contents of a feature table to a file
**
** @param [r] ftable [const AjPFeattable] Feature table
** @param [u] outf [AjPFile] Output file object
** @return [AjBool] Always true
******************************************************************************/

AjBool ajFeattablePrint(const AjPFeattable ftable, AjPFile outf)
{
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    AjIList itertag    = NULL;
    AjPTagval tv = NULL;
    ajint i=0;
    ajint j=0;

    ajFmtPrintF(outf, "  Format: %u (%S)",
		ftable->Format, ftable->Formatstr);
    ajFmtPrintF(outf, "  Start: %u\n",
		ftable->Start);
    ajFmtPrintF(outf, "  End: %u\n",
		ftable->End);
    ajFmtPrintF(outf, "  Len: %u\n",
		ftable->Len);
    ajFmtPrintF(outf, "  Offset: %u\n",
		ftable->Offset);
    ajFmtPrintF(outf, "  Groups: %u\n",
		ftable->Groups);

    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	i++;

	ajFmtPrintF(outf, "\n  Feature %d\n", i);
	ajFmtPrintF(outf, "    Source: '%S'\n", feature->Source);
	ajFmtPrintF(outf, "    Type: '%S'\n", feature->Type);
	ajFmtPrintF(outf, "    Score: %.6f\n", feature->Score);
	ajFmtPrintF(outf, "    Protein: %B\n", feature->Protein);
	ajFmtPrintF(outf, "    Strand: '%c'\n", feature->Strand);
	ajFmtPrintF(outf, "    Start: %d\n", feature->Start);
	ajFmtPrintF(outf, "    End: %d\n", feature->End);
	ajFmtPrintF(outf, "    Start2: %d\n", feature->Start2);
	ajFmtPrintF(outf, "    End2: %d\n", feature->End2);
	ajFmtPrintF(outf, "    Remote: '%S'\n", feature->Remote);
	ajFmtPrintF(outf, "    Label: '%S'\n", feature->Label);
	ajFmtPrintF(outf, "    Frame: %d\n", feature->Frame);
	ajFmtPrintF(outf, "    Exon: %u\n", feature->Exon);
	ajFmtPrintF(outf, "    Group: %u\n", feature->Group);
	ajFmtPrintF(outf, "    Flags: %x\n", feature->Flags);

	if(feature->Flags & FEATFLAG_START_BEFORE_SEQ)
	    ajFmtPrintF(outf, "      START_BEFORE_SEQ\n");

	if(feature->Flags & FEATFLAG_END_AFTER_SEQ)
	    ajFmtPrintF(outf, "      END_AFTER_SEQ\n");

	if(feature->Flags & FEATFLAG_CHILD)
	    ajFmtPrintF(outf, "      CHILD\n");

	if(feature->Flags & FEATFLAG_BETWEEN_SEQ)
	    ajFmtPrintF(outf, "      BETWEEN_SEQ\n");

	if(feature->Flags & FEATFLAG_START_TWO)
	    ajFmtPrintF(outf, "      START_TWO\n");

	if(feature->Flags & FEATFLAG_END_TWO)
	    ajFmtPrintF(outf, "      END_TWO\n");

	if(feature->Flags & FEATFLAG_POINT)
	    ajFmtPrintF(outf, "      POINT\n");

	if(feature->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    ajFmtPrintF(outf, "      COMPLEMENT_MAIN\n");

	if(feature->Flags & FEATFLAG_MULTIPLE)
	    ajFmtPrintF(outf, "      MULTIPLE\n");

	if(feature->Flags & FEATFLAG_ORDER)
	    ajFmtPrintF(outf, "      ORDER\n");

	if(feature->Flags & FEATFLAG_REMOTEID)
	    ajFmtPrintF(outf, "      REMOTEID\n");

	if(feature->Flags & FEATFLAG_LABEL)
	    ajFmtPrintF(outf, "      LABEL\n");

	if(feature->Flags & FEATFLAG_START_UNSURE)
	    ajFmtPrintF(outf, "      START_UNSURE\n");

	if(feature->Flags & FEATFLAG_END_UNSURE)
	    ajFmtPrintF(outf, "      END_UNSURE\n");

	ajFmtPrintF(outf, "    Tags: %u tags\n",
		    ajListGetLength(feature->Tags));

	j=0;
	itertag = ajListIterNewread(feature->Tags);

	while(!ajListIterDone(itertag))
	{
	    tv = ajListIterGet(itertag);
	    ajFmtPrintF(outf, "      Tag %3d %S : '%S'\n",
			++j, tv->Tag, tv->Value);
	}

	ajListIterDel(&itertag);

    }

    ajListIterDel(&iterft);

    return ajTrue;
}




/* @func ajFeatoutPrintFormat *************************************************
**
** Reports the internal data structures for output feature formats
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajFeatoutPrintFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "# Feature output formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Name is an alias\n");
    ajFmtPrintF(outf, "# Nuc   Valid for nucleotide sequences\n");
    ajFmtPrintF(outf, "# Pro   Valid for protein sequences\n");
    ajFmtPrintF(outf, "# Name         Alias   Nuc   Pro "
		"Description\n");
    ajFmtPrintF(outf, "OutFormat {\n");

    for(i=0; featOutFormatDef[i].Name; i++)
    {
	if(full || !featOutFormatDef[i].Alias)
	    ajFmtPrintF(outf, "  %-12s %5B %5B %5B \"%s\"\n",
			featOutFormatDef[i].Name,
			featOutFormatDef[i].Alias,
			featOutFormatDef[i].Nucleotide,
			featOutFormatDef[i].Protein,
			featOutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajFeatoutPrinthtmlFormat *********************************************
**
** Reports the internal data structures for output feature formats
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatoutPrinthtmlFormat(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Features Output Format</th><th>Alias</th>\n");
    ajFmtPrintF(outf, "<th>Nuc</th><th>Pro</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");

    for(i=0; featOutFormatDef[i].Name; i++)
    {
	if(!featOutFormatDef[i].Alias)
	    ajFmtPrintF(outf, "<tr><td>\n%-12s\n</td><td>%5B\n</td>"
                        "<td>%5B\n</td><td>%5B\n</td><td>\"%s\"</td></tr>\n",
			featOutFormatDef[i].Name,
			featOutFormatDef[i].Alias,
			featOutFormatDef[i].Nucleotide,
			featOutFormatDef[i].Protein,
			featOutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajFeatoutPrintbookFormat *********************************************
**
** Reports the output feature format internals in docbook format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatoutPrintbookFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Output feature formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; featOutFormatDef[i].Name; i++)
    {
	if(!featOutFormatDef[i].Alias)
        {
            namestr = ajStrNewC(featOutFormatDef[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; featOutFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],featOutFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featOutFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featOutFormatDef[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featOutFormatDef[j].Protein);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featOutFormatDef[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajFeatoutPrintwikiFormat *********************************************
**
** Reports the output feature format internals in wiki text format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatoutPrintwikiFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Nuc!!Pro!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; featOutFormatDef[i].Name; i++)
    {
        if(!featOutFormatDef[i].Alias)
        {
            ajFmtPrintF(outf, "|-\n");
            ajStrAssignC(&namestr, featOutFormatDef[i].Name);


            for(j=i+1; featOutFormatDef[j].Name; j++)
            {
                if(featOutFormatDef[j].Write == featOutFormatDef[i].Write)
                {
                    ajFmtPrintAppS(&namestr, " %s", featOutFormatDef[j].Name);
                    if(!featOutFormatDef[j].Alias) 
                    {
                        ajWarn("Feature output format '%s' same as '%s' "
                               "but not alias",
                               featOutFormatDef[j].Name,
                               featOutFormatDef[i].Name);
                    }
                }
            }
            ajFmtPrintF(outf, "|%S||%B||%B||%s\n",
			namestr,
			featOutFormatDef[i].Nucleotide,
			featOutFormatDef[i].Protein,
			featOutFormatDef[i].Desc);
        }
    }

    ajFmtPrintF(outf, "|}\n\n");

    ajStrDel(&namestr);

    return;
}




/* @funcstatic featTagAllLimit ************************************************
**
** Tests a string as a valid feature value, given a
** list of possible values.
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] values [const AjPStr] comma delimited list of values
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagAllLimit(AjPStr* pval, const AjPStr values)
{
    AjPStr limstr = NULL;
    const char* cp = NULL;
    const char* cq = NULL;
    ajint i=0;
    AjBool ret = ajFalse;

    /* ajDebug("featTagAllLimit '%S' '%S'\n", *pval, values); */

    ajStrAssignResC(&limstr, 16, "");
    cp = ajStrGetPtr(values);
    cq = cp;

    while(*cp)
    {
	if(*cp == ',')
	{
	    ajStrAssignLenC(&limstr, cq, i);

	    if(ajStrMatchCaseS(*pval, limstr))
	    {
		if(!ajStrMatchS(*pval, limstr))
		{
		    ajStrAssignS(pval, limstr);
		}
		ret = ajTrue;
		break;
	    }

	    ajStrAssignClear(&limstr);
	    cq = cp+1;
	    i=0;
	}
	else
	    i++;
	cp++;
    }

    ajStrDel(&limstr);

    return ret;
}




/* @funcstatic featTagQuoteEmbl ***********************************************
**
** Internal quotes converted to two double quotes
** for EMBL feature tag values.
**
** Quotes added around the outside.
**
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteEmbl(AjPStr* pval)
{
    const char* cp;

    /* ajDebug("featTagQuoteEmbl '%S'\n", *pval); */

    if(ajStrFindAnyK(*pval, '"') < 0)
    {
	ajStrFmtQuote(pval);
    }
    else /* double up internal quotes */
    {
	ajStrAssignS(&featoutTmpStr, *pval);
	ajStrAssignC(pval, "\"");

	cp = ajStrGetPtr(featoutTmpStr);

	while(*cp)
	{
	    if(*cp == '"')
		ajStrAppendK(pval, '"');

	    ajStrAppendK(pval, *cp++);
	}

	ajStrAppendK(pval, '"');
    }

    ajStrDelStatic(&featoutTmpStr);

    return;
}




/* @funcstatic featTagQuoteGff2 ***********************************************
**
** Internal quotes converted to escaped quotes
** for EMBL feature tag values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteGff2(AjPStr* pval)
{

    if(!featoutRegQuote)
	featoutRegQuote = ajRegCompC("([^\"]*)\"");

    /* ajDebug("featTagQuoteGff2 '%S'\n", *pval); */

    ajStrAssignS(&featoutValCopy, *pval);
    ajStrDelStatic(pval);

    while(ajRegExec(featoutRegQuote, featoutValCopy))
    {
	ajRegSubI(featoutRegQuote, 1, &featoutSubStr);
	/* ajDebug("part '%S'\n", substr); */
	ajStrAppendS(pval, featoutSubStr);
	ajStrAppendC(pval, "\\\"");
	ajRegPost(featoutRegQuote, &featoutTmpStr);
	ajStrAssignS(&featoutValCopy, featoutTmpStr);
    }

    /* ajDebug("rest '%S'\n", featoutValCopy); */
    ajStrAppendS(pval, featoutValCopy);
    ajStrFmtQuote(pval);

    return;
}




/* @funcstatic featTagQuoteGff3 ***********************************************
**
** Quoted text values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteGff3(AjPStr* pval)
{
    ajDebug("featTagQuoteGff3 '%S'\n", *pval);

    ajStrFmtPercentEncodeC(pval, ";=%&,\t");

    return;
}




/* @funcstatic featTagEmblDefault *********************************************
**
** Give up, and generate a default EMBL/Genbank feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagEmblDefault(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{
    /*ajDebug("featTagEmblDefault '%S' '%S'\n", tag, *pval);*/

    featTagQuoteEmbl(pval);
    ajFmtPrintS(pout, "/note=\"%S: %S\"", tag, *pval);

    return;
}




/* @funcstatic featTagGff2Default *********************************************
**
** Give up, and generate a default GFF feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagGff2Default(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{
    /*ajDebug("featTagGff2Default '%S' '%S'\n", tag, *pval);*/

    featTagQuoteGff2(pval);
    ajFmtPrintS(pout, "note \"%S: %S\"", tag, *pval);

    return;
}




/* @funcstatic featTagDasgffDefault *******************************************
**
** Give up, and generate a default DASGFF feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagDasgffDefault(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{

    /*ajDebug("featTagDasgffDefault '%S' '%S'\n", tag, *pval);*/


    featTagQuoteGff3(pval);

    ajFmtPrintS(pout, "%S:%S", tag, *pval);
    
    return;
}




/* @funcstatic featTypePirOut *************************************************
**
** Converts an internal feature type into the corresponding PIR type,
** because internal types are based on GFF3.
**
** @param [u] type [AjPStr*] PIR feature type in, returned as internal type
** @return [AjBool] ajTrue if the type name was found and changed
******************************************************************************/

static AjBool featTypePirOut(AjPStr* type)
{
    const AjPStr pirtype = NULL;
    const AjPStr pirintype = NULL;
    AjBool ret = ajFalse;
    const AjPTable typetable;

    typetable = ajFeatVocabGetTypesProt("pir");

    pirtype = ajFeattypeGetExternal(*type, typetable);

    pirintype = ajFeattypeGetInternalPir(pirtype);

    ajDebug("featTypePirOut '%S' in '%S' ext '%S'\n",
	    *type, pirintype, pirtype);

    if(!ajStrMatchCaseS(*type, pirintype))
	   ret = ajTrue;

    ajStrAssignS(type, pirtype);

    return ret;

}




/* @func ajFeatwriteExit ******************************************************
**
** Cleans up feature table output internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajFeatwriteExit(void)
{
    ajRegFree(&featoutRegUfoFmt);
    ajRegFree(&featoutRegUfoFile);
    ajRegFree(&featoutRegQuote);

    ajStrDel(&featoutFmtTmp);
    ajStrDel(&featoutFormatTmp);
    ajStrDel(&featoutUfoTest);
    ajStrDel(&featoutValCopy);
    ajStrDel(&featoutValTmp);
    ajStrDel(&featoutTmpStr);
    ajStrDel(&featoutStr);
    ajStrDel(&featoutSubStr);
    ajStrDel(&featoutLimTmp);

    return;
}
