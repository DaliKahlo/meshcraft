//$c3   XRL 11/11/2011 support iso-mesh faces, and platform dependent baseline values. 
//$c2   XRL 06/20/2011 Read cylinder feature data 
//$c1   XRL 04/04/2011 Created
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#include "MeshControls.h"
#include "mgc_api.h"
#include "MGCParaInterface.h"
#include <algorithm>
#include <assert.h>

#ifdef _DEBUG
   #ifndef DBG_NEW
      #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
      #define new DBG_NEW
   #endif
#endif  // _DEBUG

MeshControl::MeshControl()
{
	bl.t_premesh	= 0.0;
	bl.t_mesh		= 0.0;
	bl.elem_count	= 0;
	bl.node_count	= 0;
	bl.bad_elem_count = 0;
	bl.worst		= 0.0;

	lphysizes = 0;
	lgeosizes = 0;
	seeds = 0;
	suppressVertices = 0;
	suppressEdges = 0;
	suppressBodies = 0;
}

MeshControl::~MeshControl()
{
	if(lphysizes) delete lphysizes;
	if(lgeosizes) delete lgeosizes;
	if(seeds) {
		std::list<MeshSeed>::iterator iter;
		for( iter=seeds->begin(); iter!=seeds->end(); iter++ ) {
			if( iter->fdata )
				delete [] iter->fdata;
		}
		delete seeds;
	}
	if(suppressVertices) delete suppressVertices;
	if(suppressEdges) delete suppressEdges;
	if(suppressBodies) delete suppressBodies;
	if(faceHPoints.size()>0) {
		std::list<HardPoints>::iterator iter;
		for( iter=faceHPoints.begin(); iter!=faceHPoints.end(); iter++ ) {
			if( iter->num > 0 )
				delete [] iter->params;
		}
	}
	if(edgeHPoints.size()>0) {
		std::list<HardPoints>::iterator iter;
		for( iter=edgeHPoints.begin(); iter!=edgeHPoints.end(); iter++ ) {
			if( iter->num > 0 )
				delete [] iter->params;
		}
	}
	if(cylinders.size()>0) {
		cylinders.clear();
	}
	if(isomeshfaces.size()>0) {
		isomeshfaces.clear();
	}
}

/********************************************/
/*			local sizing/seeding			*/
void MeshControl::add_local_physical_size(LocalPhySize &lsize)
{
	if( !lphysizes )
		lphysizes = new std::list<LocalPhySize>;
	lphysizes->push_back(lsize);
}

void MeshControl::add_local_geometric_size(LocalGeoSize &lsize)
{
	if( !lgeosizes )
		lgeosizes = new std::list<LocalGeoSize>;
	lgeosizes->push_back(lsize);
}

void MeshControl::add_mesh_seed(MeshSeed &seed)
{
	if( !seeds )
		seeds = new std::list<MeshSeed>;
	seeds->push_back(seed);
}

/****************************************/
/*				 suppress				*/
void MeshControl::suppress_edge(GID &id)
{
	if( !suppressEdges )
		suppressEdges = new std::list<GID>;
	suppressEdges->push_back(id);
}

void MeshControl::suppress_vertex(GID &id)
{
	if( !suppressVertices )
		suppressVertices = new std::list<GID>;
	suppressVertices->push_back(id);
}

void MeshControl::suppress_body(int id)
{
	if( !suppressBodies )
		suppressBodies = new std::vector<int>;
	suppressBodies->push_back(id);
}

bool MeshControl::body_suppressed(int compid)
{
	if( suppressBodies ) {
		std::vector<int>::iterator result = find(suppressBodies->begin(), suppressBodies->end(), compid);
		if( result == suppressBodies->end() )
			return false;
		return true;
	}
	return false;
}

bool MeshControl::mdb_required()
{
	if(isomeshfaces.size() > 0 || cylinders.size() > 0)
		return true;
	return false;
}

/****************************************************/
/*		interatct with file, rtest and mdmgc		*/

