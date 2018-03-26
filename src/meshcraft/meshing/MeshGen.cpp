//$c9 XRL 10/13/13 support convexhull()
//$c8 XRL 10/11/13 support tri2tetra()
//$c7 XRL 03/29/13 support meshing selected faces for one body part.
//$c6 XRL 11/08/12 set edge ID and face ID into mesh database.
//$c5 XRL 01/05/12 added static/edit/combo controls into the dialog.
//$c4 XRL 01/05/12 updated to mdmgc.
//$c3 XRL 01/04/12 added createMesh dialog.
//$c2 XRL 12/22/11 added mesh().
//$c1 XRL 02/01/11 created with status bar and retrieve parts.
//=========================================================================
// MeshGen.cpp: implementation of the CMeshGen class.
//

#include "stdafx.h"
#include "Moduledef.h"
#include "MeshWorks.h"
#include "MainFrm.h"
#include "MeshGen.h"
#include "AssemblyMesh.h"
#include "../dialog/CreateMeshDlg.h"
#include "../dialog/WrapMeshDlg.h"
#include "../dialog/FillHoleDlg.h"
#include "MeshWorkOsgView.h"
#include "util.h"

#include "mgc_api.h"
#include "MGCParaInterface.h"
#include "MGCCallback.h"

#include "adaptapi.h"
#include "adaptapi_internal.h"
#include "adapt_callback.h"
#include "adapterr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct {
	CMeshGen *pMesher;
	int start, end;
} MeshProgressINFO;

