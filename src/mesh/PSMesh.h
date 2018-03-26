//$c7   XRL 11/06/2013 Add m_bFaceOrient
//$c6   XRL 12/10/2012 Replace member data "PSMeshType" with "skinned".
//$c5   XRL 10/15/2012 Added NQuad.
//$c4   XRL 07/20/2012 Added mesh regions.
//$c3   XRL 07/14/2012 Support unclassified mesh ie. classifiedOn=NULL.
//$c2   XRL 11/02/2011 Added remove operators.
//$c1   XRL 10/07/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                    //
//========================================================================//
#ifndef H_MTB_PARASOLID_MESH__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define H_MTB_PARASOLID_MESH__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "region.h"
#include "face.h"
#include "edge.h"
#include "vertex.h"
#include "geo\curve.h"
#include "geo\bsurface.h"
#include "geo\bbody.h"
#include "parasolid_kernel.h"
#include "EDList.h"
#include <fstream>

using namespace std;

class PSMesh
{
public:
  PSMesh(pGEntity classifiedOn);
  ~PSMesh();

  // useful info for efficient mesh region visualization
  void markAsSkinned() { m_bSkinned=true; }
  bool hasSkin() { return m_bSkinned; }
  void markFaceOrient(bool o) { m_bFaceOrient=o; }
  bool faceOrient() { return m_bFaceOrient; }

  // interrogation
  pGEntity model();

  int numRegions() const;
  int numFaces() const;
  int numQuadras() const;
  int numEdges() const;
  int numVertices() const;
  int numCurves() const;

  EDList<Region> * getClassifiedRegions(pGEntity);
  EDList<Face> * getClassifiedFaces(pGEntity);
  EDList<Edge> * getClassifiedEdges(pGEntity);
  EDList<Vertex> * getClassifiedVertices(pGEntity);
  EDList<Curve> * getCurves();
  EDList<Surface> * getSurfaces();

  void countClassifiedFaces(pGEntity,int *, int *);
  
  // mesh modification operations
  pRegion createRegion(pVertex *, pGEntity);
  pFace createFace(int, pVertex *, pGEntity);
  pEdge createEdge(pVertex, pVertex, pGEntity);
  pVertex createVertex(double[3], pGEntity);
  pVertex createVertex(double[3], double[2], pGEntity);
  // create brep geometry
  pCurve createCurve(geomType, char *, int, double *);
  pSurface createSurface(int, pCurve *);
  pBrep createBrep(int, pSurface *);
  pBrep createBlade();

  void remove(pVertex);
  void remove(pFace);
  void removeClassifiedMesh(gType, pGEntity);

  void transform(double[4][4]);

  // serialization
  void toMEDIT(const char *fileName);
  void toBDF(const char *fileName);
  bool toArrays(int *, int *, int **, pVertex **);
  pFace ithFace(int ith);
  pVertex ithVertex(int ith);
  int vertexIndexFromXYZ(double [3]);

protected:
  bool init();

  void add(pGEntity tag, pRegion r);
  void add(pGEntity tag, pFace f);
  void add(pGEntity tag, pEdge e);
  void add(pGEntity tag, pVertex v);
  void add(pCurve );
  void add(pSurface );
  void add(pBrep );

  void writePointRecord(ostream &, pVertex);

private:
  pGEntity classifiedOn;
  //static int mesh_count;

  PK_ATTDEF_t classifiedMVertexAttr; 
  PK_ATTDEF_t classifiedMEdgeAttr; 
  PK_ATTDEF_t classifiedMFaceAttr; 

  EDList<Vertex> * m_pMVList;
  EDList<Edge> * m_pMEList;
  EDList<Face> * m_pMFList;
  EDList<Region> * m_pMRList;
  EDList<Curve> * m_pCurveList;
  EDList<Surface> * m_pSurfaceList;
  EDList<Brep> * m_pBrepList;

  int NRegion, NFace, NEdge, NVertex, NCurve, NSurface, NBrep; 
  int NQuad;			// NTria = NFace - NQuad
  bool m_bSkinned;		// if true, the boundary trias/quads of mesh regions are created in m_pMFList
  bool m_bFaceOrient;	// if ture, normals in face orientation, eg tessadapt; otherwise, surface orientaion, cadsurf

};

#endif
