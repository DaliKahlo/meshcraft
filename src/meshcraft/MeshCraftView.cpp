//$c9 XRL 03/29/13 save picked face into CMeshWorkDoc::m_picked.
//$c8 XRL 08/01/12 added member data m_eraseBkgnd to prevent screen flash due to $c7 change.
//$c7 XRL 08/01/12 modified onEraseBkgnd to prevent screen flash when switching view.
//$c6 XRL 07/04/12 Support modeless mesh progress dialog, and launch CMeshGen worker thread.
//$c5 XRL 12/23/11 Support the display and pick of assemblies.
//$c4 XRL 11/23/11 integrate mesh database "PSMesh" to save the generated mesh.
//$c3 XRL 03/22/11 updated to parasolid v23 and integrated BLpara.
//$c2 XRL 02/03/11 added topol/body/part picking and show its tag in GUI.
//$c1 XRL 02/01/11 created.
//========================================================================//
//
// AppView.cpp : implementation of the CMeshWorkView class
//


#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkView.h"

// Parasolid Includes
#include "frustrum_tokens.h"
#include "frustrum_ifails.h"

#include "meshing/MeshGen.h"

#include <math.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkView

IMPLEMENT_DYNCREATE(CMeshWorkView, CView)

BEGIN_MESSAGE_MAP(CMeshWorkView, CView)
	//{{AFX_MSG_MAP(CMeshWorkView)
	ON_COMMAND(ID_MESH_BTN, OnMeshBtn)
	ON_COMMAND(ID_SHADED_BTN, OnShadedBtn)
	ON_COMMAND(ID_WIREFRAME_BTN, OnWireframeBtn)
	ON_COMMAND(ID_SILHOUETTE_BTN, OnSilhouetteBtn)
	ON_COMMAND(ID_HIDDEN_BTN, OnHiddenBtn)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_VIEWROTATE_BTN, OnViewrotateBtn)
	ON_COMMAND(ID_VIEWZOOM_BTN, OnViewzoomBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEWROTATE_BTN, OnUpdateViewrotateBtn)
	ON_UPDATE_COMMAND_UI(ID_SHADED_BTN, OnUpdateShadedBtn)
	ON_UPDATE_COMMAND_UI(ID_HIDDEN_BTN, OnUpdateHiddenBtn)
	ON_UPDATE_COMMAND_UI(ID_SILHOUETTE_BTN, OnUpdateSilhouetteBtn)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME_BTN, OnUpdateWireframeBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEWZOOM_BTN, OnUpdateViewzoomBtn)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEWPAN_BTN, OnViewpanBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEWPAN_BTN, OnUpdateViewpanBtn)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_DRAFTHIDDEN_BTN, OnDraftHiddenBtn)
	ON_UPDATE_COMMAND_UI(ID_DRAFTHIDDEN_BTN, OnUpdateDrafthiddenBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkView construction/destruction

CMeshWorkView::CMeshWorkView()
{
	
	// Initialise facet options
	PK_TOPOL_facet_mesh_o_m( m_facetOptions.control );
	PK_TOPOL_render_facet_go_o_m( m_facetOptions.go_option );
	m_facetOptions.go_option.go_normals = PK_facet_go_normals_yes_c;

	// Initialise line options
	PK_TOPOL_render_line_o_m( m_lineOptions );
	m_lineOptions.planar = PK_render_planar_attrib_c;
	m_lineOptions.radial = PK_render_radial_attrib_c;
	m_lineOptions.param  = PK_render_param_attrib_c;

	// Initialise geometry line options
	PK_GEOM_render_line_o_m( m_geomlineOptions );
	m_geomlineOptions.is_curve_chord_tol = PK_LOGICAL_true;

	// member variable initialisations
	m_pDC			= NULL;
	m_glContext		= NULL;
	m_pOldPalette	= NULL;

	m_winWidth	= 300;
	m_winHeight	= 300;

	m_updateNeeded		= TRUE;
   	m_firstDraw			= TRUE;

	m_partDisplaylist = 0;

	m_currentOperation = Rotate;

	m_defaultFacetColour[ 0 ] = FCOLOR_R; // 0.0f;
	m_defaultFacetColour[ 1 ] = FCOLOR_G; // 0.0f;
	m_defaultFacetColour[ 2 ] = FCOLOR_B; // 1.0f;
	m_defaultFacetColour[ 3 ] = 1.0f;

	pMeProgDlg = NULL;
	//m_eraseBkgnd = true;
}

CMeshWorkView::~CMeshWorkView()
{
	// restore the old palette - gentle way to bow out
	if( m_pOldPalette )
		m_pDC->SelectPalette( m_pOldPalette, FALSE );

	// tidy up. delete OpenGL contexts etc.
	if (m_pDC && m_glContext)
	{
		MakeCurrent();

		// Delete the displaylist, If it does not exist, OpenGL ignores it 
		VERIFY_GL( glDeleteLists( m_partDisplaylist, 1 ) );    // deleaker

		VERIFY( wglMakeCurrent(NULL,  NULL) );
		VERIFY( wglDeleteContext(m_glContext) );

		if (m_pDC) delete m_pDC;
		m_pDC = NULL;
	}

}


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkView diagnostics

#ifdef _DEBUG
void CMeshWorkView::AssertValid() const
{
	CView::AssertValid();
}

void CMeshWorkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMeshWorkDoc* CMeshWorkView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMeshWorkDoc)));
	return (CMeshWorkDoc*)m_pDocument;
}
#endif //_DEBUG


BOOL CMeshWorkView::MakeCurrent(BOOL setCurrent)
{

	if (m_glContext == NULL) 
		return FALSE;
	
	if ( setCurrent )
		VERIFY( wglMakeCurrent(m_pDC->GetSafeHdc(), m_glContext) );   // deleaker

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkView message handlers


//*****************************************************************************
// MODULE:	on Buttons
// DESC:	All toolbar button handlers go here

void CMeshWorkView::OnMeshBtn() 
{
	CMeshWorkDoc* pDoc = GetDocument();

	if( pDoc->m_ntopol > 0 ) {
		CMeshGen *pMesher = new CMeshGen;
		int err = pMesher->premesh(pDoc->m_ntopol,pDoc->m_topols,pDoc->m_transfs);
		if( !err ) {
			// premesh succeeded
			if(pMesher->elementType() == MGC_ETYPE_TESS) {
				if( pMesher->mesh() == 0 ) {
					CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
					if( mwApp->ActiveView() != mwApp->MeshView() )
						mwApp->SwitchView();
				}
				delete pMesher;
			}
			else {
				if( pMeProgDlg ) {
					pMeProgDlg->SetForegroundWindow();  // created, simply bring it foreground
					err = 101;
				}
				else {
					pMeProgDlg = new CMeshProgressDlg(pMesher, this);  // pMesher will be deleted together with pMeProgDlg
					pMeProgDlg->Create(CMeshProgressDlg::IDD,GetDesktopWindow());
					pMeProgDlg->ShowWindow(SW_SHOW);
				}
			}
		}
		if(err) delete pMesher;
	}
}

void CMeshWorkView::OnShadedBtn() 
{
	SetViewStyle( Shaded );	
}

void CMeshWorkView::OnWireframeBtn() 
{
	SetViewStyle ( Wireframe );
}

void CMeshWorkView::OnSilhouetteBtn() 
{
	SetViewStyle ( WireAndSils );
}

void CMeshWorkView::OnHiddenBtn() 
{
	SetViewStyle ( Hidden );	
}

void CMeshWorkView::OnViewrotateBtn() 
{
	m_currentOperation = Rotate;
}

void CMeshWorkView::OnViewzoomBtn() 
{
	m_currentOperation = Zoom;
}

void CMeshWorkView::OnViewpanBtn() 
{
	m_currentOperation = Pan;
}

void CMeshWorkView::OnDraftHiddenBtn() 
{
	SetViewStyle ( DraftingHidden );
}

//*****************************************************************************
// MODULE:	on Mouse Events
// DESC:	Mouse event handlers go here

void CMeshWorkView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( nFlags & MK_LBUTTON )
	{
		switch ( m_currentOperation )
		{
		case Rotate : 
			RotateView( point );
			break;
		case Pan:
			PanView( point );
			break;
		case Zoom:
			ZoomView( point );
			break;
		default:
			break;
		}

		m_currentPoint = point;
	}

	CView::OnMouseMove(nFlags, point);
}

void CMeshWorkView::OnLButtonDown(UINT nFlags, CPoint point) 
{

	m_lastPoint		= point;
	m_currentPoint	= point;

	// Indicate no other action needed
	m_updateNeeded	= FALSE;
	m_hasRotated	= FALSE;
	
	
	CView::OnLButtonDown(nFlags, point);
}

void CMeshWorkView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	
	if (m_updateNeeded) 
	{
		if (m_hasRotated) 
			NotifyViewDirectionChanged();
		else 
			ReRender( FALSE );  // update selected entities etc.
	}

	else
	{
		CSize moved = point - m_currentPoint;
		int movedist = max( abs( moved.cx ), abs( moved.cy ) );
		if (movedist <= 3)
		{
			CMeshWorkDoc* doc = GetDocument();
			PK_TOPOL_t picked = PickTopol( m_currentPoint);
		}
	}
	Invalidate( FALSE );	
	
	CView::OnLButtonUp(nFlags, point);
}


//*****************************************************************************
// MODULE:	UpdateUI
// DESC:	Handles the windows messages related to UI update

