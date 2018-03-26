//$c1 XRL 01/04/12 created.
//=========================================================================
//
// CreateMeshDlg.cpp
//
//=========================================================================
#include "stdafx.h"
#include "util.h"
#include "resource.h"
#include "CreateMeshDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateMeshDlg dialog

CCreateMeshDlg::CCreateMeshDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateMeshDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateMeshDlg)
	//}}AFX_DATA_INIT
}

CCreateMeshDlg::CCreateMeshDlg(double meshsize,
							   double gradation,
							   bool curvature,
							   double angle,
							   double minsize,
							   CWnd* pParent /*=NULL*/)
	: CDialog(CCreateMeshDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateMeshDlg)
	//}}AFX_DATA_INIT

	// set default parameters to initialize non-afx data
	m_dDefaultElementSize = meshsize;  // always meter

	m_iElemTypeIndex = 0; // tessellation; 1 tessadapt; 2-4 distene
	m_dElementSize = meshsize * 1000;
	m_dMeshGradation = gradation;
	m_sElementSizeWithUnit.Format(_T("%f mm"), m_dElementSize);

	m_bPartialMeshFlag = 0;	// mesh everything

	// curvature related
	m_bCurvatureFlag = curvature;
	if( m_bCurvatureFlag ) {
		m_iElementPerCircleIndex = getAngleIndex(angle);
		m_dMinElementSize = minsize * 1000;
		m_sMinElementSizeWithUnit.Format(_T("%f mm"), m_dMinElementSize);
	}
}

void CCreateMeshDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateMeshDlg)
	//DDX_Control(pDX, IDC_MESH_SIZE_EDIT1, m_editElementSize);
	DDX_Text(pDX, IDC_MESH_SIZE_EDIT1, m_sElementSizeWithUnit);
	//DDX_Control(pDX, IDC_MESH_GRADATION_EDIT1, m_editMeshGradation);
	DDX_Text(pDX, IDC_MESH_GRADATION_EDIT1, m_dMeshGradation);
	DDX_Control(pDX, IDC_ELEMENT_TYPE_COMBO2, m_comboElementType);
	DDX_Control(pDX, IDC_PARTIAL_MESH_CHECK, m_buttonPartialMeshFlag);
	DDX_Control(pDX, IDC_CURVATURE_CHECK1, m_buttonCurvatureFlag);
	DDX_Control(pDX, IDC_ELEMENT_PER_CIRCLE_COMBO1, m_comboElementPerCircle);
	DDX_Control(pDX, IDC_MIN_ELEMENT_SIZE_EDIT1, m_editMinElementSize);
	//DDX_Text(pDX, IDC_MIN_ELEMENT_SIZE_EDIT1, m_sMinElementSizeWithUnit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateMeshDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateMeshDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CURVATURE_CHECK1, &CCreateMeshDlg::OnBnClickedCurvatureFlag)
	ON_BN_CLICKED(IDC_PARTIAL_MESH_CHECK, &CCreateMeshDlg::OnBnClickedSelectedFaceFlag)
END_MESSAGE_MAP()


///--------------------------------------------------------------
/// 

int CCreateMeshDlg::getAngleIndex(double angle)
{
	double npc = 180./angle;
	//int filter[] = {4,6,8,12,16,24,32,36,72,360};
	int filter[] = {5,7,10,14,20,28,34,42,73,360};
	for(int i=0; i<10; i++) {
		if(npc < filter[i])
			return i;
	}
	return -1;
}

double CCreateMeshDlg::getIndexAngle(int i)
{
	int filter[] = {4,6,8,12,16,24,32,36,72,360};
	if( 0<i && i<10 )
		return 180.0/filter[i];  // limiting angle
	return -1.0;
}

