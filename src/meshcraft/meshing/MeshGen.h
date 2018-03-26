//$c3   XRL 07/16/2014 support ErrorData in tetrahedralize
//$c2   XRL 03/30/2013 Add "meshSelected" to support partial meshing
//$c1   XRL 02/01/2011 Created 
//========================================================================
//
// MeshGen.h: interface for the various meshers.
//
//========================================================================

#ifndef CMESHGEN_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define CMESHGEN_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#include "meshing/MeshOptions.h"
#include "meshing/AssemblyMeshMgr.h"
#include "adaptdef.h"
#include "ErrorData.h"

#define MIN(x,y) ((x)>(y) ? (y) : (x))


#define MAX_PROGRESS_COUNT	10000

// meshing finish status
#define MESH_SUCCEEDED		0
#define MESH_NO_GEOMETRY	1
#define MESH_INTERRUPTED	2
#define MESH_IO_PROBLEM		3
#define MESH_PARTIAL_MESHING_ASSEMBLY_NOT_SUPPORT 4
#define MESH_PARTIAL_MESHING_SELECT_FACE 5
#define MESH_FAILED			99998
#define MESH_FAILED_NO_REASON	99999

class CMeshGen  
{
public:
	CMeshGen();
	virtual ~CMeshGen();

	int premesh(int, PK_TOPOL_t *, PK_TRANSF_t *);
	int elementType() { return m_op.element;}
	int mesh();
	int tri2tetra(AssemblyMesh *);
	int run_adapt_process(int, AssemblyMesh *);
	int showmesh(int, bool);

	int triangulator(AssemblyMesh *);					// delauney triangulator of points

	void set_mesh_progress(int c) { mesh_progress_count= c; }
	int get_mesh_progress() { return mesh_progress_count; }
	void set_interrupt_mesh();

private:
	CMeshWorkDoc* pDoc;
	pAssemblyMesh m_pAsmMesh;		// 
	bool deleteLocalAsmMesh;		//
	MeshOpt m_op;					// global
	int meshSelected;				// 0 - mesh the whole model
	CErrData errData;

	int mesh_progress_count;		// [0, MAX_PROGRESS_COUNT]
	bool interrupt;

	int start_id;

	void init_global_mesh_option();
	bool get_interrupt_status();
	//
	//
	int mesh2();
	int tetrahedralizeCavity(pAssemblyMesh);
	void toMeshDB(pMWMesh db, pMesh_t p_mesh);
	//
	int convexHull(pAssemblyMesh);	
	int cxhull(pAssemblyMesh);
	int shrinkwrap(pAssemblyMesh);
	int shrinkwrap2(int, node_t *, int, int *, double, double, int, int);
	int fillhole(pAssemblyMesh);
	int fillhole2(int, node_t *, int, int *, double, double);
	void ta_toMeshDB(pMWMesh db, pMesh);
	//
	int ta_mesher();
	void ta_toMeshDB2(pMWMesh db, pMesh);
	//
};

#endif