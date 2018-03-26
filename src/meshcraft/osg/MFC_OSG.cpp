//$c24  XRL 03/11/2014 support colormap, showmodels, hidemodels, showcolormap, showlineplot
//$c23  XRL 02/22/2014 add front/top/left/perspective view, removed member data *camera
//$c22  XRL 02/11/2014 add lineplot, SetPickMode()
//$c21  XRL 10/09/2013 replace model with a vector of models, GUI controls only the current model
//$c20  XRL 07/01/2013 move lightsource into initSceneGraph; optimize model (not world) in AddAssemblyModel 
//$c19  XRL 06/09/2013 improve clipplane user experience and couple it with picking
//$c18  XRL 06/06/2013 improve the transparency experiences; add ChangeBackground() and hook with options...
//$c17  XRL 06/06/2013 turn off lightening on HUD texts and add optimize root sg. 
//$c16  XRL 06/04/2013 support opposing light source of LIGHT0 to show interior
//$c15  XRL 03/18/2013 shrink high-lighted elements
//$c14  XRL 03/18/2013 support black/white mode for patent images
//$c13  XRL 11/27/2012 added clip plane 0
//$c12  XRL 11/21/2012 added createTriad and TriadCallback
//$c11  XRL 11/08/2012 added pASME/findMeshEntityIDs. show quad in highlight()
//$c10  XRL 09/21/2012 added toggleScribe to display mesh edges (need double rendering)
//$c9   XRL 09/21/2012 added highlight, re-organized scene graph (world,model etc.)
//$c8   XRL 09/20/2012 added HudCamera, support hud text
//$c7   XRL 09/18/2012 redirected NotifyHandler
//$c6   XRL 09/17/2012 added PickHander
//$c5   XRL 07/24/2012 added displayMeshEdges and hideMeshEdges
//$c4   XRL 07/10/2012 added SetRenderAttribute.
//$c3   XRL 07/08/2012 changed InitCameraConfig() and SetupSceneData()
//$c2   XRL 05/25/2012 integrated into meshworks
//$c1   XRL 01/08/2012 Created 
//=============================================================================
// MFC_OSG.cpp : implementation of the cOSG class
//=============================================================================
#include "stdafx.h"
#include "MFC_OSG.h"
#include "osg/PickHandler.h"
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osg/PolygonMode>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/Point>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osgText/Font>
#include <osgText/Text>
#include <osgFX/Scribe>
#include <osg/MatrixTransform>
#include <osgUtil/Optimizer>
#include <osg/Depth>
//#include <osg/LightModel>
#include "FindNodeVistor.h"
#include "Meshworks.h"
#include "util.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TOLERANCE2	1.0e-28
#define SQRTONETHIRD 0.577350269

osg::ref_ptr<osgText::Font> g_font; //

///
///  LogFileHandler object
///
class LogFileHandler : public osg::NotifyHandler
{
public:
	LogFileHandler( const std::string& file )
	{ _log.open(file.c_str() ); }
	virtual ~LogFileHandler() { _log.close(); }

	virtual void notify(osg::NotifySeverity severity, const char* msg)
	{ _log << msg; }

protected:
	std::ofstream _log;
};

class TriadCallback : public osg::NodeCallback
{
public:
	TriadCallback( osg::Camera* triadcam, osg::Camera* mastercam )
	: _triadcam(triadcam), _mastercam(mastercam) {}
	~TriadCallback() {}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		if ( _triadcam.valid() && _mastercam.valid() && cv ) {
			osg::Vec3 eye, center, up;
			_mastercam->getViewMatrixAsLookAt(eye, center, up);
			//osg::notify(osg::NOTICE) << "eye: (" << eye.x() << " " << eye.y() << " " << eye.z() <<")" << std::endl;
			//osg::notify(osg::NOTICE) << "center: (" << center.x() << " " << center.y() << " " << center.z() <<")" << std::endl;
			_triadcam->setViewMatrixAsLookAt( (eye-center), osg::Vec3(0.0f,0.0f,0.0f), up);

		}
		traverse(node,nv);
	}

protected:
	osg::observer_ptr<osg::Camera> _mastercam;
	osg::observer_ptr<osg::Camera> _triadcam;
};

///
///  cOSG object
///

cOSG::cOSG(HWND hWnd) :
   m_hWnd(hWnd), mViewer(0), trackball(0), world(0)
{
	osg::setNotifyLevel( osg::INFO );
	osg::setNotifyHandler( new LogFileHandler("osglog.txt") );

	m_winWidth = 0;
	m_winHeight = 0;
	bsRadius =  0.0;

	//eye[0] = 0.0; eye[1] = -100.0; eye[2] = 0.0;
	//center[0] = 0.0; center[1] = 0.0; center[2] = 0.0;

	//transparent = false;		// not
	//edgeDisplayed = false;
	hidden = false;				// models is removed from world
	cp = NULL;
	cpON = false;

	pFactory = new CSGFactory;
	m_pPickHander = NULL;
	pASMEMgr = NULL;
}

cOSG::~cOSG()
{
	//delete m_pPickHander;
	if(world.valid())
		post.detachParent(world, hud);
    delete mViewer;
	delete pFactory;	
	world = NULL;
	models.clear();
	trackball = NULL;
}

void cOSG::InitCameraViewerAndScene(void)
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	int bkcolor = mwApp->sysOption()->getBKColor();

	///
	/// define GraphicsContext Traits
	///
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

    // Get the current window size
    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
	m_winWidth = rect.right - rect.left;
	m_winHeight = rect.bottom - rect.top;

    // create the Windata Variable that holds the handle for the Window to display OSG in.
    osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hWnd);

    // Setup the traits parameters
    traits->x = 0;
    traits->y = 0;
    traits->width = m_winWidth;
    traits->height = m_winHeight;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->setInheritedWindowPixelFormat = false;//true;
    traits->inheritedWindowData = windata;			

	///
    /// Create the Graphics Context
	///
	osg::ref_ptr<osg::GraphicsContext> gc;
    gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (gc->valid())
	{
        TRACE("OSG graphicsWindow has been created successfully.");
        // need to ensure that the window is cleared make sure that the complete window is set the correct colour
        // rather than just the parts of the window that are under the camera's viewports
		gc->setClearColor(osg::Vec4f(BKCOLOR_R,BKCOLOR_G,BKCOLOR_B,1.0f));
        gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     }
     else
     {
        TRACE("OSG graphicsWindow has not been created successfully.");
     }

	///
	/// Setup camera, which:
	///		(1) camera handle view matrx, projection matrix and viewports
	///		(2) manage graphics context
	///		(3) clear frame buffers, preset redraw values
	///		(4) manage render to texture
	///

	//// Init a new Camera (Master for this View)
	//camera = new osg::Camera;
	//// Assign Graphics Context to the Camera
	//camera->setGraphicsContext(gc);
	//// Set the viewport for the Camera
	//camera->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));
	//// frame buffer and clear color
	//camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//camera->setClearColor(osg::Vec4f(BKCOLOR_R,BKCOLOR_G,BKCOLOR_B,1.0f));
	//// set projection matrix
	//camera->setProjectionMatrixAsPerspective(30.0f, (double)traits->width/(double)traits->height, 1.0, 1000.0);

	///
	/// setup viewer (viewer has a view and a camera unless compositeViewer)
	///
    mViewer = new osgViewer::Viewer();
	   
	// osgViewer::ViewerBase::ThreadPerContext
	// osgViewer::ViewerBase::ThreadPerCamera
	mViewer->setThreadingModel( osgViewer::ViewerBase::SingleThreaded );

    // Camera
	osg::ref_ptr<osg::Camera> camera = mViewer->getCamera();
   // mViewer->setCamera(camera.get());
    camera->setGraphicsContext(gc);
    // Set the viewport for the Camera
    camera->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));
	// frame buffer and clear color
	camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	camera->setClearColor(osg::Vec4f(BKCOLOR_R,BKCOLOR_G,BKCOLOR_B,1.0f));
	// set projection matrix
	camera->setProjectionMatrixAsPerspective(30.0f, (double)traits->width/(double)traits->height, 1.0, 1000.0);

    // Camera Manipulator -- event handler 1
	//trackball = new OrbitManipulator2;
	trackball = new TrackballManipulator2(this);
	mViewer->setCameraManipulator( trackball.get() );

    // event handler 2
    //mViewer->addEventHandler(new osgViewer::StatsHandler);
	m_pPickHander = new PickHandler(this);
	mViewer->addEventHandler( m_pPickHander );

	// added 10/31/2013
	osg::CullSettings::CullingMode mode = mViewer->getCamera()->getCullingMode();
    mViewer->getCamera()->setCullingMode( mode & (~osg::CullSettings::SMALL_FEATURE_CULLING) );

	mViewer->realize();

	///
	/// setup inital scene
	///
	initSceneGraph();
}

