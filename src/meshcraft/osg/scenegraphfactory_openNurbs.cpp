
//$c1   XRL 06/09/2012 Created 
//========================================================================//
//
// CMeshSG class implementation 
//
//=========================================================================

#include "stdafx.h"
#include "MeshWorks.h"
#include "mesh\mesh_api.h"
#include "scenegraphfactory.h"
#include "parasolid_kernel.h"
#include "openNurbs\opennurbs.h"

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osg/Point>
#include <osg/ShadeModel>
#include <osg/Material>
#include <osg/PolygonOffset>

#define MERGE_DUPLICATE_EDGES 1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

osg::ref_ptr<osg::Geode> CSGFactory::bsplineCurves(pMWMesh me, osg::Vec4 color)
{
	int i;
   int degree = 3;
   int l = 4;			// number of intervals (or segments)
   double X[7], Y[7];	// = l+3 [0,l+2]: [1] and [l+1] are expected to be empty and filled by bessel_ends "tangent Bezier points"
   double bspl_x[7], bspl_y[7];
   
   X[0]=0.1; X[1]=0.0; X[2]=0.4; X[3]=1.2; X[4]=1.8; X[5]=0.0; X[6]=2.0;
   Y[0]=0.1; Y[1]=0.0; Y[2]=0.7; Y[3]=0.6; Y[4]=1.1; Y[5]=0.0; Y[6]=0.9;

	osg::Geode* geode = new osg::Geode();

	osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;

	// show the given polyline
	osg::Vec3Array* vertices = new osg::Vec3Array;
	vertices->push_back(osg::Vec3(X[0], Y[0], 0));
	for(i=2; i<=l; i++) {
		vertices->push_back(osg::Vec3(X[i], Y[i], 0));
	}
	vertices->push_back(osg::Vec3(X[l+2], Y[l+2], 0));
	geo->setVertexArray(vertices);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
    geo->setColorArray(colors);
    geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,l+1));
	geode->addDrawable(geo);

	// compute and show control points
	double* knots = c2_spline_interp(degree,l,X,Y,bspl_x, bspl_y);

	osg::ref_ptr<osg::Geometry> bpoly = new osg::Geometry;
	osg::Vec3Array* bpoly_vtx = new osg::Vec3Array;
	for(i=0; i<=l+2; i++) {
		bpoly_vtx->push_back(osg::Vec3(bspl_x[i], bspl_y[i], 0));
	}
	bpoly->setVertexArray(bpoly_vtx);

    osg::Vec4Array* bpoly_colors = new osg::Vec4Array;
    bpoly_colors->push_back(osg::Vec4(1.0f,0.0f,1.0f,1.0f));
    bpoly->setColorArray(bpoly_colors);
    bpoly->setColorBinding(osg::Geometry::BIND_OVERALL);

	bpoly->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,l+3));
	geode->addDrawable(bpoly);

	// evaluate and show the b-spline
   int dense = 10;		// how many elements per segment
   int point_xnum, point_ynum;
   double points_x[100], points_y[100];
   bspl_to_points(degree, l, bspl_x, knots, dense, points_x, &point_xnum);
   bspl_to_points(degree, l, bspl_y, knots, dense, points_y, &point_ynum);

   	osg::ref_ptr<osg::Geometry> bspl = new osg::Geometry;
	osg::Vec3Array* bspl_vtx = new osg::Vec3Array;
	for(i=0; i<point_xnum; i++) {
		bspl_vtx->push_back(osg::Vec3(points_x[i], points_y[i], 0));
	}
	bspl->setVertexArray(bspl_vtx);

    osg::Vec4Array* bspl_colors = new osg::Vec4Array;
    bspl_colors->push_back(osg::Vec4(1.0f,0.5f,0.5f,1.0f));
    bspl->setColorArray(bspl_colors);
    bspl->setColorBinding(osg::Geometry::BIND_OVERALL);

	bspl->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,point_xnum));
	geode->addDrawable(bspl);

	free_knot_vector(knots);
	return geode;
}

