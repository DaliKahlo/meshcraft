//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//																		  //
//========================================================================//
#ifndef MESH_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESH_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
///////////////////////////////////////////////////////////////////////////////////////////////

#include "typedef.h"

class Mesh 
{
public:
	Mesh() {}
	virtual ~Mesh() {}

	// query 
	virtual pGEntity model() = 0;

	virtual int numFaces() const = 0;
	virtual int numEdges() const = 0;
	virtual int numVertices() const = 0;

	// modify mesh
	virtual pFace createFace(int, pVertex *, pGEntity) = 0;
	virtual pEdge createEdge(pVertex, pVertex, pGEntity) = 0;
	virtual pVertex createVertex(double[3], pGEntity) = 0;
	virtual pVertex createVertex(double[3], double[2], pGEntity) = 0;

	virtual void remove(pFace) = 0;
	virtual void removeClassifiedMesh(gType, pGEntity) = 0;

	// serialization
	virtual void toMEDIT(const char *fileName) = 0;
	virtual bool toArrays(int *, int *, int **, double **) = 0;
};


#endif