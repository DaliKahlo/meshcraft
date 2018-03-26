//$c6   XRL 02/25/2014 add remove(pVertex)
//$c5   XRL 12/13/2013 Account for faceOrient() in toArray().
//$c4   XRL 10/15/2012 Support NQuad.
//$c3   XRL 07/20/2012 Support mesh regions.
//$c2   XRL 07/14/2012 Support unclassified mesh ie. classifiedOn=NULL.
//$c1   XRL 10/07/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
#include "PSMesh.h"
#include "mesh_api.h"
#include "Vec3d.h"
#include "Matrix.h"
#include <stdio.h>
#include <string.h>

bool XYZ_coincident(double *a, double *b)
{
	double dx, dy, dz;
	dx = a[0] - b[0];
	dy = a[1] - b[1];
	dz = a[2] - b[2];
	if(dx*dx + dy*dy + dz*dz < 1.e-16)
		return true;
	return false;
}

//int PSMesh::mesh_count = 0;

PSMesh::PSMesh(pGEntity g)
{ 
  classifiedOn = g;
  init();
  //mesh_count++;
}

PSMesh::~PSMesh()
{
  if( classifiedOn ) 
  {
    PK_FACE_t *faces;
    PK_EDGE_t *edges;
    PK_VERTEX_t *vertices;
    int num, i;

    // delete all lists hooked on topologies
    PK_BODY_ask_faces(classifiedOn,&num,&faces);
    for(i=0; i<num; i++)
	  removeClassifiedMesh(Tgface,faces[i]);
    PK_MEMORY_free(faces);

    PK_BODY_ask_edges(classifiedOn,&num,&edges);
    for(i=0; i<num; i++)
	  removeClassifiedMesh(Tgedge,edges[i]);
    PK_MEMORY_free(edges);

    PK_BODY_ask_vertices(classifiedOn,&num,&vertices);
    for(i=0; i<num; i++)
	  removeClassifiedMesh(Tgvertex,vertices[i]);
    PK_MEMORY_free(vertices);
  }
 
  // now a body can only have one region 
  // so it can always be saved as class member no matter classifiedOn NULL or not
  if( m_pMRList ) 
	{ delete m_pMRList; m_pMRList = NULL; }
  if( m_pMFList ) 
	{ delete m_pMFList; m_pMFList = NULL; }
  if( m_pMEList ) 
	{ delete m_pMEList; m_pMEList = NULL; }
  if( m_pMVList ) 
	{ delete m_pMVList; m_pMVList = NULL; }
  if( m_pCurveList ) 
	{ delete m_pCurveList; m_pCurveList = NULL; }
  if( m_pSurfaceList ) 
	{ delete m_pSurfaceList; m_pSurfaceList = NULL; }
  if( m_pBrepList ) 
	{ delete m_pBrepList; m_pBrepList = NULL; }

  NRegion = NFace = NQuad = NEdge = NVertex = 0;
  NCurve = NSurface = NBrep = 0;
  m_bSkinned = false;
}

bool PSMesh::init()
{ 
  m_bSkinned = false;		// without skinned for mesh regions 
  m_bFaceOrient = false;    // normal in the orientation of surfaces

  NRegion = NFace = NQuad = NEdge = NVertex = NCurve = NSurface = NBrep = 0;

  m_pMRList = 0;
  m_pMFList = 0;
  m_pMEList = 0;
  m_pMVList = 0;
  m_pCurveList = 0;
  m_pSurfaceList = 0;
  m_pBrepList = 0;

  //if(mesh_count == 0) {
  // define attribute to attach classified mesh vertex/edge/face
  char pCharString0[]="MSC_MVertexAttr";
  PK_ERROR_code_t ifail = PK_ATTDEF_find(pCharString0, &classifiedMVertexAttr);
  if (ifail != PK_ERROR_ok)
	  return false; 
  if (classifiedMVertexAttr == PK_ENTITY_null) {
	  PK_ATTDEF_sf_t pkAttdef;
	  pkAttdef.name = pCharString0;
	  pkAttdef.attdef_class = PK_ATTDEF_class_01_c;
	  pkAttdef.n_fields = 1;
	  PK_ATTRIB_field_t pabc[1] = { PK_ATTRIB_field_pointer_c };
	  pkAttdef.field_types = pabc;
	  pkAttdef.n_owner_types = 3;
	  PK_CLASS_t pcba[3] = { PK_CLASS_face, PK_CLASS_edge, PK_CLASS_vertex };
	  pkAttdef.owner_types = pcba;
	  ifail = PK_ATTDEF_create(&pkAttdef, &classifiedMVertexAttr);
	  if (ifail != PK_ERROR_ok)
		  return false; 
  }

  char pCharString1[]="MSC_MEdgeAttr";
  ifail = PK_ATTDEF_find(pCharString1, &classifiedMEdgeAttr);
  if (ifail != PK_ERROR_ok)
	  return false; 
  if (classifiedMEdgeAttr == PK_ENTITY_null) {
	  PK_ATTDEF_sf_t pkAttdef;
	  pkAttdef.name = pCharString1;
	  pkAttdef.attdef_class = PK_ATTDEF_class_01_c;
	  pkAttdef.n_fields = 1;
	  PK_ATTRIB_field_t pabc[1] = { PK_ATTRIB_field_pointer_c };
	  pkAttdef.field_types = pabc;
	  pkAttdef.n_owner_types = 1;
	  PK_CLASS_t pcba[1] = { PK_CLASS_edge };
	  pkAttdef.owner_types = pcba;
	  ifail = PK_ATTDEF_create(&pkAttdef, &classifiedMEdgeAttr);
	  if (ifail != PK_ERROR_ok)
		  return false; 
  }

  char pCharString2[]="MSC_MFaceAttr";
  ifail = PK_ATTDEF_find(pCharString2, &classifiedMFaceAttr);
  if (ifail != PK_ERROR_ok)
	  return false; 
  if (classifiedMFaceAttr == PK_ENTITY_null) {
	  PK_ATTDEF_sf_t pkAttdef;
	  pkAttdef.name = pCharString2;
	  pkAttdef.attdef_class = PK_ATTDEF_class_01_c;
	  pkAttdef.n_fields = 1;
	  PK_ATTRIB_field_t pabc[1] = { PK_ATTRIB_field_pointer_c };
	  pkAttdef.field_types = pabc;
	  pkAttdef.n_owner_types = 1;
	  PK_CLASS_t pcba[1] = { PK_CLASS_face };
	  pkAttdef.owner_types = pcba;
	  ifail = PK_ATTDEF_create(&pkAttdef, &classifiedMFaceAttr);
	  if (ifail != PK_ERROR_ok)
		  return false; 
  }
  //}
  return true;
}

