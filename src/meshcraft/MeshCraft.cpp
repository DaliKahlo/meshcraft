//$c2 XRL 06/07/13 Add m_strMyClassName and InitApplication() to remove flickers
//$c1 XRL 02/01/11 created.
//=========================================================================
//
// MeshWorks.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <afxpriv.h>	// added 5/21/2012 for multi-view

#include "MeshWorks.h"
#include "MainFrm.h"
#include "MeshWorkDoc.h"
#include "MeshWorkView.h"
#include "MeshWorkOsgView.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshWorks

BEGIN_MESSAGE_MAP(CMeshWorks, CWinApp)
	//{{AFX_MSG_MAP(CMeshWorks)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)

	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_VIEW_SWITCH, &CMeshWorks::OnViewSwitch)
	ON_COMMAND(ID_VIEW_FRONT, &CMeshWorks::OnViewFront)
	ON_COMMAND(ID_VIEW_TOP, &CMeshWorks::OnViewTop)
	ON_COMMAND(ID_VIEW_LEFT, &CMeshWorks::OnViewLeft)
	ON_COMMAND(ID_VIEW_PERSPECTIVE, &CMeshWorks::OnViewPerspective)
	ON_COMMAND(ID_SELECT_POINT, &CMeshWorks::OnSelectPointMode)
	ON_COMMAND(ID_SELECT_MEDGE, &CMeshWorks::OnSelectMEdgeMode)
	ON_COMMAND(ID_SELECT_MFACE, &CMeshWorks::OnSelectMFaceMode)
	ON_COMMAND(ID_SELECT_CLEAR, &CMeshWorks::OnSelectClear)
	ON_COMMAND(ID_SELECT_CURRENTPART, &CMeshWorks::OnSelectCurrentPart)
	ON_COMMAND(ID_FROMLIBRARY_ARROW, &CMeshWorks::OnToolsGeoAddArrow)
	ON_COMMAND(ID_FROMLIBRARY_L, &CMeshWorks::OnToolsGeoAddLShape)
	ON_COMMAND(ID_FROMLIBRARY_CYLINDERBOX, &CMeshWorks::OnToolsGeoAddCylBox)
	ON_COMMAND(ID_GEO_INSERT_SPHERE, &CMeshWorks::OnToolsGeoInsertSphere)
	ON_COMMAND(ID_TOOLS_CREATEMESH, &CMeshWorks::OnToolsCreateMesh)
	ON_COMMAND(ID_VIBRO_FILLHOLES, &CMeshWorks::OnToolsFillHoleMesh)
	ON_COMMAND(ID_VIBRO_SHRINKWRAP, &CMeshWorks::OnToolsWrapMesh)
	ON_COMMAND(ID_VIBRO_TETRA, &CMeshWorks::OnToolsTetrahedralize)
	ON_COMMAND(ID_VIBRO_CONVEX, &CMeshWorks::OnToolsConvexHull)
	ON_COMMAND(ID_QUERYMESH_GENERAL, &CMeshWorks::OnToolsQueryGeneral)
	ON_COMMAND(ID_QUERYMESH_HIGHLIGHT, &CMeshWorks::OnToolsQueryHighlight)
	ON_COMMAND(ID_POINTCLOUD_SEGMENT, &CMeshWorks::OnToolsMetrologySegment)
	ON_COMMAND(ID_METROLOGY_CIRCLE, &CMeshWorks::OnToolsMetrologyCircle)
	ON_COMMAND(ID_METROLOGY_SPHERE, &CMeshWorks::OnToolsMetrologySphere)
	ON_COMMAND(ID_POINTCLOUD_DELETE, &CMeshWorks::OnToolsMetrologyDelete)
	ON_COMMAND(ID_POINTCLOUD_ROTATE, &CMeshWorks::OnToolsMetrologyRotate)
	ON_COMMAND(ID_POINTCLOUD_TRANSLATE, &CMeshWorks::OnToolsMetrologyTranslate)
	ON_COMMAND(ID_POINTCLOUD_AUTOALIGN, &CMeshWorks::OnToolsMetrologyICP)
	ON_COMMAND(ID_POINTCLOUD_COMPUTERR, &CMeshWorks::OnToolsMetrologyERR)
	ON_COMMAND(ID_POINTCLOUD_LINEPLOT, &CMeshWorks::OnToolsMetrologyLinePlot)
	ON_COMMAND(ID_POINTCLOUD_COLORMAP, &CMeshWorks::OnToolsMetrologyColorMap)
	ON_COMMAND(ID_ANNOTATION_ADD, &CMeshWorks::OnToolsAnnotationAdd)
	ON_COMMAND(ID_ANNOTATION_MOVE, &CMeshWorks::OnToolsAnnotationMove)
	ON_COMMAND(ID_ANNOTATION_DELETE, &CMeshWorks::OnToolsAnnotationDelete)
	ON_COMMAND(ID_TOOLS_OPTIONS, &CMeshWorks::OnToolsOptions)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMeshWorks construction 

CMeshWorks::CMeshWorks()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMeshWorks object

