//$c3   XRL 01/13/2012 Added mgc_edge_ask_type needed by mesh matching.
//$c2   XRL 11/11/2011 Added interface to external mesh database. Changed Geometry_interface to downward_interface.
//$c1   XRL 07/19/2011 Created
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#include "mgc_const.h"
#include "mgc_api.h"
#include "MGCParaInterface.h"
#include "parasolid_kernel.h"
#include "mesh\mesh_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
 *	parasolid interface
 */

int mgc_entity_ask_class(pGEntity_t tag, MGCGeomType *type)
{
	PK_CLASS_t pclass;
	PK_ERROR_code_t err = PK_ENTITY_ask_class((PK_ENTITY_t)tag,&pclass);
	switch( pclass ) {
		case PK_CLASS_body:		{ *type=MGC_GTYPE_BODY; break; }
		case PK_CLASS_face:		{ *type=MGC_GTYPE_FACE; break; }
		case PK_CLASS_loop:		{ *type=MGC_GTYPE_LOOP; break; }
		case PK_CLASS_fin:		{ *type=MGC_GTYPE_FIN; break; }
		case PK_CLASS_plane:	{ *type=MGC_GTYPE_PLANAR; break; }
		case PK_CLASS_edge:		{ *type=MGC_GTYPE_EDGE; break; }
		case PK_CLASS_vertex:	{ *type=MGC_GTYPE_VERTEX; break; }
		default: 
			*type=MGC_GTYPE_UNKNOWN;
	}
	return err;
}

int mgc_entity_ask_identifier(pGEntity_t entity, int *p_id)
{
	return PK_ENTITY_ask_identifier(entity,p_id);
}

int mgc_topol_eval_mass_props(pGEntity_t tag, double *amount, double *periph)
{
	double mass[1], cog[3], moi[9];
	int topols[1];

	PK_TOPOL_eval_mass_props_o_t opt;
	PK_TOPOL_eval_mass_props_o_m(opt);
	opt.mass		= PK_mass_mass_c;
	opt.periphery	= PK_mass_periphery_yes_c;
	opt.bound		= PK_mass_bound_no_c;

	topols[0] = tag;
	return PK_TOPOL_eval_mass_props(1, topols, 0.95, &opt, amount, mass, cog, moi, periph);
}

int mgc_topol_range_vector(pGEntity_t tag, double xyz[3],
						   int *results, double *distance, double uv[2])
{
	PK_range_result_t result;
	PK_range_1_r_t range;
	PK_TOPOL_range_vector_o_t options;
	PK_TOPOL_range_vector_o_m(options);
	PK_VECTOR_t pos;

	pos.coord[0] = xyz[0];
	pos.coord[1] = xyz[1];
	pos.coord[2] = xyz[2];
	PK_ERROR_code_t err = PK_TOPOL_range_vector(tag,pos,&options,&result,&range);

	if( err==PK_ERROR_no_errors ) {
		*distance = range.distance;
		uv[0] = range.end.parameters[0];
		uv[1] = range.end.parameters[1];
		if (result != PK_range_result_found_c) 
			*results = MGC_STATUS_ERROR;
		else
			*results = MGC_STATUS_OK;
	}
	return err;
}

MGCGeomType mgc_body_ask_type(pGBody_t tag)
{
	PK_BODY_type_t body_type;
	PK_BODY_ask_type(tag,&body_type);
	switch( body_type ) {
		case PK_BODY_type_solid_c:		return MGC_GTYPE_SOLID_BODY;
		case PK_BODY_type_general_c:	return MGC_GTYPE_GENERAL_BODY;
		case PK_BODY_type_sheet_c:		return MGC_GTYPE_SHELL_BODY;
	}
	return MGC_GTYPE_UNKNOWN;
}

int mgc_body_ask_faces(pGBody_t body_tag_in, int *nFaces, pGFace_t **faces)
{
	return PK_BODY_ask_faces(body_tag_in,nFaces,faces);
}

int mgc_body_ask_fins(pGBody_t body_tag_in, int *nFin, pGFin_t **fins)
{
	return PK_BODY_ask_fins(body_tag_in,nFin,fins);
}

int mgc_face_ask_loops(pGFace_t face_tag_in, int *nLoops, pGLoop_t **loops)
{
	return PK_FACE_ask_loops(face_tag_in,nLoops,loops);
}

int mgc_face_ask_edges(pGFace_t face_tag_in, int *nEdges, pGEdge_t **edges)
{
	return PK_FACE_ask_edges(face_tag_in, nEdges, edges);
}

int mgc_face_ask_surf(pGFace_t face_tag_in, pGSurf_t *surf_tag_out)
{
	return PK_FACE_ask_surf(face_tag_in,surf_tag_out);
}

int mgc_face_ask_oriented_surf(pGFace_t face_tag_in, pGSurf_t *sf, bool *sense)
{
	PK_LOGICAL_t facesurfsense;
	PK_ERROR_code_t err = PK_FACE_ask_oriented_surf(face_tag_in, sf, &facesurfsense);
	*sense = (facesurfsense) ? true : false;
	return err;
}

int mgc_face_find_uvbox(pGFace_t face_tag_in, double uvbox[4])
{
	PK_UVBOX_t box;
	PK_ERROR_code_t err = PK_FACE_find_uvbox(face_tag_in,&box);
	uvbox[0] = box.param[0];
	uvbox[1] = box.param[1];
	uvbox[2] = box.param[2];
	uvbox[3] = box.param[3];
	return err;
}

int mgc_face_contains_vectors(pGFace_t face_tag_in, double uv[2], pGEntity_t *tag_out)
{
	PK_FACE_contains_vectors_o_t pkVectorOption;
	PK_FACE_contains_vectors_o_m(pkVectorOption);

	PK_TOPOL_t topol[1];
	PK_UV_t param;

	pkVectorOption.n_uvs = 1;
	pkVectorOption.uvs = &param;
	pkVectorOption.is_on_surf = true;
	
	pkVectorOption.uvs->param[0] = uv[0];
	pkVectorOption.uvs->param[1] = uv[1];
	PK_ERROR_code_t pkErr = PK_FACE_contains_vectors(face_tag_in,&pkVectorOption,topol);
	if( pkErr == PK_ERROR_no_errors )
		*tag_out = topol[0];
	return pkErr;
}

int mgc_loop_ask_fins(pGLoop_t loop_tag_in, int *nFins,	pGFin_t **fins)			/* fins */
{
	return PK_LOOP_ask_fins(loop_tag_in,nFins,fins);
}

int mgc_edge_ask_geometry(pGEdge_t edge_tag_in, bool want_interval_in,
						  pGCurv_t *crv, MGCGeomType *ctype_, double ends_[2][3], double t_interval[2], bool *sense)
{
	PK_ERROR_code_t ret;
	PK_CLASS_t ctype;
	PK_VECTOR_t ends[2];
	PK_INTERVAL_t t_int;
	PK_LOGICAL_t edgecurvesense, want_interval;
	
	want_interval = (want_interval_in) ? PK_LOGICAL_true : PK_LOGICAL_false;
	ret = PK_EDGE_ask_geometry(edge_tag_in,want_interval,crv,&ctype,ends,&t_int,&edgecurvesense);
	if( ret == PK_ERROR_no_errors ) {
		switch( ctype ) {
			case PK_CLASS_line:		{ *ctype_ = MGC_GTYPE_LINE; break; }
			case PK_CLASS_circle:	{ *ctype_ = MGC_GTYPE_CIRCLE; break; }
			case PK_CLASS_ellipse:	{ *ctype_ = MGC_GTYPE_ELLIPSE; break; }
			default: 
				*ctype_ = MGC_GTYPE_UNKNOWN;
		}
		for( int i=0; i<2; i++ ) {
			ends_[i][0] = ends[i].coord[0];
			ends_[i][1] = ends[i].coord[1];
			ends_[i][2] = ends[i].coord[2];
		}
		if( want_interval ) {
			t_interval[0] = t_int.value[0];
			t_interval[1] = t_int.value[1];
		}
		if( edgecurvesense ) *sense = true;
		else *sense = false;
	}
	return ret;
}

int mgc_edge_ask_faces(pGEdge_t edge_tag_in, int *nFaces,	pGFace_t **faces)
{
	return PK_EDGE_ask_faces(edge_tag_in,nFaces,faces);
}

int mgc_edge_ask_type(pGEdge_t edge_tag_in, int *vertex_type)
{
	PK_EDGE_ask_type_t etype;
	PK_ERROR_code_t ret = PK_EDGE_ask_type(edge_tag_in,&etype);
	if( ret == PK_ERROR_no_errors ) {
		switch( etype.vertex_type ) {
			case PK_EDGE_type_open_c: 
				{ *vertex_type = 2;	break; }   // The edge has two distinct vertices
			case PK_EDGE_type_closed_c: 
				{ *vertex_type = 1; break; }   // The edge has the same vertex at its start and end
			case PK_EDGE_type_ring_c: 
				{ *vertex_type = 0; break; }   // The edge has no vertices
		}
	}
	return ret;
}

int mgc_edge_ask_vertices(pGEdge_t edge_tag_in, pGVertex_t verts[2])
{
	return PK_EDGE_ask_vertices(edge_tag_in,verts);
}

int mgc_fin_ask_edge(pGFin_t fin_tag_in, pGEdge_t *edge_tag_out)
{
    PK_EDGE_t edge;
	PK_LOGICAL_t ret = PK_FIN_ask_edge(fin_tag_in,&edge);
	*edge_tag_out = edge;
	return (ret) ? MGC_STATUS_ERROR : MGC_STATUS_OK ;
}

int mgc_fin_ask_face(pGFin_t fin_tag_in, pGFace_t *face_tag_out)
{
	PK_FACE_t face;
	PK_ERROR_code_t err=PK_FIN_ask_face(fin_tag_in,&face);
	if( err==PK_ERROR_no_errors ) {
		*face_tag_out = face;
	}
	return err;
}

int mgc_fin_find_surf_parameters(pGFin_t fin_tag_in, double t, double uv[2])
{
	PK_UV_t fpar,pest; 
	pest.param[0] = 0.0;
	pest.param[1] = 0.0;
    PK_ERROR_code_t err = PK_FIN_find_surf_parameters(fin_tag_in,t,0,pest,&fpar);
	if( err==PK_ERROR_no_errors ) {
		uv[0] = fpar.param[0];
		uv[1] = fpar.param[1];
	}
	return err;
}

int mgc_fin_is_positive(pGFin_t fin_tag_in, bool *sense)
{
	PK_LOGICAL_t edgefinsense;
	PK_ERROR_code_t err = PK_FIN_is_positive(fin_tag_in,&edgefinsense);
	*sense = (edgefinsense) ? true : false;
	return err;
}

int mgc_fin_ask_oriented_curve(pGFin_t fin_tag_in, pGCurv_t *crv_tag_out, bool *sense)
{
	PK_LOGICAL_t curvefinsense;
	PK_ERROR_code_t err;
	err = PK_FIN_ask_oriented_curve(fin_tag_in, crv_tag_out, &curvefinsense);
	*sense = (curvefinsense) ? true : false;
	return err;
}

int mgc_fin_find_interval(pGFin_t fin_tag_in, double t_interval[2])
{
	PK_INTERVAL_t t_int;
	PK_ERROR_code_t err = PK_FIN_find_interval(fin_tag_in,&t_int);
	if( err==PK_ERROR_no_errors ) {
		t_interval[0] = t_int.value[0];
		t_interval[1] = t_int.value[1];
	}
	return err;
}

