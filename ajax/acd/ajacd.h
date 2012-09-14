/* @include ajacd *************************************************************
**
** These functions control all aspects of AJAX command definition
** syntax, command line handling and prompting of the user.
**
** The only major functions visible to callers are the initialisation
** function ajAcdInit and a series of retrieval functions ajGetAcdObject
** for each defined object type (integer, sequence, and so on).
**
** Future extensions are planned, including the ability to write out
** the ACD internal structures in a number of other interface formats.
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.16 $
** @modified Jun 25 pmr First version
** @modified May 06 2004 Jon Ison Minor mods.
** @modified $Date: 2011/12/19 16:30:31 $ by $Author: rice $
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

#ifndef AJACD_H
#define AJACD_H



/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

#include "ajstr.h"
#include "ajlist.h"
#include "ajarr.h"
#include "ajrange.h"

#include "ajalign.h"
#include "ajcod.h"
#include "ajgraph.h"
#include "ajpat.h"
#include "ajphylo.h"
#include "ajreg.h"
#include "ajreport.h"

#include "ajassemdata.h"
#include "ajfeatdata.h"
#include "ajobodata.h"
#include "ajrefseqdata.h"
#include "ajresourcedata.h"
#include "ajseqdata.h"
#include "ajtaxdata.h"
#include "ajtextdata.h"
#include "ajurldata.h"
#include "ajvardata.h"

AJ_BEGIN_DECLS


/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void          ajAcdExit (AjBool single);

AjPAlign      ajAcdGetAlign (const char *token);
AjPFloat      ajAcdGetArray (const char *token);
AjPAssemload  ajAcdGetAssembly (const char *token);
AjBool        ajAcdGetBoolean (const char *token);
AjPCod        ajAcdGetCodon (const char *token);
AjPFile       ajAcdGetCpdb (const char *token);
AjPFile       ajAcdGetDatafile (const char *token);
AjPDir        ajAcdGetDirectory (const char *token);
AjPStr        ajAcdGetDirectoryName (const char *token);
AjPList       ajAcdGetDirlist (const char *token);
AjPPhyloState* ajAcdGetDiscretestates (const char *token);
AjPPhyloState ajAcdGetDiscretestatesSingle (const char *token);
AjPPhyloDist* ajAcdGetDistances (const char *token);
AjPPhyloDist  ajAcdGetDistancesSingle (const char *token);
double        ajAcdGetFloatDouble (const char *token);
AjPFeattable  ajAcdGetFeatures (const char *token);
AjPFeattaball ajAcdGetFeaturesall (const char *token);
AjPFeattabOut ajAcdGetFeatout (const char *token);
AjPList       ajAcdGetFilelist (const char *token);
float         ajAcdGetFloat (const char *token);
AjPPhyloFreq  ajAcdGetFrequencies (const char *token);
AjPGraph      ajAcdGetGraph (const char *token);
AjPGraph      ajAcdGetGraphxy (const char *token);
AjPFile       ajAcdGetInfile (const char *token);
ajint         ajAcdGetInt (const char *token);
ajlong        ajAcdGetIntLong (const char *token);
AjPStr*       ajAcdGetList (const char *token);
AjPStr        ajAcdGetListSingle(const char *token);
AjPMatrix     ajAcdGetMatrix (const char *token);
AjPMatrixf    ajAcdGetMatrixf (const char *token);
AjPObo        ajAcdGetObo(const char *token);
AjPOboall     ajAcdGetOboall(const char *token);
AjPOutfile    ajAcdGetOutassembly(const char *token);
AjPOutfile    ajAcdGetOutcodon(const char *token);
AjPOutfile    ajAcdGetOutcpdb(const char *token);
AjPOutfile    ajAcdGetOutdata(const char *token);
AjPDirout     ajAcdGetOutdir (const char *token);
AjPStr        ajAcdGetOutdirName (const char *token);
AjPOutfile    ajAcdGetOutdiscrete(const char *token);
AjPOutfile    ajAcdGetOutdistance(const char *token);
AjPFile       ajAcdGetOutfile (const char *token);
AjPStr        ajAcdGetOutfileName (const char *token);
AjPOutfile    ajAcdGetOutfreq(const char *token);
AjPOutfile    ajAcdGetOutmatrix(const char *token);
AjPOutfile    ajAcdGetOutmatrixf(const char *token);
AjPOutfile    ajAcdGetOutobo(const char *token);
AjPOutfile    ajAcdGetOutproperties(const char *token);
AjPOutfile    ajAcdGetOutrefseq(const char *token);
AjPOutfile    ajAcdGetOutresource(const char *token);
AjPOutfile    ajAcdGetOutscop(const char *token);
AjPOutfile    ajAcdGetOuttaxon(const char *token);
AjPOutfile    ajAcdGetOuttext(const char *token);
AjPOutfile    ajAcdGetOuttree(const char *token);
AjPOutfile    ajAcdGetOuturl(const char *token);
AjPOutfile    ajAcdGetOutvariation(const char *token);
AjPPatlistSeq ajAcdGetPattern (const char *token);
AjPPhyloProp  ajAcdGetProperties (const char *token);
AjPRange      ajAcdGetRange (const char *token);
AjPRefseq     ajAcdGetRefseq (const char *token);
AjPPatlistRegex ajAcdGetRegexp (const char *token);
AjPRegexp     ajAcdGetRegexpSingle (const char *token);
AjPReport     ajAcdGetReport (const char *token);
AjPResource   ajAcdGetResource(const char *token);
AjPResourceall  ajAcdGetResourceall(const char *token);
AjPFile       ajAcdGetScop (const char *token);
AjPStr*       ajAcdGetSelect (const char *token);
AjPStr        ajAcdGetSelectSingle (const char *token);
AjPSeq        ajAcdGetSeq (const char *token);
AjPSeqall     ajAcdGetSeqall (const char *token);
AjPSeqout     ajAcdGetSeqout (const char *token);
AjPSeqout     ajAcdGetSeqoutall (const char *token);
AjPSeqout     ajAcdGetSeqoutset (const char *token);
AjPSeqset     ajAcdGetSeqset (const char *token);
AjPSeqset*    ajAcdGetSeqsetall (const char *token);
AjPSeqset     ajAcdGetSeqsetallSingle (const char *token);
AjPStr        ajAcdGetString (const char *token);
AjPTax        ajAcdGetTaxon(const char *token);
AjPTaxall     ajAcdGetTaxonall(const char *token);
AjPText       ajAcdGetText(const char *token);
AjPTextall    ajAcdGetTextall(const char *token);
AjBool        ajAcdGetToggle (const char *token);
AjPPhyloTree* ajAcdGetTree (const char *token);
AjPPhyloTree  ajAcdGetTreeSingle (const char *token);
AjPUrl        ajAcdGetUrl(const char *token);
AjPUrlall     ajAcdGetUrlall(const char *token);
AjPVarload    ajAcdGetVariation(const char *token);

const AjPStr  ajAcdGetValue (const char* token);
const AjPStr  ajAcdGetValueDefault (const char* token);

void          ajAcdInit(const char *pgm, ajint argc, char * const argv[]);
void          ajAcdInitPV(const char *pgm, ajint argc, char * const argv[],
			  const char *package, const char *packversion);
AjBool        ajAcdIsUserdefinedC(const char* token);
AjBool        ajAcdIsUserdefinedS(const AjPStr);
const AjPStr  ajAcdGetpathC(const char *token);
const AjPStr  ajAcdGetpathS(const AjPStr);
void          ajAcdPrintAppl(AjPFile outf, AjBool full);
void          ajAcdPrintQual(AjPFile outf, AjBool full);
void          ajAcdPrintType (AjPFile outf, AjBool full);
AjBool        ajAcdSetControl (const char* optionName);
void          ajAcdUnused(void);
AjBool        ajAcdedamParse(const AjPStr relation, AjPStr* id,
                             AjPStr* namespace,  AjPStr* name);
const char*   ajAcdtypeGetGroup(const AjPStr type);

/*
** End of prototype definitions
*/

