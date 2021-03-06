//$c4 XRL 03/27/14 support annotation.
//$c3 XRL 03/11/14 support error computation, colormap and lineplot
//$c2 XRL 02/04/14 Support alignment, update assembly mesh and scene.
//$c1 XRL 11/02/13 created.
//=========================================================================
//
// MetrologyTools.cpp
//
//=========================================================================
#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkOsgView.h"
#include "util.h"

//#include "pcp.h"
#include "adaptapi_internal.h"

#include <float.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


int pc_dilute(int nv, double *pxyz, int n)
{
	int mv = nv / n; 
	for(int i=1; i<mv; i++) {
		pxyz[3*i]   = pxyz[3*n*i];
		pxyz[3*i+1] = pxyz[3*n*i+1];
		pxyz[3*i+2] = pxyz[3*n*i+2];
	}
	return mv;
}

void pc_writeERR(const char *fileName, int np, double *ori, double *proj)
{
 //Open the file for writing
  FILE *fp = NULL ;
  if (fileName == NULL || np < 1)
	  return;
  fp = fopen(fileName,"w") ;
  if ( fp == NULL )
	  return;

  //Write the bdf headders
  fprintf(fp,"TITLE = generated by MeshWorks v4.00 (Sat 2/8 11:16:56 2014) \n" ) ;
  for(int i=0; i<np; i++) {
	  fprintf(fp,"%-8d%-16e%-16e%-16e%-16e%-16e%-16e%-16e\n",i+1,
		      ori[3*i],ori[3*i+1],ori[3*i+2],
			  proj[4*i],proj[4*i+1],proj[4*i+2],proj[4*i+3]);
  }
  fclose(fp) ;
  return;
}

void computeColormapValues(int nf, int np, double *proj, int nne, int *iTria, double *values)
{
	int i, j, k;
	int *iCount = new int[nf];
	memset((char *)iCount,'\0',sizeof(int)*nf);
	memset((char *)values,'\0',sizeof(double)*nf);

	// in iTria, node id starts from zero, and element id starts from 1
	for(j=0; j<nne; j++) {
		i = iTria[2*j];					// node id
		k = iTria[2*j+1] - 1;			// element id
		(iCount[k])++;
		values[2*k] += proj[6*i+3];
		if(ABS(values[2*k + 1]) < ABS(proj[6*i+3]))
			values[2*k + 1] = proj[6*i+3];
	}

	for(k=0; k<nf; k++) {
		if(iCount[k] > 1) {
			values[2*k]  = values[2*k] / ((double)iCount[k]);
		} else if(iCount[k] == 0) {
			values[2*k] = VALUE_NOT_EXIST;
			values[2*k+1] = VALUE_NOT_EXIST;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMeshWorks message handlers
void CMeshWorks::OnToolsMetrologySegment()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	CSelectionMgr * pSelMgr = osgView->GetDocument()->getSelectionMgr(); 

	int np = pSelMgr->numPoints();
	if(np < 2) {
		mwApp->SetStatusBarString("Please pick two points, then click Query/Segment...");
		return;
	}
	if(np > 2){
		mwApp->SetStatusBarString("Please clear point selections, pick two points then click Query/Segment...");
		return;
	}

	double xyz[64][3];
	for(int i=0; i<np; i++) {
		MW_Point pt=pSelMgr->ithPoint(i);
		xyz[i][0] = pt.x();
		xyz[i][1] = pt.y();
		xyz[i][2] = pt.z();
	}
	osgView->Segment(xyz);
	return;
}

void CMeshWorks::OnToolsMetrologyCircle()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	CSelectionMgr * pSelMgr = osgView->GetDocument()->getSelectionMgr(); 

	int np = pSelMgr->numPoints();
	if(np < 3) {
		mwApp->SetStatusBarString("Please pick three points, then click Query/Circle...");
		return;
	}
	if(np > 3){
		mwApp->SetStatusBarString("Please clear point selections, then pick three points and click Query/Circle...");
		return;
	}

	double xyz[3][3];
	for(int i=0; i<np; i++) {
		MW_Point pt=pSelMgr->ithPoint(i);
		xyz[i][0] = pt.x();
		xyz[i][1] = pt.y();
		xyz[i][2] = pt.z();
	}
	int err = osgView->ThreePointCircle(xyz);
	if(err > 0) {
		mwApp->SetStatusBarString("Area from the three points is too small. Please clear current selections, and pick three non collinear points...");
	}
	return;
}

void CMeshWorks::OnToolsMetrologySphere()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	CSelectionMgr * pSelMgr = osgView->GetDocument()->getSelectionMgr(); 

	int np = pSelMgr->numPoints();
	if(np < 4) {
		mwApp->SetStatusBarString("Please pick four points, and click Query/Sphere...");
		return;
	}
	if(np > 4){
		mwApp->SetStatusBarString("Please clear point selections, then pick four points and click Query/Sphere...");
		return;
	}

	double xyz[10][3];
	for(int i=0; i<np; i++) {
		MW_Point pt=pSelMgr->ithPoint(i);
		xyz[i][0] = pt.x();
		xyz[i][1] = pt.y();
		xyz[i][2] = pt.z();
	}
	int err = osgView->FourPointSphere(xyz);
	if(err > 0) {
		mwApp->SetStatusBarString("Volume from the four points is too small. Please clear current selections, and pick four non coplnnar points...");
	}
	return;
}

