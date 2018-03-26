//$c4   XRL 06/06/2013 Added interior lighting control 
//$c3   XRL 11/27/2012 Added clip plane control
//$c2   XRL 07/30/2012 Added transparency and point size control 
//$c1   XRL 02/02/2012 Created 
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#ifndef MESHWORK_SYSTEM_OPTIONS_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHWORK_SYSTEM_OPTIONS_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#pragma once

#include "MeshWorkDefs.h"

class CSystemOption  
{
public:
	CSystemOption();
	~CSystemOption() {}

	SceneStyle getSceneStyle() { return m_style; }
	void setSceneStyle(SceneStyle s) { m_style=s; }

	void getClipPlane(int* idx, int *pos) { *idx=m_clipPlaneIdx; *pos=m_clipPlanePos; }
	void setClipPlane(int i, int d) { m_clipPlaneIdx=i; m_clipPlanePos=d; }

	double getScenePointSize() { return m_pointsize; }
	void setScenePointSize(double s) { m_pointsize = s; }

	double getSceneTransparency() { return m_transparency; } 
	void setSceneTransparency(double t) { m_transparency = t; } 

	int getInteriorLightStatus() { return m_lightInterior; }
	void setInteriorLightStatus(int i) { m_lightInterior=i; }

	int getIntersectCorrect() { return m_intersectCorrect; }
	void setIntersectCorrect(int i) { m_intersectCorrect=i; }

	int getDefaultSizeCompute() { return m_defaultSizeCompute; }
	void setDefaultSizeCompute(int c) { m_defaultSizeCompute=c; }

	int getBKColor() { return m_bkcolor; } 
	void setBKColor(int c) { m_bkcolor = c;}

private:
	SceneStyle m_style;
	int m_bkcolor;				// 0 dark blue, 1 gradient, 2 white
	int m_clipPlaneIdx;
	int m_clipPlanePos;			// in range [0, 100]
	double m_pointsize;
	double m_transparency;
	int m_lightInterior;		// 0 off; 1 on

	int m_intersectCorrect;
	int m_defaultSizeCompute;
};
#endif