void CMeshWorkView::OnUpdateViewrotateBtn(CCmdUI* pCmdUI) 
{
	// put check if opertion is rotate
	(Rotate == m_currentOperation) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateShadedBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is shaded
	( Shaded == GetDocument()->m_viewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateHiddenBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is hidden line
	( Hidden == GetDocument()->m_viewStyle ) 
				? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateSilhouetteBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is silhouette
	( WireAndSils == GetDocument()->m_viewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateWireframeBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is wireframe
	( Wireframe == GetDocument()->m_viewStyle ) 
					? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateViewzoomBtn(CCmdUI* pCmdUI) 
{
	// put check if opertion is zoom
	(Zoom == m_currentOperation) ? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateViewpanBtn(CCmdUI* pCmdUI) 
{
	// put check if opertion is pan
	(Pan == m_currentOperation) ? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);
}

void CMeshWorkView::OnUpdateDrafthiddenBtn(CCmdUI* pCmdUI) 
{
	// put check if viewing style is hidden line
	( DraftingHidden == GetDocument()->m_viewStyle ) 
				? pCmdUI->SetCheck(1) : pCmdUI->SetCheck(0);	
}



//---------------------------------------------------------------
// Overloaded member functions
//

BOOL CMeshWorkView::OnEraseBkgnd(CDC* pDC) 
{
	//CRect rect;
	//const COLORREF color = BKGDCOLOR;
	//GetClientRect(&rect);	
	//pDC->FillSolidRect(&rect,color);

	// Do nothing so that the base method doesn't get invoked
	return false; 
}

void CMeshWorkView::OnSize(UINT nType, int cx, int cy) 
{
	// call a generic public function
	ReSize( nType, cx, cy) ;
}

//---------------------------------//
// CraigW 6/6/01
//	Fix minor bug where maximising the view will not properly update the view
//
//---------------------------------//

void CMeshWorkView::ReSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	
	if (cx <= 0 || cy <= 0) return;  // spurious calls, which CAN happen when the view is created

	m_winHeight  = cy;
	m_winWidth   = cx;
	//m_eraseBkgnd = false;

	if( MakeCurrent( TRUE) )
	{
		if ( !m_firstDraw )
			VERIFY_GL( glDrawBuffer( GL_FRONT_AND_BACK ) );

		VERIFY_GL( glViewport( 0, 0, m_winWidth, m_winHeight ) );

		SetViewVolume();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkView drawing

void CMeshWorkView::OnDraw(CDC* pDC)
{
	CMeshWorkDoc* pDoc = GetDocument();

	MakeCurrent( TRUE );
	
	if( m_firstDraw == TRUE )
	{ 
		m_firstDraw = FALSE; 
		// Otherwise the first draw is a blank !
		VERIFY_GL( glDrawBuffer( GL_FRONT_AND_BACK ) );
		ReRender();
		
	} 
	else 
	VERIFY_GL( glDrawBuffer( GL_BACK ) );

 	VERIFY_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

	VERIFY_GL( glMatrixMode( GL_MODELVIEW ) );
	VERIFY_GL( glPushMatrix() );
	VERIFY_GL( glTranslated( -pDoc->m_viewCentre.coord[ 0 ], 
							 -pDoc->m_viewCentre.coord[ 1 ], 
							 -pDoc->m_viewCentre.coord[ 2 ] ) );

	if ( m_geomList.IsEmpty() == FALSE || m_partList.IsEmpty() == FALSE ) 
	{
		VERIFY_GL( glCallList( m_partDisplaylist ) );
	}

	VERIFY_GL( glFlush() );
	VERIFY_GL( glPopMatrix() );

	SwapBuffers( pDC->m_hDC );
	pDC->RealizePalette();
	MakeCurrent( FALSE );
}

void CMeshWorkView::OnInitialUpdate() 
{
	m_updateNeeded = TRUE;

	if (m_pDC == NULL) 
		InitOpenGL();

	MakeCurrent( TRUE );  // deleaker

	CMeshWorkDoc* doc = GetDocument();
	doc->m_view = this;  // Store current view in document class

	SetLighting( doc->m_viewStyle );
	
	Invalidate();
	
}

void CMeshWorkView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// Present implementation will re-render all parts in the view list
	// Future implementations (or derived classed) could use hint info to
	// restrict recalculation


	CMeshWorkDoc* pDoc = GetDocument();

	// Add all the topols in document to list
	m_partList.RemoveAll();
	m_transfList.RemoveAll();
	for (int i=0; i<pDoc->m_ntopol; i++) 
		m_partList.AddTail( pDoc->m_topols[i] );
	if  (pDoc->m_transfs != NULL)
	{
		for (int i=0; i<pDoc->m_ntopol; i++) 
			m_transfList.AddTail( pDoc->m_transfs[i] );
	}

	// Add all the orphan geometry to list. Note: We are only interested in 
	// bcurves and bsurfaces
	PK_CLASS_t Class;
	m_geomList.RemoveAll();
	if (pDoc->m_ngeoms >0)
	{
		for (int i=0; i<pDoc->m_ngeoms; i++ )
		{
			PK_ENTITY_ask_class( pDoc->m_geoms[i], &Class );
			if ( Class == PK_CLASS_bcurve || Class == PK_CLASS_bsurf )
				m_geomList.AddTail( pDoc->m_geoms[i] );
		}
	}
	
	FitPartsInView();

	if (lHint > 0)
	{
		ReRender( FALSE );
	}
	else
	{
		ReRender( TRUE );
	}
	Invalidate( FALSE );
}

BOOL CMeshWorkView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	//  Change the class name to our own name.
	CMeshWorks* pApp = (CMeshWorks*)AfxGetApp ();
    //cs.lpszClass = (const char *)(pApp->m_strMyClassName);
	cs.lpszClass = pApp->m_strMyClassName;

	return CView::PreCreateWindow(cs);
}


GLfloat* CMeshWorkView::GetDefaultFacetColour()
{
	return m_defaultFacetColour;
}

// This function is used to ensure that the rotation component of a
// view matrix ie the first 3 rows and columns is orthonormal ( all the rows 
// ( and columns are unit vectors ).

BOOL CMeshWorkView::FixUpMatrix(PK_TRANSF_sf_t &vtsf)
{
	// Lets first of all get the rows of the rotation submatrix from view transformation
	double row[3][3];
	double* vector = NULL;
	double temp[3];
	int i;
	vector = temp;
//	double column[3][3];

	for ( i=0; i<3; i++ )
	{
		for ( int j=0; j<3; j++ )
		{
			row[i][j] = vtsf.matrix[i][j];
		}
		// Now lets normalise each of the vectors
		vector = VectorNormalise( row[i] );
		for ( int k=0; k<3; k++ )
			row[i][k] = vector[k];
	}
	
	double dotproduct[3];
	// cyclically circle around taking the dotproduct of the pairs of vectors
	for ( i=0; i<3; i++ )
	{
		if ( i != 2 )
			dotproduct[i] = fabs( VectorDot( row[i], row[i+1] ) );
		else 
			dotproduct[i] = fabs( VectorDot( row[i], row[0] ) );
	}

	// now lets find the smallest of these products
	double smallest = 0.0;
	int index = -1;
	for ( i=0; i<3; i++ )
	{
		if ( index == -1 )
		{
			index = i; 
			smallest = dotproduct[i];
		}
		else
		{
			if ( dotproduct[i] < smallest )
			{
				index = i;
				smallest = dotproduct[i];
			}
		}
	}

	// now from this take the cross-produce of the remaining rows and normalise it
	if ( index == 0 )
	{
		VectorCross( row[0], row[1], row[2] );
		vector = VectorNormalise( row[2] );
		for ( int k=0; k<3; k++ )
			row[2][k] = vector[k];
		VectorCross( row[1], row[2], row[0] );
	}
	if ( index == 1 )
	{
		VectorCross( row[1], row[2], row[0] );
		vector = VectorNormalise( row[0] );
		for ( int k=0; k<3; k++ )
			row[0][k] = vector[k];
		VectorCross( row[2], row[0], row[1] );
	}
	else
	{
		VectorCross( row[2], row[0], row[1] );
		vector = VectorNormalise( row[1] );
		for ( int k=0; k<3; k++ )
			row[1][k] = vector[k];
		VectorCross( row[0], row[1], row[2] );
	}

	// now fill in the matrix
	for (  i=0; i<3; i++ )
		for ( int j=0; j<3; j++ )
			vtsf.matrix[i][j] = row[i][j];

	// could also check the row values to make sure that they are unitvectors
/*	for ( i=0; i<3; i++ )
	{
		for ( int j=0; j<3; j++ )
		{
			column[i][j] = vtsf.matrix[j][i];
		}
	}
	
	ASSERT( IsUnitVector(column[0]) );
	ASSERT( IsUnitVector(column[1]) );
	ASSERT( IsUnitVector(column[2]) );*/

	return TRUE;
}

//-----------------------------------------------------------------------------
// FUNC:	ReRender
// ACTION:	rebuilds to OpenGL display list depending upon the drawing mode 
//			shaded/wireframe etc.

void CMeshWorkView::ReRender(BOOL updateAll)
{
	CMeshWorkDoc* doc = GetDocument();

	int nTopols, nGeoms, i;
	PK_TOPOL_t* topols;
	PK_TRANSF_t* transfs;
	PK_GEOM_t*  geoms;
	PK_ERROR_code_t ret;

	nTopols = m_partList.GetCount();
	TRACE( "nTopols = %d\n", nTopols );
	nGeoms = m_geomList.GetCount();
	TRACE( "nGeoms = %d\n", nGeoms );

	MakeCurrent();
	glNewList( m_partDisplaylist, GL_COMPILE_AND_EXECUTE );

	if (nTopols > 0)
	{	
		i = 0;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * nTopols, (void**)&topols );
		POSITION pos = m_partList.GetHeadPosition();
		while( pos )
			topols[i++] = m_partList.GetNext( pos );

		transfs = NULL;
		if(m_transfList.GetCount() > 0) {
			i = 0;
			PK_MEMORY_alloc( sizeof( PK_TRANSF_t ) * nTopols, (void**)&transfs );
			pos = m_transfList.GetHeadPosition();
			while( pos )
				transfs[i++] = m_transfList.GetNext( pos );
		}

		if (doc->m_viewStyle == Shaded)
		{
			// Update rendering parameters
			m_facetOptions.control.is_surface_plane_tol = PK_LOGICAL_true;
			m_facetOptions.control.surface_plane_tol = 0.005 * doc->m_bSphereRad;
			m_facetOptions.control.is_curve_chord_tol = PK_LOGICAL_true;
			m_facetOptions.control.curve_chord_tol = 0.001 * doc->m_bSphereRad;
			m_facetOptions.go_option.go_edges = PK_facet_go_edges_yes_c;
			
			// Need to check for general bodies so that we can turn vertex matching off
			PK_CLASS_t eclass;
			PK_BODY_type_t bodyType;
			BOOL general = FALSE;
			for (i=0; i<nTopols; i++)
			{
				PK_ENTITY_ask_class( topols[ i ], &eclass );
				if (eclass == PK_CLASS_body)
				{
					PK_BODY_ask_type( topols[ i ], &bodyType );
					if (bodyType == PK_BODY_type_general_c)
					{
						general = TRUE;
						break;
					}
				} 
			}
			if (general) m_facetOptions.control.match =  PK_facet_match_geom_c;

			glBegin( GL_TRIANGLES );
			ret = PK_TOPOL_render_facet( nTopols, topols, transfs, PK_ENTITY_null, &m_facetOptions );
			glEnd();
		}
		else
		{
			// Line rendering is required. Set the options for hidden line & do the
			// rendering.
			m_lineOptions.silhouette = 
			( doc->m_viewStyle == Wireframe ) ? PK_render_silhouette_no_c : PK_render_silhouette_yes_c;
			switch( doc->m_viewStyle )
			{
			case Wireframe:
			case WireAndSils:
				m_lineOptions.visibility = PK_render_vis_no_c;
				break;
			case Hidden:
				m_lineOptions.visibility = PK_render_vis_hid_c;
				break;
			case DraftingHidden:
				m_lineOptions.visibility = PK_render_vis_inv_draft_c;
				break;
			default:
				TRACE( "CParaView::OnUpdate: Unexpected enum value" );
			}

			// Now, we need a transformation for the rendering in the cases where we
			// are performing view dependent stuff ie for any view other than wireframe.
			// We get this from the OpenGL modelview matrix. The OpenGL matrix needs 
			// transposing.

			PK_TRANSF_t viewTra = PK_ENTITY_null;
			PK_TRANSF_sf_t vtsf;
			double m[ 16 ];
			if (doc->m_viewStyle != Wireframe)
			{
				glGetDoublev( GL_MODELVIEW_MATRIX, m );
				for (int i=0; i<4; i++) 
				{
					for (int j=0; j<4; j++) vtsf.matrix[ i ][ j ] = m[ j * 4 + i ];
				}

				FixUpMatrix( vtsf );

				VERIFY( PK_TRANSF_create( &vtsf, &viewTra ) == PK_ERROR_no_errors );
			}
			ret = PK_TOPOL_render_line( nTopols, topols, transfs, viewTra, &m_lineOptions );
			if (viewTra != PK_ENTITY_null) PK_ENTITY_delete( 1, &viewTra );
		}

		PK_MEMORY_free( topols );
		if(transfs) PK_MEMORY_free( transfs );

	}

	// Now deal with orphan geometry. Note: PK_GEOM_render_line only renders a view
	// independent wireframe so it makes sense only to display them in wireframe mode. 
	
	if (nGeoms > 0)
	{	
		i=0;
		PK_MEMORY_alloc( sizeof( PK_GEOM_t ) * nGeoms, (void**)&geoms );
		POSITION pos = m_geomList.GetHeadPosition();

		while( pos ) 
			geoms[ i++ ] = m_geomList.GetNext( pos );

		if (doc->m_viewStyle == Shaded)
		{
			// Don't try to display anything in Shaded mode
		}
		else
		{
			switch( doc->m_viewStyle )
			{
			// Only display in Wireframe mode. 
			case Wireframe:
				m_geomlineOptions.curve_chord_tol = 0.001 * doc->m_bSphereRad;
				PK_GEOM_render_line( nGeoms, geoms, NULL, &m_geomlineOptions );
				break;
			case WireAndSils:
			case Hidden:
			case DraftingHidden:
				break;
			default:
				TRACE( "CParaView::OnUpdate: Unexpected enum value" );
			}
		}

		PK_MEMORY_free( geoms );
	}
	
	glEndList();
	MakeCurrent( FALSE );
	Invalidate();
}

//-----------------------------------------------------------------------------
// Helper functions for Pan, Zoom and Rotate
//

//-----------------------------------------------------------------------------
// FUNC;	RotateView
// ACTION:	Rotates the view using the difference between two CPoints

void CMeshWorkView::RotateView(CPoint point)
{
	MakeCurrent();

	double diffx, diffy;
	diffx = (double) (point.x - m_currentPoint.x);
	diffy = (double) (point.y - m_currentPoint.y);

	double angleX, angleY;
	angleX = diffx * ROTATE_SENSITIVITY;
	angleY = diffy * ROTATE_SENSITIVITY;

	double m[ 16 ];

	VERIFY_GL( glMatrixMode( GL_MODELVIEW ) );
	VERIFY_GL( glGetDoublev( GL_MODELVIEW_MATRIX, m ) );
	VERIFY_GL( glLoadIdentity() );

	VERIFY_GL( glRotated( angleX, 0.0, 1.0, 0.0 ) );
	VERIFY_GL( glRotated( angleY, 1.0, 0.0, 0.0 ) );

	VERIFY_GL( glMultMatrixd( m ) );
	Invalidate( FALSE );

	MakeCurrent( FALSE );

	m_updateNeeded = TRUE;
	m_hasRotated = TRUE;
}

void CMeshWorkView::NotifyViewDirectionChanged()
{
	CMeshWorkDoc* doc = GetDocument();
	// Views with silhouettes need rerendering
	BOOL rerenderAll =   ( doc->m_viewStyle != Shaded ) &&
						 ( doc->m_viewStyle != Wireframe );
	ReRender( rerenderAll );
}

//-----------------------------------------------------------------------------
// FUNC:	ZoomView
// ACTION:	Scales up/down the object in the view using the difference between the
//			two CPoints. In this case the point and LastPoint

void CMeshWorkView::ZoomView(CPoint point)
{
	CMeshWorkDoc* pDoc = GetDocument();
	// zoom only if mouse moves in vertical direction
	double diffy;
	diffy =  ( (double)( point.y - m_currentPoint.y) ) * PAN_SENSITIVITY;

	if( ( pDoc->m_scaleFactor + diffy ) > MIN_SCALE_FACTOR )
		  pDoc->m_scaleFactor = pDoc->m_scaleFactor + diffy;

	double xextent = double( m_winWidth ) / min( double( m_winWidth ), double( m_winHeight ));
	double yextent = double( m_winHeight ) / min( double( m_winWidth ), double( m_winHeight ));

	SetViewVolume();

	Invalidate( FALSE );
	
	m_updateNeeded = TRUE;

	return;
}

//-----------------------------------------------------------------------------
// FUNC:	PanView
// ACTION:	Moves the centre of the view using the difference between two points.

void CMeshWorkView::PanView(CPoint point)
{
	// get the document, manipulating centre 
	CMeshWorkDoc* pDoc = GetDocument();
	
	MakeCurrent( TRUE );

	CSize moved		= point - m_currentPoint;
	int movedist	= max( abs( moved.cx ), abs( moved.cy ));

	double factor	= PAN_SENSITIVITY * pDoc->m_bSphereRad * pDoc->m_scaleFactor;

	double m[ 16 ];
	int    i, x, y;
	
	x =  moved.cx;
	y = -moved.cy;

	VERIFY_GL( glGetDoublev( GL_MODELVIEW_MATRIX, m ) );

	for ( i=0; i<3; i++)
	{
		pDoc->m_viewCentre.coord[i] -= ( m[4*i]*x + m[4*i+1]*y )*factor;
	}

	Invalidate();
	MakeCurrent( FALSE );

	return;
}

//-----------------------------------------------------------------------------
// FUNC:	SetViewStyle
// ACTION:	Sets the current view style. It does this by clearing the front and 
//			back buffers, setting the lighting ( only need lighting for shaded 
//			mode ) and rerendering the view.

void CMeshWorkView::SetViewStyle(ViewStyle style)
{
	CMeshWorkDoc* doc = GetDocument();

	if ( doc->m_viewStyle != style )
	{
		doc->m_viewStyle = style;
		VERIFY_GL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );
		SetLighting ( style );
		ReRender();
		Invalidate( FALSE );
		MakeCurrent( FALSE );
	}
}

