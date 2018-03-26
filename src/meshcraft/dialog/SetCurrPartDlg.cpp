// SetCurrPart.cpp : implementation file
//

#include "stdafx.h"
#include "SetCurrPartDlg.h"
#include "afxdialogex.h"


// CSetCurrPart dialog

IMPLEMENT_DYNAMIC(CSetCurrPartDlg, CDialogEx)

CSetCurrPartDlg::CSetCurrPartDlg(pAssemblyMeshMgr pMgr, 
                                 CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetCurrPartDlg::IDD, pParent)
{
	pAsmMgr = pMgr;
}

CSetCurrPartDlg::~CSetCurrPartDlg()
{
}

void CSetCurrPartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CURRENT_PART_LISTBOX, m_curPartListBox);
}


BEGIN_MESSAGE_MAP(CSetCurrPartDlg, CDialogEx)
END_MESSAGE_MAP()


// CSetCurrPartDlg message handlers

BOOL CSetCurrPartDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// define the initial position of the dialog
	CRect rectFrame, rectDlg;
    CWnd* pMainWnd = AfxGetMainWnd();
    if(pMainWnd != NULL)
    {
		pMainWnd->GetClientRect(rectFrame);
        pMainWnd->ClientToScreen(rectFrame);
        GetWindowRect(rectDlg);

		int nXPos = rectFrame.right - rectDlg.Width();
        int nYPos = rectFrame.top;

        ::SetWindowPos(m_hWnd, HWND_TOP, nXPos, nYPos, 
          rectDlg.Width(), rectDlg.Height(), SWP_NOCOPYBITS);
	}

	UpdateData(FALSE);

	CString str;
	char *name;
	int id, curr, np = pAsmMgr->size();
	if(np > 16) np = 16;
	for(int i=0; i<np; i++) {
		id = pAsmMgr->ith(i)->getID();
		name = pAsmMgr->ith(i)->getName();
		str.Format(_T("%d_%s"),id,name);
		//sprintf(str,_T("%d_%s"),id,name);
		m_curPartListBox.AddString(str);
	}

	curr = pAsmMgr->current();
	m_curPartListBox.SetCurSel(curr);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSetCurrPartDlg::OnOK() 
{
	// TODO: Add extra validation here

	int c = m_curPartListBox.GetCurSel();
	pAsmMgr->setCurrent(c);

	UpdateData();
	CDialog::OnOK();
}