void CMeshWorks::OnToolsMetrologyDelete()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	pAssemblyMeshMgr pAsmMeMgr = osgView->GetDocument()->getAssemblyMeshMgr();
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}

	CSelectionMgr * pSelMgr = osgView->GetDocument()->getSelectionMgr(); 
	if(pSelMgr == NULL || pSelMgr->numPoints()<1) {
		mwApp->SetStatusBarString("No point has been selected for deletion...");
		return;
	}

	int curr = pAsmMeMgr->current();
	pAssemblyMesh pAsmMe = pAsmMeMgr->ith(curr);

	// compare the last 11 characters
	char *partname = pAsmMe->getName();
	int len = strlen(partname);
	int pointcloud=0;
	if(len > 12) {
		const char *last_eleven = &partname[len-11];
		if(strcmp( last_eleven, "_pointcloud" ) == 0 ) {
			pointcloud = 1;
		}
	}
	if(!pointcloud) {
		mwApp->SetStatusBarString("Please select a point cloud as current part...");
		return;
	}

	// we use primitive index only (no use of occ and face id) because the assembly mesh 
	// structure is fixed, i.e. a trabsformation + the body mesh;
	// and those points not in current part is excluded when distance > tol
	int oc, fid, primIndex;
	int np = pSelMgr->numPoints();	
	double *coord = new double[3*np];
	int *index = new int[np];
	for(int i=0; i<np; i++) {
		MW_Point pt=pSelMgr->ithPoint(i);
		coord[3*i] = pt.x();
		coord[3*i+1] = pt.y();
		coord[3*i+2] = pt.z();
		pt.getID(&oc,&fid,&primIndex);
		index[i] = primIndex;
	}
	int nDel = pAsmMe->deleteVertices(np,coord,index);
	delete [] coord;
	delete [] index;

	CString str;
	if(nDel>0) {
		// update point cloud scene  
		pSelMgr->clear();
		pAsmMe->setSgConsist(false);
		osgView->SetSceneData(1,false);
		if(nDel > 1)
			str.Format(_T("%d points are deleted."),nDel);
		else
			str.Format(_T("1 point is deleted."));
	} else {
		str.Format(_T("no point is deleted."));
	}
	mwApp->SetStatusBarString(str);
}

void CMeshWorks::OnToolsMetrologyRotate()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetViewOperation(RotatePart);
}

