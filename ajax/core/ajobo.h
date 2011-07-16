#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajobo_h
#define ajobo_h

#include "ajobodata.h"
#include "ajax.h"

/*
** Prototype definitions
*/

AjPObo           ajOboNew(void);
AjPObo           ajOboNewObo(const AjPObo obo);

void             ajOboClear(AjPObo thys);
void             ajOboDel(AjPObo *Pobo);

AjPObotag        ajObotagNewData(const AjPStr name, const AjPStr value,
                                 const AjPStr modifier, const AjPStr comment,
                                 ajuint linenum);
AjPObotag        ajObotagNewTag(const AjPObotag tag);
void             ajObotagDel(AjPObotag* Ptag);

AjPOboxref       ajOboxrefNewData(const AjPStr name, const AjPStr desc);
AjPOboxref       ajOboxrefNewXref(const AjPOboxref xref);
void             ajOboxrefDel(AjPOboxref* Pxref);

AjBool           ajOboqryGetObo(const AjPStr thys, AjPObo obo);

const AjPStr     ajOboGetDb(const AjPObo obo);
AjBool           ajOboGetDef(const AjPObo obo, AjPStr *Pdefstr, ajuint *nrefs);
const AjPStr     ajOboGetEntry(const AjPObo obo);
const AjPStr     ajOboGetId(const AjPObo obo);
const AjPStr     ajOboGetNamespace(const AjPObo obo);
ajuint           ajOboGetParents(const AjPObo obo, AjPList uplist);
const char*      ajOboGetQryC(const AjPObo obo);
const AjPStr     ajOboGetQryS(const AjPObo obo);
const AjPStr     ajOboGetReplaced(const AjPObo obo);
ajuint           ajOboGetTree(const AjPObo obo, AjPList obolist);
AjBool           ajOboIsObsolete(const AjPObo obo);

void             ajOboExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
