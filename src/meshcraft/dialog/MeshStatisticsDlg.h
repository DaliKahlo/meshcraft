#pragma once

#include "resource.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"

// CMeshStaticsDlg dialog

class CMeshStaticsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMeshStaticsDlg)

public:
	CMeshStaticsDlg(pAssemblyMesh, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMeshStaticsDlg();

// Dialog Data
	enum { IDD = IDD_MESH_STATISTICS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	pAssemblyMesh m_pAsmMesh;
public:
	CString m_numMeVertices;
	CString m_numMeFaces;
	CString m_numMeRegions;
};
