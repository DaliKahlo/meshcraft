//$c12  XRL 06/04/2013 lightening model (assign specular and shineness, but still use glcolor)
//$c11  XRL 11/10/2012 set mesh face as negative and not shown if degenerated into a line or point 
//$c10  XRL 11/08/2012 fix a bug in quad normal computation in computeNormal
//$c9   XRL 07/30/2012 support quads and fix a bug in createFacetFaceGeom
//$c8   XRL 07/30/2012 added createInteriorPoints
//$c7   XRL 07/26/2012 merged classified and unclassifed. almost rewrite createFaceGeom to pass swingarm.
//$c6   XRL 07/25/2012 support regions; edge display as addition through "addMeshEdgeGeode" 
//$c5   XRL 07/10/2012 seperate view style and scene style
//$c4   XRL 07/10/2012 removed unnecessary group nodes (now 3 level: root->MatrixTransform->Geode)
//$c3   XRL 07/09/2012 Fixed memory leaks (always use osg::ref_ptr)
//$c2   XRL 02/10/2012 Show assembly mesh 
//$c1   XRL 01/08/2012 Created 
//========================================================================//
//
// CMeshSG class implementation 
//
//=========================================================================

#include "stdafx.h"
#include "MeshWorks.h"
#include "util.h"
#include "scenegraphfactory.h"
#include "parasolid_kernel.h"

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

CSGFactory::CSGFactory()
{
	// parasolid's color attrib def
	colour_attdef=0;
	// default color
	defaultfaceColor[0] = FCOLOR_R;
	defaultfaceColor[1] = FCOLOR_G;
	defaultfaceColor[2] = FCOLOR_B;
	// tetrahedral boundary faces
	_pFaceMap = NULL;
}

CSGFactory::~CSGFactory()
{ clear(); }

void CSGFactory::clear()
{
	if(_pFaceMap) 
	{delete _pFaceMap; _pFaceMap=NULL;}
}

void CSGFactory::setDefaultFaceColor(float r, float g, float b)
{
	defaultfaceColor[0] = r;
	defaultfaceColor[1] = g;
	defaultfaceColor[2] = b;
}

osg::ref_ptr<osg::Group> CSGFactory::create(AssemblyMesh *pAsmMesh)
{
	// get parasolid's color attrib def
	colour_attdef=0;
	PK_ATTDEF_find("SDL/TYSA_COLOUR", &colour_attdef);

	osg::ref_ptr<osg::Group> asmroot = new osg::Group;

	osg::StateSet * state = asmroot->getOrCreateStateSet();
	//osg::ShadeModel* sm = new osg::ShadeModel();
	//sm->setMode( osg::ShadeModel::FLAT );
	osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;
	pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	state->setAttribute( pm.get() );

	//// page 112 cook book 3.0
	//state->setAttributeAndModes(new osg::PolygonOffset(1.0f,1.0f));

	// assign specular and shineness material to model
	osg::Material *material = new osg::Material();
	//material->setDiffuse(osg::Material::FRONT,  osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));  // can we set? how about that in color binding?
	material->setSpecular(osg::Material::FRONT, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	material->setShininess(osg::Material::FRONT, 40.0f);
	material->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE );
	asmroot->getOrCreateStateSet()->setAttribute(material);

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	double pointsize = mwApp->sysOption()->getScenePointSize();
	osg::Point* point = new osg::Point;
	point->setSize(pointsize);
	state->setAttributeAndModes(point, osg::StateAttribute::ON);

	// create a node for each instanced body
	std::vector< osg::ref_ptr<osg::Node> > instanced;
	int i;
	for(i=0; i<pAsmMesh->numInstancedBodies(); i++) {
		pMWMesh pMe = pAsmMesh->ithInstancedMesh(i);
		instanced.push_back( createSwitchAndFaceGeode(pMe,i+1).get() );
	}

	// create transform nodes for each instance 
	for(i=0; i<pAsmMesh->numInstances(); i++) {
		Instance *inst = pAsmMesh->ithInstance(i);
		osg::Matrix * transf = inst->transform();
	
		osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
		mt->setMatrix( *transf );
		asmroot->addChild( mt.get() );
		mt->addChild( instanced[inst->body_index()].get() );
	}

	//asmroot->setDataVariance( osg::Object::STATIC );
	asmroot->setName("Part_" + std::to_string((long long)(pAsmMesh->getID())));
	return asmroot.get();
}