pGEntity PSMesh::model()
{ return classifiedOn; }


void PSMesh::add(pGEntity tag, pRegion r)
{
	if(m_pMRList==NULL)
		m_pMRList = new EDList<Region>;
	m_pMRList->append(r);
	NRegion++;
}

void PSMesh::add(pGEntity tag, pFace f)
{ 
  if( tag ) 
  { // classified
	int numAttr;
	PK_ATTRIB_t *pAttr;
	
	PK_ERROR_code_t ifail = PK_ENTITY_ask_attribs(tag, classifiedMFaceAttr, &numAttr, &pAttr);
	if( ifail==PK_ERROR_ok ) {
		EDList<Face> *facelist;
		if (numAttr==0)  {
			PK_ATTRIB_t pkattr;	
			PK_ATTRIB_create_empty(tag, classifiedMFaceAttr, &pkattr);
			facelist = new EDList<Face>;
			PK_ATTRIB_set_pointers(pkattr, 0, 1, (PK_POINTER_t*)&facelist);
		}
		else {
			int n=0;
			PK_POINTER_t *p=NULL;
			PK_ATTRIB_ask_pointers(pAttr[0], 0, &n, &p);
			facelist = (EDList<Face> *)p[0];
			PK_MEMORY_free(p);
			PK_MEMORY_free(pAttr);
		}
		facelist->append(f);
	}
  }
  else
  {  // unclassified
	if(m_pMFList==NULL)
		m_pMFList = new EDList<Face>;
	m_pMFList->append(f);
  }

  NFace++;
  if(f->numVertices() == 4)
	  NQuad ++;
}

void PSMesh::add(pGEntity tag, pEdge e)
{
  if(tag)
  {
	int numAttr;
	PK_ATTRIB_t *pAttr;
	
	PK_ERROR_code_t ifail = PK_ENTITY_ask_attribs(tag, classifiedMEdgeAttr, &numAttr, &pAttr);
	if( ifail==PK_ERROR_ok ) {
		EDList<Edge> *edgelist;
		if (numAttr==0)  {
			PK_ATTRIB_t pkattr;	
			PK_ATTRIB_create_empty(tag, classifiedMEdgeAttr, &pkattr);
			edgelist = new EDList<Edge>;
			PK_ATTRIB_set_pointers(pkattr, 0, 1, (PK_POINTER_t*)&edgelist);
		}
		else {
			int n=0;
			PK_POINTER_t *p=NULL;
			PK_ATTRIB_ask_pointers(pAttr[0], 0, &n, &p);
			edgelist = (EDList<Edge> *)p[0];
			PK_MEMORY_free(p);
			PK_MEMORY_free(pAttr);
		}
		edgelist->append(e);
		NEdge++;
	}
  }
  else
  { // unclassified
	if(m_pMEList==NULL)
		m_pMEList = new EDList<Edge>;
	m_pMEList->append(e);
	NEdge++;
  }
}

