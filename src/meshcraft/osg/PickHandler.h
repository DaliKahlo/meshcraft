//$c3 XRL 03/25/14 Support annotations.
//$c2 XRL 02/22/14 Add rectangularPick().
//$c1 XRL 11/26/12 Created.
//////////////////////////////////////////////////////////
///
///

#ifndef NODECALLBACKS_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_
#define NODECALLBACKS_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_

//#include <osgGA/GUIEventHandler>

#include "osg/MFC_OSG.h"
#include "osg/PointIntersector"

class PickHandler : public osgGA::GUIEventHandler
{
public: 
	PickHandler(cOSG*);
	~PickHandler() {}

	bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	void pick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer);

	void rectangularPick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer);
	void drawRectangularPick(const osgGA::GUIEventAdapter& ea);
	void setPickMode(int mode) {_mode = mode;}
	void setAnnotateMode(int mode) {_annotate = mode;}
	int  getAnnotateMode() { return _annotate; }

protected:   
	float _mx,_my;
	float _mx_normalized, _my_normalized;
	//bool _usePolytopeIntersector;
	//bool _useWindowCoordinates;

	double _last[3];	// last location picked
	double _data[4];
	
	int _mode;		// one point/vertex/edge/face (a group of ... with ^ctrl)
	int _annotate;  // 0 not; 1 to pick 1st entity; 2 to click a screen point

	cOSG* _mOSG;

	void showPOINT( int, PointIntersector::Intersection& intersection );
	void showVERTEX( osgUtil::LineSegmentIntersector::Intersection& intersection );
	void showEDGE( osgUtil::LineSegmentIntersector::Intersection& intersection );
	void showFACE( osgUtil::LineSegmentIntersector::Intersection& intersection );
	int savePick( osgUtil::LineSegmentIntersector::Intersection& intersection );
	void drawAnnotatationLine(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer);
	void annotate(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer);

	bool lookupDatabase(osg::Group*, osg::Node*, osg::Drawable*, int primitiveIndex, int ids[6], double fxyz[4][3]);
	void sceneNameToAssmID(osg::Group*, osg::Node*, osg::Drawable*, int *part, int *occ, int *fcID);
	bool lookupColorMap(int primitiveIndex, double xyz[4][3], double *value);
	bool lookupLinePlot(int primitiveIndex, double xyz[4][3], double *value);
};

#endif