bool CSGFactory::addMeshEdgeGeode(AssemblyMesh *pAsmMesh, osg::Switch *pBodySwitch)
{
	// get occurrance, which is the position of the body in instanced body array
	const std::string name = pBodySwitch->getName();
	std::string id = name.substr(8);
	int occurrance =  stoi(id);
	if(occurrance < 1)
		return false;

	// get mesh and model (unclassified if model==NULL
	pMWMesh pMe = pAsmMesh->ithInstancedMesh(occurrance-1);
	pGEntity bo = M_model(pMe);

	// create and insert edge geode node into scene graph
	osg::ref_ptr<osg::Geode> edgeNode = new osg::Geode();
	pBodySwitch->addChild( edgeNode.get(), true );

	// set attributes
	osg::StateSet * wstate = edgeNode->getOrCreateStateSet();
	//wstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF);	
	osg::LineWidth* lw = new osg::LineWidth(1.5f); 
	wstate->setAttribute(lw, osg::StateAttribute::ON); 

	// edge color
	osg::Vec4 edge_color(ECOLOR_R,ECOLOR_G,ECOLOR_B,1.0f); 

	// create line primitives
	if(bo) {
		PK_FACE_t *faces = NULL;
		int i, nGF;

		PK_BODY_ask_faces(bo,&nGF,&faces);
		for(i=0; i<nGF; i++){
			osg::ref_ptr<osg::Geometry> classifiedGeom = createFaceGeom(pMe,faces[i],false,edge_color);
			edgeNode->addDrawable( classifiedGeom.get() );
		}
		if(faces) PK_MEMORY_free(faces);
	} 

	osg::ref_ptr<osg::Geometry> unclassifiedGeom = createFaceGeom(pMe,NULL,false,edge_color);
	if(unclassifiedGeom)
		edgeNode->addDrawable( unclassifiedGeom.get() );
	
	return true;
}

//////////////////////////////////////////////////////////////////////
///
/// classified tri/quad meshes
///


// create a switch node per body mesh,
// add face primitives always in Smooth or Facets scene style
osg::ref_ptr<osg::Switch> CSGFactory::createSwitchAndFaceGeode(pMWMesh me, int id)
{
#ifdef BLACK_WHITE_MODE
	osg::Vec4 face_color(FCOLOR_R,FCOLOR_G,FCOLOR_B,0.0f); 
	osg::Vec4 edge_color(FCOLOR_R,FCOLOR_G,FCOLOR_B,0.0f); 
#else
	osg::Vec4 face_color(defaultfaceColor[0],defaultfaceColor[1],defaultfaceColor[2],1.0f); 
	osg::Vec4 edge_color(1.0f,1.0f,0.0f,1.0f); 
	osg::Vec4 mass_color(1.0f,0.0f,0.0f,1.0f); 
#endif

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	SceneStyle m_renderMode = mwApp->sysOption()->getSceneStyle();

	osg::ref_ptr<osg::Switch> switchNode = new osg::Switch;
	switchNode->setName("SW_Body_" + std::to_string((long long)id));

	// create and insert shaded geode into scene with face_color
	osg::ref_ptr<osg::Geode> facesNode = new osg::Geode();
	facesNode->setName("Body_" + std::to_string((long long)id));
	switchNode->addChild( facesNode.get(), true );

	// compute _pFaceMap in case of volume mesh and _pFaceMap==NULL
	// the createXXXXFaceGeom functions below will traverse both mesh faces and _pFaceMap  7/25/2012
	if(M_numberRegions(me) > 0) {
		// simply return if _pFaceMap has been set 
		// or the face mesh is already saved in mesh database
		compute_and_set_skin(me);		
		// create point primitives to show interior nodes
		osg::ref_ptr<osg::Geometry> ptCloud = createInteriorPoints(me,face_color);
		if(ptCloud)
			facesNode->addDrawable( ptCloud.get() );
	}

	// create tri/quad primitives
	pGEntity bo = M_model(me);
	if(bo) {
		// classified 
		PK_FACE_t *faces = NULL;
		int i, nGF, fid;
	
		PK_BODY_ask_faces(bo,&nGF,&faces);
		for(i=0; i<nGF; i++){
			PK_ENTITY_ask_identifier(faces[i],&fid);
			if(m_renderMode == Facets) {
				// add classified tri/quad primitives
				osg::ref_ptr<osg::Geometry> fcgeom1 = createFacetFaceGeom(me,faces[i],face_color);
				if(fcgeom1) {
					fcgeom1->setName("Face_" + std::to_string((long long)fid));
					facesNode->addDrawable( fcgeom1.get() );
				}
			}
			else if(m_renderMode == Smooth) {
				// add classified tri/quad primitives by setting last paramter true
				osg::ref_ptr<osg::Geometry> fcgeom2 = createFaceGeom(me,faces[i],true,face_color);
				if(fcgeom2) {
					fcgeom2->setName("Face_" + std::to_string((long long)fid));
					facesNode->addDrawable( fcgeom2.get() );
				}
			}
		}
		if(faces) PK_MEMORY_free(faces);
	} 

	if(m_renderMode == Facets) {
		// add unclassified tri/quad primitives
		osg::ref_ptr<osg::Geometry> geom3 = createFacetFaceGeom(me,NULL,face_color);
		if(geom3) {
			geom3->setName("Face_null");
			facesNode->addDrawable( geom3.get() );
		}
	}
	else if(m_renderMode == Smooth) {
		// add unclassified tri/quad primitives by setting last paramter true
		osg::ref_ptr<osg::Geometry> geom4 = createFaceGeom(me,NULL,true,face_color);
		if(geom4) {
			geom4->setName("Face_null");
			facesNode->addDrawable( geom4.get() );
		}
	}

	// create edge primitives
	if(M_numberEdges(me) > 0) {
		osg::ref_ptr<osg::Geometry> edgeGeom = createBoundaryEdges(me,edge_color);
		if(edgeGeom) {
			edgeGeom->setName("BEdges");
			facesNode->addDrawable( edgeGeom.get() );
		}
	}

	// create mass primitives
	osg::ref_ptr<osg::Geometry> massGeom = createModelVertices(me,mass_color);
	if(massGeom) {
		massGeom->setName("MoVertices");
		facesNode->addDrawable( massGeom.get() );
	}

	// create cloud points 
	if(M_numberEdges(me) == 0 && M_numberFaces(me) == 0 && 
	   M_numberRegions(me) == 0 && M_numberVertices(me) > 0) {
		osg::ref_ptr<osg::Geometry> ptCloud = createInteriorPoints2(me,0,face_color);
		if(ptCloud) {
			ptCloud->setName("PtCloud");
			facesNode->addDrawable( ptCloud.get() );
		}
	}

	if(GM_numberCurves(me) > 0) {
		//osg::ref_ptr<osg::Group> Pcurves = createPolyLines(me,face_color);
		//osg::ref_ptr<osg::Group> Bcurves = createBladeCurves(me,face_color);
		//if(Pcurves) {
		//	Pcurves->setName("Curves");
		//	switchNode->addChild( Pcurves.get(), true );
		//}
		//if(Bcurves) {
		//	Bcurves->setName("TopViewCurves");
		//	switchNode->addChild( Bcurves.get(), true );
		//}
		//osg::ref_ptr<osg::Geode> bspline = bsplineCurves(me,face_color);
		//if(bspline) {
		//	bspline->setName("ONurbs");
		//	switchNode->addChild( bspline.get(), true );
		//}
		//osg::ref_ptr<osg::Geode> Bsurface = bSurfaceTester(me);
		osg::ref_ptr<osg::Geode> Bsurface = showBladeSurfaces(me);
		if(Bsurface) {
			Bsurface->setName("Bsurface");
			switchNode->addChild( Bsurface.get(), true );
		}
	}
	return switchNode.get();  
}