//
//   world ---- hud ----  textgeode
//          |    +------  gfx2Dgeode
//          |    +------  lgd2Dgeode
//          +-- hud2 ---- triad
//          +-- lightSource
//          +-- decorator
//          +-- post switch node (lineplot and colormap)
//          +-- model(s)
void cOSG::initSceneGraph()
{
	// root node
	world = new osg::Group;
#ifdef BLACK_WHITE_MODE
	world->getOrCreateStateSet()->setMode( GL_LIGHT0, osg::StateAttribute::OFF );
#endif

	// HUD for 2D text and graphics
	hud = new osg::Camera;
	hud->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	hud->setViewMatrix( osg::Matrix::identity() );
	hud->setClearMask( GL_DEPTH_BUFFER_BIT );
	hud->setRenderOrder(osg::Camera::POST_RENDER );
	hud->setAllowEventFocus( false );
	hud->setNodeMask( 0x1 );		// make it untouchable to intersection visitor
	hud->setProjectionMatrix( osg::Matrix::ortho2D(0, m_winWidth, 0, m_winHeight) ); // left right bottom top
	//hud->setViewport(0,0,m_winWidth,m_winHeight);
	hud->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF ); 

	world->addChild(hud);
	g_font = osgText::readFontFile("fonts/arial.ttf");

	// backgroud node
	// initial value is 0 DARK_BLUE, set in setClearColor().

	// triad  - HUD camera to define a mini viewport and attach the triad
	double ori[3] = {0., 0., 0.};
	double ar = (double)m_winWidth/(double)m_winHeight;
	osg::ref_ptr<osg::Camera> hud2 = new osg::Camera;
	hud2->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	hud2->setClearMask( GL_DEPTH_BUFFER_BIT );
	hud2->setAllowEventFocus( false );
	hud2->setRenderOrder(osg::Camera::POST_RENDER );
	hud2->setViewMatrixAsLookAt(osg::Vec3(0.0f,-5.0f,0.0f), osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f,0.0f,1.0f)); 
	hud2->getProjectionMatrix() *= osg::Matrix::scale(1.0/ar,1.0,1.0);
	hud2->setViewport(0,0,0.1*m_winWidth,0.1*m_winHeight);
	hud2->addChild( createTriad(ori,1.0).get() );
	world->addChild( hud2.get() );
	world->addCullCallback( new TriadCallback(hud2.get(), mViewer->getCamera()) );

	// highlight decorator

	// light source(s)
#ifndef BLACK_WHITE_MODE
	//double ori[3];
	//ori[0] = c.x(); ori[1] = c.y(); ori[2] = c.z();
	//triad = createTriad(ori, 0.15*r ).get();
	//world->addChild( triad );

	// osg default light setting:
	// diffuse:  0.8 0.8 0.8 1.0    color
	// ambient:	 0.0 0.0 0.0 1.0
	// specular: 1.0 1.0 1.0 1.0
	// position: 0.0 0.0 1.0 0.0	directional

	// openGl setting:
	// position: 1.0 -1.0 1.0 0.0

	//
	osg::ref_ptr<osg::Light> light0 = mViewer->getLight();
	if(0) {
		//osg::View::LightingMode lm = mViewer->getLightingMode();
		//osg::Vec4 df = light0->getDiffuse();    // the smaller, the dull
		//osg::Vec4 ab = light0->getAmbient();
		//osg::Vec4 po = light0->getPosition();
		//osg::Vec4 sp = light0->getSpecular();	  // bigger or smaller shining area
		light0->setPosition( osg::Vec4(1.0f, -1.0f, 1.0f, 0.0f) );   // this is what is used for years
	}
	light0->setDiffuse( osg::Vec4(0.6f,0.6f,0.6f,1.0f) );

	// set interior light and light source
	osg::ref_ptr<osg::Light> oplight = new osg::Light;
	oplight->setLightNum( 1 );
	oplight->setDiffuse( osg::Vec4(0.8f,0.8f,0.0f,1.0f) );
	oplight->setPosition( osg::Vec4(0.0f, 0.0f, -1.0f, 0.0f) );
	osg::ref_ptr<osg::LightSource> lightSource1 = new osg::LightSource;
	lightSource1->setLight( oplight );
	lightSource1->setReferenceFrame(osg::LightSource::ABSOLUTE_RF);

	osg::StateSet* st = world->getOrCreateStateSet(); 
	st->setMode( GL_LIGHTING, osg::StateAttribute::ON); 
	st->setMode( GL_COLOR_MATERIAL, osg::StateAttribute::ON); 
	st->setMode( GL_LIGHT1, osg::StateAttribute::OFF );
	world->addChild( lightSource1.get() );
#endif

}

void cOSG::showmodels()
{
	if(hidden) {
		std::vector< osg::ref_ptr<osg::Group> >::iterator it;
		for(it=models.begin(); it!=models.end(); it++) {
			if(*it != NULL)
				world->addChild(*it);
		}
		hidden=false;
		SetAnnotateMode(0);  // switch to entity picking mode

		// hide post scene nodes
		post.detachParent(world, hud);
	}
}

void cOSG::hidemodels()
{
	std::vector< osg::ref_ptr<osg::Group> >::iterator it;
	for(it=models.begin(); it!=models.end(); it++) {
		if(*it != NULL && (*it)->getNumParents() > 0) {
			// see if scribe
			osg::ref_ptr<osg::Group> parent = (*it)->getParent(0);
			osg::ref_ptr<osgFX::Scribe> scribed = dynamic_cast<osgFX::Scribe*>( parent.get() );
			if (scribed.valid() ) {
				// remove the scribe
				scribed->removeChild(*it);
				osg::Node::ParentList parentList = scribed->getParents();
				for(osg::Node::ParentList::iterator itr=parentList.begin(); itr!=parentList.end(); ++itr)
				{
					(*itr)->removeChild(scribed);
				}
				scribed = NULL;
			} else {
				// not scribe
				world->removeChild(*it);
			}
			hidden=true;
			SetAnnotateMode(1);  // switch to post annotation mode
		}
	}
}

void cOSG::RemoveAssemblyModel(int i)
{
	if (!(models[i]))
		return;
	if (!(models[i].valid()))
		return;

	osg::ref_ptr<osg::Group> parent = (models[i])->getParent(0);
	osg::ref_ptr<osgFX::Scribe> scribed = dynamic_cast<osgFX::Scribe*>( parent.get() );
	if (scribed.valid() ) {
		scribed->removeChild(models[i]);
		osg::Node::ParentList parentList = scribed->getParents();
		for(osg::Node::ParentList::iterator itr=parentList.begin(); itr!=parentList.end(); ++itr)
		{
			(*itr)->removeChild(scribed);
		}
		scribed = NULL;
		models[i] = NULL;
	} else {
		world->removeChild(models[i]);
		int refCount = (models[i])->referenceCount();
		models[i] = NULL;
	}
	return;
}

bool cOSG::SetGeodeNode(osg::ref_ptr<osg::Geode> gn)
{
	if( !mViewer || !world.valid() ) 
		return false;		

	pFactory->clear();
	for(unsigned int i=0; i<models.size(); i++) 
		RemoveAssemblyModel(i);
	//this->RemoveAssemblyModel();
	if(pASMEMgr == NULL)
		return false;	

	this->clearHudText();
	this->clearHighlight();

	//models[0] = gn;
	world->addChild(gn.get());
	return true;
}

