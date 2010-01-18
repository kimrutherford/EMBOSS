#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajcod_h
#define ajcod_h


/* @data AjPCod ***************************************************************
**
** Ajax codon object.
**
** Holds arrays describing codon usage
** The length is known and held internally.
**
** AjPCod is implemented as a pointer to a C data structure.
**
** @alias AjSCod
** @alias AjOCod
**
** @attr Name [AjPStr] Name of codon file
** @attr Species [AjPStr] Species
** @attr Division [AjPStr] Division (gbbct etc.)
** @attr Release [AjPStr] Database name and release
** @attr Desc [AjPStr] Description
** @attr CdsCount [ajint] Number of coding sequences used
** @attr CodonCount [ajint] Number of individual codons used
** @attr aa [ajint*] Amino acid represented by codon
** @attr num [ajint*] Number of codons
** @attr tcount [double*] Codons per thousand
** @attr fraction [double*] Fraction of amino acids of this type
** @attr back [ajint*] Index of favoured amino acid for backtranslation
** @attr GeneticCode [ajint] Genetic code NCBI number to match
**                           amino acids to codons.
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSCod
{
    AjPStr Name;
    AjPStr Species;
    AjPStr Division;
    AjPStr Release;
    AjPStr Desc;
    ajint CdsCount;
    ajint CodonCount;
    ajint *aa;
    ajint *num;
    double *tcount;
    double *fraction;
    ajint *back;
    ajint GeneticCode;
    char Padding[4];
} AjOCod;
#define AjPCod AjOCod*



/*
** Prototype definitions
*/

void         ajCodSetCodenum(AjPCod thys, ajint geneticcode);
void         ajCodSetDescC(AjPCod thys, const char* desc);
void         ajCodSetDescS(AjPCod thys, const AjPStr desc);
void         ajCodSetDivisionC(AjPCod thys, const char* division);
void         ajCodSetDivisionS(AjPCod thys, const AjPStr division);
void         ajCodSetNameC(AjPCod thys, const char* name);
void         ajCodSetNameS(AjPCod thys, const AjPStr name);
void         ajCodSetNumcds(AjPCod thys, ajint numcds);
void         ajCodSetNumcodons(AjPCod thys, ajint numcodon);
void         ajCodSetReleaseC(AjPCod thys, const char* release);
void         ajCodSetReleaseS(AjPCod thys, const AjPStr release);
void         ajCodSetSpeciesC(AjPCod thys, const char* species);
void         ajCodSetSpeciesS(AjPCod thys, const AjPStr species);
void         ajCodBacktranslate(AjPStr *b, const AjPStr a, const AjPCod thys);
void         ajCodBacktranslateAmbig(AjPStr *b, const AjPStr a,
				     const AjPCod thys);
ajint        ajCodBase(ajint c);
double       ajCodCalcCaiCod(const AjPCod thys);
double       ajCodCalcCaiSeq(const AjPCod cod, const AjPStr str);
void         ajCodCalcGribskov(AjPCod thys, const AjPStr s);
double       ajCodCalcNc(const AjPCod thys);
void         ajCodCalcUsage(AjPCod thys, ajint c);
void         ajCodClear(AjPCod thys);
void         ajCodClearData(AjPCod thys);
void         ajCodComp(ajint *NA, ajint *NC, ajint *NG, ajint *NT,
		       const char *str);
void         ajCodSetTripletsS(AjPCod thys, const AjPStr s, ajint *c);
void         ajCodDel (AjPCod *thys);
ajint        ajCodGetCode(const AjPCod thys);
const AjPStr ajCodGetDesc(const AjPCod thys);
const char*  ajCodGetDescC(const AjPCod thys);
const AjPStr ajCodGetDivision(const AjPCod thys);
const char*  ajCodGetDivisionC(const AjPCod thys);
void         ajCodExit(void);
void         ajCodGetCodonlist(const AjPCod cod, AjPList list);
const AjPStr ajCodGetName(const AjPCod thys);
const char*  ajCodGetNameC(const AjPCod thys);
ajint        ajCodGetNumcds(const AjPCod thys);
ajint        ajCodGetNumcodon(const AjPCod thys);
const AjPStr ajCodGetRelease(const AjPCod thys);
const char*  ajCodGetReleaseC(const AjPCod thys);
const AjPStr ajCodGetSpecies(const AjPCod thys);
const char*  ajCodGetSpeciesC(const AjPCod thys);
ajint        ajCodIndex(const AjPStr s);
ajint        ajCodIndexC(const char *codon);
AjPCod	     ajCodNew(void);
AjPCod	     ajCodNewCodenum(ajint code);
AjPCod       ajCodNewCod (const AjPCod thys);
AjBool       ajCodRead(AjPCod thys, const AjPStr fn, const AjPStr format);
void         ajCodSetBacktranslate(AjPCod thys);
char*        ajCodTriplet(ajint idx);
void 	     ajCodWrite(AjPCod thys, AjPFile outf);
void 	     ajCodWriteOut( const AjPCod thys, AjPOutfile outf);
ajint        ajCodOutFormat(const AjPStr name);
void         ajCodPrintFormat(AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

__deprecated AjPCod       ajCodDup (const AjPCod thys);
__deprecated AjPCod	  ajCodNewCode(ajint code);
__deprecated void         ajCodAssCode(AjPCod thys, ajint geneticcode);
__deprecated void         ajCodAssDesc(AjPCod thys, const AjPStr desc);
__deprecated void         ajCodAssDescC(AjPCod thys, const char* desc);
__deprecated void         ajCodAssDivision(AjPCod thys, const AjPStr division);
__deprecated void         ajCodAssDivisionC(AjPCod thys, const char* division);
__deprecated void         ajCodAssNumcds(AjPCod thys, ajint numcds);
__deprecated void         ajCodAssNumcodon(AjPCod thys, ajint numcodon);
__deprecated void         ajCodAssRelease(AjPCod thys, const AjPStr release);
__deprecated void         ajCodAssReleaseC(AjPCod thys, const char* release);
__deprecated void         ajCodAssSpecies(AjPCod thys, const AjPStr species);
__deprecated void         ajCodAssSpeciesC(AjPCod thys, const char* species);
__deprecated void         ajCodAssName(AjPCod thys, const AjPStr name);
__deprecated void         ajCodAssNameC(AjPCod thys, const char* name);
__deprecated void         ajCodCountTriplets(AjPCod thys,
                                             const AjPStr s, ajint *c);
__deprecated double       ajCodCalcCai(const AjPCod cod, const AjPStr str);
__deprecated void         ajCodCalculateUsage(AjPCod thys, ajint c);

#endif

#ifdef __cplusplus
}
#endif
