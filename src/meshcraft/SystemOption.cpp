//$c3   XRL 06/06/2013 Added interior lighting status 
//$c2   XRL 11/27/2012 Added clip plane 
//$c1   XRL 02/02/2011 Created 
//========================================================================//
//              Copyright 2009 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#include "SystemOption.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

CSystemOption::CSystemOption()
{
	m_style =  Smooth;
	m_bkcolor = 0;		 // need change view initialization if changing this default
	m_clipPlaneIdx = 0;  // no clip plane
	m_clipPlanePos = 50;
	m_pointsize = 0.1;
	m_transparency = 1.0;	   // opaque by default
	m_lightInterior = 0;	   // off

	m_intersectCorrect = 1;    // not
	m_defaultSizeCompute = 0;  // Auto for the whole assembly
}

