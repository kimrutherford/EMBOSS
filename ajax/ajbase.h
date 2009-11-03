#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajbase_h
#define ajbase_h

#include "ajax.h"
#include <ctype.h>

typedef struct AjIUB AjIUB;
struct AjIUB
{
    AjPStr code;
    AjPStr list;
};




/*
** Prototype definitions
*/

AjBool        ajBaseAa1ToAa3(char aa1, AjPStr *aa3);
AjBool        ajBaseAa3ToAa1(char *aa1, const AjPStr aa3);
const AjPStr  ajBaseCodes(ajint ibase);
__deprecated char          ajSeqBaseComp (char base);
char          ajBaseComp(char base);
void          ajBaseExit(void);
AjBool        ajBaseInit(void);
float         ajBaseProb(ajint base1, ajint base2);

ajint         ajAZToInt(ajint c);
ajint         ajAZToBin(ajint c);
char          ajAZToBinC(char c);
char          ajBinToAZ(ajint c);
ajint         ajIntToAZ(ajint n);

/*
** End of prototype definitions
*/

#endif


#ifdef __cplusplus
}
#endif
