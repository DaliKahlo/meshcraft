//$c1 XRL 07/23/12 Created.
//////////////////////////////////////////////////////////
///
///

#ifndef TRIPLET_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_
#define TRIPLET_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_

#include "../../mesh/mesh_api.h"

struct Triplet
{
	pVertex v0, v1, v2;		// vertex in original orientation
	int iMin, iMed, iMax;	// sorted ids 

	bool Triplet::operator== (const Triplet &b) const
	{
		return (iMin == b.iMin && iMed == b.iMed && iMax == b.iMax);
	}

    bool Triplet::operator< (const Triplet &b) const
	{
		if(iMin != b.iMin)
			return (iMin < b.iMin);
		if(iMed != b.iMed)
			return (iMed < b.iMed);
		return (iMax < b.iMax);
	}

	bool Triplet::operator> (const Triplet &b) const
	{
		if(iMin != b.iMin)
			return (iMin > b.iMin);
		if(iMed != b.iMed)
			return (iMed > b.iMed);
		return (iMax > b.iMax);
	}
};

#endif