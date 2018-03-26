//$c2   XRL 12/12/2011 Added scale and reflection.
//$c1   XRL 12/03/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
#include "Matrix.h"
#include <stdio.h>

#define SET_ROW(row, v1, v2, v3, v4 )    \
    _mat[(row)][0] = (v1); \
    _mat[(row)][1] = (v2); \
    _mat[(row)][2] = (v3); \
    _mat[(row)][3] = (v4);

#define INNER_PRODUCT(a,b,r,c) \
     ((a)._mat[r][0] * (b)._mat[0][c]) \
    +((a)._mat[r][1] * (b)._mat[1][c]) \
    +((a)._mat[r][2] * (b)._mat[2][c]) \
    +((a)._mat[r][3] * (b)._mat[3][c])


Matrix::Matrix()
{
	makeIdentity();
}

void Matrix::makeIdentity()
{
    SET_ROW(0,    1, 0, 0, 0 )
    SET_ROW(1,    0, 1, 0, 0 )
    SET_ROW(2,    0, 0, 1, 0 )
    SET_ROW(3,    0, 0, 0, 1 )
}

void Matrix::set(value_type a00, value_type a01, value_type a02, value_type a03,
			      value_type a10, value_type a11, value_type a12, value_type a13,
			      value_type a20, value_type a21, value_type a22, value_type a23,
			      value_type a30, value_type a31, value_type a32, value_type a33)
{
    SET_ROW(0, a00, a01, a02, a03 )
    SET_ROW(1, a10, a11, a12, a13 )
    SET_ROW(2, a20, a21, a22, a23 )
    SET_ROW(3, a30, a31, a32, a33 )
}

//////////////////////////////////////////////////
//		translation 

void Matrix::setTranslation( value_type x, value_type y, value_type z )
{
    SET_ROW(0,    1, 0, 0, 0 )
    SET_ROW(1,    0, 1, 0, 0 )
    SET_ROW(2,    0, 0, 1, 0 )
    SET_ROW(3,    x, y, z, 1 )
}

//////////////////////////////////////////////////
//		scale 
void Matrix::setScale( value_type x, value_type y, value_type z )
{
    SET_ROW(0,    x, 0, 0, 0 )
    SET_ROW(1,    0, y, 0, 0 )
    SET_ROW(2,    0, 0, z, 0 )
    SET_ROW(3,    0, 0, 0, 1 )
}

void Matrix::setScale( const Vec3d &c, value_type x, value_type y, value_type z )
{
	setScale(x,y,z);
	
	Vec3d t(-c.x(),-c.y(),-c.z());
	preMultTranslate(t);

	t.set(c.x(),c.y(),c.z());
	postMultTranslate(t);

}

///////////////////////////////////////////////////
//		reflection around origin

void Matrix::setReflection(value_type a, value_type b, value_type c)
{
	double lenSq = a*a + b*b + c*c;
    double tol = std::numeric_limits<double>::min();
    if (lenSq <= tol*tol)
    {
        _mat[0][0] = 0.0; _mat[1][0] = 0.0; _mat[2][0] = 0.0;
        _mat[0][1] = 0.0; _mat[1][1] = 0.0; _mat[2][1] = 0.0;
        _mat[0][2] = 0.0; _mat[1][2] = 0.0; _mat[2][2] = 0.0;
    }
    else
	{
		// source:
		//
		// http://en.wikipedia.org/wiki/Transformation_matrix#Reflection

	    _mat[0][0] = 1.0 - 2.0*a*a/lenSq;
        _mat[1][0] =     - 2.0*a*b/lenSq;
        _mat[2][0] =     - 2.0*a*c/lenSq;
        
        
        _mat[0][1] = _mat[1][0];
        _mat[1][1] = 1.0 - 2.0*b*b/lenSq;
        _mat[2][1] =     - 2.0*b*c/lenSq;
        
        _mat[0][2] = _mat[2][0];
        _mat[1][2] = _mat[2][1];
        _mat[2][2] = 1.0 - 2.0*c*c/lenSq;

		_mat[0][3] = 0;
		_mat[1][3] = 0;
		_mat[2][3] = 0;

		SET_ROW(3,    0, 0, 0, 1 );
	}
}

void Matrix::setReflection(const Vec3d &nor, const Vec3d &pos)
{
	setReflection(nor.x(), nor.y(), nor.z());

	Vec3d t(-pos.x(),-pos.y(),-pos.z());
	preMultTranslate(t);

	t.set(pos.x(),pos.y(),pos.z());
	postMultTranslate(t);
}


