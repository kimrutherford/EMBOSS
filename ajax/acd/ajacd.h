#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajacd_h
#define ajacd_h

#include "ajax.h"

/*
** Prototype definitions
*/

void          ajAcdGraphicsInit(const char *pgm,
				ajint argc, char * const argv[]);
void          ajAcdGraphicsInitPV(const char *pgm,
				  ajint argc,char * const argv[],
				  const char *package,
				  const char *packversion);

void          ajAcdExit (AjBool single);

AjPAlign      ajAcdGetAlign (const char *token);
AjPFloat      ajAcdGetArray (const char *token);
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
AjPOutfile    ajAcdGetOutcodon(const char *token);
AjPOutfile    ajAcdGetOutcpdb(const char *token);
AjPOutfile    ajAcdGetOutdata(const char *token);
AjPDirout     ajAcdGetOutdir (const char *token);
AjPStr        ajAcdGetOutdirName (const char *token);
AjPOutfile    ajAcdGetOutdiscrete(const char *token);
AjPOutfile    ajAcdGetOutdistance(const char *token);
AjPFile       ajAcdGetOutfile (const char *token);
AjPFile       ajAcdGetOutfileall (const char *token);
AjPOutfile    ajAcdGetOutfreq(const char *token);
AjPOutfile    ajAcdGetOutmatrix(const char *token);
AjPOutfile    ajAcdGetOutmatrixf(const char *token);
AjPOutfile    ajAcdGetOutproperties(const char *token);
AjPOutfile    ajAcdGetOutscop(const char *token);
AjPOutfile    ajAcdGetOuttree(const char *token);
AjPPatlistSeq ajAcdGetPattern (const char *token);
AjPPhyloProp  ajAcdGetProperties (const char *token);
AjPRange      ajAcdGetRange (const char *token);
AjPPatlistRegex ajAcdGetRegexp (const char *token);
AjPRegexp     ajAcdGetRegexpSingle (const char *token);
AjPReport     ajAcdGetReport (const char *token);
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
AjBool        ajAcdGetToggle (const char *token);
AjPPhyloTree* ajAcdGetTree (const char *token);
AjPPhyloTree  ajAcdGetTreeSingle (const char *token);

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

__deprecated AjBool        ajAcdIsUserdefined(const char* token);
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

__deprecated void          ajGraphicsInit(const char *pgm,
					  ajint argc, char * const argv[]);
__deprecated void          ajGraphicsInitPV(const char *pgm,
					    ajint argc,char * const argv[],
					    const char *package,
					    const char *packversion);


/*
** End of prototype definitions
*/


#endif

#ifdef __cplusplus
}
#endif