// * line primitives and face primitives can not be added together since
//   each vertex array is only associated with one color array. Otherwise 
//   lines can not be seen.
// * if draw_face==true	--> draw faces
//				   false  --> draw lines
// * to draw both edge and face, call this function twice with different color.
// * if face==NULL, process all unclassified mesh entities
// * it is assumed that  the original mesh vertex id >= 1 and not duplicated
osg::ref_ptr<osg::Geometry> CSGFactory::createFaceGeom(pMWMesh me, pGFace face, bool draw_face, osg::Vec4 color)
{
	//CString cstr;
	//cstr.Format(_T("c:\\xli\\mwdebug%d.dat"), face);
	//FILE *pf = fopen(cstr, "w");

	// get surface type and orientation
	PK_CLASS_t pclass = 0;
	bool orient = true;
	if(face && !M_faceOrient(me)) {
		PK_SURF_t surf;
		PK_LOGICAL_t o;
		PK_FACE_ask_oriented_surf(face,&surf,&o);
		PK_ERROR_code_t err = PK_ENTITY_ask_class(surf,&pclass);
		orient = o ? true : false;
	}

	// count number of vertices
	//-----------------------------------------------------------------
	// we can't directly traverse the vertices of the face because model topology could be broken
	// when eliminating short model edges and sliver model faces, e.g. the swingarm model 
	// so the part of code below is re-written on 7/26/2012  
	//-----------------------------------------------------------------
	int i, id, nv, num, nTotal=0;
	Face *f;
	pVertex vertex;
	pFaceList flist = M_getClassifiedFaces(me,face,&num);		// face can be NULL
	if(flist) {
		pFaceIter face_iter = FIter(flist);
		while( f = FIter_next(face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				id = V_getTempID(vertex);					
				if(id != RESERVED_VERTEX_TEMP_ID_1) {
					nTotal++;
					V_setTempID(vertex,RESERVED_VERTEX_TEMP_ID_1);
				}
			}
		}
		FIter_delete(face_iter);
	}
	if(_pFaceMap) {
		std::map<Triplet,bool>::iterator fit;
		Triplet tri;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			id = V_getTempID(tri.v0); if(id != RESERVED_VERTEX_TEMP_ID_1) { nTotal++; V_setTempID(tri.v0,RESERVED_VERTEX_TEMP_ID_1); }
			id = V_getTempID(tri.v1); if(id != RESERVED_VERTEX_TEMP_ID_1) { nTotal++; V_setTempID(tri.v1,RESERVED_VERTEX_TEMP_ID_1); }
			id = V_getTempID(tri.v2); if(id != RESERVED_VERTEX_TEMP_ID_1) { nTotal++; V_setTempID(tri.v2,RESERVED_VERTEX_TEMP_ID_1); }
		}
	}
	if(nTotal < 3) 
		return NULL;

	// create the geometry object
	osg::ref_ptr<osg::Geometry> faceGeom = new osg::Geometry;

	////// create the vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(nTotal);
	faceGeom->setVertexArray( v.get() );
	
	// set vertex array
	double xyz[3];
	int count=0;  

	if(flist) {
		pFaceIter face_iter = FIter(flist);
		while( f = FIter_next(face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				id = V_getTempID(vertex);
				if(id == RESERVED_VERTEX_TEMP_ID_1) {
					V_getCoord(vertex,xyz);
					(*v)[count].set(xyz[0],xyz[1],xyz[2]);
					V_setTempID(vertex,count+1);
					count++;
				}
			}
		}
		FIter_delete(face_iter);
	}
	if(_pFaceMap) {
		std::map<Triplet,bool>::iterator fit;
		Triplet tri;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			if(V_getTempID(tri.v0) == RESERVED_VERTEX_TEMP_ID_1) {
				V_getCoord(tri.v0,xyz);
				(*v)[count].set(xyz[0],xyz[1],xyz[2]);
				V_setTempID(tri.v0,count+1);
				count++;
			}
			if(V_getTempID(tri.v1) == RESERVED_VERTEX_TEMP_ID_1) {
				V_getCoord(tri.v1,xyz);
				(*v)[count].set(xyz[0],xyz[1],xyz[2]);
				V_setTempID(tri.v1,count+1);
				count++;
			}
			if(V_getTempID(tri.v2) == RESERVED_VERTEX_TEMP_ID_1) {
				V_getCoord(tri.v2,xyz);
				(*v)[count].set(xyz[0],xyz[1],xyz[2]);
				V_setTempID(tri.v2,count+1);
				count++;
			}
		}
	}
	//-----------------------------------------------------------------

	////// retreve color from parasolid
	if(face && draw_face) {
		PK_ATTRIB_t colour;
		int nColourVals;
		double* cTemp;
		PK_ENTITY_ask_first_attrib( face, colour_attdef, &colour );
		if (colour != PK_ENTITY_null) {
			PK_ERROR_code_t err = PK_ATTRIB_ask_doubles( colour, 0, &nColourVals, &cTemp );
			if (err == PK_ERROR_no_errors && nColourVals == 3) {
				if( cTemp[0]>0.0 || cTemp[1]>0.0 || cTemp[2]>0.0 ) {	// black color is not accepted since edge is in back
					for (i=0; i<3; i++) color[ i ] = cTemp[ i ];
				}
			}
			PK_MEMORY_free( cTemp );
		}
	} 
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);

	////// set color (use the shared color array).
    faceGeom->setColorArray(shared_color.get());
    faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);


    ////// add primitives (tris, quads, lines)
	int ids[4];
	bool tFlag=false, qFlag=false;
	if(draw_face) 
		{
			osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
			osg::ref_ptr<osg::DrawElementsUInt> trias = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
			if(flist) {
				pFaceIter gf_face_iter = FIter(flist);
				while( f = FIter_next(gf_face_iter) ){
					nv = F_numVertices(f);
					for(i=0; i<nv; i++) {
						vertex = F_vertex(f,i);
						ids[i] = V_getTempID(vertex) - 1;
					}
					if(nv == 4) {
						qFlag = true;
						quads->push_back(ids[0]);
						quads->push_back(ids[1]);
						quads->push_back(ids[2]);
						quads->push_back(ids[3]);
					}
					else {
						tFlag = true;
						trias->push_back(ids[0]);
						trias->push_back(ids[1]);
						trias->push_back(ids[2]);
					}
				}
				FIter_delete(gf_face_iter);
			}

			if(_pFaceMap) {
				std::map<Triplet,bool>::iterator fit;
				Triplet tri;
				for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
					tFlag = true;
					tri = fit->first;
					trias->push_back( V_getTempID(tri.v0) );
					trias->push_back( V_getTempID(tri.v1) );
					trias->push_back( V_getTempID(tri.v2) );
				}
			}

			if(tFlag) faceGeom->addPrimitiveSet(trias);
			if(qFlag) faceGeom->addPrimitiveSet(quads);

			////// create the normal array
			osg::ref_ptr<osg::Vec3Array> norms;
			bool per_vertex = true;
			if(pclass == PK_CLASS_plane) 
			{
				per_vertex = false;
				norms = new osg::Vec3Array(1);
				faceGeom->setNormalArray(norms.get());
				faceGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
			}
			else 
			{
				/// set normal. Note: normal size must be same as vertex size, othrewise it will crash.
				norms = new osg::Vec3Array(nTotal); 
				faceGeom->setNormalArray(norms.get());
				faceGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
			}
			computeNormal(flist,orient,per_vertex,v.get(),norms.get());
		}  // end of draw_face

		else  
		{	//  draw_lines (!draw_face)
			std::list<std::pair<int,int> > edgelist;
			std::list<std::pair<int,int> >::iterator eIt;
			compute_and_set_skin_edges(me,face,&edgelist);
			osg::DrawElementsUInt* lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
			for(eIt=edgelist.begin(); eIt!=edgelist.end(); eIt++) {
				lines->push_back(eIt->first);
				lines->push_back(eIt->second);
			}
			faceGeom->addPrimitiveSet(lines);
		}

	return faceGeom.get();
}

