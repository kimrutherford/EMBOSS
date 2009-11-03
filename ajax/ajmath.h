#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmath_h
#define ajmath_h

#include "ajax.h"

# define AJM_E			2.718281828459
# define AJM_PI			3.141592653589
# define AJM_PI_2		1.570796326794
# define AJM_PI_4		0.785398163397
# define AJM_1_PI		0.318309886183
# define AJM_2_PI		0.636619772367


#define AJMAX(a,b) ((a > b) ? a : b)
#define AJMIN(a,b) ((a < b) ? a : b)



/*
** Prototype definitions
*/

float              ajDegToRad (float degrees);
double             ajGaussProb (float mean, float sd, float score);
float              ajGeoMean(const float *s, ajint n);
void               ajPolToRec (float radius, float angle, float *x, float *y);
ajint              ajPosMod (ajint a, ajint b);
float              ajRadToDeg (float radians);
ajint              ajRandomNumber (void);
double             ajRandomNumberD (void);
void               ajRandomSeed (void);
void               ajRecToPol (float x, float y, float *radius, float *angle);
ajint              ajRound (ajint i, ajint round);
float              ajRoundF (float a, ajint nbits);
unsigned long long ajSp64Crc(const AjPStr thys);
ajuint             ajSp32Crc(const AjPStr thys);
ajuint             ajMathPosI(ajuint len, ajuint imin, ajint ipos);
ajuint             ajMathPos(ajuint len, ajint ipos);
ajuint             ajNumLengthDouble(double dnumber);
ajuint             ajNumLengthFloat(float fnumber);
ajuint             ajNumLengthInt(ajlong inumber);
ajuint             ajNumLengthUint(ajulong inumber);
/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
