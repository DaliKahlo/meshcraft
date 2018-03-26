//$c3 XRL 07/27/12 supported abort button and closing dialog.
//$c2 XRL 07/26/12 added SwitchView in OnThreadFinished; display error code if mesh fails.
//$c1 XRL 06/28/12 created.
//=========================================================================
// ComputeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkView.h"
#include "MeshProgressDlg.h"
#include <afxmt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEvent g_eventStart;
CEvent g_eventKill;

UINT MeshThreadProc(LPVOID pParam)
{
	MeshThreadParam *pProcParam = (MeshThreadParam *)pParam;

	::WaitForSingleObject(g_eventStart,INFINITE);
	TRACE("Start meshing\n");

	//volatile int nTemp;
	//for(pProcParam->count=0; pProcParam->count<MAX_PROGRESS_COUNT; 
	//		::InterlockedIncrement((long*) &(pProcParam->count))) {
	//	for(nTemp=0; nTemp<100000; nTemp++) {
	//	}
	//	if(::WaitForSingleObject(g_eventKill,0) == WAIT_OBJECT_0) {
	//		break;
	//	}
	//}
	CMeshGen* pMG = pProcParam->pMesher;
	pProcParam->error_code =-1;
	if(pMG)
		pProcParam->error_code = pMG->mesh();

	::PostMessage((HWND)(pProcParam->safeHwnd), WM_THREADFINISHED, 0, 0);
	pMG->set_mesh_progress(0);
	return 0;
}


// CComputeDlg dialog

IMPLEMENT_DYNAMIC(CMeshProgressDlg, CDialog)

CMeshProgressDlg::CMeshProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMeshProgressDlg::IDD, pParent)
{
	m_pParent = pParent;
	pProcParam = NULL;
}

CMeshProgressDlg::CMeshProgressDlg(CMeshGen* pGen, CWnd* pParent /*= NULL*/)
	: CDialog(CMeshProgressDlg::IDD, pParent)
{
	m_pParent = pParent;
	pProcParam = new MeshThreadParam;
	pProcParam->error_code = 0;
	pProcParam->pMesher = pGen;
	pProcParam->safeHwnd = NULL; // GetSafeHwnd(); Get NULL since dialog win is not created yet.
}

CMeshProgressDlg::~CMeshProgressDlg()
{
	if(pProcParam->pMesher) 
	{ delete pProcParam->pMesher; pProcParam->pMesher=NULL; }
	if(pProcParam) 
	{ delete pProcParam; }
}

void CMeshProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMeshProgressDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CMeshProgressDlg::OnCancel)
	ON_MESSAGE (WM_THREADFINISHED, OnThreadFinished)
END_MESSAGE_MAP()


// CComputeDlg message handlers

void CMeshProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CMeshGen* pMG = pProcParam->pMesher;
	if(pMG) {
		CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_CTRL);
		pBar->SetPos(pMG->get_mesh_progress() * 100 / MAX_PROGRESS_COUNT);
	}

	//CDialogEx::OnTimer(nIDEvent);
}

// clicked abort button or close the progress window
void CMeshProgressDlg::OnCancel()
{
	// TODO: Add your control notification handler code here
	TRACE("entering CMeshProgressDlg::OnCancel\n");
	CMeshGen* pMG = pProcParam->pMesher;
	if(pMG)
		pMG->set_interrupt_mesh();
	else {
		// kill the window
		TRACE("kill Progress Dialog\n");
		if(pProcParam->pMesher->get_mesh_progress() == 0) {
			g_eventStart.SetEvent();
		}
		g_eventKill.SetEvent();

		DestroyWindow();   // modeless
	}
	TRACE("leavinging CMeshProgressDlg::OnCancel\n");
}

LRESULT CMeshProgressDlg::OnThreadFinished(WPARAM wParam, LPARAM lParam)
{
	int err = pProcParam->error_code;

	OnOK();  // this close the dialog box

	if(err == 0) {
		CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
		if( mwApp->ActiveView() != mwApp->MeshView() )
			mwApp->SwitchView();
	} else if(err == MESH_INTERRUPTED) {
		AfxMessageBox(_T("Meshing interrupted."),MB_OK);
	} else if(err == MESH_PARTIAL_MESHING_ASSEMBLY_NOT_SUPPORT) {
		AfxMessageBox(_T("Does not support meshing a portion of faces in assembly model."),MB_OK);
	} else if(err == MESH_PARTIAL_MESHING_SELECT_FACE) {
		AfxMessageBox(_T("Please pick the face(s) to mesh."),MB_OK);
	} else {
		CString str;
		str.Format(_T("Meshing failed with error code %d."),err);
		AfxMessageBox(str,MB_OK);
	}

	return 0;
}

BOOL CMeshProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	pProcParam->safeHwnd = GetSafeHwnd();
	AfxBeginThread(MeshThreadProc, pProcParam);

	m_nTimer = SetTimer(1,100,NULL);
	ASSERT(m_nTimer != 0);
	g_eventStart.SetEvent();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMeshProgressDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	if( m_pParent ) {
		((CMeshWorkView*)m_pParent)->pMeProgDlg = NULL;		
	}
	delete this;
}

void CMeshProgressDlg::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();    // modeless
}
