//$c3   XRL 06/07/2013 Add m_strMyClassName and InitApplication() to remove flickers
//$c2   XRL 05/25/2012 Add osg based MeshView
//$c1   XRL 02/01/2011 Created 
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#ifndef MESHWORKS_H__CD273749_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHWORKS_H__CD273749_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"     
#include "parasolid/ParaSession4.h"
#include "SystemOption.h"

/////////////////////////////////////////////////////////////////////////////
// Declear the CMeshWorks application
//
// See MeshWorks.cpp and MeshWorkTools.cpp for the implementation of this class
//

class CMeshWorks : public CWinApp
{
private:
	// This creates an instance of m_session which in turn sets and
	// starts up Parasolid. For further details see CSession
	CSession_c m_session;
	CSystemOption m_options;

	// added 5/21/2012 to support multi-views
	CView* m_pOldView;
	CView* m_pNewView;

public:
	// added 6/4/2013 to remove model flicker due to blue screen
	CString m_strMyClassName;		

	CView* SwitchView( );
	CView* MeshView( );
	CView* GeometryView( );
	CView* ActiveView();

	void SetStatusBarString(const CString &string);
	
	CMeshWorks();
	CSystemOption *sysOption() { return &m_options; }
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshWorks)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL InitApplication();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CMeshWorks)
	afx_msg void OnViewSwitch();
	afx_msg void OnViewFront();
	afx_msg void OnViewTop();
	afx_msg void OnViewLeft();
	afx_msg void OnViewPerspective();
	afx_msg void OnSelectPointMode();
	afx_msg void OnSelectMEdgeMode();
	afx_msg void OnSelectMFaceMode();
	afx_msg void OnSelectClear();
	afx_msg void OnSelectCurrentPart();
	afx_msg void OnToolsGeoAddArrow();
	afx_msg void OnToolsGeoAddLShape();
	afx_msg void OnToolsGeoAddCylBox();
	afx_msg void OnToolsGeoInsertSphere();
	afx_msg void OnToolsCreateMesh();
	afx_msg void OnToolsFillHoleMesh();
	afx_msg void OnToolsWrapMesh();
	afx_msg void OnToolsQueryGeneral();
	afx_msg void OnToolsTetrahedralize();
	afx_msg void OnToolsConvexHull();
	afx_msg void OnToolsQueryHighlight();
	afx_msg void OnToolsMetrologySegment();
	afx_msg void OnToolsMetrologyCircle();
	afx_msg void OnToolsMetrologySphere();
	afx_msg void OnToolsMetrologyDelete();
	afx_msg void OnToolsMetrologyRotate();
	afx_msg void OnToolsMetrologyTranslate();
	afx_msg void OnToolsMetrologyICP();
	afx_msg void OnToolsMetrologyERR();
	afx_msg void OnToolsMetrologyLinePlot();
	afx_msg void OnToolsMetrologyColorMap();
	afx_msg void OnToolsAnnotationAdd();
	afx_msg void OnToolsAnnotationMove();
	afx_msg void OnToolsAnnotationDelete();
	afx_msg void OnToolsOptions();
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXAMPLEAPP_H__CD273749_D581_11D2_8BF9_0000F8071DC8__INCLUDED_)
