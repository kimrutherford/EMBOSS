#ifdef __cplusplus
extern "C"
{
#endif


#ifndef aj3dvector_h
#define aj3dvector_h




/* @data AjP3dVector *********************************************************
**
** Ajax 3-D vector object.
**
** Holds three floats
**
** AjP3dVector is implemented as a pointer to a C data structure.
**
** @alias AjS3dVector
** @alias AjO3dVector
**
** @new aj3dVectorNew default constructor
** @new aj3dVectorCreate constructor initializing values of vector components
**
** @delete aj3dVectorDel default destructor
**
** @use aj3dVectorSum return sum of two vectors
** @use aj3dVectorDotProduct return dot product of two vectors
** @use aj3dVectorCrossProduct return cross product of two vectors
** @use aj3dVectorAngle return angle between two vectors
** @use aj3dVectorDihedralAngle return angle between two planes
** @modify aj3dVectorBetweenPoints return vector between two points
**
** @cast aj3dVectorLength return length of vector
**
** @attr x [float] x coordinate
** @attr y [float] y coordinate
** @attr z [float] z coordinate
** @@
******************************************************************************/

typedef struct AjS3dVector
{
    float x;
    float y;
    float z;
} AjO3dVector;
#define AjP3dVector AjO3dVector*




/* ========================================================================= */
/* =================== All functions in alphabetical order ================= */
/* ========================================================================= */

/*
** Prototype definitions
*/

float       aj3dVectorAngle(const AjP3dVector first,
			    const AjP3dVector fecond);
void        aj3dVectorBetweenPoints(AjP3dVector betweenPoints,
				    float X, float Y,
				    float Z, float endX, float endY,
				    float endZ);
AjP3dVector aj3dVectorCreate(float fX, float fY, float fZ);
void        aj3dVectorCrossProduct(const AjP3dVector first,
				   const AjP3dVector second,
				   AjP3dVector crossproduct);
float       aj3dVectorDihedralAngle(const AjP3dVector veca,
				    const AjP3dVector vecb,
				    const AjP3dVector vecc);
void        aj3dVectorDel(AjP3dVector* pthys);
float       aj3dVectorDotProduct(const AjP3dVector first,
				 const AjP3dVector second);
float       aj3dVectorLength(const AjP3dVector thys);
AjP3dVector aj3dVectorNew(void);
void        aj3dVectorSum(const AjP3dVector first,
			  const AjP3dVector second,
			  AjP3dVector sum);

/*
** End of prototype definitions
*/

#endif




#ifdef __cplusplus
}
#endif
