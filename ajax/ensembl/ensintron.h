#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensintron_h
#define ensintron_h

#include "ensdata.h"




/*
** Prototype definitions
*/

/* Ensembl Intron */

EnsPIntron ensIntronNewExons(EnsPExon exon1, EnsPExon exon2);

EnsPIntron ensIntronNewObj(const EnsPIntron object);
    
EnsPIntron ensIntronNewRef(EnsPIntron intron);

void ensIntronDel(EnsPIntron* Pintron);

EnsPFeature ensIntronGetFeature(const EnsPIntron intron);

EnsPExon ensIntronGetPreviousexon(const EnsPIntron intron);

EnsPExon ensIntronGetNextexon(const EnsPIntron intron);

AjBool ensIntronTrace(const EnsPIntron intron, ajuint level);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif
