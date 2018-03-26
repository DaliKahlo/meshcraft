//$c2   XRL 11/11/2011 Added member data "mach"
//$c1   XRL 02/10/2011 Created 
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//		Add all global mesh control options here.						  //	
//========================================================================//

#ifndef MESHOPTIONS_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHOPTIONS_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

//#include <windows.h>
#include "mgc_typedef.h"

enum MGCSerializationFormat
{
	MGC_SERIALIZATION_NONE		= 0, 
	MGC_SERIALIZATION_BDF		= 1, 
	MGC_SERIALIZATION_MEDIT		= 2, 
};

#define MAXAR_DEFAULT 1000
#define MAX_FILE_PATH 260

// ---------------------------------------------------- // 
struct MeshOpt {
	char dataFileName[MAX_FILE_PATH];	// input name without extension
	char dataFilePath[MAX_FILE_PATH];	// relative path to rtest data
	char mach[12];					// platform: WINNT, WIN8664
	int out;						// 0 nooutput; 1 BDF; 2 MEDIT

	MGCMesherType mesher;			// 1 Hybrid; 2 Distene; 2 tessadapt
	MGCElemType element;			// 3 tri3; 5 quad4 dominant; 7 tet4; 
	int method;						// 0 iso; 1 auto; 2 minimal

	double meshsize;				// 0 means default, absolute value
	double gradation;				// growth ratio	
	int scale;						// 1 - 10 with one max speed

	bool curvature;					// true - activate curvature driven sizing
	double min_csize;				// allowed minimum size from curvature
	double angle;					// limiting angle for curve/edge and surface/face 

	bool proximity;					// true - activate proximity driven sizing
	double min_psize;				// allowed minimum size from proximity
	
	bool relax;						// true - improve quality by relaxing topo conatrains
	double minLength;				// threshold for short distance
	
	bool interCorrect;				// ture - activate self intersection correction of cadsurf

	int verb;						// 1 - 10

	bool absolute;					
	int regression_test;			// 0 not; 1 repeated mesh; 2 interrogate mesh
};

#endif