//-----------------------------------------------------------------------------
// FUNC:	SetViewVolume
// ACTION:	Sets the extents of the view volume. Make sure to MakeCurrent(TRUE)
//			before calling. Call this function if you think you have changed
//			the window extents, part bounding box, or zoom


void CMeshWorkView::SetViewVolume()
{
	CMeshWorkDoc* pDoc = GetDocument();

	double xextent = double( m_winWidth ) / min( double( m_winWidth ), double( m_winHeight ));
	double yextent = double( m_winHeight ) / min( double( m_winWidth ), double( m_winHeight ));

	// we're setting a maxmimum limit to the value of m_bSphere*50.0 at a arbitrary value
	// of MAX_SPHERE_SIZE. This means that we can now pick from bodies up to the size box 
	// in size
	double sphereSize = pDoc->m_bSphereRad* 50.0;
	if ( sphereSize > MAX_SPHERE_SIZE )
		sphereSize = MAX_SPHERE_SIZE;

	VERIFY_GL( glMatrixMode(GL_PROJECTION) );
	VERIFY_GL( glLoadIdentity() );	
	VERIFY_GL( glOrtho( -xextent * pDoc->m_bSphereRad * pDoc->m_scaleFactor, 
						 xextent * pDoc->m_bSphereRad * pDoc->m_scaleFactor, 
						-yextent * pDoc->m_bSphereRad * pDoc->m_scaleFactor, 
						 yextent * pDoc->m_bSphereRad * pDoc->m_scaleFactor, 
						 sphereSize, -sphereSize));
}

