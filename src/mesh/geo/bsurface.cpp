//$c1   XRL 07/02/2015 Created.
//========================================================================//
//========================================================================//
#include "bsurface.h"

Surface::Surface(int nc, pCurve curves[4])
{
	for(int i=0; i<nc; i++)
		_curves.push_back(curves[i]);
}

