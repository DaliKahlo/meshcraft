//$c1   XRL 10/11/2011 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//                  MSC Software Corp.                                      //
//========================================================================//
#include "vertex.h"

Vertex::Vertex(double p[3], pGEntity g)
: Entity(g)
{
	xyz[0] = p[0]; xyz[1] = p[1]; xyz[2] = p[2];
	params[0] = params[1] = 0;
	temp_id = 0;
	_mark = 3;			// classified on model region, all other  bits zero
}

void Vertex::get_coordinates(double *p)
{
	p[0] = xyz[0]; p[1] = xyz[1]; p[2] = xyz[2]; 
}

void Vertex::set_coordinates(double x, double y, double z)
{
	xyz[0] = x; xyz[1] = y; xyz[2] = z;
}

void Vertex::set_param(double par[2])
{
	params[0] = par[0]; params[1] = par[1];
}

void Vertex::get_param(double par[2])
{
	par[0] = params[0]; par[1] = params[1];
}

// topotype: 0 vertex; 1 edge; 2 face; 3 region
void Vertex::reset_classification_type(int topotype)
{
	_mark &= ~3;				// clear bit 0 and 1 to 00
	_mark |= topotype;			// set bit 0 and 1
}

int Vertex::get_classification_type()
{
	return (_mark & 3);
}