bool CCreateMeshDlg::Validate(bool bShowMessage)
{
	UpdateData();
	CString message;

	m_iElemTypeIndex = m_comboElementType.GetCurSel();
	if(m_bCurvatureFlag)
	{
		m_iElementPerCircleIndex = m_comboElementPerCircle.GetCurSel();
		m_editMinElementSize.GetWindowText(m_sMinElementSizeWithUnit);
		if( !str2floatInMeter(m_sMinElementSizeWithUnit, &m_dMinElementSize) )
		{
			message.LoadString(IDS_INVALID_CHAR_IN_MIN_ELEMENT_SIZE);
			AfxMessageBox(message);
			return false;
		}
	}

	if( !str2floatInMeter(m_sElementSizeWithUnit, &m_dElementSize) )
	{
		message.LoadString(IDS_INVALID_CHAR_IN_ELEMENT_SIZE);
		AfxMessageBox(message);
		return false;
	}

	if( m_dDefaultElementSize > 0.0 ) {  // 0.0 means default size not set, so skip
		if ( m_dElementSize > (m_dDefaultElementSize*50.0) ) 
		{
			message.LoadString(IDS_ELEMENT_SIZE_TOO_LARGE);
			AfxMessageBox(message);
			return false;
		}
	}

	if( m_dElementSize < 0.000000001 )
	{
		message.LoadString(IDS_PLEASE_ENTER_POSITIVE_FLOAT_RANGE);
		AfxMessageBox(message);
		return false;
	}

	if( m_dMeshGradation < 1 || m_dMeshGradation >= 10.0 )
	{
		CString strTemp, strFormat;
		strFormat.LoadString( IDS_PLEASE_ENTER_A_NUMBER_BETWEEN_FORMAT_FOR );
		strTemp.LoadString(IDS_GLOBAL_SIZE_FACTOR);
		message.Format( strFormat, 1.0, 10.0, strTemp );
		if( bShowMessage )
			AfxMessageBox( message );
		return false;
	}

	return true;
}

///
///--------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CCreateMeshDlg message handling

// button control

BOOL CCreateMeshDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_comboElementType.SetCurSel(m_iElemTypeIndex);
	m_buttonPartialMeshFlag.SetCheck(m_bPartialMeshFlag);

	m_buttonCurvatureFlag.SetCheck(m_bCurvatureFlag);
	m_comboElementPerCircle.EnableWindow(m_bCurvatureFlag);
	m_editMinElementSize.EnableWindow(m_bCurvatureFlag);
	if(m_bCurvatureFlag) 
	{
		m_comboElementPerCircle.SetCurSel(m_iElementPerCircleIndex);
		m_editMinElementSize.SetWindowText(m_sMinElementSizeWithUnit);
	}

	// define the initial position of the dialog
	CRect rectFrame, rectDlg;
    CWnd* pMainWnd = AfxGetMainWnd();
    if(pMainWnd != NULL)
    {
		pMainWnd->GetClientRect(rectFrame);
        pMainWnd->ClientToScreen(rectFrame);
        GetWindowRect(rectDlg);

		// center (default)
        //int nXPos = rectFrame.left + (rectFrame.Width() / 2) 
        //                           - (rectDlg.Width() / 2);
        //int nYPos = rectFrame.top + (rectFrame.Height() / 2) 
        //                          - (rectDlg.Height() / 2);
		int nXPos = rectFrame.right - rectDlg.Width();
        int nYPos = rectFrame.top;

        ::SetWindowPos(m_hWnd, HWND_TOP, nXPos, nYPos, 
          rectDlg.Width(), rectDlg.Height(), SWP_NOCOPYBITS);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCreateMeshDlg::OnOK() 
{
	// TODO: Add extra validation here

	bool bValid = Validate(true);
	if( !bValid )
		return;		// can't close the dialog
	UpdateData();
	CDialog::OnOK();
}

void CCreateMeshDlg::OnCancel() 
{
	UpdateData();

	CDialog::OnCancel();
}

void CCreateMeshDlg::OnBnClickedCurvatureFlag()
{
	// TODO: Add your control notification handler code here
	if( m_bCurvatureFlag )
		m_bCurvatureFlag = 0;
	else
		m_bCurvatureFlag = 1;
	m_comboElementPerCircle.EnableWindow(m_bCurvatureFlag);
	m_editMinElementSize.EnableWindow(m_bCurvatureFlag);
}

void CCreateMeshDlg::OnBnClickedSelectedFaceFlag()
{
	// TODO: Add your control notification handler code here
	if( m_bPartialMeshFlag )
		m_bPartialMeshFlag = 0;
	else
		m_bPartialMeshFlag = 1;
}
