//$c4 XRL 12/11/12 added OnToolsQueryHighlight.
//$c3 XRL 07/31/12 added OnToolsCreateMesh.
//$c2 XRL 07/30/12 added pointsize and transparency in OnToolsOptions.
//$c1 XRL 02/01/11 created.
//=========================================================================
//
// MeshWorksTools.cpp
//
//=========================================================================
#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkView.h"
#include "dialog/SystemOptionDlg.h"
#include "dialog/MeshStatisticsDlg.h"
#include "dialog/MeshHighlightDlg.h"

#include "meshing/MeshGen.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif


/////////////////////////////////////////////////////////////////////////////
// CMeshWorks message handlers

void CMeshWorks::OnToolsGeoAddArrow()
{
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	CMeshWorkDoc * pDoc = psView->GetDocument(); 

	pDoc->AddPSModel(1);
}

void CMeshWorks::OnToolsGeoAddLShape()
{
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	CMeshWorkDoc * pDoc = psView->GetDocument(); 

	pDoc->AddPSModel(2);
}

void CMeshWorks::OnToolsGeoAddCylBox()
{
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	CMeshWorkDoc * pDoc = psView->GetDocument(); 

	pDoc->AddPSModel(3);
}

void CMeshWorks::OnToolsGeoInsertSphere()
{
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	CMeshWorkDoc * pDoc = psView->GetDocument(); 

	pDoc->InsertPSphere4P();
}

void CMeshWorks::OnToolsCreateMesh()
{
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	psView->OnMeshBtn();
}

void CMeshWorks::OnToolsFillHoleMesh()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	pAssemblyMeshMgr pAsmMeMgr = psView->GetDocument()->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshGen *pMesher = new CMeshGen;
		pMesher->run_adapt_process(2,pAsmMe);
		delete pMesher;
	}
}

void CMeshWorks::OnToolsWrapMesh()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	pAssemblyMeshMgr pAsmMeMgr = psView->GetDocument()->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshGen *pMesher = new CMeshGen;
		pMesher->run_adapt_process(1,pAsmMe);
		delete pMesher;
	}
}

void CMeshWorks::OnToolsConvexHull()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	pAssemblyMeshMgr pAsmMeMgr = psView->GetDocument()->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshGen *pMesher = new CMeshGen;
		pMesher->run_adapt_process(0,pAsmMe);
		delete pMesher;
	}
}

void CMeshWorks::OnToolsTetrahedralize()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	pAssemblyMeshMgr pAsmMeMgr = psView->GetDocument()->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshGen *pMesher = new CMeshGen;
		pMesher->tri2tetra(pAsmMe);
		delete pMesher;
	}
}

void CMeshWorks::OnToolsQueryGeneral()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	pAssemblyMeshMgr pAsmMeMgr = psView->GetDocument()->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshStaticsDlg dlg(pAsmMe);
		if(dlg.DoModal() != IDOK)
			return;
	}
}

void CMeshWorks::OnToolsQueryHighlight()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkView * psView = (CMeshWorkView *)GeometryView();
	pAssemblyMeshMgr pAsmMeMgr = psView->GetDocument()->getAssemblyMeshMgr();
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(pAsmMeMgr->current());
	if(pAsmMe) {
		CMeshHighlightDlg dlg(pAsmMe);
		if(dlg.DoModal() != IDOK)
			return;
	}
}

void CMeshWorks::OnToolsOptions()
{
	// TODO: Add your command handler code here
	int iPlane;
	int pos;
	m_options.getClipPlane(&iPlane,&pos);
	CSystemOptionDlg dlg(m_options.getSceneStyle(), 
						 m_options.getBKColor(),
						 iPlane, pos,
						 m_options.getScenePointSize(),
						 m_options.getSceneTransparency(),
						 m_options.getInteriorLightStatus(),
						 m_options.getIntersectCorrect(),
						 m_options.getDefaultSizeCompute());
	if(dlg.DoModal() != IDOK)
		return;

	m_options.setSceneStyle(dlg.m_iRenderMode);
	m_options.setBKColor(dlg.m_iBKColor);
	m_options.setClipPlane(dlg.m_iPlane, dlg.m_planePosition);
	m_options.setScenePointSize(dlg.m_pointSize);
	m_options.setSceneTransparency(dlg.m_transparency);
	m_options.setInteriorLightStatus(dlg.m_bOpposingLightFlag);
	m_options.setIntersectCorrect(dlg.m_iIntersectCorrect);
	m_options.setDefaultSizeCompute(dlg.m_iDefaultSizeCompute);
}