// draw tri/quad primitives (facet mode) with specified color
// account for unclassified, i.e. face can be NULL
osg::ref_ptr<osg::Geometry> CSGFactory::createFacetFaceGeom(pMWMesh me, pGFace face, osg::Vec4 color)
{
	int num, nTri, nQua, count;
	bool want_to_draw_two_triangles = false;
	pFaceList flist = M_getClassifiedFaces(me,face,&num);
	if(_pFaceMap)
		num += _pFaceMap->size();
	if(num < 1)
		return NULL;

	///// count the number of quads and tris on this face
	nQua = M_numberQuadras(me);
	if(nQua == 0 || !face || !flist) 
		nTri = num - nQua;		// tri only  or  not classfied
	else {
		nQua = countFaceQuadras(flist);
		nTri = num - nQua;
	}

	if(want_to_draw_two_triangles)
		count = 3*nTri + 6*nQua;
	else
		count = 3*nTri + 4*nQua;

	///// get parasolid's face orientation
	PK_LOGICAL_t orient = true;
	if(face && !M_faceOrient(me)) {
		PK_SURF_t surf;
		PK_FACE_ask_oriented_surf(face,&surf,&orient);
	}

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> faceGeom = new osg::Geometry;

	////// create vertex and normal array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(count);
	faceGeom->setVertexArray( v.get() );

	////// create normal array
	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array(count); 
	faceGeom->setNormalArray(norms.get());
	faceGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	////// set colors 
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);
	faceGeom->setColorArray(shared_color.get());
	faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

	// set normals, vertices and primitives
	osg::Vec3 norVec;
	int i, nv, elemID;
	double xyz[4][3], nor[3];

	count = 0;
	if(flist) {
		pFace f;
		pVertex vertex;

		pFaceIter gf_face_iter = FIter(flist);
		while( f = FIter_next(gf_face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				V_getCoord(vertex,xyz[i]);
			}
			if(nv == 3) {
				if( !norm(xyz[0],xyz[1],xyz[2],nor) ) {
					elemID = EN_getID((pEntity)f);
					if(elemID>0)
						EN_resetID((pEntity)f,-elemID);			// if negative, 
					continue;
				}

				(*v)[count].set(xyz[0][0],xyz[0][1],xyz[0][2]); count++;
				(*v)[count].set(xyz[1][0],xyz[1][1],xyz[1][2]); count++;
				(*v)[count].set(xyz[2][0],xyz[2][1],xyz[2][2]); count++;
	
				norVec.set(nor[0],nor[1],nor[2]);
				if(!orient) 
					norVec *= -1;

				(*norms)[count-3] = norVec;
				(*norms)[count-2] = norVec;
				(*norms)[count-1] = norVec;
				faceGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,count-3,3));
			}
			else if(nv == 4) {
				if(want_to_draw_two_triangles) {
					for(i=0; i<2; i++) {
						if( !norm(xyz[0],xyz[1+i],xyz[2+i],nor) ) {
							elemID = EN_getID((pEntity)f);
							if(elemID>0)
								EN_resetID((pEntity)f,-elemID);	
							continue;
						}

						(*v)[count].set(xyz[0][0],xyz[0][1],xyz[0][2]); count++;
						(*v)[count].set(xyz[1+i][0],xyz[1+i][1],xyz[1+i][2]); count++;
						(*v)[count].set(xyz[2+i][0],xyz[2+i][1],xyz[2+i][2]); count++;
			
						norVec.set(nor[0],nor[1],nor[2]);
						if(!orient) 
							norVec *= -1;

						(*norms)[count-3] = norVec;
						(*norms)[count-2] = norVec;
						(*norms)[count-1] = norVec;
						faceGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,count-3,3));
					}
				} else {
					bool zero_normal[4];
					double normals[4][3];
					int iii = -1;
					for(i=0;i<4;i++) {
						if( !norm(xyz[i],xyz[(i+1)%4],xyz[(i+2)%4],normals[(i+1)%4]) ) 
							zero_normal[(i+1)%4] = false;
						else {
							zero_normal[(i+1)%4] = true;
							iii = (i+1)%4;
						}
					}
					if(iii != -1)
					{
						(*v)[count].set(xyz[0][0],xyz[0][1],xyz[0][2]); count++;
						(*v)[count].set(xyz[1][0],xyz[1][1],xyz[1][2]); count++;
						(*v)[count].set(xyz[2][0],xyz[2][1],xyz[2][2]); count++;
						(*v)[count].set(xyz[3][0],xyz[3][1],xyz[3][2]); count++;
						for(i=0;i<4;i++) {
							if(zero_normal[i])
								norVec.set(normals[i][0],normals[i][1],normals[i][2]);
							else
								norVec.set(normals[iii][0],normals[iii][1],normals[iii][2]);
							(*norms)[count-4+i] = norVec;
						}
						faceGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,count-4,4));
					}
				}
			}
		}
		FIter_delete(gf_face_iter);
	}

	if(_pFaceMap) {
		std::map<Triplet,bool>::iterator fit;
		Triplet tri;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			V_getCoord(tri.v0,xyz[0]);
			V_getCoord(tri.v1,xyz[1]);
			V_getCoord(tri.v2,xyz[2]);
			if( !norm(xyz[0],xyz[1],xyz[2],nor) )
				continue;

			(*v)[count++].set(xyz[0][0],xyz[0][1],xyz[0][2]);
			(*v)[count++].set(xyz[1][0],xyz[1][1],xyz[1][2]);
			(*v)[count++].set(xyz[2][0],xyz[2][1],xyz[2][2]);
				
			norVec.set(nor[0],nor[1],nor[2]);
			(*norms)[count-3] = norVec;
			(*norms)[count-2] = norVec;
			(*norms)[count-1] = norVec;
			faceGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,count-3,3));
		}
	}

	return faceGeom.get();
}


