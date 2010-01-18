/******************************************************************************
** @source AJAX 3D vector functions
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
** @version $Revision: 1.14 $
** @modified $Date: 2009/12/29 16:42:41 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/




/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"
#include <math.h>


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section 3-D vector Constructors *******************************************
**
** All constructors return a new vector by pointer. It is the responsibility
** of the user to first destroy any previous vector. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/

/* @func aj3dVectorNew *******************************************************
**
** Default constructor for zeroed AJAX 3D vectors.
**
** @return [AjP3dVector] Pointer to a zeroed 3D vector
** @category new [AjP3dVector] default constructor
** @@
******************************************************************************/

AjP3dVector aj3dVectorNew(void)
{
    AjP3dVector returnedVector = NULL;

    AJNEW0(returnedVector);

    return returnedVector;
}




/* @func aj3dVectorCreate ****************************************************
**
** Constructor for initialized AJAX 3D vectors.
** @param [r] fX [float] x component of 3D vector
** @param [r] fY [float] y component of 3D vector
** @param [r] fZ [float] z component of 3D vector
**
** @return [AjP3dVector] Pointer to an initialized 3D vector
** @category new [AjP3dVector] constructor initializing values of
**                vector components
** @@
******************************************************************************/

AjP3dVector aj3dVectorCreate(float fX, float fY, float fZ)
{
    AjP3dVector returnedVector;

    AJNEW0(returnedVector);
    returnedVector->x = fX;
    returnedVector->y = fY;
    returnedVector->z = fZ;

    return returnedVector;  
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */


/* @section 3D Vector Destructors ********************************************
**
** Destruction is achieved by deleting the pointer to the 3-D vector and
**  freeing the associated memory
**
******************************************************************************/

/* @func aj3dVectorDel *******************************************************
**
** Default destructor for Ajax 3-D Vectors.
**
** If the given pointer is NULL, or a NULL pointer, simply returns.
**
** @param  [d] pthis [AjP3dVector*] Pointer to the 3-D vector to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjP3dVector] default destructor
** @@
******************************************************************************/

void aj3dVectorDel(AjP3dVector* pthis)
{
    AjP3dVector thys = NULL;

    thys = pthis ? *pthis :0;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    thys->x = 0.0;
    thys->y = 0.0;
    thys->z = 0.0;
    AJFREE(thys);
    *pthis = NULL;

    return;
}




/* @section 3D Vector Functions ********************************************
**
** General functions
**
******************************************************************************/

/* @func aj3dVectorCrossProduct **********************************************
**
** calculates the cross product of two 3D vectors, that is their "torque"
**
** @param [r] first [const AjP3dVector] first 3D vector
** @param [r] second [const AjP3dVector] second 3D vector
** @param [w] crossProduct [AjP3dVector] 3D vector to contain
**            cross product
** @return [void]
** @category use [AjP3dVector] return cross product of two
**                vectors
** @@
******************************************************************************/

void aj3dVectorCrossProduct(const AjP3dVector first,
			    const AjP3dVector second,
			    AjP3dVector crossProduct)
{
    float X;
    float Y;
    float Z;

    /* compute cross product */
    X  = first->y * second->z;
    X -= first->z * second->y;

    crossProduct->x = X;

    Y  = first->z * second->x;
    Y -= first->x * second->z;

    crossProduct->y = Y;

    Z  = first->x * second->y;
    Z -= first->y * second->x;

    crossProduct->z = Z;

    return;
}




/* @func aj3dVectorBetweenPoints *********************************************
**
** Calculates the vector from one point in space (start) to another (end)
**
** @param [u] betweenPoints [AjP3dVector] vector from start to end
** @param [r] fStartX [float] X coordinate of start
** @param [r] fStartY [float] Y coordinate of start
** @param [r] fStartZ [float] Z coordinate of start
** @param [r] fEndX   [float] X coordinate of end
** @param [r] fEndY   [float] Y coordinate of end
** @param [r] fEndZ   [float] Z coordinate of end
**
** @return [void]
** @category modify [AjP3dVector] return vector between two
**                points
** @@
******************************************************************************/

void aj3dVectorBetweenPoints(AjP3dVector betweenPoints,
			     float fStartX, float fStartY, float fStartZ,
			     float fEndX, float fEndY, float fEndZ)
{
    /* compute vector between points */
    betweenPoints->x = fEndX - fStartX;
    betweenPoints->y = fEndY - fStartY;
    betweenPoints->z = fEndZ - fStartZ;

    return;
}




/* @func aj3dVectorLength ****************************************************
**
** calculates the magnitude of a vector
**
** @param [r] thys [const AjP3dVector] vector to be sized
**
** @return [float] length of vector to be sized
** @category cast [AjP3dVector] return length of vector
** @@
******************************************************************************/

float aj3dVectorLength(const AjP3dVector thys)
{
    float squarelen;
    float length;

    /* compute vector length */
    squarelen = thys->x * thys->x;

    squarelen += thys->y * thys->y;

    squarelen += thys->z * thys->z;

    length = (float)sqrt((double)squarelen);

    return length;
}




/* @func aj3dVectorAngle ******************************************************
**
** Calculates the angle between two vectors
**
** method adapted from vmd
**
** @param [r] first [const AjP3dVector] first vector
** @param [r] second [const AjP3dVector] second vector
**
** @return [float] angle between vectors in degrees
** @category use [AjP3dVector] return angle between two
**                vectors
** @@
******************************************************************************/

float aj3dVectorAngle(const AjP3dVector first,
		      const AjP3dVector second)
{
    float lenfirst;
    float lensecond;
    float lenproduct;
    
    float dotproduct;
    float radians;
    float degrees;

    AjP3dVector crossproduct=NULL;

    lenfirst   = aj3dVectorLength(first);
    lensecond = aj3dVectorLength(second);

    if((lenfirst < 0.0001) || (lensecond < 0.0001))
    {
	degrees = 180;
    }
    else
    {
	crossproduct  = aj3dVectorNew();
	/* compute vector angle */
        aj3dVectorCrossProduct(first, second, crossproduct);
	dotproduct = aj3dVectorDotProduct(first, second);
	lenproduct   = aj3dVectorLength(crossproduct);
	/* return the arctangent in the range -pi to +pi */
	radians   = (float)atan2((double)lenproduct, (double)dotproduct);
	degrees   = ajCvtRadToDeg(radians);
    }

    return( degrees );
}




/* @func aj3dVectorDihedralAngle ********************************************
**
** calculates the angle from the plane perpendicular to A x B to the plane
**  perpendicular to B x C (where A, B and C are vectors)
**
** @param [r] veca [const AjP3dVector] Vector A
** @param [r] vecb [const AjP3dVector] Vector B
** @param [r] vecc [const AjP3dVector] Vector C 
**
** @return [float] dihedral angle
** @category use [AjP3dVector] return angle between two
**                planes
** @@
******************************************************************************/

float aj3dVectorDihedralAngle(const AjP3dVector veca,
			      const AjP3dVector vecb,
			      const AjP3dVector vecc)
{ 
    float angle;
    float numerator;
    float denominator;
    float bterm;
    float sign = 1.0;

    AjP3dVector torque1    = NULL;
    AjP3dVector torque2   = NULL;
    AjP3dVector torqueall= NULL;

    torque1    = aj3dVectorNew();
    torque2    = aj3dVectorNew();
    torqueall  = aj3dVectorNew();

    aj3dVectorCrossProduct(veca, vecb,
			   torque1);
    aj3dVectorCrossProduct(vecb, vecc,
			   torque2);
    numerator = aj3dVectorDotProduct(torque1, torque2);
    denominator = aj3dVectorLength(torque1) * aj3dVectorLength(torque2);
    aj3dVectorCrossProduct(torque1, torque2, torqueall);
    
    bterm = numerator / denominator;
    angle = ajCvtRadToDeg( (float)acos((double)bterm) );

    /* get sign of angle of rotation */
    if( ( aj3dVectorDotProduct(vecb, torqueall) ) < 0.0 )
	sign = -1.0;

    aj3dVectorDel(&torque1);
    aj3dVectorDel(&torque2);
    aj3dVectorDel(&torqueall);

    return (sign * angle);
}




/* @func aj3dVectorDotProduct ***********************************************
**
** calculates the dot product of two 3D vectors, that is their summed common
**  scalar magnitude
**
** @param [r] first [const AjP3dVector] first vector
** @param [r] second [const AjP3dVector] second vector
**
** @return [float] dot product of first and second vectors
** @category use [AjP3dVector] return dot product of two
**                vectors
** @@
******************************************************************************/

float aj3dVectorDotProduct(const AjP3dVector first,
			   const AjP3dVector second)
{
    float dotproduct;

    /* compute dot product */
    dotproduct  = first->x * second->x;
    dotproduct += first->y * second->y;
    dotproduct += first->z * second->z;

    return(dotproduct);
}




/* @func aj3dVectorSum ********************************************************
**
** calculates the dot product of two 3D vectors, that is their summed common
**  "scalar magnitude"
**
** @param [r] first [const AjP3dVector] first vector
** @param [r] second [const AjP3dVector] second vector
** @param [w] sum [AjP3dVector] sum of first and second vectors
** @return [void]
** @category use [AjP3dVector] return sum of two vectors
** @@
******************************************************************************/

void aj3dVectorSum(const AjP3dVector first,
		   const AjP3dVector second,
		   AjP3dVector sum)
{
    /* compute sum of vectors by adding individual components */
    sum->x = first->x + second->x;
    sum->y = first->y + second->y;
    sum->z = first->z + second->z;

    return;
}
