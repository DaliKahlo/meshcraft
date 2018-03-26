//$c1   XRL 03/11/2014 Created 
#ifndef MYHUD_H
#define MYHUD_H

#include "globals.h"
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osg/LineWidth>
#include <osg/Projection>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osg/Math>


class MyHud {
private:
	osg::ref_ptr<osgText::Text> id;
	osg::ref_ptr<osgText::Text> name;
	osg::ref_ptr<osgText::Text> location;
	osg::ref_ptr<osgText::Text> pathway;
	osg::Node* hud;
public:
	MyHud () ;
	
	osg::Node* getHud ();
	void setName ( std::string namestr );
	void setLocation ( std::string locstr );
	void setId ( std::string locstr );
	void setPathway ( std::string pathwaystr );
	osg::Node* createHud ();
	void addNode ( osg::Drawable* drawable ) ;
	osg::ref_ptr<osgText::Text> createTextNode (std::string, osg::Vec3 position);
	void createLegendEntry ( osg::Vec4 color, osg::Vec3 position, std::string text );

};

#endif /* MYHUD_H */