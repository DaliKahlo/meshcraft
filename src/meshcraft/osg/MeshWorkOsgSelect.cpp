//$c1 XRL 01/09/14 created.
//=========================================================================
//
// MeshWorkSelect.cpp
//
//=========================================================================
#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkOsgView.h"
#include "dialog/SetCurrPartDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMeshWorks message handlers
void CMeshWorks::OnSelectPointMode()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetSelectionMode(10);
	return;
}

void CMeshWorks::OnSelectMEdgeMode()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetSelectionMode(1);
	return;
}

void CMeshWorks::OnSelectMFaceMode()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetSelectionMode(2);
	return;
}

void CMeshWorks::OnSelectClear()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->ClearAllSelection();
	return;
}

void CMeshWorks::OnSelectCurrentPart()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	pAssemblyMeshMgr pAsmMeMgr = osgView->GetDocument()->getAssemblyMeshMgr(); 
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	CSetCurrPartDlg dlg(pAsmMeMgr);
	if(dlg.DoModal() != IDOK)
		return;
	return;
}

void CMeshWorks::OnViewFront()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetViewDirection(1);
}

void CMeshWorks::OnViewTop()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetViewDirection(2);
}

void CMeshWorks::OnViewLeft()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetViewDirection(3);
}

void CMeshWorks::OnViewPerspective()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetViewDirection(4);
}