//$c2 XRL 03/29/13 mesh selected faces only.
//$c1 XRL 01/04/12 created.
//=========================================================================
//
// CreateMeshDlg.h
//
//=========================================================================
#if !defined(CREATEMESHDLG_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_)
#define CREATEMESHDLG_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "stdafx.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateMeshDlg dialog

class CCreateMeshDlg : public CDialog
{
// Construction
public:
	CCreateMeshDlg(CWnd* pParent = NULL);   // standard constructor
	CCreateMeshDlg(double, double, bool, double, double, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CCreateMeshDlg)
	enum { IDD = IDD_CREATE_MESH_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateMeshDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

//--------------------------createmesh specific----------------
//
public:
	double m_dDefaultElementSize;

	CEdit m_editElementSize;		// edit control
	CString m_sElementSizeWithUnit;
	double m_dElementSize;

	CEdit m_editMeshGradation;		// edit control
	double m_dMeshGradation;

	CComboBox m_comboElementType;	// combobox control
	int m_iElemTypeIndex;			// 0 quad; 1 tet; 2 tri

	CButton m_buttonPartialMeshFlag;// check box button
	int m_bPartialMeshFlag;

	// curvature related 
	CButton m_buttonCurvatureFlag;	// check box button
	int m_bCurvatureFlag;

	CComboBox m_comboElementPerCircle; // combobox control
	int m_iElementPerCircleIndex;

	CEdit m_editMinElementSize;		// edit control
	CString m_sMinElementSizeWithUnit;
	double m_dMinElementSize;

	double getIndexAngle(int i);

protected:
	bool Validate(bool bShowMessage);
	int getAngleIndex(double angle);
//
//--------------------------createmesh specific----------------

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateMeshDlg)
		// NOTE: the ClassWizard will add member functions here
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCurvatureFlag();
	afx_msg void OnBnClickedSelectedFaceFlag();
};

#endif