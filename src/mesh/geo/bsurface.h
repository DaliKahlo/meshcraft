//$c1   XRL 07/02/2015 Created.
//========================================================================//
//========================================================================//

#ifndef H_MTB_SURFACE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_SURFACE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "curve.h"

class Surface: public EDItemBase
{
public:
  Surface(int,pCurve[4]);
  ~Surface() {}

  int numCurves() { return _curves.size(); }
  pCurve ithCurve(int i) { return _curves[i]; }

  //computeControlNet();

private:
  std::vector<pCurve> _curves;
};


#endif
