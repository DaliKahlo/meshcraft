//$c6 XRL 02/20/14 Used osg::Matrix to present transform, added preMult()
//$c5 XRL 11/20/13 Fix writing face element bug using toBDF 
//$c4 XRL 06/12/13 toBDF supports CTETRA element. Fixed memory leaks in toBDF.
//$c3 XRL 04/01/13 Added toBDF
//$c2 XRL 11/08/12 added findMeshEntityFromIDs
//$c1 XRL 02/01/12 created
//================================================================
//
// Assembly.cpp: implementation of the AssemblyMesh class.
//
//================================================================

#include "stdafx.h"
#include "util.h"
#include "AssemblyMesh.h"
//#include <osg/Vec3d>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// constructor for parasolid model
AssemblyMesh::AssemblyMesh(int nBody, PK_BODY_t *bodies, PK_TRANSF_t *transfs)
{
	bool found;
	int i,j,k;

	for(i=0; i<nBody; i++) {
		found=false;
		k = (int)m_bodyArray.size();
		for(j=0; j<k; j++) {
			if(m_bodyArray[j] == bodies[i])
			{ found=true; break; }
		}
		if(!found) {
			m_bodyArray.push_back(bodies[i]);
			m_meshArray.push_back( M_new(bodies[i]) );
		}

		Instance inst(i+1,j);
		if(transfs != NULL)
		{
			PK_TRANSF_sf_t sf;
			PK_ERROR_code_t err = PK_TRANSF_ask(transfs[i],&sf);
			if( err )
				TRACE("error %d from PK_TRANSF_ask!!!",err);

			inst.setTransform(sf.matrix[0][0],sf.matrix[1][0],sf.matrix[2][0],sf.matrix[3][0],
							  sf.matrix[0][1],sf.matrix[1][1],sf.matrix[2][1],sf.matrix[3][1],
							  sf.matrix[0][2],sf.matrix[1][2],sf.matrix[2][2],sf.matrix[3][2],
							  sf.matrix[0][3],sf.matrix[1][3],sf.matrix[2][3],sf.matrix[3][3]);
		}
		m_instances.push_back(inst);
	}

	strcpy( m_name, "");
	m_sgState = NOT_IN_SG;
	m_sgConsist = true;
	m_assem_id = 0;

	attr_init();
}

// constructor for mesh without a model
// for now, we just consider one mesh with a transf
AssemblyMesh::AssemblyMesh()
{
	m_bodyArray.push_back(0);
	m_meshArray.push_back( M_new(NULL) );

	// occurrance 1, index in m_bodyArray 0, no transformation
	Instance inst(1,0);
	m_instances.push_back(inst);

	strcpy( m_name, "");
	m_sgState = NOT_IN_SG;
	m_sgConsist = true;
	m_assem_id = 0;

	attr_init();
}

AssemblyMesh::~AssemblyMesh()
{
	attr_clean();
	std::vector<pMWMesh>::iterator iter;
	for(iter=m_meshArray.begin(); iter!=m_meshArray.end(); iter++) {
		if( *iter )
			M_delete(*iter);
	}
}

void AssemblyMesh::attr_init()
{
	m_attype = NO_ATTRIBUTE;
	m_attnum = 0;
	m_pDblattr = NULL;
	m_attmax = m_attmin = 0.0;
}

void AssemblyMesh::attr_clean()
{
	if(m_pDblattr)	 delete [] m_pDblattr;
}

void AssemblyMesh::attr_set(ATTRTYPE ty, double *p, double dmin, double dmax)
{
	attr_clean();
	m_attype = ty;
	switch(ty) {
	case VERTEX_VECTOR_SCALAR: m_attnum = 4; break;
	case FACE_SCALAR2: m_attnum = 2; break;
	}
	m_pDblattr = p;
	m_attmax = dmax;
	m_attmin = dmin;
}

