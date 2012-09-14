/* @include ajvector **********************************************************
**
** AJAX 3D vector functions
**
** AjP3dVector objects are structures each of three floats specifying the
**  components of vectors in Cartesian three-space.
**
** The three float values stored in an AjP3dVector are, respectively,
**  the components of the vector it describes in the positive X, Y and
**  Z directions in a conventional right-handed Cartesian system
**
** Alternatively they can be thought of as the coefficients of the
**  i, j, and k unit vectors in the x y and z directions respectively
**
** @author Copyright (C) 2003 Damian Counsell
** @version $Revision: 1.13 $
** @modified $Date: 2011/10/02 10:11:48 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJVECTOR_H
#define AJVECTOR_H




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjP3dVector **********************************************************
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
** @new aj3dVectorCreate constructor initialising values of vector components
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
/* =========================== public functions ============================ */
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




AJ_END_DECLS

#endif /* !AJVECTOR_H */
