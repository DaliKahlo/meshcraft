//$c3 XRL 06/06/13 support ChangeBackground: dark blue, gradient, and white.
//$c2 XRL 07/30/12 add point size and transparency slider bar.
//$c1 XRL 01/04/12 create.
//=========================================================================
//
// CreateMeshDlg.cpp
//
//=========================================================================
#include "stdafx.h"
#include "resource.h"
#include "SystemOptionDlg.h"
#include "../MeshWorks.h"
#include "../MeshWorkDoc.h"
#include "../MeshWorkOsgView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OSG_POINTSIZE_SCALE_FACTOR 0.15
#define OSG_TRANSPARENCY_SCALE_FACTOR 0.01

/////////////////////////////////////////////////////////////////////////////
// CCreateMeshDlg dialog

CSystemOptionDlg::CSystemOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemOptionDlg::IDD, pParent)
	, m_ptSizeSliderEcho(_T(""))
	, m_TransparencySliderEcho(_T(""))
{
	//{{AFX_DATA_INIT(CCreateMeshDlg)
	//}}AFX_DATA_INIT
}

CSystemOptionDlg::CSystemOptionDlg(SceneStyle style, 
								   int bkColor,
								   int planeIdx, int planePos, 
								   double ptsize, 
								   double trans,
								   int lightstatus,
								   int intercorrect,
								   int defaultsizecompute,
								   CWnd* pParent /*=NULL*/)
	: CDialog(CSystemOptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateMeshDlg)
	//}}AFX_DATA_INIT

	m_iRenderMode = style;
	m_iBKColor = bkColor;
	//m_bkColorR = BKCOLOR_R;
	//m_bkColorG = BKCOLOR_G;
	//m_bkColorB = BKCOLOR_B;
	m_iPlane = planeIdx;		
	m_planePosition = planePos;
	m_pointSize = ptsize;
	m_transparency = trans;
	m_bOpposingLightFlag = lightstatus;	

	m_iIntersectCorrect = intercorrect;
	m_iDefaultSizeCompute = defaultsizecompute;
}

void CSystemOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemOptionDlg)
	DDX_Control(pDX, IDC_OSG_RENDER_MODE_COMBO, m_comboRenderMode);
	//
	DDX_Control(pDX, IDC_BACKGROUND_COLOR_COMBO, m_comboBackgroundColor);
	//
	DDX_Control(pDX, IDC_OSG_CLIPPLANE_COMBO, m_comboClipPlane);
	DDX_Control(pDX, IDC_CLIPPLANE_SLIDER, m_clipPlaneSliderBar);
	//
	DDX_Control(pDX, IDC_POINTSIZE_SLIDER, m_PointSizeSliderBar);
	DDX_Text(pDX, IDC_POINTSIZE_OPTION, m_ptSizeSliderEcho);
	//
	DDX_Control(pDX, IDC_TRANPARENCY_SLIDER, m_TransparencySliderBar);
	DDX_Text(pDX, IDC_TRANSPARENCY_VALUE, m_TransparencySliderEcho);
	//
	DDX_Control(pDX, IDC_INTERIOR_LIGHT_CHECK, m_buttonOpposingLightFlag);
	//
	DDX_Control(pDX, IDC_INTERSECT_CORRECTION_COMBO, m_comboIntersetCorrect);
	DDX_Control(pDX, IDC_DEFAULT_SIZE_COMPUTE_COMBO, m_comboDefaultSizeCompute);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSystemOptionDlg, CDialog)
	//{{AFX_MSG_MAP(CSystemOptionDlg)
	//}}AFX_MSG_MAP
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_INTERIOR_LIGHT_CHECK, &CSystemOptionDlg::OnBnClickedOpposingLightFlag)
END_MESSAGE_MAP()


///
///--------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CCreateMeshDlg message handling

// button control