///////////////////////////////////////////////////
//		rotation around origin

#define QX  q._v[0]
#define QY  q._v[1]
#define QZ  q._v[2]
#define QW  q._v[3]

void Matrix::setRotate(const Quat& q)
{
    double length2 = q.length2();
    if (fabs(length2) <= std::numeric_limits<double>::min())
    {
        _mat[0][0] = 0.0; _mat[1][0] = 0.0; _mat[2][0] = 0.0;
        _mat[0][1] = 0.0; _mat[1][1] = 0.0; _mat[2][1] = 0.0;
        _mat[0][2] = 0.0; _mat[1][2] = 0.0; _mat[2][2] = 0.0;
    }
    else
    {
        double rlength2;
        // normalize quat if required.
        // We can avoid the expensive sqrt in this case since all 'coefficients' below are products of two q components.
        // That is a square of a square root, so it is possible to avoid that
        if (length2 != 1.0)
        {
            rlength2 = 2.0/length2;
        }
        else
        {
            rlength2 = 2.0;
        }
        
        // Source: Gamasutra, Rotating Objects Using Quaternions
        //
        //http://www.gamasutra.com/features/19980703/quaternions_01.htm
        
        double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
        
        // calculate coefficients
        x2 = rlength2*QX;
        y2 = rlength2*QY;
        z2 = rlength2*QZ;
        
        xx = QX * x2;
        xy = QX * y2;
        xz = QX * z2;
        
        yy = QY * y2;
        yz = QY * z2;
        zz = QZ * z2;
        
        wx = QW * x2;
        wy = QW * y2;
        wz = QW * z2;
        
        // Note.  Gamasutra gets the matrix assignments inverted, resulting
        // in left-handed rotations, which is contrary to OpenGL and OSG's 
        // methodology.  The matrix assignment has been altered in the next
        // few lines of code to do the right thing.
        // Don Burns - Oct 13, 2001
        _mat[0][0] = 1.0 - (yy + zz);
        _mat[1][0] = xy - wz;
        _mat[2][0] = xz + wy;
        
        
        _mat[0][1] = xy + wz;
        _mat[1][1] = 1.0 - (xx + zz);
        _mat[2][1] = yz - wx;
        
        _mat[0][2] = xz - wy;
        _mat[1][2] = yz + wx;
        _mat[2][2] = 1.0 - (xx + yy);
    }
}

Quat Matrix::getRotate() const
{
    Quat q;

    value_type s;
    value_type tq[4];
    int    i, j;

    // Use tq to store the largest trace
    tq[0] = 1 + _mat[0][0]+_mat[1][1]+_mat[2][2];
    tq[1] = 1 + _mat[0][0]-_mat[1][1]-_mat[2][2];
    tq[2] = 1 - _mat[0][0]+_mat[1][1]-_mat[2][2];
    tq[3] = 1 - _mat[0][0]-_mat[1][1]+_mat[2][2];

    // Find the maximum (could also use stacked if's later)
    j = 0;
    for(i=1;i<4;i++) j = (tq[i]>tq[j])? i : j;

    // check the diagonal
    if (j==0)
    {
        /* perform instant calculation */
        QW = tq[0];
        QX = _mat[1][2]-_mat[2][1]; 
        QY = _mat[2][0]-_mat[0][2]; 
        QZ = _mat[0][1]-_mat[1][0]; 
    }
    else if (j==1)
    {
        QW = _mat[1][2]-_mat[2][1]; 
        QX = tq[1];
        QY = _mat[0][1]+_mat[1][0]; 
        QZ = _mat[2][0]+_mat[0][2]; 
    }
    else if (j==2)
    {
        QW = _mat[2][0]-_mat[0][2]; 
        QX = _mat[0][1]+_mat[1][0]; 
        QY = tq[2];
        QZ = _mat[1][2]+_mat[2][1]; 
    }
    else /* if (j==3) */
    {
        QW = _mat[0][1]-_mat[1][0]; 
        QX = _mat[2][0]+_mat[0][2]; 
        QY = _mat[1][2]+_mat[2][1]; 
        QZ = tq[3];
    }

    s = sqrt(0.25/tq[j]);
    QW *= s;
    QX *= s;
    QY *= s;
    QZ *= s;

    return q;

}