bool cOSG::AddAssemblyModel(AssemblyMeshMgr* pAsmMeshMgr, int mode, bool setViewer)
{
	//int color_table[8][3] = { {255,255,255}, {128,0,0}, {128,128,0}, {0,128,128}, 
	//						  {128,0,128}, {0,255,0}, {0,255,255}, {255,0,255} };
	const int nColor = 12;
	int color_table[12][3] = {	{238,238,238},		// almost white
								{255,0,0},			// red
								{255,165,0},		// orange
								{0,153,204},		// blue
								{198,226,255},		// slate Gray
								{0,255,255},		// cyan
								{255,215,0},		// magenta
								{180,238,180},		// dark sea green								
								{255,192,203},		// pink
								{255,215,0},		// gold
								{128,0,128},		// purple
								{75,0,130}			// indigo
							};
	int i;

	if( !mViewer || !world.valid() ) 
		return false;		

	if(mode == 0) {
		// clean up post data
		showmodels();
		post.clear(world, hud);
		//
		pFactory->clear();
		pASMEMgr = pAsmMeshMgr;
		// - delete the previous model or scribe node if not NULL			7/6/2012
		// - it is possible that the previous model is still in world, e.g. when openning a new model.
		//   the previous model should be removed and cleaned so that the previous mesh will not be displayed	7/12/2012
		int nm = models.size();
		for(i=0; i<nm; i++) 
			RemoveAssemblyModel(i);
		//this->RemoveAllAssemblyModel();
	} 
	if(pASMEMgr == NULL)
		return false;	

	post.clear(world,hud);
	this->clearHudText();
	this->clearHighlight();

	//osg::ref_ptr<ModelController> ctrler = new ModelController( mt.get() );
	//mViewer->addEventHandler( ctrler.get() );
	//camera->setAllowEventFocus( true );

	AssemblyMesh*  pAsmMesh;
	float fcolorR, fcolorG, fcolorB;
	int count = 0;
	int nAssem = pAsmMeshMgr->size();
	models.resize(nAssem);
	for(int i=0; i<nAssem; i++) {
		pAsmMesh = pAsmMeshMgr->ith(i);
		if(!pAsmMesh) 
			continue;

		if(pAsmMesh->getSgState() == SWITCH_ON && !pAsmMesh->isSgConsist()) {
			pAsmMesh->setSgConsist(true);
			pAsmMesh->setSgState(NOT_IN_SG);
			RemoveAssemblyModel(i);
		}

		if(pAsmMesh->getSgState() == NOT_IN_SG) {
			pAsmMesh->setSgState(SWITCH_ON);
			
			fcolorR = ((float)color_table[i%nColor][0]) / 255.0;
			fcolorG = ((float)color_table[i%nColor][1]) / 255.0;
			fcolorB = ((float)color_table[i%nColor][2]) / 255.0;
			pFactory->setDefaultFaceColor(fcolorR,fcolorG,fcolorB);
			models[i] = pFactory->create(pAsmMesh);	

			osgUtil::Optimizer optimizer;
			optimizer.optimize(models[i].get());// can't optimize 'world' because it removes all group nodes without children

			//osg::ref_ptr<osg::Switch> switchNode = new osg::Switch;
			//switchNode->setName("SW_ASSEMBLY_" + std::to_string((long long)(pAsmMesh->getID())));
			//switchNode->addChild(model_i.get());
			world->addChild(models[i].get());
		}
	}

	//SetClipPlane(0,0.0);

	if(setViewer) {
		mViewer->setSceneData(NULL);
		mViewer->setSceneData(world.get());	// this automatically set the view center
		osg::BoundingSphere bs = world->getBound();
		bsRadius = bs.radius();
	}
	mViewer->realize();					    // set up windows and associated threads.

	if(count == 0)
	{
		// display something
		//world->addChild( createShapes().get() ); 
		//mViewer->setSceneData(world.get());
		//mViewer->realize();
	}
	return true;
}

AssemblyMeshMgr* cOSG::GetAsmMeshMgr()
{
	return pASMEMgr;
}

// CameraManipulator attached to the viewer overrides setViewMatrixAsLookAt  2/22/2014
// for isometric: eye (1,1,1), up (-0.5,-0.5,1) -->
void cOSG::SetLookAt(int dir)
{
	
	osg::BoundingSphere bs = world->getBound();
	osg::Vec3 center = world->getBound().center();
	bsRadius = bs.radius();

	switch(dir) {
	case 1: {			// front view
		trackball->computePosition(center - osg::Y_AXIS * (bsRadius*3.0), center, osg::Z_AXIS);
		//camera->setViewMatrixAsLookAt(center - osg::Y_AXIS * (bsRadius*3.0), center, osg::Z_AXIS);
		break;
			}
	case 2: {			// top view
		trackball->computePosition(center + osg::Z_AXIS * (bsRadius*3.0), center, osg::Y_AXIS);
		//camera->setViewMatrixAsLookAt(center + osg::Z_AXIS * (bsRadius*3.0), center, osg::Y_AXIS);
		break;
			}
	case 3: {			// left view
		trackball->computePosition(center - osg::X_AXIS * (bsRadius*3.0), center, osg::Z_AXIS);
		//camera->setViewMatrixAsLookAt(center - osg::X_AXIS * (bsRadius*3.0), center, osg::Z_AXIS);
		break;
			}
	case 4: {			// isometric view
		osg::Vec3 ISO_AXIS(-SQRTONETHIRD,-SQRTONETHIRD,-SQRTONETHIRD);
		osg::Vec3 UP_AXIS(-0.40824829,-0.40824829,0.816496581);
		trackball->computePosition(center - ISO_AXIS * (bsRadius*3.0), center, UP_AXIS);
		break;
			}
	}

	return;
}

int cOSG::GetClipPlane(osg::Vec4& __cp)
{
	if(cp == NULL || !cpON)
		return 0;

	__cp = cp->getClipPlane();
	return 1;
}

void cOSG::SetClipPlane(int dir, int pos)
{
	if(!pASMEMgr) return;
	int i = pASMEMgr->current();
	if(models[i].valid()) {
		osg::StateSet * state = models[i]->getOrCreateStateSet();

		if(dir == 0) {
			state->setAttributeAndModes( cp.get(), osg::StateAttribute::OFF);
			cpON = false;
		}
		else
		{
			if( !cp.valid() && 0<=pos && pos<=100 ) {
				cp = new osg::ClipPlane(0) ;
			}
			osg::BoundingSphere bs = (models[i])->getBound();
			osg::Vec3 c = bs.center() * osg::computeLocalToWorld((models[i])->getParentalNodePaths()[0]);
			double t = (50.0 - (double)pos) / 50.0;
			switch( dir ) {
			case 1:	cp->setClipPlane(-1,0,0,c.x() + t*bs.radius()); break;
			case 2: cp->setClipPlane(0,-1,0,c.y() + t*bs.radius()); break;
			case 3:	cp->setClipPlane(0,0,-1,c.z() + t*bs.radius()); break;
			case 4:	cp->setClipPlane(1,0,0,-c.x() - t*bs.radius()); break;
			case 5: cp->setClipPlane(0,1,0,-c.y() - t*bs.radius()); break;
			case 6:	cp->setClipPlane(0,0,1,-c.z() - t*bs.radius()); break;
			}
			state->setAttributeAndModes( cp.get(), osg::StateAttribute::ON);
			cpON = true;
		}
	}
}

void cOSG::ChangePointSize(double s)
{
	if(!pASMEMgr) return;
	int i = pASMEMgr->current();
	if( models[i].valid() ) {
		osg::StateSet * state = models[i]->getOrCreateStateSet();

		osg::Point* point = new osg::Point;
		point->setSize(s);
		state->setAttributeAndModes(point, osg::StateAttribute::ON);
	}
}

