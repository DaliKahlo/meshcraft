//$c1 XRL 12/21/11 added Comments.
//========================================================================//
//
// Example GO.cpp: PK downward GO interface implementation in CMeshWorkDoc
//
//
// See "section 4.1 Graphical Output" of Parasolid downward interface
//
// only LINE DATA are required for graphical purpose
// Line Data is produced by PK_GEOM_render_line, 
//                          PK_TOPOL_render_line 
//                      and PK_TOPOL_render_facet. 
// It is output through the GO functions GOOPSG, GOCLSG and GOSGMT.
//
// Structure of line data output
// - organized as two class of segment: hierarchical and single-level
// - GOOPSG and GOCLSG combined to output hierarchical segment
// - GOSGMT for single-level
// - for assembly: 
//		* must be flattened
//		* their constituent body tags and transformation matrices copied into entity arrays 
//
// GO....( segtyp, ntags, tags, ngeom, geom, nlntp, lntp, ifail )
// - segtyp: segment type
// - ntags, tags: tags
// - nlntp, lntp: line types
// - ngeom, geom: geometry (data are segment type dependent)
//
// There are three GO functions for producing pixel data: GOOPPX, GOPIXL, GOCLPX.
// These functions do not need to be implemented to support the PK.
//
//========================================================================//
#include "../stdafx.h"
#include "../MeshWorks.h"
#include "../MeshWorkDoc.h"
#include "../MeshWorkView.h"


// Parasolid Includes
#include "parasolid_kernel.h"
#include "frustrum_tokens.h"
#include "frustrum_ifails.h"


#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////////
// These are the frustrum GO functions which are registered in CMeshWorkView()
//

void CMeshWorkDoc::CopenSegment(const int* segtyp, const int* ntags, const int* tags, 
							 const int* ngeoms, const double* geoms, 
							 const int* nlntp, const int* lntp, int* ifail)
{
	// In CopenSegment we collect information about the hierarchical segment.
	// For example, the body type (because we need to render back-facets of
	// sheet & general bodies) or the face colour.

	// We make the assumption that the colour attribute definition has the same
	// tag (in a particular version of Parasolid). This is a fair enough assumption
	// for system attributes since they are set up in PK_SESSION_start.
	// Note, however, that user ATTDEFs may not always have the same tag if
	// PK_SESSION_stop / PK_SESSION_start has been called.

	static PK_ATTDEF_t colourDef = PK_ENTITY_null;
	GLfloat ambient[ 4 ];
	PK_ATTRIB_t colour;
	int i;
	int nColours;
	double* colourVals;

	// If this code is called then we MUST have gone through CExamlpeAppView::OnUpdate().
	// There's no other way that these Frustrum functions could have been registered.
	// The only other way is by explicitly calling these functions.
	// We'll assume this isn't going to happen. Our check for sanity of pointers need 
	// only be an assert.

	ASSERT( m_view != NULL );

	*ifail = CONTIN; // By default we won't complain to Parasolid

	if (colourDef == PK_ENTITY_null)
	{
		PK_ATTDEF_find( "SDL/TYSA_COLOUR", &colourDef );
		ASSERT( colourDef != PK_ENTITY_null );
	}

	switch( *segtyp )
	{
	case SGTPBY:  // Body: find it's type.
		PK_BODY_ask_type( tags[ 0 ], &m_currentBodyType);
		break;

	case SGTPFA:  // It's a face, i.e. we are getting facets for an entire
		          // face of a body. Why not set it's colour

		PK_ENTITY_ask_first_attrib( tags[ 0 ], colourDef, &colour );
		if (colour == PK_ENTITY_null) // No colour defined. use default.
		{
			for (i=0; i<4; i++) ambient[ i ] = (m_view->GetDefaultFacetColour())[i];
		}
		else
		{
			PK_ATTRIB_ask_doubles( colour, 0, &nColours, &colourVals );
			// The colour attrib can contain any number of doubles.
			// The Parasolid documentation says that there are only 3 entries: RGB
			// and so this is what we will adhere to. A user attribute could be used instead
			// or additionally which contained alpha value(s).

			if (nColours != 3)
			{
				TRACE( "Bad number of colours in Colour attribute. Using default." );
				// Use default colours.
				for (i=0; i<4; i++) ambient[ i ] = (m_view->GetDefaultFacetColour())[i];
			}
			else
			{
				ambient[ 3 ] = 1.0;
				for (i = 0; i<3; i++) ambient[ i ] = float( colourVals[ i ] );
			}

			// Free memory.
			PK_MEMORY_free( colourVals );
		}
	    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		break;
	case SGTPGC: // Orphan Geometry - do nothing at this moment
	case SGTPGS:
	case SGTPGB:
		break;
	default:
		// In our present implementation we aren't using hierarchical segments and so we
		// shouldn't get any other sort of segment. We might as well ASSERT _and_ return 
		// an error to Parasolid
		ASSERT( FALSE );
		*ifail = ABORT;
		break;
	}
}

