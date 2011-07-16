#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajutil_h
#define ajutil_h




/*
** Prototype definitions
*/

__noreturn void   ajExit (void);
__noreturn void   ajExitAbort (void);
__noreturn void   ajExitBad (void);

void          ajByteRevLen2 (ajshort* i);
void          ajByteRevLen4 (ajint* i);
void          ajByteRevLen8 (ajlong* i);
void          ajByteRevLen2u (ajushort* i);
void          ajByteRevLen4u (ajuint* i);
void          ajByteRevLen8u (ajulong* i);
void          ajByteRevInt(ajint* sval);
void          ajByteRevShort(ajshort* ival);
void          ajByteRevLong(ajlong* lval);
void          ajByteRevUint(ajuint* ival);
void          ajByteRevUlong(ajulong* lval);

void          ajReset(void);

void          ajUtilCatch (void);
AjBool        ajUtilGetBigendian (void);
AjBool        ajUtilGetUid (AjPStr* Puid);
void          ajUtilLoginfo (void);
const AjPStr  ajUtilGetProgram (void);
const AjPStr  ajUtilGetCmdline (void);
const AjPStr  ajUtilGetInputs (void);

size_t ajUtilBase64DecodeC(AjPStr *dest, const char *src);
AjBool ajUtilBase64EncodeC(AjPStr *dest, size_t size, const unsigned char *src);


/*
** End of prototype definitions
*/

__deprecated void  ajAcdProgramS(AjPStr* pgm);
__deprecated const char*  ajAcdProgram(void);
__deprecated const AjPStr  ajAcdGetProgram (void);
__deprecated const AjPStr  ajAcdGetCmdline (void);
__deprecated const AjPStr  ajAcdGetInputs (void);
__deprecated void   ajLogInfo (void);
__deprecated void   ajUtilRev2 (short* i);
__deprecated void   ajUtilRev4 (ajint* i);
__deprecated void   ajUtilRev8 (ajlong* i);
__deprecated void   ajUtilRevInt(ajint* ival);
__deprecated void   ajUtilRevShort(short* ival);
__deprecated void   ajUtilRevLong(ajlong* lval);
__deprecated void   ajUtilRevUint(ajuint* ival);
__deprecated AjBool ajUtilBigendian (void);
__deprecated AjBool ajUtilUid (AjPStr* dest);

#endif

#ifdef __cplusplus
}
#endif