osg::ref_ptr<osg::Geometry> CSGFactory::createInteriorPoints(pMWMesh me, osg::Vec4 color)
{
	if(_pFaceMap) {
		std::map<Triplet,bool>::iterator fit;
		Triplet tri;
		int nfv = 0;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			if( V_getClassificationType(tri.v0) == 3 ) 
			{ V_resetClassificationType(tri.v0,2); nfv++; }
			if( V_getClassificationType(tri.v1) == 3 ) 
			{ V_resetClassificationType(tri.v1,2); nfv++; }
			if( V_getClassificationType(tri.v2) == 3 ) 
			{ V_resetClassificationType(tri.v2,2); nfv++; }
		}

		int nv = M_numberVertices(me);
		if(nv > nfv) {
			return createInteriorPoints2(me,nv-nfv,color).get();
		}
	} else if(M_numberRegions(me) > 0) {
		// show interior vertices of a classified mesh
		return createInteriorPoints2(me,0,color).get();
	}
	return NULL;
}

osg::ref_ptr<osg::Geometry> CSGFactory::createInteriorPoints2(pMWMesh me, int nv, osg::Vec4 color)
{
	int num;
	pVertexList vlist = M_getClassifiedVertices(me,NULL,&num);
	if( !vlist || num < 1)
		return NULL;

	if(nv == 0) 
		nv = num;

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> ptGeom = new osg::Geometry;

	////// create vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(nv);
	ptGeom->setVertexArray( v.get() );

	///// fill vertex array
	double xyz[3];
	int count = 0;
	pVertexIter vIt = VIter(vlist);
	while(pVertex vertex = VIter_next(vIt)) {
		if(V_getClassificationType(vertex) != 3)
			continue;
		V_getCoord(vertex,xyz);
		(*v)[count].set(xyz[0],xyz[1],xyz[2]);
		count++;
	}
	VIter_delete(vIt);

	// set the color of the geometry, one single for the whole geometry.
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);
	ptGeom->setColorArray(shared_color.get());
	ptGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

	// create and add a DrawArray point primitives
	ptGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,v->size()));
	ptGeom->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	return ptGeom.get();
}

