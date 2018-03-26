//$c2   XRL 12/08/2012 Print out vertex loation and vertex->faces adjacent info in highlight()
//$c1   XRL 12/08/2012 Created 
//========================================================================//
//              Copyright 2009 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#include "stdafx.h"
#include "afxdialogex.h"
#include "MeshWorks.h"
#include "MeshHighlightDlg.h"
#include "meshing/AssemblyMesh.h"
#include "MeshWorkOsgView.h"
#include "util.h"

#include <math.h>
#include <set>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#define ALLOWED_MAX_MEANRATIO 0.2

// MeshHighlight dialog

IMPLEMENT_DYNAMIC(CMeshHighlightDlg, CDialogEx)

CMeshHighlightDlg::CMeshHighlightDlg(pAssemblyMesh pAsmMe, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMeshHighlightDlg::IDD, pParent)
	, m_pAsmMesh(pAsmMe)
{
}

CMeshHighlightDlg::~CMeshHighlightDlg()
{
}

void CMeshHighlightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_VERTEX_IDS_EDIT, m_sVertexIDs);
	DDX_Text(pDX, IDC_SHAPE_THRESHOLD_EDIT, m_sShapeThreshold);
}


BEGIN_MESSAGE_MAP(CMeshHighlightDlg, CDialogEx)
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////
//

bool CMeshHighlightDlg::Validate(int *num, int **pIDS, double *shp)
{
	CString message;
	CString cs = m_sVertexIDs;
	
	if( cs.IsEmpty() && m_sShapeThreshold.IsEmpty() ) {
		message.LoadString(IDS_EMPTY_NODEIDS_OR_SHAPETHRESHOLD);
		AfxMessageBox(message);
		return false;
	}
	
	bool ret = false;
	if( !cs.IsEmpty() ) {
		std::set<int> ids;
		int pos, len, id;
		do {
			pos = cs.Find(_T(","));
			len = cs.GetLength();
			if(len < 1)
				break;
			if(pos > 0) {
				CString left = cs.Left(pos);
				cs = cs.Right(len - pos - 1);
				id = _ttoi(left);
				if(id>0)
					ids.insert(id);
			} else {
				int id = _ttoi(cs);
				if(id>0)
					ids.insert(id);
			}
		} while (pos > 0);

		*num = ids.size();
		if(*num > 0) {
			*pIDS = new int[*num];
			std::set<int>::iterator itr;
			int i = 0;
			for(itr=ids.begin(); itr!=ids.end(); itr++) {
				(*pIDS)[i] = *itr;
				i++;
			}
			ret = true;
		}

		if(!ret) {
			message.LoadString(IDS_INVALID_VERTEX_IDENTIFIERS);
			AfxMessageBox(message);
		}
	}

	if( !m_sShapeThreshold.IsEmpty() ) {
		*shp = _ttof(m_sShapeThreshold);
		if( *shp  <= 0 || *shp  > ALLOWED_MAX_MEANRATIO) {
			message.LoadString(IDS_INVALID_SHAPE_THRESHOLD);
			AfxMessageBox(message);
			*shp = 0.0;
		} else {
			ret = true;
		}
	}

	return ret;
}


