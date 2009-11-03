#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embmol_h
#define embmol_h


#define EMBMOLPARDISP (double)1000000.0

/* @data EmbPMolFrag **********************************************************
**
** Nucleus sequence molecular fragment object.
**
** @attr begin [ajint] Start
** @attr end [ajint] End
** @attr mwt [double] Molecular weight
** @@
******************************************************************************/

typedef struct EmbSMolFrag
{
    ajint begin;
    ajint end;
    double mwt;
} EmbOMolFrag;
#define EmbPMolFrag EmbOMolFrag*




/*
** Prototype definitions
*/

ajint embMolGetFrags(const AjPStr thys, ajint rno, AjPList *l);

/*
** End of prototype definitions
*/



#endif

#ifdef __cplusplus
}
#endif
