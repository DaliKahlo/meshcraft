//$c1   XRL 10/11/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
#include "edge.h"

Edge::Edge(pVertex v0, pVertex v1, pGEntity g)
: Entity(g)
{
	v[0] = v0; 
	v[1] = v1;
}