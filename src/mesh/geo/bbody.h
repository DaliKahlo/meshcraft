//$c1   XRL 07/02/2015 Created.
//========================================================================//
//========================================================================//
#ifndef H_MTB_BREPBODY__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_BREPBODY__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "bsurface.h"

class Brep: public EDItemBase
{
public:
  Brep(int ns, pSurface *surfs);
  ~Brep() {}

private:

  std::vector<pSurface> _surfaces;
};


#endif
