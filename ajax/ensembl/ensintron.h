
#ifndef ENSINTRON_H
#define ENSINTRON_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdata.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Intron */

EnsPIntron ensIntronNewCpy(const EnsPIntron intron);

EnsPIntron ensIntronNewIni(EnsPExon exon1, EnsPExon exon2);

EnsPIntron ensIntronNewRef(EnsPIntron intron);

void ensIntronDel(EnsPIntron* Pintron);

EnsPFeature ensIntronGetFeature(const EnsPIntron intron);

EnsPExon ensIntronGetNextexon(const EnsPIntron intron);

EnsPExon ensIntronGetPreviousexon(const EnsPIntron intron);

AjBool ensIntronTrace(const EnsPIntron intron, ajuint level);

ajuint ensIntronCalculateLength(const EnsPIntron intron);

size_t ensIntronCalculateMemsize(const EnsPIntron intron);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSINTRON_H */
