//$c7   XRL 06/03/2013 remove screen flash by adding WS_BORDER in PreCreateWindow. Remove OnEraseBkgnd.
//$c6   XRL 09/20/2012 added OnLButtonDown and OnLButtonUp to support mouse click event
//$c5   XRL 08/01/2012 changed OnEraseBkgnd, ReSize and calls to mOSG->render to prevenet screen flash.
//$c4   XRL 07/31/2012 added ChangePointSize and ChangeTransparency.
//$c3   XRL 07/24/2012 Added DraftHidden to toggle mesh edges.
//$c2   XRL 07/03/2012 Added Hidden for shaded transparent view.
//$c1   XRL 02/03/2012 Created.
//=========================================================================
// MeshWorkOsgView.cpp : implementation of the CMeshWorkOsgView class
//

#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkOsgView.h"
#include "meshing/MeshGen.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

// MeshWorkOsgView

IMPLEMENT_DYNCREATE(CMeshWorkOsgView, CView)

BEGIN_MESSAGE_MAP(CMeshWorkOsgView, CView)
	//{{AFX_MSG_MAP(CMeshWorkOsgView)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_MESH_BTN, OnMeshBtn)
	ON_COMMAND(ID_SHADED_BTN, OnShadedBtn)
	ON_UPDATE_COMMAND_UI(ID_SHADED_BTN, OnUpdateShadedBtn)
	ON_COMMAND(ID_WIREFRAME_BTN, OnWireframeBtn)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME_BTN, OnUpdateWireframeBtn)
	ON_COMMAND(ID_SILHOUETTE_BTN, OnSilhouetteBtn)
	ON_UPDATE_COMMAND_UI(ID_SILHOUETTE_BTN, OnUpdateSilhouetteBtn)
	ON_COMMAND(ID_HIDDEN_BTN, OnHiddenBtn)
	ON_UPDATE_COMMAND_UI(ID_HIDDEN_BTN, OnUpdateHiddenBtn)
	ON_COMMAND(ID_DRAFTHIDDEN_BTN, OnDraftHiddenBtn)
	ON_UPDATE_COMMAND_UI(ID_DRAFTHIDDEN_BTN, OnUpdateDrafthiddenBtn)
	ON_COMMAND(ID_VIEWROTATE_BTN, OnViewrotateBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEWROTATE_BTN, OnUpdateViewrotateBtn)
	ON_COMMAND(ID_VIEWZOOM_BTN, OnViewzoomBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEWZOOM_BTN, OnUpdateViewzoomBtn)
	ON_COMMAND(ID_VIEWPAN_BTN, OnViewpanBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEWPAN_BTN, OnUpdateViewpanBtn)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

CMeshWorkOsgView::CMeshWorkOsgView() :
	mOSG(0L) 
{
	m_currentViewStyle = Wireframe;
	m_currentOperation = Rotate;
}

CMeshWorkOsgView::~CMeshWorkOsgView()
{
}

BOOL CMeshWorkOsgView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// Add border to the window to be consistent with the MeshWorkView window
	// Change the class name to our own name.
	// added 6/3/2013
    cs.style = WS_CHILD | WS_BORDER;

	CMeshWorks* pApp = (CMeshWorks*)AfxGetApp ();
    cs.lpszClass = pApp->m_strMyClassName;

	// Size the window to screen size and center it.
	// added 5/25/2012, becasue "CRect rect(0, 0, 0, 0);" when creating the view. 
	// otherwise zero width and height in ::GetWindowRect(m_hWnd, &rect)
    cs.cy = ::GetSystemMetrics(SM_CYSCREEN) / 1;
    cs.cx = ::GetSystemMetrics(SM_CXSCREEN) / 1;
    cs.y = ((cs.cy * 3) - cs.cy) / 2;
    cs.x = ((cs.cx * 3) - cs.cx) / 2;

	return CView::PreCreateWindow(cs);
}


// MeshWorkOsgView drawing

void CMeshWorkOsgView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	//m_eraseBkgnd = false;
	mOSG->Render((void *)mOSG);
	//m_eraseBkgnd = true;
}

osg::ref_ptr<osg::Node> CMeshWorkOsgView::GetSceneModel(int i)
{
	if(!mOSG)
		return NULL;
	return mOSG->getSceneModel(i);
}

// MeshWorkOsgView diagnostics

#ifdef _DEBUG
void CMeshWorkOsgView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CMeshWorkOsgView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif

CMeshWorkDoc* CMeshWorkOsgView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMeshWorkDoc)));
	return (CMeshWorkDoc*)m_pDocument;
}
#endif //_DEBUG


// MeshWorkOsgView message handlers

int CMeshWorkOsgView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    // Let MFC create the window before OSG
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Now that the window is created setup OSG
    mOSG = new cOSG(m_hWnd);
	mOSG->InitCameraViewerAndScene();
	mOSG->SetCurrentOperation(Rotate);

    return 1;
}

