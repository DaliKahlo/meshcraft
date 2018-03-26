//$c2   XRL 04/03/2014 add withAttribData()
//$c1   XRL 10/09/2013 Created 
//========================================================================//
//
// AssemblyMeshMgr.h: manager assembly mesh objects 
//                    to support insert, convex/shrinkwrap
//
//========================================================================//

#ifndef ASSEMBLYMGR_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define ASSEMBLYMGR_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#include "MeshControls.h"
#include "AssemblyMesh.h"

typedef class AssemblyMeshMgr * pAssemblyMeshMgr;
typedef class AssemblyMesh  * pAssemblyMesh;
typedef class MeshControl  * pMeshControl;


class AssemblyMeshMgr  
{
public:
	AssemblyMeshMgr();
	virtual ~AssemblyMeshMgr();

	void clear();
	void append(pAssemblyMesh);
	int size() { return m_pAsmMeshArray.size(); }
	pAssemblyMesh ith(int i) { return m_pAsmMeshArray[i]; }
	int current() { return currentAsmMeshIndex; }
	void setCurrent(int c) { currentAsmMeshIndex = c; }

	pAssemblyMesh withAttribData(ATTRTYPE);

	int getFreeID() { return idCounter++; }

protected:

private:
	
	std::vector<pAssemblyMesh> m_pAsmMeshArray; 
	int currentAsmMeshIndex;					// 0,1,2,3...
	int idCounter;
};

#endif
