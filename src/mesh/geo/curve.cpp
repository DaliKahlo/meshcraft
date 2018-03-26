//$c1   XRL 05/28/2015 Created.
//========================================================================//
//========================================================================//
#include "curve.h"
#include "farin.h"

Curve::Curve(geomType t, char *nm, int np, double *ptArr)
{
	Vec3d pt;
	_type = t;
	_name = nm;
	_pBezier = NULL;
	for(int i=0; i<np; i++) {
		pt.set(ptArr[3*i],ptArr[3*i+1],ptArr[3*i+2]);
		_pts.push_back(pt);
	}
}

Curve::~Curve()
{
	if(_pBezier) {
		delete _pBezier;
		_pBezier = NULL;
	}
}

size_t Curve::numPoints()
{
	return _pts.size();
}

void Curve::ithPoint(int i, double *x, double *y, double *z)
{
	*x = _pts[i].x();
	*y = _pts[i].y();
	*z = _pts[i].z();
}

std::string & Curve::name()
{
	return _name;
}

void Curve::computeBezier()
{
	int i, np = numPoints();
	if(np < 2)
		return;

	///
	double *coords = new double[3*np];
	for(i=0; i<np; i++) {
		coords[3*i] = _pts[i].x();
		coords[3*i + 1] = _pts[i].y();
		coords[3*i + 2] = _pts[i].z();
	}
	///
	int pt_dim = 3;
	int order = np - 1;
	_pBezier = new ON_BezierCurve(pt_dim, FALSE, order);
	_type=Bezier;

	///
	bool ret;
	double* param = NULL;
	if(np > 3) {	   // add >3 to pass compilation 2/26/2018. To verify later
		double *sampledpoints = new double[3*MAX_NUMBER_BEZIER_POINTS];
		farin::sample_as_cbspline_interp_3D(np,coords,MAX_NUMBER_BEZIER_POINTS,sampledpoints);
		np = MAX_NUMBER_BEZIER_POINTS;
		param = farin::create_centripetal_knotsequence_3D(MAX_NUMBER_BEZIER_POINTS,sampledpoints);
		ret = _pBezier->Loft(pt_dim, MAX_NUMBER_BEZIER_POINTS, 3, sampledpoints, 1, param);	
		delete [] sampledpoints;
	
	} else {
		param = farin::create_centripetal_knotsequence_3D(numPoints(),coords);
		ret = _pBezier->Loft(pt_dim, numPoints(), 3, coords, 1, param);
	}

	farin::free_knot_vector(param);
	delete [] coords;
	return;
}