#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif
#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjBool         ajAcdIsUserdefined(const char* token);
__deprecated AjBool         ajAcdDebug (void);
__deprecated AjBool         ajAcdDebugIsSet (void);
__deprecated AjBool         ajAcdFilter (void);
__deprecated AjPPhyloState  ajAcdGetDiscretestatesI (const char *token,
						    ajint num);
__deprecated AjPStr         ajAcdGetListI (const char *token, ajint num);
__deprecated AjPStr         ajAcdGetSelectI (const char *token, ajint num);
__deprecated AjPSeqset      ajAcdGetSeqsetallI (const char *token, ajint num);
__deprecated AjPPhyloTree   ajAcdGetTreeI (const char *token, ajint num);
__deprecated const AjPStr   ajAcdValue (const char* token);
__deprecated AjBool         ajAcdStdout (void);

__deprecated AjBool        ajAcdGetBool (const char *token);
__deprecated AjPFeattable  ajAcdGetFeat (const char *token);

__deprecated void          ajAcdGraphicsInit(const char *pgm,
                                             ajint argc, char * const argv[]);
__deprecated void          ajAcdGraphicsInitPV(const char *pgm,
                                               ajint argc,char * const argv[],
                                               const char *package,
                                               const char *packversion);

__deprecated void          ajGraphicsInit(const char *pgm,
					  ajint argc, char * const argv[]);
__deprecated void          ajGraphicsInitPV(const char *pgm,
					    ajint argc,char * const argv[],
					    const char *package,
					    const char *packversion);

#endif

AJ_END_DECLS

#endif  /* !AJACD_H */