void AssemblyMesh::attr_get(int *pN, double **ppDbl, double *pDmin, double *pDmax)
{
	*pN = m_attnum;
	*ppDbl = m_pDblattr;
	*pDmin = m_attmin;
	*pDmax = m_attmax;
}

void AssemblyMesh::setName(char *name)
{
	strcpy( m_name, name);
}
	
char* AssemblyMesh::getName()
{
	return m_name;
}

void AssemblyMesh::setSgState(SGSTATE state) 
{ m_sgState = state; }

void AssemblyMesh::setSgConsist(bool c) 
{ m_sgConsist=c;}


pEntity AssemblyMesh::findMeshEntityFromIDs(int occurance, int gface_id, int entity_index, osg::Matrix **transf)
{
	// note we are using instance id, not occurance

	// find a body and its mesh from instance id
	Instance *inst = ithInstance(occurance - 1);
	int bIndex = inst->body_index();
	PK_BODY_t bo = ithInstancedBody(bIndex);
	pMWMesh pME = ithInstancedMesh(bIndex);

	if(inst->isIdentity())
		*transf = NULL;
	else
		*transf = inst->transform();

	// determine the geometry face or a NULL face (face id zero)
	PK_FACE_t gf = NULL;
	if(bo > 0) {
		PK_FACE_t *faces = NULL;
		int nGF, fid;
		PK_BODY_ask_faces(bo,&nGF,&faces);
		for(int i=0; i<nGF; i++){
			PK_ENTITY_ask_identifier(faces[i],&fid);
			if(gface_id == fid) {
				gf = faces[i];
				break;
			}
		}
		if(faces) 
			PK_MEMORY_free(faces);
		else
			return NULL;
	}

	// find the mesh face
	pFace f = NULL;
	int num, index=0;
	pFaceList flist = M_getClassifiedFaces(pME,gf,&num);		// gf can be NULL
	if(flist) {
		pFaceIter face_iter = FIter(flist); 
		while( f = FIter_next(face_iter) ){
			if(EN_getID((pEntity)f) < 0)			// this face is not included as a primitive
				continue;
			if(index == entity_index)
				break;
			index++;
			if(F_numVertices(f) == 4) {
				if(index == entity_index)
					break;
				index++;
			}
		}
		FIter_delete(face_iter);
	}

	return (pEntity)f;
}

void AssemblyMesh::getNeighboringFacesFromVertexID(int occurance, int vertex_id, std::set<pFace> &faces)
{
	for(int j=0; j<numInstances(); j++) {
		Instance *inst = ithInstance(j);
		int bIndex = inst->body_index(); 

		PK_BODY_t bo = ithInstancedBody(bIndex);
		pMWMesh pME = ithInstancedMesh(bIndex);

		pFaceList flist;
		Face *f = NULL;
		pVertex vertex;
		int i, k, num, nv;
		PK_FACE_t gf = NULL;
		if(bo > 0) {
			PK_FACE_t *pkfaces = NULL;
			int nGF;
			PK_BODY_ask_faces(bo,&nGF,&pkfaces);
			for(i=0; i<nGF; i++){
				//PK_ENTITY_ask_identifier(pkfaces[i],&fid);
				flist = M_getClassifiedFaces(pME,pkfaces[i],&num);		
				if(flist) {
					pFaceIter face_iter = FIter(flist);
					while( f = FIter_next(face_iter) ){
						nv = F_numVertices(f);
						for(k=0; k<nv; k++) {
							vertex = F_vertex(f,k);
							if(EN_getID((pEntity)vertex) == vertex_id) {
								faces.insert(f);
							}
						}
					}
					FIter_delete(face_iter);
				}
			}
			if(pkfaces) 
				PK_MEMORY_free(pkfaces);
		}
		
		// loop over mesh faces not classified
		flist = M_getClassifiedFaces(pME,NULL,&num);		// gf can be NULL
		if(flist) {
			pFaceIter face_iter = FIter(flist); 
			while( f = FIter_next(face_iter) ){
				nv = F_numVertices(f);
				for(i=0; i<nv; i++) {
					vertex = F_vertex(f,i);
					if(EN_getID((pEntity)vertex) == vertex_id) {
						faces.insert(f);
					}
				}
			}
			FIter_delete(face_iter);
		}
	}
	return;
}


