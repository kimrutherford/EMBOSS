#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtax_h
#define ajtax_h

#include "ajtaxdata.h"
#include "ajax.h"

/*
** Prototype definitions
*/


AjPTax       ajTaxNew(void);
AjPTax       ajTaxNewTax(const AjPTax tax);
void         ajTaxDel(AjPTax *Ptax);
void         ajTaxClear(AjPTax tax);
const AjPStr ajTaxGetDb(const AjPTax tax);
const AjPStr ajTaxGetId(const AjPTax tax);
const AjPStr ajTaxGetName(const AjPTax tax);
ajuint       ajTaxGetParent(const AjPTax tax);
const char*  ajTaxGetQryC(const AjPTax tax);
const AjPStr ajTaxGetQryS(const AjPTax tax);
const AjPStr ajTaxGetRank(const AjPTax tax);
ajuint       ajTaxGetTree(const AjPTax tax, AjPList taxlist);
AjBool       ajTaxIsHidden(const AjPTax tax);
AjBool       ajTaxIsSpecies(const AjPTax tax);

AjPTaxcit    ajTaxcitNew(void);
void         ajTaxcitDel(AjPTaxcit *Pcit);

AjPTaxcode   ajTaxcodeNew(void);

AjPTaxdel    ajTaxdelNew(void);

AjPTaxdiv    ajTaxdivNew(void);

AjPTaxmerge  ajTaxmergeNew(void);

AjPTaxname   ajTaxnameNew(void);
AjPTaxname   ajTaxnameNewName(const AjPTaxname name);
void         ajTaxnameDel(AjPTaxname *Pname);

void         ajTaxExit(void);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