//-------------------------------------------------------------------------
//	FUNC:	FitPartsInView
//	ACTION: Scans through the part and geom list and alters the viewport and viewcentre 
//			such that all the parts fit in the view.

void CMeshWorkView::FitPartsInView()
{ 
	CMeshWorkDoc* pDoc = GetDocument();

	if ( m_partList.IsEmpty() && m_geomList.IsEmpty() )
		return;
 
	PK_BOX_t sizebox;
	PK_BOX_t partbox;
	BOOL first = TRUE;

	POSITION partListPos = m_partList.GetHeadPosition();
	while( partListPos )
	{
		PK_TOPOL_t thisTopol = m_partList.GetNext( partListPos );
		PK_TOPOL_find_box( thisTopol, &partbox );
		if ( first )
		{
			sizebox = partbox;
			first = FALSE;
		}
		else
		{
			sizebox.coord[ 0 ] = min( sizebox.coord[ 0 ], partbox.coord[ 0 ] );
			sizebox.coord[ 1 ] = min( sizebox.coord[ 1 ], partbox.coord[ 1 ] );
			sizebox.coord[ 2 ] = min( sizebox.coord[ 2 ], partbox.coord[ 2 ] );
			sizebox.coord[ 3 ] = max( sizebox.coord[ 3 ], partbox.coord[ 3 ] );
			sizebox.coord[ 4 ] = max( sizebox.coord[ 4 ], partbox.coord[ 4 ] );
			sizebox.coord[ 5 ] = max( sizebox.coord[ 5 ], partbox.coord[ 5 ] );
		}
	}

	// Now deal with orphan geometry
	POSITION geomListPos = m_geomList.GetHeadPosition();
	while( geomListPos )
	{
		PK_GEOM_t thisGeom = m_geomList.GetNext( geomListPos );
		PK_LOGICAL_t is_curve;
		PK_INTERVAL_t	t_int;
		PK_VECTOR_t centre;
		PK_VECTOR_t axes[3];
		PK_UVBOX_t uvbox;
		double widths[3];
		double x_axis[3] = { 1, 0, 0 };
		double y_axis[3] = { 0, 1, 0 };
		double z_axis[3] = { 0, 0, 1 };
		int dimension;
		// Find enclosing box of geom... 
		PK_ENTITY_is_curve( thisGeom, &is_curve );
		if ( is_curve )
		{
			PK_CURVE_ask_interval( thisGeom, &t_int );
			PK_CURVE_find_non_aligned_box( thisGeom, t_int, &centre, axes, widths, &dimension );
		}
		else
		{
			PK_SURF_ask_uvbox( thisGeom, &uvbox );
			PK_SURF_find_non_aligned_box( thisGeom, uvbox, &centre, axes, widths, &dimension );
		}
			
		if ( first )
		{
			sizebox.coord[0] = centre.coord[0] - widths[0] * fabs ( VectorDot( axes[0].coord, x_axis ) )
	    								       - widths[1] * fabs ( VectorDot( axes[1].coord, x_axis ) )
										       - widths[2] * fabs ( VectorDot( axes[2].coord, x_axis ) );
			sizebox.coord[1] = centre.coord[1] - widths[0] * fabs ( VectorDot( axes[0].coord, y_axis ) )
										       - widths[1] * fabs ( VectorDot( axes[1].coord, y_axis ) )
										       - widths[2] * fabs ( VectorDot( axes[2].coord, y_axis ) );
			sizebox.coord[2] = centre.coord[2] - widths[0] * fabs ( VectorDot( axes[0].coord, z_axis ) )
											   - widths[1] * fabs ( VectorDot( axes[1].coord, z_axis ) ) 
											   - widths[2] * fabs ( VectorDot( axes[2].coord, z_axis ) );
			sizebox.coord[3] = centre.coord[0] + widths[0] * fabs ( VectorDot( axes[0].coord, x_axis ) ) 
										       + widths[1] * fabs ( VectorDot( axes[1].coord, x_axis ) ) 
											   + widths[2] * fabs ( VectorDot( axes[2].coord, x_axis ) );
			sizebox.coord[4] = centre.coord[1] + widths[0] * fabs ( VectorDot( axes[0].coord, y_axis ) )
											   + widths[1] * fabs ( VectorDot( axes[1].coord, y_axis ) )
											   + widths[2] * fabs ( VectorDot( axes[2].coord, y_axis ) );
			sizebox.coord[5] = centre.coord[2] + widths[0] * fabs ( VectorDot( axes[0].coord, z_axis ) )
											   + widths[1] * fabs ( VectorDot( axes[1].coord, z_axis ) )
											   + widths[2] * fabs ( VectorDot( axes[2].coord, z_axis ) );
			first = FALSE;
		}
		else 
		{

			sizebox.coord[0] = min ( centre.coord[0] - widths[0] * fabs ( VectorDot( axes[0].coord, x_axis ) )
													 - widths[1] * fabs ( VectorDot( axes[1].coord, x_axis ) )
													 - widths[2] * fabs ( VectorDot( axes[2].coord, x_axis ) ), sizebox.coord[0] );
			sizebox.coord[1] = min ( centre.coord[1] - widths[0] * fabs ( VectorDot( axes[0].coord, y_axis ) )
													 - widths[1] * fabs ( VectorDot( axes[1].coord, y_axis ) )
													 - widths[2] * fabs ( VectorDot( axes[2].coord, y_axis ) ), sizebox.coord[1] );
			sizebox.coord[2] = min ( centre.coord[2] - widths[0] * fabs ( VectorDot( axes[0].coord, z_axis ) )
													 - widths[1] * fabs ( VectorDot( axes[1].coord, z_axis ) ) 
													 - widths[2] * fabs ( VectorDot( axes[2].coord, z_axis ) ), sizebox.coord[2] );
			sizebox.coord[3] = max ( centre.coord[0] + widths[0] * fabs ( VectorDot( axes[0].coord, x_axis ) ) 
													 + widths[1] * fabs ( VectorDot( axes[1].coord, x_axis ) ) 
													 + widths[2] * fabs ( VectorDot( axes[2].coord, x_axis ) ), sizebox.coord[3] );
			sizebox.coord[4] = max ( centre.coord[1] + widths[0] * fabs ( VectorDot( axes[0].coord, y_axis ) )
													 + widths[1] * fabs ( VectorDot( axes[1].coord, y_axis ) )
													 + widths[2] * fabs ( VectorDot( axes[2].coord, y_axis ) ), sizebox.coord[4] );
			sizebox.coord[5] = max ( centre.coord[2] + widths[0] * fabs ( VectorDot( axes[0].coord, z_axis ) )
													 + widths[1] * fabs ( VectorDot( axes[1].coord, z_axis ) )
													 + widths[2] * fabs ( VectorDot( axes[2].coord, z_axis ) ), sizebox.coord[5] );
			}
	}

	PK_VECTOR_t tvec;

	tvec.coord[ 0 ] = ( sizebox.coord[ 0 ] + sizebox.coord[ 3 ]) / 2.0;
	tvec.coord[ 1 ] = ( sizebox.coord[ 1 ] + sizebox.coord[ 4 ]) / 2.0;
	tvec.coord[ 2 ] = ( sizebox.coord[ 2 ] + sizebox.coord[ 5 ]) / 2.0;

	pDoc->m_viewCentre = tvec;
	
	tvec.coord[ 0 ] = sizebox.coord[ 0 ] - sizebox.coord[ 3 ];
	tvec.coord[ 1 ] = sizebox.coord[ 1 ] - sizebox.coord[ 4 ];
	tvec.coord[ 2 ] = sizebox.coord[ 2 ] - sizebox.coord[ 5 ];

	pDoc->m_bSphereRad = sqrt( VectorDot( tvec.coord, tvec.coord ) ) / 1.9;

	if ( pDoc->m_bSphereRad <= 1.0e-8) 
			pDoc->m_bSphereRad = 1.0;

	pDoc->m_scaleFactor = 1.0;

	SetViewVolume();

	MakeCurrent(FALSE);
	
}