bool mgc_progress_cb(MGCMeshProgressType type, int percent_complete,void *user_data)
{
	MeshProgressINFO *pProgInfo = (MeshProgressINFO *)user_data;
	int dt = abs(pProgInfo->end - pProgInfo->start);
	int per_complete;
	switch(type) {
	case MGC_PROGRESS_PRE_MESH:
		{
			per_complete = pProgInfo->start + 0.4* dt * percent_complete / 100;
			break;
		}
	case MGC_PROGRESS_SURFACE_MESH:
		{
			per_complete = pProgInfo->start + 0.4*dt + 0.6*dt* percent_complete / 100;
			break;
		}
	case MGC_PROGRESS_SOLID_MESH:
		{
			break;
		}
	}
	pProgInfo->pMesher->set_mesh_progress(per_complete);
	return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMeshGen::CMeshGen()
{
	CMainFrame* wnd = (CMainFrame*)AfxGetMainWnd();
	pDoc = (CMeshWorkDoc*)wnd->GetActiveDocument();

	m_pAsmMesh = NULL;
	deleteLocalAsmMesh = false;
	mesh_progress_count = 0;
	interrupt = false;
	start_id = 1;
	init_global_mesh_option();
}

CMeshGen::~CMeshGen()
{
	if(deleteLocalAsmMesh && m_pAsmMesh)
		delete m_pAsmMesh;
}

void CMeshGen::init_global_mesh_option()
{
	sprintf(m_op.dataFileName,"");
	sprintf(m_op.dataFilePath,"");
	sprintf(m_op.mach,"");
	m_op.out = MGC_SERIALIZATION_NONE;				
	//---------------------------------------------------//
	m_op.mesher = MGC_MESHER_DISTENE; 
	m_op.method = 1;						// automatic
	m_op.element = MGC_ETYPE_TRIA3;
	//---------------------------------------------------//
	m_op.meshsize = 0;					// default 
	m_op.gradation = GRADATION_DEFAULT;

	m_op.curvature = false;		
	m_op.angle = LIMITANGLE_DEFAULT;		
	m_op.min_csize = 0;		

	m_op.proximity = false;
	m_op.min_psize = 0; 
	//---------------------------------------------------//
	m_op.relax = false;		// strictly respect topology
	m_op.minLength = MINLENGTH_DEFAULT;
	//---------------------------------------------------//
	m_op.regression_test = 0;	// not run any	
	m_op.verb = 5;
	m_op.scale = SPEEDQUALITYSCALE_DEFAULT;	// TETMESH_STANDARD_OPTIMISATION 
	m_op.absolute = true;
	//---------------------------------------------------//
	meshSelected = 0;
}

void CMeshGen::set_interrupt_mesh()
{ 
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	mwApp->SetStatusBarString( _T("Stopping mesher......") );

	interrupt=true; 
}

bool CMeshGen::get_interrupt_status()
{ 
	return interrupt; 
}

// Return 0 if succeeded loading the part, 
//        1 if not able to open/find the file
int CMeshGen::premesh(int n_topol, PK_TOPOL_t *topols, PK_TRANSF_t *transfs)
{
	if( n_topol < 1 )
		return 1;		// nothing to mesh

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	int defaultComput = mwApp->sysOption()->getDefaultSizeCompute();

	m_pAsmMesh = new AssemblyMesh(n_topol,topols,transfs);
	m_pAsmMesh->setName("parasolid_part");
	deleteLocalAsmMesh = true;
	int nIB = m_pAsmMesh->numInstancedBodies();
	if(nIB < 1)
		return 1;

	double meshsize=0.0, gradation=1.4, angle=0, minsize=0.0;
	bool curvature=false;
	if(defaultComput == 0) {
		// get default mesh parameters
		PK_BODY_t *bodies = new PK_BODY_t[nIB];
		for(int i=0; i<nIB; i++) {
			bodies[i] = m_pAsmMesh->ithInstancedBody(i);
		}
		register_mgc_parasolid_interface();
		MGC_getDefaultMeshParams(nIB,bodies,&meshsize, &gradation, &curvature,&angle,&minsize);
			
		delete [] bodies;	
		delete_mgc_parasolid_interface();
	} 

	// create mesh dialog
	CCreateMeshDlg dlg(meshsize,gradation,curvature,angle,minsize);
	if(dlg.DoModal() != IDOK) {
		return 2;		// Cancelled
	}

	// set global meshing parameters
	meshSelected = dlg.m_bPartialMeshFlag;
	m_op.mesher = MGC_MESHER_DISTENE;
	if(dlg.m_iElemTypeIndex == 0 || dlg.m_iElemTypeIndex == 1)
		m_op.mesher = MGC_MESHER_TESSADAPT;
	
	if(dlg.m_iElemTypeIndex == 0)
		m_op.element=MGC_ETYPE_TESS;
	else {
		m_op.meshsize = dlg.m_dElementSize;
		m_op.gradation = dlg.m_dMeshGradation;
		switch( dlg.m_iElemTypeIndex ) {
			case 3: m_op.element=MGC_ETYPE_QUAD4; break;
			case 4: m_op.element=MGC_ETYPE_TET4; break;
			case 1:
			case 2: m_op.element=MGC_ETYPE_TRIA3; break;
		}
		if(dlg.m_bCurvatureFlag) {
			m_op.curvature = true;
			m_op.angle = dlg.getIndexAngle(dlg.m_iElementPerCircleIndex);
			m_op.min_csize = dlg.m_dMinElementSize;
		}
		else
			m_op.curvature = false;
	}

	m_op.interCorrect = mwApp->sysOption()->getIntersectCorrect() ? false : true;

	return 0;	// ready to mesh
}

int CMeshGen::mesh()
{
	if(m_pAsmMesh==NULL)
		return MESH_NO_GEOMETRY;

	if(get_interrupt_status()==true)
		return MESH_INTERRUPTED;

	// start meshing (can't go back)
	CString str, str2;
	double t_total=0;
	int ret = 0;

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	mwApp->SetStatusBarString(_T("Meshing ......"));

	
	// the previous assembly mesh in MeshDoc is deleted
	// the new assembly mesh is set into MeshDoc
	// MeshGen can't delete this assembly mesh any more
	// NOTE: must delete the previous mesh before mesh2() since the mesh entities are hooked using PK_ATTDEF_t 
	//       which is independent of PSMesh
	//       however, if mesh dails, the previous mesh has gone       7/19/2012
	pDoc->setAssemblyMesh(m_pAsmMesh);
	deleteLocalAsmMesh = false;

	clock_t start=clock();
	if(m_op.mesher == MGC_MESHER_TESSADAPT)
		ret = ta_mesher();
	else
		ret = mesh2(); 
	t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

	if(ret == 0) {
		str.Format(_T("Meshed %d body (or bodies) in %f seconds."),m_pAsmMesh->numInstancedBodies(),t_total);
		mwApp->SetStatusBarString(str);

		start=clock();
		this->showmesh(0,true);
		t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

		str2.Format(_T(" Built scene graph in %f seconds."),t_total);
		str += str2;
		mwApp->SetStatusBarString(str);
	}
	else {
		// pDoc->setAssemblyMesh(NULL);
		if(ret==-1) {
			mwApp->SetStatusBarString(_T("Meshing interrupted."));
			return MESH_INTERRUPTED;
		} else if(ret==-2) {
			mwApp->SetStatusBarString(_T("Meshing quitted because log file can not be open."));
			return MESH_IO_PROBLEM;
		} else if(ret==-4) {
			return MESH_PARTIAL_MESHING_ASSEMBLY_NOT_SUPPORT;
		} else if(ret==-5) {
			return MESH_PARTIAL_MESHING_SELECT_FACE;
		} else if(ret>0) {
			str.Format(_T("Not able to mesh %d out of %d body after %f seconds."),ret,m_pAsmMesh->numInstancedBodies(),t_total);
			mwApp->SetStatusBarString(str);
			return MESH_FAILED;
		} else {
			mwApp->SetStatusBarString(_T("Meshing failed."));
			return MESH_FAILED_NO_REASON;
		}
	}

	return MESH_SUCCEEDED;			// succeded
}

int CMeshGen::showmesh(int mode, bool setViewer)
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView* meView = (CMeshWorkOsgView *) mwApp->MeshView();
	//meView->OnInitialUpdate();
	meView->SetSceneData(mode, setViewer);

	set_mesh_progress(MAX_PROGRESS_COUNT);	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////

//  meshing process one  ---- distene precad/blsurf/tetra (parasolid)

//
// return: >0 the number of failed bodies
//          0 successful
//         -1 interruppted
//         -2 can't open log file
//         -3 no geometry to mesh
//         -4 partial meshing does not support assembly
//		   -5 no selected faces
int CMeshGen::mesh2()
{
	int nB = m_pAsmMesh->numInstancedBodies();
	if(meshSelected) {
		if(nB > 1) {
			//message.Format("Does not support meshing a portion of faces in assembly model.");
			return -4;
		} else if(pDoc->m_picked.size() < 1) {
			//message.Format("Please pick the face(s) to mesh.");
			return -5;
		} 
	}

	//PK_BODY_type_t body_type;
	//PK_BODY_ask_type(39,&body_type);

	PK_CLASS_t pclass0;
	PK_ERROR_code_t err1 = PK_ENTITY_ask_class(39,&pclass0);

	char logFileName[_MAX_FNAME];
	sprintf(logFileName,"mdmgclog.dat");

	FILE *flog; 
	errno_t err = fopen_s( &flog, logFileName, "a");
	if( flog == NULL ) {
		printf("Can't open %s. Error code %d.\n", logFileName, err);
	    return -2;
	}

	MeshEvalData actual;
	time_t ltime;
    time( &ltime );
	char outstr[MAX_PATH];
	if( m_op.element == MGC_ETYPE_TRIA3 )
		sprintf_s(outstr,"%s.x_t   tri3 ",m_op.dataFileName); 
	else if (m_op.element == MGC_ETYPE_TET4 )
		sprintf_s(outstr,"%s.x_t   tet4 ",m_op.dataFileName);
	else
		sprintf_s(outstr,"%s.x_t   quad4 ",m_op.dataFileName);
	
	if( !m_op.curvature )
		sprintf_s(outstr,"%s uniform %f",outstr,1000*m_op.meshsize);
	else 
		sprintf_s(outstr,"%s curvature %f %f %4.1f",outstr,1000*m_op.meshsize,1000*m_op.min_csize,m_op.angle);

	fprintf_s( flog, "%s\n", outstr );
	if( m_op.element == MGC_ETYPE_TET4 ) 
		fprintf( flog, "	#	pre_mesh_time(seconds)	mesh_time		#_of_elems	   worst_AR	#_of_bad_elems\n");
	else
		fprintf( flog, "	#	pre_mesh_time(seconds)	mesh_time		#_of_elems	   #_of_quads	QCheck\n");

	clock_t start;
	int reason = 1;

	register_mgc_parasolid_interface();
	pMGC_t p_mgc = MGC_new();
	MGC_logon(p_mgc, m_op.dataFileName, m_op.verb);
	
	MGC_set_license(p_mgc, mgc_license);
	MGC_set_message_cb(p_mgc, mgc_message_cb, NULL);
	MGC_set_interrupt_cb(p_mgc, mgc_interrupt_cb, &interrupt);

	MeshProgressINFO pgInfo;
	pgInfo.pMesher = this;
	MGC_set_progress_cb(p_mgc, mgc_progress_cb, &pgInfo);

	MGC_set_mesher_type(p_mgc, m_op.mesher);

	MGC_set_surface_mesh_size(p_mgc, m_op.meshsize);
	MGC_set_surface_gradation(p_mgc, m_op.gradation);
	if( m_op.curvature ) MGC_set_curvature_sizing(p_mgc, m_op.angle, m_op.min_csize);
	//if( m_op.proximity ) MGC_set_proximity_sizing(p_mgc, m_op.min_psize);
	if( m_op.relax ) MGC_set_sliver_topology_removal(p_mgc,m_op.minLength);
	MGC_set_speed_quality_scale(p_mgc,m_op.scale);
	MGC_set_intersect_correction(p_mgc, m_op.interCorrect);
	
	bool ok = false;
	pMeshControl pMeCtrl = pDoc->getLocalMeshControl();

	int percent = 0.02*MAX_PROGRESS_COUNT;					// 2% progress
	int percent_per_body = 0.9*MAX_PROGRESS_COUNT/nB;		// total 90% (nB body)
	this->set_mesh_progress(percent);		

	int numFailedBody = 0;
	for(int i=0; i<nB; i++) {

		pMesh_t p_mesh = MGC_newMesh(p_mgc);

		PK_BODY_t ithBody = m_pAsmMesh->ithInstancedBody(i);
		int meshUnselected = 0;
		if( meshSelected ) {
			int nf =  pDoc->m_picked.size();
			int j = 0;
			PK_FACE_t *pkFaces = NULL;
			if(!meshUnselected) {
				// mesh selected
				ithBody = 0;
				pkFaces = new PK_FACE_t[nf];
				for (std::list<InstancedTag>::iterator it=pDoc->m_picked.begin(); it!=pDoc->m_picked.end(); ++it) {
					pkFaces[j++] = it->tag;
				}
			} else {
				// mesh unselected
				std::list<InstancedTag>::iterator fitr;
				int num, found;
				PK_FACE_t *bfaces = NULL;
				PK_BODY_ask_faces(ithBody,&num,&bfaces);
				nf = num - nf;
				pkFaces = new PK_FACE_t[nf];
				for( int ii=0; ii<num; ii++ ) {
					found = 0;
					for(fitr=pDoc->m_picked.begin(); fitr!=pDoc->m_picked.end(); fitr++) {
						if(fitr->tag == bfaces[ii]) {
							found = 1;
							break;
						}
					}
					if(!found) {
						pkFaces[j++] = bfaces[ii];
					}
				}
				PK_MEMORY_free(bfaces);
			}

		    ok = MGC_set_cad_geometry(p_mesh, nf, pkFaces); 
			delete [] pkFaces;

		} else {
			PK_BODY_t pkBodies[1];
			pkBodies[0] = ithBody;
			ok = MGC_set_cad_geometry(p_mesh, 1, pkBodies);
		}
		if(!ok) 
		{  MGC_deleteMesh(p_mesh);	continue; }

		MGC_set_element_type(p_mgc, m_op.element);
		if(m_op.element == MGC_ETYPE_TET4  && ithBody>0) {
			PK_BODY_type_t body_type;
			PK_BODY_ask_type(ithBody,&body_type);
			if( body_type == PK_BODY_type_sheet_c ) {
				MGC_set_element_type(p_mgc, MGC_ETYPE_TRIA3);
			}
		}

		if(pMeCtrl)
			pMeCtrl->set_mdmgc1(p_mgc);

		//printf("Pre-meshing..........\n");
		pgInfo.start = percent;
		pgInfo.end = percent + 0.9*percent_per_body;

		if(get_interrupt_status()==true) {
			MGC_deleteMesh(p_mesh);
			MGC_logoff(p_mgc);
			MGC_delete(p_mgc);
			delete_mgc_parasolid_interface();
			fclose(flog);
			return -1;
		}

		start=clock();
		reason = MGC_preMesh(p_mesh);
		actual.t_premesh=(float)(clock()-start)/CLOCKS_PER_SEC;
		if( reason ) { 
			fprintf_s(flog, "	%d		%f				*** PreMeshing failed. Reason=%d ***\n",i,actual.t_premesh,reason);
			printf("PreMesh failed. Reason=%d\n",reason);
			MGC_deleteMesh(p_mesh);
			numFailedBody++;
			continue; 
		}
				
		if(get_interrupt_status()==true) {
			MGC_deleteMesh(p_mesh);
			MGC_logoff(p_mgc);
			MGC_delete(p_mgc);
			delete_mgc_parasolid_interface();
			fclose(flog);
			return -1;
		}

		// have to be after preMesh to bypass it
		if(pMeCtrl)
			pMeCtrl->set_mdmgc2(p_mgc,NULL);

		start=clock();
		reason = MGC_mesh(p_mesh); 
		actual.t_mesh=(float)(clock()-start)/CLOCKS_PER_SEC;
		printf("Total meshing time: %5.2f %5.2f\n",actual.t_premesh,actual.t_mesh);

		if(get_interrupt_status()==true) {
			MGC_deleteMesh(p_mesh);
			MGC_logoff(p_mgc);
			MGC_delete(p_mgc);
			delete_mgc_parasolid_interface();
			fclose(flog);
			return -1;
		}
				
		if( reason ) {
			fprintf_s(flog, "	%d		%f		%f		*** Meshing Failed. Reason=%d ***\n",
					  i,actual.t_premesh,actual.t_mesh,reason);
			printf("Body %d total mesh time:	%5.2f, %5.2f	Mesh failed. Reason=%d\n",
				      ithBody,actual.t_premesh,actual.t_mesh,reason);
			MGC_deleteMesh(p_mesh);
			numFailedBody++;
			continue;
		} 

		fprintf_s(flog, "	mesh body %d:		preME=%f		ME=%f\n", i,actual.t_premesh,actual.t_mesh);

		// copy into PSMesh
		pMWMesh me = m_pAsmMesh->ithInstancedMesh(i);
		//ASSERT(M_model(me) == ithBody);
		toMeshDB(me, p_mesh);
		M_markAsSkinned(me);
		M_markFaceOrient(me,false);

		this->set_mesh_progress(percent + percent_per_body);
		percent += percent_per_body; 
		MGC_deleteMesh(p_mesh);
	}

	fprintf_s(flog,"\n"); 
	printf("\n");

	MGC_logoff(p_mgc);
	MGC_delete(p_mgc);
	delete_mgc_parasolid_interface();
	fclose(flog);

	return numFailedBody;
}

void CMeshGen::toMeshDB(pMWMesh mdb, pMesh_t p_mesh)
{
    int vertex_count, vol_vertex_count, edge_count, triangle_count, quadrangle_count, tetra_count;
	MGC_mesh_get_vertex_count(p_mesh, &vertex_count, &vol_vertex_count);
	edge_count = MGC_mesh_get_edge_count(p_mesh);
	triangle_count = MGC_mesh_get_triangle_count(p_mesh);
	quadrangle_count = MGC_mesh_get_quadrangle_count(p_mesh);
	tetra_count = MGC_mesh_get_tetrahedron_count(p_mesh);
	if(vol_vertex_count==0 && (vertex_count <= 0 || triangle_count+quadrangle_count <= 0)) 
		return;
	if(vol_vertex_count>0 && vol_vertex_count<=3) 
		return;

	typedef struct { MGCGeomType t; pGEntity g; } classification;

	int maxVertexCount = max(vertex_count,vol_vertex_count);
	pVertex *vtx = new pVertex[maxVertexCount];
	classification *vtx_tags = new classification[vertex_count];
	memset((char *)vtx,'\0',sizeof(pVertex)*maxVertexCount);
	memset((char *)vtx_tags,'\0',sizeof(pGEntity)*vertex_count);
	
	// setup vertex tags since all vertices on edge/face have zero tag
	pGEntity_t tag;
	pVertex elem[4];
	double xyz[3];
	int i, f[4];
	for(i=0; i<triangle_count; i++) {
		MGC_mesh_get_triangle_vertices(p_mesh,i+1,f);
		tag = MGC_mesh_get_triangle_tag(p_mesh,i+1);
		vtx_tags[f[0]-1].g = tag;
		vtx_tags[f[1]-1].g = tag;
		vtx_tags[f[2]-1].g = tag;
		vtx_tags[f[0]-1].t = MGC_GTYPE_FACE;
		vtx_tags[f[1]-1].t = MGC_GTYPE_FACE;
		vtx_tags[f[2]-1].t = MGC_GTYPE_FACE;
	}
	for(i=0; i<quadrangle_count; i++) {
		MGC_mesh_get_quadrangle_vertices(p_mesh,i+1,f);
		tag = MGC_mesh_get_quadrangle_tag(p_mesh,i+1);
		vtx_tags[f[0]-1].g = tag;
		vtx_tags[f[1]-1].g = tag;
		vtx_tags[f[2]-1].g = tag;
		vtx_tags[f[3]-1].g = tag;
		vtx_tags[f[0]-1].t = MGC_GTYPE_FACE;
		vtx_tags[f[1]-1].t = MGC_GTYPE_FACE;
		vtx_tags[f[2]-1].t = MGC_GTYPE_FACE;
		vtx_tags[f[3]-1].t = MGC_GTYPE_FACE;
	}
	for(i=0; i<edge_count; i++) {
		MGC_mesh_get_edge_vertices(p_mesh,i+1,f);
		tag = MGC_mesh_get_edge_tag(p_mesh,i+1);
		if(tag != 0) {
			vtx_tags[f[0]-1].g = tag;
			vtx_tags[f[1]-1].g = tag;
			vtx_tags[f[0]-1].t = MGC_GTYPE_EDGE;
			vtx_tags[f[1]-1].t = MGC_GTYPE_EDGE;
		}
	}

	// create vertices
	MGCGeomType typ;
	//double params[2];
	for(i=0; i<vertex_count; i++) {
		MGC_mesh_get_vertex_coordinates(p_mesh,i+1,xyz);
		tag = MGC_mesh_get_vertex_tag(p_mesh,i+1);
		if(vtx[i] == NULL) {
			typ = MGC_GTYPE_VERTEX;
			if(tag == 0) {
				tag = vtx_tags[i].g;
				typ = vtx_tags[i].t;
			}
			vtx[i] = M_createVertex(mdb,xyz,tag);
			EN_resetID((pEntity)vtx[i],start_id);
			start_id++;
		}
	}
	delete [] vtx_tags;

	if(vol_vertex_count > vertex_count) {
		for(i=vertex_count; i<vol_vertex_count; i++) {
			MGC_mesh_get_vertex_coordinates(p_mesh,i+1,xyz);
			if(vtx[i] == NULL) {
				typ = MGC_GTYPE_BODY;
				vtx[i] = M_createVertex(mdb,xyz,0);		// held at mesh level, not attaching to parasolid entity
				EN_resetID((pEntity)vtx[i],start_id);
				start_id++;
			}
		}
	}

	// create edges classified on model edges
	pEdge ed;
	for(i=0; i<edge_count; i++) {
		tag = MGC_mesh_get_edge_tag(p_mesh,i+1);
		if(tag) {
			MGC_mesh_get_edge_vertices(p_mesh,i+1,f);
			//mesh->createEdge(vtx[f[0]-1],vtx[f[1]-1],tag);
			ed = M_createEdge(mdb,vtx[f[0]-1],vtx[f[1]-1],tag);
			EN_resetID((pEntity)ed,start_id);
			start_id++;
		} 
	}

	// create triangles and quadrangles
	pFace fc;
	for(i=0; i<triangle_count; i++) {
		MGC_mesh_get_triangle_vertices(p_mesh,i+1,f);
		elem[0] = vtx[f[0]-1];
		elem[1] = vtx[f[1]-1];
		elem[2] = vtx[f[2]-1];
		tag = MGC_mesh_get_triangle_tag(p_mesh,i+1);
		//mesh->createFace(3,elem,tag);
		fc = M_createFace(mdb,3,elem,tag);
		EN_resetID((pEntity)fc,start_id);
		start_id++;
	}
	for(i=0; i<quadrangle_count; i++) {
		MGC_mesh_get_quadrangle_vertices(p_mesh,i+1,f);
		tag = MGC_mesh_get_quadrangle_tag(p_mesh,i+1);
		elem[0] = vtx[f[0]-1];
		elem[1] = vtx[f[1]-1];
		elem[2] = vtx[f[2]-1];
		elem[3] = vtx[f[3]-1];
		fc = M_createFace(mdb,4,elem,tag);
		EN_resetID((pEntity)fc,start_id);
		start_id++;
	}

	// create tetras
	pRegion rgn;
	for(i=0; i<tetra_count; i++) {
		MGC_mesh_get_tetrahedron_vertices(p_mesh,i+1,f);
		elem[0] = vtx[f[0]-1];
		elem[1] = vtx[f[1]-1];
		elem[2] = vtx[f[2]-1];
		elem[3] = vtx[f[3]-1];
		rgn = M_createRegion(mdb,elem,0);
		EN_resetID((pEntity)rgn,start_id);
		start_id++;
	}

	delete [] vtx;
	return;
}

///////////////////////////////////////////////////////////////////////////

//  meshing process two  ---- distene tetra only

int CMeshGen::tri2tetra(pAssemblyMesh pAsmMesh)
{
	CString str, str2;
	double t_total=0;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

#ifdef _INCLUDE_DISTENE
	mwApp->SetStatusBarString(_T("Meshing ......"));
	clock_t start=clock();
	int ret = tetrahedralizeCavity(pAsmMesh);
	t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

	if(ret == 0) {
		str.Format(_T("Converted into tetra in %f seconds."),t_total);
		mwApp->SetStatusBarString(str);
	
		start=clock();
		mwApp->sysOption()->setSceneStyle(Facets);	// only support Facets style now
		this->showmesh(0,false);
		t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

		str2.Format(_T(" Built scene graph in %f seconds."),t_total);
		str += str2;
		mwApp->SetStatusBarString(str);
	} else {
		switch(ret) {
		case 1000001: 
			mwApp->SetStatusBarString(_T("Fails due to >=1 instanced bodies.")); break;
		case MGC_MSG_COINCIDENT_NODES:
			mwApp->SetStatusBarString(_T("Fails due to coincident nodes.")); break;
		case MGC_MSG_EDGE_FACE_INTERSECT:
			mwApp->SetStatusBarString(_T("Fails due to edge-face intersecting.")); break;
		case MGC_MSG_EDGE_INTERSECT:
			mwApp->SetStatusBarString(_T("Fails due to edge-edge intersecting.")); break;
		default:
			str.Format(_T("Failed into due to error code %d."),ret);
			mwApp->SetStatusBarString(str);
		}
	}
#else
	mwApp->SetStatusBarString(_T("This version does not include tri2tet capability ......"));
#endif

	return 0;
}


#ifdef _INCLUDE_DISTENE
int CMeshGen::tetrahedralizeCavity(pAssemblyMesh pAsmMesh)
{
	double *xyzALL=NULL;
	int *triaALL=NULL;
	int nf=0, nv=0; 

	if(pAsmMesh->numInstancedBodies() > 1)
		return 1000001;
	int ret = pAsmMesh->toArray(&nv, &xyzALL, &nf, &triaALL);
	if( ret ) {
		if(xyzALL) delete [] xyzALL;
		if(triaALL) delete [] triaALL;
		return ret;
	}

	//// initialize MDMGC component
	pMGC_t pMG = MGC_new();

	MGC_set_license(pMG, mgc_license);
	MGC_set_message_cb(pMG, mgc_message_cb, &errData);
	MGC_set_interrupt_cb(pMG, mgc_interrupt_cb, &interrupt);
	MGC_set_progress_cb(pMG, mgc_progress_cb, NULL);


	// tet meshing
	MGC_set_element_type(pMG, MGC_ETYPE_TET4);
	pMesh_t pME = MGC_newMesh(pMG);
	int reason = MGC_tria2tet(pME,nf, nv, triaALL, xyzALL, -1.0);

	if(!reason) {
		//MGC_mesh2medit(pME,"c:\\xli\\tmp\\tetra.mesh");
		pAssemblyMesh m_pAM = new AssemblyMesh();
		pMWMesh me = m_pAM->ithInstancedMesh(0);
		toMeshDB(me, pME);
		pDoc->setAssemblyMesh(m_pAM);
	} else if(errData.code(0) != 0) {
		reason = errData.code(0);	// get first error code
		errData.show(nv,xyzALL);
	}

	// clean up
	delete [] xyzALL;
	delete [] triaALL;
	MGC_deleteMesh(pME);
	MGC_logoff(pMG);
	MGC_delete(pMG);
	errData.clear();
	return reason;
}
#endif

///////////////////////////////////////////////////////////////////////////

//  procedure in mesh adapt library: three  

int CMeshGen::run_adapt_process(int proc, pAssemblyMesh pAsmMesh)
{
	int ret = 0;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

#ifdef _INCLUDE_MESHADAPT
	MDT_init();
	MDT_set_license_cb(license_cb);
	MDT_set_message_cb(message_cb, NULL);
	MDT_set_interrupt_cb(interrupt_cb, NULL);
	MDT_set_progress_cb(progress_cb, NULL);
	MDT_set_verbose(1);

	switch(proc) {
	case 0: ret = convexHull(pAsmMesh); break;
	case 1: ret = shrinkwrap(pAsmMesh); break;
	case 2: ret = fillhole(pAsmMesh); break;
	}

	MDT_exit();
#else
	mwApp->SetStatusBarString(_T("This version does not include convexHull capability ......"));
#endif

	return ret;
}

#ifdef _INCLUDE_MESHADAPT

int CMeshGen::convexHull(pAssemblyMesh pAsmMesh)
{
	CString str, str2;
	double t_total=0;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	mwApp->SetStatusBarString(_T("Generating convex hull ......"));
	clock_t start=clock();
	int ret = cxhull(pAsmMesh);
	t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

	if(ret == 0) { 
		str.Format(_T("Generated hull in %f seconds."),t_total);
		mwApp->SetStatusBarString(str);

		start=clock();
		mwApp->sysOption()->setSceneStyle(Facets);
		this->showmesh(1,false);
		t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

		str2.Format(_T(" Built scene graph in %f seconds."),t_total);
		str += str2;
		mwApp->SetStatusBarString(str);
	} else {
		if(ret==1001 || ret==1002) {
			mwApp->SetStatusBarString(_T("ConvexHull fails due to tet or quad elements."));
			return 1001;
		} else {
			str.Format(_T("ConvexHull fails due to error code %d."),ret);
			mwApp->SetStatusBarString(str);
		}
	}

	return 0;
}

int CMeshGen::cxhull(pAssemblyMesh pAsmMesh)
{
	double *xyzALL=NULL;
	int *triaALL=NULL;
	int nf=0, nv=0; 

	int err = pAsmMesh->toArray(&nv, &xyzALL, &nf, &triaALL);
	if( err ) {
		if(xyzALL) delete [] xyzALL;
		if(triaALL) delete [] triaALL;
		return err;
	}

	node_t *nodes = new node_t[nv];
	for(int i=0; i<nv; i++) {
		nodes[i].x = xyzALL[3*i];
		nodes[i].y = xyzALL[3*i + 1];
		nodes[i].z = xyzALL[3*i + 2];
	}
	if(xyzALL) delete [] xyzALL;
	if(triaALL) delete [] triaALL;

	pDModel mo=NULL; 
	err = DM_createConvexHull(nv,nodes,1,&mo);
	if(!err) {
		pMesh pME = DM_getMesh(mo);
		pAssemblyMesh m_pAM = new AssemblyMesh();
		pMWMesh me = m_pAM->ithInstancedMesh(0);
		pAssemblyMeshMgr AsmMgr = pDoc->getAssemblyMeshMgr();
		m_pAM->setID( AsmMgr->getFreeID() );
		m_pAM->setName( "convex_hull" );
		ta_toMeshDB(me, pME);
		AsmMgr->setCurrent(AsmMgr->size());  // index to current model is size-1
		AsmMgr->append(m_pAM);
	}

	if(nodes) delete [] nodes;
	if(mo) DM_delete(mo);
	return err;
}

int CMeshGen::shrinkwrap(pAssemblyMesh pAsmMesh)
{
	CString str, str2;
	double t_total=0;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	double *xyzALL=NULL;
	int *triaALL=NULL;
	int nf=0, nv=0; 

	int err = pAsmMesh->toArray(&nv, &xyzALL, &nf, &triaALL);
	if( err ) {
		if(xyzALL) delete [] xyzALL;
		if(triaALL) delete [] triaALL;
		return err;
	}
	double minPt[3], maxPt[3], vec[3], dd;
	boundingbox(nv, xyzALL, minPt, maxPt);
	diffVt(maxPt,minPt,vec);
	dd = dotProd(vec,vec);
	dd = sqrt(dd);

	node_t *nodes = new node_t[nv];
	for(int i=0; i<nv; i++) {
		nodes[i].x = xyzALL[3*i];
		nodes[i].y = xyzALL[3*i + 1];
		nodes[i].z = xyzALL[3*i + 2];
	}
	if(xyzALL) delete [] xyzALL;

	CWrapMeshDlg dlg(pAsmMesh, 0.02*dd, 0.0);
	if(dlg.DoModal() != IDOK) {
		if(triaALL) delete [] triaALL;
		if(nodes) delete [] nodes;
		return -1;
	}
	double wrapsize = dlg.m_dWrapMeshSize;
	double distToMesh = dlg.m_dDistToMesh;
	int cubeOnly = dlg.m_bCubeOnlyFlag;
	int deleteOri = dlg.m_bDeleteOriMeshFlag;

	mwApp->SetStatusBarString(_T("Generating shrinkwrap ......"));
	clock_t start=clock();
	int ret = shrinkwrap2(nv, nodes, nf, triaALL, wrapsize, distToMesh, cubeOnly, deleteOri);
	t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

	if(triaALL) delete [] triaALL;
	if(nodes) delete [] nodes;

	if(ret == 0) { 
		str.Format(_T("Generated shrinkwrap in %f seconds."),t_total);
		mwApp->SetStatusBarString(str);

		start=clock();
		mwApp->sysOption()->setSceneStyle(Facets);
		this->showmesh(!deleteOri,false);
		t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

		str2.Format(_T(" Built scene graph in %f seconds."),t_total);
		str += str2;
		mwApp->SetStatusBarString(str);
	} else if(ret == 2) {
		str.Format(_T("Extrusion failed."),ret);
		mwApp->SetStatusBarString(str);
	} else {
		str.Format(_T("Shrinkwrap failed due to error code %d."),ret);
		mwApp->SetStatusBarString(str);
	}
	return ret;
}

int CMeshGen::shrinkwrap2(int nv, node_t *nodes, int nf, int *tria, double wrapsize, double distToMesh, int cubeOnly, int deleteOri)
{
	pDModel mo = NULL;
	int err = DM_createFromFaceMesh2(nf, nv, tria, nodes, &mo); 
	if(err) {
		if(mo) DM_delete(mo); 
		return 1;
	}

	pDModel wrapMO = NULL;
	int mode = 0;
	if(cubeOnly) mode = mode | 2;
	if(distToMesh) {
		err = DM_extrude(mo, wrapsize, distToMesh);
		if(err && err!= MDT_WARN_zero_extrusion_distance) { 
			if(mo) DM_delete(mo); 
			return 2;
		}
	}
	DM_shrinkwrap(mo, wrapsize, mode, &wrapMO);
	pMesh wrapME = DM_getMesh(wrapMO);

	pAssemblyMesh m_pAM = new AssemblyMesh();
	pMWMesh me = m_pAM->ithInstancedMesh(0);
	m_pAM->setName( "shrinkwrap" );
	ta_toMeshDB(me, wrapME);
	if(deleteOri)
		pDoc->setAssemblyMesh(m_pAM);
	else {
		pAssemblyMeshMgr AsmMgr = pDoc->getAssemblyMeshMgr();
		m_pAM->setID( AsmMgr->getFreeID() );
		AsmMgr->setCurrent(AsmMgr->size());  // index to current model is size-1
		AsmMgr->append(m_pAM);
	}

	if(mo) DM_delete(mo);
	if(wrapMO) DM_delete(wrapMO);
	
	return err;
}

int CMeshGen::fillhole(pAssemblyMesh pAsmMesh)
{
	CString str, str2;
	double t_total=0;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	double *xyzALL=NULL;
	int *triaALL=NULL;
	int nf=0, nv=0; 

	int err = pAsmMesh->toArray(&nv, &xyzALL, &nf, &triaALL);
	if( err ) {
		if(xyzALL) delete [] xyzALL;
		if(triaALL) delete [] triaALL;
		return err;
	}

	node_t *nodes = new node_t[nv];
	for(int i=0; i<nv; i++) {
		nodes[i].x = xyzALL[3*i];
		nodes[i].y = xyzALL[3*i + 1];
		nodes[i].z = xyzALL[3*i + 2];
		nodes[i].id = i+1;
	}
	if(xyzALL) delete [] xyzALL;

	CFillHoleDlg dlg(pAsmMesh, 40., 100.0);
	if(dlg.DoModal() != IDOK) {
		if(triaALL) delete [] triaALL;
		if(nodes) delete [] nodes;
		return -1;
	}
	double featAng = dlg.m_dFeatureAngle;
	double diameter = dlg.m_dFillHoleLessThan;

	mwApp->SetStatusBarString(_T("Identify and fill holes ......"));
	clock_t start=clock();
	int ret = fillhole2(nv, nodes, nf, triaALL, featAng, diameter);
	t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

	if(triaALL) delete [] triaALL;
	if(nodes) delete [] nodes;

	if(ret == 0) { 
		str.Format(_T("Identified and filled in %f seconds."),t_total);
		mwApp->SetStatusBarString(str);

		start=clock();
		mwApp->sysOption()->setSceneStyle(Facets);
		this->showmesh(0,false);
		t_total = (double)(clock()-start)/CLOCKS_PER_SEC;

		str2.Format(_T(" Built scene graph in %f seconds."),t_total);
		str += str2;
		mwApp->SetStatusBarString(str);
	} else {
		str.Format(_T("Hole filling fails due to error code %d."),ret);
		mwApp->SetStatusBarString(str);
	}
	return ret;
}

int CMeshGen::fillhole2(int nv, node_t *nodes, int nf, int *tria, double angleBtwFace, double diameter)
{
	pDModel mo = NULL;
	int err = DM_createFromFaceMesh2(nf, nv, tria, nodes, &mo); 
	if(err) {
		if(mo) DM_delete(mo); 
		return 1;
	}

	bool seperated = true;
	bool allow_nonmanifold = true;
	double fill_size = 0.0;		// match neighbore mesh size
	double gradation = 1.2;//1.3;
	int nFilled=0;
	DM_setAngle4E(mo, angleBtwFace);
	err = DM_fillHoles(mo, diameter, fill_size, gradation, seperated, allow_nonmanifold, &nFilled);

	pMesh filledME = DM_getMesh(mo);

	pAssemblyMesh m_pAM = new AssemblyMesh();
	pMWMesh me = m_pAM->ithInstancedMesh(0);
	ta_toMeshDB(me, filledME);
	pDoc->setAssemblyMesh(m_pAM);

	if(mo) DM_delete(mo);
	return err;
}

void CMeshGen::ta_toMeshDB(pMWMesh mdb, pMesh pME)
{
	int nv = M_numVertices(pME);
	int nf = M_numFaces(pME);
	if(nv <= 3 || nf <= 3) 
		return;

	pVertex *vtx = new pVertex[nv];
	memset((char *)vtx,'\0',sizeof(pVertex)*nv);

	// create vertices
	double xyz[3];
	int id=0, j, k;
	MVIter vItr=M_vertexIter(pME);
	while(pMVertex vertex=MVIter_next(vItr) ) {
		V_coord(vertex,xyz);
		vtx[id] = M_createVertex(mdb,xyz,NULL);
		EN_resetID((pEntity)vtx[id],id+1);
		id++;
		EN_setID((pMEntity)vertex,id);

	}
	MVIter_delete(vItr);

	// create triangles 
	pFace fc;
	pMVertex verts[3];
	pVertex elem[3];
	MFIter fItr=M_faceIter(pME);
	while(pMFace face=MFIter_next(fItr) ) {
		F_verts(face, verts);
		for(j=0; j<3; j++) {
			k=EN_id((pMEntity)verts[j]);
			elem[j] = vtx[k-1];
		}
		fc = M_createFace(mdb,3,elem,NULL);
		EN_resetID((pEntity)fc,++id);
	}
	MFIter_delete(fItr);

	delete [] vtx;
	return;
}

void CMeshGen::ta_toMeshDB2(pMWMesh mdb, pMesh pME)
{
	int nv = M_numVertices(pME);
	int nf = M_numFaces(pME);
	if(nv <= 3 || nf <= 3) 
		return;

	pVertex *vtx = new pVertex[nv];
	memset((char *)vtx,'\0',sizeof(pVertex)*nv);

	// create vertices
	double xyz[3];
	int id=0, j, k, tag, typ;
	MVIter vItr=M_vertexIter(pME);
	while(pMVertex vertex=MVIter_next(vItr) ) {
		V_coord(vertex,xyz);
		tag = EN_classified((pMEntity)vertex);
		typ = EN_classifiedType((pMEntity)vertex);
		if(typ==3 || typ==-1) tag = 0;
		vtx[id] = M_createVertex(mdb,xyz,tag);
		EN_resetID((pEntity)vtx[id],id+1);
		id++;
		EN_setID((pMEntity)vertex,id);
	}
	MVIter_delete(vItr);


	//// create edges classified on model edges
	//pEdge ed;
	//for(i=0; i<edge_count; i++) {
	//	tag = MGC_mesh_get_edge_tag(p_mesh,i+1);
	//	if(tag) {
	//		MGC_mesh_get_edge_vertices(p_mesh,i+1,f);
	//		//mesh->createEdge(vtx[f[0]-1],vtx[f[1]-1],tag);
	//		ed = M_createEdge(mdb,vtx[f[0]-1],vtx[f[1]-1],tag);
	//		EN_resetID((pEntity)ed,start_id);
	//		start_id++;
	//	} 
	//}

	// create triangles 
	pFace fc;
	pMVertex verts[3];
	pVertex elem[3];
	MFIter fItr=M_faceIter(pME);
	while(pMFace face=MFIter_next(fItr) ) {
		typ = EN_classifiedType((pMEntity)face);
		if(typ>2) continue;
		tag = EN_classified((pMEntity)face);

		F_verts(face, verts);
		for(j=0; j<3; j++) {
			k=EN_id((pMEntity)verts[j]);
			elem[j] = vtx[k-1];
		}
		fc = M_createFace(mdb,3,elem,tag);
		EN_resetID((pEntity)fc,++id);
	}
	MFIter_delete(fItr);

	delete [] vtx;
	return;
}

#endif

///////////////////////////////////////////////////////////////////////////

//  meshing process four  ---- my tessadapt (parasolid)


typedef enum  
{cwRefSurfShellElementMesh=0, 
 cwSolidElementMesh=2, 
 cwMixedElementMesh=3 
} TAMeshType;

int ta_setTessEntities(int nComp, int *arrBodies,int *arrIDs, MGCGeomType *arrFlags, double *arrTs, int nMeshType, pTessEntity *pEnts);
void ta_releaseTessEntities(int nComponentCounter, pTessEntity *pEnts);

/* supported body types:
	MGC_GTYPE_VERTEX			---- cwMassElement
	MGC_GTYPE_EDGE				---- cwBeamBody
	MGC_GTYPE_SHELL_BODY		---- cwSheetBody
	MGC_GTYPE_BODY				---- cwSolidBody
	MGC_GTYPE_GENERAL_BODY		---- cwGeneralBody
*/
int ta_setTessEntities(int nComponentCounter,
				       int *arrSWbodies,
				       int *arrIDs,
				       MGCGeomType *arrFlags,
				       double *arrTs,
				       int nMeshType,
				       pTessEntity *pEnts)
{
	int i, state=0;
	for( i=0; i<nComponentCounter; i++ ) pEnts[i] = 0;

	if( nMeshType == cwMixedElementMesh || nMeshType == cwRefSurfShellElementMesh )
	{
		for( i=0; i<nComponentCounter; i++ ) 
		{
			if( arrFlags[i] == MGC_GTYPE_EDGE || arrFlags[i] == MGC_GTYPE_VERTEX )
				continue;
				
			if( arrFlags[i] == MGC_GTYPE_GENERAL_BODY )	// mixed mesh using general body
			{
				++state;
				pEnts[i] = TA_newTessEntity(arrSWbodies[i],arrIDs[i],3,arrTs[i]);
				continue;
			}

			if( arrFlags[i] == MGC_GTYPE_SHELL_BODY )
			{
				//double dThickness = getSheetThicknessFromID(arrIDs.GetAt(i));
				if(arrTs[i]>0.0) {
					pEnts[i] = TA_newTessEntity(arrSWbodies[i],arrIDs[i],1,arrTs[i]);
					++state;
				}
				continue;
			}
			
			if( nMeshType == cwMixedElementMesh )
			{
				++state;
				pEnts[i] = TA_newTessEntity(arrSWbodies[i],arrIDs[i],0,0.0);  // cwSolidBody
			}
		}
	}


	if( nMeshType == cwSolidElementMesh )
	{
		for( i=0; i<nComponentCounter; i++ ) {
			pEnts[i] = 0;
			if( arrFlags[i] == MGC_GTYPE_EDGE || arrFlags[i] == MGC_GTYPE_VERTEX )
				continue;
			++state;
			pEnts[i] = TA_newTessEntity(arrSWbodies[i],arrIDs[i],0,0.0);
		}
	}

	return state;
}

void ta_releaseTessEntities(int nComponentCounter, pTessEntity *pEnts)
{
	for( int i=0; i<nComponentCounter; i++ ) 
		if( pEnts[i] ) TA_deleteTessEntity(pEnts[i]);
	delete [] pEnts;
	return;
}

// return -1 if interrupt
int CMeshGen::ta_mesher()
{
	int i;

	//********* initialize mesh database and tools *********//
	MDT_init();

	MDT_set_license_cb(license_cb);
	MDT_set_message_cb(message_cb, NULL);
	MDT_set_interrupt_cb(interrupt_cb, &interrupt);
	//MDT_set_verbose(op.verb);

	MeshProgressINFO pgInfo;
	pgInfo.pMesher = this;
	MDT_set_progress_cb(progress_cb, &pgInfo);

	//********* set model entites to be meshed *********//
	PK_BODY_t ithBody;
	PK_BODY_type_t body_type;
	int nB = m_pAsmMesh->numInstancedBodies();
	int *arrTAGs = new int[nB];
	int *arrIDs = new int[nB];
	MGCGeomType *arrFlags = new MGCGeomType[nB];
	double *arrTs = new double[nB];
	for(i=0; i<nB; i++) {
		ithBody = m_pAsmMesh->ithInstancedBody(i);
		arrTAGs[i] = ithBody;					// PS tag
		arrIDs[i] = i+1;						// should input, maintained by application
		arrTs[i] = 1.0;							// thickness for shell elements
		PK_BODY_ask_type(ithBody,&body_type);
		if( body_type == PK_BODY_type_solid_c ) {
			arrFlags[i] = MGC_GTYPE_BODY;
		} else if( body_type == PK_BODY_type_general_c ) {
			arrFlags[i] = MGC_GTYPE_GENERAL_BODY;
		} else if( body_type == PK_BODY_type_sheet_c ) {
			arrFlags[i] = MGC_GTYPE_SHELL_BODY;
		}
	}

	pTessEntity *pTess = new pTessEntity[nB];
	TAMeshType nMeshType = cwSolidElementMesh;
	if( ! ta_setTessEntities(nB,arrTAGs,arrIDs,arrFlags,arrTs,nMeshType,pTess) )
	{
		ta_releaseTessEntities(nB,pTess);
		delete [] arrTAGs;
		delete [] arrIDs;
		delete [] arrFlags;
		delete [] arrTs;
		return 0; //2; model with remote mass or beam body only
	}

	int percent = 0.02*MAX_PROGRESS_COUNT;							// 2% progress
	double percent_per_body = 0.8*MAX_PROGRESS_COUNT/(double)nB;	// 80% and nB bodies account for 80% total
	this->set_mesh_progress(percent);		

	/** path for log file */
	char *pfname = 0;
	/** load model*/
   	pBodyMan boMan;
	int errCode;
	std::vector<int> errEnts;

	// we do not generate congruent mesh by setting the 3rd argument false
	boMan = BM_new(nB,pTess,false,pfname,&errCode,errEnts);

	percent = 0.07*MAX_PROGRESS_COUNT;					// 7% progress
	this->set_mesh_progress(percent);	

	//MD_debugListCacheOff();
	if( pfname ) delete [] pfname;

	if( errCode==1 ) {
		//std::vector<int>::iterator it;
		//for( it=errEnts.begin(); it!=errEnts.end(); it++ ) 
		//	tessDiagnostics(pSolidMgr,pTess[*it]->compID());
		ta_releaseTessEntities(nB,pTess);
		delete [] arrTAGs;
		delete [] arrIDs;
		delete [] arrFlags;
		delete [] arrTs;
		return 3;
	}

	// TessAdapt object
	int mode = 2;
	if(m_op.element == MGC_ETYPE_TESS)
		mode = 1;					// only tesselate
	pTAdapt pTa = TA_new(boMan, mode);

	// set global mesh size control
	double h, hmin, angle, gamma, hshort;
	if(m_op.element != MGC_ETYPE_TESS) {
		h = m_op.meshsize;
		hmin = h*0.2;
		angle = m_op.angle;  //360.0/((double)numElemsPerCircle);
		gamma = m_op.gradation;
		hshort = 0.5*MIN(hmin,0.01*h);  // 0.01h is needed for spr 429228
		TA_setMeshControl(pTa,angle, h, hmin, gamma, hshort, 1); // absolute sizing
		//// set local mesh size control
		//setLocalMeshControl((void *)pTa);
	}

	percent = 0.1*MAX_PROGRESS_COUNT;					// 10% progress
	this->set_mesh_progress(percent);

	int retcode;
	int status = 0; 
	int nparts = 0; // # of meshed parts

	// stage 1: shell meshing
	for( i=0; i<nB; i++ ) 
	{
		if( !pTess[i] ) continue;
		//progBar.setStatus(pTess[i]->compID());

		TA_setTessEntityTag(pTess[i],i);
		retcode = TA_createFaceMesh(pTa,pTess[i],i);
		TA_setTessEntityTag(pTess[i],retcode);
		if( abs(retcode) == 1 )
		{ status = retcode; break; }
		++nparts;
		if( retcode >= 100 ) {
			//tessDiagnostics(pSolidMgr,pTess[i]->compID());	
			TA_deleteTessEntity(pTess[i]);
			pTess[i] = 0;
			TA_clearMesh(pTa, i);
			--nparts;
		}
		percent += percent_per_body;				
		this->set_mesh_progress(percent);
	}
	// 90% has been shown

	// if successful, copy into PSMesh
	if(!status  && nparts>=1) {
		for( i=0; i<nB; i++ ) {
			pMWMesh me = m_pAsmMesh->ithInstancedMesh(i);
			pMesh pME = BM_ithMesh(boMan, i);
			ta_toMeshDB2(me, pME);
			M_markAsSkinned(me);
			M_markFaceOrient(me, true);
		}
	}

	//if( !status  && nparts>1 ) {
	//	// stage 2: make compatible
	//	int nCompSets = setEEMatchLists(pTa,nComponentCounter,pTess,compState);	// component sets
	//	if( compmap.size()>0 || compState==1 || nCompSets>0 ) {
	//		removeContactFaceSetFromMatchList(pTa);		// contact sets
	//		try { 
	//			TA_matchmergeMesh(pTa,nComponentCounter,pTess);
	//			if( nMeshType == cwMidSurfShellElementMesh || nMeshType == cwMixedElementMesh || nMeshType == cwRefSurfShellElementMesh || nMeshType == cwPlanarElementMesh) {
	//				PREFERENCES *pPreferences = CosmosApp->GetPreferences();
	//				if (pPreferences->mesh.nAutoShellRealignment)
	//					TA_alignShells(pTa);
	//			}
	//			// stage 3: remove sliver elements
	//			if( TA_isCompatible(pTa) ) {
	//				m_bCompatible = TRUE;
	//				TA_removeSliverFaces(pTa);
	//			}
	//		}
	//		catch ( ... ) {	
	//			nparts = 0;
	//			status = 401;
	//		}
	//	}
	//}

//	// stage 4: volume meshing for solid bodies
//	if( !status ) 
//	{
//		int numThreads = 0;
//		for( i=0; i<nB; i++ ) 
//		{
//			if( !pTess[i] ) continue;
//			if( pTess[i]->tag() ) continue;
//			if( pTess[i]->type() ) continue;
//			numThreads++;
//		}
//		numThreads = MIN(numThreads, ParallelJobManager_c::getNumberOfCores());
//        ParallelJobManager_c parallelJobMgr(bCosmosParallelVolumeMesher ? numThreads : -1  );
//
//		// create the worker threads
//		parallelJobMgr.CreateWorkerThreads();
//
//		m_nMeshPos = 40; //percent;
//		SetMeshProgressStatus();
//
//		for( i=0; i<nComponentCounter; i++ ) 
//		{
//			if( !pTess[i] ) continue;
//			if( pTess[i]->tag() ) continue;
//			if( pTess[i]->type() ) continue;
//					
////			CString sComponentMessage;
////			sComponentMessage.Format(_T(" (%d of %d)"), i+1, nComponentCounter);
////			m_sProcessingPart = GetCompName(pTess[i]->compID()) + sComponentMessage;
//			
//			int status = TA_meBoVol(&parallelJobMgr, pTa, pTess[i], i);
//			m_nTotalPos = (int)((nTotalMeshRange*(++nCount))/(double)((2*nSolidBody) + nSheetBodies + nMidSurfShells + nBeamBodies));
//			dPercentage = m_nTotalPos;
//			m_sTotalPercentage.Format(_T("%1.1f"), dPercentage);
//			m_sTotalPercentage+=_T("%");
//			SetMeshProgressStatus();
//
//			if( pTess[i]->tag() == 202 ) 
//			{ status = 1; break; }
//
///*			set_mesh_status(3, "Finished  mesh...",100);
//			
//			if( abs(retcode) == 1 )
//			{ 
//				status = retcode; 
//				break; 
//			}*/
//		}
//
//		// destroy the worker threads
//		parallelJobMgr.DestroyWorkerThreads();
//
//		std::vector<int> boIndices;
//		for( i=0; i<nComponentCounter; i++ ) 
//		{
//			if( !pTess[i] ) continue;
//			if( pTess[i]->type() ) continue;
//
//			if( pTess[i]->tag() == 202 ) 
//			{ status = 1; break; }
//			if( pTess[i]->tag() >= 100 ) 
//			{
//				tessDiagnostics(pSolidMgr,pTess[i]->compID());
//				if( TA_isCompatible(pTa) )
//					boIndices.push_back(i);
//				else { 
//					TA_clearMesh(pTa, i); 
//					delete pTess[i]; 
//					pTess[i]=0; 
//				}
//				--nparts;
//			}
//		}
//		TA_deleteUnusedMesh(pTa, boIndices);  // do not delete pTess[i] here.
//	}


	TA_delete(pTa);
	BM_delete(boMan);
	MDT_exit();
	ta_releaseTessEntities(nB,pTess);
	delete [] arrTAGs;
	delete [] arrIDs;
	delete [] arrFlags;
	delete [] arrTs;

	percent = 1.0*MAX_PROGRESS_COUNT;					// 100% progress
	this->set_mesh_progress(percent);

	if( status==-1 ) return -1; 
	if( status == 1 ) return 1; // failed for memory shortage
	if( nparts >  0 ) return 0; // succeeded
	return 9;	// failed
}