void CMeshWorkDoc::CcloseSegment(const int* segtyp, const int* ntags, const int* tags, 
							 const int* ngeoms, const double* geoms, 
							 const int* nlntp, const int* lntp, int* ifail)
{
	//  There's nothing to do here...

	*ifail = CONTIN;
}

void CMeshWorkDoc::CoutputSegment(const int* segtyp, const int* ntags, const int* tags, 
							 const int* ngeoms, const double* geoms, 
							 const int* nlntp, const int* lntp, int* ifail)
{
	ASSERT( m_view != NULL );
	
	// There are many different segment types...

	*ifail = CONTIN;
	switch( *segtyp )
	{
	case SGTPER: // error segment
		TRACE( "GOSGMT: Error segment, reason: %d\n", lntp[ 1 ] );
		break;
	case SGTPVT: // visibility segment
	case SGTPVP: // paramaterised visibility segment
		TRACE( "GOSGMT: Visibility segments not supported.\n" );
		*ifail = ABORT;
		break;
	case SGTPED: // edge
	case SGTPSI: // silhouette
	case SGTPPH: // planar hatch line
	case SGTPRH: // radial hatch line
	case SGTPRU: // rib line
	case SGTPBB: // blend boundary
	case SGTPPL: // parametric hatch line
	case SGTPGC: // curve (geometry)
	case SGTPGS: // surface (geometry)
	case SGTPGB: // surface boundary (geometry)
		*ifail = ParseLineData( *segtyp, *ntags, tags, *ngeoms,
														  geoms, *nlntp, lntp );
		break;
	case SGTPMF: // mangled facet (output as triangular facet)
	case SGTPFT: // facet	

		// we will only handle triangular facets.
		// lntp[ 2 ] contains the number of loops, lntp[ 3 ] the number of sides.

		if ( lntp[ 2 ] == 1 && lntp[ 3 ] == 3)
		{
			switch( lntp[ 1 ] ) // type of facet.
			{
			case L3TPFV:  // Facet (no normals)
				TRACE( "Normals required for facets." );
				*ifail = ABORT;
				break;
			case L3TPFN: // facet with normals
				AddTriangle( geoms, geoms + (*ngeoms / 2) * 3 );
				break;
			default:
				TRACE( "GOSGMT: Unsupported Facet data\n" );
				*ifail = ABORT;
				break;
			}
		}
		else
		{
			TRACE( "GOSGMT: Only triangular facets are supported\n" );
			*ifail = ABORT;
		}
		break;
	case SGTPTS: // Facet strips.
		TRACE( "GOSGMT: This facet option not supported\n" );
		*ifail = ABORT;
		break;
	default:
		TRACE( "Unknown segment type: %d\n", *segtyp );
		*ifail = ABORT;
		break;
	}
}

void CMeshWorkDoc::AddTriangle( const double* positions, const double* normals )
{
	for (int i=0; i<3; i++)
	{
		glNormal3d( normals[ i*3 ], normals[ i*3+1 ], normals[ i*3+2 ] );
		glVertex3d( positions[ i*3 ], positions[ i*3+1 ], positions[ i*3+2 ] );
	}
	
	// If the body is sheet or general then we need to render the other side of the facet

	if (m_currentBodyType == PK_BODY_type_sheet_c || m_currentBodyType == PK_BODY_type_general_c)
	{
		for (int i=2; i>=0; i--)
		{
			glNormal3d( -normals[ i*3 ], -normals[ i*3+1 ], -normals[ i*3+2 ] );
			glVertex3d( positions[ i*3 ], positions[ i*3+1 ], positions[ i*3+2 ] );
		}
	}
}

