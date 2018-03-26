//$c1   XRL 12/08/2012 Created 
//========================================================================//
//              Copyright 2009 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#pragma once

#include "stdafx.h"
#include "resource.h"
#include "MeshWorkDoc.h"

// MeshHighlight dialog

//class MeshHighlight : public CDialogEx
class CMeshHighlightDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMeshHighlightDlg)

public:
	CMeshHighlightDlg(pAssemblyMesh, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMeshHighlightDlg();

// Dialog Data
	enum { IDD = IDD_MESH_HIGHLIGHT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//{{AFX_MSG(CMeshHighlightDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


//--------------------------highlight mesh specific----------------
//
public:
	CEdit m_editVertexIDs;		// edit control
	CString m_sVertexIDs;

	CEdit m_editShapeThreshold;		// edit control
	CString m_sShapeThreshold;

protected:
	bool Validate(int *, int **, double *);
	void highlight(int, int *, double);

	pAssemblyMesh m_pAsmMesh;

};
