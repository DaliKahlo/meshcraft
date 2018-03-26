//$c1   XRL 12/03/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
//
//
//		Matrix.h: 4*4 row-major homogeneous transformation matrix.
//
//			(x',y',z',s') = (x,y,z,s) M
//			where the conventional 3-d coordinates are (x/s,y/s,z/s)
//
//			The matrix thus consists of
//				(             , Px )
//				(      R      , Py )
//			    (             , Pz )
//			    ( Tx, Ty, Tz,   S  )
//
//			The subscripts of 'matrix' corresponding to the above form are:
//			    ( [0][0]  [1][0]  [2][0]  [3][0] )
//			    ( [0][1]  [1][1]  [2][1]  [3][1] )
//			    ( [0][2]  [1][2]  [2][2]  [3][2] )
//			    ( [0][3]  [1][3]  [2][3]  [3][3] )
//========================================================================//
#ifndef H_MATRIX4__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MATRIX4__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "Quat.h"

class Matrix
{
public:
	typedef double value_type;

	Matrix();
	~Matrix() {}

	// define the matrix
	void set(value_type a00, value_type a01, value_type a02, value_type a03,
		     value_type a10, value_type a11, value_type a12, value_type a13,
		     value_type a20, value_type a21, value_type a22, value_type a23,
		     value_type a30, value_type a31, value_type a32, value_type a33);

	void setTranslation(value_type dx, value_type dy, value_type dz);
	void setScale( const Vec3d &center, value_type x_scale, value_type y_scale, value_type z_scale );
	void setRotation(const Vec3d &axis, const Vec3d &position, value_type theta);
	void setReflection(const Vec3d &normal, const Vec3d &position);

	void setRotate(const Quat&);
	Quat getRotate() const;

	// basic Matrix multiplication
    void mult( const Matrix&, const Matrix& );
    void preMult( const Matrix& );
    void postMult( const Matrix& );

	inline void preMultTranslate( const Vec3d& v );
	inline void postMultTranslate( const Vec3d& v );

	// point and Matrix multiplication
    inline Vec3d preMult( const Vec3d& ) const;
    inline Vec3d postMult( const Vec3d& ) const;

	// for debug
	void print();

protected:
	value_type _mat[4][4];

	void makeIdentity();
	void setRotation(value_type theta, value_type x, value_type y, value_type z); // axis orientation
	void setScale( value_type x_scale, value_type y_scale, value_type z_scale );  // aniso scales
	void setReflection(value_type a, value_type b, value_type c);	// plane orientation

};

inline void Matrix::preMultTranslate( const Vec3d& v )
{
    for (unsigned i = 0; i < 3; ++i)
    {
        double tmp = v[i];
        if (tmp == 0)
            continue;
        _mat[3][0] += tmp*_mat[i][0];
        _mat[3][1] += tmp*_mat[i][1];
        _mat[3][2] += tmp*_mat[i][2];
        _mat[3][3] += tmp*_mat[i][3];
    }
}

inline void Matrix::postMultTranslate( const Vec3d& v )
{
    for (unsigned i = 0; i < 3; ++i)
    {
        double tmp = v[i];
        if (tmp == 0)
            continue;
        _mat[0][i] += tmp*_mat[0][3];
        _mat[1][i] += tmp*_mat[1][3];
        _mat[2][i] += tmp*_mat[2][3];
        _mat[3][i] += tmp*_mat[3][3];
    }
}

inline Vec3d Matrix::postMult( const Vec3d& v ) const
{
    value_type d = 1.0f/(_mat[3][0]*v.x()+_mat[3][1]*v.y()+_mat[3][2]*v.z()+_mat[3][3]) ;
    return Vec3d( (_mat[0][0]*v.x() + _mat[0][1]*v.y() + _mat[0][2]*v.z() + _mat[0][3])*d,
        (_mat[1][0]*v.x() + _mat[1][1]*v.y() + _mat[1][2]*v.z() + _mat[1][3])*d,
        (_mat[2][0]*v.x() + _mat[2][1]*v.y() + _mat[2][2]*v.z() + _mat[2][3])*d) ;
}

inline Vec3d Matrix::preMult( const Vec3d& v ) const
{
    value_type d = 1.0f/(_mat[0][3]*v.x()+_mat[1][3]*v.y()+_mat[2][3]*v.z()+_mat[3][3]) ;
    return Vec3d( (_mat[0][0]*v.x() + _mat[1][0]*v.y() + _mat[2][0]*v.z() + _mat[3][0])*d,
        (_mat[0][1]*v.x() + _mat[1][1]*v.y() + _mat[2][1]*v.z() + _mat[3][1])*d,
        (_mat[0][2]*v.x() + _mat[1][2]*v.y() + _mat[2][2]*v.z() + _mat[3][2])*d);
}


#endif