void CMeshWorks::OnToolsMetrologyTranslate()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetViewOperation(MovePart);

	//CSelectionMgr * pSelMgr = osgView->GetDocument()->getSelectionMgr(); 

	//int np = pSelMgr->numPoints();
	//if(np != 2 ) {
	//	mwApp->SetStatusBarString("Please pick two points, then click PointCloud/Translate...");
	//	return;
	//}

	//pAssemblyMeshMgr pAsmMeMgr = osgView->GetDocument()->getAssemblyMeshMgr();
	//if(!pAsmMeMgr) {
	//	mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
	//	return;
	//}
	//if(pAsmMeMgr->size() != 2) {
	//	mwApp->SetStatusBarString("%d part(s) detected. Please only keep two parts...");
	//	return;
	//}

	//// 
	//int curr = pAsmMeMgr->current();
	//pAssemblyMesh pAsmMe = pAsmMeMgr->ith(curr);

	//// 
	//double xyz[2][3], vec[3], t[4][4];
	//int i,j;
	//for(i=0; i<2; i++) {
	//	MW_Point pt=pSelMgr->ithPoint(i);
	//	xyz[i][0] = pt.x();
	//	xyz[i][1] = pt.y();
	//	xyz[i][2] = pt.z();
	//}
	//diffVt(xyz[1],xyz[0],vec);
	//for(i=0; i<4; i++) {
	//	for(j=0;j<4;j++) {
	//		if(i==j) t[i][j] = 1.0;
	//		else t[i][j] = 0.0;
	//	}
	//}
	//t[3][0] = vec[0];
	//t[3][1] = vec[1];
	//t[3][2] = vec[2];
	//
	////
	//pMWMesh me = pAsmMe->ithInstancedMesh(0);
	//pAsmMe->setSgConsist(false);
	//M_transform(me,t);
	//osgView->SetSceneData(1,false);
}

void CMeshWorks::OnToolsMetrologyICP()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	pAssemblyMeshMgr pAsmMeMgr = osgView->GetDocument()->getAssemblyMeshMgr();
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}
	if(pAsmMeMgr->size() != 2) {
		mwApp->SetStatusBarString("%d part(s) detected. Please only keep two parts...");
		return;
	}

	//pAssemblyMesh pAsmMe1 = pAsmMeMgr->ith(0);
	//pAssemblyMesh pAsmMe2 = pAsmMeMgr->ith(1);

	//int nv1, nf1;
	//double *pxyz1=NULL;
	//int *ptri1=NULL;
	//pAsmMe1->toArray(&nv1, &pxyz1, &nf1, &ptri1);

	//int nv2, nf2;
	//double *pxyz2=NULL;
	//int *ptri2=NULL;
	//pAsmMe2->toArray(&nv2, &pxyz2, &nf2, &ptri2);

	//double dt1, score;
	//double t[4][4];
	//int nSP, ret;
	//mwApp->SetStatusBarString("Aligning......");
	//clock_t start=clock();
	//if( nf2 == 0 ) {
	//	// src - 2;		tgt - 1
	//	nSP = nv2;
	//	if(nv2 > 2*nv1)
	//		nSP = pc_dilute(nv2, pxyz2, 3*nv2/nv1);
	//	ret = pc_align(nSP, pxyz2, nv1, nf1, pxyz1, ptri1, &score, t, NULL);
	//} else {
	//	nSP = nv1;
	//	if(nv1 > 2*nv2)
	//		nSP = pc_dilute(nv1, pxyz1, 3*nv1/nv2);
	//	ret = pc_align(nSP, pxyz1, nv2, nf2, pxyz2, ptri2, &score, t, NULL);
	//}
	//dt1 = (double)(clock()-start)/CLOCKS_PER_SEC;

	//CString str;
	//str.Format(_T("Aligned in %f seconds."),dt1);
	//mwApp->SetStatusBarString(str);

	//std::stringstream oss;
	//osgView->ClearHUDText();
	//oss << std::setprecision(12) << "Score: " << score << "\nSample Points: " << nSP <<"\n";
	//if(ret == 1)
	//	oss << "Not converged\n";
	//osgView->HudText(1, oss.str(),false);

	//delete [] pxyz2;
	//delete [] pxyz1;
	//if(ptri2) delete [] ptri2;
	//if(ptri1) delete [] ptri1;

	//// update assembly mesh and scene graph consistently
	//osg::Matrix transf;
	//transf.set(t[0][0], t[0][1], t[0][2], t[0][3],
	//	       t[1][0], t[1][1], t[1][2], t[1][3],
	//		   t[2][0], t[2][1], t[2][2], t[2][3],
	//		   t[3][0], t[3][1], t[3][2], t[3][3]);
	//if(nf2 == 0){
	//	pAsmMe2->postMult(transf);
	//	osgView->PostMult(transf,1);
	//} else {
	//	pAsmMe1->postMult(transf);
	//	osgView->PostMult(transf,0);  // render inside
	//}

	return;
}