void CMeshHighlightDlg::highlight(int num, int *ids, double threshold)
{
	if(!m_pAsmMesh)
		return;
	
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView* meView = (CMeshWorkOsgView *) mwApp->MeshView();
	
	std::set<pFace> mfaces;
	int i, j;
	for(i=0; i<num; i++) {
		m_pAsmMesh->getNeighboringFacesFromVertexID(-1,ids[i],mfaces);
	}

	std::vector<pRegion> mrgns;
	mrgns.reserve(1024);
	if( 0.0 < threshold && threshold <= ALLOWED_MAX_MEANRATIO) {
		m_pAsmMesh->getRegionsFromShapeThreshold(threshold,mrgns);
	}

	meView->ClearHighLight();
	meView->ClearHUDText();
	std::set<pFace>::iterator fItr;
	pVertex vertex;
	double xyz[4][3], f[3][3], vids[4];
	int nv, count = 0;

	if(num == 1 && mfaces.size() >0) {
		fItr=mfaces.begin();
		nv = F_numVertices(*fItr);
		j=-1;
		for(i=0; i<nv; i++) {
			vertex = F_vertex(*fItr,i);
			V_getCoord(vertex,xyz[i]);
			if(EN_getID((pEntity)vertex) == ids[0]) {
				j=i;
			}
		}
		if(j!=-1) {
			double vec[3], l01, l12, l02;
			VectorSubtract(xyz[0],xyz[1],vec);   l01 = VectorDot(vec,vec);
			VectorSubtract(xyz[0],xyz[2],vec);   l02 = VectorDot(vec,vec);
			VectorSubtract(xyz[1],xyz[2],vec);   l12 = VectorDot(vec,vec);

			std::stringstream oss;
			oss << std::setprecision(6) << "(" << xyz[j][0] << " " << xyz[j][1] << " " << xyz[j][2] << ")";
			meView->HudText(count+2, oss.str(),false);
			meView->HighLightP( xyz[j], 0.02* sqrt((l01+l02+l12)/3.0) );
			count++;		
		}
		meView->HudText(count+2, "adjacent to " + std::to_string((long long)mfaces.size()) + " faces.", false);
	}

	for(fItr=mfaces.begin(); fItr!=mfaces.end(); fItr++) {
		nv = F_numVertices(*fItr);
		for(i=0; i<nv; i++) {
			vertex = F_vertex(*fItr,i);
			V_getCoord(vertex,xyz[i]);
			vids[i] = EN_getID((pEntity)vertex);
		}
		meView->HudText(count+3, "face " + std::to_string((long long)vids[0]) + " " 
									     + std::to_string((long long)vids[1]) + " "
										 + std::to_string((long long)vids[2]), false);
		meView->HighLight(nv,xyz);
		count ++;
	}

	int rf[] = {0,1,3,  1,2,3,  2,0,3,  2,1,0};		// normal points outside

	if(mrgns.size() > 0) 
		meView->HudText(count+2, "found " + std::to_string((long long)mrgns.size()) + " tetra.", false);

	std::vector<pRegion>::iterator rItr;
	for(rItr=mrgns.begin(); rItr!=mrgns.end(); rItr++) {
		for(i=0; i<4;i++) {
			vertex = R_vertex(*rItr,i);
			V_getCoord(vertex,xyz[i]);
			vids[i] = EN_getID((pEntity)vertex);
		}
		if(count < 8) {
			meView->HudText(count+3, "tet " + std::to_string((long long)vids[0]) + " " 
									        + std::to_string((long long)vids[1]) + " "
									        + std::to_string((long long)vids[2]) + " "
											+ std::to_string((long long)vids[3]), false);
		}
		for(i=0; i<4;i++) {
			for(j=0; j<3; j++) {
				f[0][j] = xyz[rf[3*i  ]][j];
				f[1][j] = xyz[rf[3*i+1]][j];
				f[2][j] = xyz[rf[3*i+2]][j];
			}
			meView->HighLight(3,f);
		}
		count++;
	}


	return;
}

////////////////////////////////////////////////////////////////////////////
// CMeshHighlightDlg message handlers

BOOL CMeshHighlightDlg::OnInitDialog()
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

void CMeshHighlightDlg::OnOK() 
{
	UpdateData();
	// TODO: Add extra validation here

	int num = 0;
	int *ids = NULL;
	double shp = -1.0;
	bool bValid = Validate(&num, &ids, &shp);
	
	if(num > 0 || shp > 0.0) {
		highlight(num, ids, shp);
		if(ids) delete [] ids;
	}

	if( !bValid )
		return;		// can't close the dialog
	UpdateData();
	CDialog::OnOK();
}