osg::ref_ptr<osg::Geometry> CSGFactory::createBoundaryEdges(pMWMesh me, osg::Vec4 color)
{
	int ne;
	pEdgeList elist = M_getClassifiedEdges(me,NULL,&ne);
	if( !elist || ne < 1)
		return NULL;

	// count the number of vertices
	pEdge edge;
	pVertex vertex;
	int i, nv = 0;
	pEdgeIter eIt2 = EIter(elist);
	while(edge = EIter_next(eIt2)) {
		for(i=0; i<2; i++) {
			vertex = E_vertex(edge,i);
			if(V_getTempID(vertex) != RESERVED_VERTEX_TEMP_ID_2) {
				nv++;
				V_setTempID(vertex,RESERVED_VERTEX_TEMP_ID_2);
			}
		}
	}
	EIter_delete(eIt2);

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> edGeom = new osg::Geometry;

	////// create vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(nv);
	edGeom->setVertexArray( v.get() );

	///// fill vertex array
	double xyz[3];
	int count = 0;
	pEdgeIter eIt = EIter(elist);
	while(edge = EIter_next(eIt)) {
		for(i=0; i<2; i++) {
			vertex = E_vertex(edge,i);
			if(V_getTempID(vertex) == RESERVED_VERTEX_TEMP_ID_2) {
				V_getCoord(vertex,xyz);
				(*v)[count].set(xyz[0],xyz[1],xyz[2]);
				count++;
				V_setTempID(vertex,count);
			}
		}
	}
	EIter_delete(eIt);

	// set the color of the geometry, one single for the whole geometry.
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);
	edGeom->setColorArray(shared_color.get());
	edGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

	// create and add a DrawArray line primitives
	osg::DrawElementsUInt* lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
	pEdgeIter eIt3 = EIter(elist);
	while(edge = EIter_next(eIt3)) {
		for(i=0; i<2; i++) {
			vertex = E_vertex(edge,i);
			lines->push_back(V_getTempID(vertex) - 1);
		}
	}
	EIter_delete(eIt3);
	edGeom->addPrimitiveSet(lines);

	edGeom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(5.0f));
	edGeom->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	return edGeom.get();
}

