//$c2 XRL 02/20/14 Replaced PK_TRANS_t with osg::Matrix, added preMult().
//$c1 XRL 02/01/12 created.
//================================================================
//
// Assembly.cpp: implementation of the CMeshGen class.
//
//================================================================

#include "stdafx.h"
#include "instance.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Instance::Instance(int occ, int bIndex)
{
	m_transf.makeIdentity();
	m_identity = true;
	m_occurence = occ;
	m_bodyIndex = bIndex;
}

Instance::~Instance()
{
}

void Instance::postMult(osg::Matrix &sf)
{
	m_identity = false;
	//m_transf.preMult(sf);
	m_transf.postMult(sf);
}

void Instance::setTransform(double a00, double a01, double a02, double a03,
					        double a10, double a11, double a12, double a13,
					        double a20, double a21, double a22, double a23,
					        double a30, double a31, double a32, double a33)
{
	m_transf.set(a00,a01,a02,a03,
		         a10,a11,a12,a13,
				 a20,a21,a22,a23,
				 a30,a31,a32,a33);
	if(a00 != 1.0 || a11 != 1.0 || a22 != 1.0 || a33 != 1.0 ||
	   a01 != 0.0 || a02 != 0.0 || a03 != 0.0 ||
	   a10 != 0.0 || a12 != 0.0 || a13 != 0.0 ||
	   a20 != 0.0 || a21 != 0.0 || a23 != 0.0 ||
	   a30 != 0.0 || a31 != 0.0 || a32 != 0.0)
		m_identity = false;
}

bool Instance::isIdentity()
{
	return m_identity;
}