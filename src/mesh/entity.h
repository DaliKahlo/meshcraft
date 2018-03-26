//$c2   XRL 11/8/2012 Added ID to support graphical picking.
//$c1   XRL 10/7/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//									                                      //
//========================================================================//
#ifndef H_MTB_ENTITY__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_ENTITY__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "typedef.h"
#include "EDList.h"

class Entity: public EDItemBase
{
  friend class PSMesh;
public:

  virtual ~Entity() {}
  virtual eType type() const = 0;

  pGEntity whatIn() {return WhatIn;}
  void setWhatIn(pGEntity what) { WhatIn=what; }
  
  void set_id(int i) { id = i; }
  int get_id() { return id; }

protected:
  Entity(pGEntity classifiedOn) { WhatIn=classifiedOn; }

private:

  pGEntity WhatIn;
  int id;
};


#endif