void PSMesh::add(pGEntity tag, pVertex v)
{
  if(tag)
  {
	int numAttr;
	PK_ATTRIB_t *pAttr;
	
	PK_ERROR_code_t ifail = PK_ENTITY_ask_attribs(tag, classifiedMVertexAttr, &numAttr, &pAttr);
	if( ifail==PK_ERROR_ok ) {
		EDList<Vertex> *vertexlist;
		if (numAttr==0)  {
			PK_ATTRIB_t pkattr;	
			PK_ATTRIB_create_empty(tag, classifiedMVertexAttr, &pkattr);
			vertexlist = new EDList<Vertex>;
			PK_ATTRIB_set_pointers(pkattr, 0, 1, (PK_POINTER_t*)&vertexlist);
		}
		else {
			int n=0;
			PK_POINTER_t *p=NULL;
			PK_ATTRIB_ask_pointers(pAttr[0], 0, &n, &p);
			vertexlist = (EDList<Vertex> *)p[0];
			PK_MEMORY_free(p);
			PK_MEMORY_free(pAttr);
		}
		vertexlist->append(v);
		NVertex++;
	}
  }
  else
  { // unclassified or classified onto the body
	if(m_pMVList==NULL)
		m_pMVList = new EDList<Vertex>;
	m_pMVList->append(v);
	NVertex++;
  }
}

void PSMesh::add(pCurve c)
{
	if(m_pCurveList==NULL)
		m_pCurveList = new EDList<Curve>;
	m_pCurveList->append(c);
	NCurve++;
}

void PSMesh::add(pSurface surf)
{
	if(m_pSurfaceList==NULL)
		m_pSurfaceList = new EDList<Surface>;
	m_pSurfaceList->append(surf);
	NSurface++;
}

void PSMesh::add(Brep *b)
{
	if(m_pBrepList==NULL)
		m_pBrepList = new EDList<Brep>;
	m_pBrepList->append(b);
	NBrep++;
}

int PSMesh::numRegions() const
{ return NRegion; }

int PSMesh::numFaces() const
{ return NFace; }

int PSMesh::numQuadras() const
{ return NQuad; }

int PSMesh::numEdges() const
{ return NEdge; }

int PSMesh::numVertices() const
{ return NVertex; }
 
int PSMesh::numCurves() const
{ return NCurve; }

pRegion PSMesh::createRegion(pVertex *verts, pGEntity gent)
{
  pRegion r = new Region(verts,gent);
  add(gent,r);
  return r;
}

pFace PSMesh::createFace(int nV, pVertex *verts, pGEntity gent)
{
  pFace f = new Face(nV,verts,gent);
  add(gent,f);
  return f;
}

pEdge PSMesh::createEdge(pVertex v1, pVertex v2, pGEntity gent)
{
  pEdge e = new Edge(v1,v2,gent);
  add(gent,e);
  return e;
}

pVertex PSMesh::createVertex(double c[3], double param[2], pGEntity gent)
{
  pVertex v = new Vertex(c, gent);
  add(gent,v);
  v->set_param(param);
  return v;
}

pVertex PSMesh::createVertex(double c[3], pGEntity gent)
{
  pVertex v = new Vertex(c, gent);
  add(gent,v);
  return v;
}

pCurve PSMesh::createCurve(geomType type, char *name, int np, double *pts)
{
  pCurve crv = new Curve(type, name, np, pts);
  add(crv);
  return crv;
}

pSurface PSMesh::createSurface(int nc, pCurve *curves)
{
  Surface *surf = new Surface(nc,curves);
  add(surf);
  return surf;
}

pBrep PSMesh::createBrep(int nb, pSurface *surfs)
{
  Brep *body = new Brep(nb, surfs);
  add(body);
  return body;
}

void PSMesh::remove(pFace f)
{
  // f->detach(); // not as we don't keep upward adjacency
  EDList<Face> *flist = getClassifiedFaces(f->whatIn());
  flist->remove(f);

  delete f;
  NFace--;
}

void PSMesh::remove(pVertex v)
{
  // all upward adjacency must have been detached 
  EDList<Vertex> *vlist = getClassifiedVertices(v->whatIn());
  vlist->remove(v);

  delete v;
  NVertex--;
}

EDList<Curve> * PSMesh::getCurves()
{
	return m_pCurveList;
}

EDList<Surface> * PSMesh::getSurfaces()
{
	return m_pSurfaceList;
}

EDList<Region> * PSMesh::getClassifiedRegions(pGEntity)
{
	return m_pMRList;
}

EDList<Face> * PSMesh::getClassifiedFaces(pGEntity tag)
{
  if(tag)
  {
	  int numAttr;
	  PK_ATTRIB_t *pAttr;
	
	  PK_ERROR_code_t ifail = PK_ENTITY_ask_attribs(tag, classifiedMFaceAttr, &numAttr, &pAttr);
	  if( ifail!=PK_ERROR_ok || numAttr==0 ) 
		  return NULL;
	
	  int n=0;
	  PK_POINTER_t *p=NULL;
	  PK_ATTRIB_ask_pointers(pAttr[0], 0, &n, &p);
	  EDList<Face> *facelist = (EDList<Face> *)p[0];
	  PK_MEMORY_free(p);
	  PK_MEMORY_free(pAttr);
	  return facelist;
  }

  return m_pMFList;
}
  
