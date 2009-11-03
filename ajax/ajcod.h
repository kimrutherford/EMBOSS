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

void         ajCodAssCode(AjPCod thys, ajint geneticcode);
void         ajCodAssDesc(AjPCod thys, const AjPStr desc);
void         ajCodAssDescC(AjPCod thys, const char* desc);
void         ajCodAssDivision(AjPCod thys, const AjPStr division);
void         ajCodAssDivisionC(AjPCod thys, const char* division);
void         ajCodAssNumcds(AjPCod thys, ajint numcds);
void         ajCodAssNumcodon(AjPCod thys, ajint numcodon);
void         ajCodAssRelease(AjPCod thys, const AjPStr release);
void         ajCodAssReleaseC(AjPCod thys, const char* release);
void         ajCodAssSpecies(AjPCod thys, const AjPStr species);
void         ajCodAssSpeciesC(AjPCod thys, const char* species);
void         ajCodAssName(AjPCod thys, const AjPStr name);
void         ajCodAssNameC(AjPCod thys, const char* name);
void         ajCodBacktranslate(AjPStr *b, const AjPStr a, const AjPCod thys);
void         ajCodBacktranslateAmbig(AjPStr *b, const AjPStr a,
				     const AjPCod thys);
ajint        ajCodBase(ajint c);
double       ajCodCai(const AjPCod cod, const AjPStr str);
double*      ajCodCaiW(const AjPCod cod);
void         ajCodCalcGribskov(AjPCod thys, const AjPStr s);
double       ajCodCalcCai(const AjPCod thys);
double       ajCodCalcNc(const AjPCod thys);
void         ajCodCalculateUsage(AjPCod thys, ajint c);
void         ajCodClear(AjPCod thys);
void         ajCodClearData(AjPCod thys);
void         ajCodComp(ajint *NA, ajint *NC, ajint *NG, ajint *NT,
		       const char *str);
void         ajCodCountTriplets(AjPCod thys, const AjPStr s, ajint *c);
void         ajCodDel (AjPCod *thys);
AjPCod       ajCodDup (const AjPCod thys);
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
AjPCod	     ajCodNewCode(ajint code);
AjBool       ajCodRead(AjPCod thys, const AjPStr fn, const AjPStr format);
void         ajCodSetBacktranslate(AjPCod *thys);
char*        ajCodTriplet(ajint idx);
void 	     ajCodWrite(AjPCod thys, AjPFile outf);
void 	     ajCodWriteOut( const AjPCod thys, AjPOutfile outf);
ajint        ajCodOutFormat(const AjPStr name);
void         ajCodPrintFormat(AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