void CMeshWorkOsgView::OnDestroy()
{
    if(mOSG != 0) 
	{delete mOSG; mOSG=0;}

   // WaitForSingleObject(mThreadHandle, 1000);

    CView::OnDestroy();
}

void CMeshWorkOsgView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
	Invalidate();
}

void CMeshWorkOsgView::SetGeodeNode(osg::ref_ptr<osg::Geode> gn)
{
	mOSG->SetGeodeNode(gn);
}

// build or update scene graph in terms of AssemblyMeshMgr
// if mode == 0: 
//		clear everything in view, all scene models are destroyed and rebuilt
// if mode == 1:
//		append NOT_IN_SG AssemblyMesh into scene 
void CMeshWorkOsgView::SetSceneData(int mode, bool setViewer)
{
	// Create a node for the assembly mesh, and add into the a scene graph.
	// will create the model using createShapes() if assembly mesh is NULL
	pAssemblyMeshMgr pAssMeshMgr = GetDocument()->getAssemblyMeshMgr();
	if( mOSG->AddAssemblyModel(pAssMeshMgr, mode, setViewer) )
		// needed becasue the new scene is always PolygonMode::LINE
		m_currentViewStyle = Wireframe; 
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::SetSelectionMode(int mode)
{
	if(mOSG)
		mOSG->SetPickMode(mode);
}

void CMeshWorkOsgView::SetAnnotationMode(int mode)
{
	if(mOSG)
		mOSG->SetAnnotateMode(mode);
}

void CMeshWorkOsgView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
}

void CMeshWorkOsgView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Pass Key Presses into OSG
    mOSG->getViewer()->getEventQueue()->keyPress(nChar);

    // Close Window on Escape Key
    if(nChar == VK_ESCAPE)
    {
        //GetParent()->SendMessage(WM_CLOSE);
    }
}


BOOL CMeshWorkOsgView::OnEraseBkgnd(CDC* pDC)
{
	//CRect rect;
	//const COLORREF color = BKGDCOLOR;
	//GetClientRect(&rect);	
	//pDC->FillSolidRect(&rect,color);

    /* Do nothing, to avoid flashing on MSW */
    return false;
}

void CMeshWorkOsgView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	
	if (cx <= 0 || cy <= 0) return;  // spurious calls, which CAN happen when the view is created

	//m_eraseBkgnd = false;
}