/* the memories allocated in read() will be passed around (instead of copying) in distenemesh    */
/* it will (should only) be deleted in destructor. Otherwise it becomes dangling pointer in mesh */
void MeshControl::read(FILE *f, char* mach)
{
	int i, j, num;
	GID ent;
	char str[128], str2[128];

	while( fscanf(f," %s",&str) != EOF ) {
		if ( strcmp(str,"#")==0 ) {
			fgets(str,128,f);
		} else if( strcmp(str,"local_physical_size")==0 ) {
			// local constant physical size
			LocalPhySize lphysiz;
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %ld %lf",&(lphysiz.id.cid),
									 &(lphysiz.id.id),
									 &(lphysiz.h));
				lphysiz.h /= 1000.0;  // convert into meters
				add_local_physical_size(lphysiz);
			}
		} else if( strcmp(str,"local_geometric_size")==0 ) {
			// local geometric size
			LocalGeoSize lgeosiz;
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %ld %lf %lf %lf %lf",&(lgeosiz.id.cid),
											  &(lgeosiz.id.id),
											  &(lgeosiz.max_angle_error),
											  &(lgeosiz.max_distance_error),
											  &(lgeosiz.h_min),
											  &(lgeosiz.h_max));
				lgeosiz.max_distance_error /= 1000.0;
				lgeosiz.h_min /= 1000.0;
				lgeosiz.h_max /= 1000.0;
				add_local_geometric_size(lgeosiz);
			}
		} else if ( strcmp(str,"mesh_seeds")==0 ) {
			MeshSeed seed;
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				seed.fdata = NULL;
				fscanf(f,"%d",&(seed.atype));
				if( seed.atype == MGC_ATYPE_UNIFORM ) {
					fscanf(f,"%d %ld %d",&(seed.id.cid),&(seed.id.id),&(seed.numSeed));
					add_mesh_seed(seed);
				} else if(seed.atype == MGC_ATYPE_ONE_WAY_BIAS_NUMSEED_RATIO || seed.atype == MGC_ATYPE_TWO_WAY_BIAS_NUMSEED_RATIO) {
					seed.fdata = new double[1];
					fscanf(f,"%d %ld %d %lf",&(seed.id.cid),&(seed.id.id),&(seed.numSeed),&(seed.fdata[0]));
					add_mesh_seed(seed);
				} else if( seed.atype == MGC_ATYPE_ONE_WAY_BIAS_SIZE1_SIZE2 || seed.atype == MGC_ATYPE_TWO_WAY_BIAS_SIZE1_SIZE2) {
					seed.fdata = new double[2];
					fscanf(f,"%d %ld %lf %lf",&(seed.id.cid),&(seed.id.id),&(seed.fdata[0]),&(seed.fdata[1]));
					seed.numSeed = 0;
					seed.fdata[0] /= 1000.0;
					seed.fdata[1] /= 1000.0;
					add_mesh_seed(seed);
				} else if(seed.atype == MGC_ATYPE_TABULAR_PARAMETRIC || seed.atype == MGC_ATYPE_TABULAR_ARC_LENGTH) {
					fscanf(f,"%d %ld %d",&(seed.id.cid),&(seed.id.id),&(seed.numSeed));
					if(seed.numSeed > 0) {
						seed.fdata = new double[seed.numSeed];
						for(j=0; j<seed.numSeed; j++) {
							fscanf(f,"%lf",&(seed.fdata[j]));
						}
						add_mesh_seed(seed);
					}
				} else if(seed.atype == MGC_ATYPE_TABULAR_NODE_POINT) {
					fscanf(f,"%d %ld %d",&(seed.id.cid),&(seed.id.id),&(seed.numSeed));
					if(seed.numSeed > 0) {
						seed.fdata = new double[3 * seed.numSeed + 1];
						double x,y,z;
						fscanf(f,"%lf",&x);
						seed.fdata[0] = x/1000.0;
						for(j=0; j<seed.numSeed; j++) {
							fscanf(f,"%lf %lf %lf",&x,&y,&z);
							seed.fdata[3*j+1] = x/1000.0;
							seed.fdata[3*j+2] = y/1000.0;
							seed.fdata[3*j+3] = z/1000.0;
						}
						add_mesh_seed(seed);
			}
		}
	}
		} else if ( strcmp(str,"suppress_edges")==0 ) {
			// virtual edges
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %d",&(ent.cid),&(ent.id));
				suppress_edge(ent);
}
		} else if ( strcmp(str,"suppress_vertices")==0 ) {
			// virtual vertices
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %d",&(ent.cid),&(ent.id));
				suppress_vertex(ent);
}
		} else if ( strcmp(str,"suppress_bodies")==0 ) {
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d",&(ent.cid));
				suppress_body(ent.cid);
			}
		} else if( strcmp(str,"edge_hard_points")==0 ) {
			fscanf(f,"%d",&num);
			HardPoints hpt;
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %d %d",&(hpt.id.cid),&(hpt.id.id),&(hpt.num));
				if( hpt.num > 0 ) {
					hpt.params = new double[hpt.num];
					for( int j=0; j<hpt.num; j++ ) {
						fscanf(f,"%lf",&(hpt.params[j]));
				}
					edgeHPoints.push_back(hpt);
			}
			}
		} else if( strcmp(str,"face_hard_points")==0 ) {
			fscanf(f,"%d",&num);
			HardPoints hpt;
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %d %d",&(hpt.id.cid),&(hpt.id.id),&(hpt.num));
				if( hpt.num > 0 ) {
					hpt.params = new double[2*hpt.num];
					for( int j=0; j<hpt.num; j++ ) {
						fscanf(f,"%lf %lf",&(hpt.params[2*j]),&(hpt.params[2*j+1]));
					}
						faceHPoints.push_back(hpt);
				}
			}
		} else if( strcmp(str,"cylinder_feature")==0 ) {
			GID tag2;
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d %d",&(tag2.cid),&(tag2.id));
				cylinders.push_back(tag2);
			}
		} else if( strcmp(str,"baseline")==0 ) {
			int length = (int) strlen(mach);
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				if( length > 0 ) {
					fscanf(f," %s",&str2);
					if( strcmp(str2,mach)==0 ) {
						fscanf(f,"%lf %lf %d %d %d %lf",&(bl.t_premesh), &(bl.t_mesh), &bl.elem_count, &bl.node_count, &bl.bad_elem_count, &bl.worst);
						break;
					} 
				}
				fgets(str,128,f);
			}
		} else if( strcmp(str,"iso_mesh_faces")==0 ) {
			fscanf(f,"%d",&num);
			for( i=0; i<num; i++ ) {
				fscanf(f,"%d",&j);
				isomeshfaces.push_back(j);
			}
		} else {
			fgets(str,128,f);
		}
	}
}

