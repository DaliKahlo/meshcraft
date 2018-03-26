//$c1 XRL 09/13/12 Created.
//////////////////////////////////////////////////////////
///
///

#ifndef PICK_HANDLER_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_
#define PICK_HANDLER_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_

//#include <osgGA/GUIEventHandler>

#include "osg/MFC_OSG.h"

class TriadCallback : public osg::NodeCallback
{
public:
	// this is a callback set on a HUD cemera node
	// takes the orientation of the main camera and rotates to follow it
	TriadCallback(osg::ref_ptr<osg::Camera> c) 
	{ masterCamera = c;}

	~TriadCallback() {}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osg::Vec3 eye, center, up;
		masterCamera->getViewMatrixAsLookAt(eye, center, up);

		osg::ref_ptr<osg::Camera> hudCamera = static_cast<osg::Camera*> (node) ;
		hudCamera->setViewMatrixAsLookAt( (eye-center), osg::Vec3(0.0f,0.0f,0.0f), up);
		traverse(node,nv);
	}
private:
	osg::ref_ptr<osg::Camera> masterCamera;
};
#endif