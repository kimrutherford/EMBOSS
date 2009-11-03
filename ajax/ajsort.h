#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsort_h
#define ajsort_h




/*
** Prototype definitions
*/

extern void ajSortFloatDecI(const float *a, ajuint *p, ajuint n);
extern void ajSortIntDecI(const ajint *a, ajuint *p, ajuint n);
extern void ajSortUintDecI(const ajuint *a, ajuint *p, ajuint n);
extern void ajSortFloatIncI(const float *a, ajuint *p, ajuint n);
extern void ajSortIntIncI(const ajint *a, ajuint *p, ajuint n);
extern void ajSortUintIncI(const ajuint *a, ajuint *p, ajuint n);
extern void ajSortFloatDec(float *a, ajuint n);
extern void ajSortIntDec(ajint *a, ajuint n);
extern void ajSortUintDec(ajuint *a, ajuint n);
extern void ajSortFloatInc(float *a, ajuint n);
extern void ajSortIntInc(ajint *a, ajuint n);
extern void ajSortUintInc(ajuint *a, ajuint n);
extern void ajSortTwoIntIncI(ajint *a, ajuint *p, ajuint n);
extern void ajSortTwoUintIncI(ajuint *a, ajuint *p, ajuint n);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif
