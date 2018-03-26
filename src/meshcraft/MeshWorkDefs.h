//$c2   XRL 03/18/2013 Added MACRO LACK_WHITE_MODE
//$c1   XRL 02/01/2011 Created 
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#if !defined(AFX_MESHWORKDEFS_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_)
#define AFX_MESHWORKDEFS_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "math.h"

#define M_PI       3.14159265358979323846

//*****************************************************************************
// MODULE:	Macros
// DESC:	Defines the utility macros

#ifdef NOCHECK_GL
	#define VERIFY_GL( call ) { call; }	
#else 
	#define VERIFY_GL( call ) { call; ASSERT( glGetError() == GL_NO_ERROR ); }	
#endif

// Graphics View Style
enum ViewStyle 
{ 
	Shaded, 
	ShadedLines,
	ShadedFacets,
	Wireframe,
	WireAndSils,
	Hidden,
	DraftingHidden
};

// Scene graph styple
enum SceneStyle
{
	Smooth,
	Facets
};

// Operation mode
enum ViewOperation
{ 
	Idle,
	Rotate,
	Zoom,
	Pan,
	OsgDefault,
	RotatePart,
	MovePart
};	


////*****************************************************************************
//#define BLACK_WHITE_MODE 1
#ifndef BLACK_WHITE_MODE
// default face colour	
#define FCOLOR_R	1.0f		
#define FCOLOR_G	1.0f
#define FCOLOR_B	1.0f
// default edge colour
#define ECOLOR_R	0.0f
#define ECOLOR_G	0.0f
#define ECOLOR_B	0.0f
// selected colours		red
#define SELECTED_FCOLOR_R	1.0f
#define SELECTED_FCOLOR_G	0.0f
#define SELECTED_FCOLOR_B	0.0f
// background colour		dark blue
#define BKCOLOR_R	0.2f
#define BKCOLOR_G	0.2f
#define BKCOLOR_B	0.4f
#define BKGDCOLOR	RGB(51, 51, 102)
#else

//*****************************************************************************
// black/white/grey colour setting
// default face colour
#define FCOLOR_R	1.0f
#define FCOLOR_G	1.0f
#define FCOLOR_B	1.0f
// default edge colour
#define ECOLOR_R	0.0f
#define ECOLOR_G	0.0f
#define ECOLOR_B	0.0f
// selected colour
#define SELECTED_FCOLOR_R	0.3f
#define SELECTED_FCOLOR_G	0.3f
#define SELECTED_FCOLOR_B	0.3f
// background colour
#define BKCOLOR_R	1.0f
#define BKCOLOR_G	1.0f
#define BKCOLOR_B	1.0f
#define BKGDCOLOR	RGB(255, 255, 255)
#endif

//*****************************************************************************
// MODULE:	Default
// DESC:	Defines the default values for various settings

#define ROTATE_SENSITIVITY	0.4
#define PAN_SENSITIVITY		0.005
#define ZOOM_SENSITIVITY	0.01

#define MIN_SCALE_FACTOR	0.0 

#define	MAX_SPHERE_SIZE		1000

///////////////////////////////////////////////////////////////////////////////
// Vector manipulation functions:

inline double VectorDot( const double* x, const double* y )
{
	return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];
}

inline double* VectorSubtract( const double* x, const double* y, double* z )
{
	z[ 0 ] = x[ 0 ] - y[ 0 ];
	z[ 1 ] = x[ 1 ] - y[ 1 ];
	z[ 2 ] = x[ 2 ] - y[ 2 ];
	return z;
}

inline double* VectorAdd( const double* x, const double* y, double* z )
{
	z[ 0 ] = x[ 0 ] + y[ 0 ];
	z[ 1 ] = x[ 1 ] + y[ 1 ];
	z[ 2 ] = x[ 2 ] + y[ 2 ];
	return z;
}

inline double* VectorMult( const double* x, double m, double* y )
{
	y[ 0 ] = x[ 0 ] * m;
	y[ 1 ] = x[ 1 ] * m;
	y[ 2 ] = x[ 2 ] * m;
	return y;
}

inline double* VectorNormalise( double* x )
{
	return VectorMult( x, 1.0 / sqrt( VectorDot( x, x ) ), x );
}

inline double* VectorCross( const double* x, const double* y, double* z )
{
	z[ 0 ] = x[1]*y[2] - x[2]*y[1];
	z[ 1 ] = x[2]*y[0] - x[0]*y[2];
	z[ 2 ] = x[0]*y[1] - x[1]*y[0];
	return z;
}	

#endif