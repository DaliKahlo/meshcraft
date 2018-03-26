//$c6   XRL 02/20/2014 added SetViewOperation
//$c5   XRL 08/01/2012 added m_eraseBkgnd
//$c4   XRL 07/31/2012 added ChangePointSize and ChangeTransparency
//$c3   XRL 07/10/2012 added m_currentOperation and m_currentViewStyle
//$c2   XRL 05/25/2012 added mOSG.
//$c1   XRL 02/03/2012 Created.
//=========================================================================
//
// MeshWorkOsgView.h : interface of the CMeshWorkOsgView class
//
//=========================================================================

#pragma once

#ifndef MESHWORKAPPOSGVIEW_H__CD273751_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHWORKAPPOSGVIEW_H__CD273751_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "osg/MFC_OSG.h"

class CMeshWorkOsgView : public CView
{
protected: 
	DECLARE_DYNCREATE(CMeshWorkOsgView)

public:
	CMeshWorkOsgView();           // protected constructor used by dynamic creation
	virtual ~CMeshWorkOsgView();

	CMeshWorkDoc* GetDocument();

	osg::ref_ptr<osg::Node> GetSceneModel(int);
	void SetSceneData(int, bool);
	void SetGeodeNode(osg::ref_ptr<osg::Geode>);
	void SetSelectionMode(int);
	void SetAnnotationMode(int);
	void SetViewOperation(ViewOperation);
	void SetViewDirection(int);
	void ChangePointSize(double, bool);
	void ChangeTransparency(double, bool);
	void ChangeClipPlane(int iPlane, int pos);
	void ChangeBackground(int, int);
	void ChangeInteriorLightStatus(int);
	//void SetEraseBkgnd(bool b) { m_eraseBkgnd=b;}

	void HighLight(int, double[4][3]);
	void HighLightP(double[3], double);
	void HudText(int, std::string&, bool);
	void ClearHighLight();
	void ClearHUDText();
	void ClearAllSelection();

	int Segment(double xyz[64][3]);
	int ThreePointCircle(double xyz[3][3]);
	int FourPointSphere(double xyz[10][3]);

	void LinePlot(int, double*, double*, double, double);
	void ColorMap(int, int, double *, int *, double *, double, double);
	void PostMult(osg::Matrix&, int);

	int ShowColorMap();
	int ShowLinePlot();

	// Overrides
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView*, LPARAM, CObject*);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
    cOSG* mOSG;
	ViewStyle	m_currentViewStyle;
	ViewOperation m_currentOperation;
	//bool m_eraseBkgnd;
	CPoint m_currentPoint;		// used to capture mouse click 9/20/2012

	void SetViewStyle(ViewStyle);

protected:
	//{{AFX_MSG(CMeshWorkOsgView)
	afx_msg void OnShadedBtn();
	afx_msg void OnWireframeBtn();
	afx_msg void OnSilhouetteBtn();
	afx_msg void OnHiddenBtn();
	afx_msg void OnDraftHiddenBtn();
	afx_msg void OnViewrotateBtn();
	afx_msg void OnViewzoomBtn();
	afx_msg void OnViewpanBtn();
	afx_msg void OnUpdateShadedBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWireframeBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHiddenBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSilhouetteBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDrafthiddenBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewrotateBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewzoomBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewpanBtn(CCmdUI* pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

public:
	afx_msg void OnMeshBtn();
};

#ifndef _DEBUG  // debug version in enter AppView.
inline CMeshWorkDoc* CMeshWorkOsgView::GetDocument()
   { return (CMeshWorkDoc*)m_pDocument; }
#endif


#endif 