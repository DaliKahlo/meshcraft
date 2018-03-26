//$c2   XRL 04/11/2013 Added temp_id.
//$c1   XRL 10/07/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//									                                      //
//========================================================================//
#ifndef H_MTB_VERTEX__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_VERTEX__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "entity.h"

class Vertex : public Entity
{
public:
  Vertex(double[3], pGEntity classifiedOn);
  ~Vertex() {}
  
  virtual eType type() const { return Tvertex; }

  void get_coordinates(double *);
  void set_coordinates(double x, double y, double z);
  void get_param(double[2]);
  void set_param(double[2]);

  void reset_classification_type(int);
  int get_classification_type();

  int get_temp_id() { return temp_id; }
  void set_temp_id(int id) { temp_id = id; }

private:

  double xyz[3];
  double params[2];

  int temp_id;
  unsigned char _mark;  // only bit 0 abd 1 used. Six bits (3-7) left
};


#endif