osg::ref_ptr<osg::Geometry> CSGFactory::createModelVertices(pMWMesh me, osg::Vec4 color)
{
	int nv;
	pVertexList vlist = M_getClassifiedVertices(me,NULL,&nv);
	if( !vlist || nv < 1)
		return NULL;

	// count the number of vertices
	pVertex vertex;
	pVertexIter vIt2 = VIter(vlist);
	nv = 0;
	while(vertex = VIter_next(vIt2)) {
		if( V_getClassificationType(vertex) == 0 )
			nv++;
	}
	VIter_delete(vIt2);
	if(nv < 1)
		return NULL;

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> ptGeom = new osg::Geometry;

	////// create vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(nv);
	ptGeom->setVertexArray( v.get() );

	///// fill vertex array
	double xyz[3];
	int count = 0;
	pVertexIter vIt = VIter(vlist);
	while(pVertex vertex = VIter_next(vIt)) {
		if(V_getClassificationType(vertex) != 0)
			continue;
		V_getCoord(vertex,xyz);
		(*v)[count].set(xyz[0],xyz[1],xyz[2]);
		count++;
	}
	VIter_delete(vIt);

	// set the color of the geometry, one single for the whole geometry.
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);
	ptGeom->setColorArray(shared_color.get());
	ptGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

	// create and add a DrawArray point primitives
	ptGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,v->size()));
	ptGeom->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	return ptGeom.get();

}

// if per_vertex == false, sets one normal for all face vertices
//               == true, set one normal per vertex
// assumed vertex tempID has been set properly
void CSGFactory::computeNormal(pFaceList flist, bool orient, bool per_vertex, osg::ref_ptr<osg::Vec3Array> v, osg::ref_ptr<osg::Vec3Array> norms)
{
	pVertex vertex;
	pFace f;
	osg::Vec3 pt0, pt1, pt2, nor;
	int i, nv, id[4];

	if(flist) {
		pFaceIter gf_face_iter = FIter(flist);
		while( f = FIter_next(gf_face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				id[i] = V_getTempID(vertex) - 1;
			}
			if(nv == 3) {
				pt0 = (*v)[id[0]];
				pt1 = (*v)[id[1]];
				pt2 = (*v)[id[2]];
				nor = (pt1 - pt0) ^ (pt2 - pt0);

				if(nor.length2() < TOLERANCE2)
					continue;
				if(!per_vertex) {
					(*norms)[0] += nor;
					break;
				}
				(*norms)[id[0]] += nor;
				(*norms)[id[1]] += nor;
				(*norms)[id[2]] += nor;
			}
			else if(nv == 4) {
				for(i=0; i<2; i++) {
					pt0 = (*v)[id[0]];
					pt1 = (*v)[id[1+i]];
					pt2 = (*v)[id[2+i]];
					nor = (pt1 - pt0) ^ (pt2 - pt0);

					if(nor.length2() < TOLERANCE2)
						continue;
					if(!per_vertex) {
						(*norms)[0] += nor;
						break;
					}
					(*norms)[id[0]] += nor;
					(*norms)[id[1+i]] += nor;
					(*norms)[id[2+i]] += nor;
				}
				if(!per_vertex)
					break;
			}
		}
		FIter_delete(gf_face_iter);
	}

	if(_pFaceMap) {
		std::map<Triplet,bool>::iterator fit;
		Triplet tri;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			id[0] = V_getTempID(tri.v0) - 1;
			id[1] = V_getTempID(tri.v1) - 1;
			id[2] = V_getTempID(tri.v2) - 1;
			pt0 = (*v)[id[0]];
			pt1 = (*v)[id[1]];
			pt2 = (*v)[id[2]];
			nor = (pt1 - pt0) ^ (pt2 - pt0);

			if(nor.length2() < TOLERANCE2)
				continue;
			if(!per_vertex) {
				(*norms)[0] += nor;
				break;
			}
			(*norms)[id[0]] += nor;
			(*norms)[id[1]] += nor;
			(*norms)[id[2]] += nor;
		}
	}

	int nTotal = (norms.get())->size();
	for(i=0; i<nTotal; i++ ) {
		nor = (*norms)[i];
		nor.normalize();
		if(!orient) 
			nor *= -1;
		(*norms)[i].set( nor.x(),nor.y(),nor.z() );
	}
}

