// MeshStaticsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "MeshStatisticsDlg.h"
#include "meshing/AssemblyMesh.h"
#include "Meshing/instance.h"

// CMeshStaticsDlg dialog

IMPLEMENT_DYNAMIC(CMeshStaticsDlg, CDialogEx)

CMeshStaticsDlg::CMeshStaticsDlg(pAssemblyMesh pAsmMe, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMeshStaticsDlg::IDD, pParent)
	, m_numMeVertices(_T(""))
	, m_numMeFaces(_T(""))
	, m_numMeRegions(_T(""))
	, m_pAsmMesh(pAsmMe)
{

}

CMeshStaticsDlg::~CMeshStaticsDlg()
{
}

void CMeshStaticsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NUM_MVERTICES, m_numMeVertices);
	DDX_Text(pDX, IDC_NUM_MFACES, m_numMeFaces);
	DDX_Text(pDX, IDC_NUM_MREGIONS, m_numMeRegions);
}


BEGIN_MESSAGE_MAP(CMeshStaticsDlg, CDialogEx)
END_MESSAGE_MAP()


// CMeshStaticsDlg message handlers
BOOL CMeshStaticsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// get general assembly mesh information
	pMWMesh me;
	int nv=0, nf=0, nr=0;
	int iB, nIB = m_pAsmMesh->numInstancedBodies();
	int nI = m_pAsmMesh->numInstances(); 
	for(int i=0; i<nI; i++) {
		Instance * inst = m_pAsmMesh->ithInstance(i); 
		iB = inst->body_index();
		me = m_pAsmMesh->ithInstancedMesh(iB);
		nv += M_numberVertices(me);
		nf += M_numberFaces(me);
		nr += M_numberRegions(me);
	}

	// initialize text variables
	m_numMeVertices.Format(_T("%d"),nv);
	m_numMeFaces.Format(_T("%d"),nf);
	m_numMeRegions.Format(_T("%d"),nr);

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
