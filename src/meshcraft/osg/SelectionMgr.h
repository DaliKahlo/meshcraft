//$c2 XRL 02/23/14 Add occurance, face_id and primitibe index.
//$c1 XRL 11/02/13 Created.
//////////////////////////////////////////////////////////
///
///

#ifndef MW_SELECTIONMGR_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_
#define MW_SELECTIONMGR_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_

#include <vector>
#include "osg/MFC_OSG.h"

class MW_Point {
public:
  MW_Point(double xyz[3],int o,int f,int i):
	  _x(xyz[0]), _y(xyz[1]),_z(xyz[2]), occurance(o), face_id(f), primitiveIndex(i) {}
  MW_Point(double x, double y, double z):
	  _x(x), _y(y),_z(z) {}
  ~MW_Point() {}

  double x() { return _x;}
  double y() { return _y;}
  double z() { return _z;}
  void getID(int* o,int* f,int* i) {*o=occurance; *f=face_id; *i=primitiveIndex;}

private:
	double _x,_y,_z;
	int occurance, face_id, primitiveIndex;
};

class CSelectionMgr 
{
public: 
	CSelectionMgr();
	~CSelectionMgr() {}
	void clear();

	////
	void appendPoint(double[3],int,int,int);
	int numPoints();
	MW_Point ithPoint(int);

	////
	void showPoints(cOSG*, double);

protected:   
	std::vector<MW_Point> pointContainer;
};

#endif