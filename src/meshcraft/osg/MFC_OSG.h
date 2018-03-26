//$c7   XRL 03/11/2014 add member data post, hidden, support colormap/lineplot/showmodels/hidenmodels
//$c6   XRL 12/05/2013 add bsRadius for point picking
//$c5   XRL 10/09/2013 replace model with a vector of models
//$c4   XRL 08/20/2012 add HUD camera
//$c3   XRL 07/09/2012 clean up, added SetCurrentOperation, SetLookAt and OrbitManipulator2.h
//$c2   XRL 05/25/2012 integrated into meshworks
//$c1   XRL 01/08/2012 Created 
//========================================================================//
//
// MFC_OSG.h
//
//=========================================================================

#ifndef MFCOSG_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define MFCOSG_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#pragma once

#include <osgViewer/Viewer>
#include <osgText/Text>
#include <osg/ClipPlane>
//#include "OrbitManipulator2.h"
//#include <osgGA/TrackballManipulator>
#include "osg/TrackballManipulator285.h"
#include "meshing/AssemblyMeshMgr.h"
#include "osg/scenegraphfactory.h"
#include "osg/triplet.h"
#include "Post.h"

typedef class PickHandler	* pPickHander;

//#define OSGTEXTSIZE	15.0f
enum SCENETYPE 
{ 
	__NO_SCENE, 
	__MODEL,
	__MESH,
	__COLORMAP,
	__LINEPLOT
};

class cOSG
{
public:
    cOSG(HWND hWnd);
    ~cOSG();

    void InitCameraViewerAndScene(void);
	void SetLookAt(int);
	void SetPickMode(int);
	void SetAnnotateMode(int);
	int GetAnnotateMode();
	void SetRenderAttribute(ViewStyle);
	void SetClipPlane(int dir, int pos);
	int  GetClipPlane(osg::Vec4&);
	void ChangePointSize(double);
	void ChangeTransparency(double);
	void ChangeBackground(int, int);
	void ChangeInteriorLightStatus(int);
	bool AddAssemblyModel(AssemblyMeshMgr*, int, bool);
	AssemblyMeshMgr* GetAsmMeshMgr();
	bool SetGeodeNode(osg::ref_ptr<osg::Geode>);
    void PreFrameUpdate(void);
    void PostFrameUpdate(void);
	void SetCurrentOperation(int);
    void Render(void* ptr);

	// user interactions
	//
	void GetScreenAndTextSize(int*, int*, float*);
	void addHudText(const osg::Vec3& pos, const std::string& content);
	void clearHudText();
	void drawHUDRect(float, float, float, float);
	void drawHUDLine(float x1, float y1, float x2, float y2);
	void clearHudDrawing();


	//
	double GetBSphereRadius();
	void clearHighlight();
	void highlight(int, double[4][3]);
	void highlightP(double xyz[3], double radius, int);
	void highlightCircle(double center[3], double axis[3], double radius);
	void highlightPolyLine(int num, double xyz[64][3], int, osg::Vec4&);
	void addText(const osg::Vec3& pos, const std::string& content, osgText::TextBase::AlignmentType);

	void postMult(osg::Matrix&, int);
	void lineplot(int,double*,double*,double,double);
	void colormap(int nv, int nf, double *xyz, int *tria, double *values, double minV, double maxV);

	//
	int showcolormap();
	int showlineplot();
	void showmodels();
	void hidemodels();
	SCENETYPE currentscene();

	// loop up database
	bool findMeshEntityIDs(int instance_id, int face_id, int primitiveIndex, int ids[6], double fxyz[4][3]);
	int crossEdgeAngle(int, int id0, int id1, double *angle);
	bool loopupPostData(ATTRTYPE, int primitiveIndex, double xyz[4][3], double *val);

    osgViewer::Viewer* getViewer() { return mViewer; }
	osg::ref_ptr<osg::Group> getSceneModel(int i) { return (models[i]).get(); }


private:
	HWND m_hWnd;						// can acquire from CWND object with GetSafeHwnd()

	// scene graph viewer
	//double eye[3], center[3];			// view matrix
	int m_winWidth, m_winHeight;
    osgViewer::Viewer* mViewer;
	pPickHander m_pPickHander;
	osg::ref_ptr<TrackballManipulator2> trackball;	//osg::ref_ptr<OrbitManipulator2> trackball;

	// scene graph
	//
	osg::ref_ptr<osg::Group> world;		// root
	// 
	osg::ref_ptr<osg::Camera> hud;		// 2D HUD 
	osg::ref_ptr<osg::Geode> textGeode;	// text 2D
	osg::ref_ptr<osg::Geode> gfx2Dgeode;// 2D rectangular picking node
	//
	std::vector< osg::ref_ptr<osg::Group> > models;
	bool hidden;	
	cPOST post;
	osg::ref_ptr<osg::Group> decorator;
	osg::ref_ptr<osg::ClipPlane> cp;	// clip plane
	bool cpON;							// indicate if the clip plane is on/off. 
	double bsRadius;					// radius of bounding sphere of world

	// model generator
	CSGFactory *pFactory;

	// entrance to mesh database
	AssemblyMeshMgr *pASMEMgr;

	void initSceneGraph();
	void toggleScribe(ViewStyle);
	void RemoveAssemblyModel(int);

	osgText::Text* createText( const osg::Vec3&, const std::string&, float, int, const osg::Vec4&);
	osg::ref_ptr<osg::Group> createTriad(double[3], double);
	osg::ref_ptr<osg::Geode> createCircleOnXYPlane( double radius, int approx );

};

#endif