//$c1 XRL 06/28/11 created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                      //
//========================================================================//
#ifndef MESH_TYPEDEF_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESH_TYPEDEF_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

template<class T> class EDList; 
template<class T> class EDListIter; 

extern "C"
{
// model
typedef int	   pGEntity;
typedef int	   pGFace;

// mesh
typedef class PSMesh	* pMWMesh;
typedef class Entity	* pEntity;
typedef class Vertex	* pVertex;
typedef class Edge		* pEdge;
typedef class Face		* pFace;
typedef class Region	* pRegion;

// list
typedef EDList<Vertex>	* pVertexList;
typedef EDList<Edge>	* pEdgeList;
typedef EDList<Face>	* pFaceList;
typedef EDList<Region>	* pRegionList;

typedef EDListIter<Vertex> * pVertexIter;
typedef EDListIter<Edge>   * pEdgeIter;
typedef EDListIter<Face>   * pFaceIter;
typedef EDListIter<Region> * pRegionIter;

// geometry
typedef class Curve		* pCurve;
typedef EDList<Curve>	* pCurveList;
typedef EDListIter<Curve> * pCurveIter;

typedef class Surface	* pSurface;
typedef EDList<Surface>	* pSurfaceList;
typedef EDListIter<Surface> * pSurfaceIter;

typedef class Brep		* pBrep;
typedef EDList<Brep>	* pBrepList;
typedef EDListIter<Brep	> * pBrepIter;

}

typedef enum {
  Tvertex,
  Tedge,
  Tface,
  Tregion,
  Tdeleted
} eType;

typedef enum {
  Tgvertex,
  Tgedge,
  Tgface,
  Tgregion
} gType;

typedef enum {
  Point,
  Line,
  Circle,
  Spline,
  Bezier,
  PolyLine,
  Parasolid
} geomType;

#endif