int mgc_surf_eval(pGSurf_t surf_tag_in,	double uv_in[2], int n_derivs, 
				  double *xyz, double *du, double *dv, double *duu, double *duv, double *dvv)
{
	PK_UV_t uv;
	PK_VECTOR_t pd[6];

	uv.param[0] = uv_in[0];
	uv.param[1] = uv_in[1];
	
	PK_ERROR_code_t ret = PK_SURF_eval(surf_tag_in,uv,n_derivs,n_derivs,PK_LOGICAL_true,pd);
	if( ret )
		return ret;

	/* query for the function evaluation */
	if(xyz){
		xyz[0] = pd[0].coord[0]; xyz[1] = pd[0].coord[1]; xyz[2] = pd[0].coord[2];
	}

	if( n_derivs == 1 ) {
		/* query for the first order derivatives only */
		if(du){			
			du[0] = pd[1].coord[0]; du[1] = pd[1].coord[1]; du[2] = pd[1].coord[2];		// df/du
		}
		if(dv){			
			dv[0] = pd[2].coord[0]; dv[1] = pd[2].coord[1]; dv[2] = pd[2].coord[2];		// df/dv
		}
	}

	if( n_derivs == 2 ) {
		/* query for the second order derivatives */
		if(du){			
			du[0] = pd[1].coord[0]; du[1] = pd[1].coord[1]; du[2] = pd[1].coord[2];		// df/du
		}
		if(dv){			
			dv[0] = pd[3].coord[0]; dv[1] = pd[3].coord[1]; dv[2] = pd[3].coord[2];		// df/dv
		}
		/* query for the second order derivatives */
		if(duu){
			duu[0] = pd[2].coord[0]; duu[1] = pd[2].coord[1]; duu[2] = pd[2].coord[2];	// df^2/du^2
		}
		if(duv){
			duv[0] = pd[4].coord[0]; duv[1] = pd[4].coord[1]; duv[2] = pd[4].coord[2];	// df^2/duv
		}
		if(dvv){
			dvv[0] = pd[5].coord[0]; dvv[1] = pd[5].coord[1]; dvv[2] = pd[5].coord[2];	// df^2/dvv
		}
	}
	return ret;
}

int mgc_surf_parameterise_vector(pGSurf_t surf_tag_in, double coord_in[3], double uv_out[2])
{
	PK_UV_t par;
	PK_VECTOR_t pos;
	pos.coord[0] = coord_in[0];
	pos.coord[1] = coord_in[1];
	pos.coord[2] = coord_in[2];
	PK_ERROR_code_t err = PK_SURF_parameterise_vector(surf_tag_in,pos,&par);
	if( err==PK_ERROR_no_errors ) {
		uv_out[0] = par.param[0];
		uv_out[1] = par.param[1];
	}
	return err;
}

int mgc_surf_ask_uvbox(pGSurf_t surf_tag_in, double uvbox[4])
{
	PK_UVBOX_t uvboxsurf;
	PK_ERROR_code_t err = PK_SURF_ask_uvbox(surf_tag_in,&uvboxsurf);
	uvbox[0] = uvboxsurf.param[0];
	uvbox[1] = uvboxsurf.param[1];
	uvbox[2] = uvboxsurf.param[2];
	uvbox[3] = uvboxsurf.param[3];
	return err;
}

int mgc_surf_find_min_radii(pGSurf_t surf_tag_in, double uvbox[4],
							int *n_radii, double radii[2], double locations[2][3], double params[2][2])
{
	PK_UVBOX_t uvboxsurf;
	PK_VECTOR_t pos[2];
	PK_UV_t uv[2];

	uvboxsurf.param[0] = uvbox[0];
	uvboxsurf.param[1] = uvbox[1];
	uvboxsurf.param[2] = uvbox[2];
	uvboxsurf.param[3] = uvbox[3];
	PK_ERROR_code_t err = PK_SURF_find_min_radii(surf_tag_in,uvboxsurf,n_radii,radii,pos,uv);
	for( int i=0; i<2; i++ ) {
		locations[i][0] = pos[i].coord[0];  
		locations[i][1] = pos[i].coord[1]; 
		locations[i][2] = pos[i].coord[2];
		params[i][0] = uv[i].param[0]; 
		params[i][1] = uv[i].param[1];
	}
	return err;
}

int mgc_surf_ask_params(pGSurf_t sf,
						int iParamPeriodic[2],	
						int	iParamDegenerate[2],
						int iParamInfinite[2],
						double iParamBound[2][2])		
{
	PK_PARAM_sf_t paraminfo[2];
	PK_ERROR_code_t err = PK_SURF_ask_params(sf,paraminfo);

	if( err == PK_ERROR_no_errors ) {
		// 0 for u; 1 for v
		for( int i=0; i<2; i++ ) {
			// periodicity
			iParamPeriodic[i] = (paraminfo[i].periodic != PK_PARAM_periodic_no_c) ? 1 : 0;

			// degeneracy
			iParamDegenerate[i] = (paraminfo[i].bound[0] != PK_PARAM_bound_degenerate_c) ? 0 : 0x1 ;
			if( paraminfo[i].bound[1] == PK_PARAM_bound_degenerate_c )
				iParamDegenerate[i] |= 0x2;

			// infinity
			iParamInfinite[i] = (paraminfo[i].bound[0] != PK_PARAM_bound_infinite_c) ? 0 : 0x1 ;
			if( paraminfo[i].bound[1] == PK_PARAM_bound_infinite_c )
				iParamInfinite[i] |= 0x2;

			// range
			iParamBound[i][0] = paraminfo[i].range.value[0];
			iParamBound[i][1] = paraminfo[i].range.value[1];
		}
	}
	return err;
}

int mgc_curve_eval(pGCurv_t crv, double t, double *xyz)
{
    PK_VECTOR_t pos;
    
    PK_LOGICAL_t ret = PK_CURVE_eval(crv,t,0,&pos);
	xyz[0] = pos.coord[0];
	xyz[1] = pos.coord[1];
	xyz[2] = pos.coord[2];
	return (ret) ? MGC_STATUS_ERROR : MGC_STATUS_OK ;
}

int mgc_curve_parameterise_vector(pGCurv_t crv, double xyz[3], double *t)
{
	PK_VECTOR_t pos;
	pos.coord[0] = xyz[0];
	pos.coord[1] = xyz[1];
	pos.coord[2] = xyz[2];
	PK_LOGICAL_t ret = PK_CURVE_parameterise_vector(crv,pos,t);
	return (ret) ? MGC_STATUS_ERROR : MGC_STATUS_OK ;
}

void mgc_memory_free(pGEntity_t *gents)
{
	PK_MEMORY_free(gents);
}

/*
 *	mesh database interface
 */
int mgc_face_ask_element_count(pExternalMesh_t m, pGFace_t f)
{
	int nf=0;
	if( m ) {
		pMWMesh me = (pMWMesh)m;
		M_getClassifiedFaces(me,f,&nf);
	}
	return nf;
}

int mgc_edge_ask_element_count(pExternalMesh_t m, pGEdge_t e)
{
	int ne=0;
	if( m ) {
		pMWMesh me = (pMWMesh)m;
		M_getClassifiedEdges(me,e,&ne);
	}
	return ne;
}

int mgc_edge_ask_node_count(pExternalMesh_t m, pGEdge_t e)
{
	int nv=0;
	if( m ) {
		pMWMesh me = (pMWMesh)m;
		M_getClassifiedVertices(me,e,&nv);
	}
	return nv;
}

void mgc_edge_ask_nodes(pExternalMesh_t m, pGEdge_t e, pNode_t *nodes)
{
	int nv=0;
	if(m || nodes==NULL) {
		pMWMesh me = (pMWMesh)m;
		pVertexList l = M_getClassifiedVertices(me,e,&nv);
		if( l && nv>0 ) {
			pVertex v;
			pVertexIter ge_vtx_iter = VIter(l);
			int i=0;
			while( v = VIter_next(ge_vtx_iter) )
				nodes[i++] = (pNode_t)v;
			VIter_delete(ge_vtx_iter);
		}
	}
	return;
}

void mgc_vertex_ask_node(pExternalMesh_t m, pGVertex_t v, pNode_t *nod)
{
	int nv=0;
	if(m) {
		pMWMesh me = (pMWMesh)m;
		pVertexList l = M_getClassifiedVertices(me,v,&nv);
		if( l && nv>0 ) {
			pVertexIter gv_vtx_iter = VIter(l);
			*nod = VIter_next(gv_vtx_iter);
			VIter_delete(gv_vtx_iter);
		}
		else
			*nod = NULL;
	}
	return;
}

void mgc_node_ask_coordinates(pNode_t nd, double *xyz, double *uv)
{
	pVertex v = (pVertex)nd;
	V_getCoord(v,xyz);
	V_getParam(v,uv);
	//v->get_coordinates(xyz);
	//v->get_param(uv);
	return;
}


/*
 *  register interface
 */

void register_mgc_parasolid_interface()
{
	MGCDownwardInterface_t *iPara = MGC_new_downward_interface();

	iPara->mgc_entity_ask_class = &(mgc_entity_ask_class);
	iPara->mgc_entity_ask_identifier = &(mgc_entity_ask_identifier);
	iPara->mgc_topol_range_vector = &(mgc_topol_range_vector);
	iPara->mgc_topol_eval_mass_props = &(mgc_topol_eval_mass_props);
	iPara->mgc_body_ask_type = &(mgc_body_ask_type);
	iPara->mgc_body_ask_faces = &(mgc_body_ask_faces);
	iPara->mgc_body_ask_fins = &(mgc_body_ask_fins);
	iPara->mgc_body_ask_fins = &(mgc_body_ask_fins);
	iPara->mgc_face_ask_loops = &(mgc_face_ask_loops);
	iPara->mgc_face_ask_edges = &(mgc_face_ask_edges);
	iPara->mgc_face_ask_surf = &(mgc_face_ask_surf);
	iPara->mgc_face_ask_oriented_surf = &(mgc_face_ask_oriented_surf);
	iPara->mgc_face_find_uvbox = &(mgc_face_find_uvbox);
	iPara->mgc_face_contains_vectors = &(mgc_face_contains_vectors);
	iPara->mgc_loop_ask_fins = &(mgc_loop_ask_fins);
	iPara->mgc_edge_ask_faces = &(mgc_edge_ask_faces);
	iPara->mgc_edge_ask_type = &(mgc_edge_ask_type);
	iPara->mgc_edge_ask_vertices = &(mgc_edge_ask_vertices);
	iPara->mgc_edge_ask_geometry = &(mgc_edge_ask_geometry);
	iPara->mgc_fin_find_interval = &(mgc_fin_find_interval);
	iPara->mgc_fin_find_surf_parameters = &(mgc_fin_find_surf_parameters);
	iPara->mgc_fin_ask_edge = &(mgc_fin_ask_edge);
	iPara->mgc_fin_ask_face = &(mgc_fin_ask_face);
	iPara->mgc_fin_is_positive = &(mgc_fin_is_positive);
	iPara->mgc_fin_ask_oriented_curve = &(mgc_fin_ask_oriented_curve);
	iPara->mgc_surf_ask_params = &(mgc_surf_ask_params);
	iPara->mgc_surf_eval = &(mgc_surf_eval);
	iPara->mgc_surf_parameterise_vector = &(mgc_surf_parameterise_vector);
	iPara->mgc_surf_ask_uvbox = &(mgc_surf_ask_uvbox);
	iPara->mgc_surf_find_min_radii = &(mgc_surf_find_min_radii);
	iPara->mgc_curve_eval = &(mgc_curve_eval);
	iPara->mgc_curve_parameterise_vector = &(mgc_curve_parameterise_vector);
	iPara->mgc_memory_free = &(mgc_memory_free);					

	iPara->mgc_face_ask_element_count = &(mgc_face_ask_element_count);
	iPara->mgc_edge_ask_element_count = &(mgc_edge_ask_element_count);
	iPara->mgc_edge_ask_node_count = &(mgc_edge_ask_node_count);
	iPara->mgc_edge_ask_nodes = &(mgc_edge_ask_nodes);
	iPara->mgc_vertex_ask_node = &(mgc_vertex_ask_node);
	iPara->mgc_node_ask_coordinates = &(mgc_node_ask_coordinates);

	return;
}

void delete_mgc_parasolid_interface()
{
	MGC_delete_downward_interface();
}

/*
 *	id -> pGEntity_t conversion
 */
pGEntity_t gid2pGEntity(MGCGeomType type, pGEntity_t b, int id_in)
{
	int i, num, id;
	switch(type) {
	case MGC_GTYPE_EDGE: 
		{
			PK_EDGE_t e = 0, *edges = NULL;
	PK_BODY_ask_edges(b,&num,&edges);
			for( i=0; i<num; i++ ) {
		PK_ENTITY_ask_identifier(edges[i],&id);
		if(id == id_in)
		{ e=edges[i]; break;}
	}
	PK_MEMORY_free(edges);
	return e;
}
	case MGC_GTYPE_FACE:
		{
			PK_FACE_t f = 0, *faces = NULL;
			PK_BODY_ask_faces(b,&num,&faces);
			for( i=0; i<num; i++ ) {
				PK_ENTITY_ask_identifier(faces[i],&id);
				if(id == id_in)
				{ f=faces[i]; break;}
			}
			PK_MEMORY_free(faces);
			return f;
		}
	}
	return NULL;
}