void CMeshWorkOsgView::ChangeClipPlane(int iPlane, int pos)   // pos: 0->100
{
	mOSG->SetClipPlane(iPlane,pos);
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::ChangeBackground(int iOldBKColor, int iBKColor)
{
	mOSG->ChangeBackground(iOldBKColor,iBKColor);
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::ChangeInteriorLightStatus(int opposingLight)
{
	mOSG->ChangeInteriorLightStatus(opposingLight);
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::SetViewStyle(ViewStyle style)
{
	if ( m_currentViewStyle != style )
	{
		//pAssemblyMesh pAssMesh = GetDocument()->getAssemblyMesh();
		m_currentViewStyle = style;
		mOSG->SetRenderAttribute(style);
		mOSG->Render(mOSG);
		if(style==WireAndSils)	// needed after using toggleScribe() because
		{
			//Sleep(0.1);
			mOSG->Render(mOSG);	// scribe node needs double renderring  9/21/2012 
		}
	}
}

void CMeshWorkOsgView::ChangePointSize(double size, bool rerender)
{
	mOSG->ChangePointSize(size);
	if(rerender)
		mOSG->Render(mOSG);
}


void CMeshWorkOsgView::ChangeTransparency(double transparency, bool rerender)
{
	mOSG->ChangeTransparency(transparency);
	if(rerender)
		mOSG->Render(mOSG);
}

void CMeshWorkOsgView::HighLight(int nv, double f[4][3])
{
	mOSG->highlight(nv,f);
}

void CMeshWorkOsgView::HighLightP(double xyz[3], double r)
{
	mOSG->highlightP(xyz,r,0);
}

void CMeshWorkOsgView::ClearHighLight()
{
	mOSG->clearHighlight();
}

void CMeshWorkOsgView::HudText(int row, std::string& content, bool render)
{
	int w,h;
	float x,y, ts;

	mOSG->GetScreenAndTextSize(&w, &h, &ts);
	x = 0.004*w;
	y = h - (float)row * (ts+3.0f);		
	mOSG->addHudText(osg::Vec3(x, y, 0.0f), content); 

	if(render) 
		mOSG->Render(mOSG);
}

void CMeshWorkOsgView::ClearHUDText()
{
	mOSG->clearHudText();
}

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkView message handlers


//*****************************************************************************
// MODULE:	on Buttons
// DESC:	All toolbar button handlers go here

void CMeshWorkOsgView::OnMeshBtn() 
{
	CMeshWorkDoc* pDoc = GetDocument();
	pAssemblyMeshMgr pAsmMeMgr = pDoc->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
		mwApp->SetStatusBarString("Empty mesh document. Please open point cloud data...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshGen *pMesher = new CMeshGen;
		pMesher->triangulator(pAsmMe);
		delete pMesher;
	}
}

void CMeshWorkOsgView::OnShadedBtn() 
{
	mOSG->showmodels();
	SetViewStyle( Shaded );	
}

void CMeshWorkOsgView::OnWireframeBtn() 
{
	mOSG->showmodels();
	SetViewStyle ( Wireframe );
}

void CMeshWorkOsgView::OnSilhouetteBtn() 
{
	mOSG->showmodels();
	SetViewStyle ( WireAndSils );
}

void CMeshWorkOsgView::OnHiddenBtn() 
{
	mOSG->showmodels();
	SetViewStyle ( Hidden );	
}

//void CMeshWorkOsgView::OnDraftHiddenBtn() 
//{
//	SetViewStyle ( DraftingHidden );
//}

void CMeshWorkOsgView::ClearAllSelection()
{
	CMeshWorkDoc* pDoc = GetDocument();
	CSelectionMgr* pSelectMgr = pDoc->getSelectionMgr();
	pSelectMgr->clear();
	mOSG->clearHudText();
	mOSG->clearHighlight();
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::OnDraftHiddenBtn()
{
	ClearAllSelection();
}

void CMeshWorkOsgView::OnViewrotateBtn() 
{
	m_currentOperation = Rotate;
	mOSG->SetCurrentOperation(Rotate);
}

void CMeshWorkOsgView::OnViewzoomBtn() 
{
	m_currentOperation = Zoom;
	mOSG->SetCurrentOperation(Zoom);
}

void CMeshWorkOsgView::OnViewpanBtn() 
{
	m_currentOperation = Pan;
	mOSG->SetCurrentOperation(Pan);
}

void CMeshWorkOsgView::SetViewOperation(ViewOperation op) 
{
	m_currentOperation = op;
	mOSG->SetCurrentOperation(op);
}

void CMeshWorkOsgView::SetViewDirection(int dir)
{
	mOSG->SetLookAt(dir);
	mOSG->Render(mOSG);  
}

//*****************************************************************************
// MODULE:	on Mouse Events
// DESC:	Mouse event handlers go here

// this function change one mouse move into many small movement events
// and create the interactive feeling 
// otherwise, osg will catch one event with the initail (click) and 
// last (release) mouse position
void CMeshWorkOsgView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( nFlags & MK_LBUTTON || mOSG->GetAnnotateMode() == 2)
	{
		// OSG event handler is activated (ie. picking and track ball) only after this 
		// render() function called
		// Picking and track ball will both catch the event, and do different processing
		// for example, update view materix in terms of mouse movements
		// then do the actual rendering
		mOSG->Render(mOSG);        
	}
	
	CView::OnMouseMove(nFlags, point);
}

void CMeshWorkOsgView::OnLButtonDown(UINT nFlags, CPoint point) 
{		
	m_currentPoint	= point;
	CView::OnLButtonDown(nFlags, point);
}

void CMeshWorkOsgView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	
	CSize moved = point - m_currentPoint;
	int movedist = max( abs( moved.cx ), abs( moved.cy ) );
	if (movedist <= 3)
		mOSG->Render(mOSG);

	Invalidate( FALSE );	
	
	CView::OnLButtonUp(nFlags, point);
}

//*****************************************************************************
// MODULE:	UpdateUI
// DESC:	Handles the windows messages related to UI update

void CMeshWorkOsgView::OnUpdateShadedBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is shaded
	( Shaded == m_currentViewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkOsgView::OnUpdateWireframeBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is wireframe
	( Wireframe == m_currentViewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkOsgView::OnUpdateHiddenBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is hidden line
	( Hidden == m_currentViewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkOsgView::OnUpdateSilhouetteBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is silhouette
	( WireAndSils == m_currentViewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkOsgView::OnUpdateDrafthiddenBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is hidden line
	( DraftingHidden == m_currentOperation ) 
				? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);	
}

void CMeshWorkOsgView::OnUpdateViewrotateBtn(CCmdUI* pCmdUI) 
{
	// put check if opertion is rotate
	(Rotate == m_currentOperation) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkOsgView::OnUpdateViewzoomBtn(CCmdUI* pCmdUI) 
{
	// put check if opertion is zoom
	(Zoom == m_currentOperation) ? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkOsgView::OnUpdateViewpanBtn(CCmdUI* pCmdUI) 
{
	// put check if opertion is pan
	(Pan == m_currentOperation) ? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}