EDList<Edge> * PSMesh::getClassifiedEdges(pGEntity tag)
{
  if(tag) 
  {
	  int numAttr;
	  PK_ATTRIB_t *pAttr;
	
	  PK_ERROR_code_t ifail = PK_ENTITY_ask_attribs(tag, classifiedMEdgeAttr, &numAttr, &pAttr);
	  if( ifail!=PK_ERROR_ok || numAttr==0 ) 
		  return NULL;
	
	  int n=0;
	  PK_POINTER_t *p=NULL;
	  PK_ATTRIB_ask_pointers(pAttr[0], 0, &n, &p);
	  EDList<Edge> *edgelist = (EDList<Edge> *)p[0];
	  PK_MEMORY_free(p);
	  PK_MEMORY_free(pAttr);
	  return edgelist;
  }
  return m_pMEList;
}
  
EDList<Vertex> * PSMesh::getClassifiedVertices(pGEntity tag)
{
  if(tag)
  {
	  int numAttr;
	  PK_ATTRIB_t *pAttr;
	
	  PK_ERROR_code_t ifail = PK_ENTITY_ask_attribs(tag, classifiedMVertexAttr, &numAttr, &pAttr);
	  if( ifail!=PK_ERROR_ok || numAttr==0 ) 
		  return NULL;
	
	  int n=0;
	  PK_POINTER_t *p=NULL;
	  PK_ATTRIB_ask_pointers(pAttr[0], 0, &n, &p);
	  EDList<Vertex> *vertexlist = (EDList<Vertex> *)p[0];
	  PK_MEMORY_free(p);
	  PK_MEMORY_free(pAttr);
	  return vertexlist;
  }
  return m_pMVList;
}

void PSMesh::removeClassifiedMesh(gType pclass, pGEntity tag)
{
	int n_deleted;
	switch( pclass ) {
		case Tgface:		
			{ 
				EDList<Face> *flist = getClassifiedFaces(tag);
				PK_ENTITY_delete_attribs(tag,classifiedMFaceAttr,&n_deleted);
				if(flist) 
				{NFace -= flist->size(); delete flist;}
				break; 
			}
		case Tgedge:		
			{ 
				EDList<Edge> *elist = getClassifiedEdges(tag);
				PK_ENTITY_delete_attribs(tag,classifiedMEdgeAttr,&n_deleted);
				if(elist) 
				{NEdge -= elist->size(); delete elist;}
				break; 
			}
	}

	EDList<Vertex> *vlist = getClassifiedVertices(tag);
	PK_ENTITY_delete_attribs(tag,classifiedMVertexAttr,&n_deleted);
	if(vlist) 
	{NVertex -= vlist->size(); delete vlist;}

	return;
}

void PSMesh::countClassifiedFaces(pGEntity gf,int *nt, int *nq)
{
	int tri=0, quad=0, nv;
	EDList<Face> * flist = getClassifiedFaces(gf);
	if( flist ) {
		pFace face;
		EDListIter<Face> gf_face_iter(flist);
		while( face = gf_face_iter.next() ){
			nv = face->numVertices();
			if(nv == 3)
				tri++;
			else if(nv == 4)
				quad++;
		}
	}
	*nt = tri;  
	*nq = quad;
	return;
}

void PSMesh::writePointRecord(ostream &outfile, pVertex v)
{
  double xyz[3];
  v->get_coordinates(xyz);
  outfile << xyz[0] << " " << xyz[1] << " " << xyz[2];

  pGEntity gent = v->whatIn();
  if(gent > 0) {
	  int id;
	  PK_ENTITY_ask_identifier(gent,&id);
	  outfile << " " << id;
  }
  outfile << endl;
}