void cOSG::ChangeTransparency(double t)
{
	if(!pASMEMgr) return;
	int i = pASMEMgr->current();
	if(models[i].valid()) {
		osg::StateSet * state = models[i]->getOrCreateStateSet();
		osg::Material * mm = dynamic_cast<osg::Material*>(state->getAttribute(osg::StateAttribute::MATERIAL));

		if(0.0<=t && t<0.98) {
			mm->setAlpha(osg::Material::FRONT, t);
			mm->setColorMode( osg::Material::AMBIENT );
			// Enable blending, select transparent bin.
			state->setMode(GL_BLEND,   osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
			state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			// always on in this context. both should turned on otherwise.
			//state->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );  
			//state->setMode( GL_LIGHTING, osg::StateAttribute::ON );			
		} else {
			mm->setAlpha(osg::Material::FRONT, 1.0);
			mm->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );
			state->setMode(GL_BLEND,   osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
			state->setRenderingHint(osg::StateSet::DEFAULT_BIN);
		}
	}
}

void cOSG::SetRenderAttribute(ViewStyle style)
{
	if(style == DraftingHidden)		// this button is for clear-selections, should never happen
		return;
	if(!pASMEMgr) 
		return;
	int i = pASMEMgr->current();
	if( models[i].valid() ) {
		osg::StateSet * state = models[i]->getOrCreateStateSet();

		// shaded edge button clicked
		toggleScribe(style);


		osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;

		// wireframe button clicked
		if(style == Wireframe) {
			pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
			state->setMode( GL_BLEND, osg::StateAttribute::OFF ); 
		}
		// shaded button clicked
		if(style == Shaded || style == WireAndSils) {
			pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
			state->setMode( GL_BLEND, osg::StateAttribute::ON ); 
		}
		// point cloud button clicked
		if(style == Hidden) 
			pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::POINT);

		state->setAttribute( pm.get() );
	}
}

void cOSG::toggleScribe(ViewStyle style) 
{
	if(!pASMEMgr) return;
	int i = pASMEMgr->current();
    if (!world.valid() || !models[i].valid() ) return;

	osg::ref_ptr<osg::Group> parent = models[i]->getParent(0);
    osg::ref_ptr<osgFX::Scribe> scribed = dynamic_cast<osgFX::Scribe*>( parent.get() );
	if (!scribed.valid() && style==WireAndSils)
    {
        // node not yet scribed
        scribed = new osgFX::Scribe;
		scribed->setWireframeColor( osg::Vec4(0.0f,0.0f,0.0f,1.0f) );
		scribed->setWireframeLineWidth(1.0f);
        scribed->addChild(models[i]);
        world->replaceChild(models[i],scribed);
    }
	if (scribed.valid() && style!=WireAndSils)
    {
        // remove the scribe
		scribed->removeChild(models[i]);
        osg::Node::ParentList parentList = scribed->getParents();
        for(osg::Node::ParentList::iterator itr=parentList.begin(); itr!=parentList.end(); ++itr)
        {
            (*itr)->replaceChild(scribed,models[i]);
        }
		scribed = 0;
    }
}

void cOSG::SetPickMode(int mode)
{
	if(m_pPickHander)
		m_pPickHander->setPickMode(mode);
}

void cOSG::SetAnnotateMode(int mode)
{
	if(m_pPickHander)
		m_pPickHander->setAnnotateMode(mode);
}

int cOSG::GetAnnotateMode()
{
	if(m_pPickHander)
		return m_pPickHander->getAnnotateMode();
	return 0;
}

//void cOSG::displayMeshEdges(AssemblyMesh *pAsmMesh)
//{
//	FindNodeVisitor cNodeVisitor("SW_Body_");
//	model->accept(cNodeVisitor);
//
//	std::vector<osg::Node*>::iterator ndIter;
//	std::vector<osg::Node*> foundNodeList = cNodeVisitor.getNodeList();
//	for(ndIter=foundNodeList.begin(); ndIter!=foundNodeList.end(); ndIter++) {
//		osg::Switch *pSwicthnode = (*ndIter)->asSwitch();
//		if(!pSwicthnode)
//			continue;
//		unsigned int nc = pSwicthnode->getNumChildren();
//		// should only have one or two children
//		if(nc>1) {
//			pSwicthnode->setAllChildrenOn();
//		} else {
//			CString str;
//			CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
//			clock_t start=clock();
//		    pFactory->addMeshEdgeGeode(pAsmMesh,pSwicthnode);
//			str.Format(_T("Added mesh edges in %f seconds."),(double)(clock()-start)/CLOCKS_PER_SEC);
//			mwApp->SetStatusBarString(str);
//		}
//	}
//}

//void cOSG::hideMeshEdges()
//{
//	FindNodeVisitor cNodeVisitor("SW_Body_");
//	model->accept(cNodeVisitor);
//
//	std::vector<osg::Node*>::iterator ndIter;
//	std::vector<osg::Node*> foundNodeList = cNodeVisitor.getNodeList();
//	for(ndIter=foundNodeList.begin(); ndIter!=foundNodeList.end(); ndIter++) {
//		osg::Switch *pSwicthnode = (*ndIter)->asSwitch();
//		if(!pSwicthnode)
//			continue;
//		// only two children, 0 - shaded   1 - edges
//		pSwicthnode->setValue(1,false);
//	}
//}

void cOSG::PreFrameUpdate()
{
    // Due any preframe updates in this routine
}

void cOSG::PostFrameUpdate()
{
    // Due any postframe updates in this routine
}

void cOSG::Render(void* ptr)
{
    cOSG* osg = (cOSG*)ptr;
    osgViewer::Viewer* viewer = osg->getViewer();
	if( viewer ) {
		// viewer.frame() will call realize() if the viewer is not already realized, 
		//   and installs trackball manipulator if one is not already assigned
		osg->PreFrameUpdate();
		viewer->frame();
		osg->PostFrameUpdate();
	}
}

void cOSG::SetCurrentOperation( int currentOperation )
{
	//MakeCurrent();

	//double diffx, diffy;
	//diffx = (double) (next.x - current.x);
	//diffy = (double) (next.y - current.y);

	//double angleX, angleY;
	//angleX = diffx * ROTATE_SENSITIVITY * 0.025 ;
	//angleY = diffy * ROTATE_SENSITIVITY * 0.025 ;

	//TRACE("angleX=%f, angleY=%f\n",angleX, angleY);

	//// create the rotation matrix
	//osg::Matrix x_rot, y_rot;
	//x_rot.makeRotate(angleX, osg::Vec3(0.0, 1.0, 0.0));
	//y_rot.makeRotate(angleY, osg::Vec3(1.0, 0.0, 0.0));
	//
	//osg::Matrix m = camera->getViewMatrix();
	//m.preMult(y_rot);
	//m.preMult(x_rot);
	//camera->setViewMatrix(m);

	trackball.get()->setCurrentOperation(currentOperation);
	//Render(this);
}


//
//
//

osgText::Text* cOSG::createText( const osg::Vec3& pos, const std::string& content,
						   float size, int dir,  const osg::Vec4& color)
{
	osg::ref_ptr<osgText::Text> text = new osgText::Text;
	text->setDataVariance( osg::Object::DYNAMIC );
	text->setFont( g_font.get() );
	text->setCharacterSize( size );
	switch(dir) {
	case 0: text->setAxisAlignment( osgText::TextBase::YZ_PLANE ); break;
	case 1: text->setAxisAlignment( osgText::TextBase::XZ_PLANE ); break;
	case 2: text->setAxisAlignment( osgText::TextBase::XY_PLANE ); break;
	}
	text->setPosition( pos );
	text->setText( content );
	//text->setDrawMode( osgText::Text::TEXT | osgText::Text::BOUNDINGBOX );
#ifdef BLACK_WHITE_MODE
	text->setColor( osg::Vec4(1.0f-BKCOLOR_R, 1.0f-BKCOLOR_G, 1.0f-BKCOLOR_B, 1.0f) );
#else
	text->setColor( color );
#endif
	return text.release();
}

void cOSG::GetScreenAndTextSize(int* w, int* h, float* ts)
{
	*w = m_winWidth;
	*h = m_winHeight;
	*ts = 0.01*(m_winWidth - 1280) + 15.0f;
}

double cOSG::GetBSphereRadius()
{
	return bsRadius;
}

