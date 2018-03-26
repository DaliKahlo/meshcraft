// WrapMeshDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "util.h"
#include "MeshWorks.h"
#include "resource.h"
#include "WrapMeshDlg.h"

// WrapMeshDlg dialog

IMPLEMENT_DYNAMIC(CWrapMeshDlg, CDialogEx)

CWrapMeshDlg::CWrapMeshDlg(pAssemblyMesh pAsmMe, 
                           double wrapsize0,
						   double distToMesh0,
                           CWnd* pParent /*=NULL*/)
	: CDialogEx(CWrapMeshDlg::IDD, pParent)
	, m_pAsmMesh(pAsmMe)
{
	// the unit of length is always in "meter"
	m_dWrapMeshSize = wrapsize0;
	m_dDistToMesh = distToMesh0;

	m_sWrapMeshSizeWithUnit.Format(_T("%6.4f m"), m_dWrapMeshSize);
	m_sDistToMeshWithUnit.Format(_T("%6.4f m"), m_dDistToMesh);

	m_bCubeOnlyFlag = 0;		// project cubes
	m_bDeleteOriMeshFlag = 1;	// delete original mesh
}

CWrapMeshDlg::~CWrapMeshDlg()
{
}

void CWrapMeshDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWrapMeshDlg)
	DDX_Text(pDX, IDC_WRAP_MESH_SIZE_EDIT, m_sWrapMeshSizeWithUnit);
	DDX_Text(pDX, IDC_DIST_TO_MESH_EDIT, m_sDistToMeshWithUnit);
	DDX_Control(pDX, IDC_CUBE_ONLY_CHECK, m_buttonCubeOnlyFlag);
	DDX_Control(pDX, IDC_DELETE_ORI_MESH_CHECK, m_buttonDeleteOriMeshFlag);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWrapMeshDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CUBE_ONLY_CHECK, &CWrapMeshDlg::OnBnClickedCubeOnlyFlag)
	ON_BN_CLICKED(IDC_DELETE_ORI_MESH_CHECK, &CWrapMeshDlg::OnBnClickedDeleteOriMeshFlag)
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CWrapMeshDlg message handlers

BOOL CWrapMeshDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// initialize data of this dialog
	//m_editWrapMeshSize.SetWindowText(m_sWrapMeshSizeWithUnit);
	//m_editDistToMesh.SetWindowText(m_sDistToMeshWithUnit);
	//m_editFillHoleLessThan.SetWindowText(m_sFillHoleLessThanWithUnit);
	//m_editFeatureAngle.SetWindowText(m_dFeatureAngleStr);
	m_buttonCubeOnlyFlag.SetCheck(m_bCubeOnlyFlag);
	m_buttonDeleteOriMeshFlag.SetCheck(m_bDeleteOriMeshFlag);

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

void CWrapMeshDlg::OnOK() 
{
	// TODO: Add extra validation here

	bool bValid = Validate();
	if( !bValid )
		return;		// can't close the dialog
	UpdateData();
	CDialog::OnOK();
}

void CWrapMeshDlg::OnCancel() 
{
	UpdateData();

	CDialog::OnCancel();
}

bool CWrapMeshDlg::Validate()
{
	UpdateData();

	CString message;
	if( !str2floatInMeter(m_sWrapMeshSizeWithUnit, &m_dWrapMeshSize) )
	{
		message.LoadString(IDS_INVALID_CHAR_IN_ELEMENT_SIZE);
		AfxMessageBox(message);
		return false;
	}
	if( !str2floatInMeter(m_sDistToMeshWithUnit, &m_dDistToMesh) )
	{
		message.LoadString(IDS_INVALID_DISTANCE_TO_ORIGINAL_MESH);
		AfxMessageBox(message);
		return false;
	}
	return true;
}

void CWrapMeshDlg::OnBnClickedCubeOnlyFlag()
{
	// TODO: Add your control notification handler code here
	if( m_bCubeOnlyFlag )
		m_bCubeOnlyFlag = 0;
	else
		m_bCubeOnlyFlag = 1;
}

void CWrapMeshDlg::OnBnClickedDeleteOriMeshFlag()
{
	// TODO: Add your control notification handler code here
	if( m_bDeleteOriMeshFlag )
		m_bDeleteOriMeshFlag = 0;
	else
		m_bDeleteOriMeshFlag = 1;
}