void PSMesh::toMEDIT(const char *fileName)
{
  if(NVertex <= 0) return;
  if(NFace <= 0) return;

  PK_FACE_t *faces;
  PK_EDGE_t *edges;
  PK_VERTEX_t *vertices;
  int nGV, nGE, nGF;
  int i, j, count;

  ofstream ofs(fileName);

  Vertex *vertex;
  Edge *edge;
  Face *face;
  pGEntity gent;

  ofs << "MeshVersionFormatted 1 \n\n";
  ofs << "Dimension\n";
  ofs << "3\n";
  ofs << "# Generated from MeshWorks   version 04/30/2004 \n";
  ofs << "#                            updated on 10/05/2005 \n";
  ofs << "#                            updated on 10/13/2011 \n\n";


  // Write Vertices
  ofs << "Vertices " << NVertex << endl;

  EDList<Vertex> * vlist;
  count = 0;
  PK_BODY_ask_vertices(classifiedOn,&nGV,&vertices);
  for(i=0; i<nGV; i++){
	  vlist = getClassifiedVertices(vertices[i]);
	  if( vlist ) {
		  vertex = vlist->nth(0);
		  count++;
		  writePointRecord(ofs, vertex);
		  vertex->set_id(count);
	  }
  }
  if( vertices ) PK_MEMORY_free(vertices);

  PK_BODY_ask_edges(classifiedOn,&nGE,&edges);
  for(i=0; i<nGE; i++){
	  vlist = getClassifiedVertices(edges[i]);
	  if( vlist ) {
		  EDListIter<Vertex> ge_vtx_iter(vlist);
		  while( vertex = ge_vtx_iter.next() ){
			count++;
			writePointRecord(ofs, vertex);
			vertex->set_id(count);
		  }
	  }
  }

  PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
  for(i=0; i<nGF; i++){
	  vlist = getClassifiedVertices(faces[i]);
	  if( vlist ) {
		  EDListIter<Vertex> gf_vtx_iter(vlist);
		  while( vertex = gf_vtx_iter.next() ){
			count++;
			writePointRecord(ofs, vertex);
			vertex->set_id(count);
		  }
	  }
  }


  // write edges classified on model edges
  // note you may choose not to represent boundary edges
  if(NEdge > 0) {
	  EDList<Edge> * elist;
	  ofs << "\nEdges " << NEdge << endl;
	  for(i=0; i<nGE; i++){
		  elist = getClassifiedEdges(edges[i]);
		  if( elist ) {
			  EDListIter<Edge> ge_edge_iter(elist);
			  while( edge = ge_edge_iter.next() ){
				  gent=edge->whatIn();
				  if(gent == 0)
					  continue;
				  count++;
				  for( j=0; j<2; j++ ) {
					  vertex=edge->get_vertex(j);
					  ofs << vertex->get_id() << " ";
				  }
				  PK_ENTITY_ask_identifier(gent,&j);
				  ofs << j << endl;
		      }			  
		  }
	  }
  }

  // count face elements
  int numTri=0, numQuad=0, nt, nq;
  for(i=0; i<nGF; i++){
	countClassifiedFaces(faces[i],&nt,&nq);
	numQuad += nq;
	numTri += nt;
  }

  // write triangles
  EDList<Face> * flist;
  if(numTri > 0) {
	  ofs << "\nTriangles "<< numTri << endl;
	  for(i=0; i<nGF; i++){
		  flist = getClassifiedFaces(faces[i]);
		  if( flist ) {
			  EDListIter<Face> gf_face_iter(flist);
			  while( face = gf_face_iter.next() ){
				  if(face->numVertices() != 3)
					  continue;
				  for(j=0; j<3; j++) {
					  vertex = face->get_vertex(j);
					  ofs << vertex->get_id() << " ";
				  }
				  PK_ENTITY_ask_identifier(face->whatIn(),&j);
				  ofs << j << endl;
			  }
		  }
	  }
  }

  // write quadrangles
  if(numQuad > 0) {
	  ofs << "\nQuadrilaterals "<< numQuad << endl;
	  for(i=0; i<nGF; i++){
		  flist = getClassifiedFaces(faces[i]);
		  if( flist ) {
			  EDListIter<Face> gf_face_iter(flist);
			  while( face = gf_face_iter.next() ){
				  if(face->numVertices() != 4) 
					  continue;
				  for(j=0; j<4; j++) {
					  vertex = face->get_vertex(j);
					  ofs << vertex->get_id() << " ";
				  }
				  PK_ENTITY_ask_identifier(face->whatIn(),&j);
				  ofs << j << endl;
			  }
		  }
	  }
  }

 // // Write corners (model vertices)
 // if ( gNV > 0 && toggle > 0 ) {
	//cout << "Number of corners \t" << gNV << "\n";
 //   ofs << "\nCorners " << gNV << endl;
	//viter.reset();
 //   while ( gNV ){
 //     viter(vertex);
	//  if( vertex->whatInType() == 0 ) {  //GVertex
	//	gNV--;
	//	vertex->getDataInt(nodalId, &numNode);
	//    ofs << numNode << endl; 
	//  }
 //   }
 // }

 // // Write ridges (model edges)
 // if( geNE > 0 && toggle>1 ) {
	//ofs << "\n\nRidges " << geNE << endl;
	//eiter.reset();
	//i = 0;
	//while( i<geNE ) {
	//  eiter(edge);
	//  if( edge->whatInType() == 1 ) {
	//	i++;
	//	ofs << i << endl;
	//  }  
	//} // end of while
 // }

  ofs << "\nEnd\n";
  ofs.close();

  if(edges) PK_MEMORY_free(edges);
  if(faces) PK_MEMORY_free(faces);
}