void CMeshWorks::OnToolsMetrologyERR()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	pAssemblyMeshMgr pAsmMeMgr = osgView->GetDocument()->getAssemblyMeshMgr();
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("Empty mesh document. Please open or generate a mesh...");
		return;
	}
	if(pAsmMeMgr->size() != 2) {
		mwApp->SetStatusBarString("%d part(s) detected. Please only keep two parts...");
		return;
	}

	pAssemblyMesh pAsmMe1 = pAsmMeMgr->ith(0);
	pAssemblyMesh pAsmMe2 = pAsmMeMgr->ith(1);

	int nv1, nf1,  nv2, nf2, nOutBBox=0;
	double *pxyz1=NULL;
	double *pxyz2=NULL;
	int *ptri1=NULL;
	int *ptri2=NULL;
	pAsmMe1->toArray(&nv1, &pxyz1, &nf1, &ptri1);
	pAsmMe2->toArray(&nv2, &pxyz2, &nf2, &ptri2);

	// make theoritcal model - 1; point cloud - 2
	// assume point cloud does not have any face
	if( nf1 == 0 && nf2 != 0 ) {
		SWAP(pAssemblyMesh, pAsmMe1, pAsmMe2);
		SWAP(int,nv1,nv2);
		SWAP(int,nf1,nf2);
		SWAP(double *,pxyz1,pxyz2);
		SWAP(int *,ptri1,ptri2);
	}

	if((nf1 == 0 && nf2 == 0) || (nf1 > 0 && nf2 > 0)) {
		mwApp->SetStatusBarString("Cloud points do not exist...");
	} else {
		CString str;
		int i, nne, row=1;
		double *proj = new double[nv2*6];
		int *iTria;
		mwApp->SetStatusBarString("Computing deviations...");
		clock_t start=clock();
		nOutBBox = projectPoints(nv2, pxyz2, nv1, nf1, pxyz1, ptri1, proj, &nne, &iTria);
		str.Format(_T("Computed in %f seconds."),(double)(clock()-start)/CLOCKS_PER_SEC);

		osgView->ClearHUDText();
		if(nOutBBox > 0) {
			std::stringstream oss;
			oss << nOutBBox << " out of " << nv2 <<" not close to CAD model\n";
			osgView->HudText(row, oss.str(),false);
			row++;
		}
		double d, maxd = -DBL_MAX, mind=DBL_MAX, sumOfSquareDist = 0.0;
		for(i=0; i<nv2; i++) {
			d = proj[6*i+3];
			sumOfSquareDist += d*d;
			if(d > maxd) maxd = d;
			if(d < mind) mind = d;
		}

		// store in the database
		double *pDblAttr = new double[nv2*4];  
		for(i=0; i<nv2; i++) {
			pDblAttr[4*i] = proj[6*i];
			pDblAttr[4*i + 1] = proj[6*i + 1];
			pDblAttr[4*i + 2] = proj[6*i + 2];
			pDblAttr[4*i + 3] = proj[6*i + 3];
		}
		pAsmMe2->attr_set(VERTEX_VECTOR_SCALAR,pDblAttr,mind,maxd);
		// store in the database

		std::stringstream oss2;
		oss2 << "SumOfSquareDist: "<< sumOfSquareDist << "\n";
		oss2 << "Max Distance: "<< maxd << "\n";
		oss2 << "Min Distance: "<< mind << "\n";
		osgView->HudText(row, oss2.str(),true);

		//FILE *fp = fopen("pcERR.dat","w") ;
		//if(fp != NULL) {
		//  fprintf(fp,"TITLE = generated by MeshWorks v3.11.3 (Sat 2/8 11:16:56 2014) \n" ) ;
		//  fprintf(fp,"%-8d%-16e%-16e%-16e%\n",nv2,sumOfSquareDist,maxd,mind);
		//  for(i=0; i<nv2; i++) {
		//	  fprintf(fp,"%-8d%-16e%-16e%-16e%-16e%-16e%-16e%-16e\n",iTria[i],
		//			  pxyz2[3*i],pxyz2[3*i+1],pxyz2[3*i+2],
		//			  proj[6*i],proj[6*i+1],proj[6*i+2],proj[6*i+3]);
		//  }
		//  fclose(fp) ;
		//  str += _T(" Written deviations into pcERR.dat...");
		//  mwApp->SetStatusBarString(str);
		//}

		// show color map
		double *values = new double[nf1*2];
		memset((char *)values,'\0',sizeof(double)*nf1*2);
		computeColormapValues(nf1, nv2, proj, nne, iTria, values);
		osgView->ColorMap(nv1, nf1, pxyz1, ptri1, values, mind, maxd);
		// store into database
		pAsmMe1->attr_set(FACE_SCALAR2,values,mind,maxd);

		delete [] proj;
		delete [] iTria;
	}

	delete [] pxyz2;
	delete [] pxyz1;
	if(ptri2) delete [] ptri2;
	if(ptri1) delete [] ptri1;

	return;
}

