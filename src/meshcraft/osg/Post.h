//$c1   XRL 03/11/2014 Created 
//========================================================================//
//
// Meshworks post processing
//
//=========================================================================

#ifndef MWPOST_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define MWPOST_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#pragma once

#include <osg/Switch>
#include <osgText/Text>

class cPOST
{
public:
    cPOST();
    ~cPOST();

	// for all scenes
	void setValueRangle(double, double);
	void attachParent(osg::Group *, osg::Camera *, float, float);
	void detachParent(osg::Group *, osg::Camera *);
	void clear(osg::Group *, osg::Camera *);

	// for indivisual scene
	void draw_colormap(int nv, int nf, double *xyz, int *tria, double *values);
	void draw_lineplot(int np, double *xyz, double *proj);
	int showcolormap();
	int showlineplot();

private:

	////// data
	double minVal, maxVal;
	bool attached;

	// scene graph nodes
	osg::ref_ptr<osg::Switch> m_postsw;	
	osg::ref_ptr<osg::Geode> m_colorlegend;		// 2D color legend node

	////// methods
	void computeValueColor(double, osg::Vec4 &);
	void drawHUDColorLegend(osg::Camera*, float, float);
	void clearHUDColorLegend(osg::Camera*);
	osgText::Text* createText( const osg::Vec3&, const std::string&, float, int, const osg::Vec4&);
};

#endif