void AssemblyMesh::getRegionsFromShapeThreshold(double threshold, std::vector<pRegion> &rgns)
{
	pRegion rgn;
	pVertex vt;
	double shp, xyz[4][3];
	double cubicThre = threshold*threshold*threshold;
	int i,j,nr;

	for(i=0; i<numInstances(); i++) {
		Instance *inst = ithInstance(i);
		int bIndex = inst->body_index(); 

		PK_BODY_t bo = ithInstancedBody(bIndex);
		pMWMesh pME = ithInstancedMesh(bIndex);

		pRegionList rList = M_getClassifiedRegions(pME,NULL,&nr);
		if(rList) {
			pRegionIter rIt = RIter(rList);
			while( rgn = RIter_next(rIt) ) {
				for(j=0; j<4;j++) {
				   vt = R_vertex(rgn,j);
				   V_getCoord(vt,xyz[j]);
				}
				if(cubicMeanRatio(xyz,&shp) && shp > cubicThre)
					continue;
				rgns.push_back(rgn);
			}
			RIter_delete(rIt);
		}
	}

	return;
}

// return both pVertex array and coordinate array in world system
//        Note: the coordinates with pVertex are local
int AssemblyMesh::toArray(int *pnv, double **pxyz, int *pnf, int **ptria)
{
	pVertex *verts = NULL;
	int ret = toArray(pnv,pxyz,&verts,pnf,ptria);
	if(verts) delete [] verts;
	return ret;
}

int AssemblyMesh::toArray(int *pnv, double **pxyz, pVertex **pvert, int *pnf, int **ptria)
{
	*pvert = NULL;
	*pxyz =  NULL;
	*ptria = NULL;

	// count
	pMWMesh me;
	int i,k,nv=0, nf=0, nr=0, nq=0;
	for(i=0; i<this->numInstances(); i++) {
		Instance *inst = this->ithInstance(i);
		k = inst->body_index();
		me = this->ithInstancedMesh(k);
		nv += M_numberVertices(me);
		nf += M_numberFaces(me);
		nr += M_numberRegions(me);
		nq += M_numberQuadras(me);
	}
	if(nr>0 || nq>0)
		return 1001;
	if(nv<3)
		return 1002;

	// initial arrays
	*pnv = nv;
	*pnf = nf;
	double *xyz = new double[nv*3];
	*pxyz = xyz;
	*pvert = new pVertex[nv];
	memset((char *)(*pxyz),'\0',sizeof(double)*nv*3);
	memset((char *)(*pvert),'\0',sizeof(pVertex)*nv);
	if(nf > 0) {
		*ptria = new int[nf*3];
		memset((char *)(*ptria),'\0',sizeof(int)*nf*3);
	}

	int _nf, _nv, accu_nv=0, accu_nf=0, count=0, offset;
	int *tria;
	double coord[3];
	pVertex *vert;
	for(i=0; i<this->numInstances(); i++) {
		Instance *inst = this->ithInstance(i);
		k = inst->body_index();
		me = this->ithInstancedMesh(k);

		offset = count / 3;
		M_toARRAY(me,&_nf,&_nv,&tria, &vert);

		if(!inst->isIdentity()) {
			osg::Matrix * transf = inst->transform();
			osg::Vec3d pt;
			for(k=0; k<_nv; k++) {
				V_getCoord(vert[k],coord);
				pt.set(coord[0],coord[1],coord[2]);
				pt = transf->preMult(pt);
				xyz[count++] = pt.x();
				xyz[count++] = pt.y();
				xyz[count++] = pt.z();
			}
		} else {
			for(k=0; k<_nv; k++) {
				V_getCoord(vert[k],coord);
				xyz[count++] = coord[0];
				xyz[count++] = coord[1];
				xyz[count++] = coord[2];
			}
		}

		if(offset != 0) {
			for(k=0; k<3*_nf; k++) {
				tria[k] += offset;
			}
		}

		if(_nf > 0)
			memcpy(&((*ptria)[3*accu_nf]),tria,3*_nf*sizeof(int)); // assumed the index of element array starts from 1 
		memcpy(&((*pvert)[accu_nv]), vert, _nv*sizeof(pVertex)); // assumed the index of element array starts from 1 

		accu_nf += _nf;
		accu_nv += _nv;
		delete [] tria;
		delete [] vert;
	} 

	return 0;
}

