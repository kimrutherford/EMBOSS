#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmatrices_h
#define ajmatrices_h




/* @data AjPMatrix ************************************************************
**
** AJAX data structure for sequence comparison matrices with integer values
**
** @alias AjSMatrix
** @alias AjOMatrix
**
** @attr Size [ajint] Matrix size (size of 2D array)
** @attr SizeRow [ajint] Number of rows (if different to Size)
** @attr Name [AjPStr] Matrix name
** @attr Codes [AjPStr*] Row/column codes
** @attr CodesRow [AjPStr*] Row codes (if different to Codes)
** @attr Matrix [AjIntArray*] Matrix as 2D array
** @attr Cvt [AjPSeqCvt] Conversion table
** @@
******************************************************************************/

typedef struct AjSMatrix {
  ajint Size;
  ajint SizeRow;
  AjPStr Name;
  AjPStr* Codes;
  AjPStr* CodesRow;
  AjIntArray* Matrix;
  AjPSeqCvt Cvt;
} AjOMatrix;
#define AjPMatrix AjOMatrix*




/* @data AjPMatrixf ***********************************************************
**
** AJAX data structure for sequence comparison matrices with floating
** point values
**
** @alias AjSMatrixf
** @alias AjOMatrixf
**
** @attr Size [ajint] Matrix size (size of 2D array)
** @attr SizeRow [ajint] Number of rows (if different to Size)
** @attr Name [AjPStr] Matrix name
** @attr Codes [AjPStr*] Row/column codes
** @attr CodesRow [AjPStr*] Row codes (if different to Codes)
** @attr Matrixf [AjFloatArray*] Matrix as 2D array
** @attr Cvt [AjPSeqCvt] Conversion table
** @@
******************************************************************************/

typedef struct AjSMatrixf {
  ajint Size;
  ajint SizeRow;
  AjPStr Name;
  AjPStr* Codes;
  AjPStr* CodesRow;
  AjFloatArray* Matrixf;
  AjPSeqCvt Cvt;
} AjOMatrixf;
#define AjPMatrixf AjOMatrixf*




/*
** Prototype definitions
*/

void          ajMatrixDel (AjPMatrix *thys);
AjPStr        ajMatrixGetCodes(const AjPMatrix thys);
AjPSeqCvt     ajMatrixGetCvt (const AjPMatrix thys);
const AjPStr  ajMatrixGetLabelNum (const AjPMatrix thys, ajint i);
AjIntArray*   ajMatrixGetMatrix (const AjPMatrix thys);
const AjPStr  ajMatrixGetName (const AjPMatrix thys);
ajuint        ajMatrixGetRows (const AjPMatrix thys);
ajuint        ajMatrixGetSize (const AjPMatrix thys);
AjPMatrix     ajMatrixNew (const AjPPStr codes, ajint n,
			   const AjPStr filename);
AjPMatrix     ajMatrixNewAsym(const AjPPStr codes, ajint n, 
			      const AjPPStr rcodes, ajint rn, 
			      const AjPStr filename);
AjPMatrix     ajMatrixNewFile (const AjPStr filename);
AjBool        ajMatrixSeqIndex (const AjPMatrix thys, const AjPSeq seq,
                                AjPStr* numseq);

void          ajMatrixfDel (AjPMatrixf *thys);
AjPStr        ajMatrixfGetCodes(const AjPMatrixf thys);
AjPSeqCvt     ajMatrixfGetCvt (const AjPMatrixf thys);
const AjPStr  ajMatrixfGetLabelNum (const AjPMatrixf thys, ajint i);
AjFloatArray* ajMatrixfGetMatrix (const AjPMatrixf thys);
const AjPStr  ajMatrixfGetName (const AjPMatrixf thys);
ajuint        ajMatrixfGetRows (const AjPMatrixf thys);
ajuint        ajMatrixfGetSize (const AjPMatrixf thys);
AjPMatrixf    ajMatrixfNew (const AjPPStr codes, ajint n,
			    const AjPStr filename);
AjPMatrixf    ajMatrixfNewAsym(const AjPPStr codes, ajint n, 
			       const AjPPStr rcodes, ajint rn, 
			       const AjPStr filename);
AjPMatrixf    ajMatrixfNewFile (const AjPStr filename);
AjBool        ajMatrixfSeqIndex (const AjPMatrixf thys, const AjPSeq seq,
                                 AjPStr* numseq);
void          ajMatrixExit(void);


/*
** End of prototype definitions
*/


__deprecated void          ajMatrixfChar (const AjPMatrixf thys,
                                          ajint i, AjPStr *label);
__deprecated void          ajMatrixChar (const AjPMatrix thys,
                                         ajint i, AjPStr *label);
__deprecated const AjPStr  ajMatrixfName (const AjPMatrixf thys);
__deprecated const AjPStr  ajMatrixName (const AjPMatrix thys);
__deprecated AjPSeqCvt     ajMatrixCvt (const AjPMatrix thys);
__deprecated AjPSeqCvt     ajMatrixfCvt (const AjPMatrixf thys);
__deprecated ajint         ajMatrixSize (const AjPMatrix thys);
__deprecated ajint         ajMatrixfSize (const AjPMatrixf thys);
__deprecated AjIntArray*   ajMatrixArray (const AjPMatrix thys);
__deprecated AjFloatArray* ajMatrixfArray (const AjPMatrixf thys);
__deprecated AjBool        ajMatrixRead (AjPMatrix* pthis,
                                         const AjPStr filename);
__deprecated AjBool        ajMatrixfRead (AjPMatrixf* pthis,
                                          const AjPStr filename);
__deprecated AjBool        ajMatrixSeqNum (const AjPMatrix thys,
                                           const AjPSeq seq, AjPStr* numseq);
__deprecated AjBool        ajMatrixfSeqNum (const AjPMatrixf thys,
                                           const AjPSeq seq, AjPStr* numseq);

#endif

#ifdef __cplusplus
}
#endif