osg::ref_ptr<osg::Geode> CSGFactory::bSurfaceTester(pMWMesh me)
{
	/// define a 3D Besier surface
	int xcnt = 3, ycnt = 3;
	int dim = 3;
	bool is_rat = false;
	ON_BezierSurface *bezsurf = new ON_BezierSurface(dim, is_rat, xcnt, ycnt);

	ON_3dPoint ctrlpt(1.0, 1.0, 1.0);		bezsurf->SetCV(0, 0, ctrlpt);
	ctrlpt.Set(2.0, 1.0, 2.0);				bezsurf->SetCV(0, 1, ctrlpt);
	ctrlpt.Set(3.0, 1.0, 1.0);				bezsurf->SetCV(0, 2, ctrlpt);
	ctrlpt.Set(1.0, 2.0, 1.0);				bezsurf->SetCV(1, 0, ctrlpt);
	ctrlpt.Set(2.0, 2.0, 2.0);				bezsurf->SetCV(1, 1, ctrlpt);
	ctrlpt.Set(3.0, 2.0, 0.0);				bezsurf->SetCV(1, 2, ctrlpt);
	ctrlpt.Set(1.0, 3.0, 2.0);				bezsurf->SetCV(2, 0, ctrlpt);
	ctrlpt.Set(2.0, 3.0, 2.0);				bezsurf->SetCV(2, 1, ctrlpt);
	ctrlpt.Set(3.0, 3.0, 0.0);				bezsurf->SetCV(2, 2, ctrlpt);

	/// display 
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	ON_3dPoint point;

	ON_BoundingBox box = bezsurf->BoundingBox();
	ON_Interval uRange = bezsurf->Domain(0);
	ON_Interval vRange = bezsurf->Domain(1);
	double uFirst = uRange.Min();
	double uLast = uRange.Max();
	double vFirst = vRange.Min();
	double vLast = vRange.Max();

	int uDense=10, vDense=10;
	double u,v, values[12];
	double udelta = (uLast - uFirst) / uDense;
	double vdelta = (vLast - vFirst) / vDense;

	int wireframe = 0;
	if(wireframe) {
	// Approximation in v direction.
	for (u = uFirst; u <= uLast; u += udelta)
	{
		osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
		osg::ref_ptr<osg::Vec3Array> pointsVec = new osg::Vec3Array();
		for (v = vFirst; v <= vLast; v += vdelta)
		{
			bezsurf->Evaluate(u,v,0,3,values);
			pointsVec->push_back(osg::Vec3(values[0], values[1], values[2]));
		}
		// Set the colors.
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
		linesGeom->setColorArray(colors.get());
		linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		// Set the normal in the same way of color.
		osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
		linesGeom->setNormalArray(normals.get());
		linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
		// Set vertex array.
		linesGeom->setVertexArray(pointsVec);
		linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pointsVec->size()));
		geode->addDrawable(linesGeom.get());
	}
	
	// Approximation in u direction.
	for ( v = vFirst; v <= vLast; v += vdelta)
	{
		osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
		osg::ref_ptr<osg::Vec3Array> pointsVec = new osg::Vec3Array();
		for (u = vFirst; u <= uLast; u += udelta)
		{
			bezsurf->Evaluate(u,v,0,3,values);
			pointsVec->push_back(osg::Vec3(values[0], values[1], values[2]));
		}
		// Set the colors.
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
		linesGeom->setColorArray(colors.get());
		linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		// Set the normal in the same way of color.
		osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
		linesGeom->setNormalArray(normals.get());
		linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
		// Set vertex array.
		linesGeom->setVertexArray(pointsVec);
		linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pointsVec->size()));
		geode->addDrawable(linesGeom.get());
	}


	} else { // shaded
		int i,j,k;
		osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array;
		geometry->setVertexArray(vertexArray);
		osg::ref_ptr<osg::Vec3Array> colorArray = new osg::Vec3Array;
		colorArray->push_back(osg::Vec3(1, 1, 1));
		geometry->setColorArray(colorArray);
		geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		for (u = uFirst; u <= uLast; u += udelta)
		{
			for (v = vFirst; v <= vLast; v += vdelta)
			{
				bezsurf->Evaluate(u,v,0,3,values);
				vertexArray->push_back(osg::Vec3(values[0], values[1], values[2]));
			}
		}

		osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
		for(i=0; i<uDense; i++) {
			k=(vDense+1)*i;
			for(j=0; j<vDense; j++) {
				quads->push_back(k+j);
				quads->push_back(k+j+vDense+1);
				quads->push_back(k+1+j+vDense+1);
				quads->push_back(k+1+j);
			}			
		}
		geometry->addPrimitiveSet(quads);
		geode->addDrawable(geometry);
	}

	delete bezsurf;
	return geode.get();
}