CMeshWorks mwApp;

/////////////////////////////////////////////////////////////////////////////
// CMeshWorks initialization

BOOL CMeshWorks::InitInstance()
{
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("MeshWorks"));

	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;

	pDocTemplate = new CSingleDocTemplate(
		IDR_TOOLBAR1,
		RUNTIME_CLASS(CMeshWorkDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMeshWorkView));

	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;  //XRL

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	//// ************ added 5/21/2012 for multiViews *************
	m_pOldView = ((CFrameWnd*) m_pMainWnd)->GetActiveView();
	m_pNewView = (CView*) new CMeshWorkOsgView;

	CDocument* pCurrentDoc = ((CFrameWnd*)m_pMainWnd)->GetActiveDocument();

	// Initialize a CCreateContext to point to the active document.
	// With this context, the new view is added to the document
	// when the view is created in CView::OnCreate().
	CCreateContext newContext;
	newContext.m_pNewViewClass = NULL;
	newContext.m_pNewDocTemplate = NULL;
	newContext.m_pLastView = NULL;
	newContext.m_pCurrentFrame = NULL;
	newContext.m_pCurrentDoc = pCurrentDoc;

	// The ID of the initial active view is AFX_IDW_PANE_FIRST.
	// Incrementing this value by one for additional views works
	// in the standard document/view case but the technique cannot
	// be extended for the CSplitterWnd case.
	UINT viewID = AFX_IDW_PANE_FIRST + 1;
	CRect rect(0, 0, 0, 0); // remember to resize later.

	// Create the new view. In this example, the view persists for
	// the life of the application. The application automatically
	// deletes the view when the application is closed.
	m_pNewView->Create(NULL, _T("MeshView"), WS_CHILD, rect, m_pMainWnd, viewID, &newContext);

	// When a document template creates a view, the WM_INITIALUPDATE
	// message is sent automatically. However, this code must
	// explicitly send the message, as follows.
	//m_pNewView->SendMessage(WM_INITIALUPDATE, 0, 0);
	m_pNewView->OnInitialUpdate();

	m_pNewView->ShowWindow(SW_HIDE);
	// ************ end of 5/21/2012 addition for multiview ************
	
	return TRUE;
}


// http://support.microsoft.com/kb/177352
BOOL CMeshWorks::InitApplication()
{
   // Register our own class with the same attributes as AfxFrameOrView"
   // refer to MFC Tech Note 1: Window Class Registration for more
   // information.
    m_strMyClassName = AfxRegisterWndClass (0, ::LoadCursor (NULL,
                    IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1),
                    LoadIcon(AFX_IDI_STD_FRAME));

    return CWinApp::InitApplication();
}


CView* CMeshWorks::SwitchView( )
{
   CMeshWorkOsgView* osgv = (CMeshWorkOsgView *)MeshView();
   //osgv->SetEraseBkgnd(true);

   CView* pActiveView = 
      ((CFrameWnd*) m_pMainWnd)->GetActiveView();

   CView* pNewView= NULL;
   if(pActiveView == m_pOldView)
      pNewView= m_pNewView;
   else
      pNewView= m_pOldView;

   // Exchange view window IDs so RecalcLayout() works.
   #ifndef _WIN32
   UINT temp = ::GetWindowWord(pActiveView->m_hWnd, GWW_ID);
   ::SetWindowWord(pActiveView->m_hWnd, GWW_ID, ::GetWindowWord(pNewView->m_hWnd, GWW_ID));
   ::SetWindowWord(pNewView->m_hWnd, GWW_ID, temp);
   #else
   UINT temp = ::GetWindowLong(pActiveView->m_hWnd, GWL_ID);
   ::SetWindowLong(pActiveView->m_hWnd, GWL_ID, ::GetWindowLong(pNewView->m_hWnd, GWL_ID));
   ::SetWindowLong(pNewView->m_hWnd, GWL_ID, temp);
   #endif

   pActiveView->ShowWindow(SW_HIDE);
   pNewView->ShowWindow(SW_SHOW);
   ((CFrameWnd*) m_pMainWnd)->SetActiveView(pNewView,TRUE);
   ((CFrameWnd*) m_pMainWnd)->RecalcLayout();
   pNewView->Invalidate();
  
   return pActiveView;
} 

CView* CMeshWorks::ActiveView()
{ return ((CFrameWnd*) m_pMainWnd)->GetActiveView(); }

CView* CMeshWorks::MeshView()
{ return m_pNewView; }

CView* CMeshWorks::GeometryView()
{ return m_pOldView; }

void CMeshWorks::SetStatusBarString(const CString &string)
{
	CMainFrame* wnd = (CMainFrame*)AfxGetMainWnd();
	if ( wnd )
	{
		wnd->SetMessageText( string );
	}

}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}



// App command to run the dialog
void CMeshWorks::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}




void CMeshWorks::OnViewSwitch()
{
	// TODO: Add your command handler code here
	SwitchView();
}


int CMeshWorks::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class

	return CWinApp::ExitInstance();
}