void cOSG::clearHudText()
{ 
	if( !textGeode )
		return;
	if( !textGeode.valid() )
		return;

	int num = textGeode->getNumDrawables();
	if( num>0 ) {
		textGeode->removeDrawables(0,num);
	}
}

void cOSG::clearHudDrawing()
{ 
	if( !gfx2Dgeode.valid() )
		return;

	int num = gfx2Dgeode->getNumDrawables();
	if( num>0 ) {
		gfx2Dgeode->removeDrawables(0,num);
	}
}

void cOSG::addHudText(const osg::Vec3& pos, const std::string& content)
{	
	if( !textGeode.valid() ) {
		textGeode = new osg::Geode;
		hud->addChild( textGeode.get() );
	}
	int w,h;
	float ts;
	GetScreenAndTextSize(&w,&h,&ts);
	textGeode->addDrawable( createText(pos, content, ts, 2, osg::Vec4(1.0f,1.0f,1.0f,1.0f)) );
}

void cOSG::drawHUDRect(float xmin, float xmax, float ymin, float ymax)
{
	if( !gfx2Dgeode.valid() ) {
		gfx2Dgeode = new osg::Geode;
		hud->addChild( gfx2Dgeode.get() );
	}

	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
	v->push_back(osg::Vec3(xmin,ymin,0));
	v->push_back(osg::Vec3(xmin,ymax,0));
	v->push_back(osg::Vec3(xmax,ymax,0));
	v->push_back(osg::Vec3(xmax,ymin,0));

	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array; 
	norms->push_back( osg::Vec3(0.0,0.0,1.0) );

	osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
	quad->setVertexArray( v.get() );
	quad->setNormalArray( norms.get() );
	quad->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
	c->push_back( osg::Vec4(SELECTED_FCOLOR_R, SELECTED_FCOLOR_G, SELECTED_FCOLOR_B, 0.5f) );	
	quad->setColorArray(c.get());
	quad->setColorBinding(osg::Geometry::BIND_OVERALL);

	bool fill=false;
	if(fill)
		quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
	else
		quad->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP,0,4));

	gfx2Dgeode->addDrawable( quad.get() );
	return;
}

void cOSG::drawHUDLine(float x1, float y1, float x2, float y2)
{
	if( !gfx2Dgeode.valid() ) {
		gfx2Dgeode = new osg::Geode;
		hud->addChild( gfx2Dgeode.get() );
	}

	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
	v->push_back(osg::Vec3(x1,y1,0));
	v->push_back(osg::Vec3(x2,y2,0));

	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array; 
	norms->push_back( osg::Vec3(0.0,0.0,1.0) );

	osg::ref_ptr<osg::Geometry> Segm = new osg::Geometry;
	Segm->setVertexArray( v.get() );
	Segm->setNormalArray( norms.get() );
	Segm->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
	c->push_back( osg::Vec4(SELECTED_FCOLOR_R, SELECTED_FCOLOR_G, SELECTED_FCOLOR_B, 0.5f) );	
	Segm->setColorArray(c.get());
	Segm->setColorBinding(osg::Geometry::BIND_OVERALL);

	Segm->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

	gfx2Dgeode->addDrawable( Segm.get() );
	return;
}

//void cOSG::drawHUDColorLegend(double miV, double maV)
//{
//	if( !lgd2Dgeode.valid() ) {
//		lgd2Dgeode = new osg::Geode;
//		hud->addChild( lgd2Dgeode.get() );
//	}
//
//	// 
//	static const  osg::Vec4 red = osg::Vec4 ( 1.0f, 0.0f, 0.0f, 1.0f );
//	static const  osg::Vec4 green = osg::Vec4( 0.0f, 1.0f, 0.0f, 1.0f );
//	static const  osg::Vec4 blue = osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f );
//	static const  osg::Vec4 yellow = osg::Vec4 ( 1.0f,1.0f,0.0f,1.0f );
//	static const  osg::Vec4 cyan = osg::Vec4( 0.0f, 1.0f, 1.0f, 1.0f );
//	static const  osg::Vec4 purple = osg::Vec4 ( 1.0f,0.0f,1.0f,1.0f );
//
//	float xmin = 0.93f* m_winWidth; 
//	float xmax = 0.95f* m_winWidth;
//	float ymin = 0.3f * m_winHeight; 
//	float ymax = 0.6f * m_winHeight;
//
//	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
//
//	int i, k;
//	float y=ymin, dy = (ymax - ymin) / 4.0f;
//	for(i=0; i<5; i++) {
//		v->push_back(osg::Vec3(xmin,y,0));
//		v->push_back(osg::Vec3(xmax,y,0));
//		y += dy;
//	}
//
//	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array; 
//	norms->push_back( osg::Vec3(0.0f,0.0f,1.0f) );
//
//	osg::ref_ptr<osg::Geometry> colorLegend = new osg::Geometry;
//	colorLegend->setVertexArray( v.get() );
//	colorLegend->setNormalArray( norms.get() );
//	colorLegend->setNormalBinding(osg::Geometry::BIND_OVERALL);
//
//	osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
//	c->push_back( blue );	
//	c->push_back( blue );	
//	c->push_back( cyan );	
//	c->push_back( cyan );
//	c->push_back( green );	
//	c->push_back( green );	
//	c->push_back( yellow );	
//	c->push_back( yellow );
//	c->push_back( red );	
//	c->push_back( red );
//	colorLegend->setColorArray(c.get());
//	colorLegend->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
//
//	osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
//	k = 0;
//	for(i=0; i<4; i++) {
//		quads->push_back(k);
//		quads->push_back(k+1);
//		quads->push_back(k+3);
//		quads->push_back(k+2);
//		k += 2;
//	}
//	colorLegend->addPrimitiveSet(quads);
//	
//	lgd2Dgeode->addDrawable( colorLegend.get() );
//
//	// add texts
//	osg::Vec3 pos;
//	float ts = 0.008*(m_winWidth - 1280) + 10.0f;
//	float dx = 0.001*m_winWidth;
//	double dv = (maV - miV) / 4.0;
//	double val = miV;
//	y = ymin; 
//	for(i=0; i<5; i++) {
//		std::stringstream oss;
//		pos.set(osg::Vec3(xmax+dx,y,0));
//		if(val >= 0.0) 
//			oss << std::setprecision(5) << " " << val;		
//		else
//			oss << std::setprecision(5) << val;
//		lgd2Dgeode->addDrawable( createText(pos, oss.str(), ts, 2, osg::Vec4(1.0f,1.0f,1.0f,1.0f)) );
//		y += dy;
//		val += dv;
//	}
//
//	return;
//}

void cOSG::clearHighlight()
{
	if( !decorator )
		return;
	if( !decorator.valid() )
		return;

	int num = decorator->getNumChildren();
	if(num > 0)
		decorator->removeChild(0,num);
	//int num = decorator->getNumDrawables();
	//if( num>0 ) {
	//	decorator->removeDrawables(0,num);
	//}
}