pFace AssemblyMesh::ithFaceOfArray(int ith, osg::Matrix **transf)
{
	pMWMesh me;
	int i,k,index=0, nf=0;
	for(i=0; i<this->numInstances(); i++) {
		Instance *inst = this->ithInstance(i);
		k = inst->body_index();
		me = this->ithInstancedMesh(k);
		nf = M_numberFaces(me);
		if(ith <= index+nf) {
			*transf = inst->transform();
			return M_ithFaceOfARRAY(me, ith - index);
		}
		index += nf;
	}
	return NULL;
}

pVertex AssemblyMesh::ithVertexOfArray(int ith, osg::Matrix **transf)
{
	pMWMesh me;
	int i,k,index=0, nv=0;
	for(i=0; i<this->numInstances(); i++) {
		Instance *inst = this->ithInstance(i);
		k = inst->body_index();
		me = this->ithInstancedMesh(k);
		nv = M_numberVertices(me);
		if(ith <= index+nv) {
			*transf = inst->transform();
			return M_ithVertexOfARRAY(me, ith - index);
		}
		index += nv;
	}
	return NULL;
}

int AssemblyMesh::vertexIndexFromXYZ(double picked[3])
{
	pMWMesh me;
	osg::Matrix *transf;
	int i,k,local_index, index=0, nv=0;
	for(i=0; i<this->numInstances(); i++) {
		Instance *inst = this->ithInstance(i);
		k = inst->body_index();
		me = this->ithInstancedMesh(k);
		transf = inst->transform();
		local_index = M_vertexIndexFromXYZ(me, picked);   // always identity transform for cloud points
		if(local_index) {								  // index starts from 1; zero indicates "not found"
			return index + local_index;
		}
		nv = M_numberVertices(me);
		index += nv;
	}
	return 0;
}