osg::ref_ptr<osg::Group> CSGFactory::createPolyLines(pMWMesh me, osg::Vec4 color)
{
	int i, num;
	pCurveList curves = GM_getCurves(me,&num);
	if( !curves || num < 1)
		return NULL;

	osg::ref_ptr<osg::Group> curvegroup = new osg::Group;
	osg::Geode* geode = new osg::Geode();
	curvegroup->addChild(geode);

	double x,y,z;
	int nPoints;
	pCurveIter cIt = GC_Iter(curves);
	while(pCurve crv = GC_Iter_next(cIt)) {
		nPoints = GC_numPoints(crv);

		osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
		osg::Vec3Array* vertices = new osg::Vec3Array;
		for(i=0; i<nPoints; i++) {
			GC_ithPoint(crv,i,&x,&y,&z);
			vertices->push_back(osg::Vec3(x, y, z));
		}
		geo->setVertexArray(vertices);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
		geo->setColorArray(colors);
		geo->setColorBinding(osg::Geometry::BIND_OVERALL);

		geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,nPoints));
		geode->addDrawable(geo);
	}
	GC_Iter_delete(cIt);

	return curvegroup;
}

osg::ref_ptr<osg::Group> CSGFactory::createBladeCurves(pMWMesh me, osg::Vec4 color)
{
	int i, num;
	pCurveList curves = GM_getCurves(me,&num);
	if( !curves || num < 1)
		return NULL;

	osg::ref_ptr<osg::Group> curvegroup = new osg::Group;

	std::vector<double> arrayX;
	std::vector<double> arrayY;
	std::vector<double> arrayZ;
	double x,y,z;
	int nK;
	pCurveIter cIt = GC_Iter(curves);
	while(pCurve crv = GC_Iter_next(cIt)) {
		nK = GC_numPoints(crv);
		if(nK < 2)
			continue;
		arrayX.clear();
		arrayY.clear();
		for(i=0; i<nK; i++) {
			GC_ithPoint(crv,i,&x,&y,&z);
			arrayX.push_back(x);
			arrayY.push_back(y);
		}

		if(nK < 15) {
			osg::ref_ptr<osg::Geode> bzcurve = toBZCurveAndShow(arrayX,arrayY,color);
			if(bzcurve)
				curvegroup->addChild(bzcurve);
		} else {
			osg::ref_ptr<osg::Geode> bspline = toBSplineAndShow(arrayX,arrayY,color);
			if(bspline)
				curvegroup->addChild(bspline);
		}
	}
	GC_Iter_delete(cIt);

	return curvegroup;
}

