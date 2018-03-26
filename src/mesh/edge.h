//$c1   XRL 10/7/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                      //
//========================================================================//
#ifndef H_MTB_EDGE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_EDGE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "entity.h"

class Edge : public Entity 
{
public:
  Edge(pVertex, pVertex, pGEntity);
  ~Edge() {}
  
  virtual eType type() const { return Tedge; }

  pVertex get_vertex(int i) { return v[i]; }

private:

  pVertex v[2];
};


#endif
