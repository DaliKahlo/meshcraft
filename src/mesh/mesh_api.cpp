//$c3   XRL 12/10/2012 Replace set/get mesh type with markAsSkinned and hasSkin.
//$c2   XRL 11/08/2012 Support EN_setID and EN_getID.
//$c1   XRL 01/25/2012 Create.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//									                                      //
//========================================================================//
#include "mesh_api.h"
#include "PSMesh.h"
#include "geo\farin.h"

/* create/delete */
pMWMesh M_new(pGEntity classifiedOn)
{
  pMWMesh me = new PSMesh(classifiedOn);  
  return me;
}

void M_delete(pMWMesh me)
{ delete me; me=0; }

void M_markAsSkinned(pMWMesh me)
{ me->markAsSkinned(); }

void M_markFaceOrient(pMWMesh me, bool o)
{ me->markFaceOrient(o); }

void M_transform(pMWMesh me,double t[4][4])
{ me->transform(t); }

/* modify */
pRegion M_createRegion(pMWMesh me, pVertex v[4], pGEntity whatin)
{ return me->createRegion(v,whatin); }

pFace M_createFace(pMWMesh me,int nv, pVertex *vts, pGEntity whatin)
{ return me->createFace(nv,vts,whatin); }

pEdge M_createEdge(pMWMesh me,pVertex v1, pVertex v2, pGEntity whatin)
{ return me->createEdge(v1,v2,whatin); }

pVertex M_createVertex(pMWMesh me,double xyz[3],pGEntity whatin)
{ return me->createVertex(xyz,whatin); }

pVertex M_createVertex2(pMWMesh me,double xyz[3], double param[2], pGEntity whatin)
{ return me->createVertex(xyz,param,whatin); }

void M_removeVertex(pMWMesh me,pVertex v)
{ me->remove(v); }

pCurve GM_createPolyLine(pMWMesh me, char *name, int np, double *pts)
{ return me->createCurve(PolyLine,name,np,pts); }

pBrep GM_createBlade(pMWMesh me)
{ return me->createBlade(); }

/* query */
pGEntity M_model(pMWMesh me)
{ return me->model(); }

bool M_hasSkin(pMWMesh me)
{ return me->hasSkin(); }

bool M_faceOrient(pMWMesh me)
{ return me->faceOrient(); }

int M_numberRegions(pMWMesh me)
{ return me->numRegions(); }

int M_numberFaces(pMWMesh me)
{ return me->numFaces(); }

int M_numberEdges(pMWMesh me)
{ return me->numEdges(); }

int M_numberQuadras(pMWMesh me)
{ return me->numQuadras(); }

int M_numberVertices(pMWMesh me)
{ return me->numVertices(); }

int GM_numberCurves(pMWMesh me)
{ return me->numCurves(); }

pCurveList GM_getCurves(pMWMesh me,int *num)
{ 
  *num = 0;
  pCurveList clist = me->getCurves();
  if(clist)
	  *num = clist->size();
  return clist;
}

pSurfaceList GM_getSurfaces(pMWMesh me,int *num)
{ 
  *num = 0;
  pSurfaceList surflist = me->getSurfaces();
  if(surflist)
	  *num = surflist->size();
  return surflist;
}

pRegionList M_getClassifiedRegions(pMWMesh me,pGEntity whatin,int *num)
{ 
  *num = 0;
  pRegionList rlist = me->getClassifiedRegions(whatin);
  if(rlist)
	  *num = rlist->size();
  return rlist;
}

pFaceList M_getClassifiedFaces(pMWMesh me,pGEntity whatin,int *num)
{ 
  *num = 0;
  pFaceList flist = me->getClassifiedFaces(whatin);
  if(flist)
	  *num = flist->size();
  return flist;
}

pEdgeList M_getClassifiedEdges(pMWMesh me,pGEntity whatin,int *num)
{
  *num = 0;
  pEdgeList elist = me->getClassifiedEdges(whatin);
  if(elist)
	  *num = elist->size();
  return elist;
}

pVertexList M_getClassifiedVertices(pMWMesh me,pGEntity whatin,int *num)
{
  *num = 0;
  pVertexList vlist = me->getClassifiedVertices(whatin);
  if(vlist)
	  *num = vlist->size();
  return vlist;
}

int R_numVertices(pRegion r)
{ return r->numVertices(); }

