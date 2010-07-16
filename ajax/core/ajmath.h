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

float              ajCvtDegToRad (float degrees);
double             ajCvtGaussToProb (float mean, float sd, float score);
void               ajCvtPolToRec (float radius, float angle,
                                  float *x, float *y);
float              ajCvtRadToDeg (float radians);
ajuint             ajCvtSposToPos(ajuint len, ajint ipos);
ajuint             ajCvtSposToPosStart(ajuint len, ajuint imin, ajint ipos);
float              ajMathGmean(const float *s, ajint n);
ajint              ajMathModulo (ajint a, ajint b);
ajint              ajRandomNumber (void);
double             ajRandomDouble (void);
void               ajRandomSeed (void);
void               ajCvtRecToPol (float x, float y,
                                  float *radius, float *angle);
ajint              ajRound (ajint i, ajint vround);
float              ajRoundFloat (float a, ajint nbits);
unsigned long long ajMathCrc64(const AjPStr thys);
ajuint             ajMathCrc32(const AjPStr thys);
ajuint             ajNumLengthDouble(double dnumber);
ajuint             ajNumLengthFloat(float fnumber);
ajuint             ajNumLengthInt(ajlong inumber);
ajuint             ajNumLengthUint(ajulong inumber);

/*
** End of prototype definitions
*/

__deprecated float              ajRoundF (float a, ajint nbits);
__deprecated double             ajRandomNumberD (void);
__deprecated unsigned long long ajSp64Crc(const AjPStr thys);
__deprecated ajuint             ajSp32Crc(const AjPStr thys);
__deprecated ajint              ajPosMod (ajint a, ajint b);
__deprecated float              ajGeoMean(const float *s, ajint n);
__deprecated double             ajGaussProb (float mean, float sd, float score);
__deprecated ajuint             ajMathPosI(ajuint len, ajuint imin, ajint ipos);
__deprecated ajuint             ajMathPos(ajuint len, ajint ipos);
__deprecated float              ajDegToRad (float degrees);
__deprecated float              ajRadToDeg (float radians);
__deprecated void               ajPolToRec (float radius, float angle,
                                            float *x, float *y);
__deprecated void               ajRecToPol (float x, float y,
                                            float *radius, float *angle);
#endif

#ifdef __cplusplus
}
#endif
