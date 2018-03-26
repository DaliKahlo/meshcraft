//$c2 XRL 03/11/14 Add lineplot and colormap.
//$c1 XRL 11/02/13 Created.
//========================================================================//
//              Copyright 2008 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkOsgView.h"
#include "util.h"
#include "adaptapi_internal.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

int CMeshWorkOsgView::Segment(double xyz[64][3])
{
	double vec[3], d;
	diffVt(xyz[0], xyz[1], vec);
	d = sqrt( dotProd(vec,vec) );
	mOSG->highlightPolyLine(2,xyz,3.0f,osg::Vec4(1.0f,0.0f,0.0f,1.0f));
	
	int w,h;
	float x,y,dh,ts;
	mOSG->GetScreenAndTextSize(&w, &h, &ts);
	dh= ts+3.0f;
	x = 0.004*w;
	y = h-3.0*dh;	
				
	std::stringstream oss;
	oss << std::setprecision(7) << "Length: " << d << "\n";	
	mOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());

	mOSG->Render(mOSG);
	return 0;
}

int CMeshWorkOsgView::ThreePointCircle(double xyz[3][3])
{
	double center[3], axis[3], r;

	int err = circle(xyz, center, &r, axis);
	if(err)
		return err;

	mOSG->highlightCircle(center,axis, r);
	mOSG->highlightP(center,0.02*r,0);
	
	int w,h;
	float x,y,ts,dh;
	mOSG->GetScreenAndTextSize(&w, &h, &ts);
	dh= ts+3.0f;
	x = 0.004*w;
	y = h-4.0*dh;	
				
	std::stringstream oss;
	oss << std::setprecision(7) << "Center: (" << center[0] << ", " << center[1] << ", " << center[2] << ")\n";	
	oss << std::setprecision(7) << "Radius: " << r;	
	mOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());

	mOSG->Render(mOSG);
	return 0;
}

int CMeshWorkOsgView::FourPointSphere(double xyz[10][3])
{
	double center[3], r;

	int err = XYZ_circumSphere(xyz, center, &r);
	if(err)
		return err;

	int w,h;
	float x,y,ts,dh;
	mOSG->GetScreenAndTextSize(&w, &h, &ts);
	dh = ts+3.0f;
	x = 0.004*w;
	y = h-5.0*dh;	
				
	std::stringstream oss;
	oss << std::setprecision(7) << "Center: (" << center[0] << ", " << center[1] << ", " << center[2] << ")\n";	
	oss << std::setprecision(7) << "Radius: " << r;	
	mOSG->addHudText(osg::Vec3(x, y, 0.0f), oss.str());

	mOSG->highlightP(center,r,1);
	mOSG->highlightP(center,0.02*r,0);
	mOSG->Render(mOSG);
	return 0;
}

void CMeshWorkOsgView::PostMult(osg::Matrix &mat, int index)
{
	mOSG->postMult(mat,index);
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::LinePlot(int np, double *xyz, double *proj, double minV, double maxV)
{
	mOSG->lineplot(np,xyz,proj,minV,maxV);
	mOSG->hidemodels();
	//this->SetViewOperation(Idle); // ??
	mOSG->Render(mOSG);
}

void CMeshWorkOsgView::ColorMap(int nv, int nf, double *xyz, int *tria, double *values, double minV, double maxV)
{
	mOSG->colormap(nv,nf,xyz,tria,values,minV,maxV);
	mOSG->hidemodels();
	//this->SetViewOperation(Idle); // ??
	mOSG->Render(mOSG);
}

int CMeshWorkOsgView::ShowColorMap()
{
	if( mOSG->showcolormap() ) {
		mOSG->hidemodels();
		//this->SetViewOperation(Idle); // ??
		mOSG->Render(mOSG);
		return 1;
	}
	return 0;
}

int CMeshWorkOsgView::ShowLinePlot()
{
	if( mOSG->showlineplot() ) {
		mOSG->hidemodels();
		///this->SetViewOperation(Idle); // ??
		mOSG->Render(mOSG);
		return 1;
	}
	return 0;
}