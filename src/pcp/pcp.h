//#include "typedef.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PCP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PCP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PCP_EXPORTS
#define PCP_API __declspec(dllexport)
#else
#define PCP_API __declspec(dllimport)
#endif

// This class is exported from the pcp.dll
class PCP_API Cpcp {
public:
	Cpcp(void);
	// TODO: add your methods here.
};


// exported variables
extern PCP_API int npcp;

// exported functions
PCP_API int fnpcp(void);
PCP_API int pc_align(int nsrc, double *xyzsrc, int ntgt, int nf, double *xyztgt, int *triatgt, double *score, double trans[4][4], char *);
PCP_API int pc_greedy_projection(int np, double *xyz, double, int *ne, int **elems);