void CSGFactory::compute_and_set_skin(pMWMesh me)
{
	Triplet tri;
	pRegion rgn;
	pVertex vts[4];
	int i, nr;

	std::map<Triplet,bool>::iterator mapIter;
	int rf[] = {0,1,3,  1,2,3,  2,0,3,  2,1,0};		// normal points outside

	if(_pFaceMap) 
		return;
	if(M_hasSkin(me))
		return;
	_pFaceMap = new std::map<Triplet,bool>;

	pRegionList rList = M_getClassifiedRegions(me,NULL,&nr);
	pRegionIter rIt = RIter(rList);
	while( rgn = RIter_next(rIt) ) {
		for(i=0; i<4;i++)
			vts[i] = R_vertex(rgn,i);

		// sort4int(vts);
		for(i=0; i<4;i++) {
			tri.v0 = vts[rf[3*i  ]];
			tri.v1 = vts[rf[3*i+1]];
			tri.v2 = vts[rf[3*i+2]];
			sortAndSetID(tri);
			mapIter = _pFaceMap->find(tri);
			if( mapIter == _pFaceMap->end() ) {
				_pFaceMap->insert(std::map<Triplet,bool>::value_type(tri,true));
			} else {
				_pFaceMap->erase(mapIter);
			}
		}
	}
	RIter_delete(rIt);
}

void CSGFactory::sortAndSetID(Triplet &tri)
{
	tri.iMin = EN_getID((pEntity)(tri.v0));
	tri.iMed = EN_getID((pEntity)(tri.v1));
	tri.iMax = EN_getID((pEntity)(tri.v2));

	int tmp;
	if(tri.iMin > tri.iMed) {tmp=tri.iMin; tri.iMin=tri.iMed; tri.iMed=tmp; }
	if(tri.iMed > tri.iMax) {tmp=tri.iMed; tri.iMed=tri.iMax; tri.iMax=tmp; }
	if(tri.iMin > tri.iMed) {tmp=tri.iMin; tri.iMin=tri.iMed; tri.iMed=tmp; }
}

// smallest ID at vts[0]
//void CMeshSG::sort4int(pVertex vts[4])
//{
//	pVertex vTmp;
//	int i,j;
//	for(j=3; j>0; j--) {
//		for(i=0; i<j; i++) {
//			if( V_getID(vts[i]) > V_getID(vts[i+1]) ) {
//				vTmp = vts[i];
//				vts[i] = vts[i+1];
//				vts[i+1] = vTmp;
//			}
//		}
//	}
//}

void CSGFactory::compute_and_set_skin_edges(pMWMesh me, pGFace face, std::list<std::pair<int,int> > *pEdgelist)
{
	pFace f;
	pVertex vertex;
	int ids[4];
	int num, nv, i, j;
	pFaceList flist = M_getClassifiedFaces(me,face,&num);
	if(flist) {
		pFaceIter face_iter = FIter(flist);
		while( f = FIter_next(face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				ids[i] = EN_getID((pEntity)vertex) - 1;
			}
			for(i=0; i<nv; i++) {
				j = (i+1) % nv;
				if(ids[i] < ids[j])
					pEdgelist->push_back(std::make_pair(ids[i],ids[j]));
				else
					pEdgelist->push_back(std::make_pair(ids[j],ids[i]));
			}
		}
		FIter_delete(face_iter);
	}
	if(_pFaceMap) {
		Triplet tri;
		std::map<Triplet,bool>::iterator fit;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			ids[0] = EN_getID((pEntity)tri.v0) - 1;
			ids[1] = EN_getID((pEntity)tri.v1) - 1;
			ids[2] = EN_getID((pEntity)tri.v2) - 1;
			for(i=0; i<3; i++) {
				j = (i+1) % 3;
				if(ids[i] < ids[j])
					pEdgelist->push_back(std::make_pair(ids[i],ids[j]));
				else
					pEdgelist->push_back(std::make_pair(ids[j],ids[i]));
			}
		}
	}
	
	pEdgelist->sort();
	pEdgelist->unique();
}

int CSGFactory::countFaceQuadras(pFaceList flist)
{
	int nq = 0;
	if(flist) {
		pFace f;
		int nv;
		pFaceIter gf_face_iter = FIter(flist);
		while( f = FIter_next(gf_face_iter) ){
			nv = F_numVertices(f);
			if(nv == 4)
				nq++;
		}
		FIter_delete(gf_face_iter);
	}
	return nq;
}