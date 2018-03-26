//========================================================================//
//              Copyright 2012 (Unpublished Material)                     //
//									                                      //
//========================================================================//

#ifndef MESH_API_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESH_API_H__CD27375B_D581_11D2_8BF9_0000F8071DC8_INCLUDED_

#include "typedef.h"

#ifdef MESH_API_IMPL
#define meshEXPORT _declspec( dllexport )
#else
#define meshEXPORT _declspec( dllimport )
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* create/delete */
meshEXPORT pMWMesh M_new(pGEntity); 
meshEXPORT void M_delete(pMWMesh); 
meshEXPORT void M_markAsSkinned(pMWMesh); 
meshEXPORT void M_markFaceOrient(pMWMesh,bool); 

/* modify */
meshEXPORT pRegion M_createRegion(pMWMesh, pVertex[4], pGEntity);
meshEXPORT pFace M_createFace(pMWMesh,int, pVertex *, pGEntity);
meshEXPORT pEdge M_createEdge(pMWMesh,pVertex, pVertex, pGEntity);
meshEXPORT pVertex M_createVertex(pMWMesh,double[3],pGEntity); 
meshEXPORT pVertex M_createVertex2(pMWMesh,double[3], double[2], pGEntity);

meshEXPORT void M_removeVertex(pMWMesh,pVertex);
meshEXPORT void M_transform(pMWMesh,double[4][4]); 

/* query */
meshEXPORT pGEntity M_model(pMWMesh); 
meshEXPORT bool M_hasSkin(pMWMesh); 
meshEXPORT bool M_faceOrient(pMWMesh);
meshEXPORT int M_numberRegions(pMWMesh);
meshEXPORT int M_numberFaces(pMWMesh);
meshEXPORT int M_numberEdges(pMWMesh);
meshEXPORT int M_numberQuadras(pMWMesh);
meshEXPORT int M_numberVertices(pMWMesh);

meshEXPORT pRegionList M_getClassifiedRegions(pMWMesh,pGEntity,int *);
meshEXPORT pFaceList M_getClassifiedFaces(pMWMesh,pGEntity,int *);
meshEXPORT pEdgeList M_getClassifiedEdges(pMWMesh,pGEntity,int *);
meshEXPORT pVertexList M_getClassifiedVertices(pMWMesh,pGEntity,int *);

meshEXPORT int EN_getID(pEntity);
meshEXPORT void EN_resetID(pEntity, int);

meshEXPORT int R_numVertices(pRegion);
meshEXPORT int F_numVertices(pFace);

meshEXPORT pVertex R_vertex(pRegion,int);
meshEXPORT pVertex F_vertex(pFace,int);
meshEXPORT pVertex E_vertex(pEdge,int);

meshEXPORT void V_getCoord(pVertex,double[3]);
meshEXPORT bool V_getParam(pVertex,double[2]);
meshEXPORT void V_resetClassificationType(pVertex, int);
meshEXPORT int V_getClassificationType(pVertex);
meshEXPORT int V_getTempID(pVertex);
meshEXPORT void V_setTempID(pVertex, int);

/* iterators */
meshEXPORT pVertexIter VIter(pVertexList);
meshEXPORT pVertex VIter_next(pVertexIter);
meshEXPORT void VIter_delete(pVertexIter);

meshEXPORT pEdgeIter EIter(pEdgeList);
meshEXPORT pEdge EIter_next(pEdgeIter);
meshEXPORT void EIter_delete(pEdgeIter);

meshEXPORT pFaceIter FIter(pFaceList);
meshEXPORT pFace FIter_next(pFaceIter);
meshEXPORT void FIter_delete(pFaceIter);

meshEXPORT pRegionIter RIter(pRegionList);
meshEXPORT pRegion RIter_next(pRegionIter);
meshEXPORT void RIter_delete(pRegionIter);

/* serialize */
meshEXPORT void M_toMEDIT(pMWMesh, const char *);
meshEXPORT void M_toBDF(pMWMesh, const char *);
meshEXPORT void M_toARRAY(pMWMesh, int *, int *, int **, pVertex **);
meshEXPORT pFace M_ithFaceOfARRAY(pMWMesh me, int ith);
meshEXPORT pVertex M_ithVertexOfARRAY(pMWMesh me, int ith);
meshEXPORT int M_vertexIndexFromXYZ(pMWMesh me, double picked[3]);


/* geometry */
meshEXPORT pCurve GM_createPolyLine(pMWMesh me, char *name, int np, double *);
meshEXPORT pBrep GM_createBlade(pMWMesh me);

meshEXPORT int GM_numberCurves(pMWMesh);
meshEXPORT pCurveList GM_getCurves(pMWMesh,int *);
meshEXPORT pSurfaceList GM_getSurfaces(pMWMesh,int *);

meshEXPORT pCurveIter GC_Iter(pCurveList);
meshEXPORT pCurve GC_Iter_next(pCurveIter);
meshEXPORT void GC_Iter_delete(pCurveIter);

meshEXPORT pSurfaceIter GS_Iter(pSurfaceList);
meshEXPORT pSurface GS_Iter_next(pSurfaceIter);
meshEXPORT void GS_Iter_delete(pSurfaceIter);

meshEXPORT int GC_numPoints(pCurve);
meshEXPORT void GC_ithPoint(pCurve, int, double *, double *, double *);

meshEXPORT int GS_numCurves(pSurface);
meshEXPORT pCurve GS_ithCurve(pSurface, int);

meshEXPORT double *c2_spline_interp(int degree, int l, double data_x[], double data_y[], double bspl_x[], double bspl_y[]);
meshEXPORT void free_knot_vector(double *);
meshEXPORT void bspl_to_points(int degree, int l, double coeff_x[], double knot[], int dense, double points_x[], int *point_num);

meshEXPORT double* create_centripetal_knotsequence_3D(int numPoints, double *coords);

#ifdef __cplusplus
}
#endif


#endif