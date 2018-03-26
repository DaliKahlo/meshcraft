//$c5 XRL 08/01/12 added m_eraseBkgnd.
//$c4 XRL 07/04/12 added pMeProgDlg.
//$c3 XRL 12/21/11 added m_transfList, and modified rerender() to support assembly.
//$c2 XRL 02/03/11 Support pick Topologies.
//$c1 XRL 02/01/11 Created.
//=========================================================================
//
// MeshWorksView.h : interface of the CMeshWorkView class
//
//=========================================================================

#ifndef MESHWORKVIEW_H__CD273751_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHWORKVIEW_H__CD273751_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include <vector>
#include "gl.h"
#include "glu.h"
#include "parasolid_kernel.h"
#include "frustrum_tokens.h"
#include "frustrum_ifails.h"
#include "dialog\MeshProgressDlg.h"

using namespace std;

class CMeshWorkView : public CView
{
protected: // create from serialization only
	CMeshWorkView();
	DECLARE_DYNCREATE(CMeshWorkView)

// Operations
public:
	
	CMeshWorkDoc* GetDocument();
	void InitOpenGL();
	BOOL MakeCurrent( BOOL setCurrent = TRUE );
	void FitPartsInView();
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint); 
	void ReRender( BOOL updateAll = TRUE );
	void PanView( CPoint point );
	void ReSize(UINT nType, int cx, int cy);
	void SetViewVolume();
	void ZoomView( CPoint point );
	void RotateView( CPoint point );
	void NotifyViewDirectionChanged();
	void SetViewStyle( ViewStyle style );
	PK_TOPOL_t PickTopol(CPoint point);
	bool PickParts(int &nParts, PK_PART_t *&pParts, PK_TRANSF_t *pTransf, PK_AXIS1_sf_t &ray, PK_BODY_pick_topols_o_t options, PK_ENTITY_t *picked, int *occurence );
	InstancedTag PickBodies(int nParts, PK_PART_t *pParts, PK_TRANSF_t *pTransf, PK_AXIS1_sf_t &ray, PK_BODY_pick_topols_o_t &options );
	virtual ~CMeshWorkView();

// Variables

public:
	CMeshProgressDlg *pMeProgDlg;

private:
	int m_winHeight;
	int m_winWidth;

	ViewOperation m_currentOperation;
	CPoint m_lastPoint;

	BOOL m_hasRotated;
	CPoint m_currentPoint;
	
	CClientDC* m_pDC;
	HGLRC m_glContext;
	CPalette	m_Palette;
	CPalette*	m_pOldPalette;
	//bool	m_eraseBkgnd;							// added 8/1/2012

	GLuint	m_partDisplaylist;

	CList< PK_PART_t, PK_PART_t > m_partList;
	CList< PK_TRANSF_t, PK_TRANSF_t > m_transfList;
	CList< PK_GEOM_t, PK_GEOM_t > m_geomList;

	BOOL m_updateNeeded;
	BOOL m_firstDraw;
	GLfloat m_defaultFacetColour[ 4 ];

	PK_TOPOL_render_line_o_t	m_lineOptions;
	PK_TOPOL_render_facet_o_t	m_facetOptions;
	PK_GEOM_render_line_o_t		m_geomlineOptions;

	// Module: Initialize
	BOOL SetupPixelFormat();
	void CreateRGBPalette();
	unsigned char ComponentFromIndex( int i, UINT nbits, UINT shift );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshWorkView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL FixUpMatrix( PK_TRANSF_sf_t &vtsf );
	GLfloat* GetDefaultFacetColour();
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	
	void SetLighting( ViewStyle style );

// Generated message map functions
protected:
	
	//{{AFX_MSG(CMeshWorkView)
	afx_msg void OnShadedBtn();
	afx_msg void OnWireframeBtn();
	afx_msg void OnSilhouetteBtn();
	afx_msg void OnHiddenBtn();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnViewrotateBtn();
	afx_msg void OnViewzoomBtn();
	afx_msg void OnUpdateViewrotateBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShadedBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHiddenBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSilhouetteBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWireframeBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewzoomBtn(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewpanBtn();
	afx_msg void OnUpdateViewpanBtn(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDraftHiddenBtn();
	afx_msg void OnUpdateDrafthiddenBtn(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnMeshBtn();
};

#ifndef _DEBUG  // debug version in enter AppView.
inline CMeshWorkDoc* CMeshWorkView::GetDocument()
   { return (CMeshWorkDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXAMPLEAPPVIEW_H__CD273751_D581_11D2_8BF9_0000F8071DC8__INCLUDED_)