int CMeshWorkDoc::ParseLineData( int segtyp, int ntags, const int* tags, 
							 int ngeoms, const double* geoms, 
							 int nlntp, const int* lntp )
{
	// Let's get the colour of the line segment. We won't bother looking at the segment type
	// again. If the segment is a silhouette or hatch line then tags[ 0 ] is the tag of the
	// face. We'll just get the colour attribute.
	// Note: this scheme means that edges bounding a coloured face do not get coloured with
	// the face's colour which is fair enough because this can introduce ambiguity. To colour
	// all edges of a face an application can loop through the edges of the loops of the face
	// and set colour attributes on the edges

	// We use a static (just as in CopenSegment) which has the same value.
	// This isn't too stupid since we may want to use different attributes for material 
	// attributes in future
	
	static PK_ATTDEF_t colourDef = PK_ENTITY_null;
	double colourVals[ 3 ] = { 0.0, 0.0, 0.0 };  // Default is black.
	PK_ATTRIB_t colour;
	int nColourVals;
	double* cTemp;

	if (colourDef == PK_ENTITY_null) 
	{
		PK_ATTDEF_find( "SDL/TYSA_COLOUR", &colourDef );
		ASSERT( colourDef != PK_ENTITY_null );
	}

	if ( tags[ 0 ] != PK_ENTITY_null )
	{
		PK_ENTITY_ask_first_attrib( tags[ 0 ], colourDef, &colour );
		if (colour != PK_ENTITY_null)
		{
			PK_ATTRIB_ask_doubles( colour, 0, &nColourVals, &cTemp );
			if (nColourVals == 3)
			{
				for (int i=0; i<3; i++) colourVals[ i ] = cTemp[ i ];
			}
			PK_MEMORY_free( cTemp );
		}
	}

	// Now we test for things like smoothness so that we can modify display style.

	// There are 4 smoothness tokens (passed in lntp[ 4 ] )
	// CODSMO // Edge is "smooth" 
	// CODNSM // Edge is not "smooth" 
	// CODUNS // Edge "smoothness" is unknown 
	// CODSMS // Edge "smooth" but coincident with silhouette 
	// CODUNS means that smoothness wasn't asked for.
	// CODNSM means that the edge is non-smooth.
	// CODSMS means that the edge is smooth but has to be drawn because the sil won't
	// Therefore the only time we ignore a line is if we get the code CODSMO.

	if( lntp[ 4 ] == CODSMO && segtyp == SGTPED ) // Smooth edge.
	{
		return CONTIN;
	}

	// Now deal with visibility (returned in lntp[ 3 ])
	switch( lntp[ 3 ] )  
	{
	case CODVIS:  // Visible edge. Leave as is.
	case CODUNV:  // unknown visibility (therefore assume visible)
		break;
	case CODDRV:  // Drafting hidden
		// This line is hidden by the body (by a face of the body rather than an edge)
		// We will draw it in a different colour therefore ( a paler version of the original)
		
		colourVals[ 0 ] = 0.3 * colourVals[ 0 ] + 0.7;
		colourVals[ 1 ] = 0.3 * colourVals[ 1 ] + 0.7;
		colourVals[ 2 ] = 0.3 * colourVals[ 2 ] + 0.7;
		break;
	case CODINV:  // Invisible don't plot. In fact exit now.
		return CONTIN;
		break;
	default:	// We shouldn't get here but if we do we'll continue anyway.
		TRACE( "Unknown visibility.\n" );
		break;
	}

	// Display orphan geometry in blue
	switch( segtyp )
	{
	case SGTPGC:
	case SGTPGS:
	case SGTPGB:
		colourVals[ 0 ] = 0.0;
		colourVals[ 1 ] = 0.0;
		colourVals[ 2 ] = 1.0;
		break;
	default:
		// do absolutely nothing
		break;
	}

	// Finally set the colour
	glColor3d( colourVals[ 0 ], colourVals[ 1 ], colourVals[ 2 ] );

	// O.K. That's the line style sorted out.
	// Now we need to draw it. How we do that depends (of course) on it's geometry.
	// This is returned in lntp[ 1 ].

	int i;

	switch( lntp[ 1 ] )
	{
	case L3TPSL:  // straight line
		glBegin( GL_LINES );
		glVertex3d( geoms[ 0 ], geoms[ 1 ], geoms[ 2 ] );
		glVertex3d( geoms[ 3 ], geoms[ 4 ], geoms[ 5 ] );
		glEnd();
		break;
	case L3TPPY:  // poly-line
		glBegin( GL_LINE_STRIP );
		for (i=0; i< ngeoms; i++)
		{
			glVertex3d( geoms[ i * 3 ], geoms[ i * 3 + 1 ], geoms[ i * 3 + 2 ] );
		}
		glEnd();
		break;
	case L3TPCC:  // complete circle
		AddCircle( geoms, geoms + 3, geoms[ 6 ], NULL, NULL );
		break;
	case L3TPCI:  // circular arc
		AddCircle( geoms, geoms + 3, geoms[ 6 ], geoms + 7, geoms + 10 );
		break;
	case L3TPCE:  // complete ellipse
		AddEllipse( geoms, geoms + 3, geoms + 6, geoms[ 9 ], geoms[ 10 ], NULL,
			NULL );
		break;
	case L3TPEL:  // elliptical arc
		AddEllipse( geoms, geoms + 3, geoms + 6, geoms[ 9 ], geoms[ 10 ], geoms + 11,
			geoms + 14 );
		break;
	case L3TPPC:  // non-rational B-curve (Bezier)
	case L3TPRC:  // rational B-curve (Bezier)
	case L3TPNC:  // non-rational B-curve (NURBS)
	case L3TPRN:  // rational B-curve (NURBS)
		TRACE( "Unsupported line type: %d\n", lntp[ 1 ] );
		break;
	default:
		TRACE( "Unexpected line type: %d\n", lntp[ 1 ] ); 
		break;
	}

	return CONTIN;
}

