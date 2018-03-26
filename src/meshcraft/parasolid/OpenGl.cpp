//$c1 XRL 07/31/12 Changed background color the same as osgview.
//=========================================================================
//
// CMeshWorkView.cpp : implementation of openGL related 
//
//=========================================================================

#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkView.h"


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


//////////////////////////////////////////////////////////////////////////////
// OpenGL specific functions


//-----------------------------------------------------------------------------
// FUNC:	InitOpenGL
// ACTION:	Creates DC, OpenGL RC and binds them. Also sets up lights etc

void CMeshWorkView::InitOpenGL()
{
    m_pDC = new CClientDC(this);
    ASSERT(m_pDC != NULL);

    if (!SetupPixelFormat())   // deleaker
        return;

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	int bkcolor = mwApp->sysOption()->getBKColor();

    CreateRGBPalette();

    VERIFY( m_glContext = wglCreateContext(m_pDC->GetSafeHdc()) );

	MakeCurrent( TRUE );

	// set up lights (one directional light)
    GLfloat light_position[]	=	{ 1.0f, -1.0f, 1.0f, 0.0f };

    VERIFY_GL( glLightfv( GL_LIGHT0, GL_POSITION, light_position ) );
    VERIFY_GL( glEnable(GL_LIGHTING) );
    VERIFY_GL( glEnable(GL_LIGHT0) );

	// set up material properties
	GLfloat mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_shininess[] = { 40.0f };

    VERIFY_GL( glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular) );
    VERIFY_GL( glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess) );

	// specify depth function and enable it
    VERIFY_GL( glDepthFunc(GL_GREATER) );
    VERIFY_GL( glEnable(GL_DEPTH_TEST) );

	// intialize the transformation matrices
    VERIFY_GL( glMatrixMode(GL_MODELVIEW) );
	VERIFY_GL( glLoadIdentity() );
	VERIFY_GL( glMatrixMode(GL_PROJECTION) );
	VERIFY_GL( glLoadIdentity() );
	VERIFY_GL( glMatrixMode(GL_MODELVIEW) );

	// don't care for back faces
	VERIFY_GL( glFrontFace( GL_CCW ) );
	VERIFY_GL( glCullFace( GL_BACK ) );
	VERIFY_GL( glEnable( GL_CULL_FACE ) );
	VERIFY_GL( glDisable( GL_NORMALIZE ) );

	// create and reserve a display list index from OpenGL
	VERIFY_GL( m_partDisplaylist = glGenLists(1) );

	// set the viewport, we know the OnSize has already been called
	VERIFY_GL( glViewport( 0, 0, m_winWidth, m_winHeight ) );

	// set the buffer clear colour to defined background color and clear the buffers
	VERIFY_GL( glClearColor( BKCOLOR_R,
							 BKCOLOR_G,
							 BKCOLOR_B, 1.0 ) );

	VERIFY_GL( glClearDepth( -10.0 ) );
	VERIFY_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

	MakeCurrent( FALSE );
}

//-----------------------------------------------------------------------------
// FUNC:	SetupPixelFormat
// ACTION:	Sets up the pixel format in to the HDC so that OpenGL is ready
//			to work. This function is straight from OpenGL example code.

BOOL CMeshWorkView::SetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd = 
	{
        sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
        1,                              // version number
        PFD_DRAW_TO_WINDOW |            // support window
          PFD_SUPPORT_OPENGL |          // support OpenGL
          PFD_DOUBLEBUFFER,             // double buffered
        PFD_TYPE_RGBA,                  // RGBA type
        24,                             // 24-bit color depth
        0, 0, 0, 0, 0, 0,               // color bits ignored
        0,                              // no alpha buffer
        0,                              // shift bit ignored
        0,                              // no accumulation buffer
        0, 0, 0, 0,                     // accum bits ignored
        32,                             // 32-bit z-buffer
        0,                              // no stencil buffer
        0,                              // no auxiliary buffer
        PFD_MAIN_PLANE,                 // main layer
        0,                              // reserved
        0, 0, 0                         // layer masks ignored
    };
    int pixelformat;

    if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )   // deleaker
    {
        MessageBox(_T("ChoosePixelFormat failed"));
        return FALSE;
    }

    if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
    {
        MessageBox(_T("SetPixelFormat failed"));
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// FUNC:	CreateRGBPalette
// ACTION:	This code is lifted from MSDN sample. It is needed for coloring

void CMeshWorkView::CreateRGBPalette()
{
	static int defaultOverride[13] = 
	{
		0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
	};

	static PALETTEENTRY defaultPalEntry[20] = 
	{
		{ 0,   0,   0,    0 },
		{ 0x80,0,   0,    0 },
		{ 0,   0x80,0,    0 },
		{ 0x80,0x80,0,    0 },
		{ 0,   0,   0x80, 0 },
		{ 0x80,0,   0x80, 0 },
		{ 0,   0x80,0x80, 0 },
		{ 0xC0,0xC0,0xC0, 0 },

		{ 192, 220, 192,  0 },
		{ 166, 202, 240,  0 },
		{ 255, 251, 240,  0 },
		{ 160, 160, 164,  0 },

		{ 0x80,0x80,0x80, 0 },
		{ 0xFF,0,   0,    0 },
		{ 0,   0xFF,0,    0 },
		{ 0xFF,0xFF,0,    0 },
		{ 0,   0,   0xFF, 0 },
		{ 0xFF,0,   0xFF, 0 },
		{ 0,   0xFF,0xFF, 0 },
		{ 0xFF,0xFF,0xFF, 0 }
	};

    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *pPal;
    int n, i;
 
    n = ::GetPixelFormat(m_pDC->GetSafeHdc());
    ::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        n = 1 << pfd.cColorBits;
        pPal = (PLOGPALETTE) new char[sizeof(LOGPALETTE) + n * sizeof(PALETTEENTRY)];

        ASSERT(pPal != NULL);

        pPal->palVersion = 0x300;
        pPal->palNumEntries = n;

        for (i=0; i<n; i++)
        {
            pPal->palPalEntry[i].peRed =
                    ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
            pPal->palPalEntry[i].peGreen =
                    ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
            pPal->palPalEntry[i].peBlue =
                    ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
            pPal->palPalEntry[i].peFlags = 0;
        }

        // fix up the palette to include the default GDI palette 
        if ((pfd.cColorBits == 8)                           &&
            (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
            (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
            (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
           )
        {
			for (i = 1 ; i <= 12 ; i++)
                pPal->palPalEntry[defaultOverride[i]] = defaultPalEntry[i];
        }


        m_Palette.CreatePalette(pPal);
        delete pPal;

		m_pOldPalette = m_pDC->SelectPalette(&m_Palette, FALSE);
        m_pDC->RealizePalette();
    }
}


unsigned char CMeshWorkView::ComponentFromIndex( int i, UINT nbits, UINT shift )
{
	// This function was ripped from OpenGL example code. It sets things up.

    unsigned char threeto8[8] = 
	{
		0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
	};

	unsigned char twoto8[4] = 
	{
		0, 0x55, 0xaa, 0xff
	};

	unsigned char oneto8[2] = 
	{
		0, 255
	};
    unsigned char val;

    val = (unsigned char) (i >> shift);
    switch (nbits) 
	{

    case 1:
        val &= 0x1;
        return oneto8[val];
    case 2:
        val &= 0x3;
        return twoto8[val];
    case 3:
        val &= 0x7;
        return threeto8[val];

    default:
        return 0;
    }

}