void PSMesh::toBDF(const char *fileName)
{
  if(NVertex <= 0) return;
  if(NFace <= 0) return;

  PK_FACE_t *faces;
  PK_EDGE_t *edges;
  PK_VERTEX_t *vertices;
  int nGV, nGE, nGF;
  int i, j, k, count;
  double xyz[3];
  //char sElemName[16] ;

  Vertex *vertex;
  //Edge *edge;
  Face *face;
  //pGEntity gent;

  // Initialize the global ids
  int nMaterialId = 1 ;
  int nPropertyId = 1 ;

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
  EDList<Vertex> * vlist;
  count = 0;
  PK_BODY_ask_vertices(classifiedOn,&nGV,&vertices);
  for(i=0; i<nGV; i++){
	  vlist = getClassifiedVertices(vertices[i]);
	  if( vlist ) {
		  vertex = vlist->nth(0);
		  count++;
		  vertex->get_coordinates(xyz);
		  fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
		       count, xyz[0], xyz[1], xyz[2] ) ;
		  vertex->set_id(count);
	  }
  }
  if( vertices ) PK_MEMORY_free(vertices);

  PK_BODY_ask_edges(classifiedOn,&nGE,&edges);
  for(i=0; i<nGE; i++){
	  vlist = getClassifiedVertices(edges[i]);
	  if( vlist ) {
		  EDListIter<Vertex> ge_vtx_iter(vlist);
		  while( vertex = ge_vtx_iter.next() ){
			count++;
			vertex->get_coordinates(xyz);
			fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
				count, xyz[0], xyz[1], xyz[2] ) ;
			vertex->set_id(count);
		  }
	  }
  }

  PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
  for(i=0; i<nGF; i++){
	  vlist = getClassifiedVertices(faces[i]);
	  if( vlist ) {
		  EDListIter<Vertex> gf_vtx_iter(vlist);
		  while( vertex = gf_vtx_iter.next() ){
			count++;
			vertex->get_coordinates(xyz);
			fprintf( fp, "GRID*   %-32d%-16e%-16e*\n*       %-16e\n",
				count, xyz[0], xyz[1], xyz[2] ) ;
			vertex->set_id(count);
		  }
	  }
  }


  // write elements
  EDList<Face> * flist;
  int eid = 1;

  fprintf( fp, "PSHELL  %-8d%-8d%-8f\n", 
               nPropertyId, nMaterialId, 0.01 ) ;

  for(k=0; k<nGF; k++){
	  flist = getClassifiedFaces(faces[k]);
	  if( flist ) {
		  	
		  // get surface orientation
		  bool orient = true;
		  if(faces[k]) {
			  PK_SURF_t surf;
			  PK_LOGICAL_t o;
			  PK_FACE_ask_oriented_surf(faces[k],&surf,&o);
			  orient = o ? true : false;
		  }

		  EDListIter<Face> gf_face_iter(flist);
		  while( face = gf_face_iter.next() ){
			  count = face->numVertices();
			  if(count == 3) {
				  fprintf( fp, "%s%-8d%-8d", "CTRIA3  ", eid, nPropertyId ) ;
				  if(orient) {
					  for( i=0, j=4; i<3; i++, j++ ) {
						  if( j%10 == 0 ) {
							  fprintf( fp, "\n        ") ;
						  }
						  vertex = face->get_vertex(i);
						  fprintf( fp, "%-8d", vertex->get_id() ) ;
					  }
				  } else {
					  for( i=2, j=4; i>=0; i--, j++ ) {
						  if( j%10 == 0 ) {
							  fprintf( fp, "\n        ") ;
						  }
						  vertex = face->get_vertex(i);
						  fprintf( fp, "%-8d", vertex->get_id() ) ;
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
						  vertex = face->get_vertex(i);
						  fprintf( fp, "%-8d", vertex->get_id() ) ;
					  }
				  } else {
					  for( i=3, j=4; i>=0; i--, j++ ) {
						  if( j%10 == 0 ) {
							  fprintf( fp, "\n        ") ;
						  }
						  vertex = face->get_vertex(i);
						  fprintf( fp, "%-8d", vertex->get_id() ) ;
					  }
				  }
				  fprintf( fp, "\n" ) ;
				  eid++;
			  }
		  }   //while loop
	  }    // end of if
  }		// k loop

  fprintf(fp,"ENDDATA\n");
  fclose(fp) ;

  if(edges) PK_MEMORY_free(edges);
  if(faces) PK_MEMORY_free(faces);
  return;
}

pFace PSMesh::ithFace(int ith)
{
	EDList<Face> * flist;
	pFace face;
	int i, nGF, fcount = 1;

	if(ith <= 0 )  return NULL;			// from 1
    if(classifiedOn) {
		PK_FACE_t *faces;
	    PK_BODY_ask_faces(classifiedOn,&nGF,&faces);

	    for(i=0; i<nGF; i++){
			flist = getClassifiedFaces(faces[i]);
			if( flist ) {	
				EDListIter<Face> gf_face_iter(flist);
				while( face = gf_face_iter.next() ){
					if(face->numVertices() != 3)
						return NULL;
					if(fcount == ith) 
						return face;
					fcount++;
				}
			}
	    }
    }

	flist = getClassifiedFaces(NULL);
	if( flist ) {
		EDListIter<Face> gf_face_iter(flist);
		while( face = gf_face_iter.next() ){
			if(face->numVertices() != 3)
				return NULL;
			if(fcount == ith) 
				return face;
			fcount++;
		}
	}
	return NULL;
}