osg::ref_ptr<osg::Geode> CSGFactory::toBSplineAndShow(std::vector<double> &X, std::vector<double> &Y, osg::Vec4 color)
{
   int i;
   int numPoints = (int )X.size();
   int l = numPoints - 1;			// number of intervals (or segments)
   double *XX, *YY;
   double *cv_x, *cv_y;

   int degree = 3;
   if(numPoints < 4) 
	   return NULL;

	osg::Geode* geode = new osg::Geode();

	osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;

	/// show the given polyline
	osg::Vec3Array* vertices = new osg::Vec3Array;
	for(i=0; i<numPoints; i++) 
		vertices->push_back(osg::Vec3(X[i], Y[i], 0));
	geo->setVertexArray(vertices);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
    geo->setColorArray(colors);
    geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,l+1));
	geode->addDrawable(geo);

	/// compute control polygon
	XX = new double[l+3];
	YY = new double[l+3];
	cv_x = new double[l+3];
	cv_y = new double[l+3];
	/// leave XX[1] and XX[l+1] not filled, same for YY
	XX[0] = X[0]; XX[1] = 0.0;
	YY[0] = Y[0]; YY[1] = 0.0;
	for(i=2; i<=l; i++) {
		XX[i] = X[i-1];
		YY[i] = Y[i-1];
	}
	XX[l+1] = 0.0; XX[l+2] = X[l];
	YY[l+1] = 0.0; YY[l+2] = Y[l];

	double* knots = c2_spline_interp(degree,l,XX,YY,cv_x, cv_y);

	osg::ref_ptr<osg::Geometry> bpoly = new osg::Geometry;
	osg::Vec3Array* bpoly_vtx = new osg::Vec3Array;
	for(i=0; i<=l+2; i++) {
		bpoly_vtx->push_back(osg::Vec3(cv_x[i], cv_y[i], 0));
	}
	bpoly->setVertexArray(bpoly_vtx);

    osg::Vec4Array* bpoly_colors = new osg::Vec4Array;
    bpoly_colors->push_back(osg::Vec4(1.0f,0.0f,1.0f,1.0f));
    bpoly->setColorArray(bpoly_colors);
    bpoly->setColorBinding(osg::Geometry::BIND_OVERALL);

	bpoly->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,l+3));
	geode->addDrawable(bpoly);

	// evaluate and show the b-spline
   int dense = 10;		// how many elements per segment
   int xnum, ynum;
   double *points_x = new double[(dense+1)*(l+degree-1)+1];
   double *points_y = new double[(dense+1)*(l+degree-1)+1];
   bspl_to_points(degree, l, cv_x, knots, dense, points_x, &xnum);
   bspl_to_points(degree, l, cv_y, knots, dense, points_y, &ynum);

   	osg::ref_ptr<osg::Geometry> bspl = new osg::Geometry;
	osg::Vec3Array* bspl_vtx = new osg::Vec3Array;
	for(i=0; i<xnum; i++) 
		bspl_vtx->push_back(osg::Vec3(points_x[i], points_y[i], 0));
	bspl->setVertexArray(bspl_vtx);

    osg::Vec4Array* bspl_colors = new osg::Vec4Array;
    bspl_colors->push_back(osg::Vec4(1.0f,0.5f,0.5f,1.0f));
    bspl->setColorArray(bspl_colors);
    bspl->setColorBinding(osg::Geometry::BIND_OVERALL);

	bspl->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,xnum));
	geode->addDrawable(bspl);

	delete [] points_x;
	delete [] points_y;
	delete [] cv_x;
	delete [] cv_y;
	delete [] XX;
	delete [] YY;
	free_knot_vector(knots);
	return geode;
}


