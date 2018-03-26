//$c1 XRL 10/29/13 created.
//=========================================================================
// Delauney.cpp: delauney triangulator.
//

#include "stdafx.h"
#include "MeshWorks.h"
#include "MainFrm.h"
#include "MeshGen.h"
#include "AssemblyMesh.h"
#include "MeshWorkOsgView.h"
//#include "pcp.h"

#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/DelaunayTriangulator>

int CMeshGen::triangulator(AssemblyMesh *pAsmMesh)
{ 
	CString str, str2;
	double t_total;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	double *xyz=NULL;
	int *tria=NULL;
	int nf=0, nv=0; 

	int err = pAsmMesh->toArray(&nv, &xyz, &nf, &tria);
	if( err || nf>0 || nv<3 ) {
		if(xyz) delete [] xyz;
		if(tria) delete [] tria;
		if(nf>0) {
			str.Format(_T("Terminated triangulation becasue becasue %d mesh faces exist."),nf);	
		} else if (nv<3){
			str.Format(_T("Terminated triangulation becasue number of points is %d."),nv);
		}
		mwApp->SetStatusBarString(str);
		return err;
	}
	mwApp->SetStatusBarString(_T("Triangulating point cloud ......"));

	// 1 = OSGDelaunayTriangulator
	// 2 = PCL greedty projection
	pAssemblyMesh m_pAM;
	pMWMesh mdb;
	int i,j,k,mesher = 1;
	clock_t start=clock();
	switch( mesher ) {
	case 1: {
		osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array(nv);
		for(i=0; i<nv; i++) {
			j=3*i;
			(*va)[i].set(xyz[j],xyz[j+1], xyz[j+2]);
		}

		start=clock();
		osg::ref_ptr<osgUtil::DelaunayTriangulator> dt = new osgUtil::DelaunayTriangulator;
		dt->setInputPointArray( va.get() );
		dt->setOutputNormalArray( new osg::Vec3Array );
		dt->triangulate();

		t_total = (double)(clock()-start)/CLOCKS_PER_SEC;
		str.Format(_T("Triangulated %d points in %f seconds."),nv,t_total);
		mwApp->SetStatusBarString(str);

		start=clock();

		// update mesh database
		m_pAM = new AssemblyMesh();
		mdb = m_pAM->ithInstancedMesh(0);
		////
		pVertex *vtx = new pVertex[nv];
		memset((char *)vtx,'\0',sizeof(pVertex)*nv);

		//// create vertices
		double coord[3];
		osg::ref_ptr<osg::Vec3Array> v = dt->getInputPointArray();
		osg::Vec3Array::iterator itr;
		i=0;
		for(itr = v->begin(); itr!=v->end(); itr++) {
			coord[0] = (*itr)[0];
			coord[1] = (*itr)[1];
			coord[2] = (*itr)[2];
			vtx[i++] = M_createVertex(mdb,coord,NULL);
		}

		// create triangles 
		osg::DrawElementsUInt *tr = dt->getTriangles();

		pVertex elem[3];
		nf = tr->size() / 3;
		for(i=0; i<nf; i++) {
			j = 3*i;
			k = tr->getElement(j);		elem[0] = vtx[k];
			k = tr->getElement(j+1);	elem[1] = vtx[k];
			k = tr->getElement(j+2);	elem[2] = vtx[k];
			M_createFace(mdb,3,elem,NULL);
		}
		delete [] vtx;
		break;
			}
	case 2: {
		int *elems = NULL;
		double meshsize = 0.03;
		//pc_greedy_projection(nv,xyz,meshsize, &nf,&elems);
		//t_total = (double)(clock()-start)/CLOCKS_PER_SEC;
		//str.Format(_T("Triangulated %d points in %f seconds."),nv,t_total);
		//mwApp->SetStatusBarString(str);

		//// update mesh database
		//m_pAM = new AssemblyMesh();
		////m_pAM->setName("greedy");
		//mdb = m_pAM->ithInstancedMesh(0);
		//////
		//pVertex *vtx = new pVertex[nv];
		//memset((char *)vtx,'\0',sizeof(pVertex)*nv);

		////// create vertices
		//double coord[3];
		//for(i=0; i<nv; i++) {
		//	j=3*i;
		//	coord[0] = xyz[j];
		//	coord[1] = xyz[j+1];
		//	coord[2] = xyz[j+2];
		//	vtx[i] = M_createVertex(mdb,coord,NULL);
		//}

		//// create triangles 
		//pVertex elem[3];
		//for(i=0; i<nf; i++) {
		//	j = 5*i;
		//	if(elems[j] != 3 )
		//		continue;
		//	elem[0] = vtx[ elems[j+1] ];
		//	elem[1] = vtx[ elems[j+2] ];
		//	elem[2] = vtx[ elems[j+3] ];
		//	M_createFace(mdb,3,elem,NULL);
		//}
		//delete [] vtx;
		break;
			}
	}


	pDoc->setAssemblyMesh(m_pAM);

	if(xyz) delete [] xyz;
	if(tria) delete [] tria;

	this->showmesh(0,false);

	t_total = (double)(clock()-start)/CLOCKS_PER_SEC;
	str2.Format(_T(" Built scene graph in %f seconds."),t_total);
	str += str2;
	mwApp->SetStatusBarString(str);

	//osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//geometry->setVertexArray( dt->getInputPointArray() );
	//geometry->setNormalArray( dt->getOutputNormalArray() );
	//geometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE );
	//geometry->addPrimitiveSet( dt->getTriangles() );
	//
	//osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	//geode->addDrawable( geometry.get() );

	//CMeshWorkOsgView* meView = (CMeshWorkOsgView *) mwApp->MeshView();
	//meView->SetGeodeNode(geode.get());

	return 0;
}