pVertex PSMesh::ithVertex(int ith)
{
	EDList<Vertex> * vlist;
	pVertex vertex;
	int count=1;

	if(ith <= 0 )  return NULL;	
	if(classifiedOn) {
		PK_FACE_t *faces;
		PK_EDGE_t *edges;
		PK_VERTEX_t *vertices;
		int i, nGV, nGE, nGF;

		PK_BODY_ask_vertices(classifiedOn,&nGV,&vertices);
		for(i=0; i<nGV; i++){
			vlist = getClassifiedVertices(vertices[i]);
			if( vlist ) {
				if(count == ith) 
					return vlist->nth(0);
				count++;
			}
		}
		if( vertices ) PK_MEMORY_free(vertices);

		PK_BODY_ask_edges(classifiedOn,&nGE,&edges);
		for(i=0; i<nGE; i++){
			vlist = getClassifiedVertices(edges[i]);
			if( vlist ) {
				EDListIter<Vertex> ge_vtx_iter(vlist);
				while( vertex = ge_vtx_iter.next() ){
					if(count == ith) 
						return vertex;
					count++;
				}
			}
		}
		if(edges) PK_MEMORY_free(edges);

		PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
		for(i=0; i<nGF; i++){
			vlist = getClassifiedVertices(faces[i]);
			if( vlist ) {
				EDListIter<Vertex> gf_vtx_iter(vlist);
				while( vertex = gf_vtx_iter.next() ){
					if(count == ith) 
						return vertex;
					count++;
				}
			}
		}
	}

	vlist = getClassifiedVertices(NULL);
	if( vlist ) {
		EDListIter<Vertex> ge_vtx_iter(vlist);
		while( vertex = ge_vtx_iter.next() ){
			if(count == ith) 
				return vertex;
			count++;
		}
	}
	return NULL;
}

int PSMesh::vertexIndexFromXYZ(double pt[3])
{
	EDList<Vertex> * vlist;
	pVertex vertex;
	double xyz[3];
	int count=1;

	if(classifiedOn) {
		PK_FACE_t *faces;
		PK_EDGE_t *edges;
		PK_VERTEX_t *vertices;
		int i, nGV, nGE, nGF;

		PK_BODY_ask_vertices(classifiedOn,&nGV,&vertices);
		for(i=0; i<nGV; i++){
			vlist = getClassifiedVertices(vertices[i]);
			if( vlist ) {
				vertex = vlist->nth(0);
				vertex->get_coordinates(xyz);
				if( XYZ_coincident(pt,xyz) )
					return count;
				count++;
			}
		}
		if( vertices ) PK_MEMORY_free(vertices);

		PK_BODY_ask_edges(classifiedOn,&nGE,&edges);
		for(i=0; i<nGE; i++){
			vlist = getClassifiedVertices(edges[i]);
			if( vlist ) {
				EDListIter<Vertex> ge_vtx_iter(vlist);
				while( vertex = ge_vtx_iter.next() ){
					vertex->get_coordinates(xyz);
					if( XYZ_coincident(pt,xyz) )
						return count;
					count++;
				}
			}
		}
		if(edges) PK_MEMORY_free(edges);

		PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
		for(i=0; i<nGF; i++){
			vlist = getClassifiedVertices(faces[i]);
			if( vlist ) {
				EDListIter<Vertex> gf_vtx_iter(vlist);
				while( vertex = gf_vtx_iter.next() ){
					vertex->get_coordinates(xyz);
					if( XYZ_coincident(pt,xyz) )
						return count;
					count++;
				}
			}
		}
	}

	vlist = getClassifiedVertices(NULL);
	if( vlist ) {
		EDListIter<Vertex> ge_vtx_iter(vlist);
		while( vertex = ge_vtx_iter.next() ){
			vertex->get_coordinates(xyz);
			if( XYZ_coincident(pt,xyz) )
				return count;
			count++;
		}
	}
	return 0;   // not found
}

