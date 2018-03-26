//$c1   XRL 05/28/2015 Created.
//========================================================================//
//========================================================================//
#ifndef H_MTB_CURVE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_CURVE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include <vector>
#include "../typedef.h"
#include "../Vec3d.h"
#include "../EDList.h"
#include "opennurbs.h"

#define MAX_NUMBER_BEZIER_POINTS 15

class Curve: public EDItemBase
{
public:
  Curve(geomType type, char *name, int np, double *pts);
  ~Curve();

  size_t numPoints();
  void ithPoint(int, double *x, double *y, double *z);

  std::string & name();

  void computeBezier();

private:

  geomType _type;
  std::vector<Vec3d> _pts;  // if _type=Bezier, control points; otherwise, interpolation points 
  std::string _name;

  ON_BezierCurve *_pBezier;
};


#endif