//void cOSG::highlight(osg::Vec3& p0, osg::Vec3& p1, osg::Vec3& p2)
void cOSG::highlight(int nv, double fxyz[4][3])
{
	if( !world.valid() )
		return;

	if(nv == 2) {
		double xyz[64][3];
		int i,j;
		for(i=0; i<2; i++) 
			for(j=0; j<3; j++)
				xyz[i][j] = fxyz[i][j];
		highlightPolyLine(2, xyz, 4.0f,osg::Vec4(1.0f,1.0f,0.0f,1.0f));
		return;
	}


	shrink(nv,fxyz,0.90);
	osg::Vec3 p0(fxyz[0][0], fxyz[0][1],fxyz[0][2] );
	osg::Vec3 p1(fxyz[1][0], fxyz[1][1],fxyz[1][2] );
	osg::Vec3 p2(fxyz[2][0], fxyz[2][1],fxyz[2][2] );

	osg::Vec3 nor = (p1 - p0) ^ (p2 - p0);
	if(nor.length2() < TOLERANCE2)
		return;

	if( !decorator.valid() ) {
		decorator = new osg::Group;
		decorator->setDataVariance( osg::Object::DYNAMIC );
		decorator->setNodeMask( 0x1 );	
		world->addChild( decorator );
	} 
	//else {
	//	int num = decorator->getNumDrawables();
	//	if( num>0 )
	//		decorator->removeDrawables(0,num);
	//}

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	decorator->addChild( geode );

	osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
	c->push_back( osg::Vec4(SELECTED_FCOLOR_R, 
		                    SELECTED_FCOLOR_G, 
							SELECTED_FCOLOR_B, 0.5f) );	

	if(nv == 3) {
		osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
		v->push_back( p0 );
		v->push_back( p1 );
		v->push_back( p2 );
		osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;
		nor.normalize();
		n->push_back(nor);

		osg::ref_ptr<osg::Geometry> tri = new osg::Geometry;
		tri->setVertexArray( v.get() );
		tri->setNormalArray( n.get() );
		tri->setNormalBinding(osg::Geometry::BIND_OVERALL);
		tri->setColorArray(c.get());
		tri->setColorBinding(osg::Geometry::BIND_OVERALL);
		tri->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLES, 0,3) );
		geode->addDrawable( tri.get() );
	}

	else if(nv==4) {
		bool zero_normal[4];
		double normals[4][3];
		int i, iii = -1;
		for(i=0;i<4;i++) {
			if( !norm(fxyz[i],fxyz[(i+1)%4],fxyz[(i+2)%4],normals[(i+1)%4]) ) 
				zero_normal[(i+1)%4] = false;
			else {
				zero_normal[(i+1)%4] = true;
				iii = (i+1)%4;
			}
		}
		if(iii != -1)
		{
			osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
			osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(4);
			quad->setVertexArray( v.get() );

			osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array(4); 
			quad->setNormalArray(norms.get());
			quad->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

			quad->setColorArray(c.get());
			quad->setColorBinding(osg::Geometry::BIND_OVERALL);

			(*v)[0].set(fxyz[0][0],fxyz[0][1],fxyz[0][2]);
			(*v)[1].set(fxyz[1][0],fxyz[1][1],fxyz[1][2]); 
			(*v)[2].set(fxyz[2][0],fxyz[2][1],fxyz[2][2]); 
			(*v)[3].set(fxyz[3][0],fxyz[3][1],fxyz[3][2]);
			for(i=0;i<4;i++) {
				if(zero_normal[i])
					(*norms)[i].set(normals[i][0],normals[i][1],normals[i][2]);
				else
					(*norms)[i].set(normals[iii][0],normals[iii][1],normals[iii][2]);
			}
			quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
			geode->addDrawable( quad.get() );
		}
	}

	return;
}

void cOSG::highlightP(double xyz[3], double radius, int transparent)
{
	if( !world.valid() )
		return;

	if( !decorator.valid() ) {
		decorator = new osg::Group;
		decorator->setDataVariance( osg::Object::DYNAMIC );
		decorator->setNodeMask( 0x1 );	
		world->addChild( decorator );
	} 

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	decorator->addChild( geode );
	osg::ref_ptr<osg::ShapeDrawable> point = new osg::ShapeDrawable;
	point->setShape( new osg::Sphere( osg::Vec3(xyz[0],xyz[1],xyz[2]), radius ) );			
	point->setColor( osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) );					// red
	geode->addDrawable(point.get());

	if(transparent) {
		osg::Material *material = new osg::Material();
		material->setSpecular(osg::Material::FRONT, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
		material->setShininess(osg::Material::FRONT, 40.0f);
		material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );
		material->setAlpha(osg::Material::FRONT, 0.15);
		material->setColorMode( osg::Material::AMBIENT );

		osg::StateSet * state = geode->getOrCreateStateSet();
		state->setAttribute(material);
		state->setMode(GL_BLEND,   osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
		state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	}
	return;
}

void cOSG::highlightCircle(double center[3], double axis[3], double radius)
{
	if( !world.valid() )
		return;

	if( !decorator.valid() ) {
		decorator = new osg::Group;
		decorator->setDataVariance( osg::Object::DYNAMIC );
		decorator->setNodeMask( 0x1 );	
		world->addChild( decorator );
	} 

	double zaxis[3]={0.,0.,1.0};
	double w[3], dp, theta;
	normVt(axis,axis);
	crossProd(zaxis,axis,w);
	dp = dotProd(zaxis,axis);
	theta = acos(dp) * 180 / osg::PI;

	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
	osg::Matrix m1,m2;
	m1.makeRotate(osg::DegreesToRadians(theta),w[0],w[1],w[2]);
	m2.makeTranslate(osg::Vec3(center[0],center[1],center[2]));
	mt->setMatrix(m1*m2);
	decorator->addChild(mt.get());

	osg::ref_ptr<osg::Geode> geode = createCircleOnXYPlane(radius,50);
	mt->addChild(geode.get());

	return;
}

osg::ref_ptr<osg::Geode> cOSG::createCircleOnXYPlane( double radius, int approx )
{
    double x, y, z(0.);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::LineWidth* lw = new osg::LineWidth( 3. );
    geode->getOrCreateStateSet()->setAttributeAndModes( lw, osg::StateAttribute::ON );

	const double angle( osg::PI * 2. / (double) approx );
	osg::Vec3Array* v = new osg::Vec3Array;
	for(int idx=0; idx<approx; idx++) {
		 x = radius*cosf( idx*angle );
		 y = radius*sinf( idx*angle );
		 v->push_back( osg::Vec3(x,y,z) );
	}

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( v );

    osg::Vec4Array* c = new osg::Vec4Array;
    c->push_back( osg::Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays( GL_LINE_LOOP, 0, approx ) );

    geode->addDrawable( geom );
    return geode;
}

void cOSG::highlightPolyLine(int num, double xyz[64][3], int LineWidth, osg::Vec4 &color)
{
	if(num >64) num = 64;

	if( !world.valid() )
		return;

	if( !decorator.valid() ) {
		decorator = new osg::Group;
		decorator->setDataVariance( osg::Object::DYNAMIC );
		decorator->setNodeMask( 0x1 );	
		world->addChild( decorator );
	} 
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	decorator->addChild( geode );
    osg::LineWidth* lw = new osg::LineWidth( LineWidth );
    geode->getOrCreateStateSet()->setAttributeAndModes( lw, osg::StateAttribute::ON );

	osg::Vec3Array* v = new osg::Vec3Array;
	for(int i=0; i<num; i++) 
		 v->push_back( osg::Vec3(xyz[i][0],xyz[i][1],xyz[i][2]) );

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( v );

    osg::Vec4Array* c = new osg::Vec4Array;
    c->push_back( color );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, num ) );
    geode->addDrawable( geom );

    return;
}

void cOSG::addText(const osg::Vec3& pos, const std::string& content, osgText::TextBase::AlignmentType alignment)
{	
	if( !world.valid() )
		return;
	if( !decorator.valid() ) {
		decorator = new osg::Group;
		decorator->setDataVariance( osg::Object::DYNAMIC );
		decorator->setNodeMask( 0x1 );	
		world->addChild( decorator );
	} 

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	decorator->addChild( geode );

	osg::ref_ptr<osgText::Text> text = new osgText::Text;
	text->setDataVariance( osg::Object::DYNAMIC );
	text->setFont( g_font.get() );
	text->setFontResolution(64,64);
	text->setCharacterSize( 0.03 * bsRadius );
	text->setAxisAlignment( osgText::TextBase::SCREEN );
	text->setAlignment( alignment );
	text->setLayout( osgText::TextBase::LEFT_TO_RIGHT );
	text->setPosition( pos );
	text->setText( content );
	text->setDrawMode( osgText::Text::TEXT | osgText::Text::BOUNDINGBOX );
#ifdef BLACK_WHITE_MODE
	text->setColor( osg::Vec4(1.0f-BKCOLOR_R, 1.0f-BKCOLOR_G, 1.0f-BKCOLOR_B, 1.0f) );
#else
	text->setColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
#endif

	 geode->addDrawable( text );
}

