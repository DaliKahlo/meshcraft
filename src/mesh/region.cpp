//$c1   XRL 07/20/2012 Created.
//========================================================================//
//              Copyright 2012 (Unpublished Material)                     //
//                  MSC Software Corp.                                      //
//========================================================================//
#include "region.h"

Region::Region(pVertex *p, pGEntity g)
: Entity(g)
{
  for(int i=0;i<4;i++)
	v[i] = p[i]; 
}