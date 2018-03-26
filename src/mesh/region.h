//$c1   XRL 07/20/2012 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                      //
//========================================================================//
#ifndef H_REGION__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_REGION__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "entity.h"

class Region : public Entity
{
public:
  Region(pVertex *, pGEntity);
  ~Region() {}
  
  virtual eType type() const { return Tregion; }

  int numVertices() { return 4; }
  pVertex get_vertex(int i) { return v[i]; }

private:
  pVertex v[4];

};



#endif