osg::ref_ptr<osg::Geode> CSGFactory::toBZCurveAndShow(std::vector<double> &X, std::vector<double> &Y, osg::Vec4 color)
{
   int i;
   int numPoints = (int )X.size();
 
	osg::Geode* geode = new osg::Geode();

	osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;

	/// show the given polyline
	osg::Vec3Array* vertices = new osg::Vec3Array;
	for(i=0; i<numPoints; i++) 
		vertices->push_back(osg::Vec3(X[i], Y[i], 0));
	geo->setVertexArray(vertices);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
    geo->setColorArray(colors);
    geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	geo->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,numPoints));
	geode->addDrawable(geo);

	/// compute and show control polygon
  ON_3dPoint pt;

  int pt_dim = 3;
  int order = 3;
  bool ret;
  double* coords = new double[3*numPoints];

  for(i=0; i<numPoints; i++) {
	  coords[3*i] = X[i];
	  coords[3*i + 1] = Y[i];
	  coords[3*i + 2] =0.0;
  }

  ON_BezierCurve curve(pt_dim, FALSE, order);

  double* param = create_centripetal_knotsequence_3D(numPoints,coords);
  ret = curve.Loft(pt_dim, numPoints, 3, coords, 1, param);
  free_knot_vector(param);
  delete [] coords;

  int cv_count = curve.CVCount();
  order = curve.Order();
 
	osg::ref_ptr<osg::Geometry> bpoly = new osg::Geometry;
	osg::Vec3Array* bpoly_vtx = new osg::Vec3Array;

  for(i=0; i<order; i++) {
	curve.GetCV(i,pt);
	bpoly_vtx->push_back(osg::Vec3(pt[0], pt[1], pt[2]));
  }
	bpoly->setVertexArray(bpoly_vtx);

    osg::Vec4Array* bpoly_colors = new osg::Vec4Array;
    bpoly_colors->push_back(osg::Vec4(1.0f,0.0f,1.0f,1.0f));
    bpoly->setColorArray(bpoly_colors);
    bpoly->setColorBinding(osg::Geometry::BIND_OVERALL);

	bpoly->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,order));
	geode->addDrawable(bpoly);

	// evaluate and show the bezier
    ON_Interval intvl = curve.Domain();

    double t;
    int dense=50;

	osg::ref_ptr<osg::Geometry> bspl = new osg::Geometry;
	osg::Vec3Array* bspl_vtx = new osg::Vec3Array;
    for(i=0; i<=dense; i++) {
	  t = (double)i / dense;
	  ret = curve.EvPoint(t,pt);
	  bspl_vtx->push_back(osg::Vec3(pt[0], pt[1], pt[2]));
    }
	bspl->setVertexArray(bspl_vtx);

    osg::Vec4Array* bspl_colors = new osg::Vec4Array;
    bspl_colors->push_back(osg::Vec4(1.0f,0.5f,0.5f,1.0f));
    bspl->setColorArray(bspl_colors);
    bspl->setColorBinding(osg::Geometry::BIND_OVERALL);

	bspl->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,dense+1));
	geode->addDrawable(bspl);

	return geode;
}

