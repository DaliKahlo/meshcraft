//$c1   XRL 02/01/2011 Created 
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#ifndef PARASESSION4_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define PARASESSION4_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#pragma once

#include "parasolid_kernel.h"

class CSession_c  
{
public:

	bool Start();
	bool Stop();

	static PK_ERROR_code_t PKerrorHandler( PK_ERROR_sf_t* error );
	
	CSession_c();
	~CSession_c();

private:
	bool parasolidStarted;
};
#endif