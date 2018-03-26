//$c1   XRL 07/16/2014 Created 
//========================================================================
//
// ErrorData.h: collect and show error locations
//
//========================================================================

#ifndef CERRORDATA_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_
#define CERRORDATA_H__029BF5BC_F986_11D2_8C00_0000F8071DC8__INCLUDED_

#include <vector>

class CErrData
{
public:
	CErrData();
	~CErrData();

	void clear();
	void addCode(int);
	void addEdge(int, int);
	void addFace(int, int, int);

	void show(int, double *);
	int code(int);

private:
	std::vector<int> codes;
	std::vector<int> edges;
	std::vector<int> faces;
};

#endif