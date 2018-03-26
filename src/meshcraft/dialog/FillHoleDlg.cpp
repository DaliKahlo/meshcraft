// WrapMeshDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "util.h"
#include "MeshWorks.h"
#include "resource.h"
#include "FillHoleDlg.h"

// WrapMeshDlg dialog

IMPLEMENT_DYNAMIC(CFillHoleDlg, CDialogEx)

CFillHoleDlg::CFillHoleDlg(pAssemblyMesh pAsmMe, 
						   double featureAngle0,
						   double diameter0,
                           CWnd* pParent /*=NULL*/)
	: CDialogEx(CFillHoleDlg::IDD, pParent)
	, m_pAsmMesh(pAsmMe)
{
	// the unit of length is always in "meter"
	m_dFeatureAngle = featureAngle0;
	m_dFillHoleLessThan = diameter0;

	m_dFeatureAngleStr.Format(_T("%6.1f"), m_dFeatureAngle);
	m_sFillHoleLessThanWithUnit.Format(_T("%8.4f m"), m_dFillHoleLessThan);
}

CFillHoleDlg::~CFillHoleDlg()
{
}

void CFillHoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWrapMeshDlg)
	DDX_Text(pDX, IDC_FILL_HOLE_EDIT, m_sFillHoleLessThanWithUnit);
	DDX_Text(pDX, IDC_FEATURE_ANGLE_EDIT, m_dFeatureAngleStr);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFillHoleDlg, CDialogEx)
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CFillHoleDlg message handlers

BOOL CFillHoleDlg::OnInitDialog()
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFillHoleDlg::OnOK() 
{
	// TODO: Add extra validation here

	bool bValid = Validate();
	if( !bValid )
		return;		// can't close the dialog
	UpdateData();
	CDialog::OnOK();
}

void CFillHoleDlg::OnCancel() 
{
	UpdateData();

	CDialog::OnCancel();
}

bool CFillHoleDlg::Validate()
{
	UpdateData();

	CString message;
	m_dFeatureAngle = _ttof(m_dFeatureAngleStr);
	if(m_dFeatureAngle <= 1.0e-14) {
		message.LoadString(IDS_INVALID_FEATURE_ANGLE);
		AfxMessageBox(message);
		return false;
	}
	if( !str2floatInMeter(m_sFillHoleLessThanWithUnit, &m_dFillHoleLessThan) )
	{
		message.LoadString(IDS_INVALID_MAX_HOLE_DIAMETER);
		AfxMessageBox(message);
		return false;
	}
	return true;
}