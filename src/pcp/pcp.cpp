// pcp.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "pcp.h"


// This is an example of an exported variable
PCP_API int npcp=0;

// This is an example of an exported function.
PCP_API int fnpcp(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see pcp.h for the class definition
Cpcp::Cpcp()
{
	return;
}


