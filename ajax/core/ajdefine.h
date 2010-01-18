#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdefine_h
#define ajdefine_h

#include "ajarch.h"

#ifndef WIN32
#define SLASH_CHAR   '/'
#define SLASH_STRING "/"
#define CURRENT_DIR  "./"
#define UP_DIR       "../"
#define PATH_SEPARATOR ":"
#else
#define SLASH_CHAR   '\\'
#define SLASH_STRING "\\"
#define CURRENT_DIR  ".\\"
#define UP_DIR       "..\\"
#define PATH_SEPARATOR ";"
#endif


typedef void fvoid_t(void);		/* void function type */

enum capacity {default_size, reserve};

/* @datatype AjBool ***********************************************************
**
** Boolean data type
**
** Used to store true (ajTrue) and false (ajFalse) values.
**
** ajFalse is defined as zero, and the data type is equivalent to "ajint".
**
** For definitions, macros AJTRUE and AJFALSE are also defined.
**
** On output, conversion code "%b" writes "Y" or "N"
** while conversion code "%B" writes "Yes" or "No".
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjBool;

/* @datatype AjStatus *********************************************************
**
** Status code returned with bit fields.
**
** Intended as a general return code for functions, but currently unused
** because AjBool is enough. Indicates OK, info,
** warning, error and fatal returns
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjStatus;

/* @datatype AjEnum *********************************************************
**
** Undocumented
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjEnum;

/* @datatype AjMask *********************************************************
**
** Undocumented
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjMask;

/* @datatype AjInt4 *********************************************************
**
** 4 bytes integer
**
** @attr typedef [ajint] Value
** @@
******************************************************************************/

typedef ajint AjInt4;		/* 4 bytes integer */

/* @datatype AjIntArray *******************************************************
**
** Array of integers
**
** @attr typedef [ajint*] Value
** @@
******************************************************************************/

typedef ajint* AjIntArray;

/* @datatype AjFloatArray *****************************************************
**
** Array of floats
**
** @attr typedef [float*] Value
** @@
******************************************************************************/

typedef float* AjFloatArray;

#define AJAXLONGDOUBLE double

#define AJBOOL(b) (b ? "TRUE" : "FALSE")
static const ajint ajFltDig = 3;

static const ajint ajFalse = 0;
static const ajint ajTrue = 1;
static const ajint ajStatusOK = 0;
static const ajint ajStatusInfo = 1;
static const ajint ajStatusWarn = 2;
static const ajint ajStatusError = 4;
static const ajint ajStatusFatal = 8;

#define AJFALSE 0
#define AJTRUE 1

#ifdef commentedout
#define ajFalse 0
#define ajTrue 1
#define ajStatusOK 0
#define ajStatusInfo 1
#define ajStatusWarn 2
#define ajStatusError 4
#define ajStatusFatal 8
#endif

#define CASE2(a,b) ((a << 8) + b)
#define CASE3(a,b,c) ((a << 16) + (b << 8) + c)
#define CASE4(a,b,c,d) ((a << 24) + (b << 16) + (c << 8) + d)

#define STRCASE2(a) ((a[0] << 8) + a[1])

static const ajint ajXRAY = 0;    /* Structure was determined by X-ray crystallography */
static const ajint ajNMR  = 1;    /* Structure was determined by NMR or is a model     */
static const ajint ajPDB  = 0;    /* Use original PDB residue numbering                */
static const ajint ajIDX  = 1;    /* Use corrected residue numbering                   */
#define ajESCOP "Escop.dat"       /* Scop data file */
#define ajSCOP   1                /* Type of domain */
#define ajCATH   2                /* Type of domain */
#define ajINTRA  1                /* Type of contact */
#define ajINTER  2                /* Type of contact */
#define ajLIGAND 3                /* Type of contact */
#define aj1D     1                /* Type of signature */
#define aj3D     2                /* Type of signature */



#endif

#ifdef __cplusplus
}
#endif
