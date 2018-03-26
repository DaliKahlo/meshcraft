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
#include "meshscenegraph.h"
#include "parasolid_kernel.h"

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/PolygonMode>
//#include <osg/ShadeModel>

#define MW_TOL	1.e-6
#define MERGE_DUPLICATE_EDGES 1
#define TOLERANCE2	1.0e-28

// default face colour
#define FCOLOR_R	1.0f
#define FCOLOR_G	1.0f
#define FCOLOR_B	0.0f
// default edge colour
#define ECOLOR_R	0.0f
#define ECOLOR_G	0.0f
#define ECOLOR_B	0.0f

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

	// create a node for each instanced body
	std::vector< osg::ref_ptr<osg::Node> > instanced;
	int i;
	for(i=0; i<pAsmMesh->numInstancedBodies(); i++) {
		pMesh pMe = pAsmMesh->ithInstancedMesh(i);
		instanced.push_back( createSwitchAndFaceGeode(pMe,i+1).get() );
	}

	// create transform nodes for each instance 
	for(i=0; i<pAsmMesh->numInstances(); i++) {
		Instance inst = pAsmMesh->ithInstance(i);
		PK_TRANSF_t transf = inst.transform();
	
		osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
		osg::Matrix m;
		if( transf ) {
			PK_TRANSF_sf_t sf;
			PK_ERROR_code_t err = PK_TRANSF_ask(transf,&sf);
			if( err )
				TRACE("error %d from PK_TRANSF_ask!!!",err);

			m.set(sf.matrix[0][0],sf.matrix[1][0],sf.matrix[2][0],sf.matrix[3][0],
				  sf.matrix[0][1],sf.matrix[1][1],sf.matrix[2][1],sf.matrix[3][1],
				  sf.matrix[0][2],sf.matrix[1][2],sf.matrix[2][2],sf.matrix[3][2],
				  sf.matrix[0][3],sf.matrix[1][3],sf.matrix[2][3],sf.matrix[3][3]);
		} else {
			m.makeIdentity();
		}
		mt->setMatrix( m );
		asmroot->addChild( mt.get() );
		mt->addChild( instanced[inst.body_index()].get() );
	}

	return asmroot.get();
}