//
// transform the model of the given index
void cOSG::postMult(osg::Matrix& tarnsf, int index)
{
	osg::ref_ptr<osg::Group> currMo = getSceneModel(index); 
	if(currMo.valid()) {
		unsigned int nc=currMo->getNumChildren();
		for(unsigned int i=0; i<nc; i++) {
			osg::MatrixTransform* mt = (osg::MatrixTransform *) currMo->getChild(i);
			//mt->preMult(tarnsf);
			mt->postMult(tarnsf);
		}
	}
}

// post processing
void cOSG::lineplot(int np, double *xyz, double *proj, double minV, double maxV)
{
	if( !world.valid() )
		return;
	post.setValueRangle(minV, maxV);
	post.draw_lineplot(np,xyz,proj);
	post.attachParent(world, hud, m_winWidth, m_winHeight);
}


void cOSG::colormap(int nv, int nf, double *xyz, int *tria, double *values, double minV, double maxV)
{
	if( !world.valid() )
		return;
	post.setValueRangle(minV, maxV);
	post.draw_colormap(nv,nf,xyz,tria,values);
	post.attachParent(world, hud, m_winWidth, m_winHeight);
}

int cOSG::showcolormap()
{
	if( !world.valid() )
		return 0;
	if( post.showcolormap() ) {
		post.attachParent(world, hud, m_winWidth, m_winHeight);
		return 1;
	}
	return 0;
}

int cOSG::showlineplot()
{
	if( !world.valid() )
		return 0;
	if( post.showlineplot() ) {
		post.attachParent(world, hud, m_winWidth, m_winHeight);
		return 1;
	}
	return 0;
}

SCENETYPE cOSG::currentscene()
{
	if( !world.valid() )
		return __NO_SCENE;
			
	// find post switch node
	unsigned int nc=world->getNumChildren();
	for(unsigned int i=0; i<nc; i++) {
		osg::Switch* sw = (osg::Switch *) world->getChild(i);
		if(sw != NULL) {
			std::string nodename = sw->getName();
			if(nodename == "PostSW") {
				if(sw->getValue(0))
					return __COLORMAP;
				if(sw->getValue(1))
					return __LINEPLOT;
			}
		}
	}
	return __MESH;
}

//
// primitiveIndex starts from zero
//
bool cOSG::loopupPostData(ATTRTYPE type, int primitiveIndex, double xyz[4][3], double *val)
{
	if(!pASMEMgr) 
		return false;
	AssemblyMesh *pASME = pASMEMgr->withAttribData(type);
	if(!pASME)
		return false;

	int num;
	double mindbl, maxdbl;
	double *pDbl;
	pASME->attr_get(&num, &pDbl, &mindbl, &maxdbl);  // do not free
			
	osg::Matrix *transf;
	osg::Vec3d pt;
	pVertex v;
	if(type == FACE_SCALAR2) {
		// array index starts from zero, two values: mean and max,
		// consistent with computeColormapValues
		*val = pDbl[2*primitiveIndex+1];     
		if(*val == VALUE_NOT_EXIST)
			return false;

		pFace f = pASME->ithFaceOfArray(primitiveIndex+1, &transf);  // index of 1st face = 1
		if(!f) 
			return false;
				
		int nv = F_numVertices(f);
		for(int i=0; i<nv; i++) {
			v = F_vertex(f,i);
			V_getCoord(v, xyz[i]);
			if(transf != NULL) {
				pt.set(xyz[i][0],xyz[i][1],xyz[i][2]);
				pt = transf->preMult(pt);
				xyz[i][0] = pt.x();
				xyz[i][1] = pt.y();
				xyz[i][2] = pt.z();
			}
		}

	} else if(type == VERTEX_VECTOR_SCALAR) {
		// primitiveIndex incorrect, 
		// xyz[0], the picked world coordinates, is used to search
		double picked[3], pc[3], dx, dy, dz, dd, ddmin=1.0e28;
		int i, nv, nf, nq, nr;
		bool found = false;
		pASME->countEntities(&nv, &nf, &nq, &nr);
		picked[0] = xyz[0][0];
		picked[1] = xyz[0][1];
		picked[2] = xyz[0][2];
		for(i=0; i<nv; i++) {
			dx = picked[0] - pDbl[4*i];
			dy = picked[1] - pDbl[4*i + 1];
			dz = picked[2] - pDbl[4*i + 2];
			dd = dx*dx + dy*dy + dz*dz;
			if(dd < ddmin) ddmin = dd;
			if(dd < 1.0e-16) {
				found = true;
				primitiveIndex = i;
				*val = pDbl[4*i + 3];
				break;
			}
		}

		if(found) {
			// picked a projection point
			v = pASME->ithVertexOfArray(primitiveIndex+1, &transf); // index of 1st vertex = 1
			V_getCoord(v,pc);
			pt.set(pc[0],pc[1],pc[2]);
			pt = transf->preMult(pt);
			xyz[0][0] = pt.x();
			xyz[0][1] = pt.y();
			xyz[0][2] = pt.z();
			xyz[1][0] = picked[0];
			xyz[1][1] = picked[1];
			xyz[1][2] = picked[2];
		} else {
			// picked a point in the given clouds
			// because scale factor, the cloud point can't be piacked
			return false;
		 //   primitiveIndex = pASME->vertexIndexFromXYZ(picked);
			//if(primitiveIndex <= 0)
			//	return false;   // not found
			//*val = pDbl[primitiveIndex*4 + 3];
			//xyz[1][0] = pDbl[primitiveIndex*4];
			//xyz[1][1] = pDbl[primitiveIndex*4 + 1];
			//xyz[1][2] = pDbl[primitiveIndex*4 + 2];
		}
	}
	return true;
}

bool cOSG::findMeshEntityIDs(int occurance, int gface_id, int index, int ids[5], double xyz[4][3])
{
	if(pASMEMgr) {
		AssemblyMesh *pASME = pASMEMgr->ith(pASMEMgr->current());
		if(pASME) {
			pVertex v;
			osg::Vec3d pt;
			osg::Matrix *transf;
			pFace f = (pFace)pASME->findMeshEntityFromIDs(occurance, gface_id, index, &transf);
			if(!f)
				return false;
			int nv = F_numVertices(f);
			ids[0] = EN_getID((pEntity)f);
			ids[1] = nv;
			for(int i=0; i<nv; i++) {
				v = F_vertex(f,i);
				ids[i+2] = EN_getID((pEntity)v);
				V_getCoord(v, xyz[i]);
				if(transf != NULL) {
					pt.set(xyz[i][0],xyz[i][1],xyz[i][2]);
					pt = transf->preMult(pt);
					xyz[i][0] = pt.x();
					xyz[i][1] = pt.y();
					xyz[i][2] = pt.z();
				}
			}
		}
	}
	return true;
}

// angle is invarant, so no need to account for transf here
int cOSG::crossEdgeAngle(int occurance, int id0, int id1, double *angle)
{
	int nf = 0;
	if(pASMEMgr) {
		AssemblyMesh *pASME = pASMEMgr->ith(pASMEMgr->current());
		if(pASME) {
			pVertex v;
			int i, j, id;

			std::set<pFace> mfaces;
			std::set<pFace>::iterator itr;
			std::vector<pFace> efaces;

			pASME->getNeighboringFacesFromVertexID(occurance, id0, mfaces);
			for(itr=mfaces.begin(); itr!=mfaces.end(); itr++) {
				for(i=0; i<3; i++) {
					v = F_vertex(*itr,i);
					id = EN_getID((pEntity)v);
					if(id == id1) {
						efaces.push_back(*itr);
						break;
					}
				}
			}
			nf = efaces.size();
			if(nf == 2) {
				double fxyz[3][3], n[2][3], dp;
				for(i=0; i<2; i++) {
					for(j=0; j<3; j++) {
						v = F_vertex(efaces[i],j);
						V_getCoord(v, fxyz[j]);
					}
					norm(fxyz[0],fxyz[1],fxyz[2],n[i]);
				}
				dp = dotProd(n[0],n[1]);
				*angle = 180.0*acos(dp) / M_PI;
			}
		}
	}
	return nf;
}

