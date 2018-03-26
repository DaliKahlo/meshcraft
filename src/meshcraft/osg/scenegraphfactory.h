//$c1   XRL 01/08/2012 Created 
//========================================================================//
//
// scenegraph.h
//
//=========================================================================

#ifndef MESHSCENEGRAPH_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define MESHSCENEGRAPH_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#include <osg/Geode>
#include <osg/Switch>
#include "mesh/mesh_api.h"
#include "../meshing/AssemblyMesh.h"
#include "MeshWorkDefs.h"
#include "osg/triplet.h"

class CSGFactory  
{
public:
	CSGFactory();
	~CSGFactory();

	void clear();
	void setDefaultFaceColor(float,float,float);
	osg::ref_ptr<osg::Group> create(AssemblyMesh *);
	bool addMeshEdgeGeode(AssemblyMesh *, osg::Switch *);

private:
	PK_ATTDEF_t colour_attdef;
	float defaultfaceColor[3];
    std::map<Triplet, bool> *_pFaceMap;
	bool _tetSkinInMDB;					// if true, no need to generate _pFaceMap

	osg::ref_ptr<osg::Switch> createSwitchAndFaceGeode(pMWMesh,int);
	osg::ref_ptr<osg::Geometry> createFaceGeom(pMWMesh, pGFace, bool, osg::Vec4);
	osg::ref_ptr<osg::Geometry> createFacetFaceGeom(pMWMesh, pGFace, osg::Vec4);
	osg::ref_ptr<osg::Geometry> createInteriorPoints(pMWMesh, osg::Vec4 );
	osg::ref_ptr<osg::Geometry> createInteriorPoints2(pMWMesh, int, osg::Vec4);
	osg::ref_ptr<osg::Geometry> createBoundaryEdges(pMWMesh, osg::Vec4);
	osg::ref_ptr<osg::Geometry> createModelVertices(pMWMesh, osg::Vec4);
	osg::ref_ptr<osg::Group> createPolyLines(pMWMesh, osg::Vec4);
	osg::ref_ptr<osg::Group> createBladeCurves(pMWMesh, osg::Vec4);
	osg::ref_ptr<osg::Geode> bsplineCurves(pMWMesh, osg::Vec4);
	osg::ref_ptr<osg::Geode> toBSplineAndShow(std::vector<double> &, std::vector<double> &, osg::Vec4);
	osg::ref_ptr<osg::Geode> toBZCurveAndShow(std::vector<double> &, std::vector<double> &, osg::Vec4);
	osg::ref_ptr<osg::Geode> bSurfaceTester(pMWMesh me);
	osg::ref_ptr<osg::Geode> showBladeSurfaces(pMWMesh me);

	int countFaceQuadras(pFaceList);
	void computeNormal(pFaceList, bool, bool, osg::ref_ptr<osg::Vec3Array>, osg::ref_ptr<osg::Vec3Array>);
	void compute_and_set_skin(pMWMesh me);
	void compute_and_set_skin_edges(pMWMesh me, pGFace, std::list<std::pair<int,int> > *);
	void sortAndSetID(Triplet&);


	// unclassified
	//osg::ref_ptr<osg::Geometry> createUnclassifiedFaceGeom(pMesh, bool, osg::Vec4);
	//osg::ref_ptr<osg::Geometry> createUnclassifiedFacetFaceGeom(pMesh, osg::Vec4);	

};

#endif 