void CMeshWorkDoc::AddEllipse(const double * centre, const double * majorAxis, 
						   const double * minorAxis, double majorRadius, 
						   double minorRadius, const double * start, const double * end)
{
	const double twopi = 3.141592654 * 2.0;

	// maximum number of line segments in ellipse is defined below

	const int maxSegs = 40;

	// First de-parameterise the start & end point to get the start & end parameters
	// Then x = majorAxis * majorRadius * cos( x ) + minoraxis * minorRadius * sin( x )

	double param1, param2;

	if (start == NULL || end == NULL)
	{
		param1 = 0.0;
		param2 = twopi;
	}
	else
	{
		double vec[ 3 ];
		VectorSubtract( start, centre, vec );
		double cosval = VectorDot( vec, majorAxis ) / majorRadius;
		if (fabs( cosval - 1.0 ) < 1.0e-8) cosval = 1.0;
		if (fabs( cosval + 1.0 ) < 1.0e-8) cosval = -1.0;
		param1 = acos( cosval );
		if ( VectorDot( vec, minorAxis ) < 0.0 ) param1 = twopi - param1;

		VectorSubtract( end, centre, vec );
		cosval = VectorDot( vec, majorAxis ) / majorRadius;
		if (fabs( cosval - 1.0 ) < 1.0e-8) cosval = 1.0;
		if (fabs( cosval + 1.0 ) < 1.0e-8) cosval = -1.0;
		param2 = acos( cosval );
		if ( VectorDot( vec, minorAxis ) < 0.0 ) param2 = twopi - param2;

		if (param2 < param1) param2 += twopi;
	}

	// Work out no. of segs that we want.

	int nSegs = int( floor( (param2 - param1) / twopi * maxSegs ) );
	nSegs = max( nSegs, 1 );
	double majComp[ 3 ];
	double minComp[ 3 ];
	double loc[ 3 ];
	glBegin( GL_LINE_STRIP );
	for (int i=0; i<=nSegs; i++)
	{
		double t = param1 + ((param2 - param1) * i) / nSegs;
		VectorMult( majorAxis, majorRadius * cos( t ), majComp );
		VectorMult( minorAxis, minorRadius * sin( t ), minComp );
		VectorAdd( centre, majComp, loc );
		VectorAdd( loc, minComp, loc );
		glVertex3d( loc[ 0 ], loc[ 1 ], loc[ 2 ] );
	}
	glEnd();
}

void CMeshWorkDoc::AddCircle( const double* centre, const double* normal, double radius,
							const double* start, const double* end )
{	
	double majAxis[ 3 ];
	double minAxis[ 3 ];

	if (start == NULL || end == NULL)
	{
		PK_VECTOR1_t vec, vec2, vec3;
		for (int i=0; i<3; i++) 
		{
			vec.coord[ i ] = normal[ i ];
			vec2.coord[ i ] = 0.0;
		}
		VectorNormalise( vec.coord );  // PK_VECTOR_perpendicular is very fussy about this.
		PK_VECTOR_perpendicular( vec, vec2, &vec3 );
		for (int i=0; i<3; i++) majAxis[ i ] = vec3.coord[ i ];
	}
	else
	{
		VectorSubtract( start, centre, majAxis );
		VectorNormalise( majAxis );
	}
	VectorCross( normal, majAxis, minAxis );
	AddEllipse( centre, majAxis, minAxis, radius, radius, start, end );
}