//
//
//
osg::ref_ptr<osg::Group> cOSG::createTriad(double ori[3], double len)
{
	float cyl_radius = 0.02*len;
	float cone_radius = 0.04*len;
	osg::ref_ptr<osg::Group> triad = new osg::Group;

	// z -axis
	osg::ref_ptr<osg::Geode> zzz = new osg::Geode;

	osg::ref_ptr<osg::ShapeDrawable> cyl_z = new osg::ShapeDrawable;
	osg::ref_ptr<osg::ShapeDrawable> cone_z = new osg::ShapeDrawable;
	cyl_z->setShape( new osg::Cylinder( osg::Vec3(ori[0],ori[1],ori[2]), cyl_radius, len ) );				// always along z direction
	cone_z->setShape( new osg::Cone( osg::Vec3(ori[0],ori[1],ori[2]+len*0.6), cone_radius, len*0.4 ) );
	cyl_z->setColor( osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) );					// blue
	cone_z->setColor( osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) );

	zzz->addDrawable(cyl_z.get());
	zzz->addDrawable(cone_z.get());
	triad->addChild(zzz.get());

	// y - axis
	osg::Matrix mR, mPreT, mPostT;
	mPreT.makeTranslate(-ori[0],-ori[1],-ori[2]);
	mPostT.makeTranslate(ori[0],ori[1],ori[2]);
	mR.makeRotate( osg::inDegrees(90.0f), osg::Vec3f(-1.0,0.0,0.0) );
	
	osg::ref_ptr<osg::MatrixTransform> mt_y = new osg::MatrixTransform ;
    mt_y->setMatrix( mPreT * mR * mPostT );
    triad->addChild(mt_y.get());

	osg::ref_ptr<osg::Geode> yyy = new osg::Geode;
	osg::ref_ptr<osg::ShapeDrawable> cyl_y = new osg::ShapeDrawable;
	osg::ref_ptr<osg::ShapeDrawable> cone_y = new osg::ShapeDrawable;
	cyl_y->setShape( new osg::Cylinder( osg::Vec3(ori[0],ori[1],ori[2]), cyl_radius, len ) );
	cone_y->setShape( new osg::Cone( osg::Vec3(ori[0],ori[1],ori[2]+len*0.6), cone_radius, len*0.4 ) );
	cyl_y->setColor( osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) );					// green
	cone_y->setColor( osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) );

	yyy->addDrawable(cyl_y.get());
	yyy->addDrawable(cone_y.get());
	mt_y->addChild( yyy.get() );

	// x
	osg::ref_ptr<osg::MatrixTransform> mt_x = new osg::MatrixTransform ;
	mR.makeRotate( osg::inDegrees(90.0f), osg::Y_AXIS );
    mt_x->setMatrix( mPreT * mR * mPostT );
    triad->addChild(mt_x.get());

	osg::ref_ptr<osg::Geode> xxx = new osg::Geode;
	osg::ref_ptr<osg::ShapeDrawable> cyl_x = new osg::ShapeDrawable;
	osg::ref_ptr<osg::ShapeDrawable> cone_x = new osg::ShapeDrawable;
	cyl_x->setShape( new osg::Cylinder( osg::Vec3(ori[0],ori[1],ori[2]), cyl_radius, len ) );				// always along z direction
	cone_x->setShape( new osg::Cone( osg::Vec3(ori[0],ori[1],ori[2]+len*0.6), cone_radius, len*0.4 ) );
	cyl_x->setColor( osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) );					// red
	cone_x->setColor( osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) );

	xxx->addDrawable(cyl_x.get());
	xxx->addDrawable(cone_x.get());
	mt_x->addChild( xxx.get() );

	// label axes  --> should use Billboard node
	osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
	textGeode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	textGeode->addDrawable( createText(osg::Vec3(ori[0]+len*0.9,ori[1]-0.15,ori[2]-0.15),"X",0.3f,0,osg::Vec4(1.0f,0.0f,0.0f,1.0f)));
	textGeode->addDrawable( createText(osg::Vec3(ori[0]-0.15,ori[1]+len*0.9,ori[2]-0.15),"Y",0.3f,1,osg::Vec4(0.0f,1.0f,0.0f,1.0f)));
	textGeode->addDrawable( createText(osg::Vec3(ori[0]-0.15,ori[1]-0.15,ori[2]+len*0.9),"Z",0.3f,2,osg::Vec4(0.0f,0.0f,1.0f,1.0f)));
	triad->addChild(textGeode.get());

	//triad->setDataVariance( osg::Object::STATIC );
	return triad.get();
}

void cOSG::ChangeInteriorLightStatus(int status)
{
	if(status)
		world->getOrCreateStateSet()->setMode( GL_LIGHT1, osg::StateAttribute::ON );
	else
		world->getOrCreateStateSet()->setMode( GL_LIGHT1, osg::StateAttribute::OFF );
	return;
}

// iBKColor : 0 dark blue; 1 gradient; 2 white
void cOSG::ChangeBackground(int iOldBKColor, int iBKColor)
{
	if((iOldBKColor==0 || iOldBKColor==2) && iBKColor==1) {
		osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(4);
		quad->setVertexArray( v.get() );
		(*v)[0].set(0.0f,0.0f,0.0f);
		(*v)[1].set(m_winWidth,0.0f,0.0f); 
		(*v)[2].set(m_winWidth,m_winHeight,0.0f); 
		(*v)[3].set(0.0f,m_winHeight,0.0f);

		osg::ref_ptr<osg::Vec3Array> shared_normals = new osg::Vec3Array; 
		shared_normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
		quad->setNormalArray( shared_normals.get() );
		quad->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array(4); 
		quad->setColorArray(c.get());
		quad->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		(*c)[0].set(0.8f,0.8f,0.8f,1.0f);
		(*c)[1].set(0.8f,0.8f,0.8f,1.0f); 
		(*c)[2].set(0.2f,0.2f,0.4f,1.0f); 
		(*c)[3].set(0.2f,0.2f,0.4f,1.0f);

		quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

		osg::ref_ptr<osg::Geode> bkgdnode = new osg::Geode;
		bkgdnode->addDrawable( quad.get() );

		osg::ref_ptr<osg::Camera> bkgdHUD = new osg::Camera;
		bkgdHUD->setName("BKHUD");
		bkgdHUD->setCullingActive( false );
		bkgdHUD->setClearMask( GL_DEPTH_BUFFER_BIT );
		bkgdHUD->setAllowEventFocus( false );
		bkgdHUD->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
		bkgdHUD->setRenderOrder( osg::Camera::NESTED_RENDER );  //  <---
		bkgdHUD->setProjectionMatrix( osg::Matrix::ortho2D(0.0, m_winWidth, 0.0, m_winHeight) );
		bkgdHUD->addChild( bkgdnode.get() );

		osg::StateSet* ss = bkgdHUD->getOrCreateStateSet();
		ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		ss->setAttributeAndModes( new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0) );  // <---

		bkgdHUD->addChild( bkgdnode.get() );
		world->addChild( bkgdHUD );
	}

	if(iOldBKColor==0 && iBKColor==2) {
		mViewer->getCamera()->setClearColor(osg::Vec4f(1.0f,1.0f,1.0f,1.0f));
	}

	if(iOldBKColor==2 && iBKColor==0) {
		mViewer->getCamera()->setClearColor(osg::Vec4f(BKCOLOR_R,BKCOLOR_G,BKCOLOR_B,1.0f));
	}

	if(iOldBKColor==1) {
		// delete the background node
		unsigned int nc=world->getNumChildren();
		for(unsigned int i=0; i<nc; i++) {
			osg::Node *child = world->getChild(i);
			if( child->getName() == "BKHUD" ) {
				world->removeChild(child);
				child = NULL;
				break;
			}
		}
		if(iBKColor==0) 
			mViewer->getCamera()->setClearColor(osg::Vec4f(BKCOLOR_R,BKCOLOR_G,BKCOLOR_B,1.0f));
		if(iBKColor==2) 
			mViewer->getCamera()->setClearColor(osg::Vec4f(1.0f,1.0f,1.0f,1.0f));
	}
	return;
}


