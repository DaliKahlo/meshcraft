//$c2 XRL 07/30/12 added point size and transparency slider bar.
//$c1 XRL 01/04/12 created.
//=========================================================================
//
// CSystemOptionDlg.h
//
//=========================================================================

#ifndef SYSOPTIONDLG_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define SYSOPTIONDLG_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "stdafx.h"
#include "resource.h"
#include "MeshWorkDefs.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CSystemOptionDlg dialog

class CSystemOptionDlg : public CDialog
{
// Construction
public:
	CSystemOptionDlg(CWnd* pParent = NULL);   // standard constructor
	CSystemOptionDlg(SceneStyle, int, int, int, double, double, int, int, int, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CSystemOptionDlg)
	enum { IDD = IDD_SYSTEM_OPTION_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemOptionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

//--------------------------system option specific----------------
//
public:

	// scene graph options

	// render mode control
	CComboBox m_comboRenderMode;	// combobox control
	SceneStyle m_iRenderMode;	

	CComboBox m_comboBackgroundColor;	// combobox control
	int m_iBKColor;
	//float m_bkColorR, m_bkColorG, m_bkColorB;	

	// clip plane control
	CComboBox m_comboClipPlane;			// combobox control
	CSliderCtrl m_clipPlaneSliderBar;	// slider control
	int m_iPlane;						// 0 none, 1 yz plane, 2 xz plane, 3 xy plane
	int m_planePosition;				// 0 -> 100

	// point size control
	CSliderCtrl m_PointSizeSliderBar;		// slider control
	double m_pointSize;

	// transparency control
	CSliderCtrl m_TransparencySliderBar;	// slider control
	double m_transparency;

	CButton m_buttonOpposingLightFlag;		// check box button
	int m_bOpposingLightFlag;

	// meshing options
	CComboBox m_comboIntersetCorrect;		// combobox control
	int m_iIntersectCorrect;	

	CComboBox m_comboDefaultSizeCompute;	// combobox control
	int m_iDefaultSizeCompute;

private:
	CString m_ptSizeSliderEcho;
	CString m_TransparencySliderEcho;

protected:
	bool Validate();
//
//--------------------------system option specific----------------

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemOptionDlg)
		// NOTE: the ClassWizard will add member functions here
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedOpposingLightFlag();
};

#endif