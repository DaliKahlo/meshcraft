#pragma once

#include "resource.h"
#include "MeshWorkDoc.h"

// FillHoleDlg dialog

class CFillHoleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFillHoleDlg)

public:
	CFillHoleDlg(pAssemblyMesh, double, double, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFillHoleDlg();

// Dialog Data
	enum { IDD = IDD_FILL_HOLE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//{{AFX_MSG(CWrapMeshDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

//--------------------------wrapmesh specific----------------
protected:
	bool Validate();

public:
	CEdit m_editFeatureAngle;		
	CString m_dFeatureAngleStr;
	double m_dFeatureAngle;

	CEdit m_editFillHoleLessThan;		
	CString m_sFillHoleLessThanWithUnit;
	double m_dFillHoleLessThan;

protected:
	pAssemblyMesh m_pAsmMesh;
};