osg::ref_ptr<osg::Geode> CSGFactory::showBladeSurfaces(pMWMesh me)
{
	int i, num;
	pSurfaceList surfs = GM_getSurfaces(me,&num);
	if( !surfs || num < 1)
		return NULL;

	osg::ref_ptr<osg::Geode> surfgroup = new osg::Geode;

	pCurve crv;
	int nc;
	pSurfaceIter sIt = GS_Iter(surfs);
	while(pSurface surf = GS_Iter_next(sIt)) {
		nc = GS_numCurves(surf);
		if(nc != 4)
			continue;
		//arrayX.clear();
		//arrayY.clear();
		for(i=0; i<nc; i++) {
			crv = GS_ithCurve(surf,i);
		//	arrayX.push_back(x);
		//	arrayY.push_back(y);
		}

		//if(nK < 15) {
		//	osg::ref_ptr<osg::Geode> bzcurve = toBZCurveAndShow(arrayX,arrayY,color);
		//	if(bzcurve)
		//		curvegroup->addChild(bzcurve);
		//} else {
		//	osg::ref_ptr<osg::Geode> bspline = toBSplineAndShow(arrayX,arrayY,color);
		//	if(bspline)
		//		curvegroup->addChild(bspline);
		//}
	}
	GS_Iter_delete(sIt);

	return surfgroup;


	///// define a 3D Besier surface
	//int xcnt = 3, ycnt = 3;
	//int dim = 3;
	//bool is_rat = false;
	//ON_BezierSurface *bezsurf = new ON_BezierSurface(dim, is_rat, xcnt, ycnt);

	//ON_3dPoint ctrlpt(1.0, 1.0, 1.0);		bezsurf->SetCV(0, 0, ctrlpt);
	//ctrlpt.Set(2.0, 1.0, 2.0);				bezsurf->SetCV(0, 1, ctrlpt);
	//ctrlpt.Set(3.0, 1.0, 1.0);				bezsurf->SetCV(0, 2, ctrlpt);
	//ctrlpt.Set(1.0, 2.0, 1.0);				bezsurf->SetCV(1, 0, ctrlpt);
	//ctrlpt.Set(2.0, 2.0, 2.0);				bezsurf->SetCV(1, 1, ctrlpt);
	//ctrlpt.Set(3.0, 2.0, 0.0);				bezsurf->SetCV(1, 2, ctrlpt);
	//ctrlpt.Set(1.0, 3.0, 2.0);				bezsurf->SetCV(2, 0, ctrlpt);
	//ctrlpt.Set(2.0, 3.0, 2.0);				bezsurf->SetCV(2, 1, ctrlpt);
	//ctrlpt.Set(3.0, 3.0, 0.0);				bezsurf->SetCV(2, 2, ctrlpt);

	///// display 
	//osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	//ON_3dPoint point;

	//ON_BoundingBox box = bezsurf->BoundingBox();
	//ON_Interval uRange = bezsurf->Domain(0);
	//ON_Interval vRange = bezsurf->Domain(1);
	//double uFirst = uRange.Min();
	//double uLast = uRange.Max();
	//double vFirst = vRange.Min();
	//double vLast = vRange.Max();

	//int uDense=10, vDense=10;
	//double u,v, values[12];
	//double udelta = (uLast - uFirst) / uDense;
	//double vdelta = (vLast - vFirst) / vDense;

	//int wireframe = 0;
	//if(wireframe) {
	//// Approximation in v direction.
	//for (u = uFirst; u <= uLast; u += udelta)
	//{
	//	osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
	//	osg::ref_ptr<osg::Vec3Array> pointsVec = new osg::Vec3Array();
	//	for (v = vFirst; v <= vLast; v += vdelta)
	//	{
	//		bezsurf->Evaluate(u,v,0,3,values);
	//		pointsVec->push_back(osg::Vec3(values[0], values[1], values[2]));
	//	}
	//	// Set the colors.
	//	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	//	colors->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
	//	linesGeom->setColorArray(colors.get());
	//	linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
	//	// Set the normal in the same way of color.
	//	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	//	normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	//	linesGeom->setNormalArray(normals.get());
	//	linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
	//	// Set vertex array.
	//	linesGeom->setVertexArray(pointsVec);
	//	linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pointsVec->size()));
	//	geode->addDrawable(linesGeom.get());
	//}
	//
	//// Approximation in u direction.
	//for ( v = vFirst; v <= vLast; v += vdelta)
	//{
	//	osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
	//	osg::ref_ptr<osg::Vec3Array> pointsVec = new osg::Vec3Array();
	//	for (u = vFirst; u <= uLast; u += udelta)
	//	{
	//		bezsurf->Evaluate(u,v,0,3,values);
	//		pointsVec->push_back(osg::Vec3(values[0], values[1], values[2]));
	//	}
	//	// Set the colors.
	//	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	//	colors->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
	//	linesGeom->setColorArray(colors.get());
	//	linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
	//	// Set the normal in the same way of color.
	//	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	//	normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	//	linesGeom->setNormalArray(normals.get());
	//	linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
	//	// Set vertex array.
	//	linesGeom->setVertexArray(pointsVec);
	//	linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, pointsVec->size()));
	//	geode->addDrawable(linesGeom.get());
	//}


	//} else { // shaded
	//	int i,j,k;
	//	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//	osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array;
	//	geometry->setVertexArray(vertexArray);
	//	osg::ref_ptr<osg::Vec3Array> colorArray = new osg::Vec3Array;
	//	colorArray->push_back(osg::Vec3(1, 1, 1));
	//	geometry->setColorArray(colorArray);
	//	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	//	for (u = uFirst; u <= uLast; u += udelta)
	//	{
	//		for (v = vFirst; v <= vLast; v += vdelta)
	//		{
	//			bezsurf->Evaluate(u,v,0,3,values);
	//			vertexArray->push_back(osg::Vec3(values[0], values[1], values[2]));
	//		}
	//	}

	//	osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
	//	for(i=0; i<uDense; i++) {
	//		k=(vDense+1)*i;
	//		for(j=0; j<vDense; j++) {
	//			quads->push_back(k+j);
	//			quads->push_back(k+j+vDense+1);
	//			quads->push_back(k+1+j+vDense+1);
	//			quads->push_back(k+1+j);
	//		}			
	//	}
	//	geometry->addPrimitiveSet(quads);
	//	geode->addDrawable(geometry);
	//}

	//delete bezsurf;
	//return geode.get();
}
