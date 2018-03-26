//$c1   XRL 10/7/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//									                                      //
//========================================================================//
#ifndef H_MTB_FACE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_FACE__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "entity.h"

class Face : public Entity
{
public:
  Face(int, pVertex *, pGEntity);
  ~Face() {}
  
  virtual eType type() const { return Tface; }

  int numVertices() { return nV; }
  pVertex get_vertex(int i) { return v[i]; }

private:

  int	nV; 
  pVertex v[4];
};



#endif