int MeshControl::check_baseline(FILE *f, MeshEvalData &actual)
{
	double tol1 = 0.98;
	double tol2 = 1.02;
	if(bl.t_premesh > 0) {
		if (actual.t_premesh > bl.t_premesh*tol2 ) {
			fprintf(f,"Failed premesh effeciency check: %f vs. %f\n",actual.t_premesh,bl.t_premesh);
			printf("Failed premesh effeciency check: %f vs. %f\n",actual.t_premesh,bl.t_premesh);
		}
	}
	if(bl.t_mesh > 0) {
		if (actual.t_mesh > bl.t_mesh*tol2 ) {
			fprintf(f,"Failed mesh effeciency check: %f vs. %f\n",actual.t_mesh,bl.t_mesh);
			printf("Failed mesh effeciency check: %f vs. %f\n",actual.t_mesh,bl.t_mesh);
		}
	}
	if(bl.elem_count > 0) {
		if (actual.elem_count < bl.elem_count*tol1 ) {
			fprintf(f,"Failed element count check: %d fewer\n", (bl.elem_count - actual.elem_count));
			printf("Failed element count check: %d fewer\n", (bl.elem_count - actual.elem_count));
		}
		if (actual.elem_count > bl.elem_count*tol2 ) {
			fprintf(f,"Failed element count check: %d more\n", (actual.elem_count - bl.elem_count));
			printf("Failed element count check: %d more\n", (actual.elem_count - bl.elem_count));
		}
	}
	if(bl.node_count > 0) {
		if (actual.node_count < bl.node_count*tol1 ) {
			fprintf(f,"Failed node count check: %d fewer\n", (bl.node_count - actual.node_count));
			printf("Failed node count check: %d fewer\n", (bl.node_count - actual.node_count));
		}
		if (actual.node_count > bl.node_count*tol2 ) {
			fprintf(f,"Failed node count check: %d more\n", (actual.node_count - bl.node_count));
			printf("Failed node count check: %d more\n", (actual.node_count - bl.node_count));
		}
	}
	if(bl.bad_elem_count > 0) {
		if (actual.bad_elem_count > bl.bad_elem_count*tol2 ) {
			fprintf(f,"Failed node count check: %d vs. %d\n", actual.bad_elem_count,bl.bad_elem_count);
			printf("Failed node count check: %d vs. %d\n", actual.bad_elem_count,bl.bad_elem_count);
		}
	}
	if(bl.worst > 0) {
		if (actual.worst > bl.worst*tol2 ) {
			fprintf(f,"Failed element quality check: %f vs %f\n", actual.worst,bl.worst);
			printf("Failed element quality check: %f vs %f\n", actual.worst,bl.worst);
		}
	}
	return 0;
}

