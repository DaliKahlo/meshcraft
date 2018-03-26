#pragma once
#include "resource.h"
#include "afxcmn.h"

#include "meshing/AssemblyMeshMgr.h"
#include "afxwin.h"

// CSetCurrPart dialog

class CSetCurrPartDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetCurrPartDlg)

public:
	CSetCurrPartDlg(pAssemblyMeshMgr, CWnd* pParent = NULL);   
	virtual ~CSetCurrPartDlg();

// Dialog Data
	enum { IDD = IDD_SET_CURRENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	pAssemblyMeshMgr pAsmMgr;
	CListBox m_curPartListBox;
};
