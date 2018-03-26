#pragma once

#include "resource.h"
#include "MeshWorkDoc.h"

// WrapMeshDlg dialog

class CWrapMeshDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWrapMeshDlg)

public:
	CWrapMeshDlg(pAssemblyMesh, double, double, CWnd* pParent = NULL);   // standard constructor
	virtual ~CWrapMeshDlg();

// Dialog Data
	enum { IDD = IDD_WRAP_MESH_DIALOG };

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
	CEdit m_editWrapMeshSize;		// edit control
	CString m_sWrapMeshSizeWithUnit;
	double m_dWrapMeshSize;

	CEdit m_editDistToMesh;		
	CString m_sDistToMeshWithUnit;
	double m_dDistToMesh;

	CButton m_buttonCubeOnlyFlag;// check box button
	int m_bCubeOnlyFlag;

	// curvature related 
	CButton m_buttonDeleteOriMeshFlag;	// check box button
	int m_bDeleteOriMeshFlag;

protected:
	pAssemblyMesh m_pAsmMesh;

public:
	afx_msg void OnBnClickedCubeOnlyFlag();
	afx_msg void OnBnClickedDeleteOriMeshFlag();
};