/// need rework, this function does not consider instancing.  -2/20/2014
void AssemblyMesh::toBDF(const char *fileName)
{
  PK_FACE_t *faces = NULL;
  PK_EDGE_t *edges = NULL;
  PK_VERTEX_t *vertices;
  int nGV, nGE, nGF;
  int i, j, k, count;
  double xyz[3];

  Vertex *vertex;
  Face *face;

  // Initialize the global ids
  int nMaterialId = 1 ;
  int nPropertyId = 1 ;
  int nPropertyId2 = 2 ;

  //Open the file for writing
  FILE *fp = NULL ;
  if ( fileName == NULL )
	  return;
  fp = fopen(fileName,"w") ;
  if ( fp == NULL )
	  return;

  //Write the bdf headders
  fprintf(fp,"TITLE = generated using MeshWorks v1.11 (Sat 20 11:16:56 2012) \n" ) ;
  fprintf(fp,"BEGIN BULK\n") ;

  //Create one material card
  fprintf( fp, "MAT1    %-8d%-8.1e        %-8f%-8.1e%-8.1e\n", 
           nMaterialId, 3e7, 0.33, 6.5e-6, 5.37e2) ;

  // write vertices. 
  std::vector<pMWMesh>::iterator iter;
  count = 0;
  for(iter=m_meshArray.begin(); iter!=m_meshArray.end(); iter++) {
	  int nbv = 0;
	  pMWMesh pME = *iter;
	  pGEntity classifiedOn = M_model(pME);

	  pVertexList vlist;
	  nGV = 0;
	  PK_BODY_ask_vertices(classifiedOn,&nGV,&vertices);
	  for(i=0; i<nGV; i++){
		  vlist = M_getClassifiedVertices(pME,vertices[i],&nbv);
		  if( vlist ) {
			  pVertexIter gv_vtx_iter = VIter(vlist);
			  while( vertex = VIter_next(gv_vtx_iter) ){
				  count++;
				  V_getCoord(vertex,xyz);
				  fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
				           count, xyz[0], xyz[1], xyz[2] ) ;
				  V_setTempID(vertex,count);
			  }
			  VIter_delete(gv_vtx_iter);
		  }
	  }
	  if( vertices ) PK_MEMORY_free(vertices);

	  nGE = 0;
	  PK_BODY_ask_edges(classifiedOn,&nGE,&edges);
	  for(i=0; i<nGE; i++){
		  vlist = M_getClassifiedVertices(pME,edges[i],&nbv);
		  if( vlist ) {
			  pVertexIter ge_vtx_iter = VIter(vlist);
			  while( vertex = VIter_next(ge_vtx_iter) ){
				  count++;
				  V_getCoord(vertex,xyz);
				  fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
						   count, xyz[0], xyz[1], xyz[2] ) ;
				  V_setTempID(vertex,count);
			  }
			  VIter_delete(ge_vtx_iter);
		  }
	  }

	  nGF = 0;
	  PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
	  for(i=0; i<nGF; i++){
		  vlist = M_getClassifiedVertices(pME,faces[i],&nbv);
		  if( vlist ) {
			  pVertexIter gf_vtx_iter = VIter(vlist);
			  while( vertex = VIter_next(gf_vtx_iter) ){
				count++;
				V_getCoord(vertex,xyz);
				fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
					count, xyz[0], xyz[1], xyz[2] ) ;
				V_setTempID(vertex,count);
			  }
			  VIter_delete(gf_vtx_iter);
		  }
	  }

	  vlist = M_getClassifiedVertices(pME,NULL,&nbv);  // all interior vertices on NULL model
	  if( vlist ) {
		  pVertexIter vtx_iter = VIter(vlist);
		  while( vertex = VIter_next(vtx_iter) ){
			  count++;
			  V_getCoord(vertex,xyz);
			  fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
					count, xyz[0], xyz[1], xyz[2] ) ;
			  V_setTempID(vertex,count);
		  }
		  VIter_delete(vtx_iter);
	  }
  }

  // write elements
  EDList<Face> * flist;
  int eid = 1;

  fprintf( fp, "PSHELL  %-8d%-8d%-8f\n", 
               nPropertyId, nMaterialId, 0.01 ) ;
  fprintf( fp, "PSOLID  %-8d%-8d\n", 
	           nPropertyId2, nMaterialId ) ;

  for(iter=m_meshArray.begin(); iter!=m_meshArray.end(); iter++) {
	  int nbf = 0;
	  pMWMesh pME = *iter;
	  pGEntity classifiedOn = M_model(pME);

	  int nr = M_numberRegions(pME);
	  if(nr < 1) {

		  nGF = 0;
		  PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
		  for(k=0; k<nGF; k++){
			  flist = M_getClassifiedFaces(pME,faces[k],&nbf);
			  if( flist ) {
		  	
				  // get surface orientation
				  bool orient = true;
				  if(faces[k] && !M_faceOrient(pME)) {
					  PK_SURF_t surf;
					  PK_LOGICAL_t o;
					  PK_FACE_ask_oriented_surf(faces[k],&surf,&o);
					  orient = o ? true : false;
				  }
				  
				  pFaceIter gf_face_iter=FIter(flist);
				  while( face = FIter_next(gf_face_iter) ){
					  count = F_numVertices(face);
					  if(count == 3) {
						  fprintf( fp, "%s%-8d%-8d", "CTRIA3  ", eid, nPropertyId ) ;
						  if(orient) {
							  for( i=0, j=4; i<3; i++, j++ ) {
								  if( j%10 == 0 ) {
									  fprintf( fp, "\n        ") ;
								  }
								  vertex = F_vertex(face,i);
								  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
							  }
						  } else {
							  for( i=2, j=4; i>=0; i--, j++ ) {
								  if( j%10 == 0 ) {
									  fprintf( fp, "\n        ") ;
								  }
								  vertex = F_vertex(face,i);
								  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
							  }
						  }
						  fprintf( fp, "\n" ) ;
						  eid++;

					  } else if(count == 4) {
						  fprintf( fp, "%s%-8d%-8d", "CQUAD4  ", eid, nPropertyId ) ;
						  if(orient) {
							  for( i=0, j=4; i<4; i++, j++ ) {
								  if( j%10 == 0 ) {
									  fprintf( fp, "\n        ") ;
								  }
								  vertex = F_vertex(face,i);
								  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
							  }
						  } else {
							  for( i=3, j=4; i>=0; i--, j++ ) {
								  if( j%10 == 0 ) {
									  fprintf( fp, "\n        ") ;
								  }
								  vertex = F_vertex(face,i);
								  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
							  }
						  }
						  fprintf( fp, "\n" ) ;
						  eid++;
					  }
				  }   //while loop
				  FIter_delete(gf_face_iter);
			  }    // end of if
		  }		// k loop

		  flist = M_getClassifiedFaces(pME,NULL,&nbf);	// all interior faces on NULL model
		  if( flist ) {
			  pFaceIter face_iter=FIter(flist);
			  while( face = FIter_next(face_iter) ){
				  count = F_numVertices(face);
				  if(count == 3) {
					  fprintf( fp, "%s%-8d%-8d", "CTRIA3  ", eid, nPropertyId ) ;
					  for( i=0, j=4; i<3; i++, j++ ) {
						  if( j%10 == 0 ) 
							  fprintf( fp, "\n        ") ;
						  vertex = F_vertex(face,i);
						  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
					  }

				  } else if(count == 4) {
					  fprintf( fp, "%s%-8d%-8d", "CQUAD4  ", eid, nPropertyId ) ;
					  for( i=0, j=4; i<4; i++, j++ ) {
						  if( j%10 == 0 ) 
							  fprintf( fp, "\n        ") ;
						  vertex = F_vertex(face,i);
						  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
					  }
				  }
				  fprintf( fp, "\n" ) ;
				  eid++;
			  }
			  FIter_delete(face_iter);
		  }

	  }	// end of if(nr < 1)

	  else {
		  EDList<Region> * rlist = M_getClassifiedRegions(pME,classifiedOn,&nbf);
		  if( rlist ) {
			  pRegion rgn;
			  pRegionIter region_iter=RIter(rlist);
			  while( rgn = RIter_next(region_iter) ){
				  fprintf( fp, "%s%-8d%-8d", "CTETRA  ", eid, nPropertyId2 ) ;
				  for( i=0, j=4; i<4; i++, j++ ) {
					  if( j%10 == 0 ) {
						  fprintf( fp, "\n        ") ;
					  }
					  vertex = R_vertex(rgn,i);
					  fprintf( fp, "%-8d", V_getTempID(vertex) ) ;
				  }
				  fprintf( fp, "\n" ) ;
				  eid++;
			  }
			  RIter_delete(region_iter);
		  }
	  }

  }

  fprintf(fp,"ENDDATA\n");
  fclose(fp) ;

  if(edges) PK_MEMORY_free(edges);
  if(faces) PK_MEMORY_free(faces);
  return;
}

