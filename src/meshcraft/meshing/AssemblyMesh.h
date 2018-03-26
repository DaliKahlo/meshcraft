//$c6   XRL 03/28/2014 add attributes
//$c5   XRL 02/25/2014 Add m_name
//$c4   XRL 04/01/2013 Added toBDF
//$c3   XRL 11/08/2012 Added findMeshEntityFromIDs
//$c2   XRL 07/14/2012 Added constructor for unclassified mesh
//$c1   XRL 02/01/2012 Created 
//========================================================================//
//
// assembly.h: the assembly topology object (equivalent to SW's BodyMgr)
//
//========================================================================//

#ifndef ASSEMBLY_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define ASSEMBLY_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#include <vector>
#include <set>
#include <osg/Matrix>
#include "instance.h"
#include "../../mesh/mesh_api.h"
#include "parasolid_kernel.h"

enum SGSTATE 
{ 
	NOT_IN_SG, 
	SWITCH_ON,
	SWITCH_OFF
};

enum ATTRTYPE 
{ 
	NO_ATTRIBUTE, 
	VERTEX_VECTOR_SCALAR,
	FACE_SCALAR2
};

class AssemblyMesh  
{
public:
	AssemblyMesh(int, PK_BODY_t *, PK_TRANSF_t*); 	
	AssemblyMesh();
	virtual ~AssemblyMesh();

	// set/get overall parameters
	int  getID() { return m_assem_id; }
	void setID(int id) { m_assem_id = id; }
	void setName(char *name);
	char* getName();
	SGSTATE getSgState() { return m_sgState; }
	void setSgState(SGSTATE);
	void setSgConsist(bool c);
	bool isSgConsist() { return m_sgConsist;}

	// interrogate instance and mesh
	int numInstances() { return m_instances.size(); }
	Instance * ithInstance(int i) { return &(m_instances[i]); }
	int numInstancedBodies() { return m_bodyArray.size(); }
	PK_BODY_t ithInstancedBody(int i) { return m_bodyArray[i]; }
	pMWMesh ithInstancedMesh(int i) { return m_meshArray[i]; }

	// look up mesh entities
	void countEntities(int *nv, int *nf, int *nq, int *nr);
	pEntity findMeshEntityFromIDs(int instance_id, int face_id, int entity_index, osg::Matrix **transf);
	void getNeighboringFacesFromVertexID(int instance_id, int vertex_id, std::set<pFace> &faces);
	void getRegionsFromShapeThreshold(double, std::vector<pRegion> &);
	pFace ithFaceOfArray(int ith, osg::Matrix **transf);
	pVertex ithVertexOfArray(int ith, osg::Matrix **transf);
	int dataFromXYZ(double picked[3], double pc[3], double proj[3], double *val);
	int vertexIndexFromXYZ(double picked[3]);

	int toArray(int *pnv, double **pxyz, int *pnf, int **ptria);
	void toBDF(const char *fileName);

	// attribute
	ATTRTYPE attr_type() { return m_attype; }
	void attr_set(ATTRTYPE, double *, double, double);
	void attr_get(int *, double **, double *, double*);

	// modify 
	void postMult(osg::Matrix&);
	int  deleteVertices(int np, double *coord, int *index);

protected:
	// imprint();

private:
	char m_name[128];

	// a collection of instances. Ok to use list
	std::vector<Instance> m_instances;

	// instanced bodies and meshes
	std::vector<PK_BODY_t> m_bodyArray;
	std::vector<pMWMesh> m_meshArray;

	// scene graph data
	SGSTATE m_sgState;
	bool m_sgConsist;				// if false, mesh is changed while SG is updated
	int m_assem_id;					// start from 1

	//// touching topologies
	//// always empty for incongruent mesh
	//// automatically set during imprint
	//// they are topology that holds mesh entities
	//// node id can be referenced by body mesh, and always be the smallest 
	//// 
	//std::list<GFacePair> moFacePair;
	//std::list<GEdgeSet>  moEdgeSet;
	//std::list<GVertexSet> moVertexSet;

	// index based attributes (consistent with toArray mesh entity traversal)
	ATTRTYPE m_attype;
	int m_attnum;
	double *m_pDblattr;
	double m_attmax, m_attmin;

	//
	void attr_init();
	void attr_clean();

	int toArray(int *pnv, double **pxyz, pVertex **pvert, int *pnf, int **ptria);
};

#endif
