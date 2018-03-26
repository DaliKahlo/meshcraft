//$c1 XRL 11/02/13 Created.
//========================================================================
//
//========================================================================
#include "stdafx.h"
#include "SelectionMgr.h"
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

CSelectionMgr:: CSelectionMgr()
{
	clear();
}

void CSelectionMgr::clear()
{
	pointContainer.clear();
}

void CSelectionMgr::appendPoint(double xyz[3],int o, int f, int i)
{
	pointContainer.push_back( MW_Point(xyz,o,f,i));
}
	
int CSelectionMgr::numPoints()
{
	return pointContainer.size();
}

MW_Point CSelectionMgr::ithPoint(int i)
{
	return pointContainer[i];
}


void CSelectionMgr::showPoints(cOSG* pOSG, double r)
{
	std::stringstream oss;
	double c[3];
	int w,h,i=0;
	float x,y,dh,ts;
	pOSG->GetScreenAndTextSize(&w, &h, &ts);
	pOSG->clearHudText();
	pOSG->clearHighlight();
	x = 0.004*w;
	dh = ts + 3.0f;

	std::vector<MW_Point>::iterator itrP;
	for(itrP = pointContainer.begin(); itrP != pointContainer.end(); itrP++) {
		c[0] = itrP->x(); c[1] = itrP->y(); c[2] = itrP->z(); 
		if(i<72) {
			y = h-dh;
			oss << std::setprecision(7) << "(" << c[0] << "  " << c[1] << "  " << c[2] << ")\n";
			pOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());
		}
		pOSG->highlightP(c,r,0);
		i++;
	}
	return;
}