bool CSGFactory::addMeshEdgeGeode(AssemblyMesh *pAsmMesh, osg::Switch *pBodySwitch)
{
	// get occurrance, which is the position of the body in instanced body array
	const std::string name = pBodySwitch->getName();
	std::string id = name.substr(5);
	int occurrance =  stoi(id);
	if(occurrance < 1)
		return false;

	// get mesh and model (unclassified if model==NULL
	pMesh pMe = pAsmMesh->ithInstancedMesh(occurrance-1);
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
osg::ref_ptr<osg::Switch> CSGFactory::createSwitchAndFaceGeode(pMesh me, int id)
{
	osg::Vec4 face_color(FCOLOR_R,FCOLOR_G,FCOLOR_B,1.0f); 

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	SceneStyle m_renderMode = mwApp->sysOption()->getSceneStyle();

	osg::ref_ptr<osg::Switch> switchNode = new osg::Switch;
	switchNode->setName("Body_" + std::to_string((long long)id));

	// create and insert shaded geode into scene with face_color
	osg::ref_ptr<osg::Geode> shadedNode = new osg::Geode();
	switchNode->addChild( shadedNode.get(), true );

	// compute _pFaceMap in case of volume mesh and _pFaceMap==NULL
	// the createXXXXFaceGeom functions below will traverse both mesh faces and _pFaceMap  7/25/2012
	if(M_numRegions(me) > 0) 
		compute_and_set_skin(me);		// simply return if _pFaceMap has been set

	// create tri/quad primitives
	pGEntity bo = M_model(me);
	if(bo) {
		// classified 
		PK_FACE_t *faces = NULL;
		int i, nGF;
	
		PK_BODY_ask_faces(bo,&nGF,&faces);
		for(i=0; i<nGF; i++){
			if(m_renderMode == Facets) {
				// add classified tri/quad primitives
				osg::ref_ptr<osg::Geometry> geom1 = createFacetFaceGeom(me,faces[i],face_color);
				shadedNode->addDrawable( geom1.get() );
			}
			else if(m_renderMode == Smooth) {
				// add classified tri/quad primitives by setting last paramter true
				osg::ref_ptr<osg::Geometry> geom2 = createFaceGeom(me,faces[i],true,face_color);
				shadedNode->addDrawable( geom2.get() );
			}
		}
		if(faces) PK_MEMORY_free(faces);
	} 

	if(m_renderMode == Facets) {
		// add unclassified tri/quad primitives
		osg::ref_ptr<osg::Geometry> geom3 = createFacetFaceGeom(me,NULL,face_color);
		if(geom3)
			shadedNode->addDrawable( geom3.get() );
	}
	else if(m_renderMode == Smooth) {
		// add unclassified tri/quad primitives by setting last paramter true
		osg::ref_ptr<osg::Geometry> geom4 = createFaceGeom(me,NULL,true,face_color);
		if(geom4)
			shadedNode->addDrawable( geom4.get() );
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
osg::ref_ptr<osg::Geometry> CSGFactory::createFaceGeom(pMesh me, pGFace face, bool draw_face, osg::Vec4 color)
{
	// get surface type and orientation
	PK_CLASS_t pclass;
	bool orient = true;
	if(face) {
		PK_SURF_t surf;
		PK_LOGICAL_t o;
		PK_FACE_ask_oriented_surf(face,&surf,&o);
		PK_ERROR_code_t err = PK_ENTITY_ask_class(surf,&pclass);
		orient = o ? true : false;
	}

	int i, id, num, nTotal=0;
	Face *f;
	pFaceList flist = M_getClassifiedFaces(me,face,&num);		// face can be NULL
	if(flist) {
		pFaceIter gf_face_iter = FIter(flist);
		while( f = FIter_next(gf_face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				id = V_getID(vertex);
				if(id > 0) {
					nTotal++;
					V_setID(vertex,-id);
				}
			}
		}
	}


	// count number of vertices
	PK_EDGE_t *edges=NULL;
	PK_VERTEX_t *vertices=NULL;
	pVertexList vlist=NULL;
	int i, nv, ne, num, nTotal=0;

	if(face){
		PK_FACE_ask_vertices(face,&nv,&vertices);
		nTotal += nv;
		PK_FACE_ask_edges(face,&ne,&edges);	
		for(i=0; i<ne; i++){
			vlist = M_getClassifiedVertices(me,edges[i],&num);
			nTotal += num;
		}
	}
	vlist = M_getClassifiedVertices(me,face,&num);  // face can be NULL
	nTotal += num;
	if(nTotal < 3) 
		return NULL;

	// create the geometry object
	osg::ref_ptr<osg::Geometry> faceGeom = new osg::Geometry;

	////// create the vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(nTotal);
	faceGeom->setVertexArray( v.get() );
	
	// set vertex array
	double xyz[3];
	pVertex vertex;
	int count=0;

	if( vlist ) {		// unclassified vertices or vertices on model face
		pVertexIter gf_vtx_iter = VIter(vlist);
		while( vertex = VIter_next(gf_vtx_iter) ) {
			V_coord(vertex,xyz);
			(*v)[count].set(xyz[0],xyz[1],xyz[2]);
			V_setID(vertex,count);
			count++;
		}
		VIter_delete(gf_vtx_iter);
	}

	if(face) {
		// vertices on model vertices
		for(i=0; i<nv; i++){
			vlist = M_getClassifiedVertices(me,vertices[i],&num);
			if( vlist ) {
				pVertexIter gv_vtx_iter = VIter(vlist);
				vertex = VIter_next(gv_vtx_iter);
				VIter_delete(gv_vtx_iter);
				V_coord(vertex,xyz);
				(*v)[count].set(xyz[0],xyz[1],xyz[2]);
				V_setID(vertex,count);
				count++;
			}
		}
		
		//  vertices on model edges
		for(i=0; i<ne; i++){
			vlist = M_getClassifiedVertices(me,edges[i],&num);
			if( vlist ) {
				pVertexIter ge_vtx_iter = VIter(vlist);
				while( vertex = VIter_next(ge_vtx_iter) ){
					V_coord(vertex,xyz);
					(*v)[count].set(xyz[0],xyz[1],xyz[2]);
					V_setID(vertex,count);
					count++;
				}
				VIter_delete(ge_vtx_iter);
			}
		}
		
	}
	if( vertices ) PK_MEMORY_free(vertices);
	if(edges) PK_MEMORY_free(edges);


	////// set color
	if(face && draw_face) {
		PK_ATTRIB_t colour;
		int nColourVals;
		double* cTemp;
		PK_ENTITY_ask_first_attrib( face, colour_attdef, &colour );
		if (colour != PK_ENTITY_null) {
			PK_ATTRIB_ask_doubles( colour, 0, &nColourVals, &cTemp );
			if (nColourVals == 3) {
				for (i=0; i<3; i++) color[ i ] = cTemp[ i ];
			}
			PK_MEMORY_free( cTemp );
		}
	} 
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);

	// use the shared color array.
    faceGeom->setColorArray(shared_color.get());
    faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);


    ////// add primitives (tris, quads, segments)
	int ids[4];
	bool tFlag=false, qFlag=false;
	Face *f;
	pFaceList flist = M_getClassifiedFaces(me,face,&num);			// face can be NULL
	if(flist || _pFaceMap) {
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
						ids[i] = V_getID(vertex);
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
					trias->push_back( V_getID(tri.v0) );
					trias->push_back( V_getID(tri.v1) );
					trias->push_back( V_getID(tri.v2) );
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
	}

	return faceGeom.get();
}

// draw tri/quad primitives (facet mode) with specified color
// account for unclassified, i.e. face can be NULL
osg::ref_ptr<osg::Geometry> CSGFactory::createFacetFaceGeom(pMesh me, pGFace face, osg::Vec4 color)
{
	int num;
	pFaceList flist = M_getClassifiedFaces(me,face,&num);
	if(_pFaceMap)
		num += _pFaceMap->size();
	if(num < 3)
		return NULL;

	///// get parasolid's face orientation
	PK_LOGICAL_t orient = true;
	if(face) {
		PK_SURF_t surf;
		PK_FACE_ask_oriented_surf(face,&surf,&orient);
	}

	///// create the geometry object
	osg::ref_ptr<osg::Geometry> faceGeom = new osg::Geometry;

	////// create vertex array
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(3*num);
	faceGeom->setVertexArray( v.get() );

	////// create normal array
	osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array(3*num); 
	faceGeom->setNormalArray(norms.get());
	faceGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	////// set colors 
	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
	shared_color->push_back(color);
	faceGeom->setColorArray(shared_color.get());
	faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

	// set normals, vertices and primitives
	osg::Vec3 norVec;
	int i, nv, count = 0;
	double xyz[4][3], nor[3];

	if(flist) {
		pFace f;
		pVertex vertex;

		pFaceIter gf_face_iter = FIter(flist);
		while( f = FIter_next(gf_face_iter) ){
			nv = F_numVertices(f);
			for(i=0; i<nv; i++) {
				vertex = F_vertex(f,i);
				V_coord(vertex,xyz[i]);
			}
			if(nv == 3) {
				if( !norm(xyz[0],xyz[1],xyz[2],nor) )
					continue;

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
				for(i=0; i<2; i++) {
					if( !norm(xyz[0],xyz[1+i],xyz[2+i],nor) )
						continue;

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
			}
		}
		FIter_delete(gf_face_iter);
	}

	if(_pFaceMap) {
		std::map<Triplet,bool>::iterator fit;
		Triplet tri;
		for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
			tri = fit->first;
			V_coord(tri.v0,xyz[0]);
			V_coord(tri.v1,xyz[1]);
			V_coord(tri.v2,xyz[2]);
			if( !norm(xyz[0],xyz[1+i],xyz[2+i],nor) )
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

// compute unit normal
// return false in case of zero area. true otherwise
bool CSGFactory::norm(double a[3], double b[3], double c[3], double *n)
{
	double ll, ac[3], bc[3], nor[3];
	int i;
	for(i=0; i<3; i++) {
		ac[i] = a[i] - c[i];
		bc[i] = b[i] - c[i];
	}
	crossProd(ac,bc,nor);

	ll = nor[0]*nor[0] + nor[1]*nor[1] + nor[2]*nor[2];
	if(ll < TOLERANCE2)
		return false;

	ll = sqrt(ll);
	for(i=0; i<3; i++) 
		n[i] = nor[i]/ll;
	return true;
}

void CSGFactory::crossProd(double *v1, double *v2, double *cp)
{
  cp[0] = v1[1]*v2[2] - v1[2]*v2[1] ;
  cp[1] = v1[2]*v2[0] - v1[0]*v2[2] ;
  cp[2] = v1[0]*v2[1] - v1[1]*v2[0] ;
}

//////////////////////////////////////////////////////////////////////
///
/// unclassified tri/quad/tet meshes
///

//osg::ref_ptr<osg::Geometry> CSGFactory::createUnclassifiedFaceGeom(pMesh me, bool draw_face, osg::Vec4 color)
//{
//	int num, nTotal;
//	pVertexList vlist = M_getClassifiedVertices(me,NULL,&nTotal);
//	if(vlist == NULL || nTotal < 3) 
//		return NULL;
//
//	// create the geometry object
//	osg::ref_ptr<osg::Geometry> faceGeom = new osg::Geometry;
//
//	////// create the vertex array
//	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(nTotal);
//	faceGeom->setVertexArray( v.get() );
//	
//	// set vertex array
//	double xyz[3];
//	pVertex vertex;
//	int count=0;
//	pVertexIter vtx_iter = VIter(vlist);
//	while( vertex = VIter_next(vtx_iter) ) {
//		V_coord(vertex,xyz);
//		(*v)[count].set(xyz[0],xyz[1],xyz[2]);
//		V_setID(vertex,count);
//		count++;
//	}
//	VIter_delete(vtx_iter);
//
//	////// set colors 
//	osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
//	shared_color->push_back(color);
//
//	// use the shared color array.
//    faceGeom->setColorArray(shared_color.get());
//    faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
//
//
//    ////// add primitives (tris, quads, segments)
//	int nv, i, ids[4];
//	bool tFlag=false, qFlag=false;
//	Face *f;
//	pFaceList flist = M_getClassifiedFaces(me,NULL,&num);
//	if(flist || _pFaceMap) {
//		if(draw_face) 
//		{
//			osg::ref_ptr<osg::DrawElementsUInt> quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
//			osg::ref_ptr<osg::DrawElementsUInt> trias = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
//			if(flist) {
//				pFaceIter face_iter = FIter(flist);
//				while( f = FIter_next(face_iter) ){
//					nv = F_numVertices(f);
//					for(i=0; i<nv; i++) {
//						vertex = F_vertex(f,i);
//						ids[i] = V_getID(vertex);
//					}
//					if(nv == 4) {
//						qFlag = true;
//						quads->push_back(ids[0]);
//						quads->push_back(ids[1]);
//						quads->push_back(ids[2]);
//						quads->push_back(ids[3]);
//					}
//					else {
//						tFlag = true;
//						trias->push_back(ids[0]);
//						trias->push_back(ids[1]);
//						trias->push_back(ids[2]);
//					}
//				}
//				FIter_delete(face_iter);
//			}
//			if(_pFaceMap) {
//				std::map<Triplet,bool>::iterator fit;
//				Triplet tri;
//				for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
//					tFlag = true;
//					tri = fit->first;
//					trias->push_back( V_getID(tri.v0) );
//					trias->push_back( V_getID(tri.v1) );
//					trias->push_back( V_getID(tri.v2) );
//				}
//			}
//			if(tFlag) faceGeom->addPrimitiveSet(trias);
//			if(qFlag) faceGeom->addPrimitiveSet(quads);
//
//			////// create the normal array
//			// Note: normal size must be same as vertex size, othrewise it will crash.
//			bool per_vertex = true;
//			osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array(nTotal); 
//			faceGeom->setNormalArray(norms.get());
//			faceGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
//			computeNormal(flist,1,per_vertex,v.get(),norms.get());
//		}
//
//		else  
//		{	//  draw_lines (!draw_face)
//			std::list<std::pair<int,int> > edgelist;
//			std::list<std::pair<int,int> >::iterator eIt;
//			compute_and_set_skin_edges(me,NULL,&edgelist);
//			osg::DrawElementsUInt* lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
//			for(eIt=edgelist.begin(); eIt!=edgelist.end(); eIt++) {
//				lines->push_back(eIt->first);
//				lines->push_back(eIt->second);
//			}
//			faceGeom->addPrimitiveSet(lines);
//		}
//	}
//
//	return faceGeom.get();
//}

//osg::ref_ptr<osg::Geometry> CSGFactory::createUnclassifiedFacetFaceGeom(pMesh me, osg::Vec4 color)
//{
//	// create the geometry object
//	osg::ref_ptr<osg::Geometry> faceGeom = new osg::Geometry;
//
//	int num;
//	pFaceList flist = M_getClassifiedFaces(me,NULL,&num);
//	if(_pFaceMap)
//		num += _pFaceMap->size();
//	if(num > 0) {
//		////// create vertex array
//		osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array(3*num);
//		faceGeom->setVertexArray( v.get() );
//
//		///// create normal array
//		osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array(3*num); 
//		faceGeom->setNormalArray(norms.get());
//		faceGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
//
//		////// set colors 
//		osg::ref_ptr<osg::Vec4Array> shared_color = new osg::Vec4Array;
//		shared_color->push_back(color);
//		faceGeom->setColorArray(shared_color.get());
//		faceGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
//
//
//		// set normals, vertices and primitives
//		pFace f;
//		pVertex vertex;
//		osg::Vec3 pt0, pt1, pt2, nor;
//		double xyz[3];
//		int i, nv, count = 0;
//
//		if(flist) {
//			pFaceIter face_iter = FIter(flist);
//			while( f = FIter_next(face_iter) ){
//				nv = F_numVertices(f);
//				for(i=0; i<nv; i++) {
//					vertex = F_vertex(f,i);
//					V_coord(vertex,xyz);
//					(*v)[count].set(xyz[0],xyz[1],xyz[2]);
//					count++;
//				}
//			
//				pt0 = (*v)[count-3];
//				pt1 = (*v)[count-2];
//				pt2 = (*v)[count-1];
//				nor = (pt1 - pt0) ^ (pt2 - pt0);
//				nor.normalize();
//
//				(*norms)[count-3] += nor;
//				(*norms)[count-2] += nor;
//				(*norms)[count-1] += nor;
//				faceGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,count-3,3));
//			}
//			FIter_delete(face_iter);
//		}
//
//		if(_pFaceMap) {
//			std::map<Triplet,bool>::iterator fit;
//			Triplet tri;
//			for(fit=_pFaceMap->begin(); fit!=_pFaceMap->end(); fit++) {
//				tri = fit->first;
//				V_coord(tri.v0,xyz);
//				(*v)[count++].set(xyz[0],xyz[1],xyz[2]);
//				V_coord(tri.v1,xyz);
//				(*v)[count++].set(xyz[0],xyz[1],xyz[2]);
//				V_coord(tri.v2,xyz);
//				(*v)[count++].set(xyz[0],xyz[1],xyz[2]);
//				
//				pt0 = (*v)[count-3];
//				pt1 = (*v)[count-2];
//				pt2 = (*v)[count-1];
//				nor = (pt1 - pt0) ^ (pt2 - pt0);
//				nor.normalize();
//
//				(*norms)[count-3] += nor;
//				(*norms)[count-2] += nor;
//				(*norms)[count-1] += nor;
//				faceGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,count-3,3));
//			}
//		}
//	}
//
//	return faceGeom.get();
//}

///////////////////////////////////////////////////////////////////////
///
///  utilities
///

// if per_vertex == false, sets one normal for all face vertices
//               == true, set one normal per vertex
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
				id[i] = V_getID(vertex);
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
					(*norms)[id[1]] += nor;
					(*norms)[id[2]] += nor;
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
			id[0] = V_getID(tri.v0);
			id[1] = V_getID(tri.v1);
			id[2] = V_getID(tri.v2);
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

void CSGFactory::compute_and_set_skin(pMesh me)
{
	Triplet tri;
	pRegion rgn;
	pVertex vts[4];
	int i, nr;

	std::map<Triplet,bool>::iterator mapIter;
	int rf[] = {0,1,3,  1,2,3,  2,0,3,  2,1,0};		// normal points outside

	if(_pFaceMap) 
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
	tri.iMin = V_getID(tri.v0);
	tri.iMed = V_getID(tri.v1);
	tri.iMax = V_getID(tri.v2);

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

void CSGFactory::compute_and_set_skin_edges(pMesh me, pGFace face, std::list<std::pair<int,int> > *pEdgelist)
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
				ids[i] = V_getID(vertex);
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
			ids[0] = V_getID(tri.v0);
			ids[1] = V_getID(tri.v1);
			ids[2] = V_getID(tri.v2);
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