//-------------------------------------------------------------------------
//	FUNC:	SetLighting
//	ACTION: Sets the lighting according to the current view. Note that only 
//			a shaded view actually requires lights

void CMeshWorkView::SetLighting(ViewStyle style)
{
	if( style == Shaded )
		glEnable( GL_LIGHTING );
	else
		glDisable( GL_LIGHTING );
}

//-----------------------------------------------------------------------
// CMeshWorkView: selection

PK_TOPOL_t CMeshWorkView::PickTopol(CPoint point)
{
	GLdouble mm[ 16 ];
	GLdouble pm[ 16 ];
	GLint    vp[ 4 ];

	double x, y, z;
	double radius;

	PK_VECTOR_t  location, extent;
	PK_VECTOR1_t direction;
	RECT		 lpRect;

	CMeshWorkDoc* doc = GetDocument();

	if (doc->m_ntopol == 0) 
		return PK_ENTITY_null;

	GetClientRect( &lpRect );
	point.y = lpRect.bottom - point.y;

	VERIFY_GL( glGetDoublev( GL_MODELVIEW_MATRIX, mm ) );
	VERIFY_GL( glGetDoublev( GL_PROJECTION_MATRIX, pm ) );
	VERIFY_GL( glGetIntegerv( GL_VIEWPORT, vp ) );

	VERIFY( gluUnProject( double( point.x ), double( point.y ), 1.0, 
				  mm, pm, vp, &x, &y, &z ) == GL_TRUE );
	location.coord[ 0 ] = x;
	location.coord[ 1 ] = y;
	location.coord[ 2 ] = z;

	VERIFY( gluUnProject( double( point.x ), double( point.y ), 0.0, 
				  mm, pm, vp, &x, &y, &z ) == GL_TRUE );
	direction.coord[ 0 ] = x - location.coord[ 0 ];
	direction.coord[ 1 ] = y - location.coord[ 1 ];
	direction.coord[ 2 ] = z - location.coord[ 2 ];
	VectorNormalise( direction.coord );

	VERIFY( gluUnProject( double( point.x ), double( point.y + 3. ), 1.0, 
				  mm, pm, vp, &x, &y, &z ) == TRUE );
	extent.coord[ 0 ] = x;
	extent.coord[ 1 ] = y;
	extent.coord[ 2 ] = z;

	VectorSubtract( extent.coord, location.coord, extent.coord );
	radius = sqrt( VectorDot( extent.coord, extent.coord ) );
	VectorAdd( location.coord, doc->m_viewCentre.coord, location.coord );

	PK_AXIS1_sf_t ray;
	ray.location = location;
	ray.axis = direction;

	PK_BODY_pick_topols_o_t pickOptions;
	PK_BODY_pick_topols_o_m( pickOptions );
	
	pickOptions.max_edges			= 1;
	pickOptions.max_faces		= 1;
	pickOptions.max_vertices		= 1;
	pickOptions.max_edge_dist		= radius;
	pickOptions.max_vertex_dist		= radius; 
	pickOptions.method				= PK_BODY_pick_radial_c;
    pickOptions.is_curve_chord_tol	= m_lineOptions.is_curve_chord_tol; 
    pickOptions.curve_chord_tol		= m_lineOptions.curve_chord_tol; 
    pickOptions.is_curve_chord_max	= m_lineOptions.is_curve_chord_max; 
    pickOptions.curve_chord_max		= m_lineOptions.curve_chord_max; 
    pickOptions.is_curve_chord_ang	= m_lineOptions.is_curve_chord_ang; 
    pickOptions.curve_chord_ang		= m_lineOptions.curve_chord_ang; 

	PK_ENTITY_t picked = PK_ENTITY_null;
	int	occurence = -1;
	PickParts( doc->m_ntopol, doc->m_topols, doc->m_transfs, ray, pickOptions, &picked, &occurence );
	
	CString message;
	PK_CLASS_t pk_class;
	CString entity_class;
	int id;

	if(picked != PK_ENTITY_null)
	{
		PK_ENTITY_ask_identifier(picked,&id);
		PK_ENTITY_ask_class(picked, &pk_class);
		if(pk_class == PK_CLASS_face)
			entity_class = "PK_CLASS_face";
		else if(pk_class == PK_CLASS_edge)
			entity_class = "PK_CLASS_edge";
		else if(pk_class == PK_CLASS_vertex)
			entity_class = "PK_CLASS_vertex";
		else
			entity_class = "unknown";

		if(pk_class == PK_CLASS_vertex) {
			PK_POINT_t pt;
			PK_POINT_sf_t pos;
			PK_VERTEX_ask_point(picked,&pt);
			PK_POINT_ask(pt,&pos);
			message.Format(_T("Picked a vertex (tag=%d id=%d occurence=%d)\nLocation: (%f, %f, %f)"), picked, id, occurence,
				pos.position.coord[0],pos.position.coord[1],pos.position.coord[2]);
		}
		else if(pk_class == PK_CLASS_edge) {
	
			PK_VECTOR_t ends[2];
			PK_INTERVAL_t t_int;
			PK_LOGICAL_t edgecurvesense;
			PK_CURVE_t d_curve;
			PK_CLASS_t ctype;
			PK_ERROR_code_t err;
			err = PK_EDGE_ask_geometry(picked,true,&d_curve,&ctype,ends,&t_int,&edgecurvesense);

			if( ctype == PK_CLASS_circle ) {
				PK_CIRCLE_sf_t circle;
				err = PK_CIRCLE_ask(d_curve,&circle);
				message.Format(_T("Picked a circle (tag=%d id=%d occurence=%d)\nRadius: %f\nCenter: (%f, %f, %f)"), picked, id, occurence,
								circle.radius, 
								circle.basis_set.location.coord[0], 
								circle.basis_set.location.coord[1],
								circle.basis_set.location.coord[2]);
			}
			else 
				message.Format(_T("Picked an edge (tag=%d id=%d occurence=%d)\n   Type: %d\n   t_int: (%f, %f)"), picked, id, occurence,
								ctype, 
								t_int.value[0], 
								t_int.value[1]);
		}
		else if(pk_class == PK_CLASS_face) {
			int found = 0;
			for (std::list<InstancedTag>::iterator it=doc->m_picked.begin(); it!=doc->m_picked.end(); ++it) {
				if(it->occurence == occurence && it->tag == picked) {
					it = doc->m_picked.erase(it);
					found = 1;
					break;
				}
			}
			if(!found) {
				InstancedTag curPick;
				curPick.occurence = occurence;
				curPick.tag = picked;
				doc->m_picked.push_back(curPick);
			}
			message.Format(_T("Picked a face (tag=%d id=%d occurence=%d)"), picked, id, occurence);
		}
		else
			message.Format(_T("Picked a %s entity (tag=%d id=%d occurence=%d)"), entity_class, picked, id, occurence);

		// highlight the picked entity   2/2/2011  XRL
		static PK_ATTDEF_t colour_attdef=0;
		PK_ATTRIB_t colour_attrib=0;
		double colour_doubles[4] = {SELECTED_FCOLOR_R, SELECTED_FCOLOR_G, SELECTED_FCOLOR_B, 1.0};
		int n_attribs, nDeletedAttrib;
		PK_ATTDEF_find("SDL/TYSA_COLOUR", &colour_attdef);
		PK_ENTITY_ask_attribs(picked,colour_attdef,&n_attribs,NULL);
		if( n_attribs==0 ) {
			PK_ATTRIB_create_empty(picked, colour_attdef, &colour_attrib);
			PK_ATTRIB_set_doubles(colour_attrib, 0, 3, colour_doubles );
		} else {
			PK_ENTITY_delete_attribs(picked, colour_attdef,&nDeletedAttrib);
		}
		ReRender();
	}
	else
		message = "Nothing picked";
	AfxMessageBox(message);
	MakeCurrent( FALSE );

	return picked;
}