bool PSMesh::toArrays(int *nf, int *nv, int **tria, pVertex **pverts)
{
	*nf = numFaces();
	*nv = numVertices();
	if(*nv < 1)
		return false;
	  
	pVertex *verts = new pVertex[NVertex];
	int *ptria = NULL;
	*pverts = verts;

	if(*nf > 0) {
		ptria = new int[3*NFace];
		*tria = ptria;
	} else {
		*tria = NULL;
	}

	int i, j, k, count, fcount;
	bool ret = true;

	EDList<Vertex> * vlist;
	EDList<Face> * flist;
	pVertex vertex;
	pFace face;
	count = 0;
	fcount = 0;
	if(classifiedOn) {
		PK_FACE_t *faces;
		PK_EDGE_t *edges;
		PK_VERTEX_t *vertices;
		int nGV, nGE, nGF;

		PK_BODY_ask_vertices(classifiedOn,&nGV,&vertices);
		for(i=0; i<nGV; i++){
			vlist = getClassifiedVertices(vertices[i]);
			if( vlist ) {
				verts[count] = vlist->nth(0);
				(verts[count])->set_temp_id(count+1);
				count++;
			}
		}
		if( vertices ) PK_MEMORY_free(vertices);

		PK_BODY_ask_edges(classifiedOn,&nGE,&edges);
		for(i=0; i<nGE; i++){
			vlist = getClassifiedVertices(edges[i]);
			if( vlist ) {
				EDListIter<Vertex> ge_vtx_iter(vlist);
				while( vertex = ge_vtx_iter.next() ){
					vertex->set_temp_id(count+1);
					verts[count] = vertex;
					count++;
				}
			}
		}
		if(edges) PK_MEMORY_free(edges);

		PK_BODY_ask_faces(classifiedOn,&nGF,&faces);
		for(i=0; i<nGF; i++){
			vlist = getClassifiedVertices(faces[i]);
			if( vlist ) {
				EDListIter<Vertex> gf_vtx_iter(vlist);
				while( vertex = gf_vtx_iter.next() ){
					vertex->set_temp_id(count+1);
					verts[count] = vertex;
					count++;
				}
			}
		}
				
		fcount = 0;
		for(i=0; i<nGF; i++){
			flist = getClassifiedFaces(faces[i]);
			if( flist ) {	
				// get surface orientation
				bool orient = true;
				if(faces[i] && (this->faceOrient())) {
					  PK_SURF_t surf;
					  PK_LOGICAL_t o;
					  PK_FACE_ask_oriented_surf(faces[i],&surf,&o);
					  orient = o ? true : false;
				}

				EDListIter<Face> gf_face_iter(flist);
				while( face = gf_face_iter.next() ){
					if(face->numVertices() != 3)
						ret = false;
					if(orient) {
						for( j=0; j<3; j++ ) {
							vertex = face->get_vertex(j);
							ptria[3*fcount+j] = vertex->get_temp_id();
						}
					} else {
						k = 0;
						for( j=2; j>=0; j-- ) {
							vertex = face->get_vertex(j);
							ptria[3*fcount+k] = vertex->get_temp_id();
							k++;
						}
					}
					fcount++;
				}
			}
		}
		if(faces) PK_MEMORY_free(faces);
	}

	vlist = getClassifiedVertices(NULL);
	if( vlist ) {
		EDListIter<Vertex> ge_vtx_iter(vlist);
		while( vertex = ge_vtx_iter.next() ){
			vertex->set_temp_id(count+1);
			verts[count] = vertex;
			count++;
		}
	}

	flist = getClassifiedFaces(NULL);
	if( flist ) {
		EDListIter<Face> gf_face_iter(flist);
		while( face = gf_face_iter.next() ){
			if(face->numVertices() != 3)
				ret = false;
			for( j=0; j<3; j++ ) {
				vertex = face->get_vertex(j);
				ptria[3*fcount+j] = vertex->get_temp_id();
			}
			fcount++;
		}
	}
	return ret;
}

void PSMesh::transform(double t[4][4])
{
	Matrix transf;
	transf.set(t[0][0], t[0][1], t[0][2], t[0][3],
		       t[1][0], t[1][1], t[1][2], t[1][3],
			   t[2][0], t[2][1], t[2][2], t[2][3],
			   t[3][0], t[3][1], t[3][2], t[3][3]);

	EDList<Vertex> * vlist;
	pVertex vertex;
	double coord[3];
	Vec3d ori, tgt;
	vlist = getClassifiedVertices(NULL);
	if( vlist ) {
		EDListIter<Vertex> ge_vtx_iter(vlist);
		while( vertex = ge_vtx_iter.next() ){
			vertex->get_coordinates(coord);
			ori.set(coord[0],coord[1],coord[2]);
			tgt = transf.preMult(ori);
			vertex->set_coordinates(tgt.x(), tgt.y(), tgt.z());
		}
	}
}

pBrep PSMesh::createBlade()
{
  pCurve crv, bcurves[4];
  pSurface bsurf[6];
  std::string str;
  int ncurve;

  pCurveList curves = getCurves(); 
  if( !curves || curves->size() < 4)
	  return NULL;

  // top face consists of LE#0, side1, LE#2, side2
  pCurveIter cIt = GC_Iter(curves);
  ncurve = 0;
  while(crv = GC_Iter_next(cIt)) {
	  str = crv->name();
	  if( str.compare("LE_#0") == 0 ) {
			bcurves[0] = crv;
	  } else if(str.compare("Side2_#0") == 0) {
			bcurves[1] = crv;
	  } else if(str.compare("Side1_#0") == 0) {
			bcurves[2] = crv;
	  } else if(str.compare("TE_#0") == 0) {
			bcurves[3] = crv;
	  } else {
		  continue;
	  }
	  crv->computeBezier();
	  if(++ncurve > 4) break;
  }
  GC_Iter_delete(cIt);
  if(ncurve == 4) {
	  bsurf[0] = createSurface(4,bcurves);
	  //bsurf[0]->computeControlNet();
  }

 // surf[1] = frontBladeFace();
 // surf[2] = sideBladeFace();

  pBrep blade=createBrep(1,bsurf);
  return blade;
}