void Matrix::setRotation( value_type angle, value_type x, value_type y, value_type z ) 
{
    makeIdentity();

    Quat quat;
    quat.makeRotate(angle, x, y, z);
    setRotate(quat);
}

void Matrix::setRotation(const Vec3d &axis, const Vec3d &pt, value_type theta)
{
	setRotation(theta, axis.x(), axis.y(), axis.z());

	Vec3d t(-pt.x(),-pt.y(),-pt.z());
	preMultTranslate(t);

	t.set(pt.x(),pt.y(),pt.z());
	postMultTranslate(t);
}

	
//////////////////////////////////////////////////////////
//			multiplication  - the workhorse

void Matrix::mult( const Matrix& lhs, const Matrix& rhs )
{   
    if (&lhs==this)
    {
        postMult(rhs);
        return;
    }
    if (&rhs==this)
    {
        preMult(lhs);
        return;
    }

// PRECONDITION: We assume neither &lhs nor &rhs == this
// if it did, use preMult or postMult instead
    _mat[0][0] = INNER_PRODUCT(lhs, rhs, 0, 0);
    _mat[0][1] = INNER_PRODUCT(lhs, rhs, 0, 1);
    _mat[0][2] = INNER_PRODUCT(lhs, rhs, 0, 2);
    _mat[0][3] = INNER_PRODUCT(lhs, rhs, 0, 3);
    _mat[1][0] = INNER_PRODUCT(lhs, rhs, 1, 0);
    _mat[1][1] = INNER_PRODUCT(lhs, rhs, 1, 1);
    _mat[1][2] = INNER_PRODUCT(lhs, rhs, 1, 2);
    _mat[1][3] = INNER_PRODUCT(lhs, rhs, 1, 3);
    _mat[2][0] = INNER_PRODUCT(lhs, rhs, 2, 0);
    _mat[2][1] = INNER_PRODUCT(lhs, rhs, 2, 1);
    _mat[2][2] = INNER_PRODUCT(lhs, rhs, 2, 2);
    _mat[2][3] = INNER_PRODUCT(lhs, rhs, 2, 3);
    _mat[3][0] = INNER_PRODUCT(lhs, rhs, 3, 0);
    _mat[3][1] = INNER_PRODUCT(lhs, rhs, 3, 1);
    _mat[3][2] = INNER_PRODUCT(lhs, rhs, 3, 2);
    _mat[3][3] = INNER_PRODUCT(lhs, rhs, 3, 3);
}

void Matrix::preMult( const Matrix& other )
{
    // brute force method requiring a copy
    //Matrix_implementation tmp(other* *this);
    // *this = tmp;

    // more efficient method just use a value_type[4] for temporary storage.
    value_type t[4];
    for(int col=0; col<4; ++col) {
        t[0] = INNER_PRODUCT( other, *this, 0, col );
        t[1] = INNER_PRODUCT( other, *this, 1, col );
        t[2] = INNER_PRODUCT( other, *this, 2, col );
        t[3] = INNER_PRODUCT( other, *this, 3, col );
        _mat[0][col] = t[0];
        _mat[1][col] = t[1];
        _mat[2][col] = t[2];
        _mat[3][col] = t[3];
    }

}

void Matrix::postMult( const Matrix& other )
{
    // brute force method requiring a copy
    //Matrix_implementation tmp(*this * other);
    // *this = tmp;

    // more efficient method just use a value_type[4] for temporary storage.
    value_type t[4];
    for(int row=0; row<4; ++row)
    {
        t[0] = INNER_PRODUCT( *this, other, row, 0 );
        t[1] = INNER_PRODUCT( *this, other, row, 1 );
        t[2] = INNER_PRODUCT( *this, other, row, 2 );
        t[3] = INNER_PRODUCT( *this, other, row, 3 );
        SET_ROW(row, t[0], t[1], t[2], t[3] )
    }
}


// debug
void Matrix::print()
{
	printf("(%f	%f	%f	%f)\n",_mat[0][0],_mat[0][1],_mat[0][2],_mat[0][3]);
	printf("(%f	%f	%f	%f)\n",_mat[1][0],_mat[1][1],_mat[1][2],_mat[1][3]);
	printf("(%f	%f	%f	%f)\n",_mat[2][0],_mat[2][1],_mat[2][2],_mat[2][3]);
	printf("(%f	%f	%f	%f)\n\n",_mat[3][0],_mat[3][1],_mat[3][2],_mat[3][3]);
}