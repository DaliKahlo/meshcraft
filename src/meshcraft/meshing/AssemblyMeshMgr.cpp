//$c2   XRL 04/03/2014 add withAttribData()
//$c1   XRL 10/09/2013 Created 
//================================================================
//
// AssemblyMeshMgr.cpp: implementation of the AssemblyMeshMgr class.
//
//================================================================

#include "stdafx.h"
#include "util.h"
#include "AssemblyMeshMgr.h"


AssemblyMeshMgr::AssemblyMeshMgr()
{
	currentAsmMeshIndex = 0;				// no assembly mesh
	idCounter = 1;
}		

AssemblyMeshMgr::~AssemblyMeshMgr()
{
	clear();
}

void AssemblyMeshMgr::clear() 
{ 
	std::vector<pAssemblyMesh>::iterator itrM;
	for(itrM=m_pAsmMeshArray.begin(); itrM!=m_pAsmMeshArray.end(); itrM++) {
		if( *itrM )
			delete (*itrM);
	}
	m_pAsmMeshArray.clear();
	currentAsmMeshIndex = 0;
}

void AssemblyMeshMgr::append(pAssemblyMesh me) 
{ 
	m_pAsmMeshArray.push_back(me);
}

// find the first AssemblyMesh with the specified attribute data
pAssemblyMesh AssemblyMeshMgr::withAttribData(ATTRTYPE atype)
{
	std::vector<pAssemblyMesh>::iterator itrM;
	for(itrM=m_pAsmMeshArray.begin(); itrM!=m_pAsmMeshArray.end(); itrM++) {
		if( (*itrM)->attr_type() == atype )
			return (*itrM);
	}
	return NULL;
}