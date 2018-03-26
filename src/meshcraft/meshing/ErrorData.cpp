//$c1   XRL 07/16/2014 Created
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#include "stdafx.h"
#include "MeshWorks.h"
#include "MainFrm.h"
#include "MeshWorkOsgView.h"
#include "ErrorData.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

CErrData::CErrData()
{
	clear();
}

CErrData::~CErrData()
{
	clear();
}

void CErrData::clear()
{
	codes.clear();
	edges.clear();
	faces.clear();
}

void CErrData::addCode(int c)
{ codes.push_back(c); }

void CErrData::addEdge(int a, int b)
{ 
	if(edges.size() > 1) {
		std::vector<int>::iterator ritr = edges.end();
		int c,d;
		ritr--; c=*ritr;
		ritr--; d=*ritr;
		if((a==c && b==d) || (a==d &&b==c))
			return;
	}
	edges.push_back(a);
	edges.push_back(b);
}

void CErrData::addFace(int a, int b, int c)
{
	if(faces.size() > 2) {
		std::vector<int>::iterator ritr = faces.end();
		int d,e,f;
		ritr--; d=*ritr; 
		ritr--; e=*ritr;
		ritr--; f=*ritr;
		if(a+b+c == d+e+f)
			return;
	}
	faces.push_back(a);
	faces.push_back(b);
	faces.push_back(c);
}

int CErrData::code(int ith)
{
	if(ith < (int) codes.size())
		return codes[ith];
	return 0;
}

void CErrData::show(int n, double *xyz)
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	CMeshWorkOsgView* meView = (CMeshWorkOsgView *) mwApp->MeshView();
	
	double fxyz[4][3], refxyz[2][3];
	int i,j, ref=0;
	meView->ClearHighLight();
	meView->ClearHUDText();

	std::vector<int>::iterator itr;
	for(itr=faces.begin(); itr!=faces.end(); ) {
		for(j=2; j>=0; j--) {
			i= *itr; itr++;
			fxyz[j][0] = xyz[(i-1)*3];
			fxyz[j][1] = xyz[(i-1)*3 + 1];
			fxyz[j][2] = xyz[(i-1)*3 + 2];
			if(!ref) {
				ref = 1;
				refxyz[0][0] = fxyz[j][0];
				refxyz[0][1] = fxyz[j][1];
				refxyz[0][2] = fxyz[j][2];
			}
		}
		meView->HighLight(3,fxyz);
	}

	for(itr=edges.begin(); itr!=edges.end(); ) {
		for(j=0; j<2; j++) {
			i= *itr; itr++;
			fxyz[j][0] = xyz[(i-1)*3];
			fxyz[j][1] = xyz[(i-1)*3 + 1];
			fxyz[j][2] = xyz[(i-1)*3 + 2];
			if(!ref) {
				ref = 1;
				refxyz[0][0] = fxyz[j][0];
				refxyz[0][1] = fxyz[j][1];
				refxyz[0][2] = fxyz[j][2];
			}
		}
		meView->HighLight(2,fxyz);
	}

	if(ref == 1) {
		refxyz[1][0] = 0.0;
		refxyz[1][1] = 0.0;
		refxyz[1][2] = 0.0;
		meView->HighLight(2,refxyz);
	}

	if(edges.size() >= 2 && faces.size() >= 3) {
		std::stringstream oss;
		oss << std::setprecision(6) << "Detect " << edges.size() /2 << " edge(s) and " << faces.size() /3 << " face(s) intersecting.";
		meView->HudText(1, oss.str(),true);
	}
	else if(edges.size() >= 2) {
		std::stringstream oss;
		oss << std::setprecision(6) << "Detect " << edges.size() /2 << " intersecting edge(s).";
		meView->HudText(1, oss.str(),true);
	}
	else if(faces.size() >= 3) {
		std::stringstream oss2;
		oss2 << std::setprecision(6) << "Detect " << faces.size() /3 << " face(s).";
		meView->HudText(2, oss2.str(),true);
	}
}