pVertex R_vertex(pRegion r ,int i)
{ return r->get_vertex(i); }

int F_numVertices(pFace f)
{ return f->numVertices(); }

pVertex F_vertex(pFace f ,int i)
{ return f->get_vertex(i); }

pVertex E_vertex(pEdge e ,int i)
{ return e->get_vertex(i); }

void V_getCoord(pVertex v,double xyz[3])
{ v->get_coordinates(xyz); return; }

bool V_getParam(pVertex v,double uv[2])
{ v->get_param(uv); return true; }

int V_getTempID(pVertex v)
{ return v->get_temp_id(); }

void V_setTempID(pVertex v, int id)
{ v->set_temp_id(id); return; }

int EN_getID(pEntity v)
{ return v->get_id(); }

void EN_resetID(pEntity v, int id)
{ v->set_id(id); return; }

void V_resetClassificationType(pVertex v, int ttype)
{ v->reset_classification_type(ttype); }

int V_getClassificationType(pVertex v)
{ return v->get_classification_type(); }

/* list operations */
pVertexIter VIter(pVertexList vlist)
{ return (new EDListIter<Vertex> (vlist)); }

pVertex VIter_next(pVertexIter it)
{ return it->next(); }

void VIter_delete(pVertexIter it)
{ delete it; return; }

pEdgeIter EIter(pEdgeList elist)
{ return (new EDListIter<Edge> (elist)); }

pEdge EIter_next(pEdgeIter it)
{ return it->next(); }

void EIter_delete(pEdgeIter it)
{ delete it; }

pFaceIter FIter(pFaceList flist)
{ return (new EDListIter<Face> (flist)); }

pFace FIter_next(pFaceIter it)
{ return it->next(); }

void FIter_delete(pFaceIter it)
{ delete it; }

pRegionIter RIter(pRegionList rlist)
{ return (new EDListIter<Region> (rlist)); }

pRegion RIter_next(pRegionIter it)
{ return it->next(); }

void RIter_delete(pRegionIter it)
{ delete it; }

pCurveIter GC_Iter(pCurveList clist)
{ return (new EDListIter<Curve> (clist)); }

pCurve GC_Iter_next(pCurveIter it)
{ return it->next(); }

void GC_Iter_delete(pCurveIter it)
{ delete it; }

pSurfaceIter GS_Iter(pSurfaceList slist)
{ return (new EDListIter<Surface> (slist)); }

pSurface GS_Iter_next(pSurfaceIter it)
{ return it->next(); }

void GS_Iter_delete(pSurfaceIter it)
{ delete it; }

/* serialize */
void M_toMEDIT(pMWMesh me, const char *fname)
{ me->toMEDIT(fname); }

void M_toBDF(pMWMesh me, const char *fname)
{ me->toBDF(fname); }

void M_toARRAY(pMWMesh me, int *nf, int *nv, int **tria, pVertex **verts)
{ me->toArrays(nf,nv,tria,verts); }

pFace M_ithFaceOfARRAY(pMWMesh me, int ith)
{ return me->ithFace(ith); }

pVertex M_ithVertexOfARRAY(pMWMesh me, int ith)
{ return me->ithVertex(ith); }

int M_vertexIndexFromXYZ(pMWMesh me, double picked[3])
{ return me->vertexIndexFromXYZ(picked); }

/*  */
int GC_numPoints(pCurve c)
{ return c->numPoints(); }

void GC_ithPoint(pCurve c, int i, double *x, double *y, double *z)
{ return c->ithPoint(i,x,y,z); }

int GS_numCurves(pSurface s)
{ return s->numCurves(); }

pCurve GS_ithCurve(pSurface s, int i)
{ return s->ithCurve(i); }

double* c2_spline_interp(int degree, int l, double data_x[], double data_y[], double bspl_x[], double bspl_y[])
{
	return farin::c2_spline_interp(l,degree,data_x,data_y,bspl_x,bspl_y);
}

void free_knot_vector(double *knots)
{  farin::free_knot_vector(knots); }

void bspl_to_points(int degree, int l, double coeff[], double knot[], int dense, double points_x[], int *npoint)
{
	farin::bspl_to_points(degree,l,coeff,knot,dense,points_x,npoint);
}

double* create_centripetal_knotsequence_3D(int numPoints, double *coords)
{
	return farin::create_centripetal_knotsequence_3D(numPoints,coords);
}