BOOL CSystemOptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// initialize controls
	m_comboRenderMode.SetCurSel(m_iRenderMode);
	m_comboBackgroundColor.SetCurSel(m_iBKColor);

	m_comboClipPlane.SetCurSel(m_iPlane);
	m_clipPlaneSliderBar.SetRange(0,100,true);
	m_clipPlaneSliderBar.SetPos(m_planePosition);

	m_ptSizeSliderEcho.Format(_T("%5.2lf"),m_pointSize);
	m_PointSizeSliderBar.SetRange(0,100,true);
	m_PointSizeSliderBar.SetPos(m_pointSize/OSG_POINTSIZE_SCALE_FACTOR);

	m_TransparencySliderEcho.Format(_T("%3.1lf"),m_transparency);
	m_TransparencySliderBar.SetRange(0,100,true);
	m_TransparencySliderBar.SetPos(m_transparency/OSG_TRANSPARENCY_SCALE_FACTOR);

	m_buttonOpposingLightFlag.SetCheck(m_bOpposingLightFlag);

	m_comboIntersetCorrect.SetCurSel(m_iIntersectCorrect);
	m_comboDefaultSizeCompute.SetCurSel(m_iDefaultSizeCompute);

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

void CSystemOptionDlg::OnOK() 
{
	// TODO: Add extra validation here

	bool bValid = Validate();
	if( !bValid )
		return;		// can't close the dialog
	UpdateData();
	CDialog::OnOK();
}

void CSystemOptionDlg::OnCancel() 
{
	UpdateData();
	CDialog::OnCancel();

	if(m_pointSize != OSG_POINTSIZE_SCALE_FACTOR * m_PointSizeSliderBar.GetPos() || 
	   m_transparency != OSG_TRANSPARENCY_SCALE_FACTOR * m_TransparencySliderBar.GetPos() )
	{
		CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
		CMeshWorkOsgView *osgview = (CMeshWorkOsgView *) mwApp->MeshView();
		osgview->ChangePointSize(m_pointSize,false);
		osgview->ChangeTransparency(m_transparency,false);
	}
}

bool CSystemOptionDlg::Validate()
{
	UpdateData();
	int oldBKGD = m_iBKColor;

	m_iRenderMode = (SceneStyle) m_comboRenderMode.GetCurSel();
	m_iBKColor = (int) m_comboBackgroundColor.GetCurSel();
	m_iPlane = m_comboClipPlane.GetCurSel();
	m_planePosition = m_clipPlaneSliderBar.GetPos();
	m_pointSize = OSG_POINTSIZE_SCALE_FACTOR * m_PointSizeSliderBar.GetPos();
	m_transparency = OSG_TRANSPARENCY_SCALE_FACTOR * m_TransparencySliderBar.GetPos();

	m_iIntersectCorrect = m_comboIntersetCorrect.GetCurSel();
	m_iDefaultSizeCompute = m_comboDefaultSizeCompute.GetCurSel();

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView *osgview = (CMeshWorkOsgView *) mwApp->MeshView();
	if(osgview)
		osgview->ChangeClipPlane(m_iPlane, m_planePosition);
	if(osgview && oldBKGD!=m_iBKColor)
		osgview->ChangeBackground(oldBKGD, m_iBKColor);
	if(osgview)
		osgview->ChangeInteriorLightStatus(m_bOpposingLightFlag);
	return true;
}

void CSystemOptionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView *osgview = (CMeshWorkOsgView *) mwApp->MeshView();

	double value;
	if(pScrollBar == (CScrollBar *) &m_PointSizeSliderBar) {
		value = OSG_POINTSIZE_SCALE_FACTOR * m_PointSizeSliderBar.GetPos();
		m_ptSizeSliderEcho.Format(_T("%5.2lf"),value);
		osgview->ChangePointSize(value,true);
		UpdateData( FALSE );
	} else if(pScrollBar == (CScrollBar *) &m_TransparencySliderBar) {
		value = OSG_TRANSPARENCY_SCALE_FACTOR * m_TransparencySliderBar.GetPos();
		m_TransparencySliderEcho.Format(_T("%3.1lf"),value);
		osgview->ChangeTransparency(value,true);
		UpdateData( FALSE );
	} else if(pScrollBar == (CScrollBar *) &m_clipPlaneSliderBar) {
		int idx = m_comboClipPlane.GetCurSel();
		if(idx > 0) {
			osgview->ChangeClipPlane(idx, m_clipPlaneSliderBar.GetPos());
			UpdateData( FALSE );
		}
	} else {
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}
}

void CSystemOptionDlg::OnBnClickedOpposingLightFlag()
{
	// TODO: Add your control notification handler code here
	m_bOpposingLightFlag = ! m_bOpposingLightFlag;
}
