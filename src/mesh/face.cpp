//$c1   XRL 10/11/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                      //
//========================================================================//
#include "face.h"

Face::Face(int n, pVertex *p, pGEntity g)
: Entity(g), nV(n)
{
  for(int i=0;i<nV;i++)
	v[i] = p[i]; 
}