// must set before premesh
void MeshControl::set_mdmgc1(pMGC_t p_mgc)
{
	if( faceHPoints.size() > 0 )
	{
		std::list<HardPoints>::iterator fpIter;
		for( fpIter=faceHPoints.begin(); fpIter!=faceHPoints.end(); fpIter++ )
		{
			MGC_set_local_attribute(p_mgc,MGC_ATYPE_HARD_FACE_POINT,fpIter->id,fpIter->num,fpIter->params);
		}
	}

	// must precede premesh, because it will be used by seeding which must before premesh
	if( edgeHPoints.size() > 0 )
	{
		std::list<HardPoints>::iterator epIter;
		for( epIter=edgeHPoints.begin(); epIter!=edgeHPoints.end(); epIter++ )
		{
			MGC_set_local_attribute(p_mgc,MGC_ATYPE_HARD_EDGE_POINT,epIter->id,epIter->num,epIter->params);
		}
	}

	if( seeds ) 
	{
		std::list<MeshSeed>::iterator seedIter;
		for( seedIter=seeds->begin(); seedIter!=seeds->end(); seedIter++ )
		{
			MGC_set_local_attribute(p_mgc,seedIter->atype,seedIter->id,seedIter->numSeed,(seedIter->fdata));
		}
	}

	return;
}

// before/after does not matter
void MeshControl::set_mdmgc2(pMGC_t p_mgc, pGEntity_t *comps)
{
	double h[4];

	if( lphysizes ) 
	{
		std::list<LocalPhySize>::iterator phyIter;
		for( phyIter=lphysizes->begin(); phyIter!=lphysizes->end(); phyIter++ )
		{
			h[0] = phyIter->h;
			MGC_set_local_attribute(p_mgc,MGC_ATYPE_MESH_SIZE,phyIter->id,0,h);
		}
	}

	if( lgeosizes ) 
	{
		std::list<LocalGeoSize>::iterator geoIter;
		for( geoIter=lgeosizes->begin(); geoIter!=lgeosizes->end(); geoIter++ )
		{
			h[0] = geoIter->max_angle_error;
			h[1] = geoIter->max_distance_error;
			h[2] = geoIter->h_max;
			h[3] = geoIter->h_min;
			MGC_set_local_attribute(p_mgc,MGC_ATYPE_MESH_CURVATURE_SIZE,geoIter->id,0,h);
		}
	}

	if(suppressEdges && comps) 
	{
		std::list<GID>::iterator edgeIter;
		for( edgeIter=suppressEdges->begin(); edgeIter!=suppressEdges->end(); edgeIter++ )
		{
			pGEntity_t e = gid2pGEntity(MGC_GTYPE_EDGE,comps[edgeIter->cid - 1], edgeIter->id);
			if( e )
				MGC_suppress_geometry_edge(p_mgc, e );
		}
	}

	return;
}