bool CMeshWorkView::PickParts(int &nParts, PK_PART_t *&pParts, PK_TRANSF_t *pTransf, PK_AXIS1_sf_t &ray, PK_BODY_pick_topols_o_t options, PK_ENTITY_t *pPicked, int *pOccurence )
{
	CMeshWorkDoc* pDoc = GetDocument();
	ASSERT( pDoc );

	PK_CLASS_t tClass;
	for ( int i = 0; i < nParts; i++ )
	{
		tClass	= PK_CLASS_null;
		PK_ENTITY_ask_class( pParts[i], &tClass );
		if ( tClass == PK_CLASS_body )
			;
		else
		{
			TRACE("CParaViewView::PickParts->Unexpected class of part\n");
			return false;
		}
	}

	if ( nParts )
	{
		InstancedTag picked = PickBodies( nParts, pParts, pTransf, ray, options );
		if ( picked.tag != PK_ENTITY_null && picked.occurence != -1 ) {
			*pPicked = picked.tag;
			*pOccurence = picked.occurence;
			return true;
		}
	}

	return false;
}

InstancedTag CMeshWorkView::PickBodies(int nParts, PK_PART_t *pParts, PK_TRANSF_t *pTransf, PK_AXIS1_sf_t &ray, PK_BODY_pick_topols_o_t &options )
{
	PK_BODY_pick_topols_r_t	pickResults;
	
	PK_ERROR_t res = PK_BODY_pick_topols( nParts, pParts, pTransf, &ray, &options, &pickResults );
	if ( res != PK_ERROR_no_errors )
		throw res;

	CMeshWorkDoc	*pDoc	= GetDocument();
	ASSERT( pDoc );

	BOOL transparentFaces = ( pDoc->m_viewStyle == Wireframe || 
							  pDoc->m_viewStyle == WireAndSils ||
							  pDoc->m_viewStyle == DraftingHidden
							   );

	// only need to go to this code if we have found something
	PK_VECTOR_t vvec, evec, fvec;
	double vdist, edist, fdist;
	if (pickResults.n_vertices > 0)
	{
		VectorSubtract( ray.location.coord, pickResults.vertices[ 0 ].intersect.coord, vvec.coord );
		vdist = sqrt( VectorDot( vvec.coord, vvec.coord ) );
	}
	if (pickResults.n_edges > 0)
	{
		VectorSubtract( ray.location.coord, pickResults.edges[ 0 ].intersect.coord, evec.coord );
		edist = sqrt( VectorDot( evec.coord, evec.coord ) );
	}
	if (pickResults.n_faces > 0)
	{
		VectorSubtract( ray.location.coord, pickResults.faces[ 0 ].intersect.coord, fvec.coord );
		fdist = sqrt( VectorDot( fvec.coord, fvec.coord ) );
	}
	
	BOOL foundV = (pickResults.n_vertices > 0);
	BOOL foundE = (pickResults.n_edges > 0);
	BOOL foundF = (pickResults.n_faces > 0);

	// special case code for wirebodies to ensure that if in dual display mode body is correctly
	// highlighted when selecting bodies
	if ( pDoc->m_viewStyle == Hidden )
	{
		if ( foundE )
		{
			PK_BODY_t body = PK_ENTITY_null;
			PK_EDGE_ask_body( pickResults.edges[0].entity, &body );
			if ( body != PK_ENTITY_null )
			{
				PK_BODY_type_t type = PK_ENTITY_null;
				PK_BODY_ask_type( body, &type );
				if ( type == PK_BODY_type_wire_c )
					transparentFaces = TRUE;
			}
		}
	}

	InstancedTag picked;
	double		radius	= options.max_edge_dist;

	picked.tag	= PK_ENTITY_null;
	picked.occurence = -1;

	if (transparentFaces)
	{
		if (foundV ) {
			picked.tag = pickResults.vertices[ 0 ].entity;
			picked.occurence = pickResults.vertices[ 0 ].occurence;
		}
		else if (foundE ) {
			picked.tag = pickResults.edges[ 0 ].entity;
			picked.occurence = pickResults.edges[ 0 ].occurence;
		}
		else if (foundF) {
			picked.tag = pickResults.faces[ 0 ].entity;
			picked.occurence = pickResults.faces[ 0 ].occurence;
		}
		else
		{   picked.tag = PK_ENTITY_null; picked.occurence = -1;		}
	}
	
	else
	{
		if (foundV && fdist - vdist > -radius ) {
			picked.tag = pickResults.vertices[ 0 ].entity;
			picked.occurence = pickResults.vertices[ 0 ].occurence;
		}
		else if (foundE && fdist - edist > -radius ) {
			picked.tag = pickResults.edges[ 0 ].entity;
			picked.occurence = pickResults.edges[ 0 ].occurence;
		}
		else if (foundF) {
			picked.tag = pickResults.faces[ 0 ].entity;
			picked.occurence = pickResults.faces[ 0 ].occurence;
		}
		else
		{   picked.tag = PK_ENTITY_null; picked.occurence = -1;		}
	}

	if (pickResults.n_faces > 0) 
		PK_MEMORY_free( pickResults.faces );

	if (pickResults.n_edges > 0) 
		PK_MEMORY_free( pickResults.edges );

	if (pickResults.n_vertices > 0) 
		PK_MEMORY_free( pickResults.vertices );

	if ((pickResults.n_faces == 0) && (pickResults.faces != NULL)) 
		PK_MEMORY_free( pickResults.faces );

	if ((pickResults.n_edges == 0) && (pickResults.edges != NULL)) 
		PK_MEMORY_free( pickResults.edges );

	if ((pickResults.n_vertices == 0) && (pickResults.vertices != NULL)) 
		PK_MEMORY_free( pickResults.vertices );

	return picked;
}