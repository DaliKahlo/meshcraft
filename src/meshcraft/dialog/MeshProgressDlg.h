//$c1 XRL 07/02/12 created.
//=========================================================================
//
// Modeless CMeshProgress dialog
//
//=========================================================================
#ifndef MESHPROGRESSDLG_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHPROGRESSDLG_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "resource.h"
#include "meshing\MeshGen.h"


typedef struct {
	HWND safeHwnd;
	CMeshGen *pMesher;
	int error_code;			// >=0; no_errors=0
} MeshThreadParam;


#define WM_THREADFINISHED	WM_USER + 5
UINT MeshThreadProc(LPVOID pParam);

class CMeshProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CMeshProgressDlg)

public:
	CMeshProgressDlg(CWnd* pParent = NULL); 
	CMeshProgressDlg(CMeshGen *, CWnd* pParent = NULL); 
	virtual ~CMeshProgressDlg();

// Dialog Data
	enum { IDD = IDD_MESH_PROGRESS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();  // modeless

	DECLARE_MESSAGE_MAP()

private:
	int m_nTimer;
	MeshThreadParam* pProcParam;
public:
	CWnd* m_pParent;	// modeless

public:
//	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCancel();
	afx_msg LRESULT OnThreadFinished(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};

#endif