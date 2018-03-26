//$c5   XRL 11/11/2011 Added existing mesh support.
//$c4   XRL 08/18/2011 Added mesh seeding.
//$c3   XRL 08/05/2011 Added local geometric sizing and baseline_check().
//$c2   XRL 04/29/2011 Added "suppress_body()".
//$c1   XRL 04/01/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//		Add all local mesh controls here, including:					  //	
//		  1. mesh size on vertices/edges/faces;							  //
//		  2. mesh seeds													  //
//		  3. suppressed model vertices/edges;							  //
//		  4. existed mesh on edges/faces to be respected;				  //
//		  5. hard points on edges/faces;								  //
//========================================================================//

#ifndef MESHCONTROLS_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHCONTROLS_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include <stdio.h>
#include <list>
#include <vector>
#include "mgc_typedef.h"

// ---------------------------------------------------- // 
typedef struct {
	float t_premesh;
	float t_mesh;
	int elem_count;
	int node_count;
	int bad_elem_count;		// AR < 100
	double	worst;			// AR for tet, qcheck for tri/quad
} MeshEvalData;

typedef struct {
	GID id;
	double h;
} LocalPhySize;

typedef struct {
	GID id;
	double max_angle_error;
	double max_distance_error;
	double h_max;
	double h_min;
} LocalGeoSize;

typedef struct {
	GID id;
	MGCMeshAttibType atype;
	int numSeed;
	double *fdata;
} MeshSeed;

typedef struct {
	GID id;
	int num;
	double *params;
} HardPoints;

typedef struct {
	int cid;
	int num;
	int *entities;
	MGCMeshAttibType ttype;
	double data[18];
} MatchInfo;

class MeshControl {
public:
	MeshControl();
	~MeshControl();

	void read(FILE *, char *);
	void set_mdmgc1(pMGC_t);
	void set_mdmgc2(pMGC_t, pGEntity_t *);
	int check_baseline(FILE *, MeshEvalData &);

	bool mdb_required();
	std::list<int> *get_iso_mesh_faces() { return &isomeshfaces; }
	std::list<MeshSeed> *get_seeds() { return seeds; }
	bool body_suppressed(int);


private:
	MeshEvalData bl;

	// local sizes and seeds
	std::list<LocalPhySize> *lphysizes;
	std::list<LocalGeoSize> *lgeosizes;
	std::list<MeshSeed> *seeds;

	// suppressed entities
	std::list<GID> *suppressVertices;
	std::list<GID> *suppressEdges;
	std::vector<int> *suppressBodies;

	// hard points
	std::list<HardPoints> edgeHPoints;
	std::list<HardPoints> faceHPoints;

	// Mesh Match pairs
	std::list<MatchInfo> matchlist;

	// faces with given iso mesh or to be generated by any mesher
	std::list<int> isomeshfaces;

	// feature data
	std::list<GID> cylinders;

protected:
	// local sizing and seeding
	void add_local_physical_size(LocalPhySize&);
	void add_local_geometric_size(LocalGeoSize&);
	void add_mesh_seed(MeshSeed&);

	// suppress 
	void suppress_vertex(GID&);
	void suppress_edge(GID&);
	void suppress_body(int);

	// features
	std::list<GID> *get_cylinder_feature_faces() { return &cylinders; }

};

#endif