void CMeshWorks::OnToolsMetrologyLinePlot()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();

	// see if line plot is switched off
	if( osgView->ShowLinePlot() )
		return;

	pAssemblyMeshMgr pAsmMeMgr = osgView->GetDocument()->getAssemblyMeshMgr();
	if(!pAsmMeMgr) {
		mwApp->SetStatusBarString("No open mesh document. Please open or generate a mesh...");
		return;
	}

	/// find the assembly mesh with bar plot attribute
	pAssemblyMesh pc = pAsmMeMgr->withAttribData(VERTEX_VECTOR_SCALAR);
	if(pc == NULL) {
		mwApp->SetStatusBarString("Can not find nodal error data...");
		return;
	}

	// generate a new line plot
	mwApp->SetStatusBarString("Drawing line plot...");

	int numAttr;
	double *pDblAttr, minDevi, maxDevi;
	pc->attr_get(&numAttr, &pDblAttr, &minDevi, &maxDevi);  // do not free the retrieved database array

	int np, nf;
	int *tria = 0;
	double *ori = 0;
	pc->toArray(&np, &ori, &nf, &tria);
	osgView->LinePlot(np,ori,pDblAttr,minDevi,maxDevi);		// need free since no such array in database

	if(ori) delete [] ori;
	if(tria) delete [] tria;

	mwApp->SetStatusBarString("Line plot shown.");
}

//#define MAX_LINE 128
//
//void CMeshWorks::OnToolsMetrologyLinePlot()
//{
//	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
//	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
//
//	// see if line plot is switched off
//	if( osgView->ShowLinePlot() )
//		return;
//
//	// generate a new line plot
//	mwApp->SetStatusBarString("Drawing line plot...");
//
//	FILE *fp = fopen("pcERR.dat","r") ;
//	if(fp != NULL) {
//		int i, np, id;
//		double sumOfSquareDist, maxd, mind;
//		char line[MAX_LINE];
//
//		rewind( fp ) ;
//		fgets( line, MAX_LINE, fp );
//		fgets( line, MAX_LINE, fp );
//		sscanf( line, "%d %le %le %le", &np, &(sumOfSquareDist), &(maxd), &(mind) ) ;
//
//		double *ori = new double[3*np];
//		double *tgt = new double[4*np];
//		for(i=0; i<np; i++) {
//			fgets( line, MAX_LINE, fp );
//			sscanf( line, "%d %le %le %le %le %le %le %le", &id, &(ori[3*i]), &(ori[3*i+1]), &(ori[3*i+2]), 
//				                                            &(tgt[4*i]), &(tgt[4*i+1]), &(tgt[4*i+2]),&(tgt[4*i+3]) ) ;
//		}
//		fclose(fp) ;
//
//		osgView->LinePlot(np,ori,tgt,mind,maxd);
//
//		delete [] ori;
//		delete [] tgt;
//	}
//	mwApp->SetStatusBarString("Line plot shown.");
//}

void CMeshWorks::OnToolsMetrologyColorMap()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->ShowColorMap();
}

void CMeshWorks::OnToolsAnnotationAdd() 
{
	CMeshWorkOsgView * osgView = (CMeshWorkOsgView *)MeshView();
	osgView->SetAnnotationMode(1);
	osgView->SetViewOperation(Idle);
}

void CMeshWorks::OnToolsAnnotationMove() 
{
}

void CMeshWorks::OnToolsAnnotationDelete() 
{
}