void AssemblyMesh::countEntities(int *_nv, int *_nf, int *_nq, int *_nr)
{
	pMWMesh me;
	int i,k,nv=0, nf=0, nr=0, nq=0;
	for(i=0; i<this->numInstances(); i++) {
		Instance *inst = this->ithInstance(i);
		k = inst->body_index();
		me = this->ithInstancedMesh(k);
		nv += M_numberVertices(me);
		nf += M_numberFaces(me);
		nr += M_numberRegions(me);
		nq += M_numberQuadras(me);
	}
	*_nv = nv;
	*_nf = nf; 
	*_nq = nq;
	*_nr = nr;
	return;
}

int AssemblyMesh::dataFromXYZ(double picked[3], double pc[3], double proj[3], double *val)
{
	if(attr_type() != VERTEX_VECTOR_SCALAR)
		return 0;
}

//////////////////////////////////////////////////////////////////////
//  all functions below modify instances or meshes 

void AssemblyMesh::postMult(osg::Matrix &trsf)
{
	std::vector<Instance>::iterator iter;
	for(iter=m_instances.begin(); iter!=m_instances.end(); iter++) {
		iter->postMult(trsf);
	}
	return;
}

// becasue of index array, the traversal order must be consistent with 
// CSGFactory::createInteriorPoints2
int AssemblyMesh::deleteVertices(int np, double *coord, int *index)
{
	pMWMesh pMe;
	pVertexList vlist;
	pVertex vertex;
	double xyz[3], co[3], vec[3], dot;
	int i, num, nv=0, count=0;

	// count total number of vertices
	// we only deat with one instance and one body mesh per assembly mesh
	if(numInstances() > 1)
		return 0;
	Instance * inst = ithInstance(0);
	pMe = ithInstancedMesh( inst->body_index() );
	nv = M_numberVertices(pMe);
	if(nv < 1) return 0;
	vlist = M_getClassifiedVertices(pMe,NULL,&num);
	if( !vlist || num < 1)
		return 0;

	// copy into an array
	pVertex *pV = new pVertex[nv];
	pVertexIter vIt = VIter(vlist);
	while(vertex = VIter_next(vIt)) {
		if(V_getClassificationType(vertex) != 3)
			continue;
		pV[count++] = vertex;
	}
	VIter_delete(vIt);

	// find vertices to delete 
	// so ot's ok if you have duplicated selections
	osg::Vec3d pt;
	osg::Matrix * transf = inst->transform();
	for(i=0; i<np; i++) {
		vertex = pV[ index[i] ];
		if(V_getTempID(vertex) == DELETE_VERTEX_TEMP_ID)
			continue;
		co[0] = coord[3*i];
		co[1] = coord[3*i+1];
		co[2] = coord[3*i+2];
		V_getCoord(vertex,xyz);
		pt.set(xyz[0],xyz[1],xyz[2]);
		pt = transf->preMult(pt);
		xyz[0] = pt.x();
		xyz[1] = pt.y();
		xyz[2] = pt.z();
		diffVt(co,xyz,vec);
		dot = dotProd(vec,vec);
		if(dot < MW_TOL) 
			V_setTempID(vertex,DELETE_VERTEX_TEMP_ID);
	}

	for(i=0; i<np; i++) {
		if( index[i] )
			continue;
		co[0] = coord[3*i];
		co[1] = coord[3*i+1];
		co[2] = coord[3*i+2];
		for(int j=0; j<nv; j++) {
			vertex = pV[j];
			if(V_getTempID(vertex) == DELETE_VERTEX_TEMP_ID)
				continue;
			V_getCoord(vertex,xyz);
			pt.set(xyz[0],xyz[1],xyz[2]);
			pt = transf->preMult(pt);
			xyz[0] = pt.x();
			xyz[1] = pt.y();
			xyz[2] = pt.z();
			diffVt(co,xyz,vec);
			dot = dotProd(vec,vec);
			if(dot < MW_TOL) {
				V_setTempID(vertex,DELETE_VERTEX_TEMP_ID);
				break;
			}
		}
	}

	// perform the deletion
	num = 0;
	for(i=0; i<nv; i++) {
		vertex = pV[i];
		if(V_getTempID(vertex) == DELETE_VERTEX_TEMP_ID) {
			M_removeVertex(pMe,vertex);    
			num++;
